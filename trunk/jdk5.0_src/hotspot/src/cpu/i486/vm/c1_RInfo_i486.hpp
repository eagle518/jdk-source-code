#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_RInfo_i486.hpp	1.12 03/12/23 16:36:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public:
  int fpu () const { return is_float() ? float_reg() : double_reg(); }

  void set_double_reg (int reg)            { _number = (reg << reg1_shift) + (reg << reg2_shift) + double_reg_type; }

  void pd_print_fpu () const {
    tty->print(" fpu=%d", fpu(), " ");
  }
