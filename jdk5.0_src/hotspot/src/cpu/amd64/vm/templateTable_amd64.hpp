#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)templateTable_amd64.hpp	1.2 03/12/23 16:35:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  static void prepare_invoke(Register method, Register index, int byte_no,
                             Bytecodes::Code code);
  static void invokevirtual_helper(Register index, Register recv,
                                   Register flags);
  static void volatile_barrier(Assembler::Membar_mask_bits order_constraint);
