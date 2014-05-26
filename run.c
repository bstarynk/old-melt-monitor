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

// the poll timeout is 1.6 seconds without debugging, and 3.1 with debugging 'run'
#define MOM_POLL_TIMEOUT (MOM_IS_DEBUGGING(run)?4100:1600)	/* milliseconds for mom poll timeout */



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
	  double delay = (MOM_IS_DEBUGGING (run) ? 3.7 : 1.5)
	    + 0.001 * (mom_random_32 () % 256);
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
  MOM_DEBUG (run,
	     "handling signals SIGTERM=%d SIGQUIT=%d SIGPIPE=%d SIGCHLD=%d",
	     SIGTERM, SIGQUIT, SIGPIPE, SIGCHLD);
  MOM_DEBUG (run, "before sigprocmask");
  if (sigprocmask (SIG_BLOCK, &mysetsig, NULL))
    MOM_FATAPRINTF ("failed to block signals with sigprocmask");
  MOM_DEBUG (run, "before signalfd");
  my_signals_fd_mom = signalfd (-1, &mysetsig, SFD_NONBLOCK | SFD_CLOEXEC);
  if (MOM_UNLIKELY (my_signals_fd_mom <= 0))
    MOM_FATAPRINTF ("signalfd failed");
  MOM_DEBUG (run, "my_signals_fd=%d", my_signals_fd_mom);
}


#define POLL_MAX_MOM 100
static void *
event_loop_mom (void *p __attribute__ ((unused)))
{
  long long evloopcnt = 0;
  bool repeat_loop = false;
  struct pollfd polltab[POLL_MAX_MOM];
  memset (polltab, 0, sizeof (polltab));
  assert (my_signals_fd_mom > 0);
}

#define check_for_some_child_process_mom() \
  check_for_some_child_process_at_mom(__FILE__,__LINE__)
static void check_for_some_child_process_at_mom (const char *srcfil,
						 int srclin);

void
mom_run_workers (void)
{
  bool again = false;
  MOM_DEBUGPRINTF (run, "mom_run_workers starting mom_nb_workers=%d",
		   mom_nb_workers);
  initialize_signals_mom ();
  do
    {
      if (mom_nb_workers < MOM_MIN_WORKERS)
	mom_nb_workers = MOM_MIN_WORKERS;
      else if (mom_nb_workers > MOM_MAX_WORKERS)
	mom_nb_workers = MOM_MAX_WORKERS;
      MOM_DEBUGPRINTF (run, "mom_start_workers nb_workers=%d",
		       mom_nb_workers);
      assert (mom_named__agenda != NULL
	      && mom_named__agenda->i_typnum == momty_item);
      {
	pthread_mutex_lock (&mom_named__agenda->i_mtx);
	if (stop_working_mom)
	  again = false;
	pthread_mutex_unlock (&mom_named__agenda->i_mtx);
      }
      if (!again)
	break;
      stop_working_mom = false;
      unsigned curnbwork = mom_nb_workers;
      for (unsigned ix = 1; ix <= curnbwork; ix++)
	{
	  work_data_array_mom[ix].work_magic = WORK_MAGIC;
	  work_data_array_mom[ix].work_index = ix;
	  work_data_array_mom[ix].work_running = false;
	  if (GC_pthread_create (&work_data_array_mom[ix].work_thread, NULL,
				 work_run_mom, &work_data_array_mom[ix]))
	    MOM_FATAPRINTF ("failed to create work thread #%d", ix);
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
      MOM_DEBUGPRINTF (run, "mom_start_workers joined %d work threads",
		       curnbwork);
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
	MOM_DEBUGPRINTF (run, "did %d todos", nbtodo);
	if (stop_working_mom)
	  again = false;
	if (continue_working_mom)
	  again = true;
	continue_working_mom = false;
	pthread_mutex_unlock (&mom_named__agenda->i_mtx);
      }
      MOM_DEBUGPRINTF (run, "mom_start_workers again %s",
		       again ? "true" : "false");
    }
  while (again);
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
#warning should make a tasklet for process completing
    }
}
