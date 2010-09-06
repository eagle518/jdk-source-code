#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)functionAtEnd.cpp	1.7 03/12/23 16:43:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_functionAtEnd.cpp.incl"

// An empty function, exported from the DLL, which is put last on the
// link line to wind up as the last function in the DLL/DSO. Needed to
// check whether the PC, upon a crash, is within the VM or not.

address JVM_FunctionAtEnd() {
  return CAST_FROM_FN_PTR(address,JVM_FunctionAtEnd);
}
