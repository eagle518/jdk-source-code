#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_AllocTable_i486.hpp	1.9 03/12/23 16:36:03 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

public:
  enum {
    nofRegs = FrameMap::nof_cpu_regs
  };

  enum {
    nofRegsExp2   = 2 << nofRegs     // 2 ** nofRegs
  };

private:
  // the statics are initialized with init_free_reg_table
  static int _reg_mask [nofRegs];
  static int _free_reg [nofRegsExp2];

  void reg_check (int rnr) const      { assert(0 <= rnr && rnr < nofRegs, "wrong register number"); }

  bool pd_has_double_free () const    { return has_one_free(); }
  void pd_set_double_locked (int rnr) { set_locked(rnr); }
  int pd_get_double_free ()           { return get_free(); }

public:
  static void init_tables ();
