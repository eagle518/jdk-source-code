/*
 * Copyright (c) 2000, 2006, Oracle and/or its affiliates. All rights reserved.
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

// Implementation of all inlined member functions defined in oop.hpp
// We need a separate file to avoid circular references

// Separate this out to break dependency.
inline bool oopDesc::is_perm() const {
  return Universe::heap()->is_in_permanent(this);
}

// Check for NULL also.
inline bool oopDesc::is_perm_or_null() const {
  return this == NULL || is_perm();
}

inline bool oopDesc::is_scavengable() const {
  return Universe::heap()->is_scavengable(this);
}
