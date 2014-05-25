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


void
mom_run_workers (void)
{
  bool again = false;
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
	  if (GC_pthread_join
	      (&work_data_array_mom[ix].work_thread, &retwork))
	    MOM_FATAPRINTF ("failed to join work thread #%d", ix);
	}
      MOM_DEBUGPRINTF (run, "mom_start_workers joined %d work threads",
		       curnbwork);
      {
	pthread_mutex_lock (&mom_named__agenda->i_mtx);
	for (unsigned dix = 0; dix < TODO_MAX_MOM; dix++)
	  {
	    if (todo_after_stop_mom[dix].todo_fun)
	      todo_after_stop_mom[dix].todo_fun
		(todo_after_stop_mom[dix].todo_data);
	  }
	memset (todo_after_stop_mom, 0, sizeof (todo_after_stop_mom));
	if (stop_working_mom)
	  again = false;
	if (continue_working_mom)
	  again = true;
	continue_working_mom = false;
	pthread_mutex_unlock (&mom_named__agenda->i_mtx);
      }
    }
  while (again);
}
