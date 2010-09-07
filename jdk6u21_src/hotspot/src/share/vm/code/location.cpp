/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_location.cpp.incl"

void Location::print_on(outputStream* st) const {
  if(type() == invalid) {
    // product of Location::invalid_loc() or Location::Location().
    switch (where()) {
    case on_stack:     st->print("empty");    break;
    case in_register:  st->print("invalid");  break;
    }
    return;
  }
  switch (where()) {
  case on_stack:    st->print("stack[%d]", stack_offset());    break;
  case in_register: st->print("reg %s [%d]", reg()->name(), register_number()); break;
  default:          st->print("Wrong location where %d", where());
  }
  switch (type()) {
  case normal:                                 break;
  case oop:          st->print(",oop");        break;
  case narrowoop:    st->print(",narrowoop");  break;
  case int_in_long:  st->print(",int");        break;
  case lng:          st->print(",long");       break;
  case float_in_dbl: st->print(",float");      break;
  case dbl:          st->print(",double");     break;
  case addr:         st->print(",address");    break;
  default:           st->print("Wrong location type %d", type());
  }
}


Location::Location(DebugInfoReadStream* stream) {
  _value = (juint) stream->read_int();
}


void Location::write_on(DebugInfoWriteStream* stream) {
  stream->write_int(_value);
}


// Valid argument to Location::new_stk_loc()?
bool Location::legal_offset_in_bytes(int offset_in_bytes) {
  if ((offset_in_bytes % BytesPerInt) != 0)  return false;
  return (juint)(offset_in_bytes / BytesPerInt) < (OFFSET_MASK >> OFFSET_SHIFT);
}
