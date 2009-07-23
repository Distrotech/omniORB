// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynException.cc            Created on: 10/1998
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//   Exceptions used in the Dynamic library.
//

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/tcDescriptor.h>
#include <exceptiondefs.h>

OMNI_USING_NAMESPACE(omni)

//////////////////////////////////////////////////////////////////////
////////  Implementation of Any Insertion for Exception //////////////
//////////////////////////////////////////////////////////////////////

void operator<<=(CORBA::Any& a, const CORBA::Exception& ex)
{
  if (ex.insertToAnyFn()) {
    ex.insertToAnyFn()(a,ex);
  }
  else {
    omniORB::logger log;
    log << "Error: function to insert the user exception into an Any is not available\n";
    log.flush();
    OMNIORB_THROW(INTERNAL,0,CORBA::COMPLETED_NO);
  }
}

void operator<<=(CORBA::Any& a, const CORBA::Exception* ex)
{
  if (ex->insertToAnyFnNCP()) {
    ex->insertToAnyFnNCP()(a,ex);
  }
  else {
    omniORB::logger log;
    log << "Error: function to insert the user exception into an Any is not available\n";
    log.flush();
    OMNIORB_THROW(INTERNAL,0,CORBA::COMPLETED_NO);
  }
}

//////////////////////////////////////////////////////////////////////
////////////// Implementation of standard UserException //////////////
//////////////////////////////////////////////////////////////////////

OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA, WrongTransaction,
		       "IDL:omg.org/CORBA/WrongTransaction:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ContextList, Bounds,
		       "IDL:omg.org/CORBA/ContextList/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ExceptionList, Bounds,
		       "IDL:omg.org/CORBA/ExceptionList/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::NVList, Bounds,
		       "IDL:omg.org/CORBA/NVList/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::TypeCode, Bounds,
		       "IDL:omg.org/CORBA/TypeCode/Bounds:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::TypeCode, BadKind,
		       "IDL:omg.org/CORBA/TypeCode/BadKind:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(CORBA::ORB, InconsistentTypeCode,
		       "IDL:omg.org/CORBA/ORB/InconsistentTypeCode:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(DynamicAny::DynAny, InvalidValue,
		       "IDL:omg.org/DynamicAny/DynAny/InvalidValue:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(DynamicAny::DynAny, TypeMismatch,
		       "IDL:omg.org/DynamicAny/DynAny/TypeMismatch:1.0")
OMNIORB_DEFINE_USER_EX_WITHOUT_MEMBERS(DynamicAny::DynAnyFactory,
				       InconsistentTypeCode,
             "IDL:omg.org/DynamicAny/DynAnyFactory/InconsistentTypeCode:1.0")

//////////////////////////////////////////////////////////////////////
////////////// Implementation of User   Exception Any operators //////
//////////////////////////////////////////////////////////////////////

static
CORBA::Boolean 
getMemberDesc_UserException(const tcStructDesc *,CORBA::ULong,tcDescriptor &)
{
  return 0;
}

static
CORBA::ULong
getMemberCount_UserException(const tcStructDesc *)
{
  return 0;
}

static
void
buildDesc_cUserException(tcDescriptor &_desc, 
			 const CORBA::UserException& _data)
{
  _desc.p_struct.getMemberDesc = getMemberDesc_UserException;
  _desc.p_struct.getMemberCount = getMemberCount_UserException;
  _desc.p_struct.opq_struct = (void *)&_data;
}

static
void
delete_UserException(void* _data) {
  CORBA::UserException* _0RL_t = (CORBA::UserException*) _data;
  delete _0RL_t;
}

static
void
insertUserExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			 const CORBA::UserException&  s)
{
  tcDescriptor _0RL_tcdesc;
  buildDesc_cUserException(_0RL_tcdesc, s);
  a.PR_packFrom(tc, &_0RL_tcdesc);
}

static
void
insertUserExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			   const CORBA::UserException*  s)
{
  tcDescriptor _0RL_tcdesc;
  buildDesc_cUserException(_0RL_tcdesc, *s);
  a.PR_packFrom(tc, &_0RL_tcdesc);
  delete (CORBA::UserException*)s;
}

static
CORBA::Boolean
extractUserExceptionFromAny(const CORBA::Any& a, CORBA::TypeCode_ptr tc,
			    CORBA::UserException*& s,
			    CORBA::Boolean cacheOnly)
{
  if (cacheOnly) {
    s = (CORBA::UserException *) a.PR_getCachedData();
    if (s) {
      CORBA::TypeCode_var a_tc = a.type();
      if (a_tc->equivalent(tc))
	return 1;
    }
  }
  else {
    tcDescriptor _0RL_tcdesc;
    buildDesc_cUserException(_0RL_tcdesc,*s);
    if (a.PR_unpackTo(tc, &_0RL_tcdesc)) {
      ((CORBA::Any *)&a)->PR_setCachedData(s,delete_UserException);
      return 1;
    }
  }
  return 0;
}

#define USER_EXCEPTION_1(name) \
      USER_EXCEPTION(CORBA,name,name,name,#name)

#define USER_EXCEPTION_2(scope,name) \
      USER_EXCEPTION(CORBA,scope::name,scope##_##name,name,#scope "/" #name)

#define USER_EXCEPTION_MOD_2(module,scope,name) \
      USER_EXCEPTION(module,scope::name,scope##_##name,name,#scope "/" #name)

#define USER_EXCEPTION(module,fqname,_fqname,uqname,repostr) \
static const CORBA::TypeCode_ptr _tc_##module##_##_fqname = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/" #module "/" repostr ":1.0", #uqname, (CORBA::PR_structMember*) 0, 0); \
 \
void operator<<=(CORBA::Any& _a, const module::fqname& _s) { \
  insertUserExceptionToAny(_a,_tc_##module##_##_fqname,_s); \
} \
 \
void operator<<=(CORBA::Any& _a, const module::fqname* _sp) { \
  insertUserExceptionToAny(_a,_tc_##module##_##_fqname,_sp); \
} \
 \
CORBA::Boolean operator>>=(const CORBA::Any& _a, const module::fqname*& _sp) { \
  CORBA::UserException* s = 0; \
  if (extractUserExceptionFromAny(_a,_tc_##module##_##_fqname,s,1)) { \
    _sp = (module::fqname *)s; \
    return 1; \
  } \
  else { \
    if (s == 0) { \
      s = new module::fqname; \
      if (extractUserExceptionFromAny(_a,_tc_##module##_##_fqname,s,0)) { \
	_sp = (module::fqname *)s; \
	return 1; \
      } \
      else { \
	delete s; \
      } \
    } \
  } \
  _sp = 0; \
  return 0; \
} \
 \
static \
void \
module##_##_fqname##_insertToAny(CORBA::Any& a,const CORBA::Exception& e) \
{ \
  const module::fqname & ex = (const module::fqname &) e; \
  insertUserExceptionToAny(a,_tc_##module##_##_fqname,ex); \
} \
 \
static \
void \
module##_##_fqname##_insertToAnyNCP(CORBA::Any& a,const CORBA::Exception* e) \
{ \
  const module::fqname * ex = (const module::fqname *) e; \
  insertUserExceptionToAny(a,_tc_##module##_##_fqname,ex); \
}


USER_EXCEPTION_1 (WrongTransaction)
USER_EXCEPTION_2 (ContextList,Bounds)
USER_EXCEPTION_2 (ExceptionList,Bounds)
USER_EXCEPTION_2 (NVList,Bounds)
USER_EXCEPTION_2 (TypeCode,Bounds)
USER_EXCEPTION_2 (TypeCode,BadKind)
USER_EXCEPTION_2 (ORB,InconsistentTypeCode)
USER_EXCEPTION_2 (ORB,InvalidName)
USER_EXCEPTION_MOD_2 (PortableServer,POAManager,AdapterInactive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterAlreadyExists)
USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterNonExistent)
USER_EXCEPTION_MOD_2 (PortableServer,POA,NoServant)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectAlreadyActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectNotActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantAlreadyActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantNotActive)
USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongAdapter)
USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongPolicy)
USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,InvalidValue)
USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,TypeMismatch)
USER_EXCEPTION_MOD_2 (DynamicAny,DynAnyFactory,InconsistentTypeCode)

#undef USER_EXCEPTION
#undef USER_EXCEPTION_1
#undef USER_EXCEPTION_2
#undef USER_EXCEPTION_MOD_2


//////////////////////////////////////////////////////////////////////
////////////// Exceptions with members from PortableServer ///////////
//////////////////////////////////////////////////////////////////////

// This ugly code is generated by omniidl...

static CORBA::PR_structMember
_0RL_structmember_PortableServer_mForwardRequest[] = {
  {"forward_reference", CORBA::TypeCode::PR_Object_tc()}
};
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mForwardRequest = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/PortableServer/ForwardRequest:1.0", "ForwardRequest", _0RL_structmember_PortableServer_mForwardRequest, 1);

CORBA::Boolean
_0RL_tcParser_getMemberDesc_PortableServer_mForwardRequest(const tcStructDesc *_desc, CORBA::ULong _index, tcDescriptor &_newdesc){
  switch (_index) {
  case 0:
    _0RL_buildDesc_cCORBA_mObject(_newdesc, ((PortableServer::ForwardRequest*)_desc->opq_struct)->forward_reference);
    return 1;
  default:
    return 0;
  };
}
CORBA::ULong
_0RL_tcParser_getMemberCount_PortableServer_mForwardRequest(const tcStructDesc *_desc)
{
  return 1;
}

void _0RL_buildDesc_cPortableServer_mForwardRequest(tcDescriptor &_desc, const PortableServer::ForwardRequest& _data)
{
  _desc.p_struct.getMemberDesc = _0RL_tcParser_getMemberDesc_PortableServer_mForwardRequest;
  _desc.p_struct.getMemberCount = _0RL_tcParser_getMemberCount_PortableServer_mForwardRequest;
  _desc.p_struct.opq_struct = (void *)&_data;
}

void _0RL_delete_PortableServer_mForwardRequest(void* _data) {
  PortableServer::ForwardRequest* _0RL_t = (PortableServer::ForwardRequest*) _data;
  delete _0RL_t;
}

void operator<<=(CORBA::Any& _a, const PortableServer::ForwardRequest& _s) {
  tcDescriptor _0RL_tcdesc;
  _0RL_buildDesc_cPortableServer_mForwardRequest(_0RL_tcdesc, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mForwardRequest, &_0RL_tcdesc);
}

void operator<<=(CORBA::Any& _a, const PortableServer::ForwardRequest* _sp) {
  tcDescriptor _0RL_tcdesc;
  _0RL_buildDesc_cPortableServer_mForwardRequest(_0RL_tcdesc, *_sp);
  _a.PR_packFrom(_0RL_tc_PortableServer_mForwardRequest, &_0RL_tcdesc);
  delete (PortableServer::ForwardRequest *)_sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a,const PortableServer::ForwardRequest*& _sp) {
  _sp = (PortableServer::ForwardRequest *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor _0RL_tcdesc;
    _sp = new PortableServer::ForwardRequest;
    _0RL_buildDesc_cPortableServer_mForwardRequest(_0RL_tcdesc, *_sp);
    if (_a.PR_unpackTo(_0RL_tc_PortableServer_mForwardRequest, &_0RL_tcdesc)) {
      ((CORBA::Any *)&_a)->PR_setCachedData((void*)_sp, _0RL_delete_PortableServer_mForwardRequest);
      return 1;
    } else {
      delete (PortableServer::ForwardRequest *)_sp;_sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var _0RL_tctmp = _a.type();
    if (_0RL_tctmp->equivalent(_0RL_tc_PortableServer_mForwardRequest)) return 1;
    delete (PortableServer::ForwardRequest *)_sp;_sp = 0;
    return 0;
  }
}

static void PortableServer_ForwardRequest_insertToAny(CORBA::Any& _a,const CORBA::Exception& _e) {
  const PortableServer::ForwardRequest & _ex = (const PortableServer::ForwardRequest &) _e;
  operator<<=(_a,_ex);
}

static void PortableServer_ForwardRequest_insertToAnyNCP(CORBA::Any& _a,const CORBA::Exception* _e) {
  const PortableServer::ForwardRequest * _ex = (const PortableServer::ForwardRequest *) _e;
  operator<<=(_a,_ex);
}

static CORBA::PR_structMember _0RL_structmember_PortableServer_mPOA_mInvalidPolicy[] = {
  {"index", CORBA::TypeCode::PR_ushort_tc()}
};
static CORBA::TypeCode_ptr _0RL_tc_PortableServer_mPOA_mInvalidPolicy = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/PortableServer/POA/InvalidPolicy:1.0", "InvalidPolicy", _0RL_structmember_PortableServer_mPOA_mInvalidPolicy, 1);

CORBA::Boolean
_0RL_tcParser_getMemberDesc_PortableServer_mPOA_mInvalidPolicy(const tcStructDesc *_desc, CORBA::ULong _index, tcDescriptor &_newdesc){
  switch (_index) {
  case 0:
    _0RL_buildDesc_cunsigned_pshort(_newdesc, ((PortableServer::POA::InvalidPolicy*)_desc->opq_struct)->index);
    return 1;
  default:
    return 0;
  };
}
CORBA::ULong
_0RL_tcParser_getMemberCount_PortableServer_mPOA_mInvalidPolicy(const tcStructDesc *_desc)
{
  return 1;
}

void _0RL_buildDesc_cPortableServer_mPOA_mInvalidPolicy(tcDescriptor &_desc, const PortableServer::POA::InvalidPolicy& _data)
{
  _desc.p_struct.getMemberDesc = _0RL_tcParser_getMemberDesc_PortableServer_mPOA_mInvalidPolicy;
  _desc.p_struct.getMemberCount = _0RL_tcParser_getMemberCount_PortableServer_mPOA_mInvalidPolicy;
  _desc.p_struct.opq_struct = (void *)&_data;
}

void _0RL_delete_PortableServer_mPOA_mInvalidPolicy(void* _data) {
  PortableServer::POA::InvalidPolicy* _0RL_t = (PortableServer::POA::InvalidPolicy*) _data;
  delete _0RL_t;
}

void operator<<=(CORBA::Any& _a, const PortableServer::POA::InvalidPolicy& _s) {
  tcDescriptor _0RL_tcdesc;
  _0RL_buildDesc_cPortableServer_mPOA_mInvalidPolicy(_0RL_tcdesc, _s);
  _a.PR_packFrom(_0RL_tc_PortableServer_mPOA_mInvalidPolicy, &_0RL_tcdesc);
}

void operator<<=(CORBA::Any& _a, const PortableServer::POA::InvalidPolicy* _sp) {
  tcDescriptor _0RL_tcdesc;
  _0RL_buildDesc_cPortableServer_mPOA_mInvalidPolicy(_0RL_tcdesc, *_sp);
  _a.PR_packFrom(_0RL_tc_PortableServer_mPOA_mInvalidPolicy, &_0RL_tcdesc);
  delete (PortableServer::POA::InvalidPolicy *)_sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a,const PortableServer::POA::InvalidPolicy*& _sp) {
  _sp = (PortableServer::POA::InvalidPolicy *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor _0RL_tcdesc;
    _sp = new PortableServer::POA::InvalidPolicy;
    _0RL_buildDesc_cPortableServer_mPOA_mInvalidPolicy(_0RL_tcdesc, *_sp);
    if (_a.PR_unpackTo(_0RL_tc_PortableServer_mPOA_mInvalidPolicy, &_0RL_tcdesc)) {
      ((CORBA::Any *)&_a)->PR_setCachedData((void*)_sp, _0RL_delete_PortableServer_mPOA_mInvalidPolicy);
      return 1;
    } else {
      delete (PortableServer::POA::InvalidPolicy *)_sp;_sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var _0RL_tctmp = _a.type();
    if (_0RL_tctmp->equivalent(_0RL_tc_PortableServer_mPOA_mInvalidPolicy)) return 1;
    delete (PortableServer::POA::InvalidPolicy *)_sp;_sp = 0;
    return 0;
  }
}

static void PortableServer_POA_InvalidPolicy_insertToAny(CORBA::Any& _a,const CORBA::Exception& _e) {
  const PortableServer::POA::InvalidPolicy & _ex = (const PortableServer::POA::InvalidPolicy &) _e;
  operator<<=(_a,_ex);
}

static void PortableServer_POA_InvalidPolicy_insertToAnyNCP(CORBA::Any& _a,const CORBA::Exception* _e) {
  const PortableServer::POA::InvalidPolicy * _ex = (const PortableServer::POA::InvalidPolicy *) _e;
  operator<<=(_a,_ex);
}



//////////////////////////////////////////////////////////////////////
////////////// Implementation of System Exception Any operators //////
//////////////////////////////////////////////////////////////////////

static const char* mCompletionStatus[] = { "COMPLETED_YES", 
					   "COMPLETED_NO", 
					   "COMPLETED_MAYBE"};

static CORBA::TypeCode_ptr _tc_CompletionStatus =
CORBA::TypeCode::PR_enum_tc("IDL:omg.org/CORBA/completion_status:1.0",
			    "completion_status", mCompletionStatus, 3);


static CORBA::PR_structMember mSystemException[] = {
  {"minor", CORBA::TypeCode::PR_ulong_tc()},
  {"completed", _tc_CompletionStatus}
};


struct _SystemException {
  CORBA::ULong            minor;
  CORBA::CompletionStatus completed;
};

static 
void 
buildDesc_mCompletionStatus(tcDescriptor& _desc, 
			    const CORBA::CompletionStatus& _data)
{
  _desc.p_enum.data = (void*)&_data;
  _desc.p_enum.size = sizeof(_data);
}

static
CORBA::Boolean 
getMemberDesc_mSystemException(const tcStructDesc *_desc,
			       CORBA::ULong _index,
			       tcDescriptor &_newdesc)
{
  switch( _index ) {
  case 0:
    _0RL_buildDesc_cunsigned_plong(_newdesc,
				   ((_SystemException*)
				    _desc->opq_struct)->minor);
    return 1;
  case 1:
    buildDesc_mCompletionStatus(_newdesc,
				((_SystemException*)
				 _desc->opq_struct)->completed);
    return 1;
  default:
    return 0;
  };
}

static
CORBA::ULong
getMemberCount_mSystemException(const tcStructDesc *_desc)
{
  return 2;
}

static
void
buildDesc_mSystemException(tcDescriptor &_desc,
			   const _SystemException& _data)
{
  _desc.p_struct.getMemberDesc = getMemberDesc_mSystemException;
  _desc.p_struct.getMemberCount = getMemberCount_mSystemException;
  _desc.p_struct.opq_struct = (void *)&_data;
}

static
void 
delete_mSystemException(void* _data) {
  CORBA::SystemException* _0RL_t = (CORBA::SystemException*) _data;
  delete _0RL_t;
}

static
void
insertSystemExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			   const CORBA::SystemException&  s)
{
  tcDescriptor _0RL_tcdesc;
  _SystemException ex;
  ex.minor = s.minor();
  ex.completed = s.completed();
  buildDesc_mSystemException(_0RL_tcdesc, ex);
  a.PR_packFrom(tc, &_0RL_tcdesc);
}

static
void
insertSystemExceptionToAny(CORBA::Any& a, CORBA::TypeCode_ptr tc,
			   const CORBA::SystemException*  s)
{
  tcDescriptor _0RL_tcdesc;
  _SystemException ex;
  ex.minor = s->minor();
  ex.completed = s->completed();
  buildDesc_mSystemException(_0RL_tcdesc, ex);
  a.PR_packFrom(tc, &_0RL_tcdesc);
  delete (CORBA::SystemException*)s;
}

static
CORBA::Boolean
extractSystemExceptionFromAny(const CORBA::Any& a, CORBA::TypeCode_ptr tc,
			      CORBA::SystemException*& s,
			      CORBA::Boolean cacheOnly)
{
  if (cacheOnly) {
    s = (CORBA::SystemException *) a.PR_getCachedData();
    if (s) {
      CORBA::TypeCode_var a_tc = a.type();
      if (a_tc->equivalent(tc))
	return 1;
    }
  }
  else {
    tcDescriptor _0RL_tcdesc;
    _SystemException ex;
    buildDesc_mSystemException(_0RL_tcdesc,ex);
    if (a.PR_unpackTo(tc, &_0RL_tcdesc)) {
      s->minor(ex.minor);
      s->completed(ex.completed);
      ((CORBA::Any *)&a)->PR_setCachedData(s,
					   delete_mSystemException);
      return 1;
    }
  }
  return 0;
}


#define STD_EXCEPTION(name) \
CORBA::TypeCode_ptr CORBA::_tc_##name = CORBA::TypeCode::PR_exception_tc("IDL:omg.org/CORBA/" #name ":1.0", #name, mSystemException, 2); \
 \
void operator<<=(CORBA::Any& _a, const CORBA::name & _s) { \
  insertSystemExceptionToAny(_a,CORBA::_tc_##name,_s); \
}  \
 \
void operator<<=(CORBA::Any& _a, const CORBA::name * _sp) { \
  insertSystemExceptionToAny(_a,CORBA::_tc_##name,_sp); \
}\
 \
CORBA::Boolean \
operator>>=(const CORBA::Any& _a, const CORBA::name *& _sp) \
{ \
  CORBA::SystemException* s = 0; \
  if (extractSystemExceptionFromAny(_a,CORBA::_tc_##name,s,1)) { \
    _sp = (CORBA::name *)s; \
    return 1; \
  } \
  else { \
    if (s == 0) { \
      s = new CORBA::name; \
      if (extractSystemExceptionFromAny(_a,CORBA::_tc_##name,s,0)) { \
	_sp = (CORBA::name *)s; \
	return 1; \
      } \
      else { \
	delete s; \
      } \
    } \
  } \
  _sp = 0; \
  return 0; \
} \
 \
static \
void \
name##_insertToAny(CORBA::Any& a,const CORBA::Exception& e) \
{ \
  const CORBA::name & ex = (const CORBA::name &) e; \
  insertSystemExceptionToAny(a,CORBA::_tc_##name,ex); \
} \
 \
static \
void \
name##_insertToAnyNCP(CORBA::Any& a,const CORBA::Exception* e) \
{ \
  const CORBA::name * ex = (const CORBA::name *) e; \
  insertSystemExceptionToAny(a,CORBA::_tc_##name,ex); \
}


OMNIORB_FOR_EACH_SYS_EXCEPTION(STD_EXCEPTION)
#undef STD_EXCEPTION


//////////////////////////////////////////////////////////////////////
////////////// Once only initializer to set up all insertToAnyFn /////
////////////// and insertToAnyFnNCP static variables             /////
//////////////////////////////////////////////////////////////////////


// XXX This should really be done using the initializer mechanism as in
//     in orbcore.
class dynException_Singleton {
public:

  void attach() {

#define STD_EXCEPTION(name) \
CORBA::name::insertToAnyFn    = name##_insertToAny; \
CORBA::name::insertToAnyFnNCP = name##_insertToAnyNCP;

    OMNIORB_FOR_EACH_SYS_EXCEPTION(STD_EXCEPTION)
#undef STD_EXCEPTION


#define USER_EXCEPTION_1(name) \
    USER_EXCEPTION(CORBA,name,name)

#define USER_EXCEPTION_2(scope,name) \
    USER_EXCEPTION(CORBA,scope::name,scope##_##name)

#define USER_EXCEPTION_MOD_1(module,name) \
    USER_EXCEPTION(module,name,name)

#define USER_EXCEPTION_MOD_2(module,scope,name) \
    USER_EXCEPTION(module,scope::name,scope##_##name)

#define USER_EXCEPTION(module,fqname,_fqname) \
module::fqname::insertToAnyFn    = module##_##_fqname##_insertToAny; \
module::fqname::insertToAnyFnNCP = module##_##_fqname##_insertToAnyNCP

    USER_EXCEPTION_1 (WrongTransaction);
    USER_EXCEPTION_2 (ContextList,Bounds);
    USER_EXCEPTION_2 (ExceptionList,Bounds);
    USER_EXCEPTION_2 (NVList,Bounds);
    USER_EXCEPTION_2 (TypeCode,Bounds);
    USER_EXCEPTION_2 (TypeCode,BadKind);
    USER_EXCEPTION_2 (ORB,InconsistentTypeCode);
    USER_EXCEPTION_2 (ORB,InvalidName);
    USER_EXCEPTION_MOD_1 (PortableServer,ForwardRequest);
    USER_EXCEPTION_MOD_2 (PortableServer,POAManager,AdapterInactive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterAlreadyExists);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,AdapterNonExistent);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,InvalidPolicy);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,NoServant);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectAlreadyActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ObjectNotActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantAlreadyActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,ServantNotActive);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongAdapter);
    USER_EXCEPTION_MOD_2 (PortableServer,POA,WrongPolicy);
    USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,InvalidValue);
    USER_EXCEPTION_MOD_2 (DynamicAny,DynAny,TypeMismatch);
    USER_EXCEPTION_MOD_2 (DynamicAny,DynAnyFactory,InconsistentTypeCode);

#undef USER_EXCEPTION_1
#undef USER_EXCEPTION_2
#undef USER_EXCEPTION_MOD_1
#undef USER_EXCEPTION_MOD_2
#undef USER_EXCEPTION

      }

  dynException_Singleton() {
    attach();
  }
  ~dynException_Singleton() {}
};

static dynException_Singleton dynException_Singleton_;

