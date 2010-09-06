#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)chaitin_linux.cpp	1.5 03/12/23 16:37:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_chaitin_linux.cpp.incl"

void PhaseRegAlloc::pd_preallocate_hook() {
  // no action
}

#ifdef ASSERT
void PhaseRegAlloc::pd_postallocate_verify_hook() {
  // no action
}
#endif


// Reconciliation History
// chaitin_solaris.cpp	1.7 99/07/12 23:54:22
// End
