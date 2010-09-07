/*
 * Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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

// This file holds the platform specific parts of the StubRoutines
// definition. See stubRoutines.hpp for a description on how to
// extend it.

static bool    returns_to_call_stub(address return_pc)   { return return_pc == _call_stub_return_address; }

enum platform_dependent_constants
{
  code_size1 =  19000, // simply increase if too small (assembler will
                      // crash if too small)
  code_size2 = 22000  // simply increase if too small (assembler will
                      // crash if too small)
};

class x86 {
 friend class StubGenerator;

 private:
  static address _get_previous_fp_entry;
  static address _verify_mxcsr_entry;

  static address _f2i_fixup;
  static address _f2l_fixup;
  static address _d2i_fixup;
  static address _d2l_fixup;

  static address _float_sign_mask;
  static address _float_sign_flip;
  static address _double_sign_mask;
  static address _double_sign_flip;
  static address _mxcsr_std;

 public:

  static address get_previous_fp_entry()
  {
    return _get_previous_fp_entry;
  }

  static address verify_mxcsr_entry()
  {
    return _verify_mxcsr_entry;
  }

  static address f2i_fixup()
  {
    return _f2i_fixup;
  }

  static address f2l_fixup()
  {
    return _f2l_fixup;
  }

  static address d2i_fixup()
  {
    return _d2i_fixup;
  }

  static address d2l_fixup()
  {
    return _d2l_fixup;
  }

  static address float_sign_mask()
  {
    return _float_sign_mask;
  }

  static address float_sign_flip()
  {
    return _float_sign_flip;
  }

  static address double_sign_mask()
  {
    return _double_sign_mask;
  }

  static address double_sign_flip()
  {
    return _double_sign_flip;
  }

  static address mxcsr_std()
  {
    return _mxcsr_std;
  }
};
