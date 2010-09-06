#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_RInfo_sparc.hpp	1.17 03/12/23 16:37:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 public:
  FloatRegister as_float_reg   () const;
  FloatRegister as_double_reg  () const;

  friend RInfo as_RInfo(const FloatRegister f, bool is_double) {
    RInfo rinfo;
    if (is_double) {
      rinfo.set_double_reg(f);
    } else {
      rinfo.set_float_reg(f);
    }
    return rinfo;
  }

  void set_float_reg  (const FloatRegister& f);
  void set_double_reg (const FloatRegister& d);
  void set_double_reg (int reg) {
    // because of endianness:
    //   reg1 should return as_double_reg()->successor()->encoding()
    //   reg2 should return as_double_reg()->encoding()
    _number = ((reg + 1) << reg1_shift) + (reg << reg2_shift) + double_reg_type;
    assert(reg1() == as_double_reg()->successor()->encoding(FloatRegisterImpl::S), "should match");
    assert(reg2() == as_double_reg()->encoding(FloatRegisterImpl::S), "should match");
  }

  void pd_print_fpu () const {
    if (is_float()) {
      tty->print(as_float_reg()->name());
    } else {
      assert(is_double(), "must be an fpu rinfo");
      tty->print(as_double_reg()->name());
    }
  }
