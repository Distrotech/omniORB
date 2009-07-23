// -*- Mode: C++; -*-
//                            Package   : omniORB
// portableserver.cc          Created on: 11/5/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2004-2007 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Research Cambridge
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
//    Misc code from PortableServer module.
//
 
/*
  $Log$
  Revision 1.4.2.6  2007/04/05 15:38:02  dgrisby
  Catch exceptions from servant destructor.

  Revision 1.4.2.5  2006/07/18 16:21:21  dgrisby
  New experimental connection management extension; ORB core support
  for it.

  Revision 1.4.2.4  2005/07/22 17:18:36  dgrisby
  Another merge from omni4_0_develop.

  Revision 1.4.2.3  2005/01/06 23:10:40  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.4.2.2  2004/02/16 10:10:32  dgrisby
  More valuetype, including value boxes. C++ mapping updates.

  Revision 1.4.2.1  2003/03/23 21:02:05  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.11  2002/01/16 11:32:00  dpg1
  Race condition in use of registerNilCorbaObject/registerTrackedObject.
  (Reported by Teemu Torma).

  Revision 1.2.2.10  2001/10/19 11:05:25  dpg1
  ObjectId to/from wstring

  Revision 1.2.2.9  2001/09/19 17:26:52  dpg1
  Full clean-up after orb->destroy().

  Revision 1.2.2.8  2001/08/15 10:26:14  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.2.2.7  2001/08/03 17:41:24  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.2.2.6  2001/06/07 16:24:11  dpg1
  PortableServer::Current support.

  Revision 1.2.2.5  2001/05/31 16:18:15  dpg1
  inline string matching functions, re-ordered string matching in
  _ptrToInterface/_ptrToObjRef

  Revision 1.2.2.4  2001/04/18 18:18:05  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.3  2000/11/09 12:27:58  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.2  2000/09/27 18:04:43  sll
  Use new string allocation functions. Updated to use the new calldescriptor.

  Revision 1.2.2.1  2000/07/17 10:35:58  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.10  2000/06/27 16:23:25  sll
  Merged OpenVMS port.

  Revision 1.1.2.9  2000/06/22 10:40:17  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.8  2000/04/27 10:52:12  dpg1
  Interoperable Naming Service

  omniInitialReferences::get() renamed to omniInitialReferences::resolve().

  Revision 1.1.2.7  2000/01/03 18:43:32  djr
  Fixed bug in ref counting of POA Policy objects.

  Revision 1.1.2.6  1999/10/29 13:18:20  djr
  Changes to ensure mutexes are constructed when accessed.

  Revision 1.1.2.5  1999/10/27 17:32:16  djr
  omni::internalLock and objref_rc_lock are now pointers.

  Revision 1.1.2.4  1999/10/16 13:22:54  djr
  Changes to support compiling on MSVC.

  Revision 1.1.2.3  1999/10/14 16:22:16  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/10/04 17:08:34  djr
  Some more fixes/MSVC work-arounds.

  Revision 1.1.2.1  1999/09/22 14:27:04  djr
  Major rewrite of orbcore to support POA.

*/

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <poaimpl.h>
#include <poacurrentimpl.h>
#include <localIdentity.h>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/objTracker.h>
#include <initRefs.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <omniCurrent.h>
#include <objectTable.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
////////////////////////// Policy Interfaces /////////////////////////
//////////////////////////////////////////////////////////////////////


_init_in_def_( const CORBA::ULong
	       PortableServer::THREAD_POLICY_ID = 16; )
_init_in_def_( const CORBA::ULong
	       PortableServer::LIFESPAN_POLICY_ID = 17; )
_init_in_def_( const CORBA::ULong
	       PortableServer::ID_UNIQUENESS_POLICY_ID = 18; )
_init_in_def_( const CORBA::ULong
	       PortableServer::ID_ASSIGNMENT_POLICY_ID = 19; )
_init_in_def_( const CORBA::ULong
	       PortableServer::IMPLICIT_ACTIVATION_POLICY_ID = 20; )
_init_in_def_( const CORBA::ULong
	       PortableServer::SERVANT_RETENTION_POLICY_ID = 21; )
_init_in_def_( const CORBA::ULong
	       PortableServer::REQUEST_PROCESSING_POLICY_ID = 22; )


#define DEFINE_POLICY_OBJECT(name)  \
  \
PortableServer::name::~name() {}  \
  \
CORBA::Policy_ptr  \
PortableServer::name::copy()  \
{  \
  if( _NP_is_nil() )  _CORBA_invoked_nil_pseudo_ref();  \
  return new name(pd_value);  \
}  \
  \
void*  \
PortableServer::name::_ptrToObjRef(const char* repoId)  \
{  \
  OMNIORB_ASSERT(repoId );  \
  \
  if( omni::ptrStrMatch(repoId, PortableServer::name::_PD_repoId) )  \
    return (PortableServer::name##_ptr) this;  \
  if( omni::ptrStrMatch(repoId, CORBA::Policy::_PD_repoId) )  \
    return (CORBA::Policy_ptr) this;  \
  if( omni::ptrStrMatch(repoId, CORBA::Object::_PD_repoId) )  \
    return (CORBA::Object_ptr) this;  \
  \
  return 0;  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_duplicate(PortableServer::name##_ptr obj)  \
{  \
  if( !CORBA::is_nil(obj) )  obj->_NP_incrRefCount();  \
  \
  return obj;  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_narrow(CORBA::Object_ptr obj)  \
{  \
  if( CORBA::is_nil(obj) )  return _nil();  \
  \
  name##_ptr p = (name##_ptr) obj->_ptrToObjRef(name::_PD_repoId);  \
  \
  if( p )  p->_NP_incrRefCount();  \
  \
  return p ? p : _nil();  \
}  \
  \
PortableServer::name##_ptr  \
PortableServer::name::_nil()  \
{  \
  static name* _the_nil_ptr = 0;  \
  if( !_the_nil_ptr ) {  \
    omni::nilRefLock().lock();  \
    if( !_the_nil_ptr ) { \
      _the_nil_ptr = new name;  \
      registerNilCorbaObject(_the_nil_ptr); \
    } \
    omni::nilRefLock().unlock();  \
  }  \
  return _the_nil_ptr;  \
}  \
  \
const char*  \
PortableServer::name::_PD_repoId = "IDL:omg.org/PortableServer/" #name PS_VERSION;


DEFINE_POLICY_OBJECT(ThreadPolicy)
DEFINE_POLICY_OBJECT(LifespanPolicy)
DEFINE_POLICY_OBJECT(IdUniquenessPolicy)
DEFINE_POLICY_OBJECT(IdAssignmentPolicy)
DEFINE_POLICY_OBJECT(ImplicitActivationPolicy)
DEFINE_POLICY_OBJECT(ServantRetentionPolicy)
DEFINE_POLICY_OBJECT(RequestProcessingPolicy)

#undef DEFINE_POLICY_OBJECT

//////////////////////////////////////////////////////////////////////
///////////////////////////// ServantBase ////////////////////////////
//////////////////////////////////////////////////////////////////////

static omni_tracedmutex ref_count_lock;

PortableServer::ServantBase::~ServantBase() {}


PortableServer::POA_ptr
PortableServer::ServantBase::_default_POA()
{
  return omniOrbPOA::rootPOA();
}


CORBA::_objref_InterfaceDef*
PortableServer::ServantBase::_get_interface()
{
  // Return 0 to indicate to _do_get_interface() that we have not
  // been overriden.  We cannot implement this method here, because
  // we would have to do a downcast to InterfaceDef, which would
  // introduce a dependency on the dynamic library.
  return 0;
}


void
PortableServer::ServantBase::_add_ref()
{
  omni_tracedmutex_lock l(ref_count_lock);
  // If the reference count is 0, then the object is either in the
  // process of being deleted by _remove_ref, or has already been
  // deleted. It is too late to be trying to _add_ref now. If the
  // reference count is less than zero, then _remove_ref has been
  // called too many times.
  OMNIORB_USER_CHECK(_pd_refCount > 0);

  _pd_refCount++;
}


void
PortableServer::ServantBase::_remove_ref()
{
  ref_count_lock.lock();
  int done = --_pd_refCount > 0;
  ref_count_lock.unlock();
  if( done )  return;

  if( _pd_refCount < 0 ) {
    omniORB::logs(1, "ServantBase has negative ref count!");
    return;
  }

  omniORB::logs(15, "ServantBase has zero ref count -- deleted.");

  try {
    delete this;
  }
  catch (...) {
    omniORB::logs(1, "ERROR: Servant destructor threw an exception.");
  }
}

CORBA::ULong
PortableServer::ServantBase::_refcount_value()
{
  omni_tracedmutex_lock l(ref_count_lock);
  return _pd_refCount;
}

void*
PortableServer::ServantBase::_do_this(const char* repoId)
{
  OMNIORB_ASSERT(repoId);

  if (!omni::internalLock) {
    // Not initalised yet
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_POANotInitialised,
		  CORBA::COMPLETED_NO);
  }

  omniCurrent* current = omniCurrent::get();
  if (current) {
    omniCallDescriptor* call_desc = current->callDescriptor();

    if (call_desc &&
	call_desc->localId()->servant() == (omniServant*)this) {

      // In context of an invocation on this servant
      omniObjRef* ref = omniOrbPOACurrent::real_get_reference(call_desc);
      OMNIORB_ASSERT(ref);
      return ref->_ptrToObjRef(repoId);
    }
  }

  {
    omni_tracedmutex_lock sync(*omni::internalLock);

    if (_activations().size() == 1) {
      // We only have a single activation -- return a reference to it.
      omniObjTableEntry* entry = _activations()[0];
      omniOrbPOA* poa = omniOrbPOA::_downcast(entry->adapter());
      omniIORHints hints(poa ? poa->policy_list() : 0);
      omniObjRef* ref = omni::createLocalObjRef(_mostDerivedRepoId(), repoId,
						entry, hints);
      OMNIORB_ASSERT(ref);
      return ref->_ptrToObjRef(repoId);
    }
  }

  PortableServer::POA_var poa = this->_default_POA();

  if( CORBA::is_nil(poa) )
    OMNIORB_THROW(OBJ_ADAPTER,OBJ_ADAPTER_POANotInitialised,
		  CORBA::COMPLETED_NO);

  return ((omniOrbPOA*)(PortableServer::POA_ptr) poa)->
    servant__this(this, repoId);
}


omniObjRef*
PortableServer::ServantBase::_do_get_interface()
{
  CORBA::_objref_InterfaceDef* p = _get_interface();
  if( p )  return p->_PR_getobj();

  // If we get here then we assume that _get_interface has not
  // been overriden, and provide the default implementation.

  // repoId should not be empty for statically defined
  // servants.  This version should not have been called
  // if it is a dynamic implementation.
  const char* repoId = _mostDerivedRepoId();
  OMNIORB_ASSERT(repoId && *repoId);

  // Obtain the object reference for the interface repository.
  CORBA::Object_var repository = CORBA::Object::_nil();
  try {
    repository = omniInitialReferences::resolve("InterfaceRepository");
  }
  catch (...) {
  }
  if( CORBA::is_nil(repository) )
    OMNIORB_THROW(INTF_REPOS,INTF_REPOS_NotAvailable, CORBA::COMPLETED_NO);

  // Make a call to the interface repository.
  omniStdCallDesc::_cCORBA_mObject_i_cstring
    call_desc(omniDynamicLib::ops->lookup_id_lcfn, "lookup_id", 10, repoId);
  repository->_PR_getobj()->_invoke(call_desc);

  CORBA::Object_ptr result = call_desc.result();
  return result ? result->_PR_getobj() : 0;
}


void*
PortableServer::ServantBase::_downcast()
{
  return (Servant) this;
}


//////////////////////////////////////////////////////////////////////
//////////////////////// C++ Mapping Specific ////////////////////////
//////////////////////////////////////////////////////////////////////

char*
PortableServer::ObjectId_to_string(const ObjectId& id)
{
  int len = id.length();

  char* s = _CORBA_String_helper::alloc(len);

  for( int i = 0; i < len; i++ )
    if( (char) (s[i] = id[i]) == '\0' ) {
      _CORBA_String_helper::free(s);
      OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectId, CORBA::COMPLETED_NO);
    }

  s[len] = '\0';
  return s;
}


_CORBA_WChar*
PortableServer::ObjectId_to_wstring(const ObjectId& id)
{
  if (id.length() % SIZEOF_WCHAR != 0)
    OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectId, CORBA::COMPLETED_NO);

  int len = id.length() / SIZEOF_WCHAR;

  _CORBA_WChar* s = _CORBA_WString_helper::alloc(len);

  _CORBA_WChar* buf = (_CORBA_WChar*)id.NP_data();

  for( int i = 0; i < len; i++ )
    if( (s[i] = buf[i]) == 0 ) {
      _CORBA_WString_helper::free(s);
      OMNIORB_THROW(BAD_PARAM,BAD_PARAM_InvalidObjectId, CORBA::COMPLETED_NO);
    }

  s[len] = '\0';
  return s;
}


PortableServer::ObjectId*
PortableServer::string_to_ObjectId(const char* s)
{
  int len = strlen(s);
  ObjectId* pid = new ObjectId(len);
  ObjectId& id = *pid;

  id.length(len);

  for( int i = 0; i < len; i++ )  id[i] = *s++;

  return pid;
}


PortableServer::ObjectId*
PortableServer::wstring_to_ObjectId(const _CORBA_WChar* s)
{
  int len = _CORBA_WString_helper::len(s);
  ObjectId* pid = new ObjectId(len * SIZEOF_WCHAR);
  
  pid->length(len * SIZEOF_WCHAR);

  _CORBA_WChar* buf = (_CORBA_WChar*)pid->NP_data();

  for( int i = 0; i < len; i++ )  *buf++ = *s++;

  return pid;
}
