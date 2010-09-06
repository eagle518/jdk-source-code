#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmtiRedefineClasses.cpp	1.21 04/06/08 11:08:39 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmtiRedefineClasses.cpp.incl"


objArrayOop VM_RedefineClasses::_old_methods = NULL;
objArrayOop VM_RedefineClasses::_new_methods = NULL;  
klassOop VM_RedefineClasses::_evolving_koop = NULL;
constantPoolOop VM_RedefineClasses::_old_constants = NULL;


VM_RedefineClasses::VM_RedefineClasses(jint class_count, const jvmtiClassDefinition *class_defs) {
  _class_count = class_count;
  _class_defs = class_defs;
  _res = JVMTI_ERROR_NONE;
}

bool VM_RedefineClasses::doit_prologue() {
  if (_class_count == 0) {
    _res = JVMTI_ERROR_NONE;
    return false;
  }
  if (_class_defs == NULL) {
    _res = JVMTI_ERROR_NULL_POINTER;
    return false;
  }
  for (int i = 0; i < _class_count; i++) {
    if (_class_defs[i].klass == NULL) {
      _res = JVMTI_ERROR_INVALID_CLASS;
      return false;
    }
    if (_class_defs[i].class_byte_count == 0) {
      _res = JVMTI_ERROR_INVALID_CLASS_FORMAT;
      return false;
    }
    if (_class_defs[i].class_bytes == NULL) {
      _res = JVMTI_ERROR_NULL_POINTER;
      return false;
    }
  }

  // We first load new class versions in the prologue, because somewhere down the
  // call chain it is required that the current thread is a Java thread.
  _res = load_new_class_versions(Thread::current());
  if (_res != JVMTI_ERROR_NONE) {
    // Free os::malloc allocated memory in load_new_class_version.
    os::free(_k_h_new);
    return false;
  }
  return true;
}

void VM_RedefineClasses::doit() {
  Thread *thread = Thread::current();
  for (int i = 0; i < _class_count; i++) {
    redefine_single_class(_class_defs[i].klass, _k_h_new[i], thread);
  }
  // Disable any dependent concurrent compilations
  SystemDictionary::notice_modification();
}

void VM_RedefineClasses::doit_epilogue() {
  // Free os::malloc allocated memory.
  // The memory allocated in redefine will be free'ed in next VM operation.
  os::free(_k_h_new);
}


jvmtiError VM_RedefineClasses::compare_class_versions(instanceKlassHandle k_h_old, instanceKlassHandle k_h_new) {
  int i;

  // Check superclasses, or rather their names, since superclasses themselves can be
  // requested to replace. 
  // Check for NULL superclass first since this might be java.lang.Object
  if (k_h_old->super() != k_h_new->super() && 
      (k_h_old->super() == NULL || k_h_new->super() == NULL ||
       Klass::cast(k_h_old->super())->name() != Klass::cast(k_h_new->super())->name())) {
    return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED;
  }

  // Check if the number, names and order of directly implemented interfaces are the same.
  // I think in principle we should just check if the sets of names of directly implemented
  // interfaces are the same, i.e. the order of declaration (which, however, if changed in the
  // .java file, also changes in .class file) should not matter. However, comparing sets is
  // technically a bit more difficult, and, more importantly, I am not sure at present that the
  // order of interfaces does not matter on the implementation level, i.e. that the VM does not
  // rely on it somewhere.
  objArrayOop k_interfaces = k_h_old->local_interfaces();
  objArrayOop k_new_interfaces = k_h_new->local_interfaces();
  int n_intfs = k_interfaces->length();
  if (n_intfs != k_new_interfaces->length()) {
    return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED;
  }
  for (i = 0; i < n_intfs; i++) {
    if (Klass::cast((klassOop) k_interfaces->obj_at(i))->name() !=
        Klass::cast((klassOop) k_new_interfaces->obj_at(i))->name()) {
      return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_HIERARCHY_CHANGED;
    }
  }

  // Check whether class is in the error init state.
  if (k_h_old->is_in_error_state()) {
    // TBD #5057930: special error code is needed in 1.6 
    return JVMTI_ERROR_INVALID_CLASS;
  }

  // Check whether class modifiers are the same.
  jushort old_flags = (jushort) k_h_old->access_flags().get_flags();
  jushort new_flags = (jushort) k_h_new->access_flags().get_flags();
  if (old_flags != new_flags) {
    return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_CLASS_MODIFIERS_CHANGED;
  }

  // Check if the number, names, types and order of fields declared in these classes
  // are the same.
  typeArrayOop k_old_fields = k_h_old->fields();
  typeArrayOop k_new_fields = k_h_new->fields();
  int n_fields = k_old_fields->length();
  if (n_fields != k_new_fields->length()) {
    return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED;
  }

  for (i = 0; i < n_fields; i += instanceKlass::next_offset) {
    // access
    old_flags = k_old_fields->ushort_at(i + instanceKlass::access_flags_offset);
    new_flags = k_new_fields->ushort_at(i + instanceKlass::access_flags_offset);
    if ((old_flags ^ new_flags) & JVM_RECOGNIZED_FIELD_MODIFIERS) {
      return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED;
    }
    // offset
    if (k_old_fields->short_at(i + instanceKlass::low_offset) != 
        k_new_fields->short_at(i + instanceKlass::low_offset) ||
        k_old_fields->short_at(i + instanceKlass::high_offset) != 
        k_new_fields->short_at(i + instanceKlass::high_offset)) {
      return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED;
    }
    // name and signature
    jshort name_index = k_old_fields->short_at(i + instanceKlass::name_index_offset);
    jshort sig_index = k_old_fields->short_at(i +instanceKlass::signature_index_offset);
    symbolOop name_sym1 = k_h_old->constants()->symbol_at(name_index);
    symbolOop sig_sym1 = k_h_old->constants()->symbol_at(sig_index);
    name_index = k_new_fields->short_at(i + instanceKlass::name_index_offset);
    sig_index = k_new_fields->short_at(i + instanceKlass::signature_index_offset);
    symbolOop name_sym2 = k_h_new->constants()->symbol_at(name_index);
    symbolOop sig_sym2 = k_h_new->constants()->symbol_at(sig_index);
    if (name_sym1 != name_sym2 || sig_sym1 != sig_sym2) {
      return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_SCHEMA_CHANGED;
    }
  }

  // Check if the number, names, signatures and order of methods declared in these classes
  // are the same.
  objArrayOop k_methods = k_h_old->methods();
  objArrayOop k_new_methods = k_h_new->methods();
  int n_methods = k_methods->length();
  if (n_methods < k_new_methods->length()) return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_ADDED;
  else if (n_methods > k_new_methods->length()) return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED;

  for (i = 0; i < n_methods; i++) {
    methodOop k_method = (methodOop) k_methods->obj_at(i);
    methodOop k_new_method = (methodOop) k_new_methods->obj_at(i);
    if (k_method->name() != k_new_method->name()) return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED;
    if (k_method->signature() != k_new_method->signature()) {
      return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_DELETED;
    }
    old_flags = (jushort) k_method->access_flags().get_flags();
    new_flags = (jushort) k_new_method->access_flags().get_flags();
    if (old_flags != new_flags) {
      return JVMTI_ERROR_UNSUPPORTED_REDEFINITION_METHOD_MODIFIERS_CHANGED;
    }      
  }

  return JVMTI_ERROR_NONE;
}


jvmtiError VM_RedefineClasses::load_new_class_versions(TRAPS) {
  // For consistency allocate memory using os::malloc wrapper.
  _k_h_new = (instanceKlassHandle *) os::malloc(sizeof(instanceKlassHandle) * _class_count);

  ResourceMark rm(THREAD);

  JvmtiThreadState *state = JavaThread::current()->jvmti_thread_state();
  assert(state != NULL, "JvmtiThreadState not initialized"); 
  for (int i = 0; i < _class_count; i++) {
    oop mirror = JNIHandles::resolve_non_null(_class_defs[i].klass);
    klassOop k_oop = java_lang_Class::as_klassOop(mirror);
    instanceKlassHandle k_h = instanceKlassHandle(THREAD, k_oop);
    symbolHandle k_name = symbolHandle(THREAD, k_h->name());

    ClassFileStream st((u1*) _class_defs[i].class_bytes,
      _class_defs[i].class_byte_count, (char *)"__VM_RedefineClasses__");

    // Parse the stream.
    Handle k_loader_h(THREAD, k_h->class_loader());
    Handle protection_domain;
    // Set redefined class handle in JvmtiThreadState class.
    // This redefined class is sent to agent event handler for class file
    // load hook event.
    state->set_class_being_redefined(&k_h);

    klassOop k = SystemDictionary::parse_stream(k_name, 
                                                k_loader_h, 
                                                protection_domain,
                                                &st,
                                                THREAD);
    // Clear class_being_redefined just to be sure.
    state->set_class_being_redefined(NULL);
                                     
    instanceKlassHandle k_h_new (THREAD, k);

    if (HAS_PENDING_EXCEPTION) {
      symbolOop ex_name = PENDING_EXCEPTION->klass()->klass_part()->name();
      CLEAR_PENDING_EXCEPTION;

      if (ex_name == vmSymbols::java_lang_UnsupportedClassVersionError()) {
        return JVMTI_ERROR_UNSUPPORTED_VERSION;
      } else if (ex_name == vmSymbols::java_lang_ClassFormatError()) {
        return JVMTI_ERROR_INVALID_CLASS_FORMAT;
      } else if (ex_name == vmSymbols::java_lang_ClassCircularityError()) {
        return JVMTI_ERROR_CIRCULAR_CLASS_DEFINITION;
      } else if (ex_name == vmSymbols::java_lang_NoClassDefFoundError()) {
        // The message will be "XXX (wrong name: YYY)"
        return JVMTI_ERROR_NAMES_DONT_MATCH;
      } else {  // Just in case more exceptions can be thrown..
        return JVMTI_ERROR_FAILS_VERIFICATION;
      }
    }

    // All its super classes should be linked to 
    // initialize the vtable.
    instanceKlassHandle super(THREAD, k_h_new->super());
    if (super.not_null() && !super->is_linked()) {
      super->link_class(THREAD);
      if (HAS_PENDING_EXCEPTION) {
        CLEAR_PENDING_EXCEPTION;
        return JVMTI_ERROR_INTERNAL;
      }
    }
    
    // See instanceKlass::link_klass_impl()
    { ObjectLocker ol(k_h_new, THREAD);
      Verifier::verify_byte_codes(k_h_new, THREAD);

      if (HAS_PENDING_EXCEPTION) {
        CLEAR_PENDING_EXCEPTION;
        return JVMTI_ERROR_FAILS_VERIFICATION;
      }
      Rewriter::rewrite(k_h_new, THREAD);
      if (HAS_PENDING_EXCEPTION) {
        CLEAR_PENDING_EXCEPTION;
        return JVMTI_ERROR_INTERNAL;
      }
    }

    jvmtiError res = compare_class_versions(k_h, k_h_new);
    if (res != JVMTI_ERROR_NONE) return res;
  
    _k_h_new[i] = k_h_new;
  }
  
  return JVMTI_ERROR_NONE;
}


// Field names and signatures are referenced via constantpool indexes. In the new class
// version, the layout of the constantpool can be different, so these indexes should be
// patched.
void VM_RedefineClasses::patch_indexes_for_fields(instanceKlassHandle k_h, instanceKlassHandle k_h_new) {
  typeArrayOop k_fields = k_h->fields();
  typeArrayOop k_new_fields = k_h_new->fields();
  int n_fields = k_fields->length();

  for (int i = 0; i < n_fields; i += instanceKlass::next_offset) {
    // name and signature
    k_fields->short_at_put(
      i + instanceKlass::name_index_offset,
      k_new_fields->short_at(i + instanceKlass::name_index_offset)
      );
    k_fields->short_at_put(
      i + instanceKlass::signature_index_offset,
      k_new_fields->short_at(i + instanceKlass::signature_index_offset)
      );
    k_fields->short_at_put(
      i + instanceKlass::generic_signature_offset,
      k_new_fields->short_at(i + instanceKlass::generic_signature_offset)
      );
  }
}

// Unevolving classes may point to methods of the evolving class directly
// from their constantpool caches and vtables/itables. Fix this. 
void VM_RedefineClasses::adjust_cpool_cache_and_vtable(klassOop k_oop, oop loader) {
  Klass *k = k_oop->klass_part();
  if (k->oop_is_instance()) {
    instanceKlass *ik = (instanceKlass *) k;
    bool previous_version; 

    // By this time we have already replaced the constantpool pointer in the evolving
    // class itself. However, we need to fix the entries in the old constantpool, which
    // is still referenced by active old methods of this class.
    constantPoolCacheOop cp_cache = (k_oop == _evolving_koop) ? 
      _old_constants->cache() : ik->constants()->cache();
      
    do {

      if (cp_cache != NULL) {
        cp_cache->adjust_method_entries(_old_methods, _new_methods);
      }

      // Fix vtable (this is needed for the evolving class itself and its subclasses).
      if (ik->vtable_length() > 0 && ik->is_subclass_of(_evolving_koop)) {
        ik->vtable()->adjust_entries(_old_methods, _new_methods);         
      }
      // Fix itable, if it exists.
      if (ik->itable_length() > 0) {
        ik->itable()->adjust_method_entries(_old_methods, _new_methods);
      }

      // Previous version methods could be still on stack so adjust entries
      // in all previous versions.
      if (ik->has_previous_version()) {
        ik = (instanceKlass *)(ik->previous_version())->klass_part();
        cp_cache = ik->constants()->cache();
        previous_version = true;
      } else {
        previous_version = false;
      }
    } while (previous_version);
  }
}

void VM_RedefineClasses::check_methods_and_mark_as_old() {
  for (int i = 0; i < _old_methods->length(); i++) {
    methodOop old_method = (methodOop) _old_methods->obj_at(i);
    old_method->set_old_version();
    methodOop new_method = (methodOop) _new_methods->obj_at(i);
    if (!MethodComparator::methods_EMCP(old_method, new_method)) {
      // Mark non-EMCP methods as such
      old_method->set_non_emcp_with_new_version();
    }
  }
}

// don't lose the association between a native method and its JNI function
void VM_RedefineClasses::transfer_old_native_function_registrations() {
  for (int i = 0; i < _old_methods->length(); i++) {
    methodOop old_method = (methodOop) _old_methods->obj_at(i);
    if (old_method->is_native() && old_method->has_native_function() ) {
      methodOop new_method = (methodOop) _new_methods->obj_at(i);
      if (new_method->is_native()) {
        new_method->set_native_function(old_method->native_function());
      }
    }
  }
}

void VM_RedefineClasses::flush_method_jmethod_id_cache() {
  for (int i = 0; i < _old_methods->length(); i++) {
    methodOop mop = (methodOop)_old_methods->obj_at(i);
    jmethodID mid = mop->find_jmethod_id_or_null();
    if (mid != NULL) {
      jniIdDetails* details;
      jniIdSupport::to_method_details(mid, &details);
      details->set_itable_index(-1);
    }
  }
}

// Install the redefinition of a class --
// The original instanceKlass object (k_h) always represents the latest 
// version of the respective class. However, during class redefinition we swap
// or replace much of its content with that of the instanceKlass object created
// from the bytes of the redefine (k_h_new). Specifically, k_h points to the new
// constantpool and methods objects, which we take from k_h_new. k_h_new, in turn,
// assumes the role of the previous class version, with the old constantpool and
// methods (taken from k_h) attached to it. k_h links to k_h_new to create a 
// linked list of class versions. 
void VM_RedefineClasses::redefine_single_class(jclass j_clazz, instanceKlassHandle k_h_new, TRAPS) {

  oop mirror = JNIHandles::resolve_non_null(j_clazz);
  klassOop k_oop = java_lang_Class::as_klassOop(mirror);
  instanceKlassHandle k_h = instanceKlassHandle(THREAD, k_oop);

  // Remove all breakpoints in methods of this class
  JvmtiBreakpoints& jvmti_breakpoints = JvmtiCurrentBreakpoints::get_jvmti_breakpoints();
  jvmti_breakpoints.clearall_in_class_at_safepoint(k_oop); 

  // Deoptimize all compiled code that depends on this class
  NOT_CORE(Universe::flush_evol_dependents_on(k_h));

  _old_methods = k_h->methods();
  _new_methods = k_h_new->methods();
  _evolving_koop = k_oop;
  _old_constants = k_h->constants();

  // flush the cached jmethodID fields for _old_methods
  flush_method_jmethod_id_cache();

  // Patch the indexes into the constantpool from the array of fields of the evolving
  // class. This is required, because the layout of the new constantpool can be different,
  // so old indexes corresponding to field names and signatures can become invalid.
  patch_indexes_for_fields(k_h, k_h_new);

  // Make new constantpool object (and methodOops via it) point to the original class object
  k_h_new->constants()->set_pool_holder(k_h());

  // Replace methods and constantpool
  k_h->set_methods(_new_methods);
  k_h_new->set_methods(_old_methods);     // To prevent potential GCing of the old methods, 
                                          // and to be able to undo operation easily.

  constantPoolOop old_constants = k_h->constants();
  k_h->set_constants(k_h_new->constants());
  k_h_new->set_constants(old_constants);  // See the previous comment.

  check_methods_and_mark_as_old();
  transfer_old_native_function_registrations();

  // Replace inner_classes
  typeArrayOop old_inner_classes = k_h->inner_classes();
  k_h->set_inner_classes(k_h_new->inner_classes());
  k_h_new->set_inner_classes(old_inner_classes);

  // Initialize the vtable and interface table after
  // methods have been rewritten
  { ResourceMark rm(THREAD);
  k_h->vtable()->initialize_vtable(THREAD); // No exception can happen here
  k_h->itable()->initialize_itable();
  }

  // Copy the "source file name" attribute from new class version
  k_h->set_source_file_name(k_h_new->source_file_name());

  // Copy the "source debug extension" attribute from new class version
  k_h->set_source_debug_extension(k_h_new->source_debug_extension());

  // Use of javac -g could be different in the old and the new
  if (k_h_new->access_flags().has_localvariable_table() !=
      k_h->access_flags().has_localvariable_table()) {

    AccessFlags flags = k_h->access_flags();
    if (k_h_new->access_flags().has_localvariable_table()) {
      flags.set_has_localvariable_table();
    } else {
      flags.clear_has_localvariable_table();
    }
    k_h->set_access_flags(flags);
  }

  // Replace class annotation fields values
  typeArrayOop old_class_annotations = k_h->class_annotations();
  k_h->set_class_annotations(k_h_new->class_annotations());
  k_h_new->set_class_annotations(old_class_annotations);

  // Replace fields annotation fields values
  objArrayOop old_fields_annotations = k_h->fields_annotations();
  k_h->set_fields_annotations(k_h_new->fields_annotations());
  k_h_new->set_fields_annotations(old_fields_annotations);

  // Replace methods annotation fields values
  objArrayOop old_methods_annotations = k_h->methods_annotations();
  k_h->set_methods_annotations(k_h_new->methods_annotations());
  k_h_new->set_methods_annotations(old_methods_annotations);

  // Replace methods parameter annotation fields values
  objArrayOop old_methods_parameter_annotations = k_h->methods_parameter_annotations();
  k_h->set_methods_parameter_annotations(k_h_new->methods_parameter_annotations());
  k_h_new->set_methods_parameter_annotations(old_methods_parameter_annotations);

  // Replace methods default annotation fields values
  objArrayOop old_methods_default_annotations = k_h->methods_default_annotations();
  k_h->set_methods_default_annotations(k_h_new->methods_default_annotations());
  k_h_new->set_methods_default_annotations(old_methods_default_annotations);

  // Replace major version number of class file
  u2 old_major_version = k_h->major_version();
  k_h->set_major_version(k_h_new->major_version());
  k_h_new->set_major_version(old_major_version);

  // Replace CP indexes for class and name+type of enclosing method
  u2 old_class_idx  = k_h->enclosing_method_class_index();
  u2 old_method_idx = k_h->enclosing_method_method_index();
  k_h->set_enclosing_method_indices(k_h_new->enclosing_method_class_index(),
                                    k_h_new->enclosing_method_method_index());
  k_h_new->set_enclosing_method_indices(old_class_idx, old_method_idx);

  // Maintain a linked list of versions of this class. 
  // List is in ascending age order. Current version (k_h) is the head.
  if (k_h->has_previous_version()) {
    k_h_new->set_previous_version(k_h->previous_version());
  }
  k_h->set_previous_version(k_h_new);

  // Adjust constantpool caches and vtables for all classes
  // that reference methods of the evolved class.
  SystemDictionary::classes_do(adjust_cpool_cache_and_vtable);
  k_h->set_rewritten_by_redefine(true);
}
 
