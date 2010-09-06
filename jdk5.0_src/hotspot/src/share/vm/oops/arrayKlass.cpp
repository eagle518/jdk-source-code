#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)arrayKlass.cpp	1.83 04/02/04 12:09:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_arrayKlass.cpp.incl"

int arrayKlass::object_size(int header_size) const {
  // size of an array klass object
  assert(header_size <= instanceKlass::header_size(), "bad header size");
  // If this assert fails, see comments in base_create_array_klass.
  header_size = instanceKlass::header_size();
#ifdef _LP64
  int size = header_size + align_object_offset(vtable_length());
#else
  int size = header_size + vtable_length();
#endif
  return align_object_size(size);
}


klassOop arrayKlass::java_super() const {
  if (super() == NULL)  return NULL;  // bootstrap case
  // Array klasses have primary supertypes which are not reported to Java.
  // Example super chain:  String[][] -> Object[][] -> Object[] -> Object
  return SystemDictionary::object_klass();
}


oop arrayKlass::multi_allocate(int rank, jint* sizes, int next_dim_step, TRAPS) {
  ShouldNotReachHere();
  return NULL;
}

methodOop arrayKlass::uncached_lookup_method(symbolOop name, symbolOop signature) const {
  // There are no methods in an array klass but the super class (Object) has some
  assert(super(), "super klass must be present");
  return Klass::cast(super())->uncached_lookup_method(name, signature);
}


arrayKlassHandle arrayKlass::base_create_array_klass(
const Klass_vtbl& cplusplus_vtbl, int header_size, KlassHandle klass, TRAPS) {
  // Allocation
  // Note: because the Java vtable must start at the same offset in all klasses,
  // we must insert filler fields into arrayKlass to make it the same size as instanceKlass.
  // If this assert fails, add filler to instanceKlass to make it bigger.
  assert(header_size <= instanceKlass::header_size(),
         "array klasses must be same size as instanceKlass");
  header_size = instanceKlass::header_size();
  // Arrays don't add any new methods, so their vtable is the same size as
  // the vtable of klass Object.
  int vtable_size = Universe::base_vtable_size();
  arrayKlassHandle k;
  KlassHandle base_klass = Klass::base_create_klass(klass,
                                                 header_size + vtable_size,
                                                 cplusplus_vtbl, CHECK_(k));
  k = arrayKlassHandle(THREAD, base_klass());

  assert(!k()->is_parsable(), "not expecting parsability yet.");
  k->set_super(Universe::is_bootstrapping() ? NULL : SystemDictionary::object_klass());
  k->set_size_helper(0);
  k->set_dimension(1);
  k->set_higher_dimension(NULL);
  k->set_lower_dimension(NULL);
  k->set_vtable_length(vtable_size);
  k->set_is_cloneable(); // All arrays are considered to be cloneable (See JLS 20.1.5)

  assert(k()->is_parsable(), "should be parsable here.");
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size + vtable_size), "wrong size for object");
  
  return k;
}


// Initialization of vtables and mirror object is done separatly from base_create_array_klass,
// since a GC can happen. At this point all instance variables of the arrayKlass must be setup.
void arrayKlass::complete_create_array_klass(arrayKlassHandle k, KlassHandle super_klass, TRAPS) {
  ResourceMark rm(THREAD);    
  k->initialize_supers(super_klass(), CHECK);
  k->vtable()->initialize_vtable(THREAD);  
  java_lang_Class::create_mirror(k, CHECK);     
}

objArrayOop arrayKlass::compute_secondary_supers(int num_extra_slots, TRAPS) {
  // interfaces = { cloneable_klass, serializable_klass };
  assert(num_extra_slots == 0, "sanity of primitive array type");
  // Must share this for correct bootstrapping!
  return Universe::the_array_interfaces_array();
}

bool arrayKlass::compute_is_subtype_of(klassOop k) {
  // An array is a subtype of Serializable, Clonable, and Object
  return    k == SystemDictionary::object_klass()
         || k == SystemDictionary::cloneable_klass()
         || k == SystemDictionary::serializable_klass();
}


inline intptr_t* arrayKlass::start_of_vtable() const {
  // all vtables start at the same place, that's why we use instanceKlass::header_size here
  return ((intptr_t*)as_klassOop()) + instanceKlass::header_size(); 
}


klassVtable* arrayKlass::vtable() const {
  KlassHandle kh(Thread::current(), as_klassOop());
  return new klassVtable(kh, start_of_vtable(), vtable_length() / vtableEntry::size());
}


objArrayOop arrayKlass::allocate_arrayArray(int n, int length, TRAPS) {
  if (length < 0) {
    THROW_0(vmSymbols::java_lang_NegativeArraySizeException());
  }
  if (length > arrayOopDesc::max_array_length(T_ARRAY)) {
    THROW_OOP_0(Universe::out_of_memory_error_array_size());
  }
  int size = objArrayOopDesc::object_size(length);
  klassOop k = array_klass(n+dimension(), CHECK_0);
  arrayKlassHandle ak (THREAD, k);
  objArrayOop o =
    (objArrayOop)CollectedHeap::array_allocate(ak, size, length, CHECK_0);
  // initialization to NULL not necessary, area already cleared
  return o;
}


void arrayKlass::array_klasses_do(void f(klassOop k)) {
  klassOop k = as_klassOop();
  // Iterate over this array klass and all higher dimensions
  while (k != NULL) {
    f(k);
    k = arrayKlass::cast(k)->higher_dimension();
  }
}


void arrayKlass::with_array_klasses_do(void f(klassOop k)) {
  array_klasses_do(f);
}

// JVM support

jint arrayKlass::compute_modifier_flags(TRAPS) const {
  return JVM_ACC_ABSTRACT | JVM_ACC_FINAL | JVM_ACC_PUBLIC;
}

// JVMTI support

jint arrayKlass::jvmti_class_status() const {
  return JVMTI_CLASS_STATUS_ARRAY;
}

#ifndef PRODUCT

// Printing

void arrayKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_array(), "must be array");
  Klass::oop_print_on(obj, st);
  st->print_cr(" - length: %d", arrayOop(obj)->length());
}


// Verification

void arrayKlass::oop_verify_on(oop obj, outputStream* st) {
  guarantee(obj->is_array(), "must be array");
  arrayOop a = arrayOop(obj);
  guarantee(a->length() >= 0, "array with negative length?");
}
#endif
