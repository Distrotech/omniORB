// -*- Mode: C++; -*-
//                            Package   : omniORB
// ami.h                      Created on: 2012-02-06
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2012 Apasphere Ltd
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
// Description:
//
//    AMI support

#include <omniORB4/messaging.hh>
#include <omniORB4/callDescriptor.h>
#include <omniORB4/ami.h>

OMNI_USING_NAMESPACE(omni)

//
// ExceptionHolder

omniAMI::ExceptionHolder::
~ExceptionHolder()
{
}

void
omniAMI::ExceptionHolder::
raise_exception()
{
  pd_cd->raiseException();
}


//
// Poller

omniAMI::PollerImpl::
~PollerImpl()
{
  delete _pd_cd;
}

CORBA::Boolean
omniAMI::PollerImpl::
is_ready(CORBA::ULong timeout)
{
  if (timeout == 0)
    return _pd_cd->isComplete();

  if (timeout == 0xffffffff) {
    _pd_cd->wait();
    return 1;
  }

  omni_time_t timeout_tt(timeout / 1000, (timeout % 1000) * 1000000);
  omni_time_t deadline;
  omni_thread::get_time(deadline, timeout_tt);

  return _pd_cd->wait(deadline);
}


CORBA::PollableSet_ptr
omniAMI::PollerImpl::
create_pollable_set()
{
  // *** HERE
  OMNIORB_THROW(NO_IMPLEMENT, 0, CORBA::COMPLETED_NO);
}


CORBA::Object_ptr
omniAMI::PollerImpl::
operation_target()
{
  omniObjRef* objref = _pd_cd->objref();
  if (objref)
    return CORBA::Object::
      _duplicate((CORBA::Object_ptr)objref->
		 _ptrToObjRef(CORBA::Object::_PD_repoId));
  else
    return CORBA::Object::_nil();
}

char*
omniAMI::PollerImpl::
operation_name()
{
  return CORBA::string_dup(_pd_cd->op());
}

Messaging::ReplyHandler_ptr
omniAMI::PollerImpl::
associated_handler()
{
  Messaging::ReplyHandler_ptr rh;
  rh = Messaging::ReplyHandler::_fromObjRef(_pd_cd->getHandler());
  return Messaging::ReplyHandler::_duplicate(rh);
}

void
omniAMI::PollerImpl::
associated_handler(Messaging::ReplyHandler_ptr v)
{
  _pd_cd->setHandler(v->_PR_getobj());
}

CORBA::Boolean
omniAMI::PollerImpl::
is_from_poller()
{
  return _pd_is_from_poller;
}

void
omniAMI::PollerImpl::
_wrongOperation()
{
  if (omniORB::trace(5)) {
    omniORB::logger log;
    log << "Wrong operation called on poller expecting '" << _pd_cd->op()
	<< "'.\n";
  }
  _pd_is_from_poller = 1;

  OMNIORB_THROW(BAD_OPERATION,
		BAD_OPERATION_WrongPollerOperation,
		CORBA::COMPLETED_NO);
}


void
omniAMI::PollerImpl::
_checkResult(const char* op, CORBA::ULong timeout)
{
  // Operation name uses a static pointer so we can compare pointers
  if (_pd_cd->op() != op)
    _wrongOperation();

  if (_pd_retrieved) {
    _pd_is_from_poller = 1;
    OMNIORB_THROW(OBJECT_NOT_EXIST,
		  OBJECT_NOT_EXIST_PollerAlreadyDeliveredReply,
		  CORBA::COMPLETED_NO);
  }

  if (!is_ready(timeout)) {
    _pd_is_from_poller = 1;

    if (timeout == 0)
      OMNIORB_THROW(NO_RESPONSE,
		    NO_RESPONSE_ReplyNotAvailableYet,
		    CORBA::COMPLETED_NO);

    else
      OMNIORB_THROW(TIMEOUT,
		    TIMEOUT_NoPollerResponseInTime,
		    CORBA::COMPLETED_NO);
  }

  if (_pd_cd->exceptionOccurred())
    _pd_cd->raiseException();
}
