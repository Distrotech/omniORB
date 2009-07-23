// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslAddress.cc              Created on: 29 May 2001
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
  Revision 1.1.2.19  2007/02/26 15:15:32  dgrisby
  New socketSendBuffer parameter, defaulting to 16384 on Windows.
  Avoids a bug in Windows where select() on send waits for all sent data
  to be acknowledged.

  Revision 1.1.2.18  2007/02/07 10:33:28  dgrisby
  Rounding error in poll() timeout could lead to infinite timeout.
  Thanks Richard Hirst.

  Revision 1.1.2.17  2006/04/19 11:34:42  dgrisby
  Poking an address created a new client-side connection object that
  registered itself in the SocketCollection. Since it did this while
  holding the giopServer's lock, that violated the partial lock order,
  and could lead to a deadlock.

  Revision 1.1.2.16  2004/10/17 20:14:33  dgrisby
  Updated support for OpenVMS. Many thanks to Bruce Visscher.

  Revision 1.1.2.15  2003/12/03 14:40:23  dgrisby
  Fix timeout bug with ssl transport; fix ssl configure issues.

  Revision 1.1.2.14  2003/01/14 12:20:33  dgrisby
  Verify that a connect really has connected.

  Revision 1.1.2.13  2003/01/06 11:11:55  dgrisby
  New AddrInfo instead of gethostbyname.

  Revision 1.1.2.12  2002/09/06 21:16:59  dgrisby
  Bail out if port number is 0.

  Revision 1.1.2.11  2002/08/16 16:00:53  dgrisby
  Bugs accessing uninitialised String_vars with [].

  Revision 1.1.2.10  2002/05/07 12:54:43  dgrisby
  Fix inevitable Windows header brokenness.

  Revision 1.1.2.9  2002/05/07 00:46:26  dgrisby
  Different define for TCP protocol number.

  Revision 1.1.2.8  2002/05/07 00:28:32  dgrisby
  Turn off Nagle's algorithm. Fixes odd Linux loopback behaviour.

  Revision 1.1.2.7  2001/09/07 11:27:14  sll
  Residual changes needed for the changeover to use orbParameters.

  Revision 1.1.2.6  2001/07/31 16:16:24  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.5  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.4  2001/07/13 15:35:09  sll
   Error in setblocking and setnonblocking now causes the socket to be closed
   as well.

  Revision 1.1.2.3  2001/06/20 18:35:17  sll
  Upper case send,recv,connect,shutdown to avoid silly substutition by
  macros defined in socket.h to rename these socket functions
  to something else.

  Revision 1.1.2.2  2001/06/18 20:27:56  sll
  Use strchr instead of index() for maximal portability.

  Revision 1.1.2.1  2001/06/11 18:11:07  sll
  *** empty log message ***

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <orbParameters.h>
#include <omniORB4/sslContext.h>
#include <SocketCollection.h>
#include <ssl/sslConnection.h>
#include <ssl/sslAddress.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslAddress);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
sslAddress::sslAddress(const IIOP::Address& address, sslContext* ctx) : 
  pd_address(address), pd_ctx(ctx) {

  const char* format = "giop:ssl:%s:%d";
  pd_address_string = CORBA::string_alloc(strlen(address.host)+
					  strlen(format)+6);
  sprintf((char*)pd_address_string,format,(const char*)address.host,
	  address.port);
}

/////////////////////////////////////////////////////////////////////////
sslAddress::sslAddress(const char* address, sslContext* ctx) : pd_ctx(ctx) {

  pd_address_string = address;
  // OMNIORB_ASSERT(strncmp(address,"giop:ssl:",9) == 0);
  const char* host = strchr(address,':');
  host = strchr(host+1,':') + 1;
  const char* port = strchr(host,':') + 1;
  CORBA::ULong hostlen = port - host - 1;
  // OMNIORB_ASSERT(hostlen);
  pd_address.host = CORBA::string_alloc(hostlen);
  strncpy(pd_address.host,host,hostlen);
  ((char*)pd_address.host)[hostlen] = '\0';
  int rc;
  unsigned int v;
  rc = sscanf(port,"%u",&v);
  // OMNIORB_ASSERT(rc == 1);
  // OMNIORB_ASSERT(v > 0 && v < 65536);
  pd_address.port = v;
}

/////////////////////////////////////////////////////////////////////////
const char*
sslAddress::type() const {
  return "giop:ssl";
}

/////////////////////////////////////////////////////////////////////////
const char*
sslAddress::address() const {
  return pd_address_string;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
sslAddress::duplicate() const {
  return new sslAddress(pd_address,pd_ctx);
}


/////////////////////////////////////////////////////////////////////////

static inline int setAndCheckTimeout(unsigned long deadline_secs,
				     unsigned long deadline_nanosecs,
				     struct timeval& t)
{
  if (deadline_secs || deadline_nanosecs) {
    SocketSetTimeOut(deadline_secs,deadline_nanosecs,t);
    if (t.tv_sec == 0 && t.tv_usec == 0) {
      // Already timedout.
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
  rc = poll(&fds,1,timeout);
  if (rc > 0 && fds.revents & POLLERR) {
    rc = 0;
  }
#else
  fd_set fds, efds;
  FD_ZERO(&fds);
  FD_ZERO(&efds);
  FD_SET(sock,&fds);
  FD_SET(sock,&efds);
  struct timeval* tp = &t;
  if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
  rc = select(sock+1,0,&fds,&efds,tp);
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
  rc = poll(&fds,1,timeout);
  if (rc > 0 && fds.revents & POLLERR) {
    rc = 0;
  }
#else
  fd_set fds, efds;
  FD_ZERO(&fds);
  FD_ZERO(&efds);
  FD_SET(sock,&fds);
  FD_SET(sock,&efds);
  struct timeval* tp = &t;
  if (t.tv_sec == 0 && t.tv_usec == 0) tp = 0;
  rc = select(sock+1,&fds,0,&efds,tp);
#endif
  return rc;
}


giopActiveConnection*
sslAddress::Connect(unsigned long deadline_secs,
		    unsigned long deadline_nanosecs) const {

  SocketHandle_t sock;

  if (pd_address.port == 0) return 0;

  LibcWrapper::AddrInfo_var ai;
  ai = LibcWrapper::getAddrInfo(pd_address.host, pd_address.port);

  if ((LibcWrapper::AddrInfo*)ai == 0)
    return 0;

  if ((sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET)
    return 0;

  {
    // Prevent Nagle's algorithm
    int valtrue = 1;
    if (setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,
		   (char*)&valtrue,sizeof(int)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(sock);
      return 0;
    }
  }

  if (orbParameters::socketSendBuffer != -1) {
    // Set the send buffer size
    int bufsize = orbParameters::socketSendBuffer;
    if (setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
		   (char*)&bufsize, sizeof(bufsize)) == RC_SOCKET_ERROR) {
      CLOSESOCKET(sock);
      return 0;
    }
  }

  if (SocketSetnonblocking(sock) == RC_INVALID_SOCKET) {
    CLOSESOCKET(sock);
    return 0;
  }

  if (::connect(sock,ai->addr(),ai->addrSize()) == RC_SOCKET_ERROR) {

    if (ERRNO != EINPROGRESS) {
      CLOSESOCKET(sock);
      return 0;
    }
  }

  struct timeval t;
  int rc;

  // Wait until we're connected and ready to send...
  do {
    if (setAndCheckTimeout(deadline_secs, deadline_nanosecs, t)) {
      // Already timeout.
      CLOSESOCKET(sock);
      return 0;
    }

    rc = waitWrite(sock, t);

    if (rc == 0) {
      // Time out!
#if defined(USE_FAKE_INTERRUPTABLE_RECV)
      continue;
#else
      CLOSESOCKET(sock);
      return 0;
#endif
    }
    if (rc != RC_SOCKET_ERROR) {
      // Check to make sure that the socket is connected.
      struct sockaddr_in peer;
      SOCKNAME_SIZE_T len = sizeof(peer);
      rc = getpeername(sock, (struct sockaddr*)&peer, &len);
    }
    if (rc == RC_SOCKET_ERROR) {
      if (ERRNO == RC_EINTR)
	continue;
      else {
	CLOSESOCKET(sock);
	return 0;
      }
    }
  } while (0);

  ::SSL* ssl = SSL_new(pd_ctx->get_SSL_CTX());
  SSL_set_fd(ssl, sock);
  SSL_set_connect_state(ssl);

  // Do the SSL handshake...
  while (1) {

    if (setAndCheckTimeout(deadline_secs, deadline_nanosecs, t)) {
      // Already timeout.
      SSL_free(ssl);
      CLOSESOCKET(sock);
      return 0;
    }

    int result = SSL_connect(ssl);
    int code   = SSL_get_error(ssl, result);

    switch(code) {
    case SSL_ERROR_NONE:
      {
	if (SocketSetblocking(sock) == RC_INVALID_SOCKET) {
	  SSL_free(ssl);
	  CLOSESOCKET(sock);
	  return 0;
	}
	return new sslActiveConnection(sock,ssl);
      }

    case SSL_ERROR_WANT_READ:
      {
	rc = waitRead(sock, t);
	if (rc == 0) {
	  // Timeout
#if !defined(USE_FAKE_INTERRUPTABLE_RECV)
	  SSL_free(ssl);
	  CLOSESOCKET(sock);
	  return 0;
#endif
	}
	continue;
      }

    case SSL_ERROR_WANT_WRITE:
      {
	rc = waitWrite(sock, t);
	if (rc == 0) {
	  // Timeout
#if !defined(USE_FAKE_INTERRUPTABLE_RECV)
	  SSL_free(ssl);
	  CLOSESOCKET(sock);
	  return 0;
#endif
	}
	continue;
      }

    case SSL_ERROR_SYSCALL:
      {
	if (ERRNO == RC_EINTR)
	  continue;
      }
      // otherwise falls through
    case SSL_ERROR_SSL:
      {
	if (omniORB::trace(10)) {
	  omniORB::logger log;
	  char buf[128];
	  ERR_error_string_n(ERR_get_error(),buf,128);
	  log << "openSSL error detected in sslAddress::connect. Reason: "
	      << (const char*) buf << "\n";
	}
	SSL_free(ssl);
	CLOSESOCKET(sock);
	return 0;
      }
    default:
      OMNIORB_ASSERT(0);
    }
  }
}

CORBA::Boolean
sslAddress::Poke() const {

  SocketHandle_t sock;

  if (pd_address.port == 0) return 0;

  LibcWrapper::AddrInfo_var ai;
  ai = LibcWrapper::getAddrInfo(pd_address.host, pd_address.port);

  if ((LibcWrapper::AddrInfo*)ai == 0)
    return 0;

  if ((sock = socket(INETSOCKET,SOCK_STREAM,0)) == RC_INVALID_SOCKET)
    return 0;

  if (SocketSetnonblocking(sock) == RC_INVALID_SOCKET) {
    CLOSESOCKET(sock);
    return 0;
  }

  if (::connect(sock,ai->addr(),ai->addrSize()) == RC_SOCKET_ERROR) {

    if (ERRNO != EINPROGRESS) {
      CLOSESOCKET(sock);
      return 0;
    }
  }

  // The connect has not necessarily completed by this stage, but
  // we've done enough to poke the endpoint. We do not bother with the
  // SSL handshake, so the accepting thread will get an error when it
  // tries to do the SSL accept.
  CLOSESOCKET(sock);
  return 1;
}


OMNI_NAMESPACE_END(omni)
