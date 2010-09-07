/*
 * Copyright (c) 1997, 2003, Oracle and/or its affiliates. All rights reserved.
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

// Check whether an element of a typeArrayOop with the given type must be
// aligned 0 mod 8.  The typeArrayOop itself must be aligned at least this
// strongly.

inline bool Universe::element_type_should_be_aligned(BasicType type) {
  return type == T_DOUBLE || type == T_LONG;
}

// Check whether an object field (static/non-static) of the given type must be aligned 0 mod 8.

inline bool Universe::field_type_should_be_aligned(BasicType type) {
  return type == T_DOUBLE || type == T_LONG;
}
