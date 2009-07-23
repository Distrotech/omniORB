// -*- Mode: C++; -*-
//                            Package   : omniORB
// tcpTransportImpl.cc        Created on: 29 Mar 2001
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
  Revision 1.1.2.23  2005/04/25 18:00:08  dgrisby
  Fix infinite loop with inactive interfaces on vxWorks. Thanks Florian
  Kiesswetter.

  Revision 1.1.2.22  2004/07/26 09:57:28  dgrisby
  Code using getifaddrs didn't cope with null ifa_addr member. Thanks
  Dirk Siebnich.

  Revision 1.1.2.21  2004/02/11 17:01:13  dgrisby
  Use getifaddrs where available. Thanks Craig Rodrigues.

  Revision 1.1.2.20  2004/02/11 12:19:17  dgrisby
  Cygwin patches. Thanks Douglas Brown.

  Revision 1.1.2.19  2003/11/06 10:18:39  dgrisby
  Expand FD_SETSIZE on Windows.

  Revision 1.1.2.18  2003/07/25 16:04:57  dgrisby
  vxWorks patches.

  Revision 1.1.2.17  2003/06/18 10:42:30  dgrisby
  AIX interface lookup fix.

  Revision 1.1.2.16  2003/02/17 02:03:11  dgrisby
  vxWorks port. (Thanks Michael Sturm / Acterna Eningen GmbH).

  Revision 1.1.2.15  2002/11/06 11:31:21  dgrisby
  Old ETS patches that got lost; updates patches README.

  Revision 1.1.2.14  2002/10/28 13:48:50  dgrisby
  Work around Windows ME 0.0.0.0 interface problem.

  Revision 1.1.2.13  2002/04/29 18:22:02  dgrisby
  Yet another Windows fix.

  Revision 1.1.2.12  2002/04/29 11:52:51  dgrisby
  More fixes for FreeBSD, Darwin, Windows.

  Revision 1.1.2.11  2002/04/28 20:43:25  dgrisby
  Windows, FreeBSD, ETS fixes.

  Revision 1.1.2.10  2002/03/28 17:44:35  dpg1
  return in wrong place.

  Revision 1.1.2.9  2002/03/13 16:05:40  dpg1
  Transport shutdown fixes. Reference count SocketCollections to avoid
  connections using them after they are deleted. Properly close
  connections when in thread pool mode.

  Revision 1.1.2.8  2002/03/11 12:21:07  dpg1
  ETS things.

  Revision 1.1.2.7  2002/01/09 11:37:46  dpg1
  Platform, constness fixes.

  Revision 1.1.2.6  2001/08/24 16:46:36  sll
  Use WSAIoctl SIO_ADDRESS_LIST_QUERY to get the address of the
  IP address of all network interfaces.

  Revision 1.1.2.5  2001/08/23 16:02:58  sll
  Implement getInterfaceAddress().

  Revision 1.1.2.4  2001/07/31 16:16:16  sll
  New transport interface to support the monitoring of active connections.

  Revision 1.1.2.3  2001/07/26 16:37:22  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.2  2001/06/13 20:13:50  sll
  Minor updates to make the ORB compiles with MSVC++.

  Revision 1.1.2.1  2001/04/18 18:10:43  sll
  Big checkin with the brand new internal APIs.

*/

#include <stdlib.h>
#include <stdio.h>
#include <omniORB4/CORBA.h>
#include <omniORB4/giopEndpoint.h>
#include <objectAdapter.h>
#include <SocketCollection.h>
#include <tcp/tcpConnection.h>
#include <tcp/tcpAddress.h>
#include <tcp/tcpEndpoint.h>
#include <tcp/tcpTransportImpl.h>
#include <orbParameters.h>

#if defined(UnixArchitecture)
#  include <sys/ioctl.h>
#  include <net/if.h>
#endif

#if defined(HAVE_IFADDRS_H)
#  include <ifaddrs.h>
#endif
 
#if defined(NTArchitecture)
#  include <libcWrapper.h>
#  include <ws2tcpip.h>
#endif

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(tcpAddress);
OMNI_FORCE_LINK(tcpConnection);
OMNI_FORCE_LINK(tcpEndpoint);
OMNI_FORCE_LINK(tcpActive);

OMNI_EXPORT_LINK_FORCE_SYMBOL(tcpTransportImpl);

OMNI_NAMESPACE_BEGIN(omni)

/////////////////////////////////////////////////////////////////////////
tcpTransportImpl::tcpTransportImpl() : giopTransportImpl("giop:tcp") {
}

/////////////////////////////////////////////////////////////////////////
tcpTransportImpl::~tcpTransportImpl() {
  omnivector<const char*>::iterator i = ifAddresses.begin();
  omnivector<const char*>::iterator last = ifAddresses.end();
  while ( i != last ) {
    char* p = (char*)(*i);
    CORBA::string_free(p);
    i++;
  }
}

/////////////////////////////////////////////////////////////////////////
giopEndpoint*
tcpTransportImpl::toEndpoint(const char* param) {

  const char* p = strchr(param,':');
  if (!p) return 0;
  IIOP::Address address;
  if (param == p) {
    const char* hostname = getenv(OMNIORB_USEHOSTNAME_VAR);
    if (hostname) address.host = hostname;
  }
  else {
    address.host = CORBA::string_alloc(p-param);
    strncpy(address.host,param,p-param);
    ((char*)address.host)[p-param] = '\0';
  }
  if (*(++p) != '\0') {
    int v;
    if (sscanf(p,"%d",&v) != 1) return 0;
    if (v < 0 || v > 65536) return 0;
    address.port = v;
  }
  else {
    address.port = 0;
  }
  return (giopEndpoint*)(new tcpEndpoint(address));
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::isValid(const char* param) {

  const char* p = strchr(param,':');
  if (!p || param == p || *p == '\0') return 0;
  int v;
  if (sscanf(p+1,"%d",&v) != 1) return 0;
  if (v < 0 || v > 65536) return 0;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::parseAddress(const char* param, IIOP::Address& address) {

  const char* p = strchr(param,':');
  if (!p || param == p || *p == '\0') return 0;
  address.host = CORBA::string_alloc(p-param);
  strncpy(address.host,param,p-param);
  ((char*) address.host)[p-param] = '\0';
  ++p;
  int v;
  if (sscanf(p,"%d",&v) != 1) return 0;
  if (v < 0 || v > 65536) return 0;
  address.port = v;
  return 1;
}

/////////////////////////////////////////////////////////////////////////
giopAddress*
tcpTransportImpl::toAddress(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    return (giopAddress*)(new tcpAddress(address));
  }
  else {
    return 0;
  }
}

/////////////////////////////////////////////////////////////////////////
CORBA::Boolean
tcpTransportImpl::addToIOR(const char* param) {

  IIOP::Address address;
  if (parseAddress(param,address)) {
    omniIOR::add_IIOP_ADDRESS(address);
    return 1;
  }
  return 0;
}

/////////////////////////////////////////////////////////////////////////
#if   defined(__vxWorks__)
static void vxworks_get_ifinfo(omnivector<const char*>& ifaddrs);
#elif defined(HAVE_IFADDRS_H)
static void ifaddrs_get_ifinfo(omnivector<const char*>& addrs);
#elif defined(UnixArchitecture)
static void unix_get_ifinfo(omnivector<const char*>& ifaddrs);
#elif defined(NTArchitecture)
static void win32_get_ifinfo(omnivector<const char*>& ifaddrs);
#endif

/////////////////////////////////////////////////////////////////////////
void
tcpTransportImpl::initialise() {
  if (!ifAddresses.empty()) return;

#if   defined(__vxWorks__)
  vxworks_get_ifinfo(ifAddresses);
#elif defined(HAVE_IFADDRS_H)
  ifaddrs_get_ifinfo(ifAddresses);
#elif defined(UnixArchitecture)
  unix_get_ifinfo(ifAddresses);
#elif defined(NTArchitecture)
  win32_get_ifinfo(ifAddresses);
#endif

}

/////////////////////////////////////////////////////////////////////////
const omnivector<const char*>*
tcpTransportImpl::getInterfaceAddress() {
  return &ifAddresses;
}

/////////////////////////////////////////////////////////////////////////
const tcpTransportImpl _the_tcpTransportImpl;


/////////////////////////////////////////////////////////////////////////
#if defined(HAVE_IFADDRS_H)
static
void ifaddrs_get_ifinfo(omnivector<const char*>& addrs) {

  struct ifaddrs *ifa_list;

  if ( getifaddrs(&ifa_list) < 0 ) {
    if ( omniORB::trace(1) ) {
       omniORB::logger log;
       log << "Warning: getifaddrs() failed.\n"
           << "Unable to obtain the list of all interface addresses.\n";
    }
    return;
  }

  struct ifaddrs *p;
  for (p = ifa_list; p != 0; p = p->ifa_next) {
    if (p->ifa_addr && p->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in* iaddr = (struct sockaddr_in*)p->ifa_addr;
      CORBA::String_var s;
      s = tcpConnection::ip4ToString(iaddr->sin_addr.s_addr);
      addrs.push_back(s._retn());
    }
  } 
  freeifaddrs(ifa_list);

  if ( orbParameters::dumpConfiguration || omniORB::trace(20) ) {
    omniORB::logger log;
    omnivector<const char*>::iterator i = addrs.begin();
    omnivector<const char*>::iterator last = addrs.end();
    log << "My addresses are: \n";
    while ( i != last ) {
      log << "omniORB: " << (const char*)(*i) << "\n";
      i++;
    }
  }
}

#elif defined(UnixArchitecture) && !defined(__vxWorks__)

#ifdef __aix__
#  define OMNI_SIOCGIFCONF OSIOCGIFCONF
#else
#  define OMNI_SIOCGIFCONF SIOCGIFCONF
#endif

static
void unix_get_ifinfo(omnivector<const char*>& ifaddrs) {

  SocketHandle_t sock;

  sock = socket(INETSOCKET,SOCK_STREAM,0);

  int lastlen = 0;
  int len = 100 * sizeof(struct ifreq);
  struct ifconf ifc;
  // struct ifconf and ifreq are defined in net/if.h

  while ( 1 ) {
    // There is no way to know for sure the buffer is big enough to get
    // the info for all the interfaces. We work around this by calling
    // the ioctl 2 times and increases the buffer size in the 2nd call.
    // If both calls return the info with the same size, we know we have
    // got all the interfaces.
    char* buf = (char*) malloc(len);
    ifc.ifc_len = len;
    ifc.ifc_buf = buf;
    if ( ioctl(sock, OMNI_SIOCGIFCONF, &ifc) < 0 ) {
      if ( errno != EINVAL || lastlen != 0 ) {
	if ( omniORB::trace(1) ) {
	  omniORB::logger log;
	  log << "Warning: ioctl SIOCGICONF failed.\n"
	      << "Unable to obtain the list of all interface addresses.\n";
	}
	return;
      }
    }
    else {
      if ( ifc.ifc_len == lastlen )
	break; // Success, len has not changed.
      lastlen = ifc.ifc_len;
    }
    len += 10 * sizeof(struct ifreq);
    free(buf);
  }
  close(sock);

  int total = ifc.ifc_len / sizeof(struct ifreq);
  struct ifreq* ifr = ifc.ifc_req;
  for (int i = 0; i < total; i++) {

    if ( ifr[i].ifr_addr.sa_family == AF_INET ) {
      struct sockaddr_in* iaddr = (struct sockaddr_in*)&ifr[i].ifr_addr;

      if ( iaddr->sin_addr.s_addr != 0 ) {
	CORBA::String_var s;
	s = tcpConnection::ip4ToString(iaddr->sin_addr.s_addr);
	ifaddrs.push_back(s._retn());
      }
    }
  }
  free(ifc.ifc_buf);

  if ( orbParameters::dumpConfiguration || omniORB::trace(20) ) {
    omniORB::logger log;
    omnivector<const char*>::iterator i = ifaddrs.begin();
    omnivector<const char*>::iterator last = ifaddrs.end();
    log << "My addresses are: \n";
    while ( i != last ) {
      log << "omniORB: " << (const char*)(*i) << "\n";
      i++;
    }
  }
}

/////////////////////////////////////////////////////////////////////////
#elif defined(__vxWorks__)
void vxworks_get_ifinfo(omnivector<const char*>& ifaddrs) {

  const int iMAX_ADDRESS_ENTRIES = 50;
  // Max. number of interface addresses.  There is 1 link layer address
  // (AF_LINK) and at least 1 internet address (more if ifAddrAdd has been
  // called) per configured network interface

  const int iMAX_IFREQ_SIZE=36;
  // ifreq entries have a name field (16 bytes) plus an address field,
  // AF_LINK addresses (sockaddr_dl = 20 bytes)
  // AF_INET addresses (sockaddr_in = 16 bytes)

  // buffer into which to copy retieved configuration
  char buffer[iMAX_ADDRESS_ENTRIES * iMAX_IFREQ_SIZE];

  struct ifconf ifc; // used to retrieve interface configuration
  struct ifreq *ifr; // interface structure pointer

  int s;             // socket file descriptor
  int entryLength;   // size of ifreq entry

  char ifreqBuf[iMAX_IFREQ_SIZE]; // buffer for checking flags
  struct sockaddr_dl *pDataLinkAddr;
  int offset;

  // create socket to issue ioctl call
  if ((s = socket (AF_INET, SOCK_DGRAM,0)) == ERROR) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning : socket (AF_INET, SOCK_DGRAM,0) failed. "
	  << "Unable to obtain the list of all interface addresses.\n";
    }
    return;
  }

  // set up interface request structure array
  ifc.ifc_len = sizeof (buffer);
  ifc.ifc_buf = (char *)buffer;

  // get configuration
  if (ioctl (s, SIOCGIFCONF, (int)&ifc) < 0) {
    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "Warning: ioctl SIOCGICONF failed. "
	  << "Unable to obtain the list of all interface addresses.\n";
    }
    close (s);
    return;
  }

  ifr = ifc.ifc_req;

  // ioctl call changes ifc_len to the actual total bytes copied
  entryLength = ifc.ifc_len;

  for (entryLength = ifc.ifc_len; entryLength > 0;) {
    offset = sizeof (ifr->ifr_name) + ifr->ifr_addr.sa_len;
    bcopy ((caddr_t)ifr, ifreqBuf, offset);

    if (ioctl (s, SIOCGIFFLAGS, (int)ifreqBuf) < 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Warning: ioctl SIOCGIFFLAGS failed. "
	    << "Unable to obtain the list of all interface addresses.\n";
      }
      close (s);
      return;
    }

    if (((struct ifreq *)ifreqBuf)->ifr_flags & IFF_UP) {
      if (ifr->ifr_addr.sa_family == AF_INET) {
	// AF_INET entries are of type sockaddr_in = 16 bytes
	struct sockaddr_in* iaddr = (struct sockaddr_in*) &(ifr->ifr_addr);
	CORBA::String_var s;
	s = tcpConnection::ip4ToString(iaddr->sin_addr.s_addr);
	ifaddrs.push_back(s._retn());
      }
    }      
    // ifreq structures have variable lengths
    entryLength -= offset;
    ifr = (struct ifreq *)((char *)ifr + offset);
  }
  close (s);
}

#endif


/////////////////////////////////////////////////////////////////////////
#if defined(NTArchitecture)

#if defined(__ETS_KERNEL__)
extern "C" int WSAAPI ETS_WSAIoctl(
  SOCKET s,
  DWORD dwIoControlCode,
  LPVOID lpvInBuffer,
  DWORD cbInBuffer,
  LPVOID lpvOutBuffer,
  DWORD cbOutBuffer,
  LPDWORD lpcbBytesReturned,
  LPWSAOVERLAPPED lpOverlapped,
  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
);
#define WSAIoctl ETS_WSAIoctl
#endif

static
void win32_get_ifinfo(omnivector<const char*>& ifaddrs) {

  SocketHandle_t sock;

  sock = socket(INETSOCKET,SOCK_STREAM,0);

  INTERFACE_INFO info[64];  // Assume max 64 interfaces
  DWORD retlen;
  
  if ( WSAIoctl(sock, SIO_GET_INTERFACE_LIST, NULL,0,
                (LPVOID)&info, sizeof(info), (LPDWORD)&retlen,
		NULL,NULL) == SOCKET_ERROR ) {
    if ( omniORB::trace(2) ) {
      omniORB::logger log;
      int err = WSAGetLastError();
      log << "Warning: WSAIoctl SIO_GET_INTERFACE_LIST failed.\n"
	  << "Unable to obtain the list of all interface addresses.\n"
	  << "WSAGetLastError() = " << err << "\n";
    }
    return;
  }
  CLOSESOCKET(sock);

  int numAddresses = retlen / sizeof(INTERFACE_INFO);
  for (int i = 0; i < numAddresses; i++) {
    // Only add the address if the interface is running
    if (info[i].iiFlags & IFF_UP) {
      if (info[i].iiAddress.Address.sa_family == INETSOCKET) {
	struct sockaddr_in* iaddr = &info[i].iiAddress.AddressIn;
	CORBA::String_var s;
	s = tcpConnection::ip4ToString(iaddr->sin_addr.s_addr);

	// Just to catch us out, Windows ME apparently sometimes
	// returns 0.0.0.0 as one of its interfaces...
	if (omni::strMatch((const char*)s, "0.0.0.0")) continue;

	ifaddrs.push_back(s._retn());
      }
    }
  }

  if ( orbParameters::dumpConfiguration || omniORB::trace(20) ) {
    omniORB::logger log;
    omnivector<const char*>::iterator i = ifaddrs.begin();
    omnivector<const char*>::iterator last = ifaddrs.end();
    log << "My addresses are: \n";
    while ( i != last ) {
      log << "omniORB: " << (const char*)(*i) << "\n";
      i++;
    }
  }
}

#endif

OMNI_NAMESPACE_END(omni)
