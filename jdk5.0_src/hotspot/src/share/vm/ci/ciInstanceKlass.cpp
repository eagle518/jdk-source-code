#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciInstanceKlass.cpp	1.27 03/12/23 16:39:30 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciInstanceKlass.cpp.incl"

// ciInstanceKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in an instanceKlass.

// ------------------------------------------------------------------
// ciInstanceKlass::ciInstanceKlass
//
// Loaded instance klass.
ciInstanceKlass::ciInstanceKlass(KlassHandle h_k) : ciKlass(h_k) {
  assert(get_Klass()->oop_is_instance(), "wrong type");
  instanceKlass* ik = get_instanceKlass();

  AccessFlags access_flags = ik->access_flags();
  _flags = ciFlags(access_flags);
  _has_finalizer = access_flags.has_finalizer();
  _has_subklass = ik->subklass() != NULL;
  _is_initialized = ik->is_initialized();
  _is_being_initialized = ik->is_being_initialized();
  _size_helper = ik->size_helper();
  _nonstatic_field_size = ik->nonstatic_field_size();

  _checked_implementor = false;
  _implementor = NULL;

  Thread *thread = Thread::current();
  if (ciObjectFactory::shared_is_initialized()) {
    _loader = JNIHandles::make_local(thread, ik->class_loader());
    _protection_domain = JNIHandles::make_local(thread,
                                                ik->protection_domain());
    _is_shared = false;
  } else {
    Handle h_loader(thread, ik->class_loader());
    Handle h_protection_domain(thread, ik->protection_domain());
    _loader = JNIHandles::make_global(h_loader, false);
    _protection_domain = JNIHandles::make_global(h_protection_domain, false);
    _is_shared = true;
  }
  
  // Lazy fields get filled in only upon request.
  _super  = NULL;
  _java_mirror = NULL;

  if (_is_shared) {
    if (h_k() != SystemDictionary::object_klass()) {
      super();
    }
    java_mirror();
  }

  _field_cache = NULL;
}
// ------------------------------------------------------------------
// ciInstanceKlass::compute_shared_is_initialized
bool ciInstanceKlass::compute_shared_is_initialized() {
  GUARDED_VM_ENTRY(
    instanceKlass* ik = get_instanceKlass();
    _is_initialized = ik->is_initialized();
    return _is_initialized;
  )
}

// ------------------------------------------------------------------
// ciInstanceKlass::compute_shared_has_subklass
bool ciInstanceKlass::compute_shared_has_subklass() {
  GUARDED_VM_ENTRY(
    instanceKlass* ik = get_instanceKlass();
    _has_subklass = ik->subklass() != NULL;
    return _has_subklass;
  )
}

// ------------------------------------------------------------------
// ciInstanceKlass::loader
oop ciInstanceKlass::loader() {
  ASSERT_IN_VM;
  return JNIHandles::resolve(_loader);
}

// ------------------------------------------------------------------
// ciInstanceKlass::loader_handle
jobject ciInstanceKlass::loader_handle() {
  return _loader;
}

// ------------------------------------------------------------------
// ciInstanceKlass::protection_domain
oop ciInstanceKlass::protection_domain() {
  ASSERT_IN_VM;
  return JNIHandles::resolve(_protection_domain);
}

// ------------------------------------------------------------------
// ciInstanceKlass::protection_domain_handle
jobject ciInstanceKlass::protection_domain_handle() {
  return _protection_domain;
}

// ------------------------------------------------------------------
// ciInstanceKlass::field_cache
//
// Get the field cache associated with this klass.
ciConstantPoolCache* ciInstanceKlass::field_cache() {
  if (_is_shared) {
    return NULL;
  }
  if (_field_cache == NULL) {
    assert(!is_java_lang_Object(), "Object has no fields");
    Arena* arena = CURRENT_ENV->arena();
    _field_cache = new (arena) ciConstantPoolCache(arena, 5);
  }
  return _field_cache;
}

// ------------------------------------------------------------------
// ciInstanceKlass::get_canonical_holder
//
ciInstanceKlass* ciInstanceKlass::get_canonical_holder(int offset) {
  #ifdef ASSERT
  if (!(offset >= 0 && offset < (size_helper() * wordSize))) {
    tty->print("*** get_canonical_holder(%d) on ", offset);
    this->print();
    tty->print_cr(" ***");
  };
  assert(offset >= 0 && offset < (size_helper() * wordSize), "offset must be tame");
  #endif

  if (offset < (instanceOopDesc::header_size() * wordSize)) {
    // All header offsets belong properly to java/lang/Object.
    return CURRENT_ENV->Object_klass();
  }
  
  ciInstanceKlass* self = this;
  for (;;) {
    assert(self->is_loaded(), "must be loaded to have size");
    ciInstanceKlass* super = self->super();
    if (super == NULL || !super->contains_field_offset(offset)) {
      return self;
    } else {
      self = super;  // return super->get_canonical_holder(offset)
    }
  }
}

// ------------------------------------------------------------------
// ciInstanceKlass::is_java_lang_Object
//
// Is this klass java.lang.Object?
bool ciInstanceKlass::is_java_lang_Object() {
  return equals(CURRENT_ENV->Object_klass());
}

// ------------------------------------------------------------------
// ciInstanceKlass::uses_default_loader
bool ciInstanceKlass::uses_default_loader() {
  VM_ENTRY_MARK;
  return loader() == NULL;
}

// ------------------------------------------------------------------
// ciInstanceKlass::print_impl
//
// Implementation of the print method.
void ciInstanceKlass::print_impl() {
  ciKlass::print_impl();
  GUARDED_VM_ENTRY(tty->print(" loader=0x%x", loader());)
  if (is_loaded()) {
    tty->print(" loaded=true initialized=%s finalized=%s subklass=%s size=%d flags=",
               bool_to_str(is_initialized()),
               bool_to_str(has_finalizer()),
               bool_to_str(has_subklass()),
               _size_helper);

    _flags.print_klass_flags();

    if (_super) {
      tty->print(" super=");
      _super->print_name();
    }
    if (_java_mirror) {
      tty->print(" mirror=PRESENT");
    }
  } else {
    tty->print(" loaded=false");
  }
}

// ------------------------------------------------------------------
// ciInstanceKlass::super
//
// Get the superklass of this klass.
ciInstanceKlass* ciInstanceKlass::super() {
  assert(is_loaded(), "must be loaded");
  if (_super == NULL && !is_java_lang_Object()) {
    GUARDED_VM_ENTRY(
      klassOop super_klass = get_instanceKlass()->super();
      _super = CURRENT_ENV->get_object(super_klass)->as_instance_klass();
    )
  }
  return _super;
}

// ------------------------------------------------------------------
// ciInstanceKlass::java_mirror
//
// Get the instance of java.lang.Class corresponding to this klass.
ciInstance* ciInstanceKlass::java_mirror() {
  assert(is_loaded(), "must be loaded");
  if (_java_mirror == NULL) {
    _java_mirror = ciKlass::java_mirror();
  }
  return _java_mirror;
}

// ------------------------------------------------------------------
// ciInstanceKlass::unique_concrete_subklass
ciInstanceKlass* ciInstanceKlass::unique_concrete_subklass() {
  if (!is_loaded())     return NULL; // No change if class is not loaded
  if (!is_abstract())   return NULL; // Only applies to abstract classes.
  if (!has_subklass())  return NULL; // Must have at least one subklass.
  VM_ENTRY_MARK;
  instanceKlass* ik = get_instanceKlass();
  Klass* up = ik->up_cast_abstract();
  assert(up->oop_is_instance(), "must be instanceKlass");
  if (ik == up) {
    return NULL;
  }
  return CURRENT_THREAD_ENV->get_object(up->as_klassOop())->as_instance_klass();
}

// ------------------------------------------------------------------
// ciInstanceKlass::get_field_by_offset
ciField* ciInstanceKlass::get_field_by_offset(int field_offset, bool is_static) {
  VM_ENTRY_MARK;
  instanceKlass* k = get_instanceKlass();
  fieldDescriptor fd;
  if (!k->find_field_from_offset(field_offset, is_static, &fd)) {
    return NULL;
  }
  ciField* field = new (CURRENT_THREAD_ENV->arena()) ciField(&fd);
  return field;
}

// ------------------------------------------------------------------
// ciInstanceKlass::find_method
//
// Find a method in this klass.
ciMethod* ciInstanceKlass::find_method(ciSymbol* name, ciSymbol* signature) {
  VM_ENTRY_MARK;
  instanceKlass* k = get_instanceKlass();
  symbolOop name_sym = name->get_symbolOop();
  symbolOop sig_sym= signature->get_symbolOop();

  methodOop m = k->find_method(name_sym, sig_sym);
  if (m == NULL)  return NULL;

  return CURRENT_THREAD_ENV->get_object(m)->as_method();
}

// ------------------------------------------------------------------
// ciInstanceKlass::find_method
//
// Find a method in this klass.
ciMethod* ciInstanceKlass::find_method(int slot) {
  VM_ENTRY_MARK;
  instanceKlass* k = get_instanceKlass();

  objArrayOop methods = k->methods();

  if (!methods->is_within_bounds(slot))  return NULL;

  methodOop m = methodOop(methods->obj_at(slot));
  if (m == NULL)  return NULL;
  assert(m->is_method(), "must be a method");

  return CURRENT_THREAD_ENV->get_object(m)->as_method();
}


ciInstanceKlass* ciInstanceKlass::implementor() {
  if (!_checked_implementor) {
    _checked_implementor = true;
    VM_ENTRY_MARK;
    klassOop k = get_instanceKlass()->implementor();
    if (k != NULL) {
      _implementor = CURRENT_THREAD_ENV->get_object(k)->as_instance_klass();
    }
  }
  return _implementor;
}
