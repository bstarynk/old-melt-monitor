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

#define WORK_MAGIC 0x5c59b171	/* work magic 1549382001 */
struct workdata_mom_st
{
  unsigned work_magic;		/* always WORK_MAGIC */
  uint8_t work_index;
  bool work_running;
  pthread_t work_thread;
};

#define TODO_MAX_MOM (MOM_MAX_WORKERS)
// In practice we'll have only one thing to do at most
static struct
{
  mom_todoafterstop_fun_t *todo_fun;
  void *todo_data;
} todo_after_stop_mom[TODO_MAX_MOM];

static bool stop_working_mom;
static bool continue_working_mom;
static __thread struct workdata_mom_st *cur_worker_mom;
static struct workdata_mom_st work_data_array_mom[MOM_MAX_WORKERS + 1];
static int64_t task_counter_mom;
// this cond is tied to the agenda's mutex
static pthread_cond_t agenda_cond_mom = PTHREAD_COND_INITIALIZER;

static pthread_t event_loop_thread_mom;

static int event_loop_pipe_mom[2] = { -1, -1 };

static int my_signals_fd_mom = -1;
#define event_loop_read_pipe_mom event_loop_pipe_mom[0]
#define event_loop_write_pipe_mom event_loop_pipe_mom[1]


// communication between other threads & event loop thread thru single byte sent on pipe
#define EVLOOP_STOP '.'		/* stop the event loop */
#define EVLOOP_JOB 'J'		/* something changed about jobs */

static pthread_mutex_t job_mtx_mom = PTHREAD_MUTEX_INITIALIZER;
static momitem_t *running_jobs_mom[MOM_MAX_WORKERS + 1];
static struct mom_valuequeue_st pending_jobs_queue_mom;

// the poll timeout is 1.8 seconds without debugging, and 4.5 with debugging 'run'
#define MOM_POLL_TIMEOUT (MOM_IS_DEBUGGING(run)?4500:1800)	/* milliseconds for mom poll timeout */



static void run_one_tasklet_mom (momitem_t *tskitm);

void
mom_add_tasklet_to_agenda_back (momitem_t *tkitm)
{
  // we don't bother locking the tkitm to test its taskletness
  if (!tkitm || tkitm->i_typnum != momty_item
      || tkitm->i_paylkind != mompayk_tasklet)
    return;
  assert (mom_named__agenda != NULL
	  && mom_named__agenda->i_typnum == momty_item);
  pthread_mutex_lock (&mom_named__agenda->i_mtx);
  if (MOM_UNLIKELY (mom_named__agenda->i_paylkind != mompayk_queue))
    mom_item_start_queue (mom_named__agenda);
  bool agendawasempty = mom_item_queue_is_empty (mom_named__agenda);
  mom_item_queue_add_back (mom_named__agenda, (momval_t) tkitm);
  if (agendawasempty)
    pthread_cond_broadcast (&agenda_cond_mom);
  pthread_mutex_unlock (&mom_named__agenda->i_mtx);
}

void
mom_add_tasklet_to_agenda_front (momitem_t *tkitm)
{
  // we don't bother locking the tkitm to test its taskletness
  if (!tkitm || tkitm->i_typnum != momty_item
      || tkitm->i_paylkind != mompayk_tasklet)
    return;
  assert (mom_named__agenda != NULL
	  && mom_named__agenda->i_typnum == momty_item);
  pthread_mutex_lock (&mom_named__agenda->i_mtx);
  if (MOM_UNLIKELY (mom_named__agenda->i_paylkind != mompayk_queue))
    mom_item_start_queue (mom_named__agenda);
  bool agendawasempty = mom_item_queue_is_empty (mom_named__agenda);
  mom_item_queue_add_front (mom_named__agenda, (momval_t) tkitm);
  if (agendawasempty)
    pthread_cond_broadcast (&agenda_cond_mom);
  pthread_mutex_unlock (&mom_named__agenda->i_mtx);
}

static void *
work_run_mom (void *p)
{
  assert ((char *) p > (char *) work_data_array_mom
	  && (char *) p <= (char *) (work_data_array_mom + MOM_MAX_WORKERS));
  assert (mom_named__agenda != NULL
	  && mom_named__agenda->i_typnum == momty_item);
  struct workdata_mom_st *thiswork = (struct workdata_mom_st *) p;
  unsigned wix = thiswork - work_data_array_mom;
  assert (wix > 0 && wix <= MOM_MAX_WORKERS
	  && wix <= (unsigned) mom_nb_workers);
  assert (p == (void *) (work_data_array_mom + wix));
  {
    char thrname[24];
    memset (thrname, 0, sizeof (thrname));
    snprintf (thrname, sizeof (thrname), "mom-work%02d", wix);
    pthread_setname_np (pthread_self (), thrname);
  }
  thiswork->work_index = wix;
  cur_worker_mom = thiswork;
  MOM_DEBUGPRINTF (run, "work_run_mom: begin wix#%d", wix);
  uint64_t wcount = 0;
  bool again = true;
  usleep (10 + mom_random_32 () % 256);
  while (again)
    {
      bool agendaempty = false;
      pthread_mutex_lock (&mom_named__agenda->i_mtx);
      if (MOM_UNLIKELY (mom_named__agenda->i_paylkind != mompayk_queue))
	mom_item_start_queue (mom_named__agenda);
      agendaempty = mom_item_queue_is_empty (mom_named__agenda);
      if (stop_working_mom)
	{
	  thiswork->work_running = false;
	  again = false;
	  MOM_DEBUGPRINTF (run, "work_run_mom: stop working wix#%d", wix);
	}
      else if (agendaempty)
	{
	  thiswork->work_running = false;
	  double delay = (MOM_IS_DEBUGGING (run) ? 4.7 : 2.5)
	    + 0.002 * (mom_random_32 () % 256) + wix * 0.01;
	  struct timespec ts
	    = mom_timespec (mom_clock_time (CLOCK_REALTIME) + delay);
	  MOM_DEBUGPRINTF (run, "work_run_mom: waiting agenda cond wix#%d",
			   wix);
	  pthread_cond_timedwait (&agenda_cond_mom, &mom_named__agenda->i_mtx,
				  &ts);
	  MOM_DEBUGPRINTF (run, "work_run_mom: waited agenda cond wix#%d",
			   wix);
	}
      else
	{
	  momitem_t *curtskitm =
	    mom_value_to_item (mom_item_queue_pop_front (mom_named__agenda));
	  MOM_DEBUG (run, MOMOUT_LITERAL ("work_run_mom: wix#"),
		     MOMOUT_DEC_INT ((int) wix),
		     MOMOUT_LITERAL (", curtskitm="),
		     MOMOUT_ITEM ((const momitem_t *) curtskitm));
	  if (curtskitm)
	    {
	      thiswork->work_running = true;
	      wcount++;
	      task_counter_mom++;
	      MOM_DEBUGPRINTF (run,
			       "work_run_mom: wix#%d wcount=%ld task_counter_mom=%lld",
			       wix, (long) wcount,
			       (long long) task_counter_mom);
	      pthread_mutex_unlock (&mom_named__agenda->i_mtx);
	      run_one_tasklet_mom (curtskitm);
	      continue;
	    }
	}
      pthread_mutex_unlock (&mom_named__agenda->i_mtx);
    }				/* end while again */
  cur_worker_mom = NULL;
  return NULL;
}

void
mom_stop_work_with_todo (mom_todoafterstop_fun_t * todof, void *data)
{
  assert (mom_named__agenda != NULL
	  && mom_named__agenda->i_typnum == momty_item);
  MOM_DEBUGPRINTF (run, "mom_stop_work_with_todo todof@%p data@%p", todof,
		   data);
  pthread_mutex_lock (&mom_named__agenda->i_mtx);
  stop_working_mom = true;
  if (todof)
    {
      bool foundslot = false;
      for (unsigned ix = 0; ix < TODO_MAX_MOM && !foundslot; ix++)
	if (!todo_after_stop_mom[ix].todo_fun)
	  {
	    todo_after_stop_mom[ix].todo_fun = todof;
	    todo_after_stop_mom[ix].todo_data = data;
	    foundslot = true;
	  };
      if (!foundslot)
	MOM_FATAPRINTF ("failed to find todo slot");
    }
  pthread_mutex_unlock (&mom_named__agenda->i_mtx);
}





static bool
step_tasklet_mom (momitem_t *tkitm, struct mom_taskletdata_st *itd)
{
  bool res = false;
  bool popframe = false;
  unsigned fratop = itd->dtk_fratop;
  if (!fratop)
    return false;
  int state = 0;
  momval_t *locvals = NULL;
  intptr_t *locnums = NULL;
  double *locdbls = NULL;
  struct momroutinedescr_st *rdescr = NULL;
  mom_routine_sig_t *routcod = NULL;
  assert (fratop < itd->dtk_frasize);
  struct momframe_st *curfram = itd->dtk_frames + fratop;
  const momnode_t *curclo = itd->dtk_closures[fratop];
  if (!curclo || curclo->typnum != momty_node)
    {
      mom_item_tasklet_pop_frame (tkitm);
      return itd->dtk_fratop > 0;
    }
  momitem_t *routitm = (momitem_t *) curclo->connitm;
  if (routitm || routitm->i_typnum != momty_item)
    {
      mom_item_tasklet_pop_frame (tkitm);
      return itd->dtk_fratop > 0;
    }
  pthread_mutex_lock (&routitm->i_mtx);
  if (routitm->i_paylkind != mompayk_routine)
    {
      popframe = true;
      goto end;
    }
  rdescr = routitm->i_payload;
  if (MOM_UNLIKELY
      (!rdescr || rdescr->rout_magic != MOM_ROUTINE_MAGIC
       || !rdescr->rout_codefun))
    {
      popframe = true;
      goto end;
    }
  if (curclo->slen < rdescr->rout_minclosize)
    {
      popframe = true;
      goto end;
    }
  state = curfram->fr_state;
  locvals = itd->dtk_values + curfram->fr_valoff;
  locnums = itd->dtk_scalars + curfram->fr_intoff;
  locdbls = (double *) (itd->dtk_scalars + curfram->fr_dbloff);
  routcod = rdescr->rout_codefun;
end:
  pthread_mutex_unlock (&routitm->i_mtx);
  if (routcod)
    {
      MOM_DEBUG (run, MOMOUT_LITERAL ("step_tasklet_mom alling routine "),
		 MOMOUT_LITERALV (rdescr->rout_name),
		 MOMOUT_LITERAL (" at state#"),
		 MOMOUT_DEC_INT (state),
		 MOMOUT_LITERAL (" with taskitem:"),
		 MOMOUT_ITEM ((const momitem_t *) tkitm));
      int newstate =
	routcod (state, tkitm, curclo, locvals, locnums, locdbls);
      if (newstate == routres_pop)
	popframe = true;
      else
	(itd->dtk_frames + fratop)->fr_state = newstate;
    }
  if (popframe)
    mom_item_tasklet_pop_frame (tkitm);
  return res;
}


#define TASKLET_TIMEOUT 0.002
void
run_one_tasklet_mom (momitem_t *tkitm)
{
  struct mom_taskletdata_st *itd = NULL;
  MOM_DEBUG (run, MOMOUT_LITERAL ("run_one_tasklet_mom start tkitm:"),
	     MOMOUT_ITEM ((const momitem_t *) tkitm));
  pthread_mutex_lock (&tkitm->i_mtx);
  if (tkitm->i_paylkind != mompayk_tasklet)
    goto end;
  double timestart = mom_clock_time (CLOCK_REALTIME);
  double timelimit = timestart + TASKLET_TIMEOUT;
  itd = tkitm->i_payload;
  itd->dtk_thread = pthread_self ();
  unsigned nbsteps = mom_random_32 () % 16 + 3;
  unsigned stepcount = 0;
  MOM_DEBUG (run, MOMOUT_LITERAL ("run_one_tasklet_mom nbsteps:"),
	     MOMOUT_DEC_INT ((int) nbsteps));
  for (unsigned stepix = 0; stepix < nbsteps; stepix++)
    {
      if (!step_tasklet_mom (tkitm, itd))
	break;
      stepcount++;
      if (tkitm->i_payload != itd)
	break;
      if (stepix % 2 == 0 && mom_clock_time (CLOCK_REALTIME) > timelimit)
	break;
    }
  MOM_DEBUG (run, MOMOUT_LITERAL ("run_one_tasklet_mom done stepcount="),
	     MOMOUT_DEC_INT ((int) stepcount));
end:
  if (itd)
    itd->dtk_thread = 0;
  pthread_mutex_unlock (&tkitm->i_mtx);
}


static void
initialize_signals_mom (void)
{
  // set up the signalfd 
  sigset_t mysetsig;
  sigemptyset (&mysetsig);
  errno = 0;
  sigaddset (&mysetsig, SIGTERM);
  sigaddset (&mysetsig, SIGQUIT);
  sigaddset (&mysetsig, SIGPIPE);
  sigaddset (&mysetsig, SIGCHLD);
  // according to signalfd(2) we need to block the signals
  MOM_DEBUGPRINTF (run,
		   "handling signals SIGTERM=%d SIGQUIT=%d SIGPIPE=%d SIGCHLD=%d",
		   SIGTERM, SIGQUIT, SIGPIPE, SIGCHLD);
  MOM_DEBUGPRINTF (run, "before sigprocmask");
  if (sigprocmask (SIG_BLOCK, &mysetsig, NULL))
    MOM_FATAPRINTF ("failed to block signals with sigprocmask");
  MOM_DEBUGPRINTF (run, "before signalfd");
  my_signals_fd_mom = signalfd (-1, &mysetsig, SFD_NONBLOCK | SFD_CLOEXEC);
  if (MOM_UNLIKELY (my_signals_fd_mom <= 0))
    MOM_FATAPRINTF ("signalfd failed");
  MOM_DEBUGPRINTF (run, "my_signals_fd=%d", my_signals_fd_mom);
}

#define check_for_some_child_process_mom() \
  check_for_some_child_process_at_mom(__FILE__,__LINE__)
static void check_for_some_child_process_at_mom (const char *srcfil,
						 int srclin);


typedef void mom_poll_handler_sig_t (int fd, short revent, void *data);


static void start_some_pending_jobs_mom (void);

static void
event_loop_handler_mom (int fd, short revent, void *data)
{
  MOM_DEBUGPRINTF (run, "event_loop_handler_mom fd=%d revent=%#x", fd,
		   revent);
  assert (fd == event_loop_read_pipe_mom && data);
  bool *prepeatloop = data;
  char rbuf[4];
  memset (rbuf, 0, sizeof (rbuf));
  if (read (fd, &rbuf, 1) > 0)
    {
      MOM_DEBUGPRINTF (run, "event_loop_handler rbuf='%c'", rbuf[0]);
      switch (rbuf[0])
	{
	case EVLOOP_STOP:
	  *prepeatloop = false;
	  break;
	case EVLOOP_JOB:
	  start_some_pending_jobs_mom ();
	  break;
	default:
	  MOM_FATAPRINTF ("unexpected loop command %c = %d", rbuf[0],
			  (int) rbuf[0]);
	}
    }
}


void
mom_stop_event_loop (void)
{
  static const char stopmsg[] = { EVLOOP_STOP, 0 };
  MOM_DEBUGPRINTF (run, "stop_event_loop writepipefd#%d stopmsg=%s",
		   event_loop_write_pipe_mom, stopmsg);
  if (write (event_loop_write_pipe_mom, stopmsg, 1) < 0)
    MOM_FATAPRINTF ("failed to write stop event on writepipefd#%d",
		    event_loop_write_pipe_mom);
}


static void
todo_dump_at_termination_mom (void *data)
{
  char *dpath = data;
  assert (dpath && dpath[0]);
  MOM_DEBUGPRINTF (run, "todo_dump_at_termination_mom should dump dpath=%s",
		   dpath);
  mom_full_dump ("todo dump at termination", dpath, NULL);
  mom_stop_event_loop ();
  MOM_INFORMPRINTF ("dumped after SIGTERM signal into directory %s", dpath);
}

static void
mysignalfd_handler_mom (int fd, short revent, void *data)
{
  struct signalfd_siginfo sinf;
  memset (&sinf, 0, sizeof (sinf));
  MOM_DEBUGPRINTF (run, "mysignalfd_handler fd=%d sizeof(sinf)=%d", fd,
		   (int) sizeof (sinf));
  assert (fd == my_signals_fd_mom && !data);
  int rdsiz = -2;
  if (revent & POLLIN)
    {
      MOM_DEBUGPRINTF (run, "mysignalfd_handler reading fd=%d", fd);
      rdsiz = read (fd, &sinf, sizeof (sinf));
    }
  MOM_DEBUGPRINTF (run, "mysignalfd_handler fd=%d rdsiz=%d", fd, rdsiz);
  if (rdsiz < 0)
    {
      MOM_DEBUGPRINTF (run, "mysignalfd_handler read failed (%s)",
		       strerror (errno));
      return;
    }
  MOM_DEBUGPRINTF (run, "mysignalfd_handler signo=%d (%s)",
		   sinf.ssi_signo, strsignal (sinf.ssi_signo));
  switch (sinf.ssi_signo)
    {
    case SIGCHLD:
      check_for_some_child_process_mom ();
      break;
    case SIGTERM:
      {
	char termpath[MOM_PATH_MAX];
	struct tm nowtm = { 0 };
	time_t nowt = 0;
	char pidbuf[32];
	memset (termpath, 0, sizeof (termpath));
	time (&nowt);
	strftime (termpath, sizeof (termpath) - sizeof (pidbuf),
		  "_monimelt_termdump_%Y%b%d_%Hh%M_",
		  localtime_r (&nowt, &nowtm));
	snprintf (pidbuf, sizeof (pidbuf), "pid%d", (int) getpid ());
	strcat (termpath, pidbuf);
	const char *termstr = MOM_GC_STRDUP ("terminating dir", termpath);
	MOM_DEBUGPRINTF (run, "recieved SIGTERM signal, will try dump to %s",
			 termstr);
	if (mkdir (termstr, 0750))
	  MOM_FATAPRINTF ("failed to make terminating dir %s", termstr);
	mom_stop_work_with_todo (todo_dump_at_termination_mom,
				 (char *) termstr);
      }
      break;
    default:
      MOM_WARNPRINTF ("unexpected signal#%d : %s", sinf.ssi_signo,
		      strsignal (sinf.ssi_signo));
      break;
    }
}



#define MOM_MAX_OUTPUT_LEN (1024*1024)	/* one megabyte */
static void
process_readout_handler_mom (int fd, short revent, void *data)
{
  momitem_t *procitm = data;
  assert (procitm && procitm->i_typnum == momty_item);
  MOM_DEBUG (run, MOMOUT_LITERAL ("process_readout_handler_mom revent="),
	     MOMOUT_DEC_INT ((int) revent),
	     MOMOUT_LITERAL (" fd="),
	     MOMOUT_DEC_INT (fd),
	     MOMOUT_LITERAL (" procitm:"),
	     MOMOUT_ITEM ((const momitem_t *) procitm));
  if (!mom_lock_item (procitm))
    return;
  if (procitm->i_paylkind != mompayk_process)
    goto end;
  struct mom_process_data_st *procdata = procitm->i_payload;
  assert (procdata && procdata->iproc_magic == MOM_PROCESS_MAGIC);
  if (procdata->iproc_pid <= 0 || procdata->iproc_outfd <= 0
      || !procdata->iproc_outbuf)
    goto end;
  assert (procdata->iproc_outfd == fd);
#define PROCOUT_BUFSIZE 4096
  char rdbuf[PROCOUT_BUFSIZE];
  memset (rdbuf, 0, sizeof (rdbuf));
  int nbr = read (fd, rdbuf, sizeof (rdbuf));
  MOM_DEBUGPRINTF (run, "process_readout_handler fd=%d, nbr=%d, rdbuf=%s\n",
		   fd, nbr, rdbuf);
  if (nbr > 0)
    {
      if (MOM_UNLIKELY
	  (nbr + procdata->iproc_outpos + 1 >= procdata->iproc_outsize))
	{
	  unsigned newsiz =
	    ((5 * (nbr + procdata->iproc_outpos) / 4 + 100) | 0x1f) + 1;
	  char *newbuf =
	    MOM_GC_SCALAR_ALLOC ("grown process output buffer", newsiz);
	  char *oldbuf = procdata->iproc_outbuf;
	  memcpy (newbuf, oldbuf, procdata->iproc_outpos);
	  MOM_GC_FREE (oldbuf);
	  procdata->iproc_outbuf = newbuf;
	  procdata->iproc_outsize = newsiz;
	}
      memcpy (procdata->iproc_outbuf + procdata->iproc_outpos, rdbuf, nbr);
      procdata->iproc_outbuf[procdata->iproc_outpos + nbr] = (char) 0;
      procdata->iproc_outpos += nbr;
      if (procdata->iproc_outpos > MOM_MAX_OUTPUT_LEN)
	{
	  MOM_WARNPRINTF
	    ("closing pipe from process pid#%d (%s) output buffer full (%ld bytes)",
	     (int) procdata->iproc_pid,
	     mom_string_cstr ((momval_t) procdata->iproc_progname),
	     (long) procdata->iproc_outpos);
	  procdata->iproc_outfd = -1;
	  // the child may get a SIGPIPE if he writes more.
	  close (fd);
	}
    }
end:
  mom_unlock_item (procitm);
}


#define POLL_MAX_MOM 100

static void *
event_loop_mom (void *p __attribute__ ((unused)))
{
  long long evloopcnt = 0;
  bool repeat_loop = false;
  struct pollfd polltab[POLL_MAX_MOM];
  mom_poll_handler_sig_t *handlertab[POLL_MAX_MOM];
  void **datatab[POLL_MAX_MOM];
  pthread_setname_np (pthread_self (), "mom-evloop");
  assert (my_signals_fd_mom > 0);
  /// our event loop
  do
    {
      int pollcnt = 0;
      repeat_loop = true;
      evloopcnt++;
      memset (polltab, 0, sizeof (polltab));
      memset (handlertab, 0, sizeof (handlertab));
      memset (datatab, 0, sizeof (datatab));
      MOM_DEBUGPRINTF (run, "start event loop #%lld", evloopcnt);
      GC_collect_a_little ();
      // check for ended processes from time to time, to be safe
      if (evloopcnt % 8 == 0)
	check_for_some_child_process_mom ();
      /// add various pollings
#define ADD_POLL(Ev,Fd,Hdr,Data) do {					\
	if (MOM_UNLIKELY(pollcnt>=POLL_MAX_MOM))			\
	  MOM_FATAPRINTF("failed to poll fd#%d", (Fd));			\
	polltab[pollcnt].fd = (Fd);					\
	polltab[pollcnt].events = (Ev);					\
	MOM_DEBUGPRINTF(run, "add_poll pollcnt=%d, fd=%d " #Hdr,	\
			pollcnt, (Fd));					\
	handlertab[pollcnt] = Hdr;					\
	datatab[pollcnt]= (void*)(Data);				\
	pollcnt++; } while(0)
      //
      ADD_POLL (POLLIN, event_loop_read_pipe_mom, event_loop_handler_mom,
		&repeat_loop);
      ADD_POLL (POLLIN, my_signals_fd_mom, mysignalfd_handler_mom, NULL);
      // add the running processes reading
      {
	pthread_mutex_lock (&job_mtx_mom);
	for (unsigned jix = 1; jix <= MOM_MAX_WORKERS; jix++)
	  {
	    momitem_t *curjobitm = running_jobs_mom[jix];
	    if (!curjobitm)
	      continue;
	    assert (curjobitm->i_typnum == momty_item);
	    pthread_mutex_lock (&curjobitm->i_mtx);
	    struct mom_process_data_st *curjobdata =
	      (curjobitm->i_paylkind ==
	       mompayk_process) ? curjobitm->i_payload : NULL;
	    pthread_mutex_unlock (&curjobitm->i_mtx);
	    if (!curjobdata)
	      continue;
	    assert (curjobdata->iproc_magic == MOM_PROCESS_MAGIC);
	    if (curjobdata->iproc_jobnum == jix && curjobdata->iproc_pid > 0
		&& curjobdata->iproc_outfd > 0)
	      ADD_POLL (POLLIN, curjobdata->iproc_outfd,
			process_readout_handler_mom, curjobitm);

	  }
	pthread_mutex_unlock (&job_mtx_mom);
      }
      // other polling might go here, eg. perhaps for CURL web client library
      // do the polling
      MOM_DEBUGPRINTF (run, "before poll pollcnt=%d evloopcnt=%lld",
		       pollcnt, evloopcnt);
      int respoll = poll (polltab, pollcnt, MOM_POLL_TIMEOUT);
      MOM_DEBUGPRINTF (run, "after poll respoll=%d evloopcnt=%lld", respoll,
		       evloopcnt);
      // invoke the handlers
      if (respoll > 0)
	{
	  for (int pix = 0; pix < pollcnt; pix++)
	    {
	      if (polltab[pix].revents && handlertab[pix])
		{
		  MOM_DEBUGPRINTF (run,
				   "invoking handler pix=%d fd#%d revents=%#x%s%s%s%s%s%s evloopcnt=%lld",
				   pix, polltab[pix].fd, polltab[pix].revents,
				   (polltab[pix].revents & POLLIN) ? ";POLLIN"
				   : "",
				   (polltab[pix].revents & POLLOUT) ?
				   ";POLLOUT" : "",
				   (polltab[pix].revents & POLLERR) ?
				   ";POLLERR" : "",
				   (polltab[pix].revents & POLLHUP) ?
				   ";POLLHUP" : "",
				   (polltab[pix].revents & POLLNVAL) ?
				   ";POLLNVAL" : "",
				   (polltab[pix].revents & POLLPRI) ?
				   ";POLLPRI" : "", evloopcnt);
		  handlertab[pix] (polltab[pix].fd, polltab[pix].revents,
				   datatab[pix]);
		  MOM_DEBUGPRINTF (run,
				   "done handler pix=%d fd#%d evloopcnt=%lld",
				   pix, polltab[pix].fd, evloopcnt);
		}
	    }
	  sched_yield ();
	}
      else
	{			// timed-out
	  MOM_DEBUGPRINTF (run, "poll timed out evloopcnt=%lld", evloopcnt);
	  check_for_some_child_process_mom ();
	  MOM_DEBUGPRINTF (run, "poll done timed out evloopcnt=%lld",
			   evloopcnt);
	};
    }
  while (repeat_loop);
  return NULL;
}


void
mom_start_event_loop (void)
{
  static pthread_attr_t evthattr;
  pthread_attr_init (&evthattr);
  pthread_attr_setdetachstate (&evthattr, TRUE);
  if (pipe (event_loop_pipe_mom))
    MOM_FATAPRINTF ("failed to create event loop pipe");
  MOM_DEBUGPRINTF (run,
		   "mom_start_event_loop event_loop_read_pipe=%d event_loop_write_pipe=%d",
		   event_loop_read_pipe_mom, event_loop_write_pipe_mom);
  if (GC_pthread_create
      (&event_loop_thread_mom, &evthattr, event_loop_mom, NULL))
    MOM_FATAPRINTF ("failed to create event loop thread");
  MOM_DEBUGPRINTF (run, "mom_start_event_loop done, event pthread#%ld",
		   (long) event_loop_thread_mom);
}


void
mom_run_workers (void)
{
  bool again = false;
  long workcnt = 0;
  MOM_DEBUGPRINTF (run, "mom_run_workers starting mom_nb_workers=%d",
		   mom_nb_workers);
  initialize_signals_mom ();
  mom_start_event_loop ();
  do
    {
      workcnt++;
      if (mom_nb_workers < MOM_MIN_WORKERS)
	mom_nb_workers = MOM_MIN_WORKERS;
      else if (mom_nb_workers > MOM_MAX_WORKERS)
	mom_nb_workers = MOM_MAX_WORKERS;
      MOM_DEBUGPRINTF (run, "mom_start_workers nb_workers=%d workcnt=%ld",
		       mom_nb_workers, workcnt);
      assert (mom_named__agenda != NULL
	      && mom_named__agenda->i_typnum == momty_item);
      {
	pthread_mutex_lock (&mom_named__agenda->i_mtx);
	if (stop_working_mom)
	  again = false;
	else
	  again = true;
	stop_working_mom = false;
	pthread_mutex_unlock (&mom_named__agenda->i_mtx);
      }
      if (!again)
	{
	  MOM_DEBUGPRINTF (run, "mom_start_workers breaking workcnt=%ld",
			   workcnt);
	  break;
	}
      unsigned curnbwork = mom_nb_workers;
      for (unsigned ix = 1; ix <= curnbwork; ix++)
	{
	  work_data_array_mom[ix].work_magic = WORK_MAGIC;
	  work_data_array_mom[ix].work_index = ix;
	  work_data_array_mom[ix].work_running = false;
	  if (GC_pthread_create (&work_data_array_mom[ix].work_thread, NULL,
				 work_run_mom, &work_data_array_mom[ix]))
	    MOM_FATAPRINTF ("failed to create work thread #%d workcnt=%ld",
			    ix, workcnt);
	};
      MOM_DEBUGPRINTF (run, "mom_start_workers created %d work threads",
		       curnbwork);
      sched_yield ();
      for (unsigned ix = 1; ix <= curnbwork; ix++)
	{
	  void *retwork = NULL;
	  if (GC_pthread_join (work_data_array_mom[ix].work_thread, &retwork))
	    MOM_FATAPRINTF ("failed to join work thread #%d", ix);
	}
      MOM_DEBUGPRINTF (run,
		       "mom_start_workers joined %d work threads workcnt=%ld",
		       curnbwork, workcnt);
      {
	unsigned nbtodo = 0;
	pthread_mutex_lock (&mom_named__agenda->i_mtx);
	for (unsigned dix = 0; dix < TODO_MAX_MOM; dix++)
	  {
	    if (todo_after_stop_mom[dix].todo_fun)
	      {
		todo_after_stop_mom[dix].todo_fun
		  (todo_after_stop_mom[dix].todo_data);
		nbtodo++;
	      }
	  }
	memset (todo_after_stop_mom, 0, sizeof (todo_after_stop_mom));
	MOM_DEBUGPRINTF (run, "did %d todos workcnt=%ld", nbtodo, workcnt);
	if (stop_working_mom)
	  again = false;
	if (continue_working_mom)
	  again = true;
	continue_working_mom = false;
	pthread_mutex_unlock (&mom_named__agenda->i_mtx);
      }
      MOM_DEBUGPRINTF (run, "mom_start_workers again %s workcnt=%ld",
		       again ? "true" : "false", workcnt);
    }
  while (again);
  MOM_DEBUGPRINTF (run,
		   "mom_run_workers ending mom_nb_workers=%d workcnt=%ld",
		   mom_nb_workers, workcnt);
}


static void
start_some_pending_jobs_mom (void)
{
  int nbrunning = 0;
  int nbstarting = 0;
  momitem_t *proctab[MOM_MAX_WORKERS] = { };
  memset (proctab, 0, sizeof (proctab));
  MOM_DEBUGPRINTF (run, "start_some_pending_jobs");
  pthread_mutex_lock (&job_mtx_mom);
  for (unsigned jix = 1; jix <= MOM_MAX_WORKERS; jix++)
    {
      momitem_t *curjobitm = running_jobs_mom[jix];
      if (!curjobitm)
	continue;
      assert (curjobitm->i_typnum == momty_item);
      pthread_mutex_lock (&curjobitm->i_mtx);
      struct mom_process_data_st *curjobproc =
	(curjobitm->i_paylkind ==
	 mompayk_process) ? curjobitm->i_payload : NULL;
      pthread_mutex_unlock (&curjobitm->i_mtx);
      if (!curjobproc)
	continue;
      assert (curjobproc != NULL
	      && curjobproc->iproc_magic == MOM_PROCESS_MAGIC);
      if (curjobproc->iproc_jobnum == jix && curjobproc->iproc_pid > 0
	  && !kill (curjobproc->iproc_pid, 0))
	nbrunning++;
    }
  while (nbrunning < mom_nb_workers && nbstarting < mom_nb_workers
	 && !mom_queue_is_empty (&pending_jobs_queue_mom))
    {
      momitem_t *curjobitm =
	mom_value_to_item (mom_queue_pop_value_front
			   (&pending_jobs_queue_mom));
      assert (curjobitm && curjobitm->i_typnum == momty_item);
      if (curjobitm->i_paylkind != mompayk_process)
	continue;
      proctab[nbstarting++] = curjobitm;
    }
  MOM_DEBUGPRINTF (run, "start_some_pending_jobs nbstarting=%d, nbrunning=%d",
		   nbstarting, nbrunning);
  for (unsigned rix = 0; rix < (unsigned) nbstarting; rix++)
    {
      momitem_t *curprocitm = proctab[rix];
      if (!mom_lock_item (curprocitm))
	continue;
      if (curprocitm->i_paylkind != mompayk_process)
	goto endcurproc;
      struct mom_process_data_st *curprocdata = curprocitm->i_payload;
      assert (curprocdata && curprocdata->iproc_magic == MOM_PROCESS_MAGIC);
      int pipetab[2] = { -1, -1 };
      int jobnum = -1;
      for (unsigned j = 1; j <= MOM_MAX_WORKERS && jobnum < 0; j++)
	if (!running_jobs_mom[j])
	  jobnum = j;
      const char *progname =
	mom_string_cstr ((momval_t) curprocdata->iproc_progname);
      assert (progname && progname[0]);
      MOM_DEBUG (run, MOMOUT_LITERAL ("start_some_pending_jobs jobnum="),
		 MOMOUT_DEC_INT (jobnum),
		 MOMOUT_LITERAL (" curprocitm="),
		 MOMOUT_ITEM ((const momitem_t *) curprocitm),
		 MOMOUT_LITERAL (" progname="), MOMOUT_LITERALV (progname));
      assert (curprocdata->iproc_pid <= 0 && curprocdata->iproc_outfd < 0);
      assert (jobnum > 0);
      const char **progargv =
	MOM_GC_ALLOC ("start_some_pending_jobs program argv",
		      (curprocdata->iproc_argcount + 2) * sizeof (char *));
      progargv[0] = progname;
      unsigned argcnt = 1;
      for (unsigned aix = 0; aix < curprocdata->iproc_argcount; aix++)
	{
	  const char *argstr =
	    mom_string_cstr ((momval_t) (curprocdata->iproc_argv[aix]));
	  if (argstr)
	    progargv[argcnt++] = argstr;
	}
      progargv[argcnt] = NULL;
      for (unsigned aix = 0; aix < argcnt; aix++)
	MOM_DEBUGPRINTF (run, "run progargv[%d]=%s", aix, progargv[aix]);
      if (pipe (pipetab))
	MOM_FATAPRINTF ("failed to create pipe for process %s", progname);
      MOM_DEBUGPRINTF (run, "pipetab={r:%d,w:%d}", pipetab[0], pipetab[1]);
      fflush (NULL);
      pid_t newpid = fork ();
      if (newpid == 0)
	{
	  /* child process */
	  // most of our file descriptors should be close-on-exec, but
	  // to be safe we close a few of them...
	  for (int fd = 3; fd < 64; fd++)
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
	  signal (SIGTERM, SIG_DFL);
	  signal (SIGQUIT, SIG_DFL);
	  signal (SIGCHLD, SIG_DFL);
	  sigset_t mysetsig;
	  sigemptyset (&mysetsig);
	  sigaddset (&mysetsig, SIGTERM);
	  sigaddset (&mysetsig, SIGQUIT);
	  sigaddset (&mysetsig, SIGPIPE);
	  sigaddset (&mysetsig, SIGCHLD);
	  sigprocmask (SIG_UNBLOCK, &mysetsig, NULL);
	  execvp ((const char *) progname, (char *const *) progargv);
	  fprintf (stderr, "execution of %s failed : %s\n", progname,
		   strerror (errno));
	  fflush (NULL);
	  _exit (127);
	}
      else if (newpid < 0)
	MOM_FATAL ("fork failed for %s", progname);
      // parent process:
      MOM_DEBUGPRINTF (run,
		       "start_some_pending_jobs progname=%s newpid=%d, jobnum=%d",
		       progname, (int) newpid, jobnum);
      curprocdata->iproc_outfd = pipetab[0];
      curprocdata->iproc_pid = newpid;
      curprocdata->iproc_jobnum = jobnum;
      running_jobs_mom[jobnum] = curprocitm;
    endcurproc:
      mom_unlock_item (curprocitm);
    }
  pthread_mutex_unlock (&job_mtx_mom);
}



static void
check_for_some_child_process_at_mom (const char *srcfil, int srclin)
{
  int pst = 0;
  momitem_t *curprocitm = NULL;
  const momnode_t *curproclos = NULL;
  const momstring_t *curprocoutstr = NULL;
  MOM_DEBUGPRINTF (run, "check_for_some_child_process %s:%d start", srcfil,
		   srclin);
  pid_t wpid = waitpid (-1, &pst, WNOHANG);
  MOM_DEBUGPRINTF (run, "check_for_some_child_process %s:%d wpid=%d pst=%#x",
		   srcfil, srclin, wpid, pst);
  if (wpid < 0)
    return;
  pthread_mutex_lock (&job_mtx_mom);
  for (unsigned jix = 1; jix <= MOM_MAX_WORKERS && !curprocitm; jix++)
    {
      momitem_t *curjobitm = running_jobs_mom[jix];
      if (!curjobitm)
	continue;
      if (!mom_lock_item (curjobitm))
	continue;
      if (curjobitm->i_paylkind != mompayk_process)
	goto endjob;
      struct mom_process_data_st *curjobdata = curjobitm->i_payload;
      assert (curjobdata != NULL
	      && curjobdata->iproc_magic == MOM_PROCESS_MAGIC);
      if (curjobdata->iproc_jobnum == jix && curjobdata->iproc_pid > 0
	  && curjobdata->iproc_pid == wpid)
	{
	  curprocitm = curjobitm;
	  curproclos = curjobdata->iproc_closure;
	  if (curjobdata->iproc_outbuf)
	    curprocoutstr =
	      mom_make_string_len (curjobdata->iproc_outbuf,
				   curjobdata->iproc_outpos);
	  if (curjobdata->iproc_outfd > 0)
	    close (curjobdata->iproc_outfd), (curjobdata->iproc_outfd = -1);
	  MOM_GC_FREE (curjobdata->iproc_outbuf);
	  curjobdata->iproc_outsize = 0;
	  curjobdata->iproc_outpos = 0;
	  curjobdata->iproc_pid = 0;
	}
    endjob:
      mom_unlock_item (curjobitm);
    }
  goto end;
end:
  pthread_mutex_unlock (&job_mtx_mom);
  if (curprocitm && curproclos && curprocoutstr)
    {
      MOM_DEBUG (run,
		 MOMOUT_LITERAL ("check_for_some_child_process srcfil="),
		 MOMOUT_LITERALV (srcfil),
		 MOMOUT_LITERAL (" srclin="), MOMOUT_DEC_INT (srclin),
		 MOMOUT_LITERAL (" curprocitm="),
		 MOMOUT_ITEM ((const momitem_t *) curprocitm),
		 MOMOUT_LITERAL (" curproclos="),
		 MOMOUT_VALUE ((momval_t) curproclos),
		 MOMOUT_SPACE (32),
		 MOMOUT_LITERAL (" curprocoutstr="),
		 MOMOUT_VALUE ((momval_t) curprocoutstr),
		 MOMOUT_NEWLINE (), MOMOUT_END ());
      momitem_t *newtskitm = mom_make_item ();
      // dont need to lock the tasklet item, nobody knows it!
      mom_item_start_tasklet (newtskitm);
      if (WIFEXITED (pst))
	{
	  int exitstatus = WEXITSTATUS (pst);
	  MOM_DEBUGPRINTF (run,
			   "check_for_some_child_process %s:%d exitstatus=%d",
			   srcfil, srclin, exitstatus);
	  if (exitstatus == 0)
	    {
	      mom_item_tasklet_push_frame	/////
		(newtskitm, (momval_t) curproclos,
		 MOMPFR_FOUR_VALUES ((momval_t) curprocitm,
				     (momval_t) curprocoutstr,
				     (momval_t) mom_named__exited, MOM_NULLV),
		 MOMPFR_INT ((intptr_t) exitstatus), MOMPFR_END ());
	    }
	  else
	    {			/* exitstatus != 0, so failed */
	      mom_item_tasklet_push_frame	/////
		(newtskitm, (momval_t) curproclos,
		 MOMPFR_FOUR_VALUES ((momval_t) curprocitm,
				     (momval_t) curprocoutstr,
				     (momval_t) mom_named__failed,
				     (momval_t)
				     mom_make_integer (exitstatus)),
		 MOMPFR_INT ((intptr_t) exitstatus), MOMPFR_END ());
	    }
	}
      else if (WIFSIGNALED (pst))
	{
	  int termsig = WTERMSIG (pst);
	  const char *termsigstr = strsignal (termsig);
	  MOM_DEBUGPRINTF (run,
			   "check_for_some_child_process %s:%d termsig=%d::%s",
			   srcfil, srclin, termsig, termsigstr);
	  mom_item_tasklet_push_frame	/////
	    (newtskitm, (momval_t) curproclos,
	     MOMPFR_FOUR_VALUES ((momval_t) curprocitm,
				 (momval_t) curprocoutstr,
				 (momval_t) mom_named__terminated,
				 (momval_t) mom_make_string (termsigstr)),
	     MOMPFR_INT ((intptr_t) termsig), MOMPFR_END ());
	}
      else
	MOM_FATAPRINTF ("unexpected process status pst=%#x for wpid %d", pst,
			(int) wpid);
      MOM_DEBUG (run, MOMOUT_LITERAL ("check_for_some_child_process srcfil="),
		 MOMOUT_LITERALV (srcfil), MOMOUT_LITERAL (" srclin="),
		 MOMOUT_DEC_INT (srclin),
		 MOMOUT_LITERAL ("adding newtskitm="),
		 MOMOUT_ITEM ((const momitem_t *) newtskitm), MOMOUT_END ());
      mom_add_tasklet_to_agenda_front (newtskitm);
    }
  else
    MOM_WARNPRINTF ("check_for_some_child_process %s:%d wpid=%d not found",
		    srcfil, srclin, (int) wpid);
}
