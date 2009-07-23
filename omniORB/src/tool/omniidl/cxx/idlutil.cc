// -*- c++ -*-
//                          Package   : omniidl
// idlutil.cc               Created on: 1999/10/11
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Utility functions

// $Id$
// $Log$
// Revision 1.5.2.7  2003/01/16 11:08:27  dgrisby
// Patches to support Digital Mars C++. Thanks Christof Meerwald.
//
// Revision 1.5.2.6  2002/01/15 16:38:14  dpg1
// On the road to autoconf. Dependencies refactored, configure.ac
// written. No makefiles yet.
//
// Revision 1.5.2.5  2001/06/21 11:17:15  sll
// Added darwin port.
//
// Revision 1.5.2.4  2001/06/08 17:12:24  dpg1
// Merge all the bug fixes from omni3_develop.
//
// Revision 1.5.2.3  2000/10/27 16:31:10  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.5.2.2  2000/10/10 10:18:51  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.3.2.2  2000/09/22 10:50:21  dpg1
// Digital Unix uses strtoul, not strtoull
//
// Revision 1.3.2.1  2000/08/07 15:34:36  dpg1
// Partial back-port of long long from omni3_1_develop.
//
// Revision 1.3  1999/11/04 17:16:54  dpg1
// Changes for NT.
//
// Revision 1.2  1999/11/02 17:07:24  dpg1
// Changes to compile on Solaris.
//
// Revision 1.1  1999/10/27 14:05:54  dpg1
// *** empty log message ***
//

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <idlutil.h>

char* idl_strdup(const char* s)
{
  if (s) {
    char* ret = new char[strlen(s)+1];
    strcpy(ret, s);
    return ret;
  }
  else
    return 0;
}

IDL_WChar* idl_wstrdup(const IDL_WChar* s)
{
  if (s) {
    int i, len;
    for (len=0; s[len]; len++);
    IDL_WChar* ret = new IDL_WChar[len+1];
    for (i=0; i<len; i++)
      ret[i] = s[i];
    ret[i] = 0;
    return ret;
  }
  else
    return 0;
}

int idl_wstrlen(const IDL_WChar* s)
{
  int l;
  for (l=0; *s; ++s, ++l);
  return l;
}

IDL_WChar* idl_wstrcpy(IDL_WChar* a, const IDL_WChar* b)
{
  IDL_WChar* r = a;
  for (; *b; ++a, ++b) *a = *b;
  *a = 0;
  return r;
}


IDL_WChar* idl_wstrcat(IDL_WChar* a, const IDL_WChar* b)
{
  IDL_WChar* r = a;
  for (; *a; ++a);
  for (; *b; ++a, ++b) *a = *b;
  *a = 0;
  return r;
}

#ifndef HAVE_STRCASECMP
#include <ctype.h>

int strcasecmp(const char* s1, const char* s2)
{
  for (; *s1 && *s2; ++s1, ++s2)
    if (toupper(*s1) != toupper(*s2))
      break;

  if      (!*s1 && !*s2)                return 0;
  else if (toupper(*s1) < toupper(*s2)) return -1;
  else                                  return 1;
}
#endif


#ifdef HAS_LongLong

#  if defined(_MSC_VER)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  IdlIntLiteral ull;
  switch (base) {
  case 8:
    sscanf(text, "%I64o", &ull);
    break;
  case 10:
    sscanf(text, "%I64d", &ull);
    break;
  case 16:
    sscanf(text, "%I64x", &ull);
    break;
  default:
    abort();
  }
  return ull;
}

#  elif defined(__hpux__)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  IdlIntLiteral ull;
  switch (base) {
  case 8:
    sscanf(text, "%llo", &ull);
    break;
  case 10:
    sscanf(text, "%lld", &ull);
    break;
  case 16:
    sscanf(text, "%llx", &ull);
    break;
  default:
    abort();
  }
  return ull;
}

#  elif defined(HAVE_STRTOUL) && SIZEOF_LONG == 8

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoul(text, 0, base);
}

#  elif defined(HAVE_STRTOUQ)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtouq(text, 0, base);
}

#  elif defined(HAVE_STRTOULL)

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoull(text, 0, base);
}

#  else

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoul(text, 0, base);
}

#  endif

#else

// No long long support

IdlIntLiteral
idl_strtoul(const char* text, int base)
{
  return strtoul(text, 0, base);
}

#endif


IdlFloatLiteral
idl_strtod(const char* text)
{
  // *** Should cope with long double
  return strtod(text,0);
}
