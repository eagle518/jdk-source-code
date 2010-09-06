#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_FrameMap_sparc.hpp	1.34 03/12/23 16:37:00 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 public:

  enum {
    nof_reg_args = 6   // registers o0-o5 are available for parameter passing
  };


  static RInfo _G0_RInfo;
  static RInfo _G1_RInfo;
  static RInfo _G2_RInfo;
  static RInfo _G3_RInfo;
  static RInfo _G4_RInfo;
  static RInfo _G5_RInfo;
  static RInfo _G6_RInfo;
  static RInfo _G7_RInfo;
  static RInfo _O0_RInfo;
  static RInfo _O1_RInfo;
  static RInfo _O2_RInfo;
  static RInfo _O3_RInfo;
  static RInfo _O4_RInfo;
  static RInfo _O5_RInfo;
  static RInfo _O6_RInfo;
  static RInfo _O7_RInfo;
  static RInfo _L0_RInfo;
  static RInfo _L1_RInfo;
  static RInfo _L2_RInfo;
  static RInfo _L3_RInfo;
  static RInfo _L4_RInfo;
  static RInfo _L5_RInfo;
  static RInfo _L6_RInfo;
  static RInfo _L7_RInfo;
  static RInfo _I0_RInfo;
  static RInfo _I1_RInfo;
  static RInfo _I2_RInfo;
  static RInfo _I3_RInfo;
  static RInfo _I4_RInfo;
  static RInfo _I5_RInfo;
  static RInfo _I6_RInfo;
  static RInfo _I7_RInfo;

  static RInfo _SP_RInfo;
  static RInfo _FP_RInfo;

  static RInfo _I0_I1_RInfo;
  static RInfo _O0_O1_RInfo;
  static RInfo _O2_O3_RInfo;
  static RInfo _F0_RInfo;
  static RInfo _F1_RInfo;
  static RInfo _F3_RInfo;
  static RInfo _F0_double_RInfo;
  static RInfo _F2_double_RInfo;
  
  static RInfo _Oexception_RInfo;
  static RInfo _Oissuing_pc_RInfo;
  static RInfo _GIC_RInfo;

 private:
  static FloatRegister  _fpu_regs [nof_fpu_regs];

  static c1_RegMask _callee_save_regs;
  static c1_RegMask _caller_save_regs;

  WordSize fp_offset_for_slot          (int slot) const;
  int      local_to_slot               (int local_name, bool is_two_word) const;
  WordSize fp_offset_for_monitor_lock  (int monitor_index) const;
  WordSize fp_offset_for_monitor_object(int monitor_index) const;
  Address  make_new_address_for_name   (WordSize fp_offset, bool for_hi_word) const;
  Address  make_new_address            (WordSize fp_offset) const;
  bool     location_for_fp_offset      (WordSize word_offset_from_fp,
                                        Location::Type loc_type,
                                        Location* loc) const;
  WordSize fp2sp_offset                (WordSize fp_offset) const;

 public:

  WordSize fp_offset_for_name          (int name, bool is_two_word, bool for_hi_word) const;

  static FloatRegister nr2floatreg (int rnr);

  static OptoReg::Name fpu_regname (int n);

  static c1_RegMask callee_save_regs()  { return _callee_save_regs; }
  static c1_RegMask caller_save_regs()  { return _caller_save_regs; }

  static bool is_caller_save_register (RInfo  reg);
  static bool is_caller_save_register (Register r);

