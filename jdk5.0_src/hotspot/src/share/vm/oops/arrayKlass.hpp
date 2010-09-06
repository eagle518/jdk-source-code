#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)arrayKlass.hpp	1.56 03/12/23 16:41:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// arrayKlass is the abstract baseclass for all array classes

class arrayKlass: public Klass {
  friend class VMStructs;
 private:
  int      _dimension;         // This is n'th-dimensional array.
  klassOop _higher_dimension;  // Refers the (n+1)'th-dimensional array (if present).
  klassOop _lower_dimension;   // Refers the (n-1)'th-dimensional array (if present).
  int      _vtable_len;        // size of vtable for this klass  
  juint    _alloc_size;        // allocation profiling support
  int      _array_header_in_bytes; // offset of first element, including any
				// padding for the sake of alignment

 public:
  // Testing operation
  bool oop_is_array() const { return true; }

  // Instance variables
  int dimension() const                 { return _dimension;      }
  void set_dimension(int dimension)     { _dimension = dimension; }

  klassOop higher_dimension() const     { return _higher_dimension; }
  void set_higher_dimension(klassOop k) { oop_store_without_check((oop*) &_higher_dimension, (oop) k); }
  oop* adr_higher_dimension()           { return (oop*)&this->_higher_dimension;}

  klassOop lower_dimension() const      { return _lower_dimension; }
  void set_lower_dimension(klassOop k)  { oop_store_without_check((oop*) &_lower_dimension, (oop) k); }
  oop* adr_lower_dimension()            { return (oop*)&this->_lower_dimension;}

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

  int  array_header_in_bytes() const    { return _array_header_in_bytes; }
  void set_array_header_in_bytes(int s) { _array_header_in_bytes = s;    }

  // Compiler/Interpreter offset
  static ByteSize array_header_in_bytes_offset() { return byte_offset_of(arrayKlass, _array_header_in_bytes); }

  virtual klassOop java_super() const;//{ return SystemDictionary::object_klass(); }

  // Allocation
  // Sizes points to the first dimension of the array, subsequent dimensions
  // are in higher memory if next_dim_step == 1, or lower memory if next_dim_step == -1
  virtual oop multi_allocate(int rank, jint* sizes, int next_dim_step, TRAPS);
  objArrayOop allocate_arrayArray(int n, int length, TRAPS);

  // Lookup operations
  methodOop uncached_lookup_method(symbolOop name, symbolOop signature) const;

  // Casting from klassOop
  static arrayKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_array(), "cast to arrayKlass");
    return (arrayKlass*) k->klass_part(); 
  }

  objArrayOop compute_secondary_supers(int num_extra_slots, TRAPS);
  bool compute_is_subtype_of(klassOop k);

  // Sizing
  static int header_size()                 { return oopDesc::header_size() + sizeof(arrayKlass)/HeapWordSize; }
  int object_size(int header_size) const;

  bool object_is_parsable() const          { return _vtable_len > 0; }

  // Java vtable
  klassVtable* vtable() const;             // return new klassVtable
  int  vtable_length() const               { return _vtable_len; }
  static int base_vtable_length()          { return Universe::base_vtable_size(); }
  void set_vtable_length(int len)          { assert(len == base_vtable_length(), "bad length"); _vtable_len = len; }
 protected:
  inline intptr_t* start_of_vtable() const;

 public:
  // Iterators
  void array_klasses_do(void f(klassOop k));
  void with_array_klasses_do(void f(klassOop k));

  // Shared creation method
  static arrayKlassHandle base_create_array_klass(
                                          const Klass_vtbl& vtbl,
                                          int header_size, KlassHandle klass,
                                          TRAPS);
  // Return a handle.
  static void     complete_create_array_klass(arrayKlassHandle k, KlassHandle super_klass, TRAPS);

 public:
   // jvm support
   jint compute_modifier_flags(TRAPS) const;

 public:
   // JVMTI support
   jint jvmti_class_status() const;

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on(oop obj, outputStream* st);
  void oop_verify_on(oop obj, outputStream* st);
#endif
};

