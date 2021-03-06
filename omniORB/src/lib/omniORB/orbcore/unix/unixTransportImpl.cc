// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixTransportImpl.cc       Created on: 6 Aug 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.2.6  2002/11/21 16:16:35  dgrisby
  Unix socket type bug. (Thanks Bastiaan Bakker.)

  Revision 1.1.2.5  2002/04/16 12:44:27  dpg1
  Fix SSL accept bug, clean up logging.

  Revision 1.1.2.4  2001/08/23 16:02:58  sll
  Implement getInterfaceAddress().

  Revision 1.1.2.3  2001/08/17 17:12:42  sll
  Modularise ORB configuration parameters.

  Revision 1.1.2.2  2001/08/08 15:59:23  sll
  Now accepts shorthand endpoint string "giop:unix:". Make use of
  unixTransportDirectory.

  Revision 1.1.2.1  2001/08/06 15:47:45  sll
  Added support to use the unix domain socket as the local transport.

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <objectAdapter.h>
#include <SocketCollection.h>
#include <orbParameters.h>
#include <unix/unixConnection.h>
#include <unix/unixAddress.h>
#include <unix/unixEndpoint.h>
#include <unix/unixTransportImpl.h>
#include <omniORB4/linkHacks.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>


OMNI_FORCE_LINK(unixAddress);
OMNI_FORCE_LINK(unixConnection);
OMNI_FORCE_LINK(unixEndpoint);
OMNI_FORCE_LINK(unixActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(unixTransportImpl);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
unixTransportImpl::unixTransportImpl() : giopTransportImpl("giop:unix") {
}

/////////////////////////////////////////////////////////////////////////
unixTransportImpl::~unixTransportImpl() {
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
unixTransportImpl::toEndpoint(const char* param) {

  if (!param)  return 0;

  CORBA::String_var dname;
  CORBA::String_var fname;
  struct stat sb;

  if (strlen(param) == 0) {
    param = orbParameters::unixTransportDirectory;
    
    char* p = (char*) strchr(param,'%');
    if (p && *(p+1) == 'u') {
      struct passwd* pw = getpwuid(getuid());
      if (!pw) {
	if (omniORB::trace(1)) {
	  omniORB::logger l;	
	  l << "Error: cannot get password entry of uid: " << getuid() << "\n";
	}
	return 0;
      }
      CORBA::String_var format = param;
      p = (char*) strchr(format,'%');
      *(p+1) = 's';
      dname = CORBA::string_alloc(strlen(format)+strlen(pw->pw_name));
      sprintf(dname,format,pw->pw_name);
      param = dname;
    }
    if (stat(param,&sb) == 0) {
      if (!S_ISDIR(sb.st_mode)) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;	
	  log << "Error: " << param << " exists and is not a directory. "
	      << "Please remove it and try again\n";
	}
	return 0;
      }
    }
    else {
      if (mkdir(param,0755) < 0) {
	if (omniORB::trace(1)) {
	  omniORB::logger log;	
	  log << "Error: cannot create directory: " << param << "\n";
	}
	return 0;
      }
    }
  }

  if (stat(param,&sb) == 0 && S_ISDIR(sb.st_mode)) {
    const char* format = "%s/%09u-%09u";
    fname = CORBA::string_alloc(strlen(param)+24);

    unsigned long now_sec, now_nsec;
    omni_thread::get_time(&now_sec,&now_nsec);
    
    sprintf(fname,format,param,(unsigned int)getpid(),(unsigned int)now_sec);
    param = fname;
  }

  return (giopEndpoint*)(new unixEndpoint(param));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixTransportImpl::isValid(const char* param) {

  if (!param || strlen(param) == 0) return 0;
  return 1;
}


/////////////////////////////////////////////////////////////////////////
giopAddress*
unixTransportImpl::toAddress(const char* param) {

  if (param) {
    return (giopAddress*)(new unixAddress(param));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
unixTransportImpl::addToIOR(const char* param) {

  if (param) {
    omniIOR::add_TAG_OMNIORB_UNIX_TRANS(param);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>* 
unixTransportImpl::getInterfaceAddress() {
  // There is no sensible interface address. Return an empty list.
  static omnivector<const char*> empty;
  return &empty;
}

const unixTransportImpl _the_unixTransportImpl;

OMNI_NAMESPACE_END(omni)
