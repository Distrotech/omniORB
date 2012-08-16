// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpSocket.h                Created on: 4 June 2010
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2010-2011 Apasphere Ltd.
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
//    *** PROPRIETARY INTERFACE ***
//    Utility functions for managing TCP sockets

#ifndef __TCPSOCKET_h__
#define __TCPSOCKET_h__

#include <SocketCollection.h>
#include <orbParameters.h>
#include <omniORB4/omniServer.h>


OMNI_NAMESPACE_BEGIN(omni)

class tcpSocket {
public:

  static SocketHandle_t Bind(const char*   	      host,
			     CORBA::UShort 	      port_min,
			     CORBA::UShort 	      port_max,
			     const char*   	      transport_type,
			     char*&  	              bound_host,
			     CORBA::UShort&           bound_port,
			     orbServer::EndpointList& endpoints);
  // Create a socket and bind() and listen().
  //
  // If host is null or empty string, bind to all interfaces;
  // otherwise bind to the specified interface.
  //
  // If port_min and port_max are zero, bind to an ephemeral port; if
  // they are non-zero and equal, bind to the specified port, setting
  // the SO_REUSEADDR socket option; if port_max > port_min, bind to
  // one of the ports in the specified range.
  //
  // transport_type is the URI prefix for the requesting transport,
  // e.g. giop:tcp
  //
  // Returns the bound socket, or RC_INVALID_SOCKET on error.
  //
  // bound_host is set to the chosen host address. Caller frees.
  //
  // bound_port is set to the chosen port.
  //
  // endpoints is populated with all the endpoints that result from
  // the socket.


  static SocketHandle_t Connect(const char*   	   host,
				CORBA::UShort 	   port,
				const omni_time_t& deadline,
				CORBA::ULong  	   strand_flags,
				CORBA::Boolean&    timed_out);
  // Connect to specified host and port.
  //
  // If deadline is set, connect attempt can time out.
  //
  // strand_flags contains additional requirements on the connection.
  // See giopStrandFlags.h.
  //
  // Returns bound socket, or RC_INVALID_SOCKET on error or timeout.
  // On timeout, timed_out is set true.


  static inline void
  logConnectFailure(const char*            message,
		    LibcWrapper::AddrInfo* ai)
  {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      CORBA::String_var addr = ai->asString();
      log << message << ": " << addr;
      
      CORBA::UShort port = addrToPort(ai->addr());
      if (port)
	log << ":" << port;

      log << "\n";
    }
  }

  static inline void
  logConnectFailure(const char*   message,
		    const char*   host,
		    CORBA::UShort port=0)
  {
    if (omniORB::trace(25)) {
      omniORB::logger log;
      log << message << ": " << host;
      if (port)
	log << ":" << port;
      log << "\n";
    }
  }


  static inline int setAndCheckTimeout(const omni_time_t& deadline,
				       struct timeval&    t)
  {
    if (deadline) {
      SocketSetTimeOut(deadline, t);
      if (t.tv_sec == 0 && t.tv_usec == 0) {
	// Already timed out.
	return 1;
      }
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      if (t.tv_sec > orbParameters::scanGranularity) {
	t.tv_sec = orbParameters::scanGranularity;
      }
#endif
    }
    else {
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      t.tv_sec = orbParameters::scanGranularity;
      t.tv_usec = 0;
#else
      t.tv_sec = t.tv_usec = 0;
#endif
    }
    return 0;
  }


  static inline int waitWrite(SocketHandle_t sock, struct timeval& t)
  {
    int rc;

#if defined(USE_POLL)
    struct pollfd fds;
    fds.fd = sock;
    fds.events = POLLOUT;
    int timeout = t.tv_sec*1000+((t.tv_usec+999)/1000);
    if (timeout == 0) timeout = -1;
    rc = poll(&fds, 1, timeout);
    if (rc > 0 && fds.revents & POLLERR) {
      rc = RC_SOCKET_ERROR;
    }
#else
    fd_set fds, efds;
    FD_ZERO(&fds);
    FD_ZERO(&efds);
    FD_SET(sock,&fds);
    FD_SET(sock,&efds);
    struct timeval* tp = &t;
    if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
    rc = select(sock+1, 0, &fds, &efds, tp);
#endif
    return rc;
  }

  static inline int waitRead(SocketHandle_t sock, struct timeval& t)
  {
    int rc;

#if defined(USE_POLL)
    struct pollfd fds;
    fds.fd = sock;
    fds.events = POLLIN;
    int timeout = t.tv_sec*1000+((t.tv_usec+999)/1000);
    if (timeout == 0) timeout = -1;
    rc = poll(&fds, 1, timeout);
    if (rc > 0 && fds.revents & POLLERR) {
      rc = RC_SOCKET_ERROR;
    }
#else
    fd_set fds, efds;
    FD_ZERO(&fds);
    FD_ZERO(&efds);
    FD_SET(sock,&fds);
    FD_SET(sock,&efds);
    struct timeval* tp = &t;
    if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
    rc = select(sock+1, &fds, 0, &efds, tp);
#endif
    return rc;
  }

  static char* addrToString(sockaddr* addr);
  // Return string form of an IP address in dotted decimal or
  // colon-separated hex.

  static char* addrToURI(sockaddr* addr, const char* prefix);
  // Return URI for address, with specified prefix.

  static CORBA::UShort addrToPort(sockaddr* addr);
  // Return port number in address.

  static char* peerToURI(SocketHandle_t sock, const char* prefix);
  // Return URI for peer on socket.

};


OMNI_NAMESPACE_END(omni);


#endif // __TCPSOCKET_h__
