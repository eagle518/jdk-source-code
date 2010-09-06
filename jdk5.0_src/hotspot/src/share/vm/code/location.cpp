#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)location.cpp	1.31 03/12/23 16:39:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_location.cpp.incl"
  
const int Location::OFFSET_MASK  = (jchar) 0x0FFF;
const int Location::OFFSET_SHIFT = 0;
const int Location::TYPE_MASK    = (jchar) 0x7000;
const int Location::TYPE_SHIFT   = 12;
const int Location::WHERE_MASK   = (jchar) 0x8000;
const int Location::WHERE_SHIFT  = 15;

void Location::print_on(outputStream* st) const {
  switch (where()) {
  case on_stack:    st->print("stack[%d]", stack_offset());    break;
  case in_register: st->print("reg %s [%d]", SharedInfo::regName[register_number()], register_number()); break;
  default:          st->print("Wrong location where %d", where());
  }
  switch (type()) {
  case normal:                                 break;
  case oop:          st->print(",oop");        break;
  case int_in_long:  st->print(",int");        break;
  case lng:          st->print(",long");       break;
  case float_in_dbl: st->print(",float");      break;
  case dbl:          st->print(",double");     break;
  case addr:         st->print(",address");    break;
  default:           st->print("Wrong location type %d", type());
  }
}


Location::Location(DebugInfoReadStream* stream) {
  _value = (uint16_t) stream->read_int();
}


void Location::write_on(DebugInfoWriteStream* stream) {
  stream->write_int(_value & 0x0000FFFF);
}


bool Location::legal_offset_in_bytes(int offset_in_bytes) {
  return (offset_in_bytes / BytesPerWord) < (OFFSET_MASK >> OFFSET_SHIFT);
}

