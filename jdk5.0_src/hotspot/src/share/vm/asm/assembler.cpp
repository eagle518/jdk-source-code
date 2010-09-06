#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler.cpp	1.28 03/12/23 16:38:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler.cpp.incl"


// Implementation of AbstractAssembler
//
// The AbstractAssembler is generating code into a CodeBuffer. To make code generation faster,
// the assembler keeps a copy of the code buffers boundaries & modifies them when
// emitting bytes rather than using the code buffers accessor functions all the time.
// The code buffer is updated via set_code_end(...) after emiting a whole instruction.

AbstractAssembler::AbstractAssembler(CodeBuffer* code) {
  if( !code ) return;
  _code	       = code;
  _code_begin  = code->code_begin();
  _code_limit  = code->code_limit();
  _code_pos    = code->code_end();
  _inst_mark   = NULL;
  _oop_recorder= code->oop_recorder();
}


void AbstractAssembler::flush() {
  ICache::invalidate_range(addr_at(0), offset());
}


void AbstractAssembler::a_byte(int x) {
  emit_byte(x);
}


void AbstractAssembler::a_long(jint x) {
  emit_long(x);
}




// Labels refer to positions in the (to be) generated code.
// There are bound, unbound and undefined labels.
//
// Bound labels refer to known positions in the already
// generated code. pos() is the position the label refers to.
//
// Unbound labels refer to unknown positions in the code
// to be generated; pos() is the position of the 32bit
// Displacement of the last instruction using the label.
//
// Undefined labels are labels that haven't been used yet.
// They refer to no position at all.


#ifndef PRODUCT
void AbstractAssembler::print(Label& L) {
  if (L.is_unused()) {
    tty->print_cr("unused label");
  } else if (L.is_bound()) {
    tty->print_cr("bound label to %d", L.pos());
  } else if (L.is_unbound()) {
    Label l = L;
    tty->print_cr("unbound label");
    while (l.is_unbound()) {
      Displacement disp = disp_at(l);
      tty->print("@ %d ", l.pos());
      disp.print();
      tty->cr();
      disp.next(l);
    }
  } else {
    tty->print_cr("label in inconsistent state (pos = %d)", L._pos);
  }
}
#endif // PRODUCT


void AbstractAssembler::bind_to(Label& L, int pos) {
  assert(0 <= pos && pos <= offset(), "must have a valid binding position");
  while (L.is_unbound()) {
    disp_at(L).bind(L, pos, this);
  }
  L.bind_to(pos);
}


void AbstractAssembler::bind(Label& L) {
  if (!L.is_bound())
    bind_to(L, offset());
  else
    guarantee(L.pos() == offset(), "attempt to redefine label");
}

void AbstractAssembler::generate_stack_overflow_check( int frame_size_in_bytes) {
  if (UseStackBanging) {
    // Each code entry causes one stack bang n pages down the stack where n
    // is configurable by StackBangPages.  The setting depends on the maximum
    // depth of VM call stack or native before going back into java code,
    // since only java code can raise a stack overflow exception using the
    // stack banging mechanism.  The VM and native code does not detect stack
    // overflow.
    // The code in JavaCalls::call() checks that there is at least n pages
    // available, so all entry code needs to do is bang once for the end of
    // this shadow zone.
    // The entry code may need to bang additional pages if the framesize
    // is greater than a page.

    const int page_size = os::vm_page_size();
    int bang_end = StackShadowPages*page_size;

    // This is how far the previous frame's stack banging extended.
    const int bang_end_safe = bang_end;

    if (frame_size_in_bytes > page_size) {
      bang_end += frame_size_in_bytes;
    }

    int bang_offset = bang_end_safe;
    while (bang_offset <= bang_end) {
      // Need at least one stack bang at end of shadow zone.
      bang_stack_with_offset(bang_offset);
      bang_offset += page_size;
    }
  } // end (UseStackBanging)
}


void Label::report_bad_label() const {
  ShouldNotReachHere();
}
