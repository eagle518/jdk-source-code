#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_version_sparc.hpp	1.21 03/12/23 16:37:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class VM_Version: public Abstract_VM_Version {
protected:
  enum Feature_Flag {
    v8_instructions     = 0,
    hardware_int_muldiv = 1,
    hardware_fsmuld     = 2,
    v9_instructions     = 3,
    vis1_instructions   = 4,
    vis2_instructions   = 5
  };

  enum Feature_Flag_Set {
    unknown_m             = 0,
    all_features_m        = -1,

    v8_instructions_m     = 1 << v8_instructions,
    hardware_int_muldiv_m = 1 << hardware_int_muldiv,
    hardware_fsmuld_m     = 1 << hardware_fsmuld,
    v9_instructions_m     = 1 << v9_instructions,
    vis1_instructions_m   = 1 << vis1_instructions,
    vis2_instructions_m   = 1 << vis2_instructions,

    generic_v8_m          = v8_instructions_m | hardware_int_muldiv_m | hardware_fsmuld_m,
    generic_v9_m          = generic_v8_m | v9_instructions_m | vis1_instructions_m,
    ultra3_m              = generic_v9_m | vis2_instructions_m
  };

  static int  _features;
  static const char* _features_str;

  static void print_features();
  static int  determine_features();

public:
  // Initialization
  static void initialize();

  // Instruction support
  static bool has_v8()                  { return (_features & v8_instructions_m) != 0; }
  static bool has_v9()                  { return (_features & v9_instructions_m) != 0; }
  static bool has_hardware_int_muldiv() { return (_features & hardware_int_muldiv_m) != 0; }
  static bool has_hardware_fsmuld()     { return (_features & hardware_fsmuld_m) != 0; }
  static bool has_vis1()                { return (_features & vis1_instructions_m) != 0; }
  static bool has_vis2()                { return (_features & vis2_instructions_m) != 0; }

  static bool supports_compare_and_exchange() 
                                        { return has_v9(); }

  static bool is_ultra3()               { return (_features & ultra3_m) == ultra3_m; }

  static bool has_fast_fxtof()          { return has_v9() && !is_ultra3(); }

  static const char* cpu_features()           { return _features_str; }

  // Prefetch
  static intx prefetch_copy_interval_in_bytes() {
    intx interval = PrefetchCopyIntervalInBytes;
    return interval >= 0 ? interval : (has_v9() ? 512 : 0);
  }
  static intx prefetch_scan_interval_in_bytes() {
    intx interval = PrefetchScanIntervalInBytes;
    return interval >= 0 ? interval : (has_v9() ? 512 : 0);
  }
  static intx prefetch_fields_ahead() {
    intx count = PrefetchFieldsAhead;
    return count >= 0 ? count : (is_ultra3() ? 1 : 0);
  }

  // Legacy
  static bool v8_instructions_work() { return has_v8() && !has_v9(); }
  static bool v9_instructions_work() { return has_v9(); }

  // Assembler testing
  static void allow_all();
  static void revert();
};
