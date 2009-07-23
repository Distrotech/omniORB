// -*- Mode: C++; -*-
//                            Package   : omniORB
// poamanager.cc              Created on: 12/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2005 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
//
//    This file is part of the omniORB library.
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
//    Internal implementation of the PortableServer::POAManager.
//

/*
  $Log$
  Revision 1.4.2.2  2005/01/06 23:10:40  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.4.2.1  2003/03/23 21:02:07  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.9  2002/01/16 11:32:00  dpg1
  Race condition in use of registerNilCorbaObject/registerTrackedObject.
  (Reported by Teemu Torma).

  Revision 1.2.2.8  2001/11/13 14:11:46  dpg1
  Tweaks for CORBA 2.5 compliance.

  Revision 1.2.2.7  2001/09/19 17:26:52  dpg1
  Full clean-up after orb->destroy().

  Revision 1.2.2.6  2001/06/07 16:24:11  dpg1
  PortableServer::Current support.

  Revision 1.2.2.5  2001/05/31 16:18:15  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.2.2.4  2001/04/18 18:18:05  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.3  2000/11/09 12:27:58  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.2  2000/09/27 17:41:41  sll
  Updated include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:35:57  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:56  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.7  2000/06/22 10:40:17  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.6  2000/02/04 18:11:03  djr
  Minor mods for IRIX (casting pointers to ulong instead of int).

  Revision 1.1.2.5  2000/01/20 11:51:37  djr
  (Most) Pseudo objects now used omni::poRcLock for ref counting.
  New assertion check OMNI_USER_CHECK.

  Revision 1.1.2.4  1999/10/29 13:18:19  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.1.2.3  1999/10/14 16:22:15  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/30 11:52:33  djr
  Implemented use of AdapterActivators in POAs.

  Revision 1.1.2.1  1999/09/22 14:27:02  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <poamanager.h>
#include <poaimpl.h>
#include <poacurrentimpl.h>
#include <exceptiondefs.h>
#include <omniCurrent.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/objTracker.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
///////////////////// PortableServer::POAManager /////////////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(PortableServer::POAManager,
				       AdapterInactive,
       "IDL:omg.org/PortableServer/POAManager/AdapterInactive" PS_VERSION)


PortableServer::POAManager::~POAManager() {}


PortableServer::POAManager_ptr
PortableServer::POAManager::_duplicate(PortableServer::POAManager_ptr obj)
{
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();

  return obj;
}


PortableServer::POAManager_ptr
PortableServer::POAManager::_narrow(CORBA::Object_ptr obj)
{
  if( CORBA::is_nil(obj) || !obj->_NP_is_pseudo() )  return _nil();

  POAManager_ptr p = (POAManager_ptr) obj->_ptrToObjRef(_PD_repoId);

  if( p )  p->_NP_incrRefCount();

  return p ? p : _nil();
}


PortableServer::POAManager_ptr
PortableServer::POAManager::_nil()
{
  static omniOrbPOAManager* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr ) {
      _the_nil_ptr = new omniOrbPOAManager(1 /* is nil */);
      registerNilCorbaObject(_the_nil_ptr);
    }
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}


const char* PortableServer::POAManager::_PD_repoId =
  "IDL:omg.org/PortableServer/POAManager" PS_VERSION;

//////////////////////////////////////////////////////////////////////
////////////////////////// omniOrbPOAManager /////////////////////////
//////////////////////////////////////////////////////////////////////

#define CHECK_NOT_NIL()  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref()


static omni_tracedmutex     pm_lock;
static omni_tracedcondition pm_cond(&pm_lock);
// Condition variable used to signal deactivations

omniOrbPOAManager::~omniOrbPOAManager() {}


void
omniOrbPOAManager::activate()
{
  CHECK_NOT_NIL();

  omni_tracedmutex_lock sync(pm_lock);

  if( pd_state == INACTIVE )  throw AdapterInactive();
  if( pd_state == ACTIVE   )  return;

  pd_state = ACTIVE;

  for( CORBA::ULong i = 0; i < pd_poas.length(); i++ )
    pd_poas[i]->pm_change_state(pd_state);
}


void
omniOrbPOAManager::hold_requests(CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation.
    // Fortunately, the spec says we don't have to worry about whether
    // we're in the context of an operation invocation in a POA
    // managed by this POAManager. An invocation on any POA in the
    // same ORB will do.
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  POASeq poas;
  {
    omni_tracedmutex_lock sync(pm_lock);

    if( pd_state == INACTIVE )  throw AdapterInactive();
    if( pd_state == HOLDING  )  return;

    pd_state = HOLDING;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      if( wait_for_completion ) {
	poas[i] = pd_poas[i];
	poas[i]->incrRefCount();
      }
    }
  }

  if( wait_for_completion )
    for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
      poas[i]->pm_waitForReqCmpltnOrSttChnge(HOLDING);
      poas[i]->decrRefCount();
    }
}


void
omniOrbPOAManager::discard_requests(CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation.
    // Fortunately, the spec says we don't have to worry about whether
    // we're in the context of an operation invocation in a POA
    // managed by this POAManager. An invocation on any POA in the
    // same ORB will do.
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  POASeq poas;
  {
    omni_tracedmutex_lock sync(pm_lock);

    if( pd_state == INACTIVE   )  throw AdapterInactive();
    if( pd_state == DISCARDING )  return;

    pd_state = DISCARDING;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      if( wait_for_completion ) {
	poas[i] = pd_poas[i];
	poas[i]->incrRefCount();
      }
    }
  }

  if( wait_for_completion )
    for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
      poas[i]->pm_waitForReqCmpltnOrSttChnge(DISCARDING);
      poas[i]->decrRefCount();
    }
}


static void
deactivate_thread_fn(void* args)
{
  OMNIORB_ASSERT(args);
  void** targs = (void**) args;

  omniOrbPOAManager::POASeq* ppoas = (omniOrbPOAManager::POASeq*) targs[0];
  omniOrbPOAManager::POASeq& poas = *ppoas;
  CORBA::Boolean etherealise = (CORBA::Boolean) (unsigned long) targs[1];
  int* deactivated = (int*)targs[2];
  delete[] targs;

  for( CORBA::ULong i = 0; i < poas.length(); i++ ) {
    poas[i]->pm_deactivate(etherealise);
    poas[i]->decrRefCount();
  }
  delete ppoas;

  *deactivated = 1;
  pm_cond.broadcast();
}


void
omniOrbPOAManager::deactivate(CORBA::Boolean etherealize_objects,
			      CORBA::Boolean wait_for_completion)
{
  CHECK_NOT_NIL();

  if( wait_for_completion ) {
    // Complain if in the context of an operation invocation.
    // Fortunately, the spec says we don't have to worry about whether
    // we're in the context of an operation invocation in a POA
    // managed by this POAManager. An invocation on any POA in the
    // same ORB will do.
    omniCurrent* current = omniCurrent::get();
    if (current && current->callDescriptor()) {
      OMNIORB_THROW(BAD_INV_ORDER,
		    BAD_INV_ORDER_WouldDeadLock,
		    CORBA::COMPLETED_NO);
    }
  }
  POASeq* ppoas = new POASeq;
  POASeq& poas = *ppoas;
  {
    omni_tracedmutex_lock sync(pm_lock);

    if( pd_state == INACTIVE ) {
      while( !pd_deactivated )	pm_cond.wait();
      return;
    }

    pd_state = INACTIVE;
    poas.length(pd_poas.length());

    for( CORBA::ULong i = 0; i < pd_poas.length(); i++ ) {
      pd_poas[i]->pm_change_state(pd_state);
      poas[i] = pd_poas[i];
      poas[i]->incrRefCount();
    }
  }

  void** args = new void* [3];
  args[0] = ppoas;

  if (etherealize_objects)
    args[1] = (void*)1;
  else
    args[1] = (void*)0;

  args[2] = &pd_deactivated;

  if( wait_for_completion )
    deactivate_thread_fn(args);
  else
    (new omni_thread(deactivate_thread_fn, args))->start();
}


PortableServer::POAManager::State
omniOrbPOAManager::get_state()
{
  omni_tracedmutex_lock sync(pm_lock);
  return pd_state;
}

////////////////////////////
// Override CORBA::Object //
////////////////////////////

void*
omniOrbPOAManager::_ptrToObjRef(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if( omni::ptrStrMatch(repoId, PortableServer::POAManager::_PD_repoId) )
    return (PortableServer::POAManager_ptr) this;
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;

  return 0;
}


void
omniOrbPOAManager::_NP_incrRefCount()
{
  omni::poRcLock->lock();
  pd_refCount++;
  omni::poRcLock->unlock();
}


void
omniOrbPOAManager::_NP_decrRefCount()
{
  omni::poRcLock->lock();
  int done = --pd_refCount > 0;
  omni::poRcLock->unlock();
  if( done )  return;

  OMNIORB_USER_CHECK(pd_poas.length() == 0);
  OMNIORB_USER_CHECK(pd_refCount == 0);
  // If either of these fails then the application has released a
  // POAManager reference too many times.

  delete this;
}

//////////////
// Internal //
//////////////

void
omniOrbPOAManager::gain_poa(omniOrbPOA* poa)
{
  omni_tracedmutex_lock sync(pm_lock);

  pd_poas.length(pd_poas.length() + 1);
  pd_poas[pd_poas.length() - 1] = poa;

  switch( pd_state ) {
  case HOLDING:
    break;
  default:
    poa->pm_change_state(pd_state);
    break;
  }
}


void
omniOrbPOAManager::lose_poa(omniOrbPOA* poa)
{
  omni_tracedmutex_lock sync(pm_lock);

  CORBA::ULong i, len = pd_poas.length();

  for( i = 0; i < len; i++ )
    if( pd_poas[i] == poa )
      break;

  if( i == len )
    throw omniORB::fatalException(__FILE__, __LINE__,
				  "lose_poa(...) for POA I didn't own!");

  for( ; i < len - 1; i++ )
    pd_poas[i] = pd_poas[i + 1];

  pd_poas.length(len - 1);
}
