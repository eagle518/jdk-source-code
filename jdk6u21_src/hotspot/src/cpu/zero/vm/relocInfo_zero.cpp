/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2009 Red Hat, Inc.
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
#include "incls/_relocInfo_zero.cpp.incl"

void Relocation::pd_set_data_value(address x, intptr_t o) {
  ShouldNotCallThis();
}

address Relocation::pd_call_destination(address orig_addr) {
  ShouldNotCallThis();
}

void Relocation::pd_set_call_destination(address x) {
  ShouldNotCallThis();
}

address Relocation::pd_get_address_from_code() {
  ShouldNotCallThis();
}

address* Relocation::pd_address_in_code() {
  // Relocations in Shark are just stored directly
  return (address *) addr();
}

int Relocation::pd_breakpoint_size() {
  ShouldNotCallThis();
}

void Relocation::pd_swap_in_breakpoint(address x,
                                       short*  instrs,
                                       int     instrlen) {
  ShouldNotCallThis();
}

void Relocation::pd_swap_out_breakpoint(address x,
                                        short*  instrs,
                                        int     instrlen) {
  ShouldNotCallThis();
}

void poll_Relocation::fix_relocation_after_move(const CodeBuffer* src,
                                                CodeBuffer*       dst) {
  ShouldNotCallThis();
}

void poll_return_Relocation::fix_relocation_after_move(const CodeBuffer* src,
                                                       CodeBuffer*       dst) {
  ShouldNotCallThis();
}
