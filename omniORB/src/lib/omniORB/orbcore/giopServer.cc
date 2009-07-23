// -*- Mode: C++; -*-
//                            Package   : omniORB
// giopServer.cc              Created on: 20 Dec 2000
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.22.2.47  2006/04/18 10:03:14  dgrisby
  Additional shutdown logging.

  Revision 1.22.2.46  2005/11/20 15:33:03  dgrisby
  New connection cleanup meant some connection state was not completely
  deleted.

  Revision 1.22.2.45  2005/11/17 11:35:52  dgrisby
  In thread pool mode, bidir workers become wedged waiting for incoming
  calls, so they must be closed in giopServer::deactivate.

  Revision 1.22.2.44  2005/11/16 17:35:25  dgrisby
  New connectionWatchPeriod and connectionWatchImmediate parameters.

  Revision 1.22.2.43  2005/11/15 11:07:56  dgrisby
  More shutdown cleanup.

  Revision 1.22.2.42  2005/11/14 15:20:08  dgrisby
  Race between giopServer destruction and closure of connections in
  thread pool mode.

  Revision 1.22.2.41  2005/11/14 10:58:23  dgrisby
  Better connection / thread shutdown behaviour.

  Revision 1.22.2.40  2005/11/04 14:26:02  dgrisby
  Open connections would prevent shutdown on Windows.

  Revision 1.22.2.39  2005/10/13 11:38:16  dgrisby
  Dump CloseConnection messages.

  Revision 1.22.2.38  2005/05/03 10:10:48  dgrisby
  Avoid deadlock caused by trying to deactivation SSL rendezvouser more
  than once.

  Revision 1.22.2.37  2005/04/10 22:17:19  dgrisby
  Fixes to connection management. Thanks Jon Biggar.

  Revision 1.22.2.36  2005/03/10 11:28:29  dgrisby
  Race condition between setSelectable / clearSelectable.

  Revision 1.22.2.35  2005/02/23 12:27:31  dgrisby
  Another race in setSelectable with connection shutdown. Thanks Peter
  Klotz.

  Revision 1.22.2.34  2005/02/13 20:52:51  dgrisby
  Change threadPoolWatchConnection parameter to be an integer rather
  than a boolean. Value is the count of threads that can be handling a
  connection when one decides to watch it.

  Revision 1.22.2.33  2004/12/20 18:17:55  dgrisby
  Avoid race condition on connection disconnection.

  Revision 1.22.2.32  2004/10/17 20:14:32  dgrisby
  Updated support for OpenVMS. Many thanks to Bruce Visscher.

  Revision 1.22.2.31  2004/09/13 10:03:13  dgrisby
  Fix for giopServer shutdown issue. Thanks Serguei Kolos.

  Revision 1.22.2.30  2004/04/08 10:02:20  dgrisby
  In thread pool mode, close connections that will not be selectable.

  Revision 1.22.2.29  2004/03/02 15:32:07  dgrisby
  Fix locking in connection shutdown.

  Revision 1.22.2.28  2004/02/27 12:40:09  dgrisby
  Fix race conditions in connection shutdown. Thanks Serguei Kolos.

  Revision 1.22.2.27  2003/10/20 16:11:12  dgrisby
  Race condition in connection closure.

  Revision 1.22.2.26  2003/07/16 14:22:38  dgrisby
  Speed up oneway handling a little. More tracing for split messages.

  Revision 1.22.2.25  2003/05/22 13:47:40  dgrisby
  Failed to setSelectable in some cases.

  Revision 1.22.2.24  2003/02/17 01:46:23  dgrisby
  Pipe to kick select thread (on Unix).

  Revision 1.22.2.23  2002/09/09 22:11:50  dgrisby
  SSL transport cleanup even if certificates are wrong.

  Revision 1.22.2.22  2002/09/04 23:29:30  dgrisby
  Avoid memory corruption with multiple list removals.

  Revision 1.22.2.21  2002/08/21 06:23:15  dgrisby
  Properly clean up bidir connections and ropes. Other small tweaks.

  Revision 1.22.2.20  2002/03/18 16:50:18  dpg1
  New threadPoolWatchConnection parameter.

  Revision 1.22.2.19  2002/03/13 16:05:39  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.22.2.18  2002/02/13 16:02:39  dpg1
  Stability fixes thanks to Bastiaan Bakker, plus threading
  optimisations inspired by investigating Bastiaan's bug reports.

  Revision 1.22.2.17  2002/02/01 11:21:19  dpg1
  Add a ^L to comments.

  Revision 1.22.2.16  2001/09/20 13:26:14  dpg1
  Allow ORB_init() after orb->destroy().

  Revision 1.22.2.15  2001/09/19 17:26:49  dpg1
  Full clean-up after orb->destroy().

  Revision 1.22.2.14  2001/09/10 17:47:57  sll
  startIdleCounter in csInsert.

  Revision 1.22.2.13  2001/08/21 11:02:15  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.22.2.12  2001/08/17 17:12:37  sll
  Modularise ORB configuration parameters.

  Revision 1.22.2.11  2001/07/31 16:28:00  sll
  Added GIOP BiDir support.

  Revision 1.22.2.10  2001/07/13 15:27:42  sll
  Support for the thread-pool as well as the thread-per-connection policy
  Support for serving multiple incoming calls on the same connection
  simultaneously.

  Revision 1.22.2.9  2001/06/20 18:35:17  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.22.2.8  2001/06/11 18:00:52  sll
  Fixed silly mistake in shutdown multiple endpoints.

  Revision 1.22.2.7  2001/04/18 18:10:49  sll
  Big checkin with the brand new internal APIs.


*/

#include <omniORB4/CORBA.h>
#include <invoker.h>
#include <giopServer.h>
#include <giopWorker.h>
#include <giopRendezvouser.h>
#include <giopMonitor.h>
#include <giopStrand.h>
#include <giopStream.h>
#include <giopStreamImpl.h>
#include <initialiser.h>
#include <omniORB4/omniInterceptors.h>
#include <orbOptions.h>
#include <orbParameters.h>

OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////
CORBA::Boolean orbParameters::threadPerConnectionPolicy      = 1;
//   1 means the ORB should dedicate one thread per connection on the 
//   server side. 0 means the ORB should dispatch a thread from a pool
//   to a connection only when a request has arrived.
//
//  Valid values = 0 or 1

CORBA::ULong   orbParameters::threadPerConnectionUpperLimit  = 10000;
//   If the one thread per connection is in effect, this number is
//   the max. no. of connections the server will allow before it
//   switch off the one thread per connection policy and move to
//   the thread pool policy.
//
//   Valid values = (n >= 1) 

CORBA::ULong   orbParameters::threadPerConnectionLowerLimit  = 9000;
//   If the one thread per connection was in effect and was switched
//   off because threadPerConnectionUpperLimit has been exceeded
//   previously, this number tells when the policy should be restored
//   when the number of connections drop.
//
//   Valid values = (n >= 1 && n < threadPerConnectionUpperLimit) 

CORBA::ULong   orbParameters::maxServerThreadPerConnection   = 100;
//   The max. no. of threads the server will dispatch to server the
//   requests coming from one connection.
//
//   Valid values = (n >= 1) 

CORBA::ULong   orbParameters::maxServerThreadPoolSize        = 100;
//   The max. no. of threads the server will allocate to do various
//   ORB tasks. This number does not include the dedicated thread
//   per connection when the threadPerConnectionPolicy is in effect
//
//   Valid values = (n >= 1) 

CORBA::ULong   orbParameters::threadPoolWatchConnection      = 1;
//   After dispatching an upcall in thread pool mode, the thread that
//   has just performed the call can watch the connection for a short
//   time before returning to the pool. This leads to less thread
//   switching for a series of calls from a single client, but is less
//   fair if there are concurrent clients. The connection is watched
//   if the number of threads concurrently handling the connection is
//   <= the value of this parameter. i.e. if the parameter is zero,
//   the connection is never watched; if it is 1, the last thread
//   managing a connection watches it; if 2, the connection is still
//   watched if there is one other thread still in an upcall for the
//   connection, and so on.
//
//  Valid values = (n >= 0)

CORBA::Boolean orbParameters::connectionWatchImmediate      = 0;
//   When a thread handles an incoming call, it unmarshals the
//   arguments then marks the connection as watchable by the connection
//   watching thread, in case the client sends a concurrent call on the
//   same connection. If this parameter is set to the default false,
//   the connection is not actually watched until the next connection
//   watch period (determined by the connectionWatchPeriod parameter).
//   If connectionWatchImmediate is set true, the connection watching
//   thread is immediately signalled to watch the connection. That
//   leads to faster interactive response to clients that multiplex
//   calls, but adds significant overhead along the call chain.
//
//   Note that this setting has no effect on Windows, since it has no
//   mechanism for signalling the connection watching thread.


////////////////////////////////////////////////////////////////////////////
static const char* plural(CORBA::ULong val)
{
  return val == 1 ? "" : "s";
}


////////////////////////////////////////////////////////////////////////////
giopServer::giopServer() : pd_state(IDLE), pd_nconnections(0),
			   pd_cond(&pd_lock),
			   pd_n_temporary_workers(0),
			   pd_n_dedicated_workers(0)
{
  pd_thread_per_connection = orbParameters::threadPerConnectionPolicy;
  pd_connectionState = new connectionState*[connectionState::hashsize];
  for (CORBA::ULong i=0; i < connectionState::hashsize; i++) {
    pd_connectionState[i] = 0;
  }
}

////////////////////////////////////////////////////////////////////////////
giopServer::~giopServer()
{
  singleton() = 0;
  delete [] pd_connectionState;
}


////////////////////////////////////////////////////////////////////////////
const char*
giopServer::instantiate(const char* uri,
			CORBA::Boolean no_publish,
			CORBA::Boolean no_listen)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  if (!no_listen) {

    giopEndpoint* ept = giopEndpoint::str2Endpoint(uri);
    if (!ept) return 0;

    OMNIORB_ASSERT(pd_state != ZOMBIE);

    if (ept->Bind()) {
      pd_endpoints.push_back(ept);
      if (pd_state == ACTIVE) activate();
      uri =  ept->address();
    }
    else {
      delete ept;
      return 0;
    }
  }
  else if ( !giopEndpoint::strIsValidEndpoint(uri) ) {
    return 0;
  }

  if (!no_publish) {
    (void)giopEndpoint::addToIOR(uri);
  }
  return uri;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopServer::addBiDirStrand(giopStrand* s,giopActiveCollection* watcher) {

  OMNIORB_ASSERT(s->isClient() && s->biDir && s->connection);
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
    omni_tracedmutex_lock sync(*omniTransportLock);
    s->connection->incrRefCount();
  }

  CORBA::Boolean status = 0;

  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
    omni_tracedmutex_lock sync(pd_lock);

    ensureNotInFlux();

    if (pd_state == ACTIVE) {

      pd_bidir_strands.push_back(s);

      CORBA::Boolean matched = 0;
      {
	omnivector<giopActiveCollection*>::iterator i = pd_bidir_collections.begin();
	omnivector<giopActiveCollection*>::iterator last = pd_bidir_collections.end();
	while (i != last) {
	  if ((*i) == watcher) {
	    matched = 1;
	    break;
	  }
	  ++i;
	}
      }
      if (!matched) {
	Link* p = pd_bidir_monitors.next;
	for (; p != &pd_bidir_monitors; p = p->next) {
	  if (((giopMonitor*)p)->collection() == watcher) {
	    matched = 1;
	    break;
	  }
	}
      }
      if (!matched) {
	pd_bidir_collections.push_back(watcher);
      }
      activate();

      status = 1;
    }
  }

  if (!status) {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
    omni_tracedmutex_lock sync(*omniTransportLock);
    s->connection->decrRefCount();
  }
  return status;
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::start()
{
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  switch (pd_state) {
  case IDLE:
    {
      pd_state = ACTIVE;
      activate();
    }
    break;
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::stop()
{
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  switch (pd_state) {
  case ACTIVE:
    {
      deactivate();
    }
    break;
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::remove()
{
  omni_tracedmutex_lock sync(pd_lock);

  ensureNotInFlux();

  switch (pd_state) {
  case ACTIVE:
    {
      deactivate();
    }
  case IDLE:
    {
      pd_state = ZOMBIE;
    }
    break;
  default:
    return;
  }

  giopEndpointList::iterator i;
  i = pd_endpoints.begin();

  while (i != pd_endpoints.end()) {
    (*i)->Shutdown();
    pd_endpoints.erase(i);
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::activate()
{
  // Caller is holding pd_lock
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);

  giopEndpointList::iterator i;
  i    = pd_endpoints.begin();

  while (i != pd_endpoints.end()) {
    giopRendezvouser* task = new giopRendezvouser(*i,this);

    if (!orbAsyncInvoker->insert(task)) {
      // Cannot start serving this endpoint.
      // Leave it in pd_endpoints.
      // Do not raise an exception. Instead, just moan about it.
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Cannot create a rendezvouser for this endpoint: ";
	log << (*i)->address();
	log << "\n";
      }
      delete task;
      i++;
      continue;
    }
    pd_endpoints.erase(i);

    task->insert(pd_rendezvousers);
  }

  omnivector<giopStrand*>::iterator j;
  j = pd_bidir_strands.begin();

  while (j != pd_bidir_strands.end()) {

    giopStrand* g = *j;

    pd_bidir_strands.erase(j);

    connectionState* cs = csInsert(g);

    if (cs->connection->pd_has_dedicated_thread) {
      giopWorker* task = new giopWorker(cs->strand,this,0);
      if (!orbAsyncInvoker->insert(task)) {
	// Cannot start serving this new connection.
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Cannot create a worker for this bidirectional connection: "
	      << " to "
	      << cs->connection->peeraddress()
	      << "\n";
	}
	delete task;
	cs->connection->Shutdown();
	csRemove(cs->connection);
	pd_lock.unlock();
	delete cs;
	pd_lock.lock();
	continue;
      }
      task->insert(cs->workers);
      cs->connection->pd_n_workers++;
      pd_n_dedicated_workers++;
    }
    else {
      pd_lock.unlock();
      cs->connection->setSelectable(1);
      pd_lock.lock();
    }
  }

  {
    omnivector<giopActiveCollection*>::iterator i;
    i = pd_bidir_collections.begin();

    while (i != pd_bidir_collections.end()) {
      giopMonitor* task = new giopMonitor(*i,this);

      if (!orbAsyncInvoker->insert(task)) {
	// Cannot start serving this collection.
	// Do not raise an exception. Instead, just moan about it.
	if (omniORB::trace(1)) {
	  omniORB::logger log;
	  log << "Cannot create a monitor for this bidir collection type: ";
	  log << (*i)->type();
	  log << "\n";
	}
	delete task;
      }
      else {
	task->insert(pd_bidir_monitors);
      }
      pd_bidir_collections.erase(i);
    }
  }
}

////////////////////////////////////////////////////////////////////////
static inline void
sendCloseConnection(giopStrand* s)
{
  // Send close connection message.
  char hdr[12];
  hdr[0] = 'G'; hdr[1] = 'I'; hdr[2] = 'O'; hdr[3] = 'P';
  hdr[4] = s->version.major;   hdr[5] = s->version.minor;
  hdr[6] = _OMNIORB_HOST_BYTE_ORDER_;
  hdr[7] = (char)GIOP::CloseConnection;
  hdr[8] = hdr[9] = hdr[10] = hdr[11] = 0;

  if (omniORB::trace(25)) {
    omniORB::logger log;
    log << "sendCloseConnection: to " << s->connection->peeraddress()
	<< " 12 bytes\n";
  }
  if (omniORB::trace(30))
    giopStream::dumpbuf((unsigned char*)hdr, 12);

  s->connection->Send(hdr,12);
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::deactivate()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 1);

  OMNIORB_ASSERT(pd_state == ACTIVE);

  pd_state = INFLUX;

  CORBA::Boolean waitforcompletion;
  CORBA::Boolean rendezvousers_terminated = 0;

  omniORB::logs(25, "giopServer deactivate...");

  unsigned long s, ns;

  unsigned long timeout;
  if (orbParameters::scanGranularity)
    timeout = orbParameters::scanGranularity;
  else
    timeout = 5;

  if (pd_nconnections) {
    omniORB::logs(25, "Close connections with threads and monitors...");

    CORBA::ULong closed_count = 0;

    for (CORBA::ULong i=0; i < connectionState::hashsize; i++) {
      connectionState** head = &(pd_connectionState[i]);
      while (*head) {
	if ((*head)->connection->pd_has_dedicated_thread ||
	    (*head)->strand->biDir) {

	  sendCloseConnection((*head)->strand);
	  (*head)->connection->Shutdown();
	  ++closed_count;
	}
	head = &((*head)->next);
      }
    }
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Closed " << closed_count << " connection" << plural(closed_count)
	<< " out of " << pd_nconnections << ".\n";
    }

    if (pd_n_dedicated_workers) {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "Wait for " << pd_n_dedicated_workers
	  << " dedicated thread" << plural(pd_n_dedicated_workers)
	  << " to finish...\n";
      }

      omni_thread::get_time(&s, &ns, timeout);

      int go = 1;
      while (go && pd_n_dedicated_workers) {
	go = pd_cond.timedwait(s, ns);
      }
      if (omniORB::trace(25)) {
	omniORB::logger l;
	if (!go)
	  l << "Timed out. ";

	l << pd_nconnections << " connection" << plural(pd_nconnections)
	  << " and " << pd_n_dedicated_workers << " dedicated worker"
	  << plural(pd_n_dedicated_workers) << " remaining.\n";
      }
    }
  }

  // Stop rendezvousers
  Link* p = pd_rendezvousers.next;
  if (p != &pd_rendezvousers) {
    omniORB::logs(25, "Terminate rendezvousers...");

    for (; p != &pd_rendezvousers; p = p->next) {
      ((giopRendezvouser*)p)->terminate();
    }

    omni_thread::get_time(&s, &ns, timeout);

    int go = 1;
    while (go && pd_rendezvousers.next != & pd_rendezvousers) {
      go = pd_cond.timedwait(s, ns);
    }
    if (omniORB::trace(25)) {
      omniORB::logger l;
      if (go)
	l << "Rendezvousers terminated.\n";
      else
	l << "Timed out waiting for rendezvousers to terminate.\n";
    }
  }

  // Stop bidir monitors
  if (!Link::is_empty(pd_bidir_monitors)) {
    omniORB::logs(25, "Deactivate bidirectional monitors...");

    Link* m = pd_bidir_monitors.next;
    for (; m != &pd_bidir_monitors; m = m->next) {
      ((giopMonitor*)m)->deactivate();
    }

    omni_thread::get_time(&s, &ns, timeout);
    int go = 1;
    while (go && !Link::is_empty(pd_bidir_monitors)) {
      go = pd_cond.timedwait(s, ns);
    }
    if (omniORB::trace(25)) {
      omniORB::logger l;
      if (go)
	l << "Monitors deactivated.\n";
      else
	l << "Timed out waiting for monitors to deactivate.\n";
    }
  }

  if (pd_nconnections) {
    omniORB::logs(25, "Close remaining connections...");
    CORBA::ULong closed_count = 0;

    for (CORBA::ULong i=0; i < connectionState::hashsize; i++) {
      connectionState** head = &(pd_connectionState[i]);
      while (*head) {
	if (!((*head)->connection->pd_has_dedicated_thread ||
	      (*head)->strand->biDir)) {

	  sendCloseConnection((*head)->strand);
	  (*head)->connection->Shutdown();

	  // Start a worker to notice the connection closure.
	  giopWorker* task = new giopWorker((*head)->strand, this, 1);
	  orbAsyncInvoker->insert(task);
	  task->insert((*head)->workers);
	  ++(*head)->connection->pd_n_workers;
	  ++pd_n_temporary_workers;
	  ++closed_count;
	}
	head = &((*head)->next);
      }
    }
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Closed " << closed_count << " connection" << plural(closed_count)
	<< " out of " << pd_nconnections << ".\n";
    }
  }

  if (pd_n_temporary_workers) {
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Wait for " << pd_n_temporary_workers << " temporary worker"
	<< plural(pd_n_temporary_workers) << "...\n";
    }
    int go = 1;
    while (go && pd_n_temporary_workers) {
      go = pd_cond.timedwait(s, ns);
    }
    if (omniORB::trace(25)) {
      omniORB::logger l;
      if (!go)
	l << "Timed out. ";
      l << pd_n_temporary_workers << " temporary worker"
	<< plural(pd_n_temporary_workers) << " remaining.\n";
    }
  }

  // Close bidir connections
  if (pd_bidir_strands.size()) {
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "Close " << pd_bidir_strands.size()
	<< " bidirectional connections...\n";
    }

    omnivector<giopStrand*>::iterator i = pd_bidir_strands.begin();
    while (i != pd_bidir_strands.end()) {
      giopStrand* g = *i;
      pd_bidir_strands.erase(i);
      g->connection->Shutdown();
      g->deleteStrandAndConnection();
    }
  }
  pd_state = IDLE;
  pd_cond.broadcast();
  omniORB::logs(25, "giopServer deactivated.");
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::ensureNotInFlux()
{
  while (pd_state == INFLUX) {
    pd_cond.wait();
    // Note: we could have more than one thread blocking here so must
    //       be wake up by a broadcast. Or else we have to keep a count
    //       on how many is waiting.
  }
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CORBA::ULong giopServer::connectionState::hashsize = 103;

////////////////////////////////////////////////////////////////////////////
giopServer::
connectionState::connectionState(giopConnection* c,giopStrand* s) :
  connection(c), strand(s), next(0)
{
  omni_tracedmutex_lock sync(*omniTransportLock);
  c->incrRefCount();
}

////////////////////////////////////////////////////////////////////////////
giopServer::
connectionState::~connectionState()
{
  OMNIORB_ASSERT(Link::is_empty(workers));
  omni_tracedmutex_lock sync(*omniTransportLock);
  strand->deleteStrandAndConnection();
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csLocate(giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  while (*head) {
    if ((*head)->connection == conn)
      break;
    else {
      head = &((*head)->next);
    }
  }
  return *head;
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csRemove(giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);

  connectionState* cs = 0;

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  while (*head) {
    if ((*head)->connection == conn) {
      cs = *head;
      *head = cs->next;
      pd_nconnections--;
      if (orbParameters::threadPerConnectionPolicy) {
	// Check the number of connection and decide if we need to
	// re-enable the one thread per connection policy that has
	// been temporarily suspended.
	if (!pd_thread_per_connection &&
	    pd_nconnections <= orbParameters::threadPerConnectionLowerLimit) {
	  pd_thread_per_connection = 1;
	}
      }
      break;
    }
    else {
      head = &((*head)->next);
    }
  }
  return cs;
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csInsert(giopConnection* conn)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);


  giopStrand* s = new giopStrand(conn,this);
  s->version.major = 1; s->version.minor = 0;
  {
    ASSERT_OMNI_TRACEDMUTEX_HELD(*omniTransportLock,0);
    omni_tracedmutex_lock sync(*omniTransportLock);
    s->StrandList::insert(giopStrand::passive);
    s->startIdleCounter();
  }

  connectionState* cs =  new connectionState(conn,s);

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  cs->next = *head;
  *head = cs;

  pd_nconnections++;

  if (orbParameters::threadPerConnectionPolicy) {
    // Check the number of connection and decide if we need to
    // turn off the one thread per connection policy temporarily.
    if (pd_thread_per_connection &&
	pd_nconnections >= orbParameters::threadPerConnectionUpperLimit) {
      pd_thread_per_connection = 0;
    }
  }

  conn->pd_has_dedicated_thread = pd_thread_per_connection;

  return cs;
}

////////////////////////////////////////////////////////////////////////////
giopServer::connectionState*
giopServer::csInsert(giopStrand* s)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,1);

  OMNIORB_ASSERT(s->biDir && s->isClient());

  giopConnection* conn = s->connection;

  connectionState* cs =  new connectionState(conn,s);

  connectionState** head = &(pd_connectionState[((omni::ptr_arith_t)conn)%
						connectionState::hashsize]);
  cs->next = *head;
  *head = cs;

  pd_nconnections++;

  if (orbParameters::threadPerConnectionPolicy) {
    // Check the number of connection and decide if we need to
    // turn off the one thread per connection policy temporarily.
    if (pd_thread_per_connection &&
	pd_nconnections >= orbParameters::threadPerConnectionUpperLimit) {
      pd_thread_per_connection = 0;
    }
  }

  conn->pd_has_dedicated_thread = pd_thread_per_connection;

  {
    omni_tracedmutex_lock sync(*omniTransportLock);
    s->connection->decrRefCount();
  }
  return cs;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyRzNewConnection(giopRendezvouser* r, giopConnection* conn)
{
  omni_tracedmutex_lock sync(pd_lock);

  switch (pd_state) {
  case ACTIVE:
    {
      connectionState* cs = csInsert(conn);

      if (conn->pd_has_dedicated_thread) {
	giopWorker* task = new giopWorker(cs->strand, this, 0);
	if (!orbAsyncInvoker->insert(task)) {
	  // Cannot start serving this new connection.
	  if (omniORB::trace(1)) {
	    omniORB::logger log;
	    log << "Cannot create a worker for this endpoint: "
		<< conn->myaddress()
		<< " from "
		<< conn->peeraddress()
		<< "\n";
	  }
	  delete task;
	  {
	    omni_tracedmutex_lock sync(*omniTransportLock);
	    cs->strand->safeDelete();
	  }
	  csRemove(conn);
	  pd_lock.unlock();
	  delete cs;
	  pd_lock.lock();

	  throw outOfResource();
	}
	task->insert(cs->workers);
	conn->pd_n_workers++;
	pd_n_dedicated_workers++;
      }
      else {
	if (!conn->isSelectable()) {
	  if (omniORB::trace(20)) {
	    omniORB::logger log;
	    log << "Connection from " << conn->peeraddress()
		<< " is not selectable. Closing it.\n";
	  }
	  {
	    omni_tracedmutex_lock sync(*omniTransportLock);
	    cs->strand->safeDelete();
	  }
	  csRemove(conn);
	  pd_lock.unlock();
	  delete cs;
	  pd_lock.lock();
	  throw outOfResource();
	}
	pd_lock.unlock();
	conn->setSelectable(1);
	pd_lock.lock();
      }
      break;
    }
  default:
    if (omniORB::trace(25)) {
      omniORB::logger l;
      l << "giopServer terminate connection from "
	<< conn->peeraddress() << ".\n";
    }
    throw Terminate();
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyRzDone(giopRendezvouser* r, CORBA::Boolean exit_on_error)
{
  omni_tracedmutex_lock sync(pd_lock);

  if (!exit_on_error) {
    OMNIORB_ASSERT(pd_state == INFLUX);
    // For the moment, we do not instantiate giopRendezvouser to do
    // single shot.
    // Therefore, this function will *NEVER* be called until:
    //  1. We have called deactivate()
    //  2. giopRendezvouser have encountered a non-recoverable error. In which
    //     case exit_on_error == 1.
  }

  giopEndpoint* ept = r->endpoint();

  if (omniORB::trace(25)) {
    omniORB::logger l;
    l << "giopRendezvouser for endpoint " << ept->address() << " exit.\n";
  }

  r->remove();

  delete r;

  if (exit_on_error && pd_state != INFLUX) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Unrecoverable error for this endpoint: ";
      log << ept->address();
      log << ", it will no longer be serviced.\n";
    }
    ept->Shutdown();
  }
  else {
    pd_endpoints.push_back(ept);
    // A new giopRendezvouser will be instantiated for this endpoint next
    // time activate() is called.
  }

  if (pd_state == INFLUX) {
    if (Link::is_empty(pd_rendezvousers)) {
      omniORB::logs(25, "No remaining rendezvousers.");
      pd_cond.broadcast();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyRzReadable(giopConnection* conn,
			     CORBA::Boolean force_create)
{
  // Theory of operation: read the state diagrams at the end of this file.

  omni_tracedmutex_lock sync(pd_lock);

  switch (pd_state) {
  case ACTIVE:
  case INFLUX:
    {
      if (conn->pd_dying) return;

      if (!force_create &&
	  conn->pd_n_workers >= (int)orbParameters::maxServerThreadPerConnection) {
	conn->pd_has_hit_n_workers_limit = 1;
	return;
      }

      connectionState* cs = csLocate(conn);
      if (!cs) return;

      giopWorker* task = new giopWorker(cs->strand,this,1);
      if (!orbAsyncInvoker->insert(task)) {
	// Cannot start serving this new connection.
	// Should never happen
	OMNIORB_ASSERT(0);
      }
      task->insert(cs->workers);
      conn->pd_n_workers++;
      pd_n_temporary_workers++;
      break;
    }
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyCallFullyBuffered(giopConnection* conn)
{
  notifyRzReadable(conn,1);
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::removeConnectionAndWorker(giopWorker* w)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock, 0);

  connectionState* cs;
  CORBA::Boolean   cs_removed = 0;

  {
    omni_tracedmutex_lock sync(pd_lock);

    giopConnection* conn = w->strand()->connection;

    conn->pd_dying = 1; // From now on, the giopServer will not create
                        // any more workers to serve this connection.

    cs = csLocate(conn);

    // We remove the lock on pd_lock before calling the connection's
    // clearSelectable(). This is necessary so that a simultaneous
    // callback from the Rendezvouser thread will have a chance to
    // look at the connectionState table.
    pd_lock.unlock();

    conn->clearSelectable();

    // Once we reach here, it is certain that the rendezvouser thread
    // would not take any interest in this connection anymore. It
    // is therefore safe to delete this record.
    pd_lock.lock();

    int workers;
    CORBA::Boolean singleshot = w->singleshot();

    if (singleshot)
      workers = --pd_n_temporary_workers;
    else
      workers = --pd_n_dedicated_workers;

    w->remove();
    delete w;

    conn->pd_n_workers--;

    if (Link::is_empty(cs->workers)) {
      csRemove(conn);
      cs_removed = 1;
    }

    if (pd_state == INFLUX) {
      if (omniORB::trace(25)) {
	omniORB::logger l;
	l << "removeConnectionAndWorker for "
	  << (singleshot ? "temporary" : "dedicated")
	  << " worker. " << workers << " remaining.\n";
      }
      if (workers == 0) {
	pd_cond.broadcast();
      }
    }
  }
  // Must not hold pd_lock when deleting cs, since the deletion may
  // cause a call to SocketCollection::removeSocket(), which needs to
  // lock the fdset lock. The fdset lock comes before pd_lock in the
  // partial order.
  if (cs_removed)
    delete cs;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopServer::notifyWkDone(giopWorker* w, CORBA::Boolean exit_on_error)
{
  // Theory of operation: read the state diagrams at the end of this file.

  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);

  if (exit_on_error) {
    removeConnectionAndWorker(w);
    return 0;
  }

  giopConnection* conn = w->strand()->connection;

  if (conn->pd_has_dedicated_thread) {
    // This connection is managed with the thread-per-connection policy
    if (!w->singleshot()) {
      // This is the dedicated thread
      conn->clearSelectable();
      omni_tracedmutex_lock sync(pd_lock);
      conn->pd_dedicated_thread_in_upcall = 0;
      conn->pd_has_hit_n_workers_limit = 0;
      return 1;
    }
    else {
      // This is a temporary worker thread
      omni_tracedmutex_lock sync(pd_lock);
      if (conn->pd_has_hit_n_workers_limit) {
	// Previously we have hit the limit on the no. of worker threads,
	// There is definitely a request pending. We re-cycle this
	// worker to deal with it *UNLESS* the dedicated thread is already
	// doing so.
	conn->pd_has_hit_n_workers_limit = 0;
	if (conn->pd_dedicated_thread_in_upcall ) {
	  return 1;
	}
      }
      if (conn->pd_n_workers == 1 && conn->pd_dying) {
	// Connection is dying. Go round again so this thread spots
	// the condition.
	omniORB::logs(25, "Last worker sees connection is dying.");
	return 1;
      }
      w->remove();
      delete w;
      conn->pd_n_workers--;
      pd_n_temporary_workers--;
      if (pd_state == INFLUX) {
	omniORB::logs(25, "Temporary additional worker finishing.");
	if (pd_n_temporary_workers == 0)
	  pd_cond.broadcast();
      }
      return 0;
    }
  }
  else {
    // This connection is managed with the thread-pool policy.

    OMNIORB_ASSERT(w->singleshot() == 1); // Never called by a dedicated thread
    CORBA::Boolean select_and_return = 0;
    {
      omni_tracedmutex_lock sync(pd_lock);

      if (conn->pd_has_hit_n_workers_limit) {
	// Previously we have hit the limit on the no. of worker threads,
	// There is definitely a request pending. We re-cycle this
	// worker to deal with it.
	conn->pd_has_hit_n_workers_limit = 0;
	return 1;
      }

      // If there are other workers for this connection, or there are
      // too many temporary workers, let this worker finish.

      if (conn->pd_n_workers > orbParameters::threadPoolWatchConnection ||
	  pd_n_temporary_workers > orbParameters::maxServerThreadPoolSize) {

	select_and_return = 1;
      }
    }

    if (!select_and_return) {
      // Call Peek(). This thread will be used for a short time to
      // monitor the connection. If the connection is available for
      // reading, the callback function peekCallBack is called. We can
      // probably afford to call Peek() here because this thread is
      // otherwise idle.

      CORBA::Boolean readable = 0;
      conn->Peek(peekCallBack,(void*)&readable);
      if (readable) {
	// There is data to be read. Tell the worker to go around again.
	return 1;
      }
    }

    // If connection is selectable, it's selectable now
    if (!conn->pd_dying)
      conn->setSelectable(2);

    // Worker is no longer needed.
    {
      omni_tracedmutex_lock sync(pd_lock);

      w->remove();
      delete w;
      conn->pd_n_workers--;
      pd_n_temporary_workers--;

      if (pd_state == INFLUX) {
	omniORB::logs(25, "Temporary worker finishing.");
	if (pd_n_temporary_workers == 0)
	  pd_cond.broadcast();
      }
    }
    return 0;
  }
#ifndef __DECCXX
  // Never reach here (and the hp/compaq/dec compiler is smart enough
  // to figure that out).
  OMNIORB_ASSERT(0);
  return 0;
#endif
}

////////////////////////////////////////////////////////////////////////////
// Callback function used by giopConnection::Peek(). Called when the
// connection indeed has something to read immediately.
void
giopServer::peekCallBack(void* cookie, giopConnection* conn) {
  CORBA::Boolean* readable = (CORBA::Boolean*)cookie;
  *readable = 1;
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyWkPreUpCall(giopWorker* w, CORBA::Boolean data_in_buffer) {

  // Theory of operation: read the state diagrams at the end of this file.

  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);

  giopConnection* conn = w->strand()->connection;

  if (conn->pd_has_dedicated_thread) {
    // This connection is managed with the thread-per-connection policy
    if (!w->singleshot()) {
      // This is the dedicated thread.
      // setSelectable.
      {
	omni_tracedmutex_lock sync(pd_lock);
	conn->pd_dedicated_thread_in_upcall = 1;
      }
      if (orbParameters::maxServerThreadPerConnection > 1) {
	// If only one thread per connection is allowed, there is no
	// need to setSelectable, since we won't be able to act on any
	// interleaved calls that arrive.
	conn->setSelectable(orbParameters::connectionWatchImmediate,
			    data_in_buffer);
      }
    }
    else {
      // This is a temporary worker thread
      // setSelectable only if the dedicated thread is in upcall.
      CORBA::Boolean n;
      {
	omni_tracedmutex_lock sync(pd_lock);
	n = conn->pd_dedicated_thread_in_upcall;
      }
      if (n) {
	conn->setSelectable(orbParameters::connectionWatchImmediate,
			    data_in_buffer);
      }
    }
  }
  else {
    // This connection is managed with the thread-pool policy
    conn->setSelectable(orbParameters::connectionWatchImmediate,
			data_in_buffer);
  }
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopServer::notifySwitchToBiDirectional(giopConnection* conn)
{
  return 1;
  // Note: we could override the threading policy to dictate that all
  //       bidirectional connection will be served by a dedicated thread
  //       on the server side. 
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::notifyMrDone(giopMonitor* m, CORBA::Boolean exit_on_error)
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(pd_lock,0);
  omni_tracedmutex_lock sync(pd_lock);

  if (!exit_on_error && !m->collection()->isEmpty()) {
    // We may have seen a race condition in which the Monitor is about
    // to return when another connection has been added to be monitored.
    // We should not remove the monitor in this case.
    if (orbAsyncInvoker->insert(m)) {
      return;
    }
    // Otherwise, we let the following deal with it.
  }
  m->remove();
  delete m;
  if (pd_state == INFLUX) {
    if (Link::is_empty(pd_bidir_monitors)) {
      omniORB::logs(25, "No remaining bidir monitors.");
      pd_cond.broadcast();
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::Link::insert(giopServer::Link& head)
{
  next = head.prev->next;
  head.prev->next = this;
  prev = head.prev;
  head.prev = this;
}

////////////////////////////////////////////////////////////////////////////
void
giopServer::Link::remove()
{
  prev->next = next;
  next->prev = prev;
  next = prev = this;
}

////////////////////////////////////////////////////////////////////////////
CORBA::Boolean
giopServer::Link::is_empty(giopServer::Link& head)
{
  return (head.next == &head);
}

/////////////////////////////////////////////////////////////////////////////
giopServer*&
giopServer::singleton() {
  static giopServer* singleton_ = 0;
  if (!singleton_) {
    singleton_ = new giopServer();
  }
  return singleton_;
}

/////////////////////////////////////////////////////////////////////////////
static
CORBA::Boolean
registerGiopServer(omniInterceptors::createORBServer_T::info_T& info) {
  info.servers.push_back(giopServer::singleton());
  return 1;
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class threadPerConnectionPolicyHandler : public orbOptions::Handler {
public:

  threadPerConnectionPolicyHandler() : 
    orbOptions::Handler("threadPerConnectionPolicy",
			"threadPerConnectionPolicy = 0 or 1",
			1,
			"-ORBthreadPerConnectionPolicy < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::threadPerConnectionPolicy = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::threadPerConnectionPolicy,
			     result);
  }
};

static threadPerConnectionPolicyHandler threadPerConnectionPolicyHandler_;

/////////////////////////////////////////////////////////////////////////////
class threadPerConnectionUpperLimitHandler : public orbOptions::Handler {
public:

  threadPerConnectionUpperLimitHandler() : 
    orbOptions::Handler("threadPerConnectionUpperLimit",
			"threadPerConnectionUpperLimit = n >= 1",
			1,
			"-ORBthreadPerConnectionUpperLimit < n >= 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::threadPerConnectionUpperLimit = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::threadPerConnectionUpperLimit,
			   result);
  }

};

static threadPerConnectionUpperLimitHandler threadPerConnectionUpperLimitHandler_;

/////////////////////////////////////////////////////////////////////////////
class threadPerConnectionLowerLimitHandler : public orbOptions::Handler {
public:

  threadPerConnectionLowerLimitHandler() : 
    orbOptions::Handler("threadPerConnectionLowerLimit",
			"threadPerConnectionLowerLimit = n >= 1",
			1,
			"-ORBthreadPerConnectionLowerLimit < n >= 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::threadPerConnectionLowerLimit = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::threadPerConnectionLowerLimit,
			   result);
  }
};

static threadPerConnectionLowerLimitHandler threadPerConnectionLowerLimitHandler_;

/////////////////////////////////////////////////////////////////////////////
class maxServerThreadPerConnectionHandler : public orbOptions::Handler {
public:

  maxServerThreadPerConnectionHandler() : 
    orbOptions::Handler("maxServerThreadPerConnection",
			"maxServerThreadPerConnection = n >= 1",
			1,
			"-ORBmaxServerThreadPerConnection < n >= 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxServerThreadPerConnection = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxServerThreadPerConnection,
			   result);
  }

};

static maxServerThreadPerConnectionHandler maxServerThreadPerConnectionHandler_;


/////////////////////////////////////////////////////////////////////////////
class maxServerThreadPoolSizeHandler : public orbOptions::Handler {
public:

  maxServerThreadPoolSizeHandler() : 
    orbOptions::Handler("maxServerThreadPoolSize",
			"maxServerThreadPoolSize = n >= 1",
			1,
			"-ORBmaxServerThreadPoolSize < n >= 1 >") {}

  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v) || v < 1) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_greater_than_zero_ulong_msg);
    }
    orbParameters::maxServerThreadPoolSize = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::maxServerThreadPoolSize,
			   result);
  }
};

static maxServerThreadPoolSizeHandler maxServerThreadPoolSizeHandler_;

/////////////////////////////////////////////////////////////////////////////
class threadPoolWatchConnectionHandler : public orbOptions::Handler {
public:

  threadPoolWatchConnectionHandler() : 
    orbOptions::Handler("threadPoolWatchConnection",
			"threadPoolWatchConnection = n >= 0",
			1,
			"-ORBthreadPoolWatchConnection < n >= 0 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::ULong v;
    if (!orbOptions::getULong(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_ulong_msg);
    }
    orbParameters::threadPoolWatchConnection = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVULong(key(),orbParameters::threadPoolWatchConnection,
			   result);
  }
};

static threadPoolWatchConnectionHandler threadPoolWatchConnectionHandler_;

/////////////////////////////////////////////////////////////////////////////
class connectionWatchImmediateHandler : public orbOptions::Handler {
public:

  connectionWatchImmediateHandler() : 
    orbOptions::Handler("connectionWatchImmediate",
			"connectionWatchImmediate = 0 or 1",
			1,
			"-ORBconnectionWatchImmediate < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::connectionWatchImmediate = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::connectionWatchImmediate,
			     result);
  }
};

static connectionWatchImmediateHandler connectionWatchImmediateHandler_;




/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_giopserver_initialiser : public omniInitialiser {
public:

  omni_giopserver_initialiser() {
    orbOptions::singleton().registerHandler(threadPerConnectionPolicyHandler_);
    orbOptions::singleton().registerHandler(threadPerConnectionUpperLimitHandler_);
    orbOptions::singleton().registerHandler(threadPerConnectionLowerLimitHandler_);
    orbOptions::singleton().registerHandler(maxServerThreadPerConnectionHandler_);
    orbOptions::singleton().registerHandler(maxServerThreadPoolSizeHandler_);
    orbOptions::singleton().registerHandler(threadPoolWatchConnectionHandler_);
    orbOptions::singleton().registerHandler(connectionWatchImmediateHandler_);
  }

  void attach() {
    omniInterceptors* interceptors = omniORB::getInterceptors();
    interceptors->createORBServer.add(registerGiopServer);
  }
  void detach() {
  }
};


static omni_giopserver_initialiser initialiser;

omniInitialiser& omni_giopserver_initialiser_ = initialiser;


OMNI_NAMESPACE_END(omni)


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//   The following diagram illustrates the state transition of a connection
//   when it is managed under the thread pool policy.
//
//   Under this policy, the connection is watched by the rendezvouser
//   thread. When data arrive, the rendezvouser calls notifyRzReadable and
//   a worker thread is dispatched.  The worker unmarshal data from the
//   connection. When it finishes unmarshaling all the arguments, worker
//   calls notifyWkPreUpCall before doing the upcall into the
//   implementation.  In notifyWkPreUpCall, the connection is again
//   registered to be watched by the rendezvouser. When the worker has
//   finished the upcall, it calls notifyWkDone. If data for another call
//   arrives, the whole cycle repeats again.
//
//   The state diagram below illustrates what happens:
//
//        Legend:
//               +----------+     x = 1 : the connection is watched
//               +  (x,y,z) +         0 : the connection is not watched
//               +----------+     y = 1 : rendezvouser detected data arrival
//                                    0 : no new data have arrived
//                                z = n >= 0 : no. of threads serving the
//                                             connection
//

//
//                +---------+
//                | (1,0,0) |<----------------------+
//	          +----+----+                       |
//		       |		   	    |
//           	       | DataArrive	   	    |
//		       |		   	    |
//		       V		   	    |
//                +---------+	   	            |notifyWkDone**
//     	          | (0,1,0) |	   	            |  **-tell the rendezvouser
//	          +----+----+	   	            |     to watch immediately
//		       |		   	    |
//		       | notifyRzReadable	    |
//		       |		   	    |
//     	       	       V		   	    |
//                 +---------+       	          +-+-------+
//     	       +-->| (0,0,1) +------------------->| (1,0,1) |
//	       |   +---------+ notifyWkPreUpCall  +--+------+
//	       | 	     	 	   	     | ^
//     	       |     	  +--------------------------+ |
//	       |	  |    	    DataArrive 	       |
//             |	  V  		      	       |
//             |   +---------+	      	       	       |
//notifyWkDone |   | (0,1,1) |		       	       |
//   	       |   +----+----+		       	       |notifyWkDone
//   	       |	  |    			       |
//   	       |	  | notifyRzReadable	       |
//     	       |    	  |  		    	       |
//	       |    	  V  		    	       |
//             |   +---------+       	          +----+----+
//     	       +---+ (0,0,2) +------------------->| (1,0,2) |
//	       +-->+---------+ notifyWkPreUpCall  +--+------+
//	       | 	     	 	   	     | ^
//     	       |     	  +--------------------------+ |
//	       |	  |    	    DataArrive 	       |
//             |	  V  		      	       |
//             |   +---------+	      	       	       |
//notifyWkDone |   | (0,1,2) |		       	       |
//   	       |   +----+----+		       	       |notifyWkDone
//   	       |	  |    			       |
//   	       |	  | notifyRzReadable	       |
//     	       |    	  |  		    	       |
//	       |    	  V  		    	       |
//             |   +---------+       	          +----+----+
//     	       +---+ (0,0,3) +------------------->| (1,0,3) |
//	           +---------+ notifyWkPreUpCall  +--+------+
//  .... And so on...
//
//

//   To summarise, here are what the transition function should do:
//     notifyRzReadable()
//        - Always create a temporary worker
//
//     notifyWkPreUpCall()
//        - setSelectable
//
//     notifyWkDone()
//        - setSelectable(2) if n worker == 1.
//

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//   The following diagram illustrates the state transition of a connection
//   when it is managed under the thread-per-connection policy.
//
//   Under this policy, each connection has a dedicated worker thread.
//   When a call arrives, the worker unmarshalls all the arguments and do
//   an upcall. Before it does the upcall, it calls notifyWkPreUpCall.  In
//   notifyWkPreUpCall, the connection is registered to be watched by
//   the rendezvouser. When the worker has finished the upcall, it calls
//   notifyWkDone, the connnection is un-registered with the rendezvouser.
//   The worker then waits for another call to arrive and the whole cycle
//   repeats again.
//   If while the worker is performing the upcall another call has arrived,
//   the rendezvouser calls notifyRzReadable and a temporary worker thread
//   is dispatched. This thread is 'temporary' in the sense that it is only
//   dispatched to serve one call.
//   The interaction of the dedicated worker, the temporary worker and
//   the rendezvouser is summarised by the state diagram below:
//
//        Legend:
//               +----------+     x = 1 : the connection is watched
//               +  (x,y,z) +         0 : the connection is not watched
//               +----------+     y = 1 : rendezvouser detected data arrival
//                                    0 : no new data have arrived
//                                z = n >= 0 : no. of threads serving the
//                                             connection. The dedicated thread
//                                             is not watching the connection.
//                                z* = same as z except that the dedicated
//                                     thread is watching the connection.
//
//               notifyWkDone - transition executed by the temporary workers
//
//               notifyWkDone - transition executed by the dedicated worker
//               ^^^^^^^^^^^^
//

//
//                 +---------+
//     	      +--->| (0,0,1*)|<--------------------+
//     	      |    +----+----+			   |
//     	      |	       	| notifyWkPreUpCall	   |
//     	      |	    	| ^^^^^^^^^^^^^^^^^	   |
//notifyWkDone|	       	|     			   |
//^^^^^^^^^^^^|	    	|     			   |
//	      |	    	V     			   |
//            |    +---------+			   |
//     	      +----+ (1,0,1) +			   |notifyWkDone
//	      +--->+----+----+			   |
//     	      |	       	|     			   |
//	      |	   	|DataArrive		   |
//	      |	   	|     	   		   |
//	      |	   	V     	   		   |
//            |    +---------+			   |
//     	      |    | (0,1,1) +			   |
//	      |    +----+----+			   |
//	      |	       	|      		      	   |
//	      |	       	|notifyRzReadable     	   |	notifyWkPreUpCall
//notifyWkDone|	   	|      		      	   |   +--+
//	      |	   	|                          |   |  |
//	      |	   	|                          |   |  |
//	      |	   	V                          |   V  |
//            |    +---------+         	          ++------+-+
//     	    +-|--->| (0,0,2) +------------------->| (0,0,2*)|
//	    | |    +----+----+    notifyWkDone    +---------+
//	    | |	   	|      	  ^^^^^^^^^^^^	  |  ^ ^
//	    | |	   	|  			  |  | |
//	    | |	   	|notifyWkPreUpCall	  |  | |
//	    | |	   	|  		  	  |  | |
//	    | |	   	| +-----------------------+  | |
//	    | |	   	V V     notifyWkPreUpCall(1) | |
//          | |    +---------+ 	^^^^^^^^^^^^^^^^^^^^ | |
//     	    | +----+ (1,0,2) +-----------------------+ |
//	    |   +->+----+----+ 	   notifyWkDone	       |notifyWkDone
//     	    +-+	|      	|      	   ^^^^^^^^^^^^	       |
//     	      |	|      	|      		    	       |
//	      |	|  	|DataArrive	    	       |
//	      |	|  	|      		    	       |
//	      |	|  	V      		    	       |
//            | |  +---------+ 		    	       |
//     	      |	|  | (0,1,2) + 		    	       |
//	      | |  +----+----+ 		    	   +---+
//	      |	|      	|      		      	   |
//	      |	|      	|notifyRzReadable     	   |	notifyWkPreUpCall
//notifyWkDone|	|  	|      		      	   |   +--+
//	      |	|  	|      		      	   |   |  |
//	      |	|  	|      		      	   |   |  |
//	      |	|  	V      		      	   |   V  |
//            | |  +---------+       	          ++------+-+
//     	      +----+ (0,0,3) +------------------->| (0,0,3*)|
//	        |  +----+----+    notifyWkDone    +---------+
//     	       	|      	|      	  ^^^^^^^^^^^^	  |    ^
//	       	|      	|      		    	  |    |
//  notifyWkDone|  	|notifyWkPreUpCall  	  |    |
//	       	|  	|      		    	  |    |
//	       	|  	|  +----------------------+    |
//	       	|  	V  V   	notifyWkPreUpCall(1)   |
//              |  +---------+ 	^^^^^^^^^^^^^^^^^^^^   |
//     	        +--+ (1,0,3) +-------------------------+
//     	           +----+----+	   notifyWkDone
//     	       	       	|      	   ^^^^^^^^^^^^
//
//   Note: (1) May also happen under a race condition- if dedicated thread 
//             gets the read lock before the temporary worker.
//
//   And So On...
//
//

//   To summarise, here are what the transition function should do:
//     notifyRzReadable()
//        - Always create a temporary worker
//
//     notifyWkPreUpCall()
//        dedicated thread - setSelectable unless n workers != 1
//        temporary thread - setSelectable only if the dedicated thread is
//                           in the upcall
//
//     notifyWkDone()
//        dedicated thread - clearSelectable
//        temporary thread - Do nothing
//

