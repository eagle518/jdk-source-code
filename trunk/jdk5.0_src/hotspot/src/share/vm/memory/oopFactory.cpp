#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)oopFactory.cpp	1.72 03/12/23 16:41:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_oopFactory.cpp.incl"


typeArrayOop oopFactory::new_charArray(const char* utf8_str, TRAPS) {
  int length = utf8_str == NULL ? 0 : UTF8::unicode_length(utf8_str);
  typeArrayOop result = new_charArray(length, CHECK_0);
  if (length != 0) {
    UTF8::convert_to_unicode(utf8_str, result->char_at_addr(0), length);
  }
  return result;
}

typeArrayOop oopFactory::new_permanent_charArray(int length, TRAPS) {
  return typeArrayKlass::cast(Universe::charArrayKlassObj())->allocate_permanent(length, THREAD);
}

typeArrayOop oopFactory::new_permanent_byteArray(int length, TRAPS) {
  return typeArrayKlass::cast(Universe::byteArrayKlassObj())->allocate_permanent(length, THREAD);
}


typeArrayOop oopFactory::new_permanent_shortArray(int length, TRAPS) {
  return typeArrayKlass::cast(Universe::shortArrayKlassObj())->allocate_permanent(length, THREAD);
}


typeArrayOop oopFactory::new_permanent_intArray(int length, TRAPS) {
  return typeArrayKlass::cast(Universe::intArrayKlassObj())->allocate_permanent(length, THREAD);
}


typeArrayOop oopFactory::new_typeArray(BasicType type, int length, TRAPS) {
  klassOop type_asKlassOop = Universe::typeArrayKlassObj(type);
  typeArrayKlass* type_asArrayKlass = typeArrayKlass::cast(type_asKlassOop);
  typeArrayOop result = type_asArrayKlass->allocate(length, THREAD);
  return result;
}


objArrayOop oopFactory::new_objArray(klassOop klass, int length, TRAPS) {
  assert(klass->is_klass(), "must be instance class");
  if (klass->klass_part()->oop_is_array()) {
    return ((arrayKlass*)klass->klass_part())->allocate_arrayArray(1, length, THREAD);
  } else {
    assert (klass->klass_part()->oop_is_instance(), "new object array with klass not an instanceKlass");
    return ((instanceKlass*)klass->klass_part())->allocate_objArray(1, length, THREAD);
  }
}

symbolOop oopFactory::new_symbol(const char* utf8_buffer, int utf8_length, TRAPS) {
  assert(utf8_buffer != NULL, "just checking");
  return SymbolTable::lookup(utf8_buffer, utf8_length, CHECK_0);
}

objArrayOop oopFactory::new_system_objArray(int length, TRAPS) {
  int size = objArrayOopDesc::object_size(length);
  KlassHandle klass (THREAD, Universe::systemObjArrayKlassObj());
  objArrayOop o = (objArrayOop)
    Universe::heap()->permanent_array_allocate(klass, size, length, CHECK_0);
  // initialization not needed, allocated cleared
  return o;
}


constantPoolOop oopFactory::new_constantPool(int length, TRAPS) {
  constantPoolKlass* ck = constantPoolKlass::cast(Universe::constantPoolKlassObj());
  return ck->allocate(length, CHECK_0);
}


constantPoolCacheOop oopFactory::new_constantPoolCache(int length, TRAPS) {
  constantPoolCacheKlass* ck = constantPoolCacheKlass::cast(Universe::constantPoolCacheKlassObj());
  return ck->allocate(length, CHECK_0);
}


klassOop oopFactory::new_instanceKlass(int vtable_len, int itable_len, int static_field_size, 
                                       int nonstatic_oop_map_size, ReferenceType rt, TRAPS) {
  instanceKlassKlass* ikk = instanceKlassKlass::cast(Universe::instanceKlassKlassObj());
  return ikk->allocate_instance_klass(vtable_len, itable_len, static_field_size, nonstatic_oop_map_size, rt, CHECK_0);
}


constMethodOop oopFactory::new_constMethod(int byte_code_size,
                                           int compressed_line_number_size,
                                           int localvariable_table_length,
                                           int checked_exceptions_length,
                                           TRAPS) {
  klassOop cmkObj = Universe::constMethodKlassObj();
  constMethodKlass* cmk = constMethodKlass::cast(cmkObj);
  return cmk->allocate(byte_code_size, compressed_line_number_size,
                       localvariable_table_length, checked_exceptions_length,
                       CHECK_0);
}


methodOop oopFactory::new_method(int byte_code_size, AccessFlags access_flags,
                                 int compressed_line_number_size,
                                 int localvariable_table_length,
                                 int checked_exceptions_length, TRAPS) {
  methodKlass* mk = methodKlass::cast(Universe::methodKlassObj());
  assert(!access_flags.is_native() || byte_code_size == 0,
         "native methods should not contain byte codes");
  constMethodHandle rw = new_constMethod(byte_code_size,
                                         compressed_line_number_size,
                                         localvariable_table_length,
                                         checked_exceptions_length, CHECK_0);
  return mk->allocate(rw, access_flags, CHECK_0);
}


#ifndef CORE
methodDataOop oopFactory::new_methodData(methodHandle method, TRAPS) {
  methodDataKlass* mdk = methodDataKlass::cast(Universe::methodDataKlassObj());
  return mdk->allocate(method, CHECK_0);
}
#endif // !CORE


#ifndef CORE
compiledICHolderOop oopFactory::new_compiledICHolder(methodHandle method, KlassHandle klass, TRAPS) {
  compiledICHolderKlass* ck = (compiledICHolderKlass*) Universe::compiledICHolderKlassObj()->klass_part();
  compiledICHolderOop c = ck->allocate(CHECK_0);
  c->set_holder_method(method());
  c->set_holder_klass(klass());
  return c;
}
#endif

