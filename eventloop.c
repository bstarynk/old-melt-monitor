// file eventloop.c - the event loop in main thread, and also dynamic loading of mdules

/**   Copyright (C)  2015 Free Software Foundation, Inc.
    MONIMELT is a monitor for MELT - see http://gcc-melt.org/
    This file is part of GCC.
  
    GCC is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.
  
    GCC is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with GCC; see the file COPYING3.   If not see
    <http://www.gnu.org/licenses/>.
**/

#include "monimelt.h"

static int writefdeventloop_mom = -1;
static int readfdeventloop_mom = -1;
static int signalfd_mom = -1;
static int timerfd_mom = -1;
static int mastersocketfd_mom = -1;
static char *finaleventloopdump_mom;

typedef void mom_poll_handler_t (int fd, short revent, intptr_t dataindex);
static void eventloopupdate_mom (void);
static void signalprocess_mom (void);
static void ringtimer_mom (void);
static void process_evcb_mom (int fd, short revent, intptr_t dataindex);
static void mastersocket_evcb_mom (int fd, short revent, intptr_t dataindex);
static void activesocket_evcb_mom (int fd, short revent, intptr_t dataindex);

struct runningbatchprocess_mom_st
{
  pid_t bproc_pid;
  int bproc_outfd;
  momvalue_t bproc_hclosv;
  char *bproc_outbuf;
  unsigned bproc_outsiz;
  unsigned bproc_outoff;
  long bproc__spare;
};

static struct runningbatchprocess_mom_st
  runbatchproc_mom[MOM_MAX_WORKERS + 2];
static int nb_processes_mom;
// the queue of nodes ^in(<closure>,<procnode>)
static struct momqueuevalues_st pendingbatchprocque_mom;



struct socket_mom_st
{
  int msock_fd;
  momitem_t *msock_itm;
  momvalue_t msock_handlerv;
};

#define MOM_MAX_SOCKETS (2*MOM_MAX_WORKERS)
static struct socket_mom_st activesockets_mom[MOM_MAX_SOCKETS + 2];
int nb_sockets_mom;

static CURLM *curlmulti_mom;

#define MAX_POLL_MOM (MOM_MAX_SOCKETS+MOM_MAX_WORKERS+10)
#if MAX_POLL_MOM > 256
#warning MAX_POLL_MOM is quite big
#endif

/***
We should dlopen every module with RTLD_GLOBAL | RTLD_NOW |
RTLD_DEEPBIND so that the symbols it is defining are visible from the
main program and from other modules. Then, we can always use dlopn on
the main program handle.
 ***/

static pthread_mutex_t dynload_mtx_mom = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t eventloop_mtx_mom = PTHREAD_MUTEX_INITIALIZER;


void *
mom_dynload_symbol (const char *name)
{
  void *ptr = NULL;
  assert (name && isalpha (name[0]));
  pthread_mutex_lock (&dynload_mtx_mom);
  // since plugins are dlopen-ed with RTLD_DEEPBIND we can just bind
  // symbols using the program handle.
  ptr = dlsym (mom_prog_dlhandle, name);
  if (!ptr)
    {
      MOM_WARNPRINTF ("dlsym %s failed : %s", name, dlerror ());
    }
  pthread_mutex_unlock (&dynload_mtx_mom);
  return ptr;
}

void
mom_wake_event_loop (void)
{
  if (MOM_UNLIKELY (write (writefdeventloop_mom, "*", 1) <= 0))
    MOM_FATAPRINTF
      ("wake_event_loop failed to write to event loop fd#%d : %m",
       writefdeventloop_mom);
}				/* end mom_wake_event_loop */


static int
open_bind_socket_mom (void)
{
  int sfd = -1;
  struct sockaddr_un sun;
  memset (&sun, 0, sizeof (sun));
  if (!strchr (mom_socket_path, '/')
      || strlen (mom_socket_path) >= (int) sizeof (sun.sun_path))
    MOM_FATAPRINTF ("bad path (no /) for AF_UNIX socket but got %s",
		    mom_socket_path);
  (void) remove (mom_socket_path);	// should remove the socket before binding it
  sfd = socket (AF_UNIX, SOCK_STREAM, 0);
  if (sfd < 0)
    MOM_FATAPRINTF ("socket AF_UNIX failed : %m");
  sun.sun_family = AF_UNIX;
  strncmp (sun.sun_path, mom_socket_path, sizeof (sun.sun_path) - 1);
  if (bind (mastersocketfd_mom, (struct sockaddr *) &sun, sizeof (sun)))
    MOM_FATAPRINTF ("failed to bind unix socket %s : %m", sun.sun_path);
  MOM_DEBUGPRINTF (run, "bind unix socket #%d to %s", sfd, sun.sun_path);
  return sfd;
}				/* end of open_bind_socket_mom */


struct polldata_fdmom_st
{
  mom_poll_handler_t *polld_hdlr;
  intptr_t polld_dataindex;
  int polld_fd;
  short polld_event;
};

static inline void
handle_polldata_mom (struct polldata_fdmom_st *pfd, fd_set *preadfdset,
		     fd_set *pwritefdset, fd_set *pexceptfdset)
{
  int curfd = pfd->polld_fd;
  if (curfd <= 0 || !pfd->polld_hdlr)
    return;
  short revent = 0;
  if ((pfd->polld_event & POLLIN) && FD_ISSET (curfd, preadfdset))
    revent |= POLLIN;
  if ((pfd->polld_event & POLLOUT) && FD_ISSET (curfd, pwritefdset))
    revent |= POLLOUT;
  if ((pfd->polld_event & POLLPRI) && FD_ISSET (curfd, pexceptfdset))
    revent |= POLLPRI;
  if (revent)
    (*pfd->polld_hdlr) (curfd, revent, pfd->polld_dataindex);
}

static inline void
read_output_from_batchproc_mom (struct runningbatchprocess_mom_st *bp,
				void *rbuf, size_t rsiz)
{
  if (bp->bproc_outfd <= 0)
    return;
  errno = 0;
  for (;;)
    {
      int rcnt = read (bp->bproc_outfd, rbuf, rsiz);
      if (rcnt < 0)
	{
	  if (errno == EINTR)
	    continue;
	  if (errno == EWOULDBLOCK)
	    return;
	  MOM_WARNPRINTF ("read output from batch process %d failed (%m)",
			  bp->bproc_pid);
	  close (bp->bproc_outfd);
	  bp->bproc_outfd = -1;
	  return;
	}
      else if (rcnt == 0)
	{
	  // got eof
	  close (bp->bproc_outfd);
	  bp->bproc_outfd = -1;
	  return;
	};
      char *pbuf = bp->bproc_outbuf;
      unsigned psiz = bp->bproc_outsiz;
      unsigned poff = bp->bproc_outoff;
      if (MOM_UNLIKELY (poff + rcnt >= MOM_MAX_STRING_LENGTH))
	{
	  MOM_WARNPRINTF
	    ("output from batch process %d exceeds string capacity",
	     bp->bproc_pid);
	  rcnt = MOM_MAX_STRING_LENGTH - 1 - (rcnt + poff);
	  close (bp->bproc_outfd);
	  bp->bproc_outfd = -1;
	  if (bp->bproc_pid > 0)
	    kill (SIGTERM, bp->bproc_pid);
	  if (rcnt <= 0)
	    return;
	  // the offending child process would probably get SIGPIPE fairly
	  // quickly on its next output!
	};
      if (MOM_UNLIKELY (poff + rcnt + 1 >= psiz))
	{
	  unsigned newpsiz = ((poff + rcnt + 300 + poff / 4) | 0xff) + 1;
	  char *newpbuf =
	    MOM_GC_SCALAR_ALLOC ("grown output buffer", newpsiz);
	  char *oldpbuf = pbuf;
	  unsigned oldpsiz = psiz;
	  memcpy (newpbuf, oldpbuf, poff);
	  pbuf = bp->bproc_outbuf = newpbuf;
	  psiz = bp->bproc_outsiz = newpsiz;
	  MOM_GC_FREE (oldpbuf, oldpsiz);
	};
      memcpy (pbuf + poff, rbuf, rcnt);
      pbuf[poff + rcnt] = (char) 0;
      poff += rcnt;
      bp->bproc_outoff = poff;
    };				/* end forever loop */
}				/* end of read_output_from_batchproc_mom */


/// called from mom_run_workers in work.c; we assume that stdin is
/// opened, that is that pollable file descriptors are >0
void
mom_event_loop (void)
{
#define BUSY_MILLISEC_DELAY_MOM 25	/* milliseconds, i.e. 0.025 sec */
#define IDLE_MILLISEC_DELAY_MOM 2500	/* milliseconds, i.e. 2.5 sec */
  bool idle = false;
  int evpipe[2] = { -1, -1 };
  MOM_DEBUGPRINTF (run, "start mom_event_loop");
  if (MOM_UNLIKELY (pipe2 (evpipe, O_CLOEXEC)
		    || evpipe[0] <= 0 || evpipe[1] <= 0))
    MOM_FATAPRINTF ("failed to create pipe for event loop");
  curlmulti_mom = curl_multi_init ();
  if (MOM_UNLIKELY (curlmulti_mom == NULL))
    MOM_FATAPRINTF ("failed to initialize CURL multi-handle");
  readfdeventloop_mom = evpipe[0];
  writefdeventloop_mom = evpipe[1];
  sigset_t mysigmasks;
  sigemptyset (&mysigmasks);
  sigaddset (&mysigmasks, SIGINT);
  sigaddset (&mysigmasks, SIGQUIT);
  sigaddset (&mysigmasks, SIGTERM);
  sigaddset (&mysigmasks, SIGCHLD);
  if (sigprocmask (SIG_BLOCK, &mysigmasks, NULL) < 0)
    MOM_FATAPRINTF ("event_loop failed to block signals (%m)");
  signalfd_mom = signalfd (-1, &mysigmasks, SFD_NONBLOCK | SFD_CLOEXEC);
  if (signalfd_mom < 0)
    MOM_FATAPRINTF ("event_loop failed to signalfd (%m)");
  timerfd_mom = timerfd_create (CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd_mom < 0)
    MOM_FATAPRINTF ("event_loop failed to timerfd_create (%m)");
  if (mom_socket_path && mom_socket_path[0])
    mastersocketfd_mom = open_bind_socket_mom ();
  long loopcount = 0;
#define LOOPDBG_FREQUENCY_MOM 32
  while (!mom_should_stop ())
    {
      int nbpoll = 0;
      fd_set readfdset;
      fd_set writefdset;
      fd_set exceptfdset;
      FD_ZERO (&readfdset);
      FD_ZERO (&writefdset);
      FD_ZERO (&exceptfdset);
      struct timeval seltv = { 0, 0 };
      int maxfd = 0;
      int curlmaxfd = 0;
      struct polldata_fdmom_st polldata[MAX_POLL_MOM];
      memset (polldata, 0, sizeof (polldata));
      if (MOM_UNLIKELY (loopcount % LOOPDBG_FREQUENCY_MOM == 0))
	{
	  MOM_DEBUGPRINTF (run, "start of eventloop #%ld", loopcount);
	};
      {
	pthread_mutex_lock (&eventloop_mtx_mom);
	{
	  CURLMcode err =
	    curl_multi_fdset (curlmulti_mom, &readfdset, &writefdset,
			      &exceptfdset, &curlmaxfd);
	  if (err)
	    MOM_FATAPRINTF ("curl_multi_fdset failed %s",
			    curl_multi_strerror (err));
	}
	if (curlmaxfd > 0)
	  idle = false;
	if (curlmaxfd > maxfd)
	  maxfd = curlmaxfd;
#define DO_READ_POLL_MOM(Fd,Handler,DataIndex)	do {		\
	  int rdfd = (Fd);					\
	  if (rdfd<=0 || rdfd>=FD_SETSIZE)			\
	    MOM_FATAPRINTF("bad read filedescriptor %d",	\
			   rdfd);				\
	  if (nbpoll >= MAX_POLL_MOM)				\
	    MOM_FATAPRINTF("too many (%d) read polls",		\
			   nbpoll);				\
	  if (rdfd>maxfd)					\
	    maxfd = rdfd;					\
	  FD_SET(rdfd, &readfdset);            			\
	  polldata[nbpoll].polld_hdlr = (Handler);		\
	  polldata[nbpoll].polld_dataindex = (DataIndex);	\
	  polldata[nbpoll].polld_event |= POLLIN;		\
	  nbpoll++;						\
	} while(0)
	/* We don't DO_READ_POLL_MOM for the special file descriptors
	   readfdeventloop_mom, signalfd_mom, timerfd_mom since these 
	   are processed specifically. */
	if (readfdeventloop_mom > 0)
	  FD_SET (readfdeventloop_mom, &readfdset);
	if (signalfd_mom > 0)
	  FD_SET (signalfd_mom, &readfdset);
	if (timerfd_mom > 0)
	  FD_SET (timerfd_mom, &readfdset);
	for (int pix = 1; pix <= MOM_MAX_WORKERS; pix++)
	  if (runbatchproc_mom[pix].bproc_pid > 0
	      && runbatchproc_mom[pix].bproc_outfd > 0
	      && runbatchproc_mom[pix].bproc_outbuf != NULL)
	    DO_READ_POLL_MOM (runbatchproc_mom[pix].bproc_outfd,
			      process_evcb_mom, pix);
	if (mastersocketfd_mom > 0)
	  DO_READ_POLL_MOM (mastersocketfd_mom, mastersocket_evcb_mom, 0);
	for (int six = 0; six < MOM_MAX_SOCKETS; six++)
	  if (activesockets_mom[six].msock_fd > 0
	      && activesockets_mom[six].msock_itm != NULL)
	    DO_READ_POLL_MOM (activesockets_mom[six].msock_fd,
			      activesocket_evcb_mom, six);
	int millisec =
	  idle ? IDLE_MILLISEC_DELAY_MOM : BUSY_MILLISEC_DELAY_MOM;
	long cutimeout = 0;
	{
	  CURLMcode cuerr = curl_multi_timeout (curlmulti_mom, &cutimeout);
	  if (cuerr)
	    MOM_FATAPRINTF ("curl_multi_timeout failed %s",
			    curl_multi_strerror (cuerr));
	}
	if (cutimeout > 0 && millisec > cutimeout)
	  millisec = cutimeout;
	if (millisec < 0)
	  millisec = 1;
	seltv.tv_sec = millisec / 1000;
	seltv.tv_usec = millisec / 1000;
#undef DO_READ_POLL_MOM
	pthread_mutex_unlock (&eventloop_mtx_mom);
      };
      errno = 0;
      int selres =
	select (maxfd + 1, &readfdset, &writefdset, &exceptfdset, &seltv);
      if (selres < 0 && errno != EINTR)
	{
	  MOM_WARNPRINTF ("event_loop select failed : %m");
	  eventloopupdate_mom ();
	  continue;
	};
      if (selres == 0)
	idle = true;
      if (readfdeventloop_mom > 0
	  && FD_ISSET (readfdeventloop_mom, &readfdset))
	eventloopupdate_mom ();
      if (signalfd_mom > 0 && FD_ISSET (signalfd_mom, &readfdset))
	signalprocess_mom ();
      if (timerfd_mom > 0 && FD_ISSET (timerfd_mom, &readfdset))
	ringtimer_mom ();
      if (curlmaxfd > 0)
	{
	  int nbcuhd = 0;
	  CURLMcode cuerr = curl_multi_perform (curlmulti_mom, &nbcuhd);
	  if (cuerr)
	    MOM_FATAPRINTF ("curl_multi_perform failed %s",
			    curl_multi_strerror (cuerr));
	};
      if (nbpoll > 0)
	{
	  // we handle the polldata starting at some random index.
	  int startix =
	    (nbpoll >
	     1) ? ((mom_random_nonzero_32_here () & 0x3ffffff) % nbpoll) : 0;
	  for (int pix = startix; pix < nbpoll; pix++)
	    {
	      handle_polldata_mom (polldata + pix, &readfdset, &writefdset,
				   &exceptfdset);
	    }
	  for (int pix = 0; pix < startix; pix++)
	    {
	      handle_polldata_mom (polldata + pix, &readfdset, &writefdset,
				   &exceptfdset);
	    };
	};
      if (MOM_UNLIKELY (loopcount % LOOPDBG_FREQUENCY_MOM == 0))
	{
	  MOM_DEBUGPRINTF (run, "end of eventloop #%ld", loopcount);
	};
      loopcount++;
    };				/* end while !mom_should_stop */
  if (finaleventloopdump_mom && finaleventloopdump_mom[0])
    {
      MOM_DEBUGPRINTF (run,
		       "ending mom_event_loop done %ld loops before dumping to %s",
		       loopcount, finaleventloopdump_mom);
      mom_dump_state (finaleventloopdump_mom);
      MOM_INFORMPRINTF ("event loop dumped state to %s after %ld loops",
			finaleventloopdump_mom, loopcount);
      MOM_DEBUGPRINTF (run,
		       "ending mom_event_loop after %ld loops before dumping to %s",
		       loopcount, finaleventloopdump_mom);
    }

  // perhaps we should call curl_multi_cleanup(curlmulti_mom); etc.
  MOM_DEBUGPRINTF (run, "end mom_event_loop, done %ld loops", loopcount);
}				/* end of mom_event_loop */


static void
start1batchprocess_mom (void)
{
  momvalue_t vinode = mom_queuevalue_pop_front (&pendingbatchprocque_mom);
  const momnode_t *inod = mom_value_to_node (vinode);
  assert (inod && mom_node_conn (inod) == MOM_PREDEFINED_NAMED (in)
	  && mom_node_arity (inod) == 2);
  momvalue_t vclos = mom_node_nth (inod, 0);
  momvalue_t vprocnode = mom_node_nth (inod, 1);
  struct runningbatchprocess_mom_st *bp = NULL;
  for (unsigned ix = 1; ix <= MOM_MAX_WORKERS; ix++)
    if (runbatchproc_mom[ix].bproc_pid == 0
	&& runbatchproc_mom[ix].bproc_outfd == 0)
      {
	bp = runbatchproc_mom + ix;
	break;
      };
  if (bp == NULL)
    MOM_FATAPRINTF
      ("corrupted runbatchproc array, filled but should have %d processes",
       nb_processes_mom);
  const momnode_t *procnod = mom_value_to_node (vprocnode);
  unsigned proclen = mom_node_arity (procnod);
  assert (proclen > 0);
  int pipefds[2] = { -1, -1 };
  if (MOM_UNLIKELY (pipe (pipefds)))
    MOM_FATAPRINTF
      ("failed to pipe when starting batch process for closure %s & process-node %s (%m)",
       mom_output_gcstring (vclos), mom_output_gcstring (vprocnode));
  assert (pipefds[0] > 2 && pipefds[1] > 2);
  fflush (NULL);
  pid_t pid = fork ();
  if (MOM_UNLIKELY (pid < 0))
    MOM_FATAPRINTF
      ("failed to fork when starting batch process for closure %s & process-node %s (%m)",
       mom_output_gcstring (vclos), mom_output_gcstring (vprocnode));
  if (pid == 0)
    {
      // child process
      close (STDIN_FILENO);
      int nullfd = open ("/dev/null", O_RDONLY);
      assert (nullfd == STDIN_FILENO);
      dup2 (pipefds[1], STDOUT_FILENO);
      dup2 (pipefds[1], STDERR_FILENO);
      const char **argv =
	MOM_GC_ALLOC ("argv batchprocess", (proclen + 2) * sizeof (char *));
      for (unsigned ix = 0; ix < proclen; ix++)
	argv[ix] =		//
	  MOM_GC_STRDUP ("arg string batchprocess",	//
			 mom_string_cstr (mom_value_to_string
					  (mom_node_nth (procnod, ix))));
      dup2 (pipefds[1], STDOUT_FILENO);
      dup2 (pipefds[1], STDERR_FILENO);
      for (int fd = 3; fd < 48; fd++)
	close (fd);
      execvp (argv[0], (char *const *) argv);
      {
	char errbuf[128];
	snprintf (errbuf, sizeof (errbuf), "execvp %s", argv[0]);
	perror (errbuf);
	fflush (NULL);
	_exit (127);
      };
      return;
    }
  // parent process
  unsigned busiz = 4096;
  char *buf = MOM_GC_SCALAR_ALLOC ("process output buffer", busiz);
  memset (bp, 0, sizeof (*bp));
  close (pipefds[1]);
  fcntl (pipefds[0], F_SETFL, O_NONBLOCK);
  bp->bproc_pid = pid;
  bp->bproc_outfd = pipefds[0];
  bp->bproc_hclosv = vclos;
  bp->bproc_outbuf = buf;
  bp->bproc_outsiz = busiz;
  bp->bproc_outoff = 0;
  nb_processes_mom++;
  MOM_INFORMPRINTF ("started batch process %s as pid %d",
		    mom_output_gcstring (vprocnode), (int) pid);
}				/* end of start1batchprocess_mom */



static void
eventloopupdate_mom (void)
{
  char inbuf[2 * MOM_MAX_WORKERS];
  // consume the bytes on the eventloop fd, but we don't use them
  (void) read (readfdeventloop_mom, inbuf, sizeof (inbuf));
  {
    pthread_mutex_lock (&eventloop_mtx_mom);
    while (mom_queuevalue_size (&pendingbatchprocque_mom) > 0
	   && nb_processes_mom < mom_nb_workers)
      start1batchprocess_mom ();
    pthread_mutex_unlock (&eventloop_mtx_mom);
  }
  MOM_FATAPRINTF ("unimplemented eventloopupdate");
#warning unimplemented eventloopupdate_mom
}


void
waitprocess_mom (void)
{
  for (;;)
    {
      int wsta = 0;
      pid_t wpid = waitpid (0, &wsta, WNOHANG);
      if (wpid <= 0)
	return;
      momvalue_t vclos = MOM_NONEV;
      momvalue_t vout = MOM_NONEV;
      {
	struct runningbatchprocess_mom_st bproc;
	memset (&bproc, 0, sizeof (bproc));
	pthread_mutex_lock (&eventloop_mtx_mom);
	for (unsigned ix = 1; ix <= MOM_MAX_WORKERS; ix++)
	  if (runbatchproc_mom[ix].bproc_pid == wpid)
	    {
	      bproc = runbatchproc_mom[ix];
	      memset (&runbatchproc_mom[ix], 0, sizeof (bproc));
	      nb_processes_mom--;
	      break;
	    }
	// some residual bytes might stay in the pipe. We use a
	// rather small local buffer to read them.
	char rdbuf[256];
	read_output_from_batchproc_mom (&bproc, rdbuf, sizeof (rdbuf));
	pthread_mutex_unlock (&eventloop_mtx_mom);
	vclos = bproc.bproc_hclosv;
	assert (bproc.bproc_outbuf);
	assert (bproc.bproc_outoff < bproc.bproc_outsiz);
	bproc.bproc_outbuf[bproc.bproc_outoff] = '\0';
	vout = mom_stringv (mom_make_string_cstr (bproc.bproc_outbuf));
	if (bproc.bproc_outfd > 0)
	  close (bproc.bproc_outfd);
	memset (&bproc, 0, sizeof (bproc));
	if (!mom_applval_1val1int_to_void (vclos, vout, wsta))
	  MOM_WARNPRINTF ("failed to apply closure %s after process %d ended",
			  mom_output_gcstring (vclos), (int) wpid);
      }
    }
}				/* end waitprocess_mom */


static void
signalprocess_mom (void)
{
  struct signalfd_siginfo sifd;
  memset (&sifd, 0, sizeof (sifd));
  int rd = read (signalfd_mom, &sifd, sizeof (sifd));
  if (rd <= 0)
    return;
  assert (rd == sizeof (sifd));
  MOM_DEBUGPRINTF (run, "signalprocess sifd: signo=%d code=%d",
		   (int) sifd.ssi_signo, (int) sifd.ssi_code);
  if (sifd.ssi_signo == SIGCHLD)
    {
    }
  /* should handle SIGCHLD by waiting the process */
  MOM_FATAPRINTF ("unimplemented signalprocess");
#warning unimplemented signalprocess_mom
}

static void
ringtimer_mom (void)
{
  MOM_FATAPRINTF ("unimplemented ringtimer_mom");
#warning unimplemented ringtimer_mom
}

static void
process_evcb_mom (int fd, short revent, intptr_t dataindex)
{
  // we use a static input buffer, since we are only called from the
  // event loop and its thread
  static char inbuf[4096];
  assert (dataindex > 0 && dataindex <= MOM_MAX_WORKERS);
  assert (revent & POLLIN);
  assert (fd > 0);
  memset (inbuf, 0, sizeof (inbuf));
  pthread_mutex_lock (&eventloop_mtx_mom);
  assert (runbatchproc_mom[dataindex].bproc_outfd == fd);
  read_output_from_batchproc_mom (&runbatchproc_mom[dataindex], inbuf,
				  sizeof (inbuf));
  pthread_mutex_unlock (&eventloop_mtx_mom);
}

static void
mastersocket_evcb_mom (int fd, short revent, intptr_t dataindex)
{
  MOM_FATAPRINTF
    ("unimplemented mastersocket_evcb fd=%d revent=%d dataindex=%ld", fd,
     (int) revent, (long) dataindex);
#warning unimplemented mastersocket_evcb_mom
}

static void
activesocket_evcb_mom (int fd, short revent, intptr_t dataindex)
{
  MOM_FATAPRINTF
    ("unimplemented activesocket_evcb fd=%d revent=%d dataindex=%ld", fd,
     (int) revent, (long) dataindex);
#warning unimplemented activesocket_evcb_mom
}


void
mom_start_batch_process (momvalue_t vclos, momvalue_t vprocnode)
{
  bool ok = true;
  MOM_DEBUGPRINTF (run, "start_batch_process vclos=%s vprocnode=%s",
		   mom_output_gcstring (vclos),
		   mom_output_gcstring (vprocnode));
  if (vclos.typnum != momty_node)
    ok = false;
  momnode_t *procnod = mom_value_to_node (vprocnode);
  unsigned procarity = 0;
  if (!procnod
      || mom_node_conn (procnod) != MOM_PREDEFINED_NAMED (batch_process)
      || (procarity = mom_node_arity (procnod)) == 0)
    ok = false;
  else
    {
      for (unsigned ix = 0; ix < procarity && ok; ix++)
	if (!mom_value_to_string (mom_node_nth (procnod, ix)))
	  ok = false;
    };
  if (!ok)
    {
      MOM_WARNPRINTF
	("invalid arguments to start_batch_process vclos=%s vprocnode=%s",
	 mom_output_gcstring (vclos), mom_output_gcstring (vprocnode));
      mom_applval_1val1int_to_void (vclos, vprocnode, -1);
      return;
    };
  momvalue_t vinode =
    mom_nodev_new (MOM_PREDEFINED_NAMED (in), 2, vclos, vprocnode);
  {
    pthread_mutex_lock (&eventloop_mtx_mom);
    mom_queuevalue_push_back (&pendingbatchprocque_mom, vinode);
    pthread_mutex_unlock (&eventloop_mtx_mom);
    mom_wake_event_loop ();
  }
}				/* end of mom_start_batch_process */
