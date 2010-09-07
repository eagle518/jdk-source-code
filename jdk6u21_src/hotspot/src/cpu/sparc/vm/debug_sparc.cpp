/*
 * Copyright (c) 1999, 2007, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_debug_sparc.cpp.incl"

#ifndef PRODUCT

extern "C" void findpc(int x);


void pd_ps(frame f) {
  intptr_t* sp = f.sp();
  intptr_t* prev_sp = sp - 1;
  intptr_t *pc = NULL;
  intptr_t *next_pc = NULL;
  int count = 0;
  tty->print("register window backtrace from %#x:\n", sp);
  while (sp != NULL && ((intptr_t)sp & 7) == 0 && sp > prev_sp && sp < prev_sp+1000) {
    pc      = next_pc;
    next_pc = (intptr_t*) sp[I7->sp_offset_in_saved_window()];
    tty->print("[%d] sp=%#x pc=", count, sp);
    findpc((intptr_t)pc);
    if (WizardMode && Verbose) {
      // print register window contents also
      tty->print_cr("    L0..L7: {%#x %#x %#x %#x %#x %#x %#x %#x}",
                    sp[0+0],sp[0+1],sp[0+2],sp[0+3],
                    sp[0+4],sp[0+5],sp[0+6],sp[0+7]);
      tty->print_cr("    I0..I7: {%#x %#x %#x %#x %#x %#x %#x %#x}",
                    sp[8+0],sp[8+1],sp[8+2],sp[8+3],
                    sp[8+4],sp[8+5],sp[8+6],sp[8+7]);
      // (and print stack frame contents too??)

      CodeBlob *b = CodeCache::find_blob((address) pc);
      if (b != NULL) {
        if (b->is_nmethod()) {
          methodOop m = ((nmethod*)b)->method();
          int nlocals = m->max_locals();
          int nparams  = m->size_of_parameters();
          tty->print_cr("compiled java method (locals = %d, params = %d)", nlocals, nparams);
        }
      }
    }
    prev_sp = sp;
    sp = (intptr_t *)sp[FP->sp_offset_in_saved_window()];
    sp = (intptr_t *)((intptr_t)sp + STACK_BIAS);
    count += 1;
  }
  if (sp != NULL)
    tty->print("[%d] sp=%#x [bogus sp!]", count, sp);
}

#endif // PRODUCT
