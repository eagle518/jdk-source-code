/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_linux_ia64.cpp.incl"

// We don't have implicit null check support yet so we have
// to do explicit checks for nulls.
bool MacroAssembler::needs_explicit_null_check(intptr_t offset) {
#if 0
  // The kernel is at low addresses on x86 Linux so we can
  // avoid checking for NULL in it's range.
  // I'm not sure that this is true for IA64
  return (offset < 0 || offset >= 0x100000);
#else
  return true;
#endif
}
