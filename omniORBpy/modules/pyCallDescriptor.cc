// -*- Mode: C++; -*-
//                            Package   : omniORBpy
// pyCallDescriptor.cc        Created on: 2000/02/02
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2003-2012 Apasphere Ltd
//    Copyright (C) 2000 AT&T Laboratories Cambridge
//
//    This file is part of the omniORBpy library
//
//    The omniORBpy library is free software; you can redistribute it
//    and/or modify it under the terms of the GNU Lesser General
//    Public License as published by the Free Software Foundation;
//    either version 2.1 of the License, or (at your option) any later
//    version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
//    MA 02111-1307, USA
//
//
// Description:
//    Implementation of Python call descriptor object

#include <omnipy.h>
#include <pyThreadCache.h>
#include <omniORB4/IOP_C.h>

#ifdef HAS_Cplusplus_Namespace
namespace {
#endif
  class cdLockHolder {
  public:
    inline cdLockHolder(omniPy::Py_omniCallDescriptor* cd)
      : ul_(cd->unlocker()), cn_(0)
    {
      if (ul_)
        ul_->lock();
      else
        cn_ = omnipyThreadCache::acquire();
    }
    inline ~cdLockHolder() {
      if (ul_)
        ul_->unlock();
      else
        omnipyThreadCache::release(cn_);
    }
  private:
    omniPy::InterpreterUnlocker*  ul_;
    omnipyThreadCache::CacheNode* cn_;
  };
#ifdef HAS_Cplusplus_Namespace
};
#endif


OMNI_USING_NAMESPACE(omni)


//
// Python type used to hold call descriptor and implement AMI Poller.

extern "C" {

  struct pyCDObj {
    PyObject_HEAD
    omniPy::Py_omniCallDescriptor* cd;
    CORBA::Boolean                 from_poller;
    CORBA::Boolean                 retrieved;
  };

  static void
  pyCDObj_dealloc(pyCDObj* self)
  {
    delete self->cd;
    PyObject_Del((PyObject*)self);
  }

  static PyObject*
  pyCDObj_poll(pyCDObj* self, PyObject* args)
  {
    omniPy::Py_omniCallDescriptor* cd = self->cd;

    const char*  op;
    int          op_len;
    PyObject*    pytimeout;
    CORBA::ULong timeout;

    if (!PyArg_ParseTuple(args, (char*)"s#O", &op, &op_len, &pytimeout))
      return 0;

    timeout = PyLong_AsUnsignedLong(pytimeout);
    if (timeout == (CORBA::ULong)-1 && PyErr_Occurred())
      return 0;

    try {
      if ((CORBA::ULong)op_len+1 != cd->op_len() ||
          !omni::strMatch(op, cd->op())) {

        OMNIORB_THROW(BAD_OPERATION,
                      BAD_OPERATION_WrongPollerOperation,
                      CORBA::COMPLETED_NO);
      }

      if (self->retrieved) {
        OMNIORB_THROW(OBJECT_NOT_EXIST,
                      OBJECT_NOT_EXIST_PollerAlreadyDeliveredReply,
                      CORBA::COMPLETED_NO);
      }

      {
        omniPy::InterpreterUnlocker u;

        if (!cd->isReady(timeout)) {
          if (timeout == 0) {
            OMNIORB_THROW(NO_RESPONSE,
                          NO_RESPONSE_ReplyNotAvailableYet,
                          CORBA::COMPLETED_NO);
          }
          else {
            OMNIORB_THROW(TIMEOUT,
                          TIMEOUT_NoPollerResponseInTime,
                          CORBA::COMPLETED_NO);
          }
        }
      }

      self->retrieved = 1;

      if (cd->exceptionOccurred())
        return cd->raisePyException();

      return cd->result();
    }
    catch (CORBA::SystemException& ex) {
      // Only catches exceptions thrown by the poller.
      self->from_poller = 1;
      return omniPy::handleSystemException(ex);
    }
  }

  static PyObject*
  pyCDObj_is_ready(pyCDObj* self, PyObject* args)
  {
    PyObject*    pytimeout;
    CORBA::ULong timeout;

    if (!PyArg_ParseTuple(args, (char*)"O", &pytimeout))
      return 0;
    
    timeout = PyLong_AsUnsignedLong(pytimeout);
    if (timeout == (CORBA::ULong)-1 && PyErr_Occurred())
      return 0;

    CORBA::Boolean result;
    {
      omniPy::InterpreterUnlocker u;
      result = self->cd->isReady(timeout);
    }
    return PyBool_FromLong(result);
  }

  static PyObject*
  pyCDObj_operation_target(pyCDObj* self, PyObject* args)
  {
    omniObjRef* objref = self->cd->objref();
    omni::duplicateObjRef(objref);

    CORBA::Object_ptr obj =
      (CORBA::Object_ptr)objref->_ptrToObjRef(CORBA::Object::_PD_repoId);

    return omniPy::createPyCorbaObjRef(0, obj);
  }

  static PyObject*
  pyCDObj_operation_name(pyCDObj* self, PyObject* args)
  {
    return PyString_FromString(self->cd->op());
  }

  static PyObject*
  pyCDObj_get_handler(pyCDObj* self, PyObject* args)
  {
    return self->cd->callback();
  }

  static PyObject*
  pyCDObj_set_handler(pyCDObj* self, PyObject* args)
  {
    PyObject* pyhandler;

    if (!PyArg_ParseTuple(args, (char*)"O", &pyhandler))
      return 0;
    
    self->cd->callback(pyhandler);
    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyObject*
  pyCDObj_is_from_poller(pyCDObj* self, PyObject* args)
  {
    return PyBool_FromLong(self->from_poller);
  }

  static PyObject*
  pyCDObj_raise_exception(pyCDObj* self, PyObject* args)
  {
    return self->cd->raisePyException();
  }

  static PyMethodDef pyCDObj_methods[] = {
    {(char*)"poll",             (PyCFunction)pyCDObj_poll,        METH_VARARGS},
    {(char*)"is_ready",         (PyCFunction)pyCDObj_is_ready,    METH_VARARGS},
    {(char*)"operation_target", (PyCFunction)pyCDObj_operation_target,
                                                                  METH_NOARGS},
    {(char*)"operation_name",   (PyCFunction)pyCDObj_operation_name,
                                                                  METH_NOARGS},
    {(char*)"get_handler",      (PyCFunction)pyCDObj_get_handler, METH_VARARGS},
    {(char*)"set_handler",      (PyCFunction)pyCDObj_set_handler, METH_VARARGS},
    {(char*)"is_from_poller",   (PyCFunction)pyCDObj_is_from_poller,
                                                                  METH_NOARGS},
    {(char*)"raise_exception",  (PyCFunction)pyCDObj_raise_exception,
                                                                  METH_NOARGS},
    {0,0}
  };

  static PyTypeObject pyCDType = {
    PyObject_HEAD_INIT(0)
    0,                                 /* ob_size */
    (char*)"_omnipy.pyCDObj",          /* tp_name */
    sizeof(pyCDObj),                   /* tp_basicsize */
    0,                                 /* tp_itemsize */
    (destructor)pyCDObj_dealloc,       /* tp_dealloc */
    0,                                 /* tp_print */
    0,                                 /* tp_getattr */
    0,                                 /* tp_setattr */
    0,                                 /* tp_compare */
    0,                                 /* tp_repr */
    0,                                 /* tp_as_number */
    0,                                 /* tp_as_sequence */
    0,                                 /* tp_as_mapping */
    0,                                 /* tp_hash  */
    0,                                 /* tp_call */
    0,                                 /* tp_str */
    0,                                 /* tp_getattro */
    0,                                 /* tp_setattro */
    0,                                 /* tp_as_buffer */
    0,                                 /* tp_flags */
    (char*)"Call descriptor",          /* tp_doc */
    0,                                 /* tp_traverse */
    0,                                 /* tp_clear */
    0,                                 /* tp_richcompare */
    0,                                 /* tp_weaklistoffset */
    0,                                 /* tp_iter */
    0,                                 /* tp_iternext */
    pyCDObj_methods,                   /* tp_methods */
  };


  static pyCDObj*
  pyCDObj_alloc(omniPy::Py_omniCallDescriptor* cd)
  {
    pyCDObj* self = PyObject_New(pyCDObj, &pyCDType);

    self->cd          = cd;
    self->from_poller = 0;
    self->retrieved   = 0;

    return self;
  }

}


void
omniPy::initCallDescriptor(PyObject* mod)
{
  int r = PyType_Ready(&pyCDType);
  OMNIORB_ASSERT(r == 0);
}


omniPy::Py_omniCallDescriptor::~Py_omniCallDescriptor()
{
  OMNIORB_ASSERT(!unlocker_);
}


//
// Client side

void
omniPy::Py_omniCallDescriptor::initialiseCall(cdrStream&)
{
  // initialiseCall() is called with the interpreter lock
  // released. Reacquire it so we can touch the descriptor objects
  // safely
  cdLockHolder _l(this);

  for (int i=0; i < in_l_; i++) {
    try {
      omniPy::validateType(PyTuple_GET_ITEM(in_d_,i),
                           PyTuple_GET_ITEM(args_,i),
                           CORBA::COMPLETED_NO);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Operation %r parameter %d",
                                  "si", op(), i));
      throw;
    }
  }
}


void
omniPy::Py_omniCallDescriptor::marshalArguments(cdrStream& stream)
{
  int i;
  if (in_marshal_) {
    omniORB::logs(25, "Python marshalArguments re-entered.");

    // marshalArguments can be re-entered when using GIOP 1.0, to
    // calculate the message size if the message is too big for a
    // single buffer. In that case, the interpreter lock has been
    // released by the PyUnlockingCdrStream, meaning the call
    // descriptor does not have the lock details. We have to use the
    // thread cache lock.

    omnipyThreadCache::lock _t;

    for (i=0; i < in_l_; i++)
      omniPy::marshalPyObject(stream,
                              PyTuple_GET_ITEM(in_d_,i),
                              PyTuple_GET_ITEM(args_,i));
    if (ctxt_d_.valid())
      omniPy::marshalContext(stream, ctxt_d_, PyTuple_GET_ITEM(args_, i));
  }
  else {
    cdLockHolder _l(this);

    in_marshal_ = 1;
    PyUnlockingCdrStream pystream(stream);

    try {
      for (i=0; i < in_l_; i++)
        omniPy::marshalPyObject(pystream,
                                PyTuple_GET_ITEM(in_d_,i),
                                PyTuple_GET_ITEM(args_,i));
      if (ctxt_d_.valid())
        omniPy::marshalContext(pystream, ctxt_d_, PyTuple_GET_ITEM(args_, i));
    }
    catch (...) {
      in_marshal_ = 0;
      throw;
    }
    in_marshal_ = 0;
  }
}


void
omniPy::Py_omniCallDescriptor::unmarshalReturnedValues(cdrStream& stream)
{
  if (out_l_ == -1) return;  // Oneway operation

  cdLockHolder _l(this);

  if (out_l_ == 0) {
    Py_INCREF(Py_None);
    result_ = Py_None;
  }
  else {
    PyUnlockingCdrStream pystream(stream);

    if (out_l_ == 1)
      result_ = omniPy::unmarshalPyObject(pystream,
                                          PyTuple_GET_ITEM(out_d_, 0));
    else {
      result_ = PyTuple_New(out_l_);
      if (!result_.valid())
        OMNIORB_THROW(NO_MEMORY, 0,
                      (CORBA::CompletionStatus)stream.completion());

      for (int i=0; i < out_l_; i++) {
        PyTuple_SET_ITEM(result_, i,
                         omniPy::unmarshalPyObject(pystream,
                                                   PyTuple_GET_ITEM(out_d_,
                                                                    i)));
      }
    }
  }
}


void
omniPy::Py_omniCallDescriptor::userException(cdrStream&  stream,
                                             IOP_C*      iop_client,
                                             const char* repoId)
{
  CORBA::Boolean skip = 0;

  try {
    cdLockHolder _l(this);

    PyObject* d_o = 0;

    if (exc_d_ != Py_None)
      d_o = PyDict_GetItemString(exc_d_, (char*)repoId);

    if (d_o) { // class, repoId, exc name, name, descriptor, ...
      PyUserException ex(d_o);
      ex <<= stream;
      ex._raise();
    }
    else {
      // Unexpected exception. Skip the remaining data in the reply
      // and throw UNKNOWN.
      skip = 1;
      OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException,
                    (CORBA::CompletionStatus)stream.completion());
    }
  }
  catch (...) {
    // The code above always throws, so this always executes.
    if (iop_client)
      iop_client->RequestCompleted(skip);
    throw;
  }
}


//
// AMI

static PyObject* pyEHClass = 0;

static inline PyObject*
getPyEHClass()
{
  if (!pyEHClass) {
    omniPy::PyRefHolder mod(PyImport_ImportModule((char*)"omniORB.ami"));
    
    if (mod.valid())
      pyEHClass = PyObject_GetAttrString(mod, (char*)"ExceptionHolderImpl");

    if (!pyEHClass) {
      if (omniORB::trace(1))
        PyErr_Print();
      else
        PyErr_Clear();
    }      
  }
  return pyEHClass;
}


void
omniPy::Py_omniCallDescriptor::completeCallback()
{
  omnipyThreadCache::lock _t;

  // If there is a poller, ensure our reference to it is released when
  // the function completes.
  PyRefHolder poller(poller_.retn());

  if (callback_.valid() && callback_.obj() != Py_None) {
    PyRefHolder method;
    PyRefHolder args;
    PyRefHolder result;

    if (!exceptionOccurred()) {
      method = PyObject_GetAttrString(callback_, (char*)op());
      if (PyTuple_Check(result_)) {
        args = result_.dup();
      }
      else {
        args = PyTuple_New(1);
        PyTuple_SET_ITEM(args, 0, result_.dup());
      }
    }
    else {
      // Exception. We need a poller.
      if (!poller.valid())
        poller = (PyObject*)pyCDObj_alloc(this);

      method = PyObject_GetAttr(callback_, excep_name_);

      PyObject* ehc = getPyEHClass();
      if (ehc) {
        PyObject* eh = PyObject_CallFunctionObjArgs(ehc, (PyObject*)poller, 0);
        if (eh) {
          args = PyTuple_New(1);
          PyTuple_SET_ITEM(args, 0, eh);
        }
      }
    }

    if (method.valid() && args.valid())
      result = PyObject_CallObject(method, args);

    if (!result.valid()) {
      if (omniORB::trace(1)) {
        omniORB::logs(1, "Exception performing AMI callback:");
        PyErr_Print();
      }
      else {
        PyErr_Clear();
      }
    }
  }

  if (!poller.valid()) {
    // No poller so this call descriptor is finished with.
    delete this;
  }
}


PyObject*
omniPy::Py_omniCallDescriptor::raisePyException()
{
  OMNIORB_ASSERT(pd_exception);

  PyUserException* ue = PyUserException::_downcast(pd_exception);
  if (ue)
    return ue->setPyExceptionState();

  CORBA::SystemException* se = CORBA::SystemException::_downcast(pd_exception);
  if (se)
    return handleSystemException(*se);

  try {
    OMNIORB_THROW(UNKNOWN, UNKNOWN_UserException, CORBA::COMPLETED_NO);
  }
  catch (CORBA::UNKNOWN& ex) {
    return handleSystemException(ex);
  }
}


PyObject*
omniPy::Py_omniCallDescriptor::makePoller()
{
  return (PyObject*)pyCDObj_alloc(this);
}


//
// Server-side

void
omniPy::Py_localCallBackFunction(omniCallDescriptor* cd, omniServant* svnt)
{
  Py_omniCallDescriptor* pycd = (Py_omniCallDescriptor*)cd;
  Py_omniServant*        pyos =
    (Py_omniServant*)svnt->_ptrToInterface(omniPy::string_Py_omniServant);

  // We can't use the call descriptor's unlocker to re-lock, because
  // this call-back may be running in a different thread to the
  // creator of the call descriptor.

  if (cd->is_upcall()) {
    omnipyThreadCache::lock _t;
    pyos->remote_dispatch(pycd);
  }
  else {
    omnipyThreadCache::lock _t;
    pyos->local_dispatch(pycd);
  }
}


void
omniPy::Py_omniCallDescriptor::unmarshalArguments(cdrStream& stream)
{
  OMNIORB_ASSERT(!args_.valid());

  omnipyThreadCache::lock _t;

  if (ctxt_d_.valid())
    args_ = PyTuple_New(in_l_ + 1);
  else
    args_ = PyTuple_New(in_l_);


  PyUnlockingCdrStream pystream(stream);

  int i;
  for (i=0; i < in_l_; i++) {
    PyTuple_SET_ITEM(args_, i,
                     omniPy::unmarshalPyObject(pystream,
                                               PyTuple_GET_ITEM(in_d_, i)));
  }
  if (ctxt_d_.valid())
    PyTuple_SET_ITEM(args_, i, omniPy::unmarshalContext(pystream));
}

void
omniPy::Py_omniCallDescriptor::setAndValidateReturnedValues(PyObject* result)
{
  OMNIORB_ASSERT(!result_.valid());
  result_ = result;

  if (out_l_ == -1 || out_l_ == 0) {
    if (result_ != Py_None) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_MAYBE,
                         omniPy::formatString("Operation %r should return "
                                              "None, got %r",
                                              "sO", op(), result->ob_type));
    }
  }
  else if (out_l_ == 1) {
    try {
      omniPy::validateType(PyTuple_GET_ITEM(out_d_,0),
                           result,
                           CORBA::COMPLETED_MAYBE);
    }
    catch (Py_BAD_PARAM& bp) {
      bp.add(omniPy::formatString("Operation %r return value",
                                  "s", op()));
      throw;
    }
  }
  else {
    if (!PyTuple_Check(result) || PyTuple_GET_SIZE(result) != out_l_) {
      THROW_PY_BAD_PARAM(BAD_PARAM_WrongPythonType, CORBA::COMPLETED_MAYBE,
                         omniPy::formatString("Operation %r should return "
                                              "%d-tuple, got %r",
                                              "siO",
                                              op(), out_l_, result->ob_type));
    }

    for (int i=0; i < out_l_; i++) {
      try {
        omniPy::validateType(PyTuple_GET_ITEM(out_d_,i),
                             PyTuple_GET_ITEM(result,i),
                             CORBA::COMPLETED_MAYBE);
      }
      catch (Py_BAD_PARAM& bp) {
        bp.add(omniPy::formatString("Operation %r return value %d",
                                    "si", op(), i));
        throw;
      }
    }
  }
}

void
omniPy::Py_omniCallDescriptor::marshalReturnedValues(cdrStream& stream)
{
  omnipyThreadCache::lock _t;
  PyUnlockingCdrStream pystream(stream);

  if (out_l_ == 1) {
    omniPy::marshalPyObject(pystream,
                            PyTuple_GET_ITEM(out_d_, 0),
                            result_);
  }
  else {
    for (int i=0; i < out_l_; i++) {
      omniPy::marshalPyObject(pystream,
                              PyTuple_GET_ITEM(out_d_,i),
                              PyTuple_GET_ITEM(result_,i));
    }
  }
}
