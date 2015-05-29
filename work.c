// file work.c - working loop, web & socket

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

static volatile atomic_bool stop_work_loop_mom;
_Thread_local int mom_worker_num;

static pthread_t worker_threads_mom[MOM_MAX_WORKERS + 2];

bool
mom_should_stop (void)
{
  return atomic_load (&stop_work_loop_mom);
}

void
mom_stop_work (void)
{
  atomic_store (&stop_work_loop_mom, true);
  MOM_DEBUGPRINTF (run, "stop working");
}

static void *
work_run_mom (void *p)
{
  char wbuf[16];
  memset (wbuf, 0, sizeof (wbuf));
  intptr_t ix = (intptr_t) p;
  assert (ix > 0 && ix <= mom_nb_workers);
  snprintf (wbuf, sizeof (wbuf), "momwork#%d", (int) ix);
  pthread_setname_np (pthread_self (), wbuf);
  long count = 0;
  mom_worker_num = ix;
  while (!mom_should_stop ())
    {
      count++;
      MOM_DEBUGPRINTF (run, "work_run count#%ld before step", count);
      if (MOM_UNLIKELY (!momhook_agenda_step ()))
	MOM_WARNPRINTF ("agenda_step failed count#%ld", count);
    }
  return NULL;
}				/* end work_run_mom */



static void
start_workers_mom (void)
{
  MOM_DEBUGPRINTF (run, "start_workers nbworkers=%d", mom_nb_workers);
  for (unsigned ix = 1; ix <= (unsigned) mom_nb_workers; ix++)
    {
      int err = 0;
      if ((err =
	   pthread_create (&worker_threads_mom[ix], (pthread_attr_t *) NULL,
			   work_run_mom, (void *) ((intptr_t) ix))) != 0)
	MOM_FATAPRINTF ("pthread_create for worker#%d failed (%d = %s)", ix,
			err, strerror (err));
      MOM_DEBUGPRINTF (run, "start_workers worker#%u thread = %ld", ix,
		       (long) worker_threads_mom[ix]);
      usleep (1000);
    }
}				/* end of start_workers_mom */


static void
join_workers_mom (void)
{
  MOM_DEBUGPRINTF (run, "join_workers nbworkers=%d", mom_nb_workers);
  for (unsigned ix = 1; ix <= (unsigned) mom_nb_workers; ix++)
    {
      int err = 0;
      void *ret = NULL;
      MOM_DEBUGPRINTF (run, "join_workers before joining worker#%d",
		       (int) ix);
      if ((err = pthread_join (worker_threads_mom[ix], &ret)) != 0)
	MOM_FATAPRINTF ("pthread_join for worker#%d failed (%d = %s)",
			ix, err, strerror (err));
    };
}				/* end of join_workers_mom */

void
mom_run_workers (void)
{
  MOM_INFORMPRINTF ("start run %d workers (webhost %s, socket %s)",
		    mom_nb_workers, mom_web_host, mom_socket);
  if (MOM_UNLIKELY (mom_nb_workers < MOM_MIN_WORKERS
		    || mom_nb_workers > MOM_MAX_WORKERS))
    MOM_FATAPRINTF ("invalid mom_nb_workers %d", mom_nb_workers);
  start_workers_mom ();
  sched_yield ();
#warning should start the webservice, the socketservice, handle signals & timers
  join_workers_mom ();
  assert (mom_should_stop ());
  MOM_INFORMPRINTF ("done running %d workers", mom_nb_workers);
}				/* end of mom_run_workers */
