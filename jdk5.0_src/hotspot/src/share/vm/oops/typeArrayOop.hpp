#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)typeArrayOop.hpp	1.37 03/12/23 16:42:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A typeArrayOop is an array containing basic types (non oop elements).
// It is used for arrays of {characters, singles, doubles, bytes, shorts, integers, longs}
#include <limits.h>

class typeArrayOopDesc : public arrayOopDesc {
 protected:
  jchar*    char_base()   const { return (jchar*)   base(T_CHAR); }
  jboolean* bool_base()   const { return (jboolean*)base(T_BOOLEAN); }
  jbyte*    byte_base()   const { return (jbyte*)   base(T_BYTE); }
  jint*     int_base()    const { return (jint*)    base(T_INT); }
  jlong*    long_base()   const { return (jlong*)   base(T_LONG); }
  jshort*   short_base()  const { return (jshort*)  base(T_SHORT); }
  jfloat*   float_base()  const { return (jfloat*)  base(T_FLOAT); }
  jdouble*  double_base() const { return (jdouble*) base(T_DOUBLE); }

  friend class typeArrayKlass;

 public:
  jbyte* byte_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &byte_base()[which];
  }

  jboolean* bool_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &bool_base()[which];
  }

  jchar* char_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &char_base()[which];
  }

  jint* int_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &int_base()[which];
  }

  jshort* short_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &short_base()[which];
  }

  jushort* ushort_at_addr(int which) const {  // for field descriptor arrays
    assert(is_within_bounds(which), "index out of bounds");
    return (jushort*) &short_base()[which];
  }

  jlong* long_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &long_base()[which];
  }

  jfloat* float_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &float_base()[which];
  }

  jdouble* double_at_addr(int which) const {
    assert(is_within_bounds(which), "index out of bounds");
    return &double_base()[which];
  }  

  jbyte byte_at(int which) const                  { return *byte_at_addr(which); }
  void byte_at_put(int which, jbyte contents)     { *byte_at_addr(which) = contents; }

  jboolean bool_at(int which) const               { return *bool_at_addr(which); }
  void bool_at_put(int which, jboolean contents)  { *bool_at_addr(which) = contents; }

  jchar char_at(int which) const                  { return *char_at_addr(which); }
  void char_at_put(int which, jchar contents)     { *char_at_addr(which) = contents; }

  jint int_at(int which) const                    { return *int_at_addr(which); }
  void int_at_put(int which, jint contents)       { *int_at_addr(which) = contents; }

  jshort short_at(int which) const                { return *short_at_addr(which); }
  void short_at_put(int which, jshort contents)   { *short_at_addr(which) = contents; }

  jushort ushort_at(int which) const              { return *ushort_at_addr(which); }
  void ushort_at_put(int which, jushort contents) { *ushort_at_addr(which) = contents; }

  jlong long_at(int which) const                  { return *long_at_addr(which); }
  void long_at_put(int which, jlong contents)     { *long_at_addr(which) = contents; }

  jfloat float_at(int which) const                { return *float_at_addr(which); }
  void float_at_put(int which, jfloat contents)   { *float_at_addr(which) = contents; }

  jdouble double_at(int which) const              { return *double_at_addr(which); }
  void double_at_put(int which, jdouble contents) { *double_at_addr(which) = contents; }

  jbyte byte_at_acquire(int which) const              { return OrderAccess::load_acquire(byte_at_addr(which)); }
  void release_byte_at_put(int which, jbyte contents) { OrderAccess::release_store(byte_at_addr(which), contents); }

  // Sizing

  // Returns the number of words necessary to hold an array of "len"
  // elements each of the given "byte_size".
  static int word_array_size(int byte_size, int len) {
    return (byte_size * len + (HeapWordSize-1)) >> LogHeapWordSize;
  }

  static int object_size(int scale, int length, int instance_header_size) {
    return align_object_size(instance_header_size + word_array_size(scale, length));
  }     

  int object_size() {
    typeArrayKlass* tk = typeArrayKlass::cast(klass());
    return object_size(tk->scale(), length(), tk->array_header_in_bytes() / wordSize);
  }
};
