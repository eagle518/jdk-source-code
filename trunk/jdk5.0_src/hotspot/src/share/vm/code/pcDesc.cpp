#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)pcDesc.cpp	1.22 03/12/23 16:39:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_pcDesc.cpp.incl"

PcDesc::PcDesc(int pc_offset, bool at_call, int scope_decode_offset) {
  _pc_offset           = pc_offset;
  _scope_decode_offset = at_call
                       ? scope_decode_offset
                       :-scope_decode_offset;
}

address PcDesc::real_pc(const nmethod* code) const {
  return code->instructions_begin() + pc_offset();
}

void PcDesc::print(nmethod* code) {
#ifndef PRODUCT
  ResourceMark rm;
  tty->print_cr("PcDesc(pc=0x%lx offset=%x):", real_pc(code), pc_offset());

  for (ScopeDesc* sd = code->scope_desc_at(real_pc(code), at_call());
       sd != NULL;
       sd = sd->sender()) {
    tty->print("  ");
    sd->method()->print_short_name(tty);
    tty->print("  @%d", sd->bci());
    tty->print("  (%s)", at_call() ? "at_call" : "not_at_call");
    tty->cr();
  }
#endif
}

bool PcDesc::verify(nmethod* code) {
  //Unimplemented();
  return true;
}

