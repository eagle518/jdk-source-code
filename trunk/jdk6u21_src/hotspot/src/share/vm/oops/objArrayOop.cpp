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

# include "incls/_precompiled.incl"
# include "incls/_objArrayOop.cpp.incl"

#define ObjArrayOop_OOP_ITERATE_DEFN(OopClosureType, nv_suffix)                    \
                                                                                   \
int objArrayOopDesc::oop_iterate_range(OopClosureType* blk, int start, int end) {  \
  SpecializationStats::record_call();                                              \
  return ((objArrayKlass*)blueprint())->oop_oop_iterate_range##nv_suffix(this, blk, start, end); \
}

ALL_OOP_OOP_ITERATE_CLOSURES_1(ObjArrayOop_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_2(ObjArrayOop_OOP_ITERATE_DEFN)
