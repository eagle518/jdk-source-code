/*
 * Copyright (c) 1999, 2001, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
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
// chaitin_solaris.cpp  1.7 99/07/12 23:54:22
// End
