#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cpCacheOop.hpp	1.55 04/05/25 15:29:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A ConstantPoolCacheEntry describes an individual entry of the constant
// pool cache. There's 2 principal kinds of entries: field entries for in-
// stance & static field access, and method entries for invokes. Some of
// the entry layout is shared and looks as follows:
//
// bit number |31                0|
// bit length |-8--|-8--|---16----|
// --------------------------------
// _indices   [ b2 | b1 |  index  ]
// _f1        [  entry specific   ]
// _f2        [  entry specific   ]
// _flags     [t|O|f|vf|v|unused|hindex ] (for field entries)
// bit length |4|1|1| 1|1|---8--|--16---]
//#ifdef HOTSWAP
// _flags     [t|f|vf|v|m|h|unused|eidx|psze] (for method entries)
// bit length |4|1|1 |1|1|1|---7--|-8--|-8--]
//#else
// _flags     [t|f|vf|v|m|unused|eidx|psze] (for method entries)
// bit length |4|1| 1|1|1|--8---|-8--|-8--]
//#endif HOTSWAP

// --------------------------------
//
// with:
// index  = original constant pool index
// b1     = bytecode 1
// b2     = bytecode 2
// psze   = parameters size (method entries only)
// eidx   = interpreter entry index (method entries only)
// hindex = original field index in method holder
// t      = type (see below)
// vf     = virtual, final (method entries only : is_vfinal())
//
// The flags have the following interpretation:
// bit 27: f flag  true if field is marked final
// bit 26: vf flag true if virtual final method
// bit 25: v flag true if field is volatile (only for fields)
// bit 24: m flag true if invokeinterface used for method in class Object
//#ifdef HOTSWAP
// bit 23: 0 for fields, 1 for methods
//#endif HOTSWAP
//
// The flags 31, 30, 29, 28 together build a 4 bit number 0 to 8 with the
// following mapping to the TosState states:
//
// btos: 0
// ctos: 1
// stos: 2
// itos: 3
// ltos: 4
// ftos: 5
// dtos: 6
// atos: 7
// vtos: 8
//
// Entry specific: field entries:
// _indices = get (b1 section) and put (b2 section) bytecodes, original constant pool index
// _f1      = field holder
// _f2      = field offset in words
// _flags   = field type information, original field index in field holder (hindex section)
//
// Entry specific: method entries:
// _indices = invoke code for f1 (b1 section), invoke code for f2 (b2 section),
//            original constant pool index
// _f1      = method for all but virtual calls, unused by virtual calls
#ifdef HOTSWAP
//            (note: for interface calls, which are essentially virtual,
//             contains klassOop for the corresponding interface. This is an
//             original HotSpot feature, #ifdef HOTSWAP is here just
//             for code authorship consistency, and should be deleted eventually)
#endif HOTSWAP
// _f2      = method/vtable index for virtual calls only, unused by all other
//            calls.  The vf flag indicates this is a method pointer not an
//            index.
// _flags   = field type info (f section),
//            virtual final entry (vf),
//            interpreter entry index (eidx section),
//            parameter size (psze section)
//
// Note: invokevirtual & invokespecial bytecodes can share the same constant
//       pool entry and thus the same constant pool cache entry. All invoke
//       bytecodes but invokevirtual use only _f1 and the corresponding b1
//       bytecode, while invokevirtual uses only _f2 and the corresponding
//       b2 bytecode.  The value of _flags is shared for both types of entries.
//
// The fields are volatile so that they are stored in the order written in the
// source code.  The _indices field with the bytecode must be written last.

class ConstantPoolCacheEntry VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  volatile intx  _indices;  // constant pool index & rewrite bytecodes
  volatile oop   _f1;       // entry specific oop field
  volatile intx  _f2;       // entry specific int/oop field
  volatile intx  _flags;    // flags


#ifdef ASSERT
  bool same_methodOop(oop cur_f1, oop f1);
#endif

  void set_bytecode_1(Bytecodes::Code code);
  void set_bytecode_2(Bytecodes::Code code);
  void set_f1(oop f1)                            {
    assert(_f1 == NULL || _f1 == f1, "illegal field change");
    oop_store(&_f1, f1);
  }
  void set_f2(intx f2)                           { assert(_f2 == 0    || _f2 == f2, "illegal field change"); _f2 = f2; }
  int as_flags(TosState state, bool is_final, bool is_vfinal, bool is_volatile,
               bool is_method_interface, bool is_method);
  void set_flags(intx flags)                     { _flags = flags; }

 public:
  // specific bit values in flag field
  // Note: the interpreter knows this layout!
  enum FlagBitValues {
    hotSwapBit    = 23,
    methodInterface = 24,
    volatileField = 25,
    vfinalMethod  = 26,
    finalField    = 27
  }; 

  // start of type bits in flags
  // Note: the interpreter knows this layout!
  enum FlagValues {
    tosBits      = 28 
  };

  // Initialization
  void set_initial_state(int index);             // sets entry to initial state

  void set_field(                                // sets entry to resolved field state
    Bytecodes::Code get_code,                    // the bytecode used for reading the field
    Bytecodes::Code put_code,                    // the bytecode used for writing the field
    KlassHandle     field_holder,                // the object/klass holding the field
    int             field_index,                 // the original field index in the field holder
    int             field_offset,                // the field offset in words in the field holder
    TosState        field_type,                  // the (machine) field type    
    bool            is_final,                     // the field is final 
    bool            is_volatile                  // the field is volatile 
  );

  void set_method(                               // sets entry to resolved method entry
    Bytecodes::Code invoke_code,                 // the bytecode used for invoking the method
    methodHandle    method,                      // the method/prototype if any (NULL, otherwise)
    int             vtable_index                 // the vtable index if any (-1, otherwise)
  );

  void set_interface_call(
    methodHandle method,                         // Resolved method    
    int index                                    // Method index into interface
  );               

  void set_parameter_size(int value) {
    assert(parameter_size() == 0 || parameter_size() == value, 
           "size must not change");
    // We set the "HOTSWAP - method or field bit" in the following 
    // statement, since this call is used by HotSwap functionality in certain
    // cases to "partially initialize" a cpool cache entry (see the 
    // patch_param_size_in_new_cpcache() function in jvmdi_hotswap.cpp). 
    // Setting the method bit here prevents hitting an "is_method_entry"
    // assertion in that function if the same class is immediately redefined
    // for the second time.
    //
    // Setting the parameter size by itself is only safe if the
    // current value of _flags is 0, otherwise another thread may have
    // updated it and we don't want to overwrite that value.  Don't
    // bother trying to update it once it's nonzero but always make
    // sure that the final parameter size agrees with what was passed.
    if (_flags == 0) {
      Atomic::cmpxchg_ptr((value & 0xFF) | (1 << hotSwapBit), &_flags, 0);
    }
    guarantee(parameter_size() == value, "size must not change");
  }

  // Which bytecode number (1 or 2) in the index field is valid for this bytecode?
  // Returns -1 if neither is valid.
  static int bytecode_number(Bytecodes::Code code) {
    switch (code) {
      case Bytecodes::_getstatic       :    // fall through
      case Bytecodes::_getfield        :    // fall through
      case Bytecodes::_invokespecial   :    // fall through
      case Bytecodes::_invokestatic    :    // fall through
      case Bytecodes::_invokeinterface : return 1;
      case Bytecodes::_putstatic       :    // fall through
      case Bytecodes::_putfield        :    // fall through
      case Bytecodes::_invokevirtual   : return 2;
      default                          : break;
    }
    return -1;
  }

  // Has this bytecode been resolved? Only valid for invokes and get/put field/static.
  bool is_resolved(Bytecodes::Code code) const {
    switch (bytecode_number(code)) {
      case 1:  return (bytecode_1() == code);
      case 2:  return (bytecode_2() == code);
    }
    return false;      // default: not resolved
  }

  // Accessors
  int constant_pool_index() const                { return _indices & 0xFFFF; }
  Bytecodes::Code bytecode_1() const             { return Bytecodes::cast((_indices >> 16) & 0xFF); }
  Bytecodes::Code bytecode_2() const             { return Bytecodes::cast((_indices >> 24) & 0xFF); }
  oop  f1() const                                { return _f1; }
  intx f2() const                                { return _f2; }
  int  holder_index() const                      { return _flags & 0xFFFF; }
  int  parameter_size() const                    { return _flags & 0xFF; }
  bool is_vfinal() const                         { return ((_flags & (1 << vfinalMethod)) == (1 << vfinalMethod)); }
  bool is_volatile() const                       { return ((_flags & (1 << volatileField)) == (1 << volatileField)); }
  bool is_methodInterface() const                { return ((_flags & (1 << methodInterface)) == (1 << methodInterface)); }
  bool is_byte() const                           { return (((uintx) _flags >> tosBits) == btos); }
  bool is_char() const                           { return (((uintx) _flags >> tosBits) == ctos); }
  bool is_short() const                          { return (((uintx) _flags >> tosBits) == stos); }
  bool is_int() const                            { return (((uintx) _flags >> tosBits) == itos); }
  bool is_long() const                           { return (((uintx) _flags >> tosBits) == ltos); }
  bool is_float() const                          { return (((uintx) _flags >> tosBits) == ftos); }
  bool is_double() const                         { return (((uintx) _flags >> tosBits) == dtos); }
  bool is_object() const                         { return (((uintx) _flags >> tosBits) == atos); }
  TosState flag_state() const                    { assert( ( (_flags >> tosBits) & 0x0F ) < number_of_states, "Invalid state in as_flags");
                                                   return (TosState)((_flags >> tosBits) & 0x0F); }
#ifdef HOTSWAP
  bool is_field_entry() const                    { return ! (_flags & (1 << hotSwapBit)); }
  bool is_method_entry() const                   { return _flags & (1 << hotSwapBit); }
#endif HOTSWAP

  // Code generation support
  static WordSize size()                         { return in_WordSize(sizeof(ConstantPoolCacheEntry) / HeapWordSize); }
  static ByteSize indices_offset()               { return byte_offset_of(ConstantPoolCacheEntry, _indices); }
  static ByteSize f1_offset()                    { return byte_offset_of(ConstantPoolCacheEntry, _f1); }
  static ByteSize f2_offset()                    { return byte_offset_of(ConstantPoolCacheEntry, _f2); }
  static ByteSize flags_offset()                 { return byte_offset_of(ConstantPoolCacheEntry, _flags); }

  // GC Support
  void oops_do(void f(oop*));
  void oop_iterate(OopClosure* blk);
  void oop_iterate_m(OopClosure* blk, MemRegion mr);
  void follow_contents();
  void adjust_pointers();

#ifdef HOTSWAP
  // Evolution support
  void adjust_method_entry(methodOop old_method, // If this entry points to the old_method, 
	                   methodOop new_method);// replace it the corresponding new_method
#endif HOTSWAP


  // Debugging & Printing
  void print (outputStream* st, int index) const;
  void verify(outputStream* st) const;

  static void verify_tosBits() {
    assert(tosBits == 28, "interpreter now assumes tosBits is 28");
  }
};


// A constant pool cache is a runtime data structure set aside to a constant pool. The cache
// holds interpreter runtime information for all field access and invoke bytecodes. The cache
// is created and initialized before a class is actively used (i.e., initialized), the indivi-
// dual cache entries are filled at resolution (i.e., "link") time (see also: rewriter.*).

class constantPoolCacheOopDesc: public arrayOopDesc {
  friend class VMStructs;
 private:
  constantPoolOop _constant_pool;                // the corresponding constant pool

  // Sizing
  static int header_size()                       { return sizeof(constantPoolCacheOopDesc) / HeapWordSize; }
  static int object_size(int length)             { return align_object_size(header_size() + length * in_words(ConstantPoolCacheEntry::size())); }
  int object_size()                              { return object_size(length()); }

  // Helpers
  constantPoolOop*        constant_pool_addr()   { return &_constant_pool; }
  ConstantPoolCacheEntry* base() const           { return (ConstantPoolCacheEntry*)((address)this + in_bytes(base_offset())); }

  friend class constantPoolCacheKlass;

 public:
  // Initialization
  void initialize(intArray& inverse_index_map);

  // Accessors
  void set_constant_pool(constantPoolOop pool)   { oop_store_without_check((oop*)&_constant_pool, (oop)pool); }
  constantPoolOop constant_pool() const          { return _constant_pool; }
  ConstantPoolCacheEntry* entry_at(int i) const  { assert(0 <= i && i < length(), "index out of bounds"); return base() + i; }

  // Code generation
  static ByteSize base_offset()                  { return in_ByteSize(sizeof(constantPoolCacheOopDesc)); }

#ifdef HOTSWAP
  // Evolution support
  void adjust_method_entries(objArrayOop old_methods,  // If any entry points to any of old_methods, 
	                     objArrayOop new_methods); // replace it with the corresponding new_method
#endif HOTSWAP
};

