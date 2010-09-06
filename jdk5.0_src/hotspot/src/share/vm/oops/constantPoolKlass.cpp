#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)constantPoolKlass.cpp	1.89 03/12/23 16:41:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_constantPoolKlass.cpp.incl"

constantPoolOop constantPoolKlass::allocate(int length, TRAPS) {
  int size = constantPoolOopDesc::object_size(length);
  KlassHandle klass (THREAD, as_klassOop());
  constantPoolOop c = 
    (constantPoolOop)CollectedHeap::permanent_array_allocate(klass, size, length, CHECK_0);

  c->set_tags(NULL);
  c->set_cache(NULL);
  c->set_pool_holder(NULL);
  // all fields are initialized; needed for GC

  // initialize tag array
  // Note: cannot introduce constant pool handle before since it is not
  //       completely initialized (no class) -> would cause assertion failure
  constantPoolHandle pool (THREAD, c);
  typeArrayOop t_oop = oopFactory::new_permanent_byteArray(length, CHECK_0);
  typeArrayHandle tags (THREAD, t_oop);
  pool->set_tags(tags());
  for (int index = 0; index < length; index++) {
    pool->tag_at_put(index, JVM_CONSTANT_Invalid);
  }

  return pool();
}

klassOop constantPoolKlass::create_klass(TRAPS) {
  constantPoolKlass o;
  KlassHandle klassklass(THREAD, Universe::arrayKlassKlassObj());  
  arrayKlassHandle k = base_create_array_klass(o.vtbl_value(), header_size(), klassklass, CHECK_0);
  arrayKlassHandle super (THREAD, k->super());
  complete_create_array_klass(k, super, CHECK_0);
  return k();
}


int constantPoolKlass::oop_size(oop obj) const { 
  assert(obj->is_constantPool(), "must be constantPool");
  return constantPoolOop(obj)->object_size();
}


void constantPoolKlass::oop_follow_contents(oop obj) {
  assert (obj->is_constantPool(), "obj must be constant pool");
  constantPoolOop cp = (constantPoolOop) obj;
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolKlassObj never moves.

  // If the tags array is null we are in the middle of allocating this constant pool
  if (cp->tags() != NULL) {
    // gc of constant pool contents
    oop* base = (oop*)cp->base();
    for (int i = 0; i < cp->length(); i++) {
      if ( (cp->tag_at(i).is_klass())  ||
           (cp->tag_at(i).is_unresolved_klass()) ||
           (cp->tag_at(i).is_symbol()) ||
           (cp->tag_at(i).is_unresolved_string()) ||
           (cp->tag_at(i).is_string())) {
        if (*base != NULL) MarkSweep::mark_and_push(base); 
      } 
      base++;
    }
    // gc of constant pool instance variables
    MarkSweep::mark_and_push(cp->tags_addr());
    MarkSweep::mark_and_push(cp->cache_addr());
    MarkSweep::mark_and_push(cp->pool_holder_addr());
  }
}


int constantPoolKlass::oop_adjust_pointers(oop obj) {
  assert (obj->is_constantPool(), "obj must be constant pool");
  constantPoolOop cp = (constantPoolOop) obj;
  // Get size before changing pointers. 
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cp->object_size();
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolKlassObj never moves.

  // If the tags array is null we are in the middle of allocating this constant pool
  if (cp->tags() != NULL) {
    oop* base = (oop*)cp->base();
    for (int i = 0; i< cp->length();  i++) {
      if ( (cp->tag_at(i).is_klass())  ||
           (cp->tag_at(i).is_unresolved_klass()) ||
           (cp->tag_at(i).is_symbol()) ||
           (cp->tag_at(i).is_unresolved_string()) ||
           (cp->tag_at(i).is_string())) {
        MarkSweep::adjust_pointer(base); 
      } 
      base++;
    }
  }
  MarkSweep::adjust_pointer(cp->tags_addr());
  MarkSweep::adjust_pointer(cp->cache_addr());
  MarkSweep::adjust_pointer(cp->pool_holder_addr());
  return size;
}


int constantPoolKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert (obj->is_constantPool(), "obj must be constant pool");
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolKlassObj never moves.
  constantPoolOop cp = (constantPoolOop) obj;
  // Get size before changing pointers. 
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cp->object_size();

  // If the tags array is null we are in the middle of allocating this constant pool
  if (cp->tags() != NULL) {
    oop* base = (oop*)cp->base();
    for (int i = 0; i< cp->length();  i++) {
      if ( (cp->tag_at(i).is_klass())  ||
           (cp->tag_at(i).is_unresolved_klass()) ||
           (cp->tag_at(i).is_symbol()) ||
           (cp->tag_at(i).is_unresolved_string()) ||
           (cp->tag_at(i).is_string())) {
        blk->do_oop(base); 
      } 
      base++;
    }
  }
  blk->do_oop(cp->tags_addr());
  blk->do_oop(cp->cache_addr());
  blk->do_oop(cp->pool_holder_addr());
  return size;
}


int constantPoolKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert (obj->is_constantPool(), "obj must be constant pool");
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::constantPoolKlassObj never moves.
  constantPoolOop cp = (constantPoolOop) obj;
  // Get size before changing pointers. 
  // Don't call size() or oop_size() since that is a virtual call.
  int size = cp->object_size();

  // If the tags array is null we are in the middle of allocating this constant pool
  if (cp->tags() != NULL) {
    oop* base = (oop*)cp->base();
    for (int i = 0; i< cp->length();  i++) {
      if (mr.contains(base)) {
        if ( (cp->tag_at(i).is_klass())  ||
             (cp->tag_at(i).is_unresolved_klass()) ||
             (cp->tag_at(i).is_symbol()) ||
             (cp->tag_at(i).is_unresolved_string()) ||
             (cp->tag_at(i).is_string())) {
          blk->do_oop(base); 
        }
      }
      base++;
    }
  }
  oop* addr;
  addr = cp->tags_addr();
  blk->do_oop(addr);
  addr = cp->cache_addr();
  blk->do_oop(addr);
  addr = cp->pool_holder_addr();
  blk->do_oop(addr);
  return size;
}


void constantPoolKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_constantPool(), "should be constant pool");
}


#ifndef PRODUCT

// Printing

void constantPoolKlass::oop_print_on(oop obj, outputStream* st) {
  EXCEPTION_MARK;
  oop anObj;
  assert(obj->is_constantPool(), "must be constantPool");
  arrayKlass::oop_print_on(obj, st);
  constantPoolOop cp = constantPoolOop(obj);  

  // Temp. remove cache so we can do lookups with original indicies.
  constantPoolCacheHandle cache (THREAD, cp->cache());
  cp->set_cache(NULL);

  for (int index = 1; index < cp->length(); index++) {      // Index 0 is unused
    st->print(" - %3d : ", index);
    cp->tag_at(index).print_on(st);
    st->print(" : ");
    switch (cp->tag_at(index).value()) {
      case JVM_CONSTANT_Class :
        { anObj = cp->klass_at(index, CATCH);
          anObj->print_value_on(st);
          st->print(" {0x%lx}", anObj);
        }
        break;
      case JVM_CONSTANT_Fieldref :
      case JVM_CONSTANT_Methodref :
      case JVM_CONSTANT_InterfaceMethodref :        
        st->print("klass_index=%d", cp->klass_ref_index_at(index));
        st->print(" name_and_type_index=%d", cp->name_and_type_ref_index_at(index));        
        break;
      case JVM_CONSTANT_UnresolvedString :
      case JVM_CONSTANT_String :
        anObj = cp->string_at(index, CATCH);
        anObj->print_value_on(st);
        st->print(" {0x%lx}", anObj);
        break;
      case JVM_CONSTANT_Integer :
        st->print("%d", cp->int_at(index));
        break;
      case JVM_CONSTANT_Float :
        st->print("%f", cp->float_at(index));
        break;
      case JVM_CONSTANT_Long :
        st->print_jlong(cp->long_at(index));
        index++;   // Skip entry following eigth-byte constant
        break;
      case JVM_CONSTANT_Double :
        st->print("%lf", cp->double_at(index));
        index++;   // Skip entry following eigth-byte constant
        break;
      case JVM_CONSTANT_NameAndType :        
        st->print("name_index=%d", cp->name_ref_index_at(index));
        st->print(" signature_index=%d", cp->signature_ref_index_at(index));
        break;
      case JVM_CONSTANT_Utf8 :
        cp->symbol_at(index)->print_value_on(st);
        break;
      case JVM_CONSTANT_UnresolvedClass : {
        // unresolved_klass_at requires lock or safe world.
        oop entry = *cp->obj_at_addr(index);
        entry->print_value_on(st);
        }
        break;
      default:
        ShouldNotReachHere();
        break;
    }
    st->cr();
  }
  st->cr();

  // Restore cache
  cp->set_cache(cache());
}


const char* constantPoolKlass::internal_name() const {
  return "{constant pool}";
}

// Verification

void constantPoolKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  guarantee(obj->is_constantPool(), "object must be constant pool");
  constantPoolOop cp = constantPoolOop(obj);  
  guarantee(cp->is_perm(), "should be in permspace");
  if (!cp->partially_loaded()) {
    oop* base = (oop*)cp->base();
    for (int i = 0; i< cp->length();  i++) {
      if (cp->tag_at(i).is_klass()) {
        guarantee((*base)->is_perm(),     "should be in permspace");
        guarantee((*base)->is_klass(),    "should be klass");
      }
      if (cp->tag_at(i).is_unresolved_klass()) {
        guarantee((*base)->is_perm(),     "should be in permspace");
        guarantee((*base)->is_symbol() || (*base)->is_klass(),
                  "should be symbol or klass");
      }
      if (cp->tag_at(i).is_symbol()) {
        guarantee((*base)->is_perm(),     "should be in permspace");
        guarantee((*base)->is_symbol(),   "should be symbol");
      }
      if (cp->tag_at(i).is_unresolved_string()) {
        guarantee((*base)->is_perm(),     "should be in permspace");
        guarantee((*base)->is_symbol() || (*base)->is_instance(),
                  "should be symbol or instance");
      }
      if (cp->tag_at(i).is_string()) {
        guarantee((*base)->is_perm(),     "should be in permspace");
        guarantee((*base)->is_instance(), "should be instance");
      }
      base++;
    }
    guarantee(cp->tags()->is_perm(),         "should be in permspace");
    guarantee(cp->tags()->is_typeArray(),    "should be type array");
    if (cp->cache() != NULL) {
      // Note: cache() can be NULL before a class is completely setup
      guarantee(cp->cache()->is_perm(),              "should be in permspace");
      guarantee(cp->cache()->is_constantPoolCache(), "should be constant pool cache");
    }
    guarantee(cp->pool_holder()->is_perm(),  "should be in permspace");
    guarantee(cp->pool_holder()->is_klass(), "should be klass");
  }
}

bool constantPoolKlass::oop_partially_loaded(oop obj) const {
  assert(obj->is_constantPool(), "object must be constant pool");
  constantPoolOop cp = constantPoolOop(obj);
  return cp->tags() == NULL || cp->pool_holder() == (klassOop) cp;   // Check whether pool holder points to self
}


void constantPoolKlass::oop_set_partially_loaded(oop obj) {
  assert(obj->is_constantPool(), "object must be constant pool");
  constantPoolOop cp = constantPoolOop(obj);
  assert(cp->pool_holder() == NULL, "just checking");
  cp->set_pool_holder((klassOop) cp);   // Temporarily set pool holder to point to self
}

// CompileTheWorld support. Preload all classes loaded references in the passed in constantpool
void constantPoolKlass::preload_and_initialize_all_classes(oop obj, TRAPS) {
  guarantee(obj->is_constantPool(), "object must be constant pool");
  constantPoolHandle cp(THREAD, (constantPoolOop)obj);  
  guarantee(!cp->partially_loaded(), "must be fully loaded");
    
  for (int i = 0; i< cp->length();  i++) {    
    if (cp->tag_at(i).is_unresolved_klass()) {
      // This will force loading of the class
      klassOop klass = cp->klass_at(i, CHECK);
      if (klass->is_instance()) {
        // Force initialization of class
        instanceKlass::cast(klass)->initialize(CHECK);
      } 
    }    
  }
}

#endif
