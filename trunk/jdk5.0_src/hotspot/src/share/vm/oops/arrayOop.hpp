#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)arrayOop.hpp	1.26 03/12/23 16:41:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// arrayOopDesc is the abstract baseclass for all arrays.

class arrayOopDesc : public oopDesc {
  friend class VMStructs;
 private:
  int _length; // number of elements in the array

 public:
  // Interpreter/Compiler offsets
  static int length_offset_in_bytes()             { return (intptr_t)&(arrayOop(NULL)->_length); }
  static int base_offset_in_bytes(BasicType type) { return header_size(type) * HeapWordSize; }

  // Returns the address of the first element.
  void* base(BasicType type) const              { return (void*) (((intptr_t) this) + base_offset_in_bytes(type)); }

  // Tells whether index is within bounds.
  bool is_within_bounds(int index) const	{ return 0 <= index && index < length(); }

  // Accessores for instance variable
  int length() const				{ return _length;   }
  void set_length(int length)			{ _length = length; }

  // Header size computation. 
  // Should only be called with constants as argument (will not constant fold otherwise)
  static int header_size(BasicType type) {
    return Universe::element_type_should_be_aligned(type) 
#ifdef _LP64
      ? align_object_size(sizeof(arrayOopDesc)/HeapWordSize) 
#else
      ? align_size_up(sizeof(arrayOopDesc)/HeapWordSize, 2) 
#endif
      : sizeof(arrayOopDesc)/HeapWordSize; 
  }

  // This method returns the  maximum length that can passed into object_size(scale, length, type) without
  // causing an overflow. We substract an extra 2*wordSize to guard against double word alignments.
  // It gets the scale from the type2aelembytes array.
  static int max_array_length(BasicType type) { 
    assert(type >= 0 && type < T_CONFLICT, "wrong type");
    assert(type2aelembytes[type] != 0, "wrong type");
// [RGV]  Should change Array's so they can be larger than 32 bits can express
// [RGV] Changed BitsPerWord to BitsPerInt below!
    return (((unsigned)1 << (BitsPerInt-1)) - HeapWordSize * header_size(type) - 2*HeapWordSize)
           / type2aelembytes[type];
  }

};
