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

typedef void mom_poll_handler_t (int fd, void *data1, intptr_t data2);
struct process_mom_st
{
  pid_t mproc_pid;
  int mproc_out;
  momitem_t *mproc_itm;
  momvalue_t mproc_handlerv;
};

static struct process_mom_st workingprocesses_mom[MOM_MAX_WORKERS + 1];
static int nb_processes_mom;
static struct momqueuevalues_st pendingprocque_mom;

struct socket_mom_st
{
  int msock_fd;
  momitem_t *msock_itm;
  momvalue_t msock_handlerv;
};

#define MOM_MAX_SOCKETS (2*MOM_MAX_WORKERS+1)
static struct socket_mom_st sockets_mom[MOM_MAX_SOCKETS];
int nb_sockets_mom;

#define MAX_POLL_MOM (MOM_MAX_SOCKETS+MOM_MAX_WORKERS+10)

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


/// called from mom_run_workers in work.c
void
mom_event_loop (void)
{
  int evpipe[2] = { -1, -1 };
  MOM_DEBUGPRINTF (run, "start mom_event_loop");
  if (MOM_UNLIKELY (pipe (evpipe) || evpipe[0] <= 0 || evpipe[1] <= 0))
    MOM_FATAPRINTF ("failed to create pipe for event loop");
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
  signalfd_mom = signalfd (-1, &mysigmasks, SFD_CLOEXEC);
  if (signalfd_mom < 0)
    MOM_FATAPRINTF ("event_loop failed to signalfd (%m)");
  timerfd_mom = timerfd_create (CLOCK_REALTIME, TFD_CLOEXEC);
  if (timerfd_mom < 0)
    MOM_FATAPRINTF ("event_loop failed to timerfd_create (%m)");
  if (mom_socket_path && mom_socket_path[0])
    mastersocketfd_mom = open_bind_socket_mom ();
  while (!mom_should_stop ())
    {
      int nbpoll = 0;
      struct pollfd pollarr[MAX_POLL_MOM];
      struct
      {
	mom_poll_handler_t *polld_hdlr;
	void *polld_data1;
	intptr_t polld_data2;
      } polldata[MAX_POLL_MOM];
      memset (pollarr, 0, sizeof (pollarr));
      memset (polldata, 0, sizeof (polldata));
      {
	pthread_mutex_lock (&eventloop_mtx_mom);
#define DO_POLL(Fd,Event,Handler,Data1,Data2) do {	\
      pollarr[nbpoll].fd = (Fd);			\
      pollarr[nbpoll].event = (Event);			\
      polldata[nbpoll].polld_hdlr = (Handler);		\
      polldata[nbpoll].polld_data1 = (Data1);		\
      polldata[nbpoll].polld_data2 = (Data2);		\
      nbpoll++;						\
    } while(0)
	MOM_FATAPRINTF ("incomplete mom_event_loop");
#warning incomplete mom_event_loop
	pthread_mutex_unlock (&eventloop_mtx_mom);
      }
    };
}				/* end of mom_event_loop */
