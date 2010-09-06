#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciFlags.cpp	1.5 03/12/23 16:39:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciFlags.cpp.incl"

// ciFlags
//
// This class represents klass or method flags

// ------------------------------------------------------------------
// ciFlags::print_klass_flags
void ciFlags::print_klass_flags() {
  if (is_public()) {
    tty->print("public");
  } else {
    tty->print("DEFAULT_ACCESS");
  }

  if (is_final()) {
    tty->print(",final");
  }
  if (is_super()) {
    tty->print(",super");
  }
  if (is_interface()) {
    tty->print(",interface");
  }
  if (is_abstract()) {
    tty->print(",abstract");
  }
}

// ------------------------------------------------------------------
// ciFlags::print_member_flags
void ciFlags::print_member_flags() {
  if (is_public()) {
    tty->print("public");
  } else if (is_private()) {
    tty->print("private");
  } else if (is_protected()) {
    tty->print("protected");
  } else {
    tty->print("DEFAULT_ACCESS");
  }

  if (is_static()) {
    tty->print(",static");
  }
  if (is_final()) {
    tty->print(",final");
  }
  if (is_synchronized()) {
    tty->print(",synchronized");
  }
  if (is_volatile()) {
    tty->print(",volatile");
  }
  if (is_transient()) {
    tty->print(",transient");
  }
  if (is_native()) {
    tty->print(",native");
  }
  if (is_abstract()) {
    tty->print(",abstract");
  }
  if (is_strict()) {
    tty->print(",strict");
  }
    
}

// ------------------------------------------------------------------
// ciFlags::print
void ciFlags::print() {
  tty->print(" flags=%x", _flags);
}
