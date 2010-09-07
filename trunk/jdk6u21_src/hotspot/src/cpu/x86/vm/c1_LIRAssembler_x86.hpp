/*
 * Copyright (c) 2000, 2008, Oracle and/or its affiliates. All rights reserved.
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

 private:

  Address::ScaleFactor array_element_size(BasicType type) const;

  void monitorexit(LIR_Opr obj_opr, LIR_Opr lock_opr, Register new_hdr, int monitor_no, Register exception);

  void arith_fpu_implementation(LIR_Code code, int left_index, int right_index, int dest_index, bool pop_fpu_stack);

  // helper functions which checks for overflow and sets bailout if it
  // occurs.  Always returns a valid embeddable pointer but in the
  // bailout case the pointer won't be to unique storage.
  address float_constant(float f);
  address double_constant(double d);

  bool is_literal_address(LIR_Address* addr);

  // When we need to use something other than rscratch1 use this
  // method.
  Address as_Address(LIR_Address* addr, Register tmp);


public:

  void store_parameter(Register r, int offset_from_esp_in_words);
  void store_parameter(jint c,     int offset_from_esp_in_words);
  void store_parameter(jobject c,  int offset_from_esp_in_words);

  enum { call_stub_size = NOT_LP64(15) LP64_ONLY(28),
         exception_handler_size = DEBUG_ONLY(1*K) NOT_DEBUG(175),
         deopt_handler_size = NOT_LP64(10) LP64_ONLY(17)
       };
