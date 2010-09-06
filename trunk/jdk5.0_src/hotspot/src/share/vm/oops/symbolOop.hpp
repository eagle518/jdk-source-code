#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)symbolOop.hpp	1.32 04/03/01 17:22:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A symbolOop is a canonicalized string.
// All symbolOops reside in global symbolTable.
// See oopFactory::new_symbol for how to allocate a symbolOop

class symbolOopDesc : public oopDesc {
  friend class VMStructs;
 private:
  unsigned short _length; // number of UTF8 characters in the symbol
  jbyte _body[1];  

  enum {
    // max_symbol_length is constrained by type of _length 
    max_symbol_length = (1 << 16) -1
  };
 public:

  // Low-level access (used with care, since not GC-safe)
  jbyte* base() { return &_body[0]; }


  // Returns the largest size symbol we can safely hold. 
  static int max_length() {
    return max_symbol_length;
  }

  static int object_size(int length) {
    int size = header_size() + (sizeof(unsigned short) + length + HeapWordSize - 1) / HeapWordSize;
    return align_object_size(size);
  }

  int object_size() { return object_size(utf8_length()); }

  int byte_at(int index) const {
    assert(index >=0 && index < _length, "symbol index overflow");
    return ((symbolOopDesc*)this)->base()[index];
  }

  void byte_at_put(int index, int value) {
    assert(index >=0 && index < _length, "symbol index overflow");
    ((symbolOopDesc*)this)->base()[index] = value;
  }

  jbyte* bytes() { return base(); }

  int utf8_length() const { return _length; }

  void set_utf8_length(int len) { _length = len; }

  // Compares the symbol with a string
  bool equals(const char* str, int len) const;

  // Three-way compare for sorting; returns -1/0/1 if receiver is </==/> than arg
  // note that the ordering is not alfabetical
  inline int fast_compare(symbolOop other) const;

  // Returns receiver converted to null-terminated UTF-8 string; string is
  // allocated in resource area, or in the char buffer provided by caller.
  char* as_C_string() const;
  char* as_C_string(char* buf, int size) const;


  // Returns a null terminated utf8 string in a resource array
  char* as_utf8() const { return as_C_string(); }

  jchar* as_unicode(int& length) const;  

  // Treating this symbol as a class name, returns the Java name for the class.
  // String is allocated in resource area if buffer is not provided.
  // See Klass::external_name()
  const char* as_klass_external_name() const;
  const char* as_klass_external_name(char* buf, int size) const;

  bool object_is_parsable() const {
    return (utf8_length() > 0 || (oop)this == Universe::emptySymbol());
  }

  // Printing
  void print_symbol_on(outputStream* st = NULL);
};


// Note: this comparison is used for vtable sorting only; it doesn't matter
// what order it defines, as long as it is a total, time-invariant order
// Since symbolOops are in permSpace, their relative order in memory never changes,
// so use address comparison for speed
int symbolOopDesc::fast_compare(symbolOop other) const {
  return (int)(intptr_t(this) - intptr_t(other));
}

