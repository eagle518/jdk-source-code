#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icache_amd64.cpp	1.5 04/03/12 17:39:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icache_amd64.cpp.incl"

#define __ _masm->

void ICacheStubGenerator::generate_icache_flush(
  ICache::flush_icache_stub_t* flush_icache_stub
) {
  StubCodeMark mark(this, "ICache", "flush_icache_stub");

  address start = __ pc();

  const Register addr  = rarg0;
  const Register lines = rarg1;
  const Register magic = rarg2;

  Label flush_line, done;

  __ testl(lines, lines);
  __ jcc(Assembler::zero, done);

  // Force ordering wrt cflush.
  // Other fence and sync instructions won't do the job.
  __ mfence();

  __ bind(flush_line);
  __ clflush(Address(addr));
  __ addq(addr, ICache::line_size);
  __ decl(lines);
  __ jcc(Assembler::notZero, flush_line);

  __ mfence();

  __ bind(done);
  __ movl(rax, magic); // Handshake with caller to make sure it happened!
  __ ret(0);

  // Must be set here so StubCodeMark destructor can call the flush stub.
  *flush_icache_stub = (ICache::flush_icache_stub_t)start;
};

#undef __
