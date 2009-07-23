// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.cc              Created on: 15/9/99
//                            Author    : David Riddoch (djr)
//
//    Copyright (C) 2003 Apasphere Ltd
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    Implementation of dynamic library 'hook' functions.
//

/*
  $Log$
  Revision 1.4.2.2  2003/10/23 11:25:54  dgrisby
  More valuetype support.

  Revision 1.4.2.1  2003/03/23 21:02:49  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.2.2.9  2003/01/16 11:08:26  dgrisby
  Patches to support Digital Mars C++. Thanks Christof Meerwald.

  Revision 1.2.2.8  2001/11/27 14:35:08  dpg1
  Context, DII fixes.

  Revision 1.2.2.7  2001/11/06 15:41:36  dpg1
  Reimplement Context. Remove CORBA::Status. Tidying up.

  Revision 1.2.2.6  2001/08/17 17:08:35  sll
  Use LinkHack mechanism to ensure dynamicLib is linked.

  Revision 1.2.2.5  2001/04/19 09:14:14  sll
  Scoped where appropriate with the omni namespace.

  Revision 1.2.2.4  2000/10/09 16:26:28  sll
  Temporary disable IR support.

  Revision 1.2.2.3  2000/10/06 16:40:54  sll
  Changed to use cdrStream.

  Revision 1.2.2.2  2000/09/27 17:25:42  sll
  Changed include/omniORB3 to include/omniORB4.

  Revision 1.2.2.1  2000/07/17 10:35:42  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:26:02  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/22 14:26:33  djr
  Major rewrite of orbcore to support POA.

*/

#define ENABLE_CLIENT_IR_SUPPORT
#include <omniORB4/CORBA.h>
#include <omniORB4/callDescriptor.h>
#include <dynamicLib.h>
#include <context.h>
#include <initialiser.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(dynamicLib);

OMNI_NAMESPACE_BEGIN(omni)

//
// Module initialisers

extern omniInitialiser& omni_valueFactory_initialiser_;


static void init();
static void deinit();
static void lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt);


static omniDynamicLib dynamic_ops = {
  init,
  deinit,
  lookup_id_lcfn
};

// Static constructor to initialise omniDynamicLib::hook.
struct omniDynamicLib_initialiser {
  inline omniDynamicLib_initialiser() {
    omniDynamicLib::hook = &dynamic_ops;
  }
  static omniDynamicLib_initialiser instance;
};
omniDynamicLib_initialiser omniDynamicLib_initialiser::instance;


static void
init()
{
  omniORB::logs(5, "Initialising omniDynamic library.");
  omni_valueFactory_initialiser_.attach();
}


static void
deinit()
{
  omniORB::logs(5, "Deinitialising omniDynamic library.");
  ContextImpl::releaseDefault();
  omni_valueFactory_initialiser_.detach();
}


static void
lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt)
{
  omniStdCallDesc::_cCORBA_mObject_i_cstring* tcd = (omniStdCallDesc::_cCORBA_mObject_i_cstring*) cd;
  CORBA::_impl_Repository* impl = (CORBA::_impl_Repository*) svnt->_ptrToInterface(CORBA::Repository::_PD_repoId);
  tcd->pd_result = impl->lookup_id(tcd->arg_0);
}

OMNI_NAMESPACE_END(omni)


#if defined(__DMC__) && defined(_WINDLL)
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
  return TRUE;
}
#endif
