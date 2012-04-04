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

const char* omniAMI::PollerImpl::_PD_repoId = "omni:AMI/PollerImpl/1.0";

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
  return new PollableSetImpl(this);
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


//
// PollableSet

omniAMI::PollableSetImpl::
PollableSetImpl(PollerImpl* poller)
  : pd_cond(&omniAsyncCallDescriptor::sd_lock),
    pd_dii_pollable(0),
    pd_ref_count(1)
{
  omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

  if (poller->_PR_cd()->addToSet(&pd_cond)) {
    poller->_add_ref();
    pd_ami_pollers.length(1);
    pd_ami_pollers[0] = poller;
  }
  else {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PollableAlreadyInPollableSet,
                  CORBA::COMPLETED_NO);
  }
}

omniAMI::PollableSetImpl::
~PollableSetImpl()
{
  omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

  for (CORBA::ULong i=0; i != pd_ami_pollers.length(); ++i)
    pd_ami_pollers[i]->_PR_cd()->remFromSet(&pd_cond);
}


CORBA::DIIPollable*
omniAMI::PollableSetImpl::
create_dii_pollable()
{
  OMNIORB_THROW(NO_IMPLEMENT, 0, CORBA::COMPLETED_NO);
}


void
omniAMI::PollableSetImpl::
add_pollable(CORBA::Pollable* potential)
{
  PollerImpl* impl = 
    potential ? (PollerImpl*)potential->_ptrToValue(PollerImpl::_PD_repoId) : 0;

  if (impl) {
    omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

    if (impl->_PR_cd()->addToSet(&pd_cond)) {
      impl->_add_ref();

      CORBA::ULong len = pd_ami_pollers.length();
      pd_ami_pollers.length(len+1);
      pd_ami_pollers[len] = impl;
    }
    else {
      OMNIORB_THROW(BAD_PARAM, BAD_PARAM_PollableAlreadyInPollableSet,
                    CORBA::COMPLETED_NO);
    }
  }
  else {
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidPollerType,
                  CORBA::COMPLETED_NO);
  }
}


CORBA::Pollable*
omniAMI::PollableSetImpl::
get_ready_pollable(CORBA::ULong timeout)
{
  omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

  CORBA::Pollable* pollable = getAndRemoveReadyPollable();
  if (pollable)
    return pollable;

  if (timeout == 0)
    OMNIORB_THROW(NO_RESPONSE, NO_RESPONSE_ReplyNotAvailableYet,
                  CORBA::COMPLETED_NO);

  if (timeout == 0xffffffff) {
    do {
      pd_cond.wait();

      pollable = getAndRemoveReadyPollable();
      if (pollable)
        return pollable;

    } while(1);
  }

  omni_time_t timeout_tt(timeout / 1000, (timeout % 1000) * 1000000);
  omni_time_t deadline;
  omni_thread::get_time(deadline, timeout_tt);

  pd_cond.timedwait(deadline);

  pollable = getAndRemoveReadyPollable();
  if (!pollable)
    OMNIORB_THROW(TIMEOUT, TIMEOUT_NoPollerResponseInTime, CORBA::COMPLETED_NO);

  return pollable;
}


void
omniAMI::PollableSetImpl::
remove(CORBA::Pollable* potential)
{
  omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

  PollerImpl* impl = 
    potential ? (PollerImpl*)potential->_ptrToValue(PollerImpl::_PD_repoId) : 0;

  if (!impl)
    OMNIORB_THROW(BAD_PARAM, BAD_PARAM_InvalidPollerType,
                  CORBA::COMPLETED_NO);


  CORBA::ULong len = pd_ami_pollers.length();

  for (CORBA::ULong i=0; i != len; ++i) {
    PollerImpl* poller = pd_ami_pollers[i];
      
    if (poller == impl) {
      if (i + 1 < len) {
        // Swap last item in sequence with this one
        pd_ami_pollers[i] = pd_ami_pollers[len-1];
      }
      pd_ami_pollers.length(len-1);
      return;
    }
  }
  throw UnknownPollable();
}


CORBA::UShort
omniAMI::PollableSetImpl::
number_left()
{
  omni_tracedmutex_lock l(omniAsyncCallDescriptor::sd_lock);

  if (pd_ami_pollers.length() > 0xffff)
    return 0xffff; // Yikes!

  return pd_ami_pollers.length();
}


CORBA::Pollable*
omniAMI::PollableSetImpl::
getAndRemoveReadyPollable()
{
  ASSERT_OMNI_TRACEDMUTEX_HELD(omniAsyncCallDescriptor::sd_lock, 1);

  CORBA::ULong len = pd_ami_pollers.length();
  if (len == 0)
    throw NoPossiblePollable();

  for (CORBA::ULong i=0; i != len; ++i) {
    PollerImpl* poller = pd_ami_pollers[i];
      
    if (poller->_PR_cd()->lockedIsComplete()) {
      poller->_add_ref();

      if (i + 1 < len) {
        // Swap last item in sequence with this one
        pd_ami_pollers[i] = pd_ami_pollers[len-1];
      }
      pd_ami_pollers.length(len-1);
      return poller;
    }
  }
  return 0;
}


void
omniAMI::PollableSetImpl::
_add_ref()
{
  pd_ref_count.inc();
}

void
omniAMI::PollableSetImpl::
_remove_ref()
{
  if (pd_ref_count.dec() > 0)
    return;

  delete this;
}
