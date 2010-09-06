#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oop.inline2.hpp	1.5 03/12/23 16:42:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Implementation of all inlined member functions defined in oop.hpp
// We need a separate file to avoid circular references

// Separate this out to break dependency.
inline bool oopDesc::is_perm() const {
  return Universe::heap()->is_in_permanent(this);
}


