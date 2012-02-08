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

#ifndef _OMNIORB_AMI_H_
#define _OMNIORB_AMI_H_

_CORBA_MODULE omniAMI
_CORBA_MODULE_BEG


//
// Implementation of standard Messaging::ExceptionHolder valuetype

class ExceptionHolder
  : public virtual ::Messaging::ExceptionHolder,
    public virtual ::CORBA::DefaultValueRefCountBase
{
public:
  inline ExceptionHolder(omniAsyncCallDescriptor* cd)
    : pd_cd(cd)
  {}

  virtual ~ExceptionHolder();

  virtual void raise_exception();

private:
  omniAsyncCallDescriptor* pd_cd;
};


_CORBA_MODULE_END  // omniAMI



#endif // _OMNIORB_AMI_H_
