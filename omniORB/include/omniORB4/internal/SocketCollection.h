// -*- Mode: C++; -*-
//                            Package   : omniORB
// SocketCollection.h         Created on: 23 Jul 2001
//                            Author    : Sai Lai Lo (sll)
//                            Author    : Duncan Grisby
//
//    Copyright (C) 2005-2012 Apasphere Ltd.
//    Copyright (C) 2001      AT&T Laboratories Cambridge
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
//	*** PROPRIETARY INTERFACE ***
//

#ifndef __SOCKETCOLLECTION_H__
#define __SOCKETCOLLECTION_H__

////////////////////////////////////////////////////////////////////////
//  Platform feature selection

#if !defined(OMNI_DISABLE_IPV6) && defined(HAVE_STRUCT_SOCKADDR_IN6) && defined(HAVE_STRUCT_SOCKADDR_STORAGE) && defined(HAVE_GETADDRINFO) && defined(HAVE_GETNAMEINFO)
#  define OMNI_SUPPORT_IPV6
#  define OMNI_SOCKADDR_STORAGE sockaddr_storage
#else
#  define OMNI_SOCKADDR_STORAGE sockaddr_in
#endif

#define SOCKNAME_SIZE_T OMNI_SOCKNAME_SIZE_T
#define USE_NONBLOCKING_CONNECT
#define OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#define OMNIORB_HOSTNAME_MAX 512

#ifdef HAVE_POLL
#   define USE_POLL
#endif

// Darwin implementation of poll() appears to be broken
#if defined(__darwin__)
#   undef USE_POLL
#endif

#if defined(__hpux__)
#   if __OSVERSION__ < 11
#       undef USE_POLL
#   endif
#   define USE_FAKE_INTERRUPTABLE_RECV
#endif

#if defined(__WIN32__)
#   define USE_FAKE_INTERRUPTABLE_RECV
#   undef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#endif

#if defined(__freebsd__) || defined(__netbsd__)
#   undef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#endif

// By default, Linux does accept IPv4 connections on IPv6, but some
// distributions misconfigure it not to.
#if defined(__linux__) || defined(IPV6_V6ONLY)
#   undef OMNI_IPV6_SOCKETS_ACCEPT_IPV4_CONNECTIONS
#endif


////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             win32 API
//
#if defined(__WIN32__)

#  include <sys/types.h>
#  include <libcWrapper.h>

#  if defined(OMNI_SUPPORT_IPV6)
#    include <ws2tcpip.h>
#    include <Wspiapi.h>
#    if !defined(IPV6_V6ONLY)
#      define IPV6_V6ONLY 27  // Defined to this on Vista
#    endif
#  endif

#  define RC_INADDR_NONE     INADDR_NONE
#  define RC_INVALID_SOCKET  INVALID_SOCKET
#  define RC_SOCKET_ERROR    SOCKET_ERROR
#  define INETSOCKET         PF_INET
#  define CLOSESOCKET(sock)  closesocket(sock)
#  define SHUTDOWNSOCKET(sock) ::shutdown(sock,2)
#  define ERRNO              ::WSAGetLastError()
#  define RC_EINPROGRESS     WSAEWOULDBLOCK
#  define RC_EINTR           WSAEINTR
#  define RC_EBADF           WSAENOTSOCK

OMNI_NAMESPACE_BEGIN(omni)

typedef SOCKET SocketHandle_t;

OMNI_NAMESPACE_END(omni)

#else

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//             unix(ish)
//
#  if defined(__vxWorks__)
#    include <sockLib.h>
#    include <hostLib.h>
#    include <ioLib.h>
#    include <iosLib.h>
#    include <netinet/tcp.h>
#  else
#    include <sys/time.h>
#  endif
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netinet/tcp.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <sys/types.h>
#  include <errno.h>
#  include <libcWrapper.h>

#  if defined(USE_POLL)
#    include <poll.h>
#  endif

#  include <fcntl.h>

#  if defined (__uw7__)
#    ifdef shutdown
#      undef shutdown
#    endif
#  endif

#  if defined(__VMS) && defined(USE_tcpSocketVaxRoutines)
#    include "tcpSocketVaxRoutines.h"
#    undef accept
#    undef recv
#    undef send
#    define accept(a,b,c) tcpSocketVaxAccept(a,b,c)
#    define recv(a,b,c,d) tcpSocketVaxRecv(a,b,c,d)
#    define send(a,b,c,d) tcpSocketVaxSend(a,b,c,d)
#  endif

#  ifdef __rtems__
extern "C" int select (int,fd_set*,fd_set*,fd_set*,struct timeval *);
#  endif

#  define RC_INADDR_NONE     ((CORBA::ULong)-1)
#  define RC_INVALID_SOCKET  (-1)
#  define RC_SOCKET_ERROR    (-1)
#  define INETSOCKET         AF_INET
#  define CLOSESOCKET(sock)  close(sock)

#  if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ >= 5
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  elif defined(__osf1__) && defined(__alpha__)
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  else
     // XXX none of the above, calling shutdown() may not have the
     // desired effect.
#    define SHUTDOWNSOCKET(sock)  ::shutdown(sock,2)
#  endif

#  define ERRNO              errno
#  define RC_EINTR           EINTR
#  define RC_EINPROGRESS     EINPROGRESS
#  if defined (__vxWorks__)
#    define RC_EBADF         S_iosLib_INVALID_FILE_DESCRIPTOR  
#  else
#    define RC_EBADF         EBADF
#  endif
#  define RC_EAGAIN          EAGAIN


OMNI_NAMESPACE_BEGIN(omni)

typedef int SocketHandle_t;

OMNI_NAMESPACE_END(omni)

#endif

#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
#endif

OMNI_NAMESPACE_BEGIN(omni)

class SocketCollection;

extern void SocketSetTimeOut(const omni_time_t& deadline, struct timeval& t);

extern int SocketSetnonblocking(SocketHandle_t sock);

extern int SocketSetblocking(SocketHandle_t sock);

extern int SocketSetCloseOnExec(SocketHandle_t sock);


//
// Class SocketHolder holds a socket inside a collection. It contains
// flags to indicate whether the socket is selectable, and so on.
// Connection classes (e.g. tcpConnection) derive from this class.

class SocketHolder {

public:
  SocketHolder(SocketHandle_t s)
    : pd_socket(s),
      pd_belong_to(0),
      pd_shutdown(0),
      pd_selectable(0),
      pd_data_in_buffer(0),
      pd_peeking(0),
      pd_peek_go(0),
      pd_peek_cond(0),
      pd_fd_index(-1),
      pd_next(0),
      pd_prev(0) { }

  virtual ~SocketHolder();

  void setSelectable(int now,
		     CORBA::Boolean data_in_buffer,
		     CORBA::Boolean hold_lock=0);
  // Indicate that this socket should be watched for readability.
  //
  // If now is 1, immediately make the socket selectable (if the
  // platform permits it), rather than waiting until the select loop
  // rescans.
  //
  // If now is 2, immediately make the socket selectable (if the
  // platform permits it), but only if it is already marked
  // selectable.
  //
  // If data_in_buffer is true, the socket is considered to already
  // have data available to read.
  //
  // If hold_lock is true, the associated SocketCollection's lock is
  // already held.

  void clearSelectable();
  // Indicate that this socket should not be watched any more.

  CORBA::Boolean Peek();
  // Watch the socket for a while to see if any data arrives. If the
  // socket is not already selectable, wait for a bit in case it
  // becomes selectable. Mark the socket as no longer selectable and
  // return true if the socket becomes readable, otherwise return
  // false.

  friend class SocketCollection;

protected:
  SocketHandle_t       	pd_socket;
  SocketCollection*    	pd_belong_to;
  CORBA::Boolean       	pd_shutdown;

private:
  CORBA::Boolean       	pd_selectable;     // True if socket is selectable
  CORBA::Boolean       	pd_data_in_buffer; // True if data already available
  CORBA::Boolean       	pd_peeking;        // True if a thread is currently
					   // peeking
  CORBA::Boolean        pd_peek_go;        // True if the peeking thread
					   // should return true, even if it
					   // did not see data to read
  omni_tracedcondition* pd_peek_cond;      // Condition to signal a waiting
					   // peeker
  int                  	pd_fd_index;       // -1 if select thread is not
					   // watching; otherwise, index of
					   // the fd within the poll / select
					   // list.
  SocketHolder*        	pd_next;
  SocketHolder**       	pd_prev;
};


//
// SocketCollection manages a collection of sockets.

class SocketCollection {
public:

  SocketCollection();

  virtual ~SocketCollection();

  virtual CORBA::Boolean notifyReadable(SocketHolder*) = 0;
  // Callback used by Select(). If it returns false, something has
  // gone very wrong with the collection and exits the Select loop.
  // This method is called while holding pd_collection_lock.

  CORBA::Boolean isSelectable(SocketHandle_t sock);
  // Indicates whether the given socket can be selected upon.

  CORBA::Boolean Select();
  // Returns true if the Select() has successfully done a scan.
  // otherwise returns false to indicate that an error has been
  // detected and this function should not be called again.
  //
  // For each of the sockets that has been marked watchable and indeed
  // has become readable, call notifyReadable() with the socket as the
  // argument.

  void wakeUp();
  // On platforms where is is possible, immediately wake up a thread
  // blocked in Select().

  void incrRefCount();
  void decrRefCount();

  void addSocket(SocketHolder* sock);
  // Add this socket to the collection. Increments this collection's
  // refcount.

  void removeSocket(SocketHolder* sock);
  // Remove the socket from this collection. Returns the socket which
  // has been removed. Decrements this collection's refcount.

  static omni_time_t   scan_interval;
  static unsigned      idle_scans;

private:
  int                  pd_refcount;
  omni_tracedmutex     pd_collection_lock;

  // Absolute time at which we scan through the socket list choosing
  // the selectable ones.
  omni_time_t          pd_abs_time;

  // On platforms that support it, we use a pipe to wake up the select.
  int                  pd_pipe_read;
  int                  pd_pipe_write;
  CORBA::Boolean       pd_pipe_full;

  // On platforms that support pipes, after scanning a while with no
  // activity, we poll / select with an infinite timeout to prevent
  // unnecessary processing.
  unsigned             pd_idle_count;

#if defined(USE_POLL)
  // On platforms where we use poll(), we maintain a pre-allocated
  // array of pollfd structures and a parallel array of pointers to
  // SocketHolders. pd_pollfd_n is the number of populated pollfds.
  // pd_pollfd_len is the current length of both arrays.
  struct pollfd*       pd_pollfds;
  SocketHolder**       pd_pollsockets;
  unsigned             pd_pollfd_n;
  unsigned             pd_pollfd_len;

  void growPollLists();
  // Expand the pd_pollfds and pd_pollsockets to fit more values.

#elif defined(__WIN32__)
  // Windows has select() but its fd_sets are more like pollfds, just
  // less convenient...
  omni_tracedcondition pd_select_cond; // timedwait on if nothing to select
  fd_set               pd_fd_set;
  SocketHolder*        pd_fd_sockets[FD_SETSIZE];
  
#else
  // On other platforms we use select(). We maintain an fd_set and the
  // highest socket number set in it plus 1.
  fd_set               pd_fd_set;
  int                  pd_fd_set_n;
#endif

  // Linked list of registered sockets.
  SocketHolder*        pd_collection;
  CORBA::Boolean       pd_changed;

  friend class SocketHolder;
};

OMNI_NAMESPACE_END(omni)

#endif // __SOCKETCOLLECTION_H__
