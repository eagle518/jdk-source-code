#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)klassVtable.hpp	1.44 04/03/24 09:24:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A klassVtable abstracts the variable-length vtable that is embedded in instanceKlass
// and arrayKlass.  klassVtable objects are used just as convenient transient accessors to the vtable,
// not to actually hold the vtable data.
// Note: the klassVtable should not be accessed before the class has been verified
// (until that point, the vtable is uninitialized).

// Currently a klassVtable contains a direct reference to the vtable data, and is therefore
// not preserved across GCs.

class vtableEntry;

class klassVtable : public ResourceObj {
  KlassHandle  _klass;            // my klass 
  int          _tableOffset;      // offset of start of vtable data within klass
  int          _length;           // length of vtable (number of entries)
#ifndef PRODUCT
  int          _verify_count;     // to make verify faster
#endif

  // Ordering important, so greater_than (>) can be used as an merge operator.
  enum AccessType {
    acc_private         = 0,
    acc_package_private = 1,
    acc_publicprotected = 2
  };

 public:
  klassVtable(KlassHandle h_klass, void* base, int length) : _klass(h_klass) {
    _tableOffset = (address)base - (address)h_klass(); _length = length;
  }

  // accessors
  vtableEntry* table() const      { return (vtableEntry*)(address(_klass()) + _tableOffset); }
  KlassHandle klass() const       { return _klass;  }
  int length() const              { return _length; }
  inline methodOop method_at(int i) const;
  inline methodOop unchecked_method_at(int i) const;

  // searching; all methods return -1 if not found
  int index_of(methodOop m) const                         { return index_of(m, _length); }
  int index_of_miranda(symbolOop name, symbolOop signature);

  void initialize_vtable(Thread *thread);   // initialize vtable of a new klass
  
  // conputes vtable length (in words) and the number of miranda methods
  static void compute_vtable_size_and_num_mirandas(int &vtable_length, int &num_miranda_methods,
						   klassOop super, objArrayOop methods, 
						   AccessFlags class_flags, oop classloader,
						   symbolOop classname, objArrayOop local_interfaces);

#ifdef HOTSWAP
  // Evolution support. If any entry of this vtable points to any of old_methods, replace it
  // with the corresponding new_method.
  void adjust_entries(objArrayOop old_methods, objArrayOop new_methods);
#endif HOTSWAP
    
  // Garbage collection
  void oop_follow_contents();
  void oop_adjust_pointers();

  // Iterators
  void oop_oop_iterate(OopClosure* blk);
  void oop_oop_iterate_m(OopClosure* blk, MemRegion mr);

  // Debugging code
  void print()                                              PRODUCT_RETURN;
  void verify(outputStream* st, bool force = false)         PRODUCT_RETURN;
  static void print_statistics()                            PRODUCT_RETURN;

 protected:
  friend class vtableEntry;
 private:
  void copy_vtable_to(vtableEntry* start);
  int  initialize_from_super(KlassHandle super);
  int  index_of(methodOop m, int len) const; // same as index_of, but search only up to len
  void put_method_at(methodOop m, int index);
  static bool needs_new_vtable_entry(methodOop m, klassOop super, oop classloader, symbolOop classname, AccessFlags access_flags);
  AccessType vtable_accessibility_at(int i);

  bool update_super_vtable(instanceKlass* klass, methodOop target_method, int super_vtable_len);

  // support for miranda methods
  bool is_miranda_entry_at(int i);
  void fill_in_mirandas(int& initialized);
  static bool is_miranda(methodOop m, objArrayOop class_methods, klassOop super);
  static void add_new_mirandas_to_list(GrowableArray<methodOop>* list_of_current_mirandas, objArrayOop current_interface_methods, objArrayOop class_methods, klassOop super);
  static void get_mirandas(GrowableArray<methodOop>* mirandas, klassOop super, objArrayOop class_methods, objArrayOop local_interfaces);
  static int get_num_mirandas(klassOop super, objArrayOop class_methods, objArrayOop local_interfaces);
    
  
  void verify_against(outputStream* st, klassVtable* vt, int index) PRODUCT_RETURN;
  inline instanceKlass* ik() const;    
};


// private helper class for klassVtable
// description of entry points:
//    destination is interpreted:
//      from_compiled_code_entry_point -> c2iadapter
//      from_interpreter_entry_point   -> interpreter entry point
//    destination is compiled:
//      from_compiled_code_entry_point -> nmethod entry point
//      from_interpreter_entry_point   -> i2cadapter
class vtableEntry VALUE_OBJ_CLASS_SPEC {
 public:
  // size in words
  static int size() {
    return sizeof(vtableEntry) / sizeof(HeapWord);
  }
  static int method_offset_in_bytes() { return (intptr_t)&((vtableEntry*)NULL)->_method; }
  methodOop method() const    { return _method; }

 private:
  methodOop _method;
  void set(methodOop method)  { assert(method != NULL, "use clear"); _method = method; }
  void clear()                { _method = NULL; }
  void print()                                        PRODUCT_RETURN;
  void verify(klassVtable* vt, outputStream* st)      PRODUCT_RETURN;

  friend class klassVtable;
};


inline methodOop klassVtable::method_at(int i) const { 
  assert(i >= 0 && i < _length, "index out of bounds");
  assert(table()[i].method() != NULL, "should not be null");
  assert(oop(table()[i].method())->is_method(), "should be method");
  return table()[i].method();
}


inline methodOop klassVtable::unchecked_method_at(int i) const { 
  assert(i >= 0 && i < _length, "index out of bounds");
  return table()[i].method();
}

// --------------------------------------------------------------------------------
class klassItable;
class itableMethodEntry;

class itableOffsetEntry VALUE_OBJ_CLASS_SPEC {
 private:
  klassOop _interface;
  int      _offset;
 public:
  klassOop interface_klass() const { return _interface; }
  int      offset() const          { return _offset; }

  itableMethodEntry* first_method_entry(klassOop k) { return (itableMethodEntry*)(((address)k) + _offset); }

  void initialize(klassOop interf, int offset) { _interface = interf; _offset = offset; }

  // Static size and offset accessors
  static int size()                       { return sizeof(itableOffsetEntry) / HeapWordSize; }    // size in words
  static int interface_offset_in_bytes()  { return (intptr_t)&((itableOffsetEntry*)NULL)->_interface; }
  static int offset_offset_in_bytes()     { return (intptr_t)&((itableOffsetEntry*)NULL)->_offset; }

  friend class klassItable;
};


class itableMethodEntry VALUE_OBJ_CLASS_SPEC { 
 private:
  methodOop _method;

 public:
  methodOop method() const { return _method; }

  void clear()             { _method = NULL; }

  void initialize(methodOop method); 

  // Static size and offset accessors
  static int size()                         { return sizeof(itableMethodEntry) / HeapWordSize; }  // size in words
  static int method_offset_in_bytes()       { return (intptr_t)&((itableMethodEntry*)NULL)->_method; }

  friend class klassItable;
};

//
// Format of an itable
//
//    ---- offset table ---
//    klassOop of interface 1             \
//    offset to vtable from start of oop  / offset table entry
//    ...
//    klassOop of interface n             \
//    offset to vtable from start of oop  / offset table entry
//    --- vtable for interface 1 ---
//    methodOop                           \ 
//    compiler entry point                / method table entry
//    ...
//    methodOop                           \ 
//    compiler entry point                / method table entry
//    -- vtable for interface 2 ---
//    ...
//      
class klassItable : public ResourceObj {
 private:
  instanceKlassHandle  _klass;             // my klass 
  int                  _table_offset;      // offset of start of itable data within klass (in words)
  int                  _size_offset_table; // size of offset table (in itableOffset entries)
  int                  _size_method_table; // size of methodtable (in itableMethodEntry entries)

  void initialize_itable_for_interface(klassOop interf, itableMethodEntry* method_table);
 public:
  klassItable(instanceKlassHandle klass);

  itableOffsetEntry* offset_entry(int i) { assert(0 <= i && i <= _size_offset_table, "index out of bounds");
                                           return &((itableOffsetEntry*)vtable_start())[i]; }

  itableMethodEntry* method_entry(int i) { assert(0 <= i && i <= _size_method_table, "index out of bounds");
                                           return &((itableMethodEntry*)method_start())[i]; }
  
  int nof_interfaces()                   { return _size_offset_table; }

  // Initialization
  void initialize_itable();    

  // Updates
  void initialize_with_method(methodOop m);

#ifdef HOTSWAP
  // Evolution support
  void adjust_method_entries(objArrayOop old_methods, objArrayOop new_methods);
#endif HOTSWAP

  // Garbage collection
  void oop_follow_contents();
  void oop_adjust_pointers();

  // Iterators
  void oop_oop_iterate(OopClosure* blk);
  void oop_oop_iterate_m(OopClosure* blk, MemRegion mr);

  // Setup of itable
  static int compute_itable_size(objArrayHandle transitive_interfaces);
  static void setup_itable_offset_table(instanceKlassHandle klass);

  // Resolving of method to index
  static int compute_itable_index(methodOop m);

  // Debugging/Statistics
  static void print_statistics() PRODUCT_RETURN;
 private:  
  intptr_t* vtable_start() const { return ((intptr_t*)_klass()) + _table_offset; }
  intptr_t* method_start() const { return vtable_start() + _size_offset_table * itableOffsetEntry::size(); }

  // Helper methods  
  static int  calc_itable_size(int num_interfaces, int num_methods) { return (num_interfaces * itableOffsetEntry::size()) + (num_methods * itableMethodEntry::size()); }

  // Statistics
  NOT_PRODUCT(static int  _total_classes;)   // Total no. of classes with itables
  NOT_PRODUCT(static long _total_size;)      // Total no. of bytes used for itables

  static void update_stats(int size) PRODUCT_RETURN NOT_PRODUCT({ _total_classes++; _total_size += size; })
};
