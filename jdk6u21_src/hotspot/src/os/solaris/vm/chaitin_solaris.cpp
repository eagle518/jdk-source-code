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
#include "incls/_chaitin_solaris.cpp.incl"

void PhaseRegAlloc::pd_preallocate_hook() {
  // no action
}

#ifdef ASSERT
void PhaseRegAlloc::pd_postallocate_verify_hook() {
  // no action
}
#endif


//Reconciliation History
// 1.1 99/02/12 15:35:26 chaitin_win32.cpp
// 1.2 99/02/18 15:38:56 chaitin_win32.cpp
// 1.4 99/03/09 10:37:48 chaitin_win32.cpp
// 1.6 99/03/25 11:07:44 chaitin_win32.cpp
// 1.8 99/06/22 16:38:58 chaitin_win32.cpp
//End
