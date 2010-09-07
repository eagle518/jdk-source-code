/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009 Red Hat, Inc.
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

 public:
  static address call_stub_return_pc() {
    return (address) -1;
  }

  static bool returns_to_call_stub(address return_pc) {
    return return_pc == call_stub_return_pc();
  }

  enum platform_dependent_constants {
    code_size1 = 0,      // The assembler will fail with a guarantee
    code_size2 = 0       // if these are too small.  Simply increase
  };                     // them if that happens.

#ifdef IA32
  class x86 {
    friend class VMStructs;

   private:
    static address _call_stub_compiled_return;
  };
#endif // IA32
