/*
 * Copyright (c) 1998, 2002, Oracle and/or its affiliates. All rights reserved.
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

  // helper function
  static void invokevfinal_helper(Register Rcache, Register Rret);
  static void invokeinterface_object_method(Register RklassOop, Register Rcall,
                                            Register Rret,
                                            Register Rflags);
  static void generate_vtable_call(Register Rrecv, Register Rindex, Register Rret);
  static void volatile_barrier(Assembler::Membar_mask_bits order_constraint);
