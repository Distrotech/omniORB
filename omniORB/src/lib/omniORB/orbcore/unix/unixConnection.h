// -*- Mode: C++; -*-
//                            Package   : omniORB
// unixConnection.h           Created on: 6 Aug 2001
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
  Revision 1.1.4.3  2005/01/13 21:10:16  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:58  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:57  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.2  2001/08/07 15:42:17  sll
  Make unix domain connections distinguishable on both the server and client
  side.

  Revision 1.1.2.1  2001/08/06 15:47:44  sll
  Added support to use the unix domain socket as the local transport.

*/

#ifndef __UNIXCONNECTION_H__
#define __UNIXCONNECTION_H__

#include <SocketCollection.h>

OMNI_NAMESPACE_BEGIN(omni)

class unixEndpoint;

class unixConnection : public giopConnection, public SocketHolder {
 public:

  int Send(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  int Recv(void* buf, size_t sz,
	   unsigned long deadline_secs = 0,
	   unsigned long deadline_nanosecs = 0);

  void Shutdown();

  const char* myaddress();

  const char* peeraddress();

  void setSelectable(int now = 0,CORBA::Boolean data_in_buffer = 0);

  void clearSelectable();

  CORBA::Boolean isSelectable();

  CORBA::Boolean Peek();

  SocketHandle_t handle() const { return pd_socket; }

  unixConnection(SocketHandle_t,SocketCollection*,
		 const char* filename, CORBA::Boolean isActive);

  ~unixConnection();

  static char* unToString(const char* filename);

  friend class unixEndpoint;

 private:
  CORBA::String_var pd_myaddress;
  CORBA::String_var pd_peeraddress;
};


class unixActiveConnection : public giopActiveConnection, public unixConnection {
public:
  giopActiveCollection* registerMonitor();
  giopConnection& getConnection();

  unixActiveConnection(SocketHandle_t,const char* filename);
  ~unixActiveConnection();

private:
  CORBA::Boolean pd_registered;

  unixActiveConnection(const unixActiveConnection&);
  unixActiveConnection& operator=(const unixActiveConnection&);
};


OMNI_NAMESPACE_END(omni)

#endif //__UNIXCONNECTION_H__
