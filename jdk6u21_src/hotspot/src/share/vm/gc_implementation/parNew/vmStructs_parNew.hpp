/*
 * Copyright (c) 2006, 2007, Oracle and/or its affiliates. All rights reserved.
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

#define VM_TYPES_PARNEW(declare_type)                                     \
           declare_type(ParNewGeneration,             DefNewGeneration)

#define VM_INT_CONSTANTS_PARNEW(declare_constant)                         \
  declare_constant(Generation::ParNew)
