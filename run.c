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

static long long work_tasklet_counter;

static pthread_t event_loop_thread;

static int event_loop_pipe[2] = { -1, -1 };

static int my_signals_fd = -1;
#define event_loop_read_pipe event_loop_pipe[0]
#define event_loop_write_pipe event_loop_pipe[1]

// communication between other threads & event loop thread thru single byte sent on pipe
#define EVLOOP_STOP '.'		/* stop the event loop */
#define EVLOOP_JOB 'J'		/* something changed about jobs */


// the poll timeout is 1.6 seconds without debugging, and 3.1 with debugging 'run'
#define MOM_POLL_TIMEOUT (MOM_IS_DEBUGGING(run)?4100:1600)	/* milliseconds for mom poll timeout */

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

#define MAX_POST_RUNNERS (2*MOM_MAX_WORKERS)

static mom_post_runner_sig_t *post_runner_funtab[MAX_POST_RUNNERS];
static void *post_runner_datatab[MAX_POST_RUNNERS];

extern momit_queue_t *mom_item__agenda;

static struct momworkdata_st workers[MOM_MAX_WORKERS + 1];
static bool working_flag;
static bool event_loop_started;
void
mom_agenda_add_tasklet_front (momval_t tsk)
{
  if (!tsk.ptr || *tsk.ptype != momty_taskletitem)
    return;
  assert (mom_item__agenda
	  && ((mom_anyitem_t *) mom_item__agenda)->typnum == momty_queueitem);
  mom_item_queue_push_front ((momval_t) mom_item__agenda, tsk);
  MOM_DBG_VALUE (run, "add tasklet front tsk=", tsk);
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
  MOM_DBG_VALUE (run, "add tasklet back tsk=", tsk);
  pthread_cond_broadcast (&mom_run_changed_cond);
}

long long
mom_agenda_work_counter (void)
{
  long long c = 0;
  pthread_mutex_lock (&mom_run_mtx);
  c = work_tasklet_counter;
  pthread_mutex_unlock (&mom_run_mtx);
  return c;
}

#define WORK_DELAY (MOM_IS_DEBUGGING(run)?9.0:5.8)	/* seconds */
#define WORK_GARBAGE_COLLECTION_CHECK_PERIOD 16
#define WORK_GARBAGE_COLLECTION_DELAY (MOM_IS_DEBUGGING(run)?15.0:4.5)	/* seconds */
static void *
work_loop (struct GC_stack_base *sb, void *data)
{
  struct momworkdata_st *wd = data;
  int myindex = 0;
  MOMGC_REGISTER_MY_THREAD (sb);
  assert (wd != NULL);
  assert (wd->work_magic == WORK_MAGIC);
  mom_anyitem_t *curtsk = NULL;
  myindex = wd->work_index;
  MOM_DEBUG (run,
	     "work_loop tid %d start index %d wd@%p  cur_worker@%p",
	     (int) mom_gettid (), myindex, wd, cur_worker);
  bool working = false;
  long loopcnt = 0;
  double lastgctime = 0.0;
  do
    {
      assert (wd->work_magic == WORK_MAGIC);
      curtsk = NULL;
      pthread_mutex_lock (&mom_run_mtx);
      working = working_flag;
      assert (!working
	      || (myindex > 0 && myindex <= MOM_MAX_WORKERS
		  && workers + myindex == wd));
      if (myindex == 1 && loopcnt % WORK_GARBAGE_COLLECTION_CHECK_PERIOD == 0)
	{
	  double nowtime = mom_clock_time (CLOCK_REALTIME);
	  if (lastgctime + WORK_GARBAGE_COLLECTION_DELAY < nowtime)
	    {
	      MOM_DEBUG (run, "work_loop loopcnt %ld before full GC",
			 loopcnt);
	      GC_gcollect ();
	      MOM_DEBUG (run, "work_loop loopcnt %ld after full GC", loopcnt);
	      lastgctime = mom_clock_time (CLOCK_REALTIME);
	    }
	}
      pthread_mutex_unlock (&mom_run_mtx);
      loopcnt++;
      MOM_DEBUG (run, "work_loop index %d, loopcnt=%ld, working=%d",
		 wd->work_index, loopcnt, (int) working);
      if (!working)
	break;
      long long tc = 0;
      pthread_mutex_lock (&mom_run_mtx);
      curtsk = mom_item_queue_pop_front ((momval_t) mom_item__agenda);
      MOM_DBG_ITEM (run, "work_loop curtsk=", curtsk);
      if (curtsk)
	work_tasklet_counter++;
      tc = work_tasklet_counter;
      pthread_mutex_unlock (&mom_run_mtx);
      if (curtsk)
	{
	  MOM_DEBUG (run, "work_loop taskletcounter %lld before step", tc);
	  mom_tasklet_step ((momit_tasklet_t *) curtsk);
	  MOM_DEBUG (run, "work_loop taskletcounter %lld after step", tc);
	  if (tc % MOM_MAX_WORKERS == 0)
	    GC_collect_a_little ();
	}
      else
	{
	  double curwtim = mom_clock_time (CLOCK_REALTIME);
	  MOM_DEBUG (run, "work_loop index %d no curtsk", wd->work_index);
	  pthread_mutex_lock (&mom_run_mtx);
	  struct timespec endts =
	    mom_timespec (curwtim + WORK_DELAY + (myindex * 0.03 + 0.01));
	  working = working_flag;
	  if (working)
	    pthread_cond_timedwait (&mom_run_changed_cond, &mom_run_mtx,
				    &endts);
	  pthread_mutex_unlock (&mom_run_mtx);
	  GC_collect_a_little ();
	}
    }
  while (working);
  MOM_DEBUG (run, "work_loop index %d ending", myindex);
  {
    pthread_mutex_lock (&mom_run_mtx);
    assert (!working_flag);
    MOM_DEBUG (run, "work_loop index %d clearing", myindex);
    wd->work_index = 0;
    pthread_mutex_unlock (&mom_run_mtx);
    pthread_cond_broadcast (&mom_run_changed_cond);
  }
  sched_yield ();
  MOM_DEBUG (run, "work_loop ended index %d", myindex);
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
    MOM_FATAL ("bad agenda");
  cur_worker = wd;
  MOMGC_CALL_WITH_STACK_BASE (work_loop, wd);
  cur_worker = NULL;
  return NULL;
}


void
mom_run_at (const char *srcfil, int srclin, const char *reason)
{
  bool restart_work = false;
  MOM_DEBUG (run, "mom_run %s:%d reason %s start", srcfil, srclin, reason);
  do
    {
      // start the work threads
      {
	pthread_mutex_lock (&mom_run_mtx);
	if (mom_nb_workers < MOM_MIN_WORKERS)
	  mom_nb_workers = MOM_MIN_WORKERS;
	else if (mom_nb_workers > MOM_MAX_WORKERS)
	  mom_nb_workers = MOM_MAX_WORKERS;
	assert (!working_flag);
	memset (workers, 0, sizeof (workers));
	working_flag = true;
	for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
	  {
	    workers[ix].work_magic = WORK_MAGIC;
	    workers[ix].work_index = ix;
	    pthread_create (&workers[ix].work_thread, NULL, work_cb,
			    workers + ix);
	  }
	pthread_mutex_unlock (&mom_run_mtx);
      }
      usleep (75000);
      MOM_DEBUG (run, "mom_run %s:%d reason %s joining %d workers",
		 srcfil, srclin, reason, mom_nb_workers);
      // join the work threads
      {
	for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
	  {
	    void *ret = NULL;
	    pthread_join (workers[ix].work_thread, &ret);
	    MOM_DEBUG (run, "mom_run %s:%d reason %s joined worker#%d",
		       srcfil, srclin, reason, ix);
	  }
      }
      // check that we are not working
      usleep (75000);
      {
	pthread_mutex_lock (&mom_run_mtx);
	if (working_flag)
	  MOM_FATAL ("corruption, working after run %s:%d reason %s",
		     srcfil, srclin, reason);
	pthread_mutex_unlock (&mom_run_mtx);
      }
      // run the post runner functions
      int nbpostrunner = 0;
      int nbfailpost = 0;
      {
	pthread_mutex_lock (&mom_run_mtx);
	for (int pix = 0; pix < MAX_POST_RUNNERS; pix++)
	  {
	    if (post_runner_funtab[pix])
	      {
		nbpostrunner++;
		int pr = post_runner_funtab[pix] (post_runner_datatab[pix]);
		if (pr <= 0)
		  nbfailpost++;
	      }
	  }
	pthread_mutex_unlock (&mom_run_mtx);
      }
      if (nbpostrunner > 0 && nbfailpost == 0)
	{
	  restart_work = 1;
	  MOM_DEBUG (run, "mom_run %s:%d reason %s restarting", srcfil,
		     srclin, reason);
	}
    }
  while (restart_work);
  MOM_DEBUG (run, "mom_run %s:%d reason %s ending", srcfil, srclin, reason);
}

void
mom_request_stop_at (const char *srcfil, int srcline, const char *reason,
		     mom_post_runner_sig_t * postrunner, void *clientdata)
{
  int nbworkers = 0;
  long stopcount = 0;
  MOM_DEBUG (run, "mom_request_stop start %s:%d reason %s", srcfil,
	     srcline, reason);
  // first register the postrunner
  if (postrunner)
    {
      int pix = -1;
      pthread_mutex_lock (&mom_run_mtx);
      for (unsigned rix = 0; rix < MAX_POST_RUNNERS && pix < 0; rix++)
	if (!post_runner_funtab[rix])
	  {
	    post_runner_funtab[rix] = postrunner;
	    post_runner_datatab[rix] = clientdata;
	    pix = rix;
	  }
      pthread_mutex_unlock (&mom_run_mtx);
      if (pix < 0)
	MOM_FATAL ("failed to register postrunner @%p", postrunner);
      MOM_DEBUG (run,
		 "request_stop reason %s registered pix %d", reason, pix);
    };
  do
    {
#define STOP_DELAY 1.5
      double stoptime = mom_clock_time (CLOCK_REALTIME) + STOP_DELAY;
      pthread_mutex_lock (&mom_run_mtx);
      stopcount++;
      MOM_DEBUG (run,
		 "mom_request_stop stopcount=%ld reason %s clearing working_flag",
		 stopcount, reason);
      working_flag = false;
      nbworkers = 0;
      for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
	if (workers[ix].work_magic == WORK_MAGIC
	    && workers[ix].work_index == ix && cur_worker != workers + ix)
	  nbworkers++;
      MOM_DEBUG (run, "mom_request_stop nbworkers=%d reason %s",
		 nbworkers, reason);
      int errwait = 0;
      if (nbworkers > 0)
	{
	  struct timespec stopts = mom_timespec (stoptime);
	  errwait =
	    pthread_cond_timedwait (&mom_run_changed_cond, &mom_run_mtx,
				    &stopts);
	}
      pthread_mutex_unlock (&mom_run_mtx);
      MOM_DEBUG (run, "nbworkers=%d, errwait#%d (%s) reason %s",
		 nbworkers, errwait, strerror (errwait), reason);
      usleep (500);
    }
  while (nbworkers > 0);
  MOM_DEBUG (run, "mom_request_stop end %s:%d reason %s", srcfil,
	     srcline, reason);
}


////////////////

static void
start_some_pending_jobs (void)
{
  int nbrunning = 0;
  int nbstarting = 0;
  momit_process_t *proctab[MOM_MAX_WORKERS] = { };
  memset (proctab, 0, sizeof (proctab));
  MOM_DEBUG (run, "start_some_pending_jobs");
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
  MOM_DEBUG (run, "start_some_pending_jobs nbstarting=%d, nbrunning=%d",
	     nbstarting, nbrunning);
  for (unsigned rix = 0; rix < nbstarting; rix++)
    {
      momit_process_t *curproc = proctab[rix];
      int pipetab[2] = { -1, -1 };
      int jobnum = -1;
      for (unsigned j = 1; j <= MOM_MAX_WORKERS && jobnum < 0; j++)
	if (!running_jobs[j])
	  jobnum = j;
      MOM_DBG_ITEM (run, "start_some_pending_jobs curproc=",
		    (mom_anyitem_t *) curproc);
      MOM_DEBUG (run, "start_some_pending_jobs jobnum=%d", jobnum);
      assert (curproc && curproc->iproc_item.typnum == momty_processitem);
      assert (curproc->iproc_pid <= 0 && curproc->iproc_outfd < 0);
      assert (jobnum > 0);
      pthread_mutex_lock (&curproc->iproc_item.i_mtx);
      const char *progname =
	mom_string_cstr ((momval_t) curproc->iproc_progname);
      const char **progargv =
	MOM_GC_ALLOC ("start_some_pending_jobs program argv",
		      (curproc->iproc_argcount + 2) * sizeof (char *));
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
      for (unsigned aix = 0; aix < argcnt; aix++)
	MOM_DEBUG (run, "run progargv[%d]=%s", aix, progargv[aix]);
      assert (progname != NULL);
      if (pipe (pipetab))
	MOM_FATAL ("failed to create pipe for process %s", progname);
      MOM_DEBUG (run, "pipetab={r:%d,w:%d}", pipetab[0], pipetab[1]);
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
	  sigset_t mysetsig;
	  sigemptyset (&mysetsig);
	  sigaddset (&mysetsig, SIGTERM);
	  sigaddset (&mysetsig, SIGQUIT);
	  sigaddset (&mysetsig, SIGPIPE);
	  sigaddset (&mysetsig, SIGCHLD);
	  sigprocmask (SIG_UNBLOCK, &mysetsig, NULL);
	  signal (SIGTERM, SIG_DFL);
	  signal (SIGQUIT, SIG_DFL);
	  signal (SIGCHLD, SIG_DFL);
	  execvp ((const char *) progname, (char *const *) progargv);
	  fprintf (stderr, "execution of %s failed : %s\n", progname,
		   strerror (errno));
	  fflush (NULL);
	  _exit (127);
	}
      else if (newpid < 0)
	MOM_FATAL ("fork failed for %s", progname);
      // parent process:
      MOM_DEBUG (run, "start_some_pending_jobs newpid=%d, jobnum=%d",
		 (int) newpid, jobnum);
      curproc->iproc_outfd = pipetab[0];
      curproc->iproc_pid = newpid;
      curproc->iproc_jobnum = jobnum;
      running_jobs[jobnum] = curproc;
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
  MOM_DEBUG (run, "event_loop_handler fd=%d", fd);
  assert (fd == event_loop_read_pipe && data);
  bool *prepeatloop = data;
  char rbuf[4];
  memset (rbuf, 0, sizeof (rbuf));
  if (read (fd, &rbuf, 1) > 0)
    {
      MOM_DEBUG (run, "event_loop_handler rbuf='%c'", rbuf[0]);
      switch (rbuf[0])
	{
	case EVLOOP_STOP:
	  *prepeatloop = false;
	  break;
	case EVLOOP_JOB:
	  start_some_pending_jobs ();
	  break;
	default:
	  MOM_FATAL ("unexpected loop command %c = %d", rbuf[0],
		     (int) rbuf[0]);
	}
    }
}


#define check_for_some_child_process() \
  check_for_some_child_process_at(__FILE__,__LINE__)
static void
check_for_some_child_process_at (const char *srcfil, int srclin)
{
  MOM_DEBUG (run, "check_for_some_child_process %s:%d", srcfil, srclin);
  int pst = 0;
  pid_t wpid = waitpid (-1, &pst, WNOHANG);
  if (wpid > 0)
    {
      momit_process_t *wproc = NULL;
      const momclosure_t *clos = NULL;
      const momstring_t *outstr = NULL;
      MOM_DEBUG (run, "check_for_some_child_process wpid=%d pst=%#x",
		 wpid, pst);
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
      MOM_DEBUG (run, "wpid %d, exitstatus=%d termsig=%d",
		 (int) wpid, exitstatus, termsig);
      if (wproc && (exitstatus >= 0 || termsig >= 0))
	{
	  extern momit_box_t *mom_item__exited;
	  extern momit_box_t *mom_item__terminated;
	  MOM_DBG_ITEM (run, "wproc=", (const mom_anyitem_t *) wproc);
	  pthread_mutex_lock (&wproc->iproc_item.i_mtx);
	  outstr =
	    mom_make_string_len (wproc->iproc_outbuf, wproc->iproc_outpos);
	  close (wproc->iproc_outfd);
	  wproc->iproc_outfd = -1;
	  GC_FREE (wproc->iproc_outbuf);
	  wproc->iproc_outbuf = NULL;
	  wproc->iproc_outsize = wproc->iproc_outpos = 0;
	  wproc->iproc_pid = 0;
	  pthread_mutex_unlock (&wproc->iproc_item.i_mtx);
	  // should run the closure in a new tasklet
	  {
	    momit_tasklet_t *protasklet = NULL;
	    if (exitstatus >= 0)
	      {
		protasklet = mom_make_item_tasklet (MOM_SPACE_NONE);
		mom_tasklet_push_frame ((momval_t) protasklet,
					(momval_t) clos,
					MOMPFR_THREE_VALUES, wproc,
					outstr, mom_item__exited,
					MOMPFR_INT, exitstatus, MOMPFR_END);
	      }
	    else if (termsig >= 0)
	      {
		protasklet = mom_make_item_tasklet (MOM_SPACE_NONE);
		mom_tasklet_push_frame ((momval_t) protasklet,
					(momval_t) clos,
					MOMPFR_THREE_VALUES, wproc,
					outstr, mom_item__terminated,
					MOMPFR_INT, termsig, MOMPFR_END);
	      };
	    if (protasklet)
	      {
		MOM_DBG_ITEM (run, "new protasklet=",
			      (const mom_anyitem_t *) protasklet);
		mom_agenda_add_tasklet_front ((momval_t) protasklet);
	      }
	  }
	}
    }
}


static int
terminating_dump (void *data)
{
  const char *filepath = data;
  MOM_DEBUG (run, "terminating_dump filepath=%s", filepath);
  mom_full_dump ("terminating dump after SIGTERM", filepath);
  MOM_INFORM ("dumped state on SIGTERM signal to %s", filepath);
  return 0;
}

static void
mysignalfd_handler (int fd, short revent, void *data)
{
  struct signalfd_siginfo sinf;
  memset (&sinf, 0, sizeof (sinf));
  MOM_DEBUG (run, "mysignalfd_handler fd=%d sizeof(sinf)=%d", fd,
	     (int) sizeof (sinf));
  assert (fd == my_signals_fd && !data);
  int rdsiz = -2;
  if (revent & POLLIN)
    {
      MOM_DEBUG (run, "mysignalfd_handler reading fd=%d", fd);
      rdsiz = read (fd, &sinf, sizeof (sinf));
    }
  MOM_DEBUG (run, "mysignalfd_handler fd=%d rdsiz=%d", fd, rdsiz);
  if (rdsiz < 0)
    {
      MOM_DEBUG (run, "mysignalfd_handler read failed (%s)",
		 strerror (errno));
      return;
    }
  MOM_DEBUG (run, "mysignalfd_handler signo=%d (%s)",
	     sinf.ssi_signo, strsignal (sinf.ssi_signo));
  switch (sinf.ssi_signo)
    {
    case SIGCHLD:
      check_for_some_child_process ();
      break;
    case SIGTERM:
      {
	char termpath[MOM_PATH_LEN];
	struct tm nowtm = { };
	time_t nowt = 0;
	char pidbuf[32];
	memset (termpath, 0, sizeof (termpath));
	time (&nowt);
	strftime (termpath, sizeof (termpath) - sizeof (pidbuf),
		  "_monimelt_sigterm_state_%d_%b_%Y_%Hh%M_",
		  localtime_r (&nowt, &nowtm));
	snprintf (pidbuf, sizeof (pidbuf), "pid%d.dbsqlite", (int) getpid ());
	strcat (termpath, pidbuf);
	char *termstr = GC_STRDUP (termpath);
	MOM_DEBUG (run, "recieved SIGTERM signal, will try dump to %s",
		   termstr);
	mom_request_stop ("mysignalfd_handler got SIGTERM", terminating_dump,
			  termstr);
	MOM_WARNING ("got SIGTERM, will try to dump to %s", termstr);
      }
      break;
    default:
      MOM_WARNING ("unexpected signal#%d : %s", sinf.ssi_signo,
		   sys_siglist[sinf.ssi_signo]);
      break;
    }
}

#define MOM_MAX_OUTPUT_LEN (1024*1024)	/* one megabyte */
static void
process_readout_handler (int fd, short revent, void *data)
{
#define PROCOUT_BUFSIZE 4096
  char rdbuf[PROCOUT_BUFSIZE];
  memset (rdbuf, 0, sizeof (rdbuf));
  int nbr = read (fd, rdbuf, sizeof (rdbuf));
  MOM_DEBUG (run, "process_readout_handler fd=%d, nbr=%d, rdbuf=%s\n",
	     fd, nbr, rdbuf);
  momit_process_t *procitm = data;
  assert (procitm && procitm->iproc_item.typnum == momty_processitem
	  && fd == procitm->iproc_outfd);
  if (nbr < 0)
    return;
  pthread_mutex_lock (&procitm->iproc_item.i_mtx);
  if (MOM_UNLIKELY
      (nbr + procitm->iproc_outpos + 1 >= procitm->iproc_outsize))
    {
      unsigned newsiz =
	((5 * (nbr + procitm->iproc_outpos) / 4 + 10) | 0x1f) + 1;
      char *newbuf =
	MOM_GC_SCALAR_ALLOC ("grown process output buffer", newsiz);
      char *oldbuf = procitm->iproc_outbuf;
      memcpy (newbuf, oldbuf, procitm->iproc_outpos);
      MOM_GC_FREE (oldbuf);
      procitm->iproc_outbuf = newbuf;
      procitm->iproc_outsize = newsiz;
    };
  memcpy (procitm->iproc_outbuf + procitm->iproc_outpos, rdbuf, nbr);
  procitm->iproc_outbuf[procitm->iproc_outpos + nbr] = (char) 0;
  procitm->iproc_outpos += nbr;
  if (procitm->iproc_outpos > MOM_MAX_OUTPUT_LEN)
    {
      close (procitm->iproc_outfd);	// the child process could later get a SIGPIPE if it writes more
      procitm->iproc_outfd = -1;
    }
  pthread_mutex_unlock (&procitm->iproc_item.i_mtx);
}

static void
curl_handler (int fd, short revent, void *data)
{
  MOM_DEBUG (run, "curl_handler fd=%d", fd);
  int *pnb = data;
  if (*pnb > 0)
    curl_multi_perform (&multicurl_job, pnb);
}

void
mom_initialize_signals (void)
{
  // set up the signalfd 
  sigset_t mysetsig;
  sigemptyset (&mysetsig);
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
    MOM_FATAL ("failed to block signals with sigprocmask");
  MOM_DEBUG (run, "before signalfd");
  my_signals_fd = signalfd (-1, &mysetsig, SFD_NONBLOCK | SFD_CLOEXEC);
  if (MOM_UNLIKELY (my_signals_fd < 0))
    MOM_FATAL ("signalfd failed");
  MOM_DEBUG (run, "my_signals_fd=%d", my_signals_fd);
}

static void *
event_loop (struct GC_stack_base *sb, void *data)
{
  long long evloopcnt = 0;
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
  assert (my_signals_fd >= 0);
  MOM_DEBUG (run, "event_loop start");
  // set up CURL multi handle
  assert (multicurl_job == NULL);
  multicurl_job = curl_multi_init ();
  if (MOM_UNLIKELY (!multicurl_job))
    MOM_FATAL ("failed to initial multi CURL handle");
  /// our event loop
  do
    {
      repeat_loop = true;
      evloopcnt++;
      MOM_DEBUG (run, "start eventloop evloopcnt=%lld", evloopcnt);
      GC_collect_a_little ();
      // check for ended process from time to time, to be safe
      if (evloopcnt % 8 == 0)
	check_for_some_child_process ();
      unsigned pollcnt = 0;
      memset (polltab, 0, sizeof (polltab));
      memset (handlertab, 0, sizeof (handlertab));
      memset (datatab, 0, sizeof (datatab));
      //
#define ADD_POLL(Ev,Fd,Hdr,Data) do {			\
    if (MOM_UNLIKELY(pollcnt>=MOM_POLL_MAX))		\
      MOM_FATAL("failed to poll fd#%d", (Fd));		\
    polltab[pollcnt].fd = (Fd);				\
    polltab[pollcnt].events = (Ev);			\
    MOM_DEBUG(run, "add_poll pollcnt=%d, fd=%d " #Hdr,	\
		   pollcnt, (Fd));			\
    handlertab[pollcnt] = Hdr;				\
    datatab[pollcnt]= (void*)(Data);			\
    pollcnt++; } while(0)
      //
      ADD_POLL (POLLIN, event_loop_read_pipe, event_loop_handler,
		&repeat_loop);
      ADD_POLL (POLLIN, my_signals_fd, mysignalfd_handler, NULL);
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
	      ADD_POLL (POLLIN, curproc->iproc_outfd,
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
		curlpollflags |= POLLIN;
	      if (FD_ISSET (curlfd, &outscurl))
		curlpollflags |= POLLOUT;
	      if (FD_ISSET (curlfd, &excscurl))
		curlpollflags |= POLLPRI;
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
      MOM_DEBUG (run, "before poll pollcnt=%d evloopcnt=%lld",
		 pollcnt, evloopcnt);
      int respoll = poll (polltab, pollcnt, MOM_POLL_TIMEOUT);
      MOM_DEBUG (run, "after poll respoll=%d evloopcnt=%lld", respoll,
		 evloopcnt);
      // invoke the handlers
      if (respoll > 0)
	{
	  for (int pix = 0; pix < pollcnt; pix++)
	    {
	      if (polltab[pix].revents && handlertab[pix])
		{
		  MOM_DEBUG (run,
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
		  MOM_DEBUG (run,
			     "done handler pix=%d fd#%d evloopcnt=%lld",
			     pix, polltab[pix].fd, evloopcnt);
		}
	    }
	  sched_yield ();
	}
      else
	{			// timed-out
	  MOM_DEBUG (run, "poll timed out evloopcnt=%lld", evloopcnt);
	  check_for_some_child_process ();
	  MOM_DEBUG (run, "poll done timed out evloopcnt=%lld", evloopcnt);
	};
    }
  while (repeat_loop);
  MOM_DEBUG (run, "end of eventloop evloopcnt=%lld", evloopcnt);
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
    MOM_FATAL ("failed to create event loop pipe");
  MOM_DEBUG (run, "event_loop_read_pipe=%d event_loop_write_pipe=%d",
	     event_loop_read_pipe, event_loop_write_pipe);
  if (pthread_create (&event_loop_thread, &evthattr, eventloop_cb, NULL))
    MOM_FATAL ("failed to create event loop thread");
  event_loop_started = true;
}


void
mom_process_destroy (mom_anyitem_t * itm)
{
  assert (itm && itm->typnum == momty_processitem);
  momit_process_t *procitm = (momit_process_t *) itm;
  if (procitm->iproc_pid > 0)
    {
      MOM_WARNING ("destroying running process %d for %s",
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
      argv =
	MOM_GC_ALLOC ("process argvals arguments",
		      nbargstr * sizeof (momstring_t *));
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
			       MOM_SPACE_NONE);
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
      argv =
	MOM_GC_ALLOC ("process arguments from array",
		      nbargstr * sizeof (momstring_t *));
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
			       MOM_SPACE_NONE);
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
	  argv =
	    MOM_GC_ALLOC ("process arguments from node",
			  nbargstr * sizeof (momstring_t *));
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
			       MOM_SPACE_NONE);
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
  MOM_DBG_VALUE (run, "process_start procv=", procv);
  MOM_DBG_VALUE (run, "process_start clov=", clov);
  if (!procv.ptr || *procv.ptype != momty_processitem
      || !clov.ptr || *clov.ptype != momty_closure)
    return;
  if (!event_loop_started)
    MOM_FATAL ("cannot start process without event loop");
  pthread_mutex_lock (&procv.panyitem->i_mtx);
  momit_process_t *procitm = procv.pprocessitem;
  const momclosure_t *clos = clov.pclosure;
  if (procitm->iproc_closure)	// already started
    goto end;
  procitm->iproc_outbuf =
    MOM_GC_SCALAR_ALLOC ("process output initial", OUTPROC_INITIAL_SIZE);
  procitm->iproc_outsize = OUTPROC_INITIAL_SIZE;
  procitm->iproc_outpos = 0;
  procitm->iproc_closure = clos;
  {
    struct mom_itqueue_st *qel =
      MOM_GC_ALLOC ("job item queue element", sizeof (struct mom_itqueue_st));
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


int
mom_load_code_post_runner (const char *modname)
{
  GModule *codmodu = NULL;
  char pathbuf[MOM_PATH_LEN];
  char symname[MOM_SYMBNAME_LEN];
  int foundnamecount = 0;
  typedef void after_code_load_sig_t (const char *modname);
  after_code_load_sig_t *aftercodeloadfun = NULL;
  memset (pathbuf, 0, sizeof (pathbuf));
  memset (symname, 0, sizeof (symname));
  snprintf (pathbuf, sizeof (pathbuf), "./%s.%s", modname, G_MODULE_SUFFIX);
  MOM_DEBUG (run, "mom_load_code_post_runner pathbuf=%s", pathbuf);
  if (access (pathbuf, R_OK))
    {
      MOM_WARNING ("fail to access code module %s", pathbuf);
      return 0;
    }
  codmodu = g_module_open (pathbuf, 0);
  if (!codmodu)
    MOM_FATAL ("failed to load code module %s: %s", pathbuf,
	       g_module_error ());
  MOM_DEBUG (run, "mom_load_code_post_runner opened module pathbuf=%s",
	     pathbuf);
  pthread_mutex_lock (&embryonic_mtx);
  if (g_module_symbol
      (codmodu, "mom_after_code_load", (gpointer *) & aftercodeloadfun)
      && aftercodeloadfun)
    MOM_DEBUG (run,
	       "mom_load_code_post_runner got mom_after_code_load@%p in %s",
	       (void *) aftercodeloadfun, pathbuf);
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
      MOM_DEBUG (run, "symname=%s eix=%d", symname, eix);
      struct momroutinedescr_st *rdescr = NULL;
      if ((g_module_symbol
	   (codmodu, symname, (gpointer *) & rdescr)
	   || g_module_symbol
	   (mom_prog_module, symname, (gpointer *) & rdescr)) && rdescr)
	{
	  if (rdescr->rout_magic != ROUTINE_MAGIC || !rdescr->rout_code
	      || strcmp (rdescr->rout_name, curname))
	    MOM_FATAL ("bad routine descriptor %s", symname);
	  curout->irt_descr = rdescr;
	  foundnamecount++;
	}
      pthread_mutex_unlock (&curout->irt_item.i_mtx);
    }
  if (aftercodeloadfun != NULL)
    {
      MOM_DEBUG (run, "before aftercodeloadfun %s", modname);
      aftercodeloadfun (modname);
      MOM_DEBUG (run, "after aftercodeloadfun %s", modname);
    }
  else
    MOM_DEBUG (run, "no aftercodeloadfun in %s", modname);
  pthread_mutex_unlock (&embryonic_mtx);
  MOM_INFORM ("loaded code module %s and found %d embryonic routines",
	      pathbuf, foundnamecount);
  usleep (500);
  return 1;
}


const char *
mom_embryonic_routine_name (momit_routine_t * itrout)
{
  const char *res = NULL;
  if (!itrout || itrout->irt_item.typnum != momty_routineitem)
    return NULL;
  if (itrout->irt_descr != NULL)
    return NULL;
  pthread_mutex_lock (&embryonic_mtx);
  for (unsigned eix = 0; eix < embryonic_routine_size; eix++)
    {
      const char *curname = embryonic_routine_name[eix];
      momit_routine_t *curout = embryonic_routine_arr[eix];
      if (!curname || !curout)
	continue;
      if (curout == itrout)
	{
	  res = GC_STRDUP (curname);
	  break;
	}
    }
  pthread_mutex_unlock (&embryonic_mtx);
  return res;
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
    MOM_FATAL ("failed to duplicate name %s", name);
  pthread_mutex_lock (&embryonic_mtx);
  for (int j = 0; j < (int) embryonic_routine_size && eix < 0; j++)
    if (!embryonic_routine_arr[j] || embryonic_routine_arr[j] == itrout)
      eix = j;
  if (MOM_UNLIKELY (eix < 0))
    {
      unsigned newsiz = ((3 * embryonic_routine_size / 2 + 10) | 0x1f) + 1;
      char **newnames =
	MOM_GC_ALLOC ("new embryonic names", newsiz * sizeof (char *));
      momit_routine_t **newarr = MOM_GC_ALLOC ("new embryonic routines",
					       newsiz *
					       sizeof (momit_routine_t *));
      if (embryonic_routine_size > 0)
	{
	  memcpy (newnames, embryonic_routine_name,
		  embryonic_routine_size * sizeof (char *));
	  memcpy (newarr, embryonic_routine_arr,
		  embryonic_routine_size * sizeof (momit_routine_t *));
	};
      MOM_GC_FREE (embryonic_routine_name);
      embryonic_routine_name = (const char **) newnames;
      MOM_GC_FREE (embryonic_routine_arr);
      embryonic_routine_arr = newarr;
      eix = embryonic_routine_size;
      embryonic_routine_size = newsiz;
    }
  embryonic_routine_name[eix] = dupname;
  embryonic_routine_arr[eix] = itrout;
  pthread_mutex_unlock (&embryonic_mtx);
}
