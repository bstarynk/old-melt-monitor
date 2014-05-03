// file run.c

/**   Copyright (C)  2014 Free Software Foundation, Inc.
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

static pthread_t event_loop_thread;

static int event_loop_pipe[2] = { -1, -1 };

static int my_signals_fd = -1;
#define event_loop_read_pipe event_loop_pipe[0]
#define event_loop_write_pipe event_loop_pipe[1]

// communication between other threads & event loop thread thru single byte sent on pipe
#define EVLOOP_STOP '.'		/* stop the event loop */
#define EVLOOP_JOB 'J'		/* something changed about jobs */


#define MOM_POLL_TIMEOUT 500	/* milliseconds for mom poll timeout */

// the job_mtx is both for processes and for CURL
static pthread_mutex_t job_mtx = PTHREAD_MUTEX_INITIALIZER;
static momit_process_t *running_jobs[MOM_MAX_WORKERS + 1];
static struct mom_itqueue_st *jobq_first;
static struct mom_itqueue_st *jobq_last;
static CURLM *multicurl_job;

static momit_routine_t **embryonic_routine_arr;
static const char **embryonic_routine_name;
static unsigned embryonic_routine_size;
static pthread_mutex_t embryonic_mtx = PTHREAD_MUTEX_INITIALIZER;

#define WORK_MAGIC 0x5c59b171	/* work magic 1549382001 */
struct momworkdata_st
{
  unsigned work_magic;		/* always WORK_MAGIC */
  unsigned work_index;
  pthread_t work_thread;
};

static __thread struct momworkdata_st *cur_worker;

extern momit_queue_t *mom_item__agenda;

static struct momworkdata_st workers[MOM_MAX_WORKERS + 1];
static bool working_flag;

void
mom_agenda_add_tasklet_front (momval_t tsk)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  assert (mom_item__agenda
	  && ((mom_anyitem_t *) mom_item__agenda)->typnum == momty_queueitem);
  mom_item_queue_push_front ((momval_t) mom_item__agenda, tsk);
  mom_dbg_value (run, "add tasklet front tsk=", tsk);
  pthread_cond_broadcast (&mom_run_changed_cond);
}

void
mom_agenda_add_tasklet_back (momval_t tsk)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  assert (mom_item__agenda
	  && ((mom_anyitem_t *) mom_item__agenda)->typnum == momty_queueitem);
  mom_item_queue_push_back ((momval_t) mom_item__agenda, tsk);
  mom_dbg_value (run, "add tasklet back tsk=", tsk);
  pthread_cond_broadcast (&mom_run_changed_cond);
}

#define WORK_DELAY 5.8		/* seconds */
static void *
work_loop (struct GC_stack_base *sb, void *data)
{
  struct momworkdata_st *wd = data;
  MOMGC_REGISTER_MY_THREAD (sb);
  assert (wd != NULL);
  mom_anyitem_t *curtsk = NULL;
  MONIMELT_DEBUG (run,
		  "work_loop tid %d start index %d wd@%p  cur_worker@%p",
		  (int) mom_gettid (), wd->work_index, wd, cur_worker);
  bool working = false;
  long loopcnt = 0;
  do
    {
      curtsk = NULL;
      pthread_mutex_lock (&mom_run_mtx);
      working = working_flag;
      pthread_mutex_unlock (&mom_run_mtx);
      loopcnt++;
      MONIMELT_DEBUG (run, "work_loop index %d, loopcnt=%ld, working=%d",
		      wd->work_index, loopcnt, (int) working);
      if (working)
	curtsk = mom_item_queue_pop_front ((momval_t) mom_item__agenda);
      mom_dbg_item (run, "work_loop curtsk=", curtsk);
      if (curtsk)
	{
	  if (MONIMELT_UNLIKELY (curtsk->typnum != momty_taskletitem))
	    MONIMELT_FATAL ("invalid current task");
	  mom_tasklet_step ((momit_tasklet_t *) curtsk);
	}
      else
	{
	  double curwtim = monimelt_clock_time (CLOCK_REALTIME);
	  pthread_mutex_lock (&mom_run_mtx);
	  struct timespec endts =
	    monimelt_timespec (curwtim + WORK_DELAY +
			       (wd->work_index * 0.03 + 0.01));
	  working = working_flag;
	  if (working)
	    pthread_cond_timedwait (&mom_run_changed_cond, &mom_run_mtx,
				    &endts);
	  pthread_mutex_unlock (&mom_run_mtx);
	}
    }
  while (working);
  MONIMELT_DEBUG (run, "work_loop index %d ending", wd->work_index);
  pthread_mutex_lock (&mom_run_mtx);
  cur_worker->work_index = 0;
  pthread_mutex_unlock (&mom_run_mtx);
  pthread_cond_broadcast (&mom_run_changed_cond);
  MOMGC_UNREGISTER_MY_THREAD ();
  return NULL;
}

static void *
work_cb (void *ad)
{
  struct momworkdata_st *wd = ad;
  {
    char thnambuf[24];
    memset (thnambuf, 0, sizeof (thnambuf));
    snprintf (thnambuf, sizeof (thnambuf), "mom-work%02d", wd->work_index);
    pthread_setname_np (pthread_self (), thnambuf);
  }
  assert (wd && wd->work_magic == WORK_MAGIC);
  if (!mom_item__agenda
      || ((mom_anyitem_t *) mom_item__agenda)->typnum != momty_queueitem)
    MONIMELT_FATAL ("bad agenda");
  cur_worker = wd;
  MOMGC_CALL_WITH_STACK_BASE (work_loop, wd);
  cur_worker = NULL;
  return NULL;
}


void
mom_run (void)
{
  pthread_mutex_lock (&mom_run_mtx);
  if (mom_nb_workers < MOM_MIN_WORKERS)
    mom_nb_workers = MOM_MIN_WORKERS;
  else if (mom_nb_workers > MOM_MAX_WORKERS)
    mom_nb_workers = MOM_MAX_WORKERS;
  assert (!working_flag);
  memset (workers, 0, sizeof (workers));
  for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
    {
      workers[ix].work_magic = WORK_MAGIC;
      workers[ix].work_index = ix;
      pthread_create (&workers[ix].work_thread, NULL, work_cb, workers + ix);
    }
  working_flag = true;
  pthread_mutex_unlock (&mom_run_mtx);
}

void
mom_wait_for_stop (void)
{
  int nbworkers = 0;
  bool isworking = false;
  MONIMELT_DEBUG (run, "mom_wait_for_stop start");
  do
    {
      nbworkers = 0;
      isworking = false;
      pthread_mutex_lock (&mom_run_mtx);
      isworking = working_flag;
      for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
	if (workers[ix].work_magic == WORK_MAGIC
	    && workers[ix].work_index == ix)
	  nbworkers++;
      if (nbworkers > 0 && isworking)
	pthread_cond_wait (&mom_run_changed_cond, &mom_run_mtx);
      pthread_mutex_unlock (&mom_run_mtx);
      sched_yield ();
    }
  while (nbworkers > 0 || isworking);
  pthread_mutex_lock (&mom_run_mtx);
  for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
    {
      void *ret = NULL;
      if (workers[ix].work_magic == WORK_MAGIC
	  && workers[ix].work_index == ix)
	pthread_join (workers[ix].work_thread, &ret);
      assert (ret == NULL);
      workers[ix].work_thread = (pthread_t) 0;
    }
  pthread_mutex_unlock (&mom_run_mtx);
  usleep (1000);
  MONIMELT_DEBUG (run, "mom_wait_for_stop end");
}

void
mom_request_stop (void)
{
  int nbworkers = 0;
  long stopcount = 0;
  MONIMELT_DEBUG (run, "mom_request_stop start");
  do
    {
      pthread_mutex_lock (&mom_run_mtx);
      stopcount++;
      MONIMELT_DEBUG (run, "mom_request_stop stopcount=%ld", stopcount);
      working_flag = false;
      nbworkers = 0;
      for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
	if (workers[ix].work_magic == WORK_MAGIC
	    && workers[ix].work_index == ix)
	  nbworkers++;
      MONIMELT_DEBUG (run, "mom_request_stop nbworkers=%d", nbworkers);
      int errwait = 0;
      if (nbworkers > 0)
	errwait = pthread_cond_wait (&mom_run_changed_cond, &mom_run_mtx);
      pthread_mutex_unlock (&mom_run_mtx);
      MONIMELT_DEBUG (run, "nbworkers=%d, errwait#%d (%s)", nbworkers,
		      errwait, strerror (errwait));
      usleep (500);
    }
  while (nbworkers > 0);
  MONIMELT_DEBUG (run, "mom_request_stop end");
}


static void
start_some_pending_jobs (void)
{
  int nbrunning = 0;
  int nbstarting = 0;
  momit_process_t *proctab[MOM_MAX_WORKERS] = { };
  memset (proctab, 0, sizeof (proctab));
  pthread_mutex_lock (&job_mtx);
  for (unsigned jix = 1; jix <= MOM_MAX_WORKERS; jix++)
    {
      momit_process_t *curjob = running_jobs[jix];
      if (!curjob)
	continue;
      if (curjob->iproc_jobnum == jix && curjob->iproc_pid > 0
	  && !kill (curjob->iproc_pid, 0))
	nbrunning++;
    }
  while (nbrunning < mom_nb_workers && nbstarting < mom_nb_workers
	 && jobq_first != NULL)
    {
      proctab[nbstarting++] = (momit_process_t *) jobq_first->iq_item;
      jobq_first = jobq_first->iq_next;
      if (jobq_first == NULL)
	jobq_last = NULL;
    }
  for (unsigned rix = 0; rix < nbstarting; rix++)
    {
      momit_process_t *curproc = proctab[rix];
      int pipetab[2] = { -1, -1 };
      int jobnum = -1;
      for (unsigned j = 1; j <= MOM_MAX_WORKERS && jobnum < 0; j++)
	if (!running_jobs[j])
	  jobnum = j;
      assert (curproc && curproc->iproc_item.typnum == momty_processitem);
      assert (curproc->iproc_pid <= 0 && curproc->iproc_outfd < 0);
      assert (jobnum > 0);
      pthread_mutex_lock (&curproc->iproc_item.i_mtx);
      const char *progname =
	mom_string_cstr ((momval_t) curproc->iproc_progname);
      const char **progargv =
	GC_MALLOC ((curproc->iproc_argcount + 2) * sizeof (char *));
      if (MONIMELT_UNLIKELY (!progargv))
	MONIMELT_FATAL ("cannot allocate array for %d program arguments",
			curproc->iproc_argcount);
      memset (progargv, 0, (curproc->iproc_argcount + 2) * sizeof (char *));
      progargv[0] = progname;
      unsigned argcnt = 1;
      for (unsigned aix = 0; aix < curproc->iproc_argcount; aix++)
	{
	  const char *argstr =
	    mom_string_cstr ((momval_t) (curproc->iproc_argv[aix]));
	  if (argstr)
	    progargv[argcnt++] = argstr;
	}
      progargv[argcnt] = NULL;
      assert (progname != NULL);
      if (pipe (pipetab))
	MONIMELT_FATAL ("failed to create pipe for process %s", progname);
      fflush (NULL);
      pid_t newpid = fork ();
      if (newpid == 0)
	{
	  /* child process */
	  for (int fd = 3; fd < 128; fd++)
	    if (fd != pipetab[1])
	      close (fd);
	  int nullfd = open ("/dev/null", O_RDONLY);
	  if (nullfd < 0)
	    {
	      perror ("open /dev/null child process");
	      _exit (127);
	    };
	  close (STDIN_FILENO);
	  dup2 (nullfd, STDIN_FILENO);
	  dup2 (pipetab[1], STDOUT_FILENO);
	  dup2 (pipetab[1], STDERR_FILENO);
	  nice (1);
	  execvp ((const char *) progname, (char *const *) progargv);
	  fprintf (stderr, "execution of %s failed : %s\n", progname,
		   strerror (errno));
	  fflush (NULL);
	  _exit (127);
	}
      else if (newpid < 0)
	MONIMELT_FATAL ("fork failed for %s", progname);
      // parent process:
      curproc->iproc_outfd = pipetab[0];
      curproc->iproc_pid = newpid;
      curproc->iproc_jobnum = jobnum;
      pthread_mutex_unlock (&curproc->iproc_item.i_mtx);
      GC_FREE (progargv), progargv = NULL;
    }
  pthread_mutex_unlock (&job_mtx);
}


void
mom_stop_event_loop (void)
{
  char buf[4] = { EVLOOP_STOP, 0, 0, 0 };
  write (event_loop_write_pipe, buf, 1);
}

typedef void mom_poll_handler_sig_t (int fd, short revent, void *data);

static void
event_loop_handler (int fd, short revent, void *data)
{
  assert (fd == event_loop_read_pipe && data);
  bool *prepeatloop = data;
  char rbuf[4];
  memset (rbuf, 0, sizeof (rbuf));
  if (read (fd, &rbuf, 1) > 0)
    {
      switch (rbuf[0])
	{
	case EVLOOP_STOP:
	  *prepeatloop = false;
	  break;
	case EVLOOP_JOB:
	  start_some_pending_jobs ();
	  break;
	default:
	  MONIMELT_FATAL ("unexpected loop command %c = %d", rbuf[0],
			  (int) rbuf[0]);
	}
    }
}

static void
signal_fd_handler (int fd, short revent, void *data)
{
  assert (fd == my_signals_fd && !data);
  struct signalfd_siginfo sinf;
  memset (&sinf, 0, sizeof (sinf));
  if (read (fd, &sinf, sizeof (sinf)) != sizeof (sinf))
    MONIMELT_FATAL ("failed to read from signalfd %d", fd);
  switch (sinf.ssi_signo)
    {
    case SIGCHLD:
      {
	int pst = 0;
	pid_t wpid = waitpid (-1, &pst, WNOHANG);
	if (wpid > 0)
	  {
	    momit_process_t *wproc = NULL;
	    const momclosure_t *clos = NULL;
	    const momstring_t *outstr = NULL;
	    pthread_mutex_lock (&job_mtx);
	    // handle the ended process
	    for (unsigned jix = 1; jix <= MOM_MAX_WORKERS && !wproc; jix++)
	      {
		momit_process_t *curproc = running_jobs[jix];
		if (!curproc)
		  continue;
		assert (curproc->iproc_item.typnum == momty_processitem);
		pthread_mutex_lock (&curproc->iproc_item.i_mtx);
		if (curproc->iproc_pid == wpid)
		  {
		    wproc = curproc;
		    clos = curproc->iproc_closure;
		    running_jobs[jix] = NULL;
		  }
		pthread_mutex_unlock (&curproc->iproc_item.i_mtx);
	      }
	    pthread_mutex_unlock (&job_mtx);
	    int exitstatus = -1;
	    int termsig = -1;
	    if (WIFEXITED (pst))
	      exitstatus = WEXITSTATUS (pst);
	    else if (WIFSIGNALED (pst))
	      termsig = WTERMSIG (pst);
	    if (wproc && (exitstatus >= 0 || termsig >= 0))
	      {
		extern momit_box_t *mom_item__exited;
		extern momit_box_t *mom_item__terminated;
		pthread_mutex_lock (&wproc->iproc_item.i_mtx);
		outstr =
		  mom_make_string_len (wproc->iproc_outbuf,
				       wproc->iproc_outpos);
		close (wproc->iproc_outfd);
		wproc->iproc_outfd = -1;
		GC_FREE (wproc->iproc_outbuf);
		wproc->iproc_outbuf = NULL;
		wproc->iproc_outsize = wproc->iproc_outpos = 0;
		wproc->iproc_pid = 0;
		pthread_mutex_unlock (&wproc->iproc_item.i_mtx);
		// should run the closure
		if (exitstatus >= 0)
		  mom_run_closure ((momval_t) clos,
				   MOMPFR_THREE_VALUES, wproc, outstr,
				   mom_item__exited, MOMPFR_INT, exitstatus,
				   MOMPFR_END);
		else if (termsig >= 0)
		  mom_run_closure ((momval_t) clos,
				   MOMPFR_THREE_VALUES, wproc, outstr,
				   mom_item__terminated, MOMPFR_INT, termsig,
				   MOMPFR_END);
	      }
	  }
      }
      break;
    default:
      MONIMELT_WARNING ("unexpected signal#%d : %s", sinf.ssi_signo,
			sys_siglist[sinf.ssi_signo]);
      break;
    }
}

#define MONIMELT_MAX_OUTPUT_LEN (1024*1024)	/* one megabyte */
static void
process_readout_handler (int fd, short revent, void *data)
{
#define PROCOUT_BUFSIZE 4096
  char rdbuf[PROCOUT_BUFSIZE];
  memset (rdbuf, 0, sizeof (rdbuf));
  int nbr = read (fd, rdbuf, sizeof (rdbuf));
  momit_process_t *procitm = data;
  assert (procitm && procitm->iproc_item.typnum == momty_processitem
	  && fd == procitm->iproc_outfd);
  if (nbr < 0)
    return;
  pthread_mutex_lock (&procitm->iproc_item.i_mtx);
  if (MONIMELT_UNLIKELY
      (nbr + procitm->iproc_outpos + 1 >= procitm->iproc_outsize))
    {
      unsigned newsiz =
	((5 * (nbr + procitm->iproc_outpos) / 4 + 10) | 0x1f) + 1;
      char *newbuf = GC_MALLOC_ATOMIC (newsiz);
      char *oldbuf = procitm->iproc_outbuf;
      if (MONIMELT_UNLIKELY (!newbuf))
	MONIMELT_FATAL ("failed to grow output buffer to %d", (int) newsiz);
      memset (newbuf, 0, newsiz);
      memcpy (newbuf, oldbuf, procitm->iproc_outpos);
      GC_FREE (oldbuf);
      procitm->iproc_outbuf = newbuf;
      procitm->iproc_outsize = newsiz;
    };
  memcpy (procitm->iproc_outbuf + procitm->iproc_outpos, rdbuf, nbr);
  procitm->iproc_outbuf[procitm->iproc_outpos + nbr] = (char) 0;
  procitm->iproc_outpos += nbr;
  if (procitm->iproc_outpos > MONIMELT_MAX_OUTPUT_LEN)
    {
      close (procitm->iproc_outfd);	// the child process could later get a SIGPIPE if it writes more
      procitm->iproc_outfd = -1;
    }
  pthread_mutex_unlock (&procitm->iproc_item.i_mtx);
}

static void
curl_handler (int fd, short revent, void *data)
{
  int *pnb = data;
  if (*pnb > 0)
    curl_multi_perform (&multicurl_job, pnb);
}

static void *
event_loop (struct GC_stack_base *sb, void *data)
{
  bool repeat_loop = false;
  extern momit_box_t *mom_item__heart_beat;
#define MOM_POLL_MAX 100
  struct pollfd polltab[MOM_POLL_MAX];
  mom_poll_handler_sig_t *handlertab[MOM_POLL_MAX];
  void **datatab[MOM_POLL_MAX];
  int curlnbhandles = 0;
  memset (polltab, 0, sizeof (polltab));
  memset (handlertab, 0, sizeof (handlertab));
  memset (datatab, 0, sizeof (datatab));
  MOMGC_REGISTER_MY_THREAD (sb);
  assert (data == NULL);
  assert (mom_item__heart_beat != NULL);
  // set up CURL multi handle
  assert (multicurl_job == NULL);
  multicurl_job = curl_multi_init ();
  if (MONIMELT_UNLIKELY (!multicurl_job))
    MONIMELT_FATAL ("failed to initial multi CURL handle");
  // set up the signalfd 
  {
    sigset_t mask;
    sigemptyset (&mask);
    sigaddset (&mask, SIGINT);
    sigaddset (&mask, SIGTERM);
    sigaddset (&mask, SIGQUIT);
    sigaddset (&mask, SIGPIPE);
    sigaddset (&mask, SIGCHLD);
    my_signals_fd = signalfd (-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (MONIMELT_UNLIKELY (my_signals_fd < 0))
      MONIMELT_FATAL ("signalfd failed");
  }
  /// our event loop
  do
    {
      repeat_loop = true;
      unsigned pollcnt = 0;
      memset (polltab, 0, sizeof (polltab));
      memset (handlertab, 0, sizeof (handlertab));
      memset (datatab, 0, sizeof (datatab));
#define ADD_POLL(Ev,Fd,Hdr,Data) do {					\
    if (MONIMELT_UNLIKELY(pollcnt>=MOM_POLL_MAX))			\
      MONIMELT_FATAL("failed to poll fd#%d", (Fd));			\
    polltab[pollcnt].fd = (Fd);  polltab[pollcnt].events = (Ev);	\
    handlertab[pollcnt] = Hdr; datatab[pollcnt]= (void*)(Data);		\
    pollcnt++; } while(0)
      ADD_POLL (POLL_IN, event_loop_read_pipe, event_loop_handler,
		&repeat_loop);
      ADD_POLL (POLL_IN, my_signals_fd, signal_fd_handler, NULL);
      // add the process output and CURL
      {
	pthread_mutex_lock (&job_mtx);
	// add the running processes
	for (unsigned jix = 1; jix <= MOM_MAX_WORKERS; jix++)
	  {
	    momit_process_t *curproc = running_jobs[jix];
	    if (!curproc)
	      continue;
	    pthread_mutex_lock (&curproc->iproc_item.i_mtx);
	    if (curproc->iproc_outfd >= 0)
	      ADD_POLL (POLL_IN, curproc->iproc_outfd,
			process_readout_handler, curproc);
	    pthread_mutex_unlock (&curproc->iproc_item.i_mtx);
	  }
	// add the CURL file descriptors
	{
	  int curlmaxfd = -1;
	  fd_set inscurl, outscurl, excscurl;
	  FD_ZERO (&inscurl);
	  FD_ZERO (&outscurl);
	  FD_ZERO (&excscurl);
	  curlnbhandles = 0;
	  curl_multi_fdset (multicurl_job, &inscurl, &outscurl, &excscurl,
			    &curlmaxfd);
	  for (int curlfd = 0; curlfd < curlmaxfd; curlfd++)
	    {
	      unsigned curlpollflags = 0;
	      if (FD_ISSET (curlfd, &inscurl))
		curlpollflags |= POLL_IN;
	      if (FD_ISSET (curlfd, &outscurl))
		curlpollflags |= POLL_OUT;
	      if (FD_ISSET (curlfd, &excscurl))
		curlpollflags |= POLL_PRI;
	      if (curlpollflags)
		{
		  ADD_POLL (curlpollflags, curlfd, curl_handler,
			    &curlnbhandles);
		  curlnbhandles++;
		}
	    }
	}
	pthread_mutex_unlock (&job_mtx);
      }
      // do the polling
      int respoll = poll (polltab, pollcnt, MOM_POLL_TIMEOUT);
      // invoke the handlers
      if (respoll > 0)
	{
	  for (int pix = 0; pix < pollcnt; pix++)
	    {
	      if (polltab[pix].revents && handlertab[pix])
		handlertab[pix] (polltab[pix].fd, polltab[pix].revents,
				 datatab[pix]);
	    }
	  sched_yield ();
	}
      else
	{			// timed-out
#warning should handle time out in event loop
	}
    }
  while (repeat_loop);
  MOMGC_UNREGISTER_MY_THREAD ();
  return NULL;
}


static void *
eventloop_cb (void *p)
{
  pthread_setname_np (pthread_self (), "mom-evloop");
  MOMGC_CALL_WITH_STACK_BASE (event_loop, p);
  return p;
}

void
mom_start_event_loop (void)
{
  static pthread_attr_t evthattr;
  pthread_attr_init (&evthattr);
  pthread_attr_setdetachstate (&evthattr, TRUE);
  if (pipe (event_loop_pipe))
    MONIMELT_FATAL ("failed to create event loop pipe");
  if (pthread_create (&event_loop_thread, &evthattr, eventloop_cb, NULL))
    MONIMELT_FATAL ("failed to create event loop thread");
}


void
mom_process_destroy (mom_anyitem_t * itm)
{
  assert (itm && itm->typnum == momty_processitem);
  momit_process_t *procitm = (momit_process_t *) itm;
  if (procitm->iproc_pid > 0)
    {
      MONIMELT_WARNING ("destroying running process %d for %s",
			(int) procitm->iproc_pid,
			mom_string_cstr ((momval_t) procitm->iproc_progname) ?
			: "??");
      kill (procitm->iproc_pid, SIGTERM);
    }
}



#define OUTPROC_INITIAL_SIZE 512
momit_process_t *
mom_make_item_process_argvals (momval_t progstr, ...)
{
  momit_process_t *procitm = NULL;
  va_list args;
  unsigned nbargstr = 0;
  if (!progstr.ptr || *progstr.ptype != momty_string
      || progstr.pstring->cstr[0] == (char) 0)
    return NULL;
  va_start (args, progstr);
  for (;;)
    {
      const momstring_t *curargv = va_arg (args, const momstring_t *);
      if (!curargv)
	break;
      if (curargv->typnum == momty_string)
	nbargstr++;
    }
  va_end (args);
  const momstring_t **argv = NULL;
  if (nbargstr > 0)
    {
      argv = GC_MALLOC (nbargstr * sizeof (momstring_t *));
      if (MONIMELT_UNLIKELY (!argv))
	MONIMELT_FATAL ("cannot allocate %d argument strings",
			(int) nbargstr);
      memset (argv, 0, nbargstr * sizeof (momstring_t *));
      unsigned cnt = 0;
      va_start (args, progstr);
      for (;;)
	{
	  const momstring_t *curargv = va_arg (args, const momstring_t *);
	  if (!curargv)
	    break;
	  if (curargv->typnum != momty_string)
	    continue;
	  assert (cnt < nbargstr);
	  argv[cnt++] = curargv;
	}
      va_end (args);
    };
  procitm = mom_allocate_item (momty_processitem, sizeof (momit_process_t),
			       MONIMELT_SPACE_NONE);
  procitm->iproc_progname = progstr.pstring;
  procitm->iproc_argv = argv;
  procitm->iproc_argcount = nbargstr;
  procitm->iproc_outfd = -1;
  procitm->iproc_pid = 0;
  return procitm;
}



momit_process_t *
mom_make_item_process_from_array (momval_t progstr, unsigned argc,
				  momval_t * argvals)
{
  momit_process_t *procitm = NULL;
  unsigned nbargstr = 0;
  if (!progstr.ptr || *progstr.ptype != momty_string
      || progstr.pstring->cstr[0] == (char) 0 || (argc > 0 && !argvals))
    return NULL;
  for (unsigned ix = 0; ix < argc; ix++)
    {
      const momstring_t *curargv = argvals[ix].pstring;
      if (!curargv)
	break;
      if (curargv->typnum != momty_string)
	continue;
      nbargstr++;
    }
  const momstring_t **argv = NULL;
  if (nbargstr > 0)
    {
      unsigned cnt = 0;
      argv = GC_MALLOC (nbargstr * sizeof (momstring_t *));
      if (MONIMELT_UNLIKELY (!argv))
	MONIMELT_FATAL ("cannot allocate %d argument strings",
			(int) nbargstr);
      memset (argv, 0, nbargstr * sizeof (momstring_t *));
      for (unsigned ix = 0; ix < argc; ix++)
	{
	  const momstring_t *curargv = argvals[ix].pstring;
	  if (!curargv)
	    break;
	  assert (cnt < nbargstr);
	  if (curargv->typnum == momty_string)
	    argv[cnt++] = curargv;
	}
    }
  procitm = mom_allocate_item (momty_processitem, sizeof (momit_process_t),
			       MONIMELT_SPACE_NONE);
  procitm->iproc_progname = progstr.pstring;
  procitm->iproc_argv = argv;
  procitm->iproc_argcount = nbargstr;
  procitm->iproc_outfd = -1;
  procitm->iproc_pid = 0;
  return procitm;
}


momit_process_t *
mom_make_item_process_from_node (momval_t progstr, momval_t nodv)
{
  momit_process_t *procitm = NULL;
  unsigned nbargstr = 0;
  const momstring_t **argv = NULL;
  if (!progstr.ptr || *progstr.ptype != momty_string
      || progstr.pstring->cstr[0] == (char) 0)
    return NULL;
  if (nodv.ptr && *nodv.ptype == momty_node)
    {
      const momnode_t *nd = nodv.pnode;
      unsigned ndlen = nd->slen;
      for (unsigned ix = 0; ix < ndlen; ix++)
	{
	  const momstring_t *curargv = nd->sontab[ix].pstring;
	  if (!curargv)
	    break;
	  if (curargv->typnum != momty_string)
	    continue;
	  nbargstr++;
	}
      if (nbargstr > 0)
	{
	  unsigned cnt = 0;
	  argv = GC_MALLOC (nbargstr * sizeof (momstring_t *));
	  if (MONIMELT_UNLIKELY (!argv))
	    MONIMELT_FATAL ("cannot allocate %d argument strings",
			    (int) nbargstr);
	  memset (argv, 0, nbargstr * sizeof (momstring_t *));
	  for (unsigned ix = 0; ix < ndlen; ix++)
	    {
	      const momstring_t *curargv = nd->sontab[ix].pstring;
	      if (!curargv)
		break;
	      assert (cnt < nbargstr);
	      if (curargv->typnum == momty_string)
		argv[cnt++] = curargv;
	    }
	}
    }
  procitm = mom_allocate_item (momty_processitem, sizeof (momit_process_t),
			       MONIMELT_SPACE_NONE);
  procitm->iproc_progname = progstr.pstring;
  procitm->iproc_argv = argv;
  procitm->iproc_argcount = nbargstr;
  procitm->iproc_outfd = -1;
  procitm->iproc_pid = 0;
  return procitm;
}

void
mom_item_process_start (momval_t procv, momval_t clov)
{
  if (!procv.ptr || *procv.ptype != momty_processitem
      || !clov.ptr || *clov.ptype != momty_closure)
    return;
  pthread_mutex_lock (&procv.panyitem->i_mtx);
  momit_process_t *procitm = procv.pprocessitem;
  const momclosure_t *clos = clov.pclosure;
  if (procitm->iproc_closure)	// already started
    goto end;
  procitm->iproc_outbuf = GC_MALLOC_ATOMIC (OUTPROC_INITIAL_SIZE);
  if (MONIMELT_UNLIKELY (!procitm->iproc_outbuf))
    MONIMELT_FATAL ("failed to allocate output buffer for process %s",
		    mom_string_cstr ((momval_t) procitm->iproc_progname));
  memset (procitm->iproc_outbuf, 0, OUTPROC_INITIAL_SIZE);
  procitm->iproc_outsize = OUTPROC_INITIAL_SIZE;
  procitm->iproc_outpos = 0;
  procitm->iproc_closure = clos;
  {
    struct mom_itqueue_st *qel = GC_MALLOC (sizeof (struct mom_itqueue_st));
    if (MONIMELT_UNLIKELY (!qel))
      MONIMELT_FATAL ("failed to allocate job item queue element");
    memset (qel, 0, sizeof (struct mom_itqueue_st));
    qel->iq_item = (mom_anyitem_t *) procitm;
    pthread_mutex_lock (&job_mtx);
    if (!jobq_first)
      {
	jobq_first = jobq_last = qel;
      }
    else
      {
	jobq_last->iq_next = qel;
	jobq_last = qel;
      }
    char buf[4] = { EVLOOP_JOB, 0, 0, 0 };
    write (event_loop_write_pipe, buf, 1);
    pthread_mutex_unlock (&job_mtx);
  }
end:
  pthread_mutex_unlock (&procv.panyitem->i_mtx);
}


void
mom_load_code_then_run (const char *modname)
{
  GModule *codmodu = NULL;
  char pathbuf[MONIMELT_PATH_LEN];
  char symname[MOM_SYMBNAME_LEN];
  int foundnamecount = 0;
  memset (pathbuf, 0, sizeof (pathbuf));
  memset (symname, 0, sizeof (symname));
  snprintf (pathbuf, sizeof (pathbuf), "./%s.%s", modname, G_MODULE_SUFFIX);
  if (access (pathbuf, R_OK))
    {
      MONIMELT_WARNING ("fail to access code module %s", pathbuf);
      return;
    }
  mom_request_stop ();
  mom_wait_for_stop ();
  codmodu = g_module_open (pathbuf, 0);
  if (!codmodu)
    MONIMELT_FATAL ("failed to load code module %s: %s", pathbuf,
		    g_module_error ());
  pthread_mutex_lock (&embryonic_mtx);
  for (unsigned eix = 0; eix < embryonic_routine_size; eix++)
    {
      const char *curname = embryonic_routine_name[eix];
      momit_routine_t *curout = embryonic_routine_arr[eix];
      if (!curname || !curout)
	continue;
      pthread_mutex_lock (&curout->irt_item.i_mtx);
      assert (curout->irt_descr == NULL);
      memset (symname, 0, sizeof (symname));
      snprintf (symname, sizeof (symname), MOM_ROUTINE_NAME_FMT, curname);
      struct momroutinedescr_st *rdescr = NULL;
      if ((g_module_symbol
	   (codmodu, symname, (gpointer *) & rdescr)
	   || g_module_symbol
	   (mom_prog_module, symname, (gpointer *) & rdescr)) && rdescr)
	{
	  if (rdescr->rout_magic != ROUTINE_MAGIC || !rdescr->rout_code
	      || strcmp (rdescr->rout_name, curname))
	    MONIMELT_FATAL ("bad routine descriptor %s", symname);
	  curout->irt_descr = rdescr;
	  foundnamecount++;
	}
      pthread_mutex_unlock (&curout->irt_item.i_mtx);
    }
  pthread_mutex_unlock (&embryonic_mtx);
  MONIMELT_INFORM ("loaded code module %s and found %d embryonic routines",
		   pathbuf, foundnamecount);
  usleep (500);
  mom_run ();
}


void
mom_mark_delayed_embryonic_routine (momit_routine_t * itrout,
				    const char *name)
{
  assert (itrout && itrout->irt_item.typnum == momty_routineitem);
  assert (name && name[0]);
  int eix = -1;
  char *dupname = GC_STRDUP (name);
  if (!dupname)
    MONIMELT_FATAL ("failed to duplicate name %s", name);
  pthread_mutex_lock (&embryonic_mtx);
  for (int j = 0; j < (int) embryonic_routine_size && eix < 0; j++)
    if (!embryonic_routine_arr[j] || embryonic_routine_arr[j] == itrout)
      eix = j;
  if (MONIMELT_UNLIKELY (eix < 0))
    {
      unsigned newsiz = ((3 * embryonic_routine_size / 2 + 10) | 0x1f) + 1;
      char **newnames = GC_MALLOC (newsiz * sizeof (char *));
      momit_routine_t **newarr =
	GC_MALLOC (newsiz * sizeof (momit_routine_t *));
      if (!newnames || !newarr)
	MONIMELT_FATAL ("failed to allocate for %d embryonic routines",
			newsiz);
      memset (newnames, 0, newsiz * sizeof (char *));
      memset (newarr, 0, newsiz * sizeof (momit_routine_t *));
      if (embryonic_routine_size > 0)
	{
	  memcpy (newnames, embryonic_routine_name,
		  embryonic_routine_size * sizeof (char *));
	  memcpy (newarr, embryonic_routine_arr,
		  embryonic_routine_size * sizeof (momit_routine_t *));
	};
      GC_FREE (embryonic_routine_name), embryonic_routine_name =
	(const char **) newnames;
      GC_FREE (embryonic_routine_arr), embryonic_routine_arr = newarr;
      eix = embryonic_routine_size;
      embryonic_routine_size = newsiz;
    }
  embryonic_routine_name[eix] = dupname;
  embryonic_routine_arr[eix] = itrout;
  pthread_mutex_unlock (&embryonic_mtx);
}
