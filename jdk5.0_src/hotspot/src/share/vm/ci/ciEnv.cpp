#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciEnv.cpp	1.96 04/03/02 02:08:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciEnv.cpp.incl"

// ciEnv
//
// This class is the top level broker for requests from the compiler
// to the VM.

ciObject*              ciEnv::_null_object_instance;
ciMethodKlass*         ciEnv::_method_klass_instance;
ciSymbolKlass*         ciEnv::_symbol_klass_instance;
ciKlassKlass*          ciEnv::_klass_klass_instance;
ciInstanceKlassKlass*  ciEnv::_instance_klass_klass_instance;
ciTypeArrayKlassKlass* ciEnv::_type_array_klass_klass_instance;
ciObjArrayKlassKlass*  ciEnv::_obj_array_klass_klass_instance;
 
ciInstanceKlass* ciEnv::_ArrayStoreException;
ciInstanceKlass* ciEnv::_Class;
ciInstanceKlass* ciEnv::_ClassCastException;
ciInstanceKlass* ciEnv::_Object;
ciInstanceKlass* ciEnv::_Throwable;
ciInstanceKlass* ciEnv::_Thread;
ciInstanceKlass* ciEnv::_OutOfMemoryError;
ciInstanceKlass* ciEnv::_String;

ciSymbol*        ciEnv::_unloaded_cisymbol = NULL;
ciInstanceKlass* ciEnv::_unloaded_ciinstance_klass = NULL;
ciObjArrayKlass* ciEnv::_unloaded_ciobjarrayklass = NULL;

jobject ciEnv::_ArrayIndexOutOfBoundsException_handle = NULL;
jobject ciEnv::_ArrayStoreException_handle = NULL;
jobject ciEnv::_ClassCastException_handle = NULL;

#ifndef PRODUCT
static bool firstEnv = true;
int              ciEnv::_fire_out_of_memory_count_delay = 0;
#endif /* PRODUCT */

// ------------------------------------------------------------------
// ciEnv::ciEnv
ciEnv::ciEnv(JNIEnv* jni_env, int system_dictionary_modification_counter, bool break_at_compile, int comp_level) {
  VM_ENTRY_MARK;

  CompilerThread* current_thread = (CompilerThread*)THREAD;
  current_thread->set_data(NULL);
  current_thread->set_env(this);
  _recorder = NULL;
  _failure_reason = NULL;
  _compilable = MethodCompilable;
  _method_registered = false;
  _break_at_compile = break_at_compile;
  _comp_level = comp_level;
#ifndef PRODUCT
  _fire_out_of_memory_count = 0;
  if (firstEnv) {
    _fire_out_of_memory_count_delay = CIFireOOMAtDelay;
    firstEnv = false;
  }
#endif /* !PRODUCT */

  _jni_env = jni_env;
  _system_dictionary_modification_counter = system_dictionary_modification_counter;
  _num_inlined_bytecodes = 0;
  // Note:  This value is null if the thread is not a CompilerThread.
  // This can happen when we are called from compiler2_init().
  _log = current_thread->log();  // faster access to this pointer
  
  // Temporary buffer for creating symbols and such.
  _name_buffer = NULL;
  _name_buffer_len = 0;

  _arena   = &_ciEnv_arena;
  _factory = new (_arena) ciObjectFactory(_arena, 128);

  // Preload commonly referenced system ciObjects.

  // During VM initialization, these instances have not yet been created.
  // Assertions ensure that these instances are not accessed before
  // their initialization.

  if (Universe::is_fully_initialized()) {
    oop o = Universe::null_ptr_exception_instance();
    assert(o != NULL, "should have been initialized");
    _NullPointerException_instance = get_object(o)->as_instance();
    o = Universe::arithmetic_exception_instance();
    assert(o != NULL, "should have been initialized");
    _ArithmeticException_instance = get_object(o)->as_instance();
  } else {
    _NullPointerException_instance = NULL;
    _ArithmeticException_instance = NULL;
  }
  _ArrayIndexOutOfBoundsException_instance = NULL;
  _ArrayStoreException_instance = NULL;
  _ClassCastException_instance = NULL;
}

ciEnv::~ciEnv() {
}

// ------------------------------------------------------------------
// helper for lazy exception creation
ciInstance* ciEnv::get_or_create_exception(jobject& handle, symbolHandle name) {
  VM_ENTRY_MARK;
  if (handle == NULL) {
    // Cf. universe.cpp, creation of Universe::_null_ptr_exception_instance.
    klassOop k = SystemDictionary::resolve_or_fail(name, NULL, NULL, true, THREAD);
    jobject objh = NULL;
    if (!HAS_PENDING_EXCEPTION) {
      oop obj = instanceKlass::cast(k)->allocate_permanent_instance(THREAD);
      if (!HAS_PENDING_EXCEPTION)
        objh = JNIHandles::make_global(obj, false);
    }
    if (HAS_PENDING_EXCEPTION) {
      CLEAR_PENDING_EXCEPTION;
    } else {
      handle = objh;
    }
  }
  oop obj = JNIHandles::resolve(handle);
  return obj == NULL? NULL: get_object(obj)->as_instance();
}

// ------------------------------------------------------------------
// ciEnv::ArrayIndexOutOfBoundsException_instance, etc.
ciInstance* ciEnv::ArrayIndexOutOfBoundsException_instance() {
  if (_ArrayIndexOutOfBoundsException_instance == NULL) {
    _ArrayIndexOutOfBoundsException_instance
          = get_or_create_exception(_ArrayIndexOutOfBoundsException_handle,
          vmSymbolHandles::java_lang_ArrayIndexOutOfBoundsException());
  }
  return _ArrayIndexOutOfBoundsException_instance;
}
ciInstance* ciEnv::ArrayStoreException_instance() {
  if (_ArrayStoreException_instance == NULL) {
    _ArrayStoreException_instance
          = get_or_create_exception(_ArrayStoreException_handle,
          vmSymbolHandles::java_lang_ArrayStoreException());
  }
  return _ArrayStoreException_instance;
}
ciInstance* ciEnv::ClassCastException_instance() {
  if (_ClassCastException_instance == NULL) {
    _ClassCastException_instance
          = get_or_create_exception(_ClassCastException_handle,
          vmSymbolHandles::java_lang_ClassCastException());
  }
  return _ClassCastException_instance;
}

// ------------------------------------------------------------------
// ciEnv::get_method_from_handle
ciMethod* ciEnv::get_method_from_handle(jobject method) {
  VM_ENTRY_MARK;
  return get_object(JNIHandles::resolve(method))->as_method();
}

// ------------------------------------------------------------------
// ciEnv::make_array
ciArray* ciEnv::make_array(GrowableArray<ciObject*>* objects) {
  VM_ENTRY_MARK;
  int length = objects->length();
  objArrayOop a = oopFactory::new_system_objArray(length, THREAD);
#ifndef PRODUCT
  maybe_fire_out_of_memory(THREAD);
#endif /* !PRODUCT */
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    record_out_of_memory_failure();
    return NULL;
  }
  for (int i = 0; i < length; i++) {
    a->obj_at_put(i, objects->at(i)->get_oop());
  }
  assert(a->is_perm(), "");
  return get_object(a)->as_array();
}


// ------------------------------------------------------------------
// ciEnv::array_element_offset_in_bytes
int ciEnv::array_element_offset_in_bytes(ciArray* a_h, ciObject* o_h) {
  VM_ENTRY_MARK;
  objArrayOop a = (objArrayOop)a_h->get_oop();
  assert(a->is_objArray(), "");
  int length = a->length();
  oop o = o_h->get_oop();
  for (int i = 0; i < length; i++) {
    if (a->obj_at(i) == o)  return i;
  }
  return -1;
}


// ------------------------------------------------------------------
// ciEnv::check_klass_accessiblity
//
// Note: the logic of this method should mirror the logic of
// constantPoolOopDesc::verify_constant_pool_resolve.
bool ciEnv::check_klass_accessibility(ciKlass* accessing_klass,
				      klassOop resolved_klass) {
  if (accessing_klass == NULL || !accessing_klass->is_loaded()) {
    return true;
  }
  if (accessing_klass->is_obj_array()) {
    accessing_klass = accessing_klass->as_obj_array_klass()->base_element_klass();
  }
  if (!accessing_klass->is_instance_klass()) {
    return true;
  }

  if (resolved_klass->klass_part()->oop_is_objArray()) {
    // Find the element klass, if this is an array.
    resolved_klass = objArrayKlass::cast(resolved_klass)->bottom_klass();
  }
  if (resolved_klass->klass_part()->oop_is_instance()) {
    return Reflection::verify_class_access(accessing_klass->get_klassOop(),
					   resolved_klass,
					   true);
  }
  return true;
}

// ------------------------------------------------------------------
// ciEnv::get_klass_by_name_impl
ciKlass* ciEnv::get_klass_by_name_impl(ciKlass* accessing_klass,
                                       ciSymbol* name,
                                       bool require_local) {
  ASSERT_IN_VM;
  EXCEPTION_CONTEXT;

  // Now we need to check the SystemDictionary
  symbolHandle sym(THREAD, name->get_symbolOop());
  if (sym->byte_at(0) == 'L' &&
    sym->byte_at(sym->utf8_length()-1) == ';') {
    // This is a name from a signature.  Strip off the trimmings.
    sym = oopFactory::new_symbol_handle(sym->as_utf8()+1,
                                        sym->utf8_length()-2,
                                        KILL_COMPILE_ON_FATAL_(_unloaded_ciinstance_klass));
    name = get_object(sym())->as_symbol();
  }

  // Check for prior unloaded klass.  The SystemDictionary's answers
  // can vary over time but the compiler needs consistency.
  ciKlass* unloaded_klass = check_get_unloaded_klass(accessing_klass, name);
  if (unloaded_klass != NULL) {
    if (require_local)  return NULL;
    return unloaded_klass;
  }

  Handle loader(THREAD, (oop)NULL);
  Handle domain(THREAD, (oop)NULL);
  if (accessing_klass != NULL) {
    loader = Handle(THREAD, accessing_klass->loader());
    domain = Handle(THREAD, accessing_klass->protection_domain());
  }

  klassOop found_klass;
  if (!require_local) {
    found_klass =
      SystemDictionary::find_constrained_instance_or_array_klass(sym, loader,
                                                                 KILL_COMPILE_ON_FATAL_(_unloaded_ciinstance_klass));
  } else {
    found_klass =
      SystemDictionary::find_instance_or_array_klass(sym, loader, domain,
                                                     KILL_COMPILE_ON_FATAL_(_unloaded_ciinstance_klass));
  }

  if (found_klass != NULL) {
    // Found it.  Build a CI handle.
    return get_object(found_klass)->as_klass();
  }

  // If we fail to find an array klass, look again for its element type.
  // The element type may be available either locally or via constraints.
  // In either case, if we can find the element type in the system dictionary,
  // we must build an array type around it.  The CI requires array klasses
  // to be loaded if their element klasses are loaded, except when memory
  // is exhausted.
  if (sym->byte_at(0) == '[' &&
      (sym->byte_at(1) == '[' || sym->byte_at(1) == 'L')) {
    // We have an unloaded array.
    // Build it on the fly if the element class exists.
    symbolOop elem_sym = oopFactory::new_symbol(sym->as_utf8()+1, 
                                                sym->utf8_length()-1, 
                                                KILL_COMPILE_ON_FATAL_(_unloaded_ciinstance_klass));
    // Get element ciKlass recursively.
    ciKlass* elem_klass =
      get_klass_by_name_impl(accessing_klass,
                             get_object(elem_sym)->as_symbol(),
                             require_local);
    if (elem_klass != NULL && elem_klass->is_loaded()) {
      // Now make an array for it
      return ciObjArrayKlass::make_impl(elem_klass);
    }
  }

  if (require_local)  return NULL;
  // Not yet loaded into the VM, or not governed by loader constraints.
  // Make a CI representative for it.
  return get_unloaded_klass(accessing_klass, name);
}

// ------------------------------------------------------------------
// ciEnv::get_klass_by_name
ciKlass* ciEnv::get_klass_by_name(ciKlass* accessing_klass,
                                  ciSymbol* klass_name,
                                  bool require_local) {
  GUARDED_VM_ENTRY(return get_klass_by_name_impl(accessing_klass,
                                                 klass_name,
                                                 require_local);)
}

// ------------------------------------------------------------------
// ciEnv::get_klass_by_index_impl
//
// Implementation of get_klass_by_index.
ciKlass* ciEnv::get_klass_by_index_impl(ciInstanceKlass* accessor,
                                        int index,
                                        bool& is_accessible) {
  assert(accessor->get_instanceKlass()->is_linked(), "must be linked before accessing constant pool");
  EXCEPTION_CONTEXT;
  constantPoolHandle cpool(THREAD, accessor->get_instanceKlass()->constants());
  klassOop klass = constantPoolOopDesc::klass_at_if_loaded(cpool, index);
  symbolHandle klass_name;
  if (klass == NULL) {
    // The klass has not been inserted into the constant pool.
    // Try to look it up by name.
    {
      // We have to lock the cpool to keep the oop from being resolved
      // while we are accessing it.
      ObjectLocker ol(cpool, THREAD);
      
      constantTag tag = cpool->tag_at(index);
      if (tag.is_klass()) {
	// The klass has been inserted into the constant pool
	// very recently.
	klass = cpool->resolved_klass_at(index);
      } else if (tag.is_symbol()) {
	klass_name = symbolHandle(THREAD, cpool->symbol_at(index));
      } else {
	assert(cpool->tag_at(index).is_unresolved_klass(), "wrong tag");
	klass_name = symbolHandle(THREAD, cpool->unresolved_klass_at(index));
      }
    }
  }

  if (klass == NULL) {
    // Not found in constant pool.  Use the name to do the lookup.
    ciKlass* k = get_klass_by_name_impl(accessor,
                                        get_object(klass_name())->as_symbol(),
                                        false);
    // Calculate accessibility the hard way.
    if (!k->is_loaded()) {
      is_accessible = false;
    } else if (k->loader() != accessor->loader() &&
               get_klass_by_name_impl(accessor, k->name(), true) == NULL) {
      // Loaded only remotely.  Not linked yet.
      is_accessible = false;
    } else {
      // Linked locally, and we must also check public/private, etc.
      is_accessible = check_klass_accessibility(accessor, k->get_klassOop());
    }
    return k;
  }

  // Check for prior unloaded klass.  The SystemDictionary's answers
  // can vary over time but the compiler needs consistency.
  ciSymbol* name = get_object(klass->klass_part()->name())->as_symbol();
  ciKlass* unloaded_klass = check_get_unloaded_klass(accessor, name);
  if (unloaded_klass != NULL) {
    is_accessible = false;
    return unloaded_klass;
  }

  // It is known to be accessible, since it was found in the constant pool.
  is_accessible = true;
  return get_object(klass)->as_klass();
}

// ------------------------------------------------------------------
// ciEnv::get_klass_by_index
//
// Get a klass from the constant pool.
ciKlass* ciEnv::get_klass_by_index(ciInstanceKlass* accessor,
                                   int index,
                                   bool& is_accessible) {
  GUARDED_VM_ENTRY(return get_klass_by_index_impl(accessor, index, is_accessible);)
}

// ------------------------------------------------------------------
// ciEnv::get_constant_by_index_impl
//
// Implementation of get_constant_by_index().
ciConstant ciEnv::get_constant_by_index_impl(ciInstanceKlass* accessor,
					     int index) {
  EXCEPTION_CONTEXT;
  instanceKlass* ik_accessor = accessor->get_instanceKlass();
  assert(ik_accessor->is_linked(), "must be linked before accessing constant pool");
  constantPoolOop cpool = ik_accessor->constants();
  constantTag tag = cpool->tag_at(index);
  if (tag.is_int()) {
    return ciConstant(T_INT, (jint)cpool->int_at(index));
  } else if (tag.is_long()) {
    return ciConstant((jlong)cpool->long_at(index));
  } else if (tag.is_float()) {
    return ciConstant((jfloat)cpool->float_at(index));
  } else if (tag.is_double()) {
    return ciConstant((jdouble)cpool->double_at(index));
  } else if (tag.is_string() || tag.is_unresolved_string()) {
    oop string = cpool->string_at(index, THREAD);
#ifndef PRODUCT
    maybe_fire_out_of_memory(THREAD);
#endif /* !PRODUCT */
    if (HAS_PENDING_EXCEPTION) {
      CLEAR_PENDING_EXCEPTION;
      record_out_of_memory_failure();
      return ciConstant();
    }
    ciObject* constant = get_object(string);
    assert (constant->is_instance(), "must be an instance, or not? ");
    return ciConstant(T_OBJECT, constant);
  } else if (tag.is_klass() || tag.is_unresolved_klass()) {
    // 4881222: allow ldc to take a class type
    bool ignore;
    ciKlass* klass = get_klass_by_index_impl(accessor, index, ignore);
#ifndef PRODUCT
    maybe_fire_out_of_memory(THREAD);
#endif /* !PRODUCT */
    if (HAS_PENDING_EXCEPTION) {
      CLEAR_PENDING_EXCEPTION;
      record_out_of_memory_failure();
      return ciConstant();
    }
    assert (klass->is_instance_klass() || klass->is_array_klass(), 
            "must be an instance or array klass ");
    return ciConstant(T_OBJECT, klass);
  } else {
    ShouldNotReachHere();
    return ciConstant();
  }
}

// ------------------------------------------------------------------
// ciEnv::is_unresolved_string_impl
//
// Implementation of is_unresolved_string().
bool ciEnv::is_unresolved_string_impl(instanceKlass* accessor, int index) const {
  EXCEPTION_CONTEXT;
  assert(accessor->is_linked(), "must be linked before accessing constant pool");
  constantPoolOop cpool = accessor->constants();
  constantTag tag = cpool->tag_at(index);
  return tag.is_unresolved_string();
}

// ------------------------------------------------------------------
// ciEnv::is_unresolved_klass_impl
//
// Implementation of is_unresolved_klass().
bool ciEnv::is_unresolved_klass_impl(instanceKlass* accessor, int index) const {
  EXCEPTION_CONTEXT;
  assert(accessor->is_linked(), "must be linked before accessing constant pool");
  constantPoolOop cpool = accessor->constants();
  constantTag tag = cpool->tag_at(index);
  return tag.is_unresolved_klass();
}

// ------------------------------------------------------------------
// ciEnv::get_constant_by_index
//
// Pull a constant out of the constant pool.  How appropriate.
//
// Implementation note: this query is currently in no way cached.
ciConstant ciEnv::get_constant_by_index(ciInstanceKlass* accessor,
					int index) {
  GUARDED_VM_ENTRY(return get_constant_by_index_impl(accessor, index); )
}

// ------------------------------------------------------------------
// ciEnv::is_unresolved_string
//
// Check constant pool
//
// Implementation note: this query is currently in no way cached.
bool ciEnv::is_unresolved_string(ciInstanceKlass* accessor,
					int index) const {
  GUARDED_VM_ENTRY(return is_unresolved_string_impl(accessor->get_instanceKlass(), index); )
}

// ------------------------------------------------------------------
// ciEnv::is_unresolved_klass
//
// Check constant pool
//
// Implementation note: this query is currently in no way cached.
bool ciEnv::is_unresolved_klass(ciInstanceKlass* accessor,
					int index) const {
  GUARDED_VM_ENTRY(return is_unresolved_klass_impl(accessor->get_instanceKlass(), index); )
}

// ------------------------------------------------------------------
// ciEnv::get_field_by_index_impl
//
// Implementation of get_field_by_index.
//
// Implementation note: the results of field lookups are cached
// in the accessor klass.
ciField* ciEnv::get_field_by_index_impl(ciInstanceKlass* accessor,
					int index) {
  ciConstantPoolCache* cache = accessor->field_cache();
  if (cache == NULL) {
    ciField* field = new (arena()) ciField(accessor, index);
    return field;
  } else {
    ciField* field = (ciField*)cache->get(index);
    if (field == NULL) {
      field = new (arena()) ciField(accessor, index);
      cache->insert(index, field);
    }
    return field;
  }
}

// ------------------------------------------------------------------
// ciEnv::get_field_by_index
//
// Get a field by index from a klass's constant pool.
ciField* ciEnv::get_field_by_index(ciInstanceKlass* accessor,
				   int index) {
  GUARDED_VM_ENTRY(return get_field_by_index_impl(accessor, index);)
}

// ------------------------------------------------------------------
// ciEnv::lookup_method
//
// Perform an appropriate method lookup based on accessor, holder,
// name, signature, and bytecode.
methodOop ciEnv::lookup_method(instanceKlass*  accessor,
			       instanceKlass*  holder,
			       symbolOop       name,
			       symbolOop       sig,
			       Bytecodes::Code bc) {
  EXCEPTION_CONTEXT;
  KlassHandle h_accessor(THREAD, accessor);
  KlassHandle h_holder(THREAD, holder);
  symbolHandle h_name(THREAD, name);
  symbolHandle h_sig(THREAD, sig);
  LinkResolver::check_klass_accessability(h_accessor, h_holder, KILL_COMPILE_ON_FATAL_(NULL));
  methodHandle dest_method;
  switch (bc) {
  case Bytecodes::_invokestatic:
    dest_method = 
      LinkResolver::resolve_static_call_or_null(h_holder, h_name, h_sig, h_accessor); 
    break;
  case Bytecodes::_invokespecial:
    dest_method = 
      LinkResolver::resolve_special_call_or_null(h_holder, h_name, h_sig, h_accessor); 
    break;
  case Bytecodes::_invokeinterface: 
    dest_method =
      LinkResolver::linktime_resolve_interface_method_or_null(h_holder, h_name, h_sig,
							      h_accessor, true);
    break;
  case Bytecodes::_invokevirtual:
    dest_method = 
      LinkResolver::linktime_resolve_virtual_method_or_null(h_holder, h_name, h_sig,
							    h_accessor, true);
    break;
  default: ShouldNotReachHere();
  }

  return dest_method();
}


// ------------------------------------------------------------------
// ciEnv::get_method_by_index_impl
ciMethod* ciEnv::get_method_by_index_impl(ciInstanceKlass* accessor,
                                     int index, Bytecodes::Code bc) {
  // Get the method's declared holder.
                       
  assert(accessor->get_instanceKlass()->is_linked(), "must be linked before accessing constant pool");
  constantPoolHandle cpool = accessor->get_instanceKlass()->constants();
  int holder_index = cpool->klass_ref_index_at(index);
  bool holder_is_accessible;
  ciKlass* holder = get_klass_by_index_impl(accessor, holder_index, holder_is_accessible);
  ciInstanceKlass* declared_holder = get_instance_klass_for_declared_method_holder(holder);

  // Get the method's name and signature.
  int nt_index = cpool->name_and_type_ref_index_at(index);
  int sig_index = cpool->signature_ref_index_at(nt_index);
  symbolOop name_sym = cpool->name_ref_at(index);
  symbolOop sig_sym = cpool->symbol_at(sig_index);

  if (holder_is_accessible) { // Our declared holder is loaded.
    instanceKlass* lookup = declared_holder->get_instanceKlass();
    methodOop m = lookup_method(accessor->get_instanceKlass(), lookup, name_sym, sig_sym, bc);
    if (m != NULL) {
      // We found the method.
      return get_object(m)->as_method();
    }
  }

  // Either the declared holder was not loaded, or the method could
  // not be found.  Create a dummy ciMethod to represent the failed
  // lookup.

  return get_unloaded_method(declared_holder,
                             get_object(name_sym)->as_symbol(),
                             get_object(sig_sym)->as_symbol());
}


// ------------------------------------------------------------------
// ciEnv::get_instance_klass_for_declared_method_holder
ciInstanceKlass* ciEnv::get_instance_klass_for_declared_method_holder(ciKlass* method_holder) {
  // For the case of <array>.clone(), the method holder can be a ciArrayKlass
  // instead of a ciInstanceKlass.  For that case simply pretend that the
  // declared holder is Object.clone since that's where the call will bottom out.
  // A more correct fix would trickle out through many interfaces in CI,
  // requiring ciInstanceKlass* to become ciKlass* and many more places would
  // require checks to make sure the expected type was found.  Given that this
  // only occurs for clone() the more extensive fix seems like overkill so
  // instead we simply smear the array type into Object.
  if (method_holder->is_instance_klass()) {
    return method_holder->as_instance_klass();
  } else if (method_holder->is_array_klass()) {
    return current()->Object_klass();
  } else {
    ShouldNotReachHere();
  }
  return NULL;
}
  



// ------------------------------------------------------------------
// ciEnv::get_method_by_index
ciMethod* ciEnv::get_method_by_index(ciInstanceKlass* accessor,
                                     int index, Bytecodes::Code bc) {
  GUARDED_VM_ENTRY(return get_method_by_index_impl(accessor, index, bc);)
}

// ------------------------------------------------------------------
// ciEnv::name_buffer
char *ciEnv::name_buffer(int req_len) {
  if (_name_buffer_len < req_len) {
    if (_name_buffer == NULL) {
      _name_buffer = (char*)arena()->Amalloc(sizeof(char)*req_len);
      _name_buffer_len = req_len;
    } else {
      _name_buffer =
	(char*)arena()->Arealloc(_name_buffer, _name_buffer_len, req_len);
      _name_buffer_len = req_len;
    }
  }
  return _name_buffer;
}

// ------------------------------------------------------------------
// ciEnv::is_in_vm
bool ciEnv::is_in_vm() {
  return JavaThread::current()->thread_state() == _thread_in_vm;
} 

// ------------------------------------------------------------------
// ciEnv::call_has_multiple_targets
int ciEnv::call_has_multiple_targets(instanceKlass* current,
                              symbolHandle   method_name,
                              symbolHandle   method_sig,
                              bool&           found) {
  methodOop target = current->find_method(method_name(), method_sig());
  if (target != NULL && !target->is_abstract()) {
    // We have found an implementation.
    if (found) {
      // There is more than one concrete target for the virtual call.
      return true;
    } else {
      // We have found one concrete target for the virtual call.
      found = true;
    }
    
    if (target->is_final_method() || target->is_private()) {
      // This method cannot be overridden, so we do not need to check
      // our subclasses.
      return false;
    }
  }
  
  // Check recursively.
  for (Klass* s = current->subklass(); s != NULL; s = s->next_sibling()) {
    if (!s->is_interface() && s->oop_is_instance()) {
      if (call_has_multiple_targets((instanceKlass*)s, method_name, method_sig, found)) {
        return true;
      }
    }
  }
  return false;
}

// ------------------------------------------------------------------
// ciEnv::is_dependence_violated
bool ciEnv::is_dependence_violated(klassOop k, methodOop m) {
  if (m != NULL) {
    // check if it has been redefined with JVMTI RedefineClasses
    if (m->is_old_version()) return true;
    if (m->number_of_breakpoints() > 0) return true;
  }

  if (k == NULL) {
    // these kind of dependencies are generated only by method
    // replacement code and are checked above
    assert(m != NULL, "garbage dependency");
    return false;
  }

  if (Klass::cast(k)->is_interface() &&
      instanceKlass::cast(k)->nof_implementors() > 1) {
    return true;
  }
  
  instanceKlass* ik = instanceKlass::cast(k);
  if (m == NULL) {
    // Subtype dependence
    return (ik->subklass() != NULL);
  } else {
    Thread *current_thread = Thread::current();
    symbolHandle h_name(current_thread, m->name());
    symbolHandle h_sig (current_thread, m->signature());
    bool found = false;
    return call_has_multiple_targets(ik, h_name, h_sig, found);
  }
  return false;
}

bool ciEnv::system_dictionary_modification_counter_changed() {
  return _system_dictionary_modification_counter != SystemDictionary::number_of_modifications();
}

// ------------------------------------------------------------------
// ciEnv::check_for_system_dictionary_modification
// Check for changes to the system dictionary during compilation
// class loads, evolution, breakpoints
void ciEnv::check_for_system_dictionary_modification(ciMethod* target) {
  if (failing())  return;  // no need for further checks

  // We need this lock to access SystemDictionary::number_of_adds().
  MutexLocker ml(Compile_lock);


  if (system_dictionary_modification_counter_changed()) {

    if (target->get_methodOop()->number_of_breakpoints() > 0) {
      if (log() != NULL) {
	log()->elem("method has breakpoint method='%d'",
		    log()->identify(get_object(target->get_methodOop())));
      }
      record_failure("method has breakpoint");
      return;
    }
    int first = recorder()->first_dependent();
    int limit = first + recorder()->number_of_dependents();
    OopRecorder* oop_recorder= recorder()->oop_recorder();
    for (int index = first; index < limit; index += 2) {
      assert(index != 0, "invalidated_index assumes index is not zero");
      klassOop klass = klassOop(JNIHandles::resolve(oop_recorder->handle_at(index-1)));
      // If null, it is kind of rigid dependency, like a static inlined method, that can't
      // be violated by concurrent class loading. It is important only in case of evolution.
      debug_only(if (klass == NULL) { assert(HotSwap, "NULL klass in dependency should only come from HotSwap"); });
      assert((klass == NULL) || klass->is_klass(), "type check");
      methodOop method = methodOop(JNIHandles::resolve(oop_recorder->handle_at(index)));
      assert(!method || method->is_method(), "type check");
      if (is_dependence_violated(klass, method)) {
        if (log() != NULL) {
          log()->elem("dependence_violated klass='%d' method='%d'",
                      log()->identify(get_object(klass)),
                      log()->identify(get_object(method)));
        }
        record_failure("concurrent class loading");
        return;
      }
    }	
  }
}

// ------------------------------------------------------------------
// ciEnv::register_method
void ciEnv::register_method(ciMethod* target,
			    int entry_bci,
			    int iep_offset,
			    int ep_offset,
			    int vep_offset,
			    int code_offset,
			    int osr_offset,
			    CodeBuffer* code_buffer,
			    int frame_words,
			    OopMapSet* oop_map_set,
			    ExceptionHandlerTable* handler_table,
			    ImplicitExceptionTable* inc_table,
                            ExceptionRangeTable* exception_range_table,
                            AbstractCompiler* compiler,
                            bool has_debug_info,
                            bool has_unsafe_access) {
  VM_ENTRY_MARK;    
  nmethod* nm = NULL;
  {
    // To prevent compile queue updates.
    MutexLocker locker(MethodCompileQueue_lock, THREAD);

    if (log() != NULL) {
      // Log the dependencies which this compilation declares.
      int first = recorder()->first_dependent();
      int limit = first + recorder()->number_of_dependents();
      OopRecorder* oop_recorder = recorder()->oop_recorder();
      for (int index = first; index < limit; index += 2) {
        ciObject* x = get_object(JNIHandles::resolve(oop_recorder->handle_at(index-1)));
        ciObject* y = get_object(JNIHandles::resolve(oop_recorder->handle_at(index)));
        log()->elem("dependence klass='%d' method='%d'",
                    log()->identify(x), log()->identify(y));
      }
    }

    // Check for {class loads, evolution, breakpoints} during compilation
    check_for_system_dictionary_modification(target);

    if (failing()) {
      return;
    }

    methodHandle method(THREAD, target->get_methodOop());

    nm =  nmethod::new_nmethod(method,
                                      entry_bci,
				      iep_offset,
                                      ep_offset, vep_offset, code_offset, osr_offset,
                                      recorder(), code_buffer, 
                                      frame_words, oop_map_set, 
                                      handler_table, inc_table, exception_range_table,
                                      compiler);
  
    if (nm == NULL) {
      // The CodeCache is full.  Print out warning and disable compilation.
      record_failure("code cache is full");
      UseInterpreter = true;
      if (UseCompiler || AlwaysCompileLoopMethods ) {
#ifndef PRODUCT
        warning("CodeCache is full. Compiler has been disabled");
        if (CompileTheWorld || ExitOnFullCodeCache) {
          before_exit(JavaThread::current());
          exit_globals(); // will delete tty
          exit(CompileTheWorld ? 0 : 1);
        }
#endif
        UseCompiler               = false;    
        AlwaysCompileLoopMethods  = false;
      }
    } else {
      NOT_PRODUCT(nm->set_has_debug_info(has_debug_info); )
      nm->set_has_unsafe_access(has_unsafe_access);
      nm->set_compile_id(compile_id());
      if (entry_bci == InvocationEntryBci) {
        method->set_code(nm);
      } else {
        instanceKlass::cast(method->method_holder())->add_osr_nmethod(nm);
      }
    }
    // Record successful registration.
    _method_registered = true;
  }
  // JVMTI/JVMPI -- compiled method notification (must be done outside lock)
  post_compiled_method_load_event(nm);
}

// ------------------------------------------------------------------
// ciEnv::register_i2c_adapter
#ifdef COMPILER2
void ciEnv::register_i2c_adapter(ciMethod* target,
				 OopMapSet *oopmaps,
				 CodeBuffer* cb,
				 int frame_words) {
  VM_ENTRY_MARK;
  address code_begin;
  address code_end;
  {
    MutexLocker locker(AdapterCompileQueue_lock, THREAD);
    methodHandle method (THREAD, target->get_methodOop());
  
    AdapterInfo info(method, true);
    if (I2CAdapterGenerator::_cache->lookup(&info) != NULL) {
      return;                     // Already compiled by another thread
    }

    // Make the NMethod
    I2CAdapter* adapter = I2CAdapter::new_i2c_adapter( cb, 
						       oopmaps, 
						       frame_words);  
    assert(adapter != NULL && adapter->is_i2c_adapter(), "sanity check");
    adapter->set_prototypical_signature(method()->signature(), method()->is_static());
    I2CAdapterGenerator::_cache->insert(&info, adapter);  
    
    code_begin = adapter->instructions_begin();
    code_end = adapter->instructions_end();
  }
  // notify JVMTI profiler about this adapter
  if (JvmtiExport::should_post_dynamic_code_generated()) {
    JvmtiExport::post_dynamic_code_generated("I2CAdapter", code_begin, code_end); 
  }
}
#endif


// ------------------------------------------------------------------
// ciEnv::register_c2i_adapter
#ifdef COMPILER2
void ciEnv::register_c2i_adapter(ciMethod* target,
                                 OopMapSet* oopmaps,
                                 CodeBuffer* cb,
                                 int first_block_size,
                                 int frame_words) {
  VM_ENTRY_MARK;
  address code_begin;
  address code_end;
  {
    MutexLocker locker(AdapterCompileQueue_lock, THREAD);
    methodHandle method (THREAD, target->get_methodOop());

    AdapterInfo info(method, false);
    if (C2IAdapterGenerator::_cache->lookup(&info) != NULL) {
      return;
    }

    C2IAdapter* adapter = C2IAdapter::new_c2i_adapter(cb,
						      first_block_size,
						      oopmaps,
						      frame_words );  
    assert(adapter != NULL && adapter->is_c2i_adapter(), "sanity check");
  
    adapter->set_prototypical_signature(method->signature(), method->is_static());
    C2IAdapterGenerator::_cache->insert(&info, adapter);

    code_begin = adapter->instructions_begin();
    code_end = adapter->instructions_end();
  }

  // notify JVMTI profiler about this adapter
  if (JvmtiExport::should_post_dynamic_code_generated()) {
    JvmtiExport::post_dynamic_code_generated("C2IAdapter", code_begin, code_end); 
  }
}
#endif // COMPILER2


// ------------------------------------------------------------------
// ciEnv::post_compiled_method_load_event
// new method for install_code() path
// Transfer information from compilation to jvmpi
// NOTE: lineno_table must be from long-term storage
void ciEnv::post_compiled_method_load_event(nmethod *nm) {
  if (nm != NULL && jvmpi::is_event_enabled(JVMPI_EVENT_COMPILED_METHOD_LOAD)) {
    ResourceMark rm;
    compiled_method_t compiled_method;
    compiled_method.method    = nm->method();
    compiled_method.code_addr = nm->code_begin();
    compiled_method.code_size = nm->code_size();
    build_jvmpi_line_number_mapping(nm, &compiled_method);

    jvmpi::post_compiled_method_load_event(&compiled_method);
  }
  
  if (nm != NULL && JvmtiExport::should_post_compiled_method_load()) {
    JvmtiExport::post_compiled_method_load(nm);
  }
}


// JVMPI - support
class OffsetBciPair: public ResourceObj {
 private:
  jint _offset;
  jint _bci;
 public:
  OffsetBciPair(): _offset(-1), _bci(-1) {}
  void set(jint offset, jint bci) { _offset = offset; _bci = bci; }
  jint bci() const  { return _bci; }
  jint offset() const { return _offset; }
};

// ------------------------------------------------------------------
// Helper function for post_compiled_method_load_event
void ciEnv::build_jvmpi_line_number_mapping(nmethod *nm, compiled_method_t *cm) {
  // Generate line numbers using PcDesc and ScopeDesc info
  methodHandle mh(nm->method());
  if( mh->is_native() || !mh->has_linenumber_table() ) {  // No bci info for safepoints in "native methods"
    cm->lineno_table_len = 0;
    cm->lineno_table     = NULL;
  } else {
    enum { unused = -1 };
    int size = mh->code_size();   // byte code length

    // Build mapping from BCIs to source lines
    GrowableArray<jint> *bci_to_line  = new GrowableArray<jint>(size, size, unused);
    int index = 0;
    CompressedLineNumberReadStream stream(mh->compressed_linenumber_table());
    while( stream.read_pair() ) {
      bci_to_line->at_put_grow(stream.bci(), stream.line(), unused);
    }
    // Define a line for remaining BCIs given the assumption that 
    // the line mapping provided for a BCI is valid for all following 
    // BCIs until another line mapping is provided.
    int previous_line = unused;
    for( index = 0; index < size; ++index ) {
      int current_line = bci_to_line->at(index);
      if( current_line == unused ) {
        bci_to_line->at_put_grow(index, previous_line);
      } else {
        previous_line = current_line;
      }
    }

    // Compute displacement between PcDesc offset and nmethod code
    int ep_offset = (nm->code_begin() - nm->instructions_begin());

    // Collect information for pairs <offset, bci> from PcDesc and ScopeDesc.
    // Count PcDescs
    PcDesc  *pcd = NULL;
    int pcds_in_method = 0;
    for( pcd = nm->scopes_pcs_begin(); pcd < nm->scopes_pcs_end(); ++pcd ) {
      ++pcds_in_method;
    }
    // Fill array with OffsetBciPairs
    int offset_to_bci_length = 0;
    GrowableArray<OffsetBciPair*>* offset_to_bci = new GrowableArray<OffsetBciPair*>(pcds_in_method, pcds_in_method, NULL);
    address scopes_data = nm->scopes_data_begin();
    for( pcd = nm->scopes_pcs_begin(); pcd < nm->scopes_pcs_end(); ++pcd ) {
      ScopeDesc sc0(nm, pcd->scope_decode_offset());
      ScopeDesc *sd  = &sc0;
      while( !sd->is_top() ) { sd = sd->sender(); }

      int bci = sd->bci() ;
      if( bci != InvocationEntryBci ) {
        int offset = pcd->pc_offset() - ep_offset;
        OffsetBciPair* offset_bci_pair = new OffsetBciPair();
        offset_bci_pair->set( offset, bci );
        offset_to_bci->at_put_grow(offset_to_bci_length, offset_bci_pair);
        ++offset_to_bci_length;
      }
    }

    // Combine the mappings, <bci, line> and <offset, bci> into <line, offset>
    JVMPI_Lineno* line_to_pcOffset = NEW_RESOURCE_ARRAY(JVMPI_Lineno, offset_to_bci_length);
    index = 0;
    for (int i = 0; i < offset_to_bci_length; i++) {
      int offset = offset_to_bci->at(i)->offset();
      int bci    = offset_to_bci->at(i)->bci();
      int line   = bci_to_line->at(bci);
      if( line != unused ) {
        line_to_pcOffset[index].offset = offset;
        line_to_pcOffset[index].lineno = line;
        ++index;
      }
    }

    cm->lineno_table_len = offset_to_bci_length;
    cm->lineno_table     = line_to_pcOffset;
  }
  assert( true, "debug breakpoint");
}


// ------------------------------------------------------------------
// ciEnv::find_system_klass
ciKlass* ciEnv::find_system_klass(ciSymbol* klass_name) {
  VM_ENTRY_MARK;
  return get_klass_by_name_impl(NULL, klass_name, false);
}

// ------------------------------------------------------------------
// ciEnv::compile_id
uint ciEnv::compile_id() {
  return ((CompilerThread*)JavaThread::current())->task()->compile_id();
}

// ------------------------------------------------------------------
// ciEnv::notice_inlined_method()
void ciEnv::notice_inlined_method(ciMethod* method) {
  _num_inlined_bytecodes += method->code_size();
}

// ------------------------------------------------------------------
// ciEnv::num_inlined_bytecodes()
int ciEnv::num_inlined_bytecodes() const {
  return _num_inlined_bytecodes;
}

// ------------------------------------------------------------------
// ciEnv::record_failure()
void ciEnv::record_failure(const char* reason) {
  if (log() != NULL) {
    log()->elem("failure reason='%s'", reason);
  }
  if (_failure_reason == NULL) {
    // Record the first failure reason.
    _failure_reason = reason;
  }
}

// ------------------------------------------------------------------
// ciEnv::record_method_not_compilable()
void ciEnv::record_method_not_compilable(const char* reason, bool all_tiers) {
  int new_compilable = 
    all_tiers ? MethodCompilable_never : MethodCompilable_not_at_tier ;

  // Only note transitions to a worse state
  if (new_compilable > _compilable) {
    if (log() != NULL) {
      if (all_tiers) {
        log()->elem("method_not_compilable");
      } else {
        log()->elem("method_not_compilable_at_tier");
      }
    }
    _compilable = new_compilable;

    // Reset failure reason; this one is more important.
    _failure_reason = NULL;
    record_failure(reason);
  }
}

// ------------------------------------------------------------------
// ciEnv::record_out_of_memory_failure()
void ciEnv::record_out_of_memory_failure() {
  // If memory is low, we stop compiling methods.
  record_method_not_compilable("out of memory");
}

#ifndef PRODUCT
// ------------------------------------------------------------------
// ciEnv::maybe_fire_out_of_memory()
void ciEnv::maybe_fire_out_of_memory(TRAPS) {
  if (!HAS_PENDING_EXCEPTION && should_fire_out_of_memory()) {
    THROW_OOP(Universe::out_of_memory_error_perm_gen());
  }
}

// ------------------------------------------------------------------
// ciEnv::should_fire_out_of_memory()
bool ciEnv::should_fire_out_of_memory() {
  if (CIFireOOMAt <= 0) {
    return false;
  }
  if (_fire_out_of_memory_count_delay > 0) {
    --_fire_out_of_memory_count_delay;
    return false;
  }
  return (++_fire_out_of_memory_count >= CIFireOOMAt);
}
#endif
