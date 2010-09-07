/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
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

// This file contains the platform-independent parts
// of the template interpreter generator.

#ifdef CC_INTERP

class CppInterpreterGenerator: public AbstractInterpreterGenerator {
  protected:
  // shared code sequences
  // Converter for native abi result to tosca result
  address generate_result_handler_for(BasicType type);
  address generate_tosca_to_stack_converter(BasicType type);
  address generate_stack_to_stack_converter(BasicType type);
  address generate_stack_to_native_abi_converter(BasicType type);

  void generate_all();

 public:
  CppInterpreterGenerator(StubQueue* _code);

   #include "incls/_cppInterpreterGenerator_pd.hpp.incl"
};

#endif // CC_INTERP
