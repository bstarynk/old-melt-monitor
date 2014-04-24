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

static void
work_loop ()
{
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

}

static void *
work_cb (void *ad)
{
  struct GC_stack_base sb;
  memset (&sb, 0, sizeof (sb));
  struct momworkdata_st *wd = ad;
  assert (wd && wd->work_magic == WORK_MAGIC);
  GC_register_my_thread (&sb);
  if (!mom_item__agenda
      || ((mom_anyitem_t *) mom_item__agenda)->typnum != momty_queueitem)
    MONIMELT_FATAL ("bad agenda");
  cur_worker = wd;
  work_loop ();
  memset (wd, 0, sizeof (struct momworkdata_st));
  cur_worker = NULL;
  GC_unregister_my_thread ();
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
