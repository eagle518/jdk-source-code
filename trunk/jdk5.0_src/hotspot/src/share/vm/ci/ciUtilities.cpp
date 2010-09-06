#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciUtilities.cpp	1.5 03/12/23 16:39:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciUtilities.cpp.incl"

// ciUtilities
//
// Miscellaneous internal compiler interface routines.

// ------------------------------------------------------------------
// basictype_to_str
const char* basictype_to_str(BasicType t) {
  const char* str = type2name(t);
  if (str == NULL) return "illegal";
  return str;
}

// ------------------------------------------------------------------
// basictype_to_char
const char basictype_to_char(BasicType t) {
  char c = type2char(t);
  return c ? c : 'X';
}
