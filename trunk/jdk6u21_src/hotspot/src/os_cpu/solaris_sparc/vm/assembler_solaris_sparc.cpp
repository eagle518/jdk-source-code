/*
 * Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_assembler_solaris_sparc.cpp.incl"

#include <sys/trap.h>          // For trap numbers
#include <v9/sys/psr_compat.h> // For V8 compatibility

void MacroAssembler::read_ccr_trap(Register ccr_save) {
  // Execute a trap to get the PSR, mask and shift
  // to get the condition codes.
  get_psr_trap();
  nop();
  set(PSR_ICC, ccr_save);
  and3(O0, ccr_save, ccr_save);
  srl(ccr_save, PSR_ICC_SHIFT, ccr_save);
}

void MacroAssembler::write_ccr_trap(Register ccr_save, Register scratch1, Register scratch2) {
  // Execute a trap to get the PSR, shift back
  // the condition codes, mask the condition codes
  // back into and PSR and trap to write back the
  // PSR.
  sll(ccr_save, PSR_ICC_SHIFT, scratch2);
  get_psr_trap();
  nop();
  set(~PSR_ICC, scratch1);
  and3(O0, scratch1, O0);
  or3(O0, scratch2, O0);
  set_psr_trap();
  nop();
}

void MacroAssembler::flush_windows_trap() { trap(ST_FLUSH_WINDOWS); }
void MacroAssembler::clean_windows_trap() { trap(ST_CLEAN_WINDOWS); }
void MacroAssembler::get_psr_trap()       { trap(ST_GETPSR); }
void MacroAssembler::set_psr_trap()       { trap(ST_SETPSR); }
