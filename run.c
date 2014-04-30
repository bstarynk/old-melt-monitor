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
  pthread_cond_broadcast (&mom_run_changed_cond);
}

static void *
work_loop (struct GC_stack_base *sb, void *data)
{
  struct momworkdata_st *wd = data;
  GC_register_my_thread (sb);
  assert (wd != NULL);
  mom_anyitem_t *curtsk = NULL;
  bool working = false;
  do
    {
      curtsk = NULL;
      pthread_mutex_lock (&mom_run_mtx);
      working = working_flag;
      pthread_mutex_unlock (&mom_run_mtx);
      if (working)
	curtsk = mom_item_queue_pop_front ((momval_t) mom_item__agenda);
      if (curtsk)
	{
	  if (MONIMELT_UNLIKELY (curtsk->typnum != momty_taskletitem))
	    MONIMELT_FATAL ("invalid current task");
	  mom_tasklet_step ((momit_tasklet_t *) curtsk);
	}
      else
	{
	  pthread_mutex_lock (&mom_run_mtx);
	  working = working_flag;
	  if (working)
	    pthread_cond_wait (&mom_run_changed_cond, &mom_run_mtx);
	  pthread_mutex_unlock (&mom_run_mtx);
	}
    }
  while (working);
  pthread_mutex_lock (&mom_run_mtx);
  cur_worker->work_index = 0;
  pthread_mutex_unlock (&mom_run_mtx);
  pthread_cond_broadcast (&mom_run_changed_cond);
  GC_unregister_my_thread ();
  return NULL;
}

static void *
work_cb (void *ad)
{
  struct momworkdata_st *wd = ad;
  {
    char thnambuf[24];
    memset (thnambuf, 0, sizeof (thnambuf));
    snprintf (thnambuf, sizeof (thnambuf), "monimelt-r%02d", wd->work_index);
    pthread_setname_np (pthread_self (), thnambuf);
  }
  assert (wd && wd->work_magic == WORK_MAGIC);
  if (!mom_item__agenda
      || ((mom_anyitem_t *) mom_item__agenda)->typnum != momty_queueitem)
    MONIMELT_FATAL ("bad agenda");
  cur_worker = wd;
  GC_call_with_stack_base (work_loop, wd);
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
}

void
mom_stop (void)
{
  int nbworkers = 0;
  do
    {
      pthread_mutex_lock (&mom_run_mtx);
      working_flag = false;
      for (unsigned ix = 1; ix <= mom_nb_workers; ix++)
	if (workers[ix].work_magic == WORK_MAGIC
	    && workers[ix].work_index == ix)
	  nbworkers++;
      if (nbworkers > 0)
	pthread_cond_wait (&mom_run_changed_cond, &mom_run_mtx);
      pthread_mutex_unlock (&mom_run_mtx);
      sched_yield ();
    }
  while (nbworkers > 0);
}


static pthread_t event_loop_thread;

static int event_loop_pipe[2] = { -1, -1 };

static int my_signals_fd = -1;
#define event_loop_read_pipe event_loop_pipe[0]
#define event_loop_write_pipe event_loop_pipe[1]

#define EVLOOP_STOP '.'

void
mom_stop_event_loop (void)
{
  char buf[4] = { EVLOOP_STOP, 0, 0, 0 };
  write (event_loop_write_pipe, buf, 1);
}

static void *
event_loop (struct GC_stack_base *sb, void *data)
{
  extern momit_box_t *mom_item__heart_beat;
  GC_register_my_thread (sb);
  assert (data == NULL);
  assert (mom_item__heart_beat != NULL);
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
  // should set up the child processes
#warning missing code inside event_loop
  MONIMELT_WARNING ("event_loop not implemented");
  GC_unregister_my_thread ();
  return NULL;
}


static void *
eventloop_cb (void *p)
{
  pthread_setname_np (pthread_self (), "monimelt-evloop");
  GC_call_with_stack_base (event_loop, p);
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
