#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oopFactory.hpp	1.50 03/12/23 16:41:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// oopFactory is a class used for creating new objects.

class vframeArray;

class oopFactory: AllStatic {
 public:
  // Basic type leaf array allocation
  static typeArrayOop    new_boolArray  (int length, TRAPS) { return typeArrayKlass::cast(Universe::boolArrayKlassObj  ())->allocate(length, CHECK_0); }
  static typeArrayOop    new_charArray  (int length, TRAPS) { return typeArrayKlass::cast(Universe::charArrayKlassObj  ())->allocate(length, CHECK_0); }
  static typeArrayOop    new_singleArray(int length, TRAPS) { return typeArrayKlass::cast(Universe::singleArrayKlassObj())->allocate(length, CHECK_0); }
  static typeArrayOop    new_doubleArray(int length, TRAPS) { return typeArrayKlass::cast(Universe::doubleArrayKlassObj())->allocate(length, CHECK_0); }
  static typeArrayOop    new_byteArray  (int length, TRAPS) { return typeArrayKlass::cast(Universe::byteArrayKlassObj  ())->allocate(length, CHECK_0); }
  static typeArrayOop    new_shortArray (int length, TRAPS) { return typeArrayKlass::cast(Universe::shortArrayKlassObj ())->allocate(length, CHECK_0); }
  static typeArrayOop    new_intArray   (int length, TRAPS) { return typeArrayKlass::cast(Universe::intArrayKlassObj   ())->allocate(length, CHECK_0); }
  static typeArrayOop    new_longArray  (int length, TRAPS) { return typeArrayKlass::cast(Universe::longArrayKlassObj  ())->allocate(length, CHECK_0); }

  static typeArrayOop    new_charArray           (const char* utf8_str,  TRAPS);
  static typeArrayOop    new_permanent_charArray (int length, TRAPS);
  static typeArrayOop    new_permanent_byteArray (int length, TRAPS);  // used for class file structures
  static typeArrayOop    new_permanent_shortArray(int length, TRAPS);  // used for class file structures
  static typeArrayOop    new_permanent_intArray  (int length, TRAPS);  // used for class file structures
  
  static typeArrayOop    new_typeArray(BasicType type, int length, TRAPS);

  // Symbols
  static symbolOop       new_symbol(const char* name, int length, TRAPS);
  static symbolOop       new_symbol(char* name, TRAPS) { return new_symbol(name, (int)strlen(name), CHECK_0); }
  static symbolOop       new_symbol(const char* name, TRAPS) { return new_symbol(name, (int)strlen(name), CHECK_0); }

  // Create symbols as above but return a handle
  static symbolHandle    new_symbol_handle(const char* name, int length, TRAPS) {
    symbolOop sym = new_symbol(name, length, THREAD);
    return symbolHandle(THREAD, sym);
  }
  static symbolHandle    new_symbol_handle(char* name, TRAPS) { return new_symbol_handle(name, (int)strlen(name), CHECK_(symbolHandle())); }
  static symbolHandle    new_symbol_handle(const char* name, TRAPS) { return new_symbol_handle(name, (int)strlen(name), CHECK_(symbolHandle())); }

  // Constant pools
  static constantPoolOop      new_constantPool     (int length, TRAPS);
  static constantPoolCacheOop new_constantPoolCache(int length, TRAPS);

  // Instance classes
  static klassOop        new_instanceKlass(int vtable_len, int itable_len, int static_field_size, 
                                           int nonstatic_oop_map_size, ReferenceType rt, TRAPS);

  // Methods
private:
  static constMethodOop  new_constMethod(int byte_code_size,
                                         int compressed_line_number_size,
                                         int localvariable_table_length,
                                         int checked_exceptions_length, TRAPS);
public:
  static methodOop       new_method(int byte_code_size, AccessFlags access_flags, int compressed_line_number_size, int localvariable_table_length, int checked_exceptions_length, TRAPS);

#ifndef CORE
  // Method Data containers
  static methodDataOop   new_methodData(methodHandle method, TRAPS);
#endif // !CORE

  // System object arrays
  static objArrayOop     new_system_objArray(int length, TRAPS);

  // Regular object arrays
  static objArrayOop     new_objArray(klassOop klass, int length, TRAPS);

#ifndef CORE
  // For compiled ICs
  static compiledICHolderOop new_compiledICHolder(methodHandle method, KlassHandle klass, TRAPS);
#endif
};
