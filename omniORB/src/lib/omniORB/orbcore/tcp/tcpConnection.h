// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpConnection.h            Created on: 19 Mar 2001
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
  Revision 1.1.4.4  2005/03/02 12:10:48  dgrisby
  setSelectable / Peek fixes.

  Revision 1.1.4.3  2005/01/13 21:10:03  dgrisby
  New SocketCollection implementation, using poll() where available and
  select() otherwise. Windows specific version to follow.

  Revision 1.1.4.2  2005/01/06 23:10:55  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:58  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.7  2001/12/03 13:39:55  dpg1
  Explicit socket shutdown flag for Windows.

  Revision 1.1.2.6  2001/07/31 16:16:17  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.5  2001/07/13 15:33:28  sll
  Revised declaration to match the changes in giopConnection.

  Revision 1.1.2.4  2001/06/20 18:35:16  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.3  2001/06/18 20:28:31  sll
  Remove garbage after #endif

  Revision 1.1.2.2  2001/06/13 20:13:49  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.1  2001/04/18 18:10:44  sll
  Big checkin with the brand new internal APIs.

*/

#ifndef __TCPCONNECTION_H__
#define __TCPCONNECTION_H__

#include <SocketCollection.h>

OMNI_NAMESPACE_BEGIN(omni)

class tcpEndpoint;

class tcpConnection : public giopConnection, public SocketHolder {
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

  tcpConnection(SocketHandle_t,SocketCollection*);

  ~tcpConnection();

  static char* addrToString(sockaddr* addr);
  static char* addrToURI(sockaddr* addr, const char* prefix);
  static CORBA::UShort addrToPort(sockaddr* addr);

  friend class tcpEndpoint;

private:
  CORBA::String_var pd_myaddress;
  CORBA::String_var pd_peeraddress;
};


class tcpActiveConnection : public giopActiveConnection, public tcpConnection {
public:
  giopActiveCollection* registerMonitor();
  giopConnection& getConnection();

  tcpActiveConnection(SocketHandle_t);
  ~tcpActiveConnection();

private:
  CORBA::Boolean pd_registered;

  tcpActiveConnection(const tcpActiveConnection&);
  tcpActiveConnection& operator=(const tcpActiveConnection&);
};


OMNI_NAMESPACE_END(omni)

#endif //__TCPCONNECTION_H__
