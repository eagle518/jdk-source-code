/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
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

  static void prepare_invoke(Register method, Register index, int byte_no);
  static void invokevirtual_helper(Register index, Register recv,
                                   Register flags);
  static void volatile_barrier(Assembler::Membar_mask_bits order_constraint);

  // Helpers
  static void index_check(Register array, Register index);
  static void index_check_without_pop(Register array, Register index);
