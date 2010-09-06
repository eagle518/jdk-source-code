#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciConstant.cpp	1.6 03/12/23 16:39:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciConstant.cpp.incl"

// ciConstant
//
// This class represents a constant value.

// ------------------------------------------------------------------
// ciConstant::print
void ciConstant::print() {
  tty->print("<ciConstant type=%s value=",
	     basictype_to_str(basic_type()));
  switch (basic_type()) {
  case T_BOOLEAN:
    tty->print("%s", bool_to_str(_value._int == 0));
    break;
  case T_CHAR:
  case T_BYTE:
  case T_SHORT:
  case T_INT:
    tty->print("%d", _value._int);
    break;
  case T_LONG:
    tty->print("%ld", _value._long);
    break;
  case T_FLOAT:
    tty->print("%f", _value._float);
    break;
  case T_DOUBLE:
    tty->print("%lf", _value._double);
    break;
  case T_OBJECT:
  case T_ARRAY:
    _value._object->print();
    break;
  default:
    tty->print("ILLEGAL");
    break;
  }
  tty->print(">");
}
