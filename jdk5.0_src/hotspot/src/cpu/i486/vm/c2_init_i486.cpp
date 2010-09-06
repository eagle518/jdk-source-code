#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c2_init_i486.cpp	1.15 04/02/25 22:15:02 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c2_init_i486.cpp.incl"

// processor dependent initialization for i486

void Compile::pd_compiler2_init() {  
  guarantee(CodeEntryAlignment >= InteriorEntryAlignment, "" );
  if (!VM_Version::supports_cmov()) {
    ConditionalMoveLimit = 0;
  }
  // UseSSE is set to the smaller of what hardware supports and what
  // the command line requires.  I.e., you cannot set UseSSE to 2 on
  // older Pentiums which do not support it.
  if( UseSSE > 2 ) UseSSE=2;
  if( UseSSE < 0 ) UseSSE=0;
  if( !VM_Version::supports_sse2() ) // Drop to 1 if no SSE2 support
    UseSSE = MIN2(1,UseSSE);
  if( !VM_Version::supports_sse () ) // Drop to 0 if no SSE  support
    UseSSE = 0;
#ifndef PRODUCT
  if (PrintMiscellaneous && Verbose) {
    static const char *msg[3] = {"SSE is off", "SSE only (no SSE2)", "SSE/SSE2" };
    tty->print_cr(msg[UseSSE]);
  }
#endif
}
