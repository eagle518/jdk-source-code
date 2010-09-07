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
#include "incls/_assembler_linux_sparc.cpp.incl"

#include <asm-sparc/traps.h>

void MacroAssembler::read_ccr_trap(Register ccr_save) {
  // No implementation
  breakpoint_trap();
}

void MacroAssembler::write_ccr_trap(Register ccr_save, Register scratch1, Register scratch2) {
  // No implementation
  breakpoint_trap();
}

void MacroAssembler::flush_windows_trap() { trap(SP_TRAP_FWIN); }
void MacroAssembler::clean_windows_trap() { trap(SP_TRAP_CWIN); }

// Use software breakpoint trap until we figure out how to do this on Linux
void MacroAssembler::get_psr_trap()       { trap(SP_TRAP_SBPT); }
void MacroAssembler::set_psr_trap()       { trap(SP_TRAP_SBPT); }
