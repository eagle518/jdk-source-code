/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
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

enum platform_dependent_constants {
  code_size1 =  9000,           // simply increase if too small (assembler will crash if too small)
  code_size2 = 22000            // simply increase if too small (assembler will crash if too small)
};

class x86 {
 friend class StubGenerator;
 friend class VMStructs;

 private:
  // If we call compiled code directly from the call stub we will
  // need to adjust the return back to the call stub to a specialized
  // piece of code that can handle compiled results and cleaning the fpu
  // stack. The variable holds that location.
  static address _call_stub_compiled_return;
  static address _verify_mxcsr_entry;
  static address _verify_fpu_cntrl_wrd_entry;
  static jint    _mxcsr_std;

 public:
  static address verify_mxcsr_entry()                        { return _verify_mxcsr_entry; }
  static address verify_fpu_cntrl_wrd_entry()                { return _verify_fpu_cntrl_wrd_entry; }

  static address get_call_stub_compiled_return()             { return _call_stub_compiled_return; }
  static void set_call_stub_compiled_return(address ret)     { _call_stub_compiled_return = ret; }
};

  static bool    returns_to_call_stub(address return_pc)     { return (return_pc == _call_stub_return_address) ||
                                                                       return_pc == x86::get_call_stub_compiled_return(); }
