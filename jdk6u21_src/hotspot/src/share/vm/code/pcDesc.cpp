/*
 * Copyright (c) 1997, 2010, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_pcDesc.cpp.incl"

PcDesc::PcDesc(int pc_offset, int scope_decode_offset, int obj_decode_offset) {
  assert(sizeof(PcDescFlags) <= 4, "occupies more than a word");
  _pc_offset           = pc_offset;
  _scope_decode_offset = scope_decode_offset;
  _obj_decode_offset   = obj_decode_offset;
  _flags.word          = 0;
}

address PcDesc::real_pc(const nmethod* code) const {
  return code->instructions_begin() + pc_offset();
}

void PcDesc::print(nmethod* code) {
#ifndef PRODUCT
  ResourceMark rm;
  tty->print_cr("PcDesc(pc=0x%lx offset=%x):", real_pc(code), pc_offset());

  if (scope_decode_offset() == DebugInformationRecorder::serialized_null) {
    return;
  }

  for (ScopeDesc* sd = code->scope_desc_at(real_pc(code));
       sd != NULL;
       sd = sd->sender()) {
    tty->print("  ");
    sd->method()->print_short_name(tty);
    tty->print("  @%d", sd->bci());
    if (sd->should_reexecute())
      tty->print("  reexecute=true");
    tty->cr();
  }
#endif
}

bool PcDesc::verify(nmethod* code) {
  //Unimplemented();
  return true;
}
