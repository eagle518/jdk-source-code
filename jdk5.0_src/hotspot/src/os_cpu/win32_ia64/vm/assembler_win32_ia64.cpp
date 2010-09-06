#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_win32_ia64.cpp	1.7 03/12/23 16:38:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_win32_ia64.cpp.incl"


/*
 * We don't have implicit null check support yet so we have
 * to do explicit checks for nulls. 
 */
bool MacroAssembler::needs_explicit_null_check(intptr_t offset) {
#if 0
  return offset < 0 || (intptr_t)os::vm_page_size() <= offset;
#else
  return true;
#endif
}

