#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)constMethodOop.hpp	1.8 04/04/27 10:57:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An constMethodOop represents portions of a Java method which
// do not vary. 
//
// Memory layout (each line represents a word). Note that most
// applications load thousands of methods, so keeping the size of this
// structure small has a big impact on footprint.
//
// |------------------------------------------------------|
// | header                                               |
// | klass                                                |
// |------------------------------------------------------|
// | fingerprint 1                                        |
// | fingerprint 2                                        |
// | exception_table                (oop)                 |
// | constMethod_size                                     |
// | interp_kind  | flags    | code_size                  |
// | name index              | signature index            |
// | method_index            | generic_signature_index    |
// | stackmap_u1             | stackmap_u2                |
// |------------------------------------------------------|
// |                                                      |
// | byte codes                                           |
// |                                                      |
// |------------------------------------------------------|
// | compressed linenumber table                          |
// |  (see class CompressedLineNumberReadStream)          |
// |  (note that length is unknown until decompressed)    |
// |  (access flags bit tells whether table is present)   |
// |  (indexed from start of constMethodOop)              |
// |  (elements not necessarily sorted!)                  |
// |------------------------------------------------------|
// | localvariable table elements + length (length last)  |
// |  (length is u2, elements are 6-tuples of u2)         |
// |  (see class LocalVariableTableElement)               |
// |  (access flags bit tells whether table is present)   |
// |  (indexed from end of contMethodOop)                 |
// |------------------------------------------------------|
// | checked exceptions elements + length (length last)   |
// |  (length is u2, elements are u2)                     |
// |  (see class CheckedExceptionElement)                 |
// |  (access flags bit tells whether table is present)   |
// |  (indexed from end of constMethodOop)                |
// |------------------------------------------------------|


// Utitily class decribing elements in checked exceptions table inlined in methodOop.
class CheckedExceptionElement VALUE_OBJ_CLASS_SPEC {
 public:
  u2 class_cp_index;
};


// Utitily class decribing elements in local variable table inlined in methodOop.
class LocalVariableTableElement VALUE_OBJ_CLASS_SPEC {
 public:
  u2 start_bci;
  u2 length;
  u2 name_cp_index;
  u2 descriptor_cp_index;
  u2 signature_cp_index;
  u2 slot;
};


class constMethodOopDesc : public oopDesc {
  friend class constMethodKlass;
  friend class VMStructs;
private:
  enum {
    _has_linenumber_table = 1,
    _has_checked_exceptions = 2,
    _has_localvariable_table = 4
  };

  // Bit vector of signature
  // Callers interpret 0=not initialized yet and
  // -1=too many args to fix, must parse the slow way.
  // The real initial value is special to account for nonatomicity of 64 bit
  // loads and stores.  This value may updated and read without a lock by
  // multiple threads, so is volatile.
  volatile uint64_t _fingerprint;

  // Backpointer to non-const methodOop (needed for some JVMTI operations)
  methodOop         _method;

  // The exception handler table. 4-tuples of ints [start_pc, end_pc,
  // handler_pc, catch_type index] For methods with no exceptions the
  // table is pointing to Universe::the_empty_int_array
  typeArrayOop      _exception_table;            

  int               _constMethod_size;
  jbyte             _interpreter_kind;
  jbyte             _flags;

  // Size of Java bytecodes allocated immediately after methodOop.
  u2                _code_size;
  u2                _name_index;                 // Method name (index in constant pool)
  u2                _signature_index;            // Method signature (index in constant pool)
  u2                _method_index;               // index into the methods()
                                                 // array of instanceKlass
                                                 // (for JNI ids)
  u2                _generic_signature_index;    // Generic signature (index in constant pool, 0 if absent)

  typeArrayOop      _stackmap_u1;                // The u1 elements in stack map table.
                                                 // For methods with no stackmaps the table is pointing to Universe::the_empty_byte_array
  typeArrayOop      _stackmap_u2;                // The u2 elements in stack map table.
                                                 // For methods with no stackmaps the table is pointing to Universe::the_empty_short_array

public:
  // Inlined tables
  void set_inlined_tables_length(int checked_exceptions_len,
                                 int compressed_line_number_size,
                                 int localvariable_table_len);

  bool has_linenumber_table() const
    { return (_flags & _has_linenumber_table) != 0; }

  bool has_checked_exceptions() const
    { return (_flags & _has_checked_exceptions) != 0; }

  bool has_localvariable_table() const
    { return (_flags & _has_localvariable_table) != 0; }

  void set_interpreter_kind(int kind)      { _interpreter_kind = kind; }
  int  interpreter_kind(void) const        { return _interpreter_kind; }

  // backpointer to non-const methodOop
  methodOop method() const                 { return _method; }
  void set_method(methodOop m)             { oop_store_without_check((oop*)&_method, (oop) m); }

  // exception handler table
  typeArrayOop exception_table() const           { return _exception_table; }
  void set_exception_table(typeArrayOop e)       { oop_store_without_check((oop*) &_exception_table, (oop) e); }
  bool has_exception_handler() const             { return exception_table() != NULL && exception_table()->length() > 0; }

  void init_fingerprint() { 
    const uint64_t initval = CONST64(0x8000000000000000);
    _fingerprint = initval;
  }

  uint64_t fingerprint() const                   {
    // Since reads aren't atomic for 64 bits, if any of the high or low order
    // word is the initial value, return 0.  See init_fingerprint for initval.
    int high_fp = (int)(_fingerprint >> 32);
    if ((int) _fingerprint == 0 || high_fp == 0x80000000) {
      return 0L;
    } else {
      return _fingerprint;
    }
  }

  uint64_t set_fingerprint(uint64_t new_fingerprint) {
#ifdef ASSERT
    // Assert only valid if complete/valid 64 bit _fingerprint value is read.
    uint64_t oldfp = fingerprint();
#endif // ASSERT
    _fingerprint = new_fingerprint;
    assert(oldfp == 0L || new_fingerprint == oldfp,
           "fingerprint cannot change");
    assert(((new_fingerprint >> 32) != 0x80000000) && (int)new_fingerprint !=0,
           "fingerprint should call init to set initial value");
    return new_fingerprint;
  }

  // name
  int name_index() const                         { return _name_index; }
  void set_name_index(int index)                 { _name_index = index; }

  // signature
  int signature_index() const                    { return _signature_index; }
  void set_signature_index(int index)            { _signature_index = index; }

  // generics support
  int generic_signature_index() const            { return _generic_signature_index; }
  void set_generic_signature_index(int index)    { _generic_signature_index = index; }

  // Sizing
  static int header_size() {
    return sizeof(constMethodOopDesc)/HeapWordSize;
  }

  // Object size needed
  static int object_size(int code_size, int compressed_line_number_size,
                         int local_variable_table_length,
                         int checked_exceptions_length);

  int object_size() const                 { return _constMethod_size; }
  void set_constMethod_size(int size)     { _constMethod_size = size; }

  // code size
  int code_size() const                          { return _code_size; }
  void set_code_size(int size) {
    assert(max_method_code_size < (1 << 16),
           "u2 is too small to hold method code size in general");
    assert(0 <= size && size <= max_method_code_size, "invalid code size");
    _code_size = size;
  }

  // linenumber table - note that length is unknown until decompression,
  // see class CompressedLineNumberReadStream.
  u_char* compressed_linenumber_table() const;         // not preserved by gc
  u2* checked_exceptions_length_addr() const;
  u2* localvariable_table_length_addr() const;

  // checked exceptions
  int checked_exceptions_length() const;
  CheckedExceptionElement* checked_exceptions_start() const;

  // localvariable table
  int localvariable_table_length() const;
  LocalVariableTableElement* localvariable_table_start() const;

  // byte codes
  address code_base() const            { return (address) (this+1); }
  address code_end() const             { return code_base() + code_size(); }
  bool    contains(address bcp) const  { return code_base() <= bcp
                                                     && bcp < code_end(); }
  // Offset to bytecodes
  static ByteSize codes_offset()
                            { return in_ByteSize(sizeof(constMethodOopDesc)); }

  // interpreter support
  static ByteSize exception_table_offset()
               { return byte_offset_of(constMethodOopDesc, _exception_table); }

  // Garbage collection support
  oop*  adr_method() const             { return (oop*)&_method;          }
  oop*  adr_exception_table() const    { return (oop*)&_exception_table; }
  oop*  adr_stackmap_u1() const        { return (oop*)&_stackmap_u1;     }
  oop*  adr_stackmap_u2() const        { return (oop*)&_stackmap_u2;     }

  // index into instanceKlass methods() array
  u2 method_index() const                        { return _method_index; }
  void set_method_index(u2 index)                { _method_index = index; }

  // stackmap table
  typeArrayOop stackmap_u1() const               { return _stackmap_u1; }
  typeArrayOop stackmap_u2() const               { return _stackmap_u2; }
  void set_stackmap_u1(typeArrayOop t)           { oop_store_without_check((oop*) &_stackmap_u1, (oop) t); }
  void set_stackmap_u2(typeArrayOop t)           { oop_store_without_check((oop*) &_stackmap_u2, (oop) t); }
  bool has_stackmap_table() const                { return stackmap_u2() != NULL && stackmap_u2()->length() > 0; }
  void delete_stackmap()                         { 
    set_stackmap_u1(Universe::the_empty_byte_array()); 
    set_stackmap_u2(Universe::the_empty_short_array());
  }

private:
  // Since the size of the compressed line number table is unknown, the
  // offsets of the other variable sized sections are computed backwards
  // from the end of the constMethodOop.

  // First byte after constMethodOop
  address constMethod_end() const
                          { return (address)((oop*)this + _constMethod_size); }

  // Last short in constMethodOop
  u2* last_u2_element() const
                                         { return (u2*)constMethod_end() - 1; }
};
