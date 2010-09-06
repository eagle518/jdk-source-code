#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciObjectFactory.cpp	1.27 03/12/23 16:39:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciObjectFactory.cpp.incl"

// ciObjectFactory
//
// This class handles requests for the creation of new instances
// of ciObject and its subclasses.  It contains a caching mechanism
// which ensures that for each oop, at most one ciObject is created.
// This invariant allows more efficient implementation of ciObject.
//
// Implementation note: the oop->ciObject mapping is represented as
// a table stored in an array.  Even though objects are moved
// by the garbage collector, the compactor preserves their relative
// order; address comparison of oops (in perm space) is safe so long
// as we prohibit GC during our comparisons.  We currently use binary
// search to find the oop in the table, and inserting a new oop
// into the table may be costly.  If this cost ends up being
// problematic the underlying data structure can be switched to some
// sort of balanced binary tree.

GrowableArray<ciObject*>* ciObjectFactory::_shared_ci_objects = NULL;
GrowableArray<ciObject*>* ciObjectFactory::_shared_ci_symbols = NULL;
GrowableArray<int>*       ciObjectFactory::_shared_ci_symbol_map = NULL;
int                       ciObjectFactory::_shared_ident_limit = 0;


// ------------------------------------------------------------------
// ciObjectFactory::ciObjectFactory
ciObjectFactory::ciObjectFactory(Arena* arena,
                                 int expected_size) {
  for (int i = 0; i < NON_PERM_BUCKETS; i++) {
    _non_perm_bucket[i] = NULL;
  }
  _non_perm_count = 0;

  if (!shared_is_initialized()) {
    init_shared_objects();
  }
  _next_ident = _shared_ident_limit;
  _arena = arena;
  _ci_objects = new (arena) GrowableArray<ciObject*>(arena, expected_size, 0, NULL);
  _ci_objects->appendAll(_shared_ci_objects);

  _unloaded_methods = new (arena) GrowableArray<ciMethod*>(arena, 4, 0, NULL);
  _unloaded_klasses = new (arena) GrowableArray<ciKlass*>(arena, 8, 0, NULL);
  _return_addresses =
    new (arena) GrowableArray<ciReturnAddress*>(arena, 8, 0, NULL);
}

// ------------------------------------------------------------------
// ciObjectFactory::ciObjectFactory
void ciObjectFactory::init_shared_objects() {
  ASSERT_IN_VM;
  ciEnv* env = ciEnv::current();

  Arena* save_env_arena = env->arena();

  env->_factory = this;

  // Create an arena for the shared ciObjects.
  _arena = new Arena();

  _next_ident = 1;  // start numbering CI objects at 1
  
  env->_arena = _arena;

  {
    // Create the shared symbols, but not in _shared_ci_objects.
    const int sym_count = (int) vmSymbolHandles::symbol_handle_count();
    int i;
    GrowableArray<ciObject*>* syms = new (_arena) GrowableArray<ciObject*>(_arena, sym_count, 0, NULL);
    for (i = 0; i < sym_count; i++) {
      symbolHandle sym_handle = vmSymbolHandles::symbol_handle_at(i);
      ciSymbol* sym = new (arena()) ciSymbol(sym_handle);
      int index = find(sym_handle(), syms);
      if (!is_found_at(index, sym_handle(), syms)) {
        init_ident_of(sym);
	insert(index, sym, syms);
      }
    }
    GrowableArray<int>* map = new (_arena) GrowableArray<int>(_arena, sym_count, 0, 0);
    for (i = 0; i < sym_count; i++) {
      symbolHandle sym_handle = vmSymbolHandles::symbol_handle_at(i);
      int index = find(sym_handle(), syms);
      assert(is_found_at(index, sym_handle(), syms), "must be valid index");
      map->append(index);
    }
    _shared_ci_symbols = syms;
    _shared_ci_symbol_map = map;
#ifdef ASSERT
    for (i = 0; i < sym_count; i++) {
      symbolHandle sym_handle = vmSymbolHandles::symbol_handle_at(i);
      ciSymbol* sym = vm_symbol_at(i);
      assert(sym->get_oop() == sym_handle(), "oop must match");
      assert(map->at(i) == find(sym_handle(), syms), "index must match");
    }
    assert(ciSymbol::void_class_signature()->get_oop() == vmSymbols::void_class_signature(), "spot check");
#endif
  }

  _ci_objects = new (_arena) GrowableArray<ciObject*>(_arena, 64, 0, NULL);

  for (int i = T_BOOLEAN; i <= T_CONFLICT; i++) {
    BasicType t = (BasicType)i;
    if (type2name(t) != NULL && t != T_OBJECT && t != T_ARRAY) {
      ciType::_basic_types[t] = new (_arena) ciType(t);
      init_ident_of(ciType::_basic_types[t]);
    }
  }

  ciEnv::_null_object_instance = new (_arena) ciNullObject();
  init_ident_of(ciEnv::_null_object_instance);
  ciEnv::_method_klass_instance =
    get(Universe::methodKlassObj())->as_method_klass();
  ciEnv::_symbol_klass_instance =
    get(Universe::symbolKlassObj())->as_symbol_klass();
  ciEnv::_klass_klass_instance =
    get(Universe::klassKlassObj())->as_klass_klass();
  ciEnv::_instance_klass_klass_instance =
    get(Universe::instanceKlassKlassObj())
      ->as_instance_klass_klass();
  ciEnv::_type_array_klass_klass_instance =
    get(Universe::typeArrayKlassKlassObj())
      ->as_type_array_klass_klass();
  ciEnv::_obj_array_klass_klass_instance =
    get(Universe::objArrayKlassKlassObj())
      ->as_obj_array_klass_klass();
  ciEnv::_ArrayStoreException =
    get(SystemDictionary::ArrayStoreException_klass())
      ->as_instance_klass();
  ciEnv::_Class =
    get(SystemDictionary::class_klass())
      ->as_instance_klass();
  ciEnv::_ClassCastException =
    get(SystemDictionary::ClassCastException_klass())
      ->as_instance_klass();
  ciEnv::_Object =
    get(SystemDictionary::object_klass())
      ->as_instance_klass();
  ciEnv::_Throwable =
    get(SystemDictionary::throwable_klass())
      ->as_instance_klass();
  ciEnv::_Thread =
    get(SystemDictionary::thread_klass())
      ->as_instance_klass();
  ciEnv::_OutOfMemoryError =
    get(SystemDictionary::OutOfMemoryError_klass())
      ->as_instance_klass();
  ciEnv::_String =
    get(SystemDictionary::string_klass())
      ->as_instance_klass(); 
  ciEnv::_unloaded_cisymbol = (ciSymbol*) ciObjectFactory::get(vmSymbols::dummy_symbol_oop());
  ciEnv::_unloaded_ciinstance_klass = new (_arena) ciInstanceKlass(ciEnv::_unloaded_cisymbol, NULL, NULL);
  ciEnv::_unloaded_ciobjarrayklass = new (_arena) ciObjArrayKlass(ciEnv::_unloaded_cisymbol, ciEnv::_unloaded_ciinstance_klass, 1);
  assert(ciEnv::_unloaded_ciobjarrayklass->is_obj_array_klass(), "just checking");

  get(Universe::boolArrayKlassObj());
  get(Universe::charArrayKlassObj());
  get(Universe::singleArrayKlassObj());
  get(Universe::doubleArrayKlassObj());
  get(Universe::byteArrayKlassObj());
  get(Universe::shortArrayKlassObj());
  get(Universe::intArrayKlassObj());
  get(Universe::longArrayKlassObj());

  env->_arena = save_env_arena;

  _shared_ci_objects = _ci_objects;
  assert(_non_perm_count == 0, "no shared non-perm objects");
  _shared_ident_limit = _next_ident;
  // The shared_ident_limit is the first ident number that will
  // be used for non-shared objects.  That is, numbers less than
  // this limit are permanently assigned to shared CI objects,
  // while the higher numbers are recycled afresh by each new ciEnv.
}

// ------------------------------------------------------------------
// ciObjectFactory::get
//
// Get the ciObject corresponding to some oop.  If the ciObject has
// already been created, it is returned.  Otherwise, a new ciObject
// is created.
ciObject* ciObjectFactory::get(oop key) {
  ASSERT_IN_VM;

#ifdef ASSERT
  oop last = NULL;
  for (int j = 0; j< _ci_objects->length(); j++) {
    oop o = _ci_objects->at(j)->get_oop();
    assert(last < o, "out of order");
    last = o;
  }
#endif ASSERT
  int len = _ci_objects->length();
  int index = find(key, _ci_objects);
#ifdef ASSERT
  for (int i=0; i<_ci_objects->length(); i++) {
    if (_ci_objects->at(i)->get_oop() == key) {
      assert(index == i, " bad lookup");
    }
  }
#endif
  if (!is_found_at(index, key, _ci_objects)) {

    // Check in the non-perm area before putting it in the list.
    NonPermObject* &bucket = find_non_perm(key);
    if (bucket != NULL) {
      return bucket->object();
    }

    // Check in the shared symbol area before putting it in the list.
    if (key->is_symbol()) {
      int sym_index = find(key, _shared_ci_symbols);
      if (is_found_at(sym_index, key, _shared_ci_symbols)) {
	// do not pollute the main cache with it
	return _shared_ci_symbols->at(sym_index);
      }
    }

    // The ciObject does not yet exist.  Create it and insert it
    // into the cache.
    Handle keyHandle(key);
    ciObject* new_object = create_new_object(keyHandle());
    assert(keyHandle() == new_object->get_oop(), "must be properly recorded");
    init_ident_of(new_object);
    if (!keyHandle->is_perm()) {
      // Not a perm-space object.
      insert_non_perm(bucket, keyHandle(), new_object);
      return new_object;
    }
    new_object->set_perm();
    if (len != _ci_objects->length()) {
      // creating the new object has recursively entered new objects
      // into the table.  We need to recompute our index.
      index = find(keyHandle(), _ci_objects);
    }
    assert(!is_found_at(index, keyHandle(), _ci_objects), "no double insert");
    insert(index, new_object, _ci_objects);
    return new_object;
  }
  return _ci_objects->at(index);
}

// ------------------------------------------------------------------
// ciObjectFactory::create_new_object
//
// Create a new ciObject from an oop.
//
// Implementation note: this functionality could be virtual behavior
// of the oop itself.  For now, we explicitly marshal the object.
ciObject* ciObjectFactory::create_new_object(oop o) {
  EXCEPTION_CONTEXT;

  if (o->is_symbol()) {
    symbolHandle h_o(THREAD, (symbolOop)o);
    return new (arena()) ciSymbol(h_o);
  } else if (o->is_klass()) {
    KlassHandle h_k(THREAD, (klassOop)o);
    Klass* k = ((klassOop)o)->klass_part();
    if (k->oop_is_instance()) {
      return new (arena()) ciInstanceKlass(h_k);
    } else if (k->oop_is_objArray()) {
      return new (arena()) ciObjArrayKlass(h_k);
    } else if (k->oop_is_typeArray()) {
      return new (arena()) ciTypeArrayKlass(h_k);
    } else if (k->oop_is_method()) {
      return new (arena()) ciMethodKlass(h_k);
    } else if (k->oop_is_symbol()) {
      return new (arena()) ciSymbolKlass(h_k);
    } else if (k->oop_is_klass()) {
      if (k->oop_is_objArrayKlass()) {
        return new (arena()) ciObjArrayKlassKlass(h_k);
      } else if (k->oop_is_typeArrayKlass()) {
        return new (arena()) ciTypeArrayKlassKlass(h_k);
      } else if (k->oop_is_instanceKlass()) {
        return new (arena()) ciInstanceKlassKlass(h_k);
      } else {
        assert(o == Universe::klassKlassObj(), "bad klassKlass");
        return new (arena()) ciKlassKlass(h_k);
      }
    }
  } else if (o->is_method()) {
    methodHandle h_m(THREAD, (methodOop)o);
    return new (arena()) ciMethod(h_m);
  } else if (o->is_methodData()) {
    methodDataHandle h_md(THREAD, (methodDataOop)o);
    return new (arena()) ciMethodData(h_md);
  } else if (o->is_instance()) {
    instanceHandle h_i(THREAD, (instanceOop)o);
    return new (arena()) ciInstance(h_i);
  } else if (o->is_objArray()) {
    objArrayHandle h_oa(THREAD, (objArrayOop)o);
    return new (arena()) ciObjArray(h_oa);
  } else if (o->is_typeArray()) {
    typeArrayHandle h_ta(THREAD, (typeArrayOop)o);
    return new (arena()) ciTypeArray(h_ta);
  }

  // The oop is of some type not supported by the compiler interface.
  ShouldNotReachHere();
  return NULL;
}

//------------------------------------------------------------------
// ciObjectFactory::get_unloaded_method
//
// Get the ciMethod representing an unloaded/unfound method.
//
// Implementation note: unloaded methods are currently stored in
// an unordered array, requiring a linear-time lookup for each
// unloaded method.  This may need to change.
ciMethod* ciObjectFactory::get_unloaded_method(ciInstanceKlass* holder,
                                               ciSymbol*        name,
                                               ciSymbol*        signature) {
  for (int i=0; i<_unloaded_methods->length(); i++) {
    ciMethod* entry = _unloaded_methods->at(i);
    if (entry->holder()->equals(holder) &&
        entry->name()->equals(name) &&
        entry->signature()->as_symbol()->equals(signature)) {
      // We've found a match.
      return entry;
    }
  }

  // This is a new unloaded method.  Create it and stick it in
  // the cache.
  ciMethod* new_method = new (arena()) ciMethod(holder, name, signature);

  init_ident_of(new_method);
  _unloaded_methods->append(new_method);

  return new_method;
}

//------------------------------------------------------------------
// ciObjectFactory::get_unloaded_klass
//
// Get a ciKlass representing an unloaded klass.
//
// Implementation note: unloaded klasses are currently stored in
// an unordered array, requiring a linear-time lookup for each
// unloaded klass.  This may need to change.
ciKlass* ciObjectFactory::get_unloaded_klass(ciKlass* accessing_klass,
                                             ciSymbol* name,
                                             bool create_if_not_found) {
  EXCEPTION_CONTEXT;
  oop loader = NULL;
  oop domain = NULL;
  if (accessing_klass != NULL) {
    loader = accessing_klass->loader();
    domain = accessing_klass->protection_domain();
  }
  for (int i=0; i<_unloaded_klasses->length(); i++) {
    ciKlass* entry = _unloaded_klasses->at(i);
    if (entry->name()->equals(name) &&
        entry->loader() == loader &&
        entry->protection_domain() == domain) {
      // We've found a match.
      return entry;
    }
  }

  if (!create_if_not_found)
    return NULL;

  // This is a new unloaded klass.  Create it and stick it in
  // the cache.
  ciKlass* new_klass = NULL;

  // Two cases: this is an unloaded objArrayKlass or an
  // unloaded instanceKlass.  Deal with both.
  if (name->byte_at(0) == '[') {
    // Decompose the name.'
    jint dimension = 0;
    symbolOop element_name = NULL;
    BasicType element_type= FieldType::get_array_info(name->get_symbolOop(),
                                                      &dimension,
                                                      &element_name,
                                                      THREAD);
#ifndef PRODUCT
    CURRENT_THREAD_ENV->maybe_fire_out_of_memory(THREAD);
#endif /* !PRODUCT */
    if (PENDING_EXCEPTION) {
      CLEAR_PENDING_EXCEPTION;
      CURRENT_THREAD_ENV->record_out_of_memory_failure();
      return ciEnv::_unloaded_ciobjarrayklass;
    }
    assert(element_type != T_ARRAY, "unsuccessful decomposition");
    ciKlass* element_klass = NULL;
    if (element_type == T_OBJECT) {
      ciEnv *env = CURRENT_THREAD_ENV;
      ciSymbol* ci_name = env->get_object(element_name)->as_symbol();
      element_klass =
        env->get_klass_by_name(accessing_klass, ci_name, false)->as_instance_klass();
    } else {
      assert(dimension > 1, "one dimensional type arrays are always loaded.");

      // The type array itself takes care of one of the dimensions.
      dimension--;

      // The element klass is a typeArrayKlass.
      element_klass = ciTypeArrayKlass::make(element_type);
    }
    new_klass = new (arena()) ciObjArrayKlass(name, element_klass, dimension);
  } else {
    jobject loader_handle = NULL;
    jobject domain_handle = NULL;
    if (accessing_klass != NULL) {
      loader_handle = accessing_klass->loader_handle();
      domain_handle = accessing_klass->protection_domain_handle();
    }
    new_klass = new (arena()) ciInstanceKlass(name, loader_handle, domain_handle);
  }
  init_ident_of(new_klass);
  _unloaded_klasses->append(new_klass);

  return new_klass;
}

//------------------------------------------------------------------
// ciObjectFactory::get_empty_methodData
//
// Get the ciMethodData representing the methodData for a method with 
// none.
ciMethodData* ciObjectFactory::get_empty_methodData() {
  ciMethodData* new_methodData = new (arena()) ciMethodData();
  init_ident_of(new_methodData);
  return new_methodData;
}

//------------------------------------------------------------------
// ciObjectFactory::get_return_address
//
// Get a ciReturnAddress for a specified bci.
ciReturnAddress* ciObjectFactory::get_return_address(int bci) {
  for (int i=0; i<_return_addresses->length(); i++) {
    ciReturnAddress* entry = _return_addresses->at(i);
    if (entry->bci() == bci) {
      // We've found a match.
      return entry;
    }
  }
  
  ciReturnAddress* new_ret_addr = new (arena()) ciReturnAddress(bci);
  init_ident_of(new_ret_addr);
  _return_addresses->append(new_ret_addr);
  return new_ret_addr;
}

// ------------------------------------------------------------------
// ciObjectFactory::init_ident_of
void ciObjectFactory::init_ident_of(ciObject* obj) {
  obj->set_ident(_next_ident++);
}


// ------------------------------------------------------------------
// ciObjectFactory::find
//
// Use binary search to find the position of this oop in the cache.
// If there is no entry in the cache corresponding to this oop, return
// the position at which the oop should be inserted.
int ciObjectFactory::find(oop key, GrowableArray<ciObject*>* objects) {
  int min = 0;
  int max = objects->length()-1;

  // print_contents();

  while (max >= min) {
    int mid = (max + min) / 2;
    oop value = objects->at(mid)->get_oop();
    if (value < key) {
      min = mid + 1;
    } else if (value > key) {
      max = mid - 1;
    } else {
      return mid;
    }
  }
  return min;
}

// ------------------------------------------------------------------
// ciObjectFactory::is_found_at
//
// Verify that the binary seach found the given key.
bool ciObjectFactory::is_found_at(int index, oop key, GrowableArray<ciObject*>* objects) {
  return (index < objects->length() &&
	  objects->at(index)->get_oop() == key);
}


// ------------------------------------------------------------------
// ciObjectFactory::insert
//
// Insert a ciObject into the table at some index.
void ciObjectFactory::insert(int index, ciObject* obj, GrowableArray<ciObject*>* objects) {
  int len = objects->length();
  if (len == index) {
    objects->append(obj);
  } else {
    objects->append(objects->at(len-1));
    int pos;
    for (pos = len-2; pos >= index; pos--) {
      objects->at_put(pos+1,objects->at(pos));
    }
    objects->at_put(index, obj);
  }
#ifdef ASSERT
  oop last = NULL;
  for (int j = 0; j< objects->length(); j++) {
    oop o = objects->at(j)->get_oop();
    assert(last < o, "out of order");
    last = o;
  }
#endif ASSERT
}

static ciObjectFactory::NonPermObject* emptyBucket = NULL;

// ------------------------------------------------------------------
// ciObjectFactory::find_non_perm
//
// Use a small hash table, hashed on the klass of the key.
// If there is no entry in the cache corresponding to this oop, return
// the null tail of the bucket into which the oop should be inserted.
ciObjectFactory::NonPermObject* &ciObjectFactory::find_non_perm(oop key) {
  // Be careful:  is_perm might change from false to true.
  // Thus, there might be a matching perm object in the table.
  // If there is, this probe must find it.
  if (key->is_perm() && _non_perm_count == 0) {
    return emptyBucket;
  } else if (key->is_instance()) {
    if (key->klass() == SystemDictionary::class_klass()) {
      // class mirror instances are always perm
      return emptyBucket;
    }
    // fall through to probe
  } else if (key->is_array()) {
    // fall through to probe
  } else {
    // not an array or instance
    return emptyBucket;
  }

  ciObject* klass = get(key->klass());
  NonPermObject* *bp = &_non_perm_bucket[(unsigned) klass->hash() % NON_PERM_BUCKETS];
  for (NonPermObject* p; (p = (*bp)) != NULL; bp = &p->next()) {
    if (is_equal(p, key))  break;
  }
  return (*bp);
}



// ------------------------------------------------------------------
// Code for for NonPermObject
//
inline ciObjectFactory::NonPermObject::NonPermObject(ciObjectFactory::NonPermObject* &bucket, oop key, ciObject* object) {
  assert(ciObjectFactory::shared_is_initialized(), "");
  _object = object;
  _next = bucket;
  bucket = this;
}



// ------------------------------------------------------------------
// ciObjectFactory::insert_non_perm
//
// Insert a ciObject into the non-perm table.
void ciObjectFactory::insert_non_perm(ciObjectFactory::NonPermObject* &where, oop key, ciObject* obj) {
  assert(&where != &emptyBucket, "must not try to fill empty bucket");
  NonPermObject* p = new (arena()) NonPermObject(where, key, obj);
  assert(where == p && is_equal(p, key) && p->object() == obj, "entry must match");
  assert(find_non_perm(key) == p, "must find the same spot");
  ++_non_perm_count;
}

// ------------------------------------------------------------------
// ciObjectFactory::vm_symbol_at
// Get the ciSymbol corresponding to some index in vmSymbols.
ciSymbol* ciObjectFactory::vm_symbol_at(int index) {
  int map_index = _shared_ci_symbol_map->at(index);
  ciObject* sym = _shared_ci_symbols->at(map_index);
  return sym->as_symbol();
}

// ------------------------------------------------------------------
// ciObjectFactory::print_contents_impl
void ciObjectFactory::print_contents_impl() {
  int len = _ci_objects->length();
  tty->print_cr("ciObjectFactory (%d) oop contents:", len);
  for (int i=0; i<len; i++) {
    _ci_objects->at(i)->print();
    tty->cr();
  }
}

// ------------------------------------------------------------------
// ciObjectFactory::print_contents
void ciObjectFactory::print_contents() {
  print();
  tty->cr();
  GUARDED_VM_ENTRY(print_contents_impl();)
}

// ------------------------------------------------------------------
// ciObjectFactory::print
//
// Print debugging information about the object factory
void ciObjectFactory::print() {
  tty->print("<ciObjectFactory oops=%d unloaded_methods=%d unloaded_klasses=%d>",
             _ci_objects->length(), _unloaded_methods->length(),
             _unloaded_klasses->length());
}

