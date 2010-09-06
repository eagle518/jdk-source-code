#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icache_ia64.cpp	1.12 04/03/12 17:40:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icache_ia64.cpp.incl"

#define __ _masm->

void ICacheStubGenerator::generate_icache_flush(
  ICache::flush_icache_stub_t* flush_icache_stub
) {
  StubCodeMark mark(this, "ICache", "flush_icache_stub");

  address start = __ emit_fd();

  const Register addr  = GR_I0;
  const Register lines = GR_I1;
  const Register magic = GR_I2;

  const Register save_LC  = GR35;
  const Register save_PFS = GR36;

  const PredicateRegister flush    = PR15_SCRATCH;
  const PredicateRegister no_flush = PR14_SCRATCH;

  __ alloc(save_PFS, 3, 2, 0, 0);
  __ mov(save_LC, AR_LC);
  __ cmp4(no_flush, flush, 0, lines, Assembler::equal);

  __ mov(no_flush, AR_PFS, save_PFS);
  __ sub(lines, lines, 1);

  __ mov(flush, AR_LC, lines);
  __ mov(no_flush, GR_RET, magic);
  __ ret(no_flush);

  Label flush_line;
  __ bind(flush_line);

  __ fc(addr);

  __ add(addr, addr, ICache::line_size);
  __ cloop(flush_line, Assembler::sptk, Assembler::few);

  __ synci();

  __ srlzi();

  __ mov(AR_LC, save_LC);
  __ mov(AR_PFS, save_PFS);
  __ mov(GR_RET, magic);     // Handshake with caller to make sure it happened!
  __ ret();

  // Must be set here so StubCodeMark destructor can call the flush stub.
  *flush_icache_stub = (ICache::flush_icache_stub_t)start;
};

#undef __
