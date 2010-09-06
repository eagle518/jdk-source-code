#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objArrayOop.hpp	1.23 03/12/23 16:42:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An objArrayOop is an array containing oops.
// Evaluating "String arg[10]" will create an objArrayOop.

class objArrayOopDesc : public arrayOopDesc {
 public:
  // Accessing
  oop obj_at(int index) const           { return *obj_at_addr(index);           }
  void obj_at_put(int index, oop value) { oop_store(obj_at_addr(index), value); }
  oop* base() const                     { return (oop*) arrayOopDesc::base(T_OBJECT); }

  // Sizing
  static int header_size()              { return arrayOopDesc::header_size(T_OBJECT); }
  static int object_size(int length)    { return align_object_size(header_size() + length); }  
  int object_size()                     { return object_size(length()); }

  // Returns the address of the index'th element
  oop* obj_at_addr(int index) const {
    assert(is_within_bounds(index), "index out of bounds");
    return &base()[index];
  }
};
