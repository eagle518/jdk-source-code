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
#include "incls/_icache_x86.cpp.incl"

#define __ _masm->

void ICacheStubGenerator::generate_icache_flush(ICache::flush_icache_stub_t* flush_icache_stub) {
  StubCodeMark mark(this, "ICache", "flush_icache_stub");

  address start = __ pc();
#ifdef AMD64

  const Register addr  = c_rarg0;
  const Register lines = c_rarg1;
  const Register magic = c_rarg2;

  Label flush_line, done;

  __ testl(lines, lines);
  __ jcc(Assembler::zero, done);

  // Force ordering wrt cflush.
  // Other fence and sync instructions won't do the job.
  __ mfence();

  __ bind(flush_line);
  __ clflush(Address(addr, 0));
  __ addptr(addr, ICache::line_size);
  __ decrementl(lines);
  __ jcc(Assembler::notZero, flush_line);

  __ mfence();

  __ bind(done);

#else
  const Address magic(rsp, 3*wordSize);
  __ lock(); __ addl(Address(rsp, 0), 0);
#endif // AMD64
  __ movptr(rax, magic); // Handshake with caller to make sure it happened!
  __ ret(0);

  // Must be set here so StubCodeMark destructor can call the flush stub.
  *flush_icache_stub = (ICache::flush_icache_stub_t)start;
}

#undef __
