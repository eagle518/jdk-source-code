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

  protected:

  void generate_fixed_frame(bool native_call); // template interpreter only
  void generate_stack_overflow_check(Register Rframe_size, Register Rscratch,
                                     Register Rscratch2);
