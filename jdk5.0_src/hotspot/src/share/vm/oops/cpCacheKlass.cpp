#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cpCacheKlass.cpp	1.35 03/12/23 16:41:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_cpCacheKlass.cpp.incl"


int constantPoolCacheKlass::oop_size(oop obj) const { 
  assert(obj->is_constantPoolCache(), "must be constantPool");
  return constantPoolCacheOop(obj)->object_size();
}


constantPoolCacheOop constantPoolCacheKlass::allocate(int length, TRAPS) {
  // allocate memory
  int size = constantPoolCacheOopDesc::object_size(length);
  KlassHandle klass (THREAD, as_klassOop());
  constantPoolCacheOop cache = (constantPoolCacheOop)
    CollectedHeap::permanent_array_allocate(klass, size, length, CHECK_0);
  cache->set_constant_pool(NULL);
  return cache;
}


klassOop constantPoolCacheKlass::create_klass(TRAPS) {
  constantPoolCacheKlass o;
  KlassHandle klassklass(THREAD, Universe::arrayKlassKlassObj());  
  arrayKlassHandle k = base_create_array_klass(o.vtbl_value(), header_size(), klassklass, CHECK_0);
  KlassHandle super (THREAD, k->super());
  complete_create_array_klass(k, super, CHECK_0);
  return k();
}


void constantPoolCacheKlass::oop_follow_contents(oop obj) {
  assert(obj->is_constantPoolCache(), "obj must be constant pool cache");
  constantPoolCacheOop cache = (constantPoolCacheOop)obj;
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolCacheKlassObj never moves.
  // gc of constant pool cache instance variables
  MarkSweep::mark_and_push((oop*)cache->constant_pool_addr());
  // gc of constant pool cache entries
  int i = cache->length();
  while (i-- > 0) cache->entry_at(i)->follow_contents();
}


int constantPoolCacheKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert(obj->is_constantPoolCache(), "obj must be constant pool cache");
  constantPoolCacheOop cache = (constantPoolCacheOop)obj;
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cache->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolCacheKlassObj never moves.
  // iteration over constant pool cache instance variables
  blk->do_oop((oop*)cache->constant_pool_addr());
  // iteration over constant pool cache entries
  for (int i = 0; i < cache->length(); i++) cache->entry_at(i)->oop_iterate(blk);
  return size;
}


int constantPoolCacheKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert(obj->is_constantPoolCache(), "obj must be constant pool cache");
  constantPoolCacheOop cache = (constantPoolCacheOop)obj;
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cache->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolCacheKlassObj never moves.
  // iteration over constant pool cache instance variables
  oop* addr = (oop*)cache->constant_pool_addr();
  if (mr.contains(addr)) blk->do_oop(addr);
  // iteration over constant pool cache entries
  for (int i = 0; i < cache->length(); i++) cache->entry_at(i)->oop_iterate_m(blk, mr);
  return size;
}


int constantPoolCacheKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_constantPoolCache(), "obj must be constant pool cache");
  constantPoolCacheOop cache = (constantPoolCacheOop)obj;
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cache->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolCacheKlassObj never moves.
  // Iteration over constant pool cache instance variables
  MarkSweep::adjust_pointer((oop*)cache->constant_pool_addr());
  // iteration over constant pool cache entries
  for (int i = 0; i < cache->length(); i++)
    cache->entry_at(i)->adjust_pointers();
  return size;
}


void constantPoolCacheKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_constantPoolCache(), "should be constant pool");
}

#ifndef PRODUCT

void constantPoolCacheKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_constantPoolCache(), "obj must be constant pool cache");
  constantPoolCacheOop cache = (constantPoolCacheOop)obj;
  // super print
  arrayKlass::oop_print_on(obj, st);
  // print constant pool cache entries
  for (int i = 0; i < cache->length(); i++) cache->entry_at(i)->print(st, i);
}


void constantPoolCacheKlass::oop_verify_on(oop obj, outputStream* st) {
  assert(obj->is_constantPoolCache(), "obj must be constant pool cache");
  constantPoolCacheOop cache = (constantPoolCacheOop)obj;
  // super verify
  arrayKlass::oop_verify_on(obj, st);
  // print constant pool cache entries
  for (int i = 0; i < cache->length(); i++) cache->entry_at(i)->verify(st);
}


const char* constantPoolCacheKlass::internal_name() const { 
  return "{constant pool cache}"; 
}

#endif

