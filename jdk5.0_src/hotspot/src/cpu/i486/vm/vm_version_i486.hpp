#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_version_i486.hpp	1.20 04/04/19 16:17:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VM_Version: public Abstract_VM_Version {

protected:
   static int _cpu;
   static int _cpuFeatures;	// features returned by the "cpuid" instruction
				// 0 if this instruction is not available
   static const char* _features_str;

   enum {
     CPU_CX8  = (1 << 8),
     CPU_CMOV = (1 << 15),
     CPU_MMX  = (1 << 23),
     CPU_FXSR = (1 << 24),
     CPU_SSE  = (1 << 25),
     CPU_SSE2 = (1 << 26),
     CPU_HT   = (1 << 28)
   } cpuFeatureFlags;

  // Extractors and predicates
  static uint32_t cpu_family(uint32_t eax) {
    // CPU family from eax
    const uint32_t CpuFamily_shift = 8;
    const uint32_t CpuFamily_mask  = 0xf;
    uint32_t result =
      (eax >> CpuFamily_shift) & CpuFamily_mask;
    return result;
  }
  static bool is_pentium4_family(uint32_t eax) {
    // CPU family from eax
    const uint32_t Pentium4Family = 0xf;
    bool result = cpu_family(eax) == Pentium4Family;
    return result;
  }
  static uint32_t extended_cpu_family(uint32_t eax) {
    // Extended family from eax
    const uint32_t ExtendedCpuFamily_shift = 16;
    const uint32_t ExtendedCpuFamily_mask  = 0xff;
    uint32_t result =
      (eax >> ExtendedCpuFamily_shift) & ExtendedCpuFamily_mask;
    return result;
  }
  static bool is_extended_family(uint32_t eax) {
    bool result = extended_cpu_family(eax) != 0x0;
    return result;
  }
  static unsigned int logical_processor_count(uint32_t ebx) {
    // Logical processor count in ebx
    const uint32_t LogicalProcessors_shift = 16;
    const uint32_t LogicalProcessors_mask  = 0xff;
    unsigned int result =
      (ebx >> LogicalProcessors_shift) & LogicalProcessors_mask;
    return result;
  }

  static void get_processor_features();

public:
  static void initialize();
  //
  // Processor family:
  //       3   -  386
  //       4   -  486
  //       5   -  Pentium
  //       6   -  PentiumPro, Pentium II, Celeron, Zenon, Pentium III
  //    0x0f   -  Pentium 4
  //
  // Note: The cpu family should be used to select between
  //       instruction sequences which are valid on all Intel
  //       processors.  Use the feature test functions below to
  //       determine whether a particular instruction is supported.
  //
  static int  cpu_family()        { return _cpu;}
  static bool is_P6()             { return cpu_family() >= 6; }

  //
  // Feature identification
  //
  static bool supports_cpuid()		{ return _cpuFeatures  != 0; }
  static bool supports_cmov()		{ return (_cpuFeatures & CPU_CMOV) != 0; }
  static bool supports_cmpxchg8()	{ return (_cpuFeatures & CPU_CX8) != 0; }
  static bool supports_mmx()		{ return (_cpuFeatures & CPU_MMX) != 0; }
  static bool supports_fxsr()		{ return (_cpuFeatures & CPU_FXSR) != 0; }
  static bool supports_sse()		{ return (_cpuFeatures & CPU_SSE) != 0; }
  static bool supports_sse2()		{ return (_cpuFeatures & CPU_SSE2) != 0; }
  static bool supports_ht()		{ return (_cpuFeatures & CPU_HT) != 0; }

  static bool supports_compare_and_exchange()    { return true; }

  static const char* cpu_features()           { return _features_str; }
};
