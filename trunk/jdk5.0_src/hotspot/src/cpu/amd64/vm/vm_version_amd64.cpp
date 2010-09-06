#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_version_amd64.cpp	1.3 04/06/08 14:52:00 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version_amd64.cpp.incl"

VM_Version::CpuidInfo VM_Version::_cpuid_info = { 0, };

extern "C" {
  typedef void (*getPsrInfo_stub_t)(void*);
}

// So we can see the stub after it's made
static getPsrInfo_stub_t getPsrInfo_stub = NULL;

class VM_Version_StubGenerator : public StubCodeGenerator {
 public:
  VM_Version_StubGenerator(CodeBuffer *c) : StubCodeGenerator(c) {}

  // void getPsrInfo(VM_Version::CpuidInfo* cpuid_info);
  address generate_getPsrInfo() {
    StubCodeMark mark(this, "VM_Version", "getPsrInfo_stub");
#   define __ _masm->

    address start = __ pc();

    // rbx is callee-save on both unix and windows
    // rcx and rdx are first and second argument registers on windows

    __ pushq(rbx);
    __ movq(r8, rarg0);

    __ xorl(rax, rax);
    __ cpuid();
    __ leaq(r9, Address(r8, in_bytes(VM_Version::std_cpuid0_offset())));
    __ movl(Address(r9, 0),  rax);
    __ movl(Address(r9, 4),  rbx);
    __ movl(Address(r9, 8),  rcx);
    __ movl(Address(r9, 12), rdx);

    __ movl(rax, 1);
    __ cpuid();
    __ leaq(r9, Address(r8, in_bytes(VM_Version::std_cpuid1_offset())));
    __ movl(Address(r9, 0),  rax);
    __ movl(Address(r9, 4),  rbx);
    __ movl(Address(r9, 8),  rcx);
    __ movl(Address(r9, 12), rdx);

    __ movl(rax, 0x80000001);
    __ cpuid();
    __ leaq(r9, Address(r8, in_bytes(VM_Version::ext_cpuid1_offset())));
    __ movl(Address(r9, 0),  rax);
    __ movl(Address(r9, 4),  rbx);
    __ movl(Address(r9, 8),  rcx);
    __ movl(Address(r9, 12), rdx);

    __ popq(rbx);

    __ ret(0);

    return start;
  }
};

void VM_Version::initialize() {
  ResourceMark rm;

  // Create the processor info stub
  BufferBlob* blob = BufferBlob::create("getPsrInfo stub", 160);
  if (blob == NULL) {
    vm_exit_during_initialization("Unable to allocate getPsrInfo stub");
  }
  CodeBuffer* buffer = new CodeBuffer(blob->instructions_begin(),
                                      blob->instructions_size());
  VM_Version_StubGenerator g(buffer);
  getPsrInfo_stub = CAST_TO_FN_PTR(getPsrInfo_stub_t, g.generate_getPsrInfo());

  // Get raw processor info
  getPsrInfo_stub(&_cpuid_info);

  // We know 64-bit x86's support cmpxchg8
  _supports_cx8 = true;

  // Prefetch settings
  PrefetchCopyIntervalInBytes = prefetch_copy_interval_in_bytes();
  PrefetchCopyIntervalInBytes = prefetch_scan_interval_in_bytes();
  PrefetchFieldsAhead         = prefetch_fields_ahead();
}

void VM_Version_init() {
  VM_Version::initialize();
}
