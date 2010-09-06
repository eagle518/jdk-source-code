#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_version_i486.cpp	1.45 04/04/19 16:18:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version_i486.cpp.incl"


int VM_Version::_cpu;
int VM_Version::_cpuFeatures;
const char* VM_Version::_features_str = "";

static BufferBlob* stub_blob;
static const int stub_size = 120;

extern "C" {
    typedef void (*_getPsrInfo_stub_t)(jint *, jint *, jint *);
}
static _getPsrInfo_stub_t getPsrInfo_stub = NULL;


class VM_Version_StubGenerator: public StubCodeGenerator {
 public:

  VM_Version_StubGenerator(CodeBuffer *c) : StubCodeGenerator(c) {}

  address generate_getPsrInfo() {
    // Flags to test CPU type.
    const uint32_t EFL_AC           = 0x40000;
    const uint32_t EFL_ID           = 0x200000;
    // Values for when we don't have a CPUID instruction.
    const int      CPU_FAMILY_SHIFT = 8;
    const uint32_t CPU_FAMILY_386   = (3 << CPU_FAMILY_SHIFT);
    const uint32_t CPU_FAMILY_486   = (4 << CPU_FAMILY_SHIFT);

    Label cpu386, cpu486, done;

    StubCodeMark mark(this, "VM_Version", "getPsrInfo_stub");
#   define __ _masm->

    address start = __ pc();

    //
    // void cpuInfo(jint *cpu, jint *featureFlags, jint *logical_processors)
    //
    __ pushl(ebp);
    __ movl(ebp, esp);
    __ pushl(ebx);
    __ pushfd();		// preserve ebx and flags
    __ popl(eax);
    __ pushl(eax);
    __ movl(ecx, eax);
    __ xorl(edx,edx);		// initialize feature flags return to 0
    //
    // if we are unable to change the AC flag, we have a 386
    //
    __ xorl(eax, EFL_AC);
    __ pushl(eax);
    __ popfd();
    __ pushfd();
    __ popl(eax);
    __ cmpl(eax, ecx);
    __ jcc(Assembler::equal, cpu386);
    //
    // If we are unable to change the ID flag, we have a 486 which does
    // not support the "cpuid" instruction.
    //
    __ movl(eax, ecx);
    __ xorl(eax, EFL_ID);
    __ pushl(eax);
    __ popfd();
    __ pushfd();
    __ popl(eax);
    __ cmpl(ecx, eax);
    __ jcc(Assembler::equal, cpu486);

    //
    // at this point, we have a chip which supports the "cpuid" instruction
    //
    __ xorl(eax, eax);
    __ cpuid();
    __ orl(eax, eax);
    __ jcc(Assembler::equal, cpu486);	// if cpuid doesn't support an input value
					// value of at least 1, we give up and
					// assume a 486
    __ movl(eax, 1);
    __ cpuid();
    __ jmp(done);

    __ bind(cpu386);
    __ movl(eax, CPU_FAMILY_386);
    __ jmp(done);

    __ bind(cpu486);
    __ movl(eax, CPU_FAMILY_486);
    // fall-through

    //
    // store result and return
    //
    __ bind(done);
    __ movl(ecx, Address(ebp, 8));
    __ movl(Address(ecx, 0), eax);  // store cpu info
    __ movl(ecx, Address(ebp, 12));
    __ movl(Address(ecx, 0), edx);  // store feature flags
    __ movl(ecx, Address(ebp, 16));
    __ movl(Address(ecx, 0), ebx);  // store logical processor count
    __ popfd();
    __ popl(ebx);
    __ popl(ebp);
    __ ret(0);

#   undef __

    return start;
  };
};


void VM_Version::get_processor_features() {

  if (Use486InstrsOnly) {
    _cpu = 4;
    _cpuFeatures = 0;
    _logical_processors_per_package = 1;
  } else {
    jint cpu, featureFlags, logical_processors;

    getPsrInfo_stub(&cpu, &featureFlags, &logical_processors);

    _cpu = cpu_family(cpu);
    if (is_pentium4_family(cpu)) {
      _cpu |= extended_cpu_family(cpu) << 4;
    }
    _cpuFeatures = featureFlags;
    // Logical processors are only available on P4s and above,
    // and only if hyperthreading is available.
    _logical_processors_per_package = 1;
    if (is_pentium4_family(cpu) || is_extended_family(cpu)) {
      if (supports_ht()) {
        _logical_processors_per_package =
          logical_processor_count(logical_processors);
      }
    }
  }
  _supports_cx8 = supports_cmpxchg8();
  // if the OS doesn't support SSE, we can't use this feature even if the HW does
  if( !os::supports_sse())
    _cpuFeatures &= ~(CPU_SSE|CPU_SSE2);
#ifdef  COMPILER2
  if (UseSSE < 2)
    _cpuFeatures &= ~CPU_SSE2;
  if (UseSSE < 1)
    _cpuFeatures &= ~CPU_SSE;
#endif

  char buf[256];
  jio_snprintf(buf, sizeof(buf), "family %d%s%s%s%s%s%s%s",
               cpu_family(),
               (supports_cmov() ? ", cmov" : ""),
               (supports_cmpxchg8() ? ", cx8" : ""),
               (supports_fxsr() ? ", fxsr" : ""),
               (supports_mmx() ? ", mmx" : ""),
               (supports_sse() ? ", sse" : ""),
               (supports_sse2() ? ", sse2" : ""),
               (supports_ht() ? ", ht": ""));
  _features_str = strdup(buf);

#ifndef PRODUCT
  if (PrintMiscellaneous && Verbose) {
    tty->print_cr("CPU: %s", cpu_features());
    tty->print_cr("Logical CPUs per package: %u",
                  logical_processors_per_package());
  }
#endif
}

void VM_Version::initialize() {
  ResourceMark rm;
  // Making this stub must be FIRST use of assembler

  stub_blob = BufferBlob::create("getPsrInfo_stub", stub_size);
  if (stub_blob == NULL) {
    vm_exit_during_initialization("Unable to allocate getPsrInfo_stub");
  }
  CodeBuffer* c = new CodeBuffer(stub_blob->instructions_begin(),
                                 stub_blob->instructions_size());
  VM_Version_StubGenerator g(c);
  getPsrInfo_stub = CAST_TO_FN_PTR(_getPsrInfo_stub_t,
                                   g.generate_getPsrInfo());

  get_processor_features();
}

void VM_Version_init() {
  VM_Version::initialize();
}
