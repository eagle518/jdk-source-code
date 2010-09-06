#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)linkResolver.cpp	1.159 03/12/23 16:40:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_linkResolver.cpp.incl"

//------------------------------------------------------------------------------------------------------------------------
// Implementation of FieldAccessInfo

void FieldAccessInfo::set(KlassHandle klass, symbolHandle name, int field_index, int field_offset,
BasicType field_type, AccessFlags access_flags) {
  _klass        = klass;
  _name         = name;
  _field_index  = field_index;
  _field_offset = field_offset;
  _field_type   = field_type;
  _access_flags = access_flags;
}


//------------------------------------------------------------------------------------------------------------------------
// Implementation of CallInfo


void CallInfo::set(KlassHandle resolved_klass, methodHandle resolved_method, TRAPS) {
  set(resolved_klass, resolved_klass, resolved_method, resolved_method, resolved_method->vtable_index(), CHECK);
}


void CallInfo::set(KlassHandle resolved_klass, KlassHandle selected_klass, methodHandle resolved_method, methodHandle selected_method, TRAPS) {
  // This is only called for interface methods. If the resolved_method
  // comes from java/lang/Object, it means it is not in the itable and
  // we should pick the vtable index from the resolved method.
  int vtable_index = -1;
  if (resolved_method->method_holder() == SystemDictionary::object_klass()) {
    assert(resolved_method->vtable_index() == selected_method->vtable_index(), "sanity check");
    vtable_index = resolved_method->vtable_index();
  }
  set(resolved_klass, selected_klass, resolved_method, selected_method, vtable_index, CHECK);
}


void CallInfo::set(KlassHandle resolved_klass, KlassHandle selected_klass, methodHandle resolved_method, methodHandle selected_method, int vtable_index, TRAPS) {
  assert(resolved_method->signature() == selected_method->signature(), "signatures must correspond");
  _resolved_klass  = resolved_klass;
  _selected_klass  = selected_klass;
  _resolved_method = resolved_method;
  _selected_method = selected_method;
  _vtable_index    = vtable_index;
#ifndef CORE
  if (CompilationPolicy::mustBeCompiled(selected_method)) {
    // Note: with several active threads, the mustBeCompiled may be true
    //       while canBeCompiled is false; remove assert
    // assert(CompilationPolicy::canBeCompiled(selected_method), "cannot compile");
    if (THREAD->is_Compiler_thread()) {
      // don't force compilation, resolve was on behalf of compiler
      return;
    }
    CompileBroker::compile_method(selected_method, InvocationEntryBci,
                                  methodHandle(), 0, "mustBeCompiled", CHECK);
  }
#endif // CORE
}


//------------------------------------------------------------------------------------------------------------------------
// Klass resolution

void LinkResolver::check_klass_accessability(KlassHandle ref_klass, KlassHandle sel_klass, TRAPS) {
  if (!Reflection::verify_class_access(ref_klass->as_klassOop(), 
                                       sel_klass->as_klassOop(), 
                                       true)) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(
      THREAD_AND_LOCATION,
      vmSymbolHandles::java_lang_IllegalAccessError(),
      "tried to access class %s from class %s",
      sel_klass->external_name(),
      ref_klass->external_name()
      );
  }
}

void LinkResolver::resolve_klass(KlassHandle& result, constantPoolHandle pool, int index, TRAPS) {
  klassOop result_oop = pool->klass_ref_at(index, CHECK);
  result = KlassHandle(THREAD, result_oop);
}

void LinkResolver::resolve_klass_no_update(KlassHandle& result, constantPoolHandle pool, int index, TRAPS) {
  klassOop result_oop = 
         constantPoolOopDesc::klass_ref_at_if_loaded_check(pool, index, CHECK);
  result = KlassHandle(THREAD, result_oop);
}


//------------------------------------------------------------------------------------------------------------------------
// Method resolution
//
// According to JVM spec. $5.4.3c & $5.4.3d

void LinkResolver::lookup_method_in_klasses(methodHandle& result, KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS) {
  methodOop result_oop = klass->uncached_lookup_method(name(), signature());
  result = methodHandle(THREAD, result_oop);
}

// returns first instance method
void LinkResolver::lookup_instance_method_in_klasses(methodHandle& result, KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS) {
  methodOop result_oop = klass->uncached_lookup_method(name(), signature());
  result = methodHandle(THREAD, result_oop);
  while (!result.is_null() && result->is_static()) {
    klass = KlassHandle(THREAD, Klass::cast(result->method_holder())->super());
    result = methodHandle(THREAD, klass->uncached_lookup_method(name(), signature()));
  }
}


int LinkResolver::vtable_index_of_miranda_method(KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS) {
  ResourceMark rm(THREAD);
  klassVtable *vt = instanceKlass::cast(klass())->vtable();
  return vt->index_of_miranda(name(), signature());
}

void LinkResolver::lookup_method_in_interfaces(methodHandle& result, KlassHandle klass, symbolHandle name, symbolHandle signature, TRAPS) {
  instanceKlass *ik = instanceKlass::cast(klass());
  result = methodHandle(THREAD, ik->lookup_method_in_all_interfaces(name(), signature()));
}

void LinkResolver::check_method_accessability(KlassHandle ref_klass, 
                                              KlassHandle resolved_klass, 
                                              KlassHandle sel_klass, 
                                              methodHandle sel_method, 
                                              TRAPS) {

  AccessFlags flags = sel_method->access_flags();

  // Special case:  arrays always override "clone". JVMS 2.15.
  // If the resolved klass is an array class, and the declaring class
  // is java.lang.Object and the method is "clone", set the flags
  // to public.
  //
  // We'll check for the method name first, as that's most likely
  // to be false (so we'll short-circuit out of these tests).
  if (sel_method->name() == vmSymbols::clone_name() &&
      sel_klass() == SystemDictionary::object_klass() &&
      resolved_klass->oop_is_array()) {
    // We need to change "protected" to "public".
    assert(flags.is_protected(), "clone not protected?");
    jint new_flags = flags.as_int();
    new_flags = new_flags & (~JVM_ACC_PROTECTED);
    new_flags = new_flags | JVM_ACC_PUBLIC;
    flags.set_flags(new_flags);
  }

  if (!Reflection::verify_field_access(ref_klass->as_klassOop(), 
                                       resolved_klass->as_klassOop(),
                                       sel_klass->as_klassOop(), 
                                       flags, 
                                       true)) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(
      THREAD_AND_LOCATION,
      vmSymbolHandles::java_lang_IllegalAccessError(),
      "tried to access method %s.%s%s from class %s",
      sel_klass->external_name(),
      sel_method->name()->as_C_string(),
      sel_method->signature()->as_C_string(),
      ref_klass->external_name()
      );
  }
}

void LinkResolver::resolve_method(methodHandle& resolved_method, KlassHandle& resolved_klass, 
				  constantPoolHandle pool, int index, TRAPS) {

  // resolve klass
  resolve_klass(resolved_klass, pool, index, CHECK);
  
  symbolHandle method_name      (THREAD, pool->name_ref_at(index));
  symbolHandle method_signature (THREAD, pool->signature_ref_at(index));
  KlassHandle  current_klass(THREAD, pool->pool_holder());
  
  resolve_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, true, CHECK);
}

void LinkResolver::resolve_interface_method(methodHandle& resolved_method, KlassHandle& resolved_klass, constantPoolHandle pool, int index, TRAPS) {

  // resolve klass
  resolve_klass(resolved_klass, pool, index, CHECK);  
  symbolHandle method_name      (THREAD, pool->name_ref_at(index));
  symbolHandle method_signature (THREAD, pool->signature_ref_at(index));
  KlassHandle  current_klass(THREAD, pool->pool_holder());
  
  resolve_interface_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, true, CHECK);
}


void LinkResolver::resolve_method(methodHandle& resolved_method, KlassHandle resolved_klass,
				  symbolHandle method_name, symbolHandle method_signature, 
				  KlassHandle current_klass, bool check_access, TRAPS) {

  // 1. check if klass is not interface
  if (resolved_klass->is_interface()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }
  
  // 2. lookup method in resolved klass and its super klasses
  lookup_method_in_klasses(resolved_method, resolved_klass, method_name, method_signature, CHECK);
  
  if (resolved_method.is_null()) { // not found in the class hierarchy
    // 3. lookup method in all the interfaces implemented by the resolved klass
    lookup_method_in_interfaces(resolved_method, resolved_klass, method_name, method_signature, CHECK);
    
    if (resolved_method.is_null()) {
      // 4. method lookup failed
      ResourceMark rm(THREAD);
      THROW_MSG(vmSymbols::java_lang_NoSuchMethodError(),
                methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                                        method_name(),
                                                        method_signature()));
    }
  }

  // 5. check if method is concrete
  if (resolved_method->is_abstract() && !resolved_klass->is_abstract()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                                      method_name(),
                                                      method_signature()));
  }

  // 6. access checks, access checking may be turned off when calling from within the VM.
  if (check_access) {
    assert(current_klass.not_null() , "current_klass should not be null");

    // check if method can be accessed by the referring class
    check_method_accessability(current_klass, 
                               resolved_klass, 
                               KlassHandle(THREAD, resolved_method->method_holder()), 
                               resolved_method, 
                               CHECK);
    
    // check loader constraints
    Handle loader (THREAD, instanceKlass::cast(current_klass())->class_loader());
    Handle class_loader (THREAD, instanceKlass::cast(resolved_method->method_holder())->class_loader());
    SystemDictionary::check_signature_loaders(method_signature, loader,
                                              class_loader, true, CHECK);
  }
}

void LinkResolver::resolve_interface_method(methodHandle& resolved_method,
                                            KlassHandle resolved_klass,
                                            symbolHandle method_name,
                                            symbolHandle method_signature,
                                            KlassHandle current_klass,
                                            bool check_access, TRAPS) {

 // check if klass is interface
  if (!resolved_klass->is_interface()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }
  
  // lookup method in this interface or its super, java.lang.Object
  lookup_instance_method_in_klasses(resolved_method, resolved_klass, method_name, method_signature, CHECK);
  
  if (resolved_method.is_null()) {
    // lookup method in all the super-interfaces
    lookup_method_in_interfaces(resolved_method, resolved_klass, method_name, method_signature, CHECK);
    if (resolved_method.is_null()) {
      // no method found
      ResourceMark rm(THREAD);
      THROW_MSG(vmSymbols::java_lang_NoSuchMethodError(),
                methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                                        method_name(),
                                                        method_signature()));
    }
  }

  if (check_access) {  
    HandleMark hm(THREAD);
    Handle loader (THREAD, instanceKlass::cast(current_klass())->class_loader());
    Handle class_loader (THREAD, instanceKlass::cast(resolved_method->method_holder())->class_loader());
    SystemDictionary::check_signature_loaders(method_signature,
                                              loader, class_loader,
					      true,
					      CHECK);  
  }
}

//------------------------------------------------------------------------------------------------------------------------
// Field resolution

void LinkResolver::check_field_accessability(KlassHandle ref_klass, 
                                             KlassHandle resolved_klass,
                                             KlassHandle sel_klass, 
                                             fieldDescriptor& fd, 
                                             TRAPS) {
  if (!Reflection::verify_field_access(ref_klass->as_klassOop(), 
                                       resolved_klass->as_klassOop(),
                                       sel_klass->as_klassOop(), 
                                       fd.access_flags(), 
                                       true)) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(
      THREAD_AND_LOCATION,
      vmSymbolHandles::java_lang_IllegalAccessError(),
      "tried to access field %s.%s from class %s",
      sel_klass->external_name(),
      fd.name()->as_C_string(),
      ref_klass->external_name()
      );
  }
}

void LinkResolver::resolve_field(FieldAccessInfo& result, constantPoolHandle pool, int index, Bytecodes::Code byte, bool check_only, TRAPS) {
  resolve_field(result, pool, index, byte, check_only, true, CHECK);
}

void LinkResolver::resolve_field(FieldAccessInfo& result, constantPoolHandle pool, int index, Bytecodes::Code byte, bool check_only, bool update_pool, TRAPS) {
  assert(byte == Bytecodes::_getstatic || byte == Bytecodes::_putstatic ||
         byte == Bytecodes::_getfield  || byte == Bytecodes::_putfield, "bad bytecode");

  bool is_static = (byte == Bytecodes::_getstatic || byte == Bytecodes::_putstatic);
  bool is_put    = (byte == Bytecodes::_putfield  || byte == Bytecodes::_putstatic);

  // resolve specified klass
  KlassHandle resolved_klass;
  if (update_pool) {
    resolve_klass(resolved_klass, pool, index, CHECK);
  } else {
    resolve_klass_no_update(resolved_klass, pool, index, CHECK);
  }
  // Load these early in case the resolve of the containing klass fails
  symbolOop field = pool->name_ref_at(index);
  symbolOop sig   = pool->signature_ref_at(index);
  // Check if there's a resolved klass containing the field
  if( resolved_klass.is_null() ) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_NoSuchFieldError(), field->as_C_string());
  }

  // Resolve instance field
  fieldDescriptor fd; // find_field initializes fd if found
  KlassHandle sel_klass(THREAD, instanceKlass::cast(resolved_klass())->find_field(field, sig, &fd));
  // check if field exists; i.e., if a klass containing the field def has been selected
  if (sel_klass.is_null()){
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_NoSuchFieldError(), field->as_C_string());
  }
  
  // check access
  KlassHandle ref_klass(THREAD, pool->pool_holder());
  check_field_accessability(ref_klass, resolved_klass, sel_klass, fd, CHECK);

  // check for errors
  if (is_static != fd.is_static()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }

  // Final fields can only be accessed from its own class.  
  if (is_put && fd.access_flags().is_final() && sel_klass() != pool->pool_holder()) {    
    THROW(vmSymbols::java_lang_IllegalAccessError());
  }
  
  // initialize resolved_klass if necessary
  // note 1: the klass which declared the field must be initialized (i.e, sel_klass)
  //         according to the newest JVM spec (5.5, p.170) - was bug (gri 7/28/99)
  //
  // note 2: we don't want to force initialization if we are just checking
  //         if the field access is legal; e.g., during compilation
  if (is_static && !check_only) {
    sel_klass->initialize(CHECK);
  }

  {
    HandleMark hm(THREAD);
    Handle ref_loader (THREAD, instanceKlass::cast(ref_klass())->class_loader());
    Handle sel_loader (THREAD, instanceKlass::cast(sel_klass())->class_loader());
    symbolHandle signature_ref (THREAD, pool->signature_ref_at(index));
    SystemDictionary::check_signature_loaders(signature_ref,
                                              ref_loader, sel_loader,
					      false,
					      CHECK);
  }

  // return information. note that the klass is set to the actual klass containing the
  // field, otherwise access of static fields in superclasses will not work.
  KlassHandle holder (THREAD, fd.field_holder());
  symbolHandle name  (THREAD, fd.name());
  result.set(holder, name, fd.index(), fd.offset(), fd.field_type(), fd.access_flags());
}


//------------------------------------------------------------------------------------------------------------------------
// Invoke resolution
//
// Naming conventions:
//
// resolved_method    the specified method (i.e., static receiver specified via constant pool index)
// sel_method         the selected method  (selected via run-time lookup; e.g., based on dynamic receiver class)
// resolved_klass     the specified klass  (i.e., specified via constant pool index)
// recv_klass         the receiver klass


void LinkResolver::resolve_static_call(CallInfo& result, KlassHandle& resolved_klass, symbolHandle method_name, 
                                       symbolHandle method_signature, KlassHandle current_klass, 
                                       bool check_access, bool initialize_class, TRAPS) {
  methodHandle resolved_method;  
  linktime_resolve_static_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  resolved_klass = KlassHandle(THREAD, Klass::cast(resolved_method->method_holder()));
  
  // Initialize klass (this should only happen if everything is ok)  
  if (initialize_class && resolved_klass->should_be_initialized()) {
    resolved_klass->initialize(CHECK);
    linktime_resolve_static_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);      
  }

  // setup result
  result.set(resolved_klass, resolved_method, CHECK);
}

// throws linktime exceptions
void LinkResolver::linktime_resolve_static_method(methodHandle& resolved_method, KlassHandle resolved_klass, 
						  symbolHandle method_name, symbolHandle method_signature, 
						  KlassHandle current_klass, bool check_access, TRAPS) {
  
  resolve_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  assert(resolved_method->name() != vmSymbols::class_initializer_name(), "should have been checked in verifier");

  // check if static
  if (!resolved_method->is_static()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }
}


void LinkResolver::resolve_special_call(CallInfo& result, KlassHandle resolved_klass, symbolHandle method_name, 
                                        symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS) {
  methodHandle resolved_method;
  linktime_resolve_special_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  runtime_resolve_special_method(result, resolved_method, resolved_klass, current_klass, check_access, CHECK);
}

// throws linktime exceptions
void LinkResolver::linktime_resolve_special_method(methodHandle& resolved_method, KlassHandle resolved_klass, 
						   symbolHandle method_name, symbolHandle method_signature, 
						   KlassHandle current_klass, bool check_access, TRAPS) {

  resolve_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  
  // check if method name is <init>, that it is found in same klass as static type
  if (resolved_method->name() == vmSymbols::object_initializer_name() &&
      resolved_method->method_holder() != resolved_klass()) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(
      THREAD_AND_LOCATION,
      vmSymbolHandles::java_lang_NoSuchMethodError(), 
      "%s: method %s%s not found", 
      resolved_klass->external_name(), 
      resolved_method->name()->as_C_string(),
      resolved_method->signature()->as_C_string()
    );
  }
  
  // check if not static
  if (resolved_method->is_static()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }
}

// throws runtime exceptions
void LinkResolver::runtime_resolve_special_method(CallInfo& result, methodHandle resolved_method, KlassHandle resolved_klass,
						  KlassHandle current_klass, bool check_access, TRAPS) {
  
  // resolved method is selected method unless we have an old-style lookup
  methodHandle sel_method(THREAD, resolved_method());
  
  // check if this is an old-style super call and do a new lookup if so
  { KlassHandle method_klass  = KlassHandle(THREAD,
                                            resolved_method->method_holder());
           
    if (check_access &&
        // a) check if ACC_SUPER flag is set for the current class
        current_klass->is_super() &&
        // b) check if the method class is a superclass of the current class (superclass relation is not reflexive!)
        current_klass->is_subtype_of(method_klass()) && current_klass() != method_klass() &&
        // c) check if the method is not <init>
        resolved_method->name() != vmSymbols::object_initializer_name()) {
      // Lookup super method
      KlassHandle super_klass(THREAD, current_klass->super());
      lookup_instance_method_in_klasses(sel_method, super_klass, 
                           symbolHandle(THREAD, resolved_method->name()),
                           symbolHandle(THREAD, resolved_method->signature()), CHECK);
      // check if found
      if (sel_method.is_null()) {
        ResourceMark rm(THREAD);
        THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
                  methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                            resolved_method->name(),
                                            resolved_method->signature()));
      }
    }
  }

  // check if not static
  if (sel_method->is_static()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }
  
  // check if abstract
  if (sel_method->is_abstract()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                                      sel_method->name(),
                                                      sel_method->signature()));
  }
  
  // setup result
  result.set(resolved_klass, sel_method, CHECK);
}

void LinkResolver::resolve_virtual_call(CallInfo& result, Handle recv, KlassHandle receiver_klass, KlassHandle resolved_klass,
                                        symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, 
                                        bool check_access, bool check_null_and_abstract, TRAPS) {
  methodHandle resolved_method;  
  linktime_resolve_virtual_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  runtime_resolve_virtual_method(result, resolved_method, resolved_klass, recv, receiver_klass, check_null_and_abstract, CHECK);
}

// throws linktime exceptions
void LinkResolver::linktime_resolve_virtual_method(methodHandle &resolved_method, KlassHandle resolved_klass, 
						   symbolHandle method_name, symbolHandle method_signature,
						   KlassHandle current_klass, bool check_access, TRAPS) {
  // normal method resolution
  resolve_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  
  assert(resolved_method->name() != vmSymbols::object_initializer_name(), "should have been checked in verifier");
  assert(resolved_method->name() != vmSymbols::class_initializer_name (), "should have been checked in verifier");

  // check if not static
  if (resolved_method->is_static()) {
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }  
}

// throws runtime exceptions
void LinkResolver::runtime_resolve_virtual_method(CallInfo& result,
                                                  methodHandle resolved_method,
                                                  KlassHandle resolved_klass, 
                                                  Handle recv,
                                                  KlassHandle recv_klass,
                                                  bool check_null_and_abstract,
                                                  TRAPS) {
    
  // setup default return values
  int vtable_index = -1;
  methodHandle selected_method;
  
  assert(recv.is_null() || recv->is_oop(), "receiver is not an oop");

  // runtime method resolution
  if (check_null_and_abstract && recv.is_null()) { // check if receiver exists
    THROW(vmSymbols::java_lang_NullPointerException());
  }
    
  // Virtual methods cannot be resolved before its klass has been linked, for otherwise the methodOop's
  // has not been rewritten, and the vtable initialized.
  assert(instanceKlass::cast(resolved_method->method_holder())->is_linked(), "must be linked");

  // Virtual methods cannot be resolved before its klass has been linked, for otherwise the methodOop's
  // has not been rewritten, and the vtable initialized. Make sure to do this after the nullcheck, since
  // a missing receiver might result in a bogus lookup.
  assert(instanceKlass::cast(resolved_method->method_holder())->is_linked(), "must be linked");

  // do lookup based on receiver klass using the vtable index
  if (resolved_method->method_holder()->klass_part()->is_interface()) { // miranda method
    vtable_index = vtable_index_of_miranda_method(resolved_klass, 
                           symbolHandle(THREAD, resolved_method->name()), 
                           symbolHandle(THREAD, resolved_method->signature()), CHECK);
    assert(vtable_index != -1 , "we should have valid vtable index at this point");

    instanceKlass* inst = instanceKlass::cast(recv_klass());
    selected_method = methodHandle(THREAD, inst->method_at_vtable(vtable_index));
  } else {    
    // at this point we are sure that resolved_method is virtual and not
    // a miranda method; therefore, it must have a valid vtable index.
    vtable_index = resolved_method->vtable_index();
    // we could get a vtable_index equal to -1 for final methods, 
    // because as an optimization they are they are never put in the vtable, 
    // unless they override an existing method.
    // if we do get a -1, it means the resolved method is final and the selected
    // method cannot be a different method as final methods cannot be overriden.
    if (vtable_index == -1) {
      assert(resolved_method->is_final_method(), "should be final at this point");
      selected_method = resolved_method;
    } else {
      // recv_klass might be an arrayKlassOop but all vtables start at
      // the same place. The cast is to avoid virtual call and assertion.
      instanceKlass* inst = (instanceKlass*)recv_klass()->klass_part();
      selected_method = methodHandle(THREAD, inst->method_at_vtable(vtable_index));
    }
  }
  
  // check if method exists
  if (selected_method.is_null()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                                      resolved_method->name(),
                                                      resolved_method->signature()));
  }
  
  // check if abstract
  if (check_null_and_abstract && selected_method->is_abstract()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(resolved_klass()),
                                                      selected_method->name(),
                                                      selected_method->signature()));
  }
  
  // setup result
  result.set(resolved_klass, recv_klass, resolved_method, selected_method, vtable_index, CHECK);
}

void LinkResolver::resolve_interface_call(CallInfo& result, Handle recv, KlassHandle recv_klass, KlassHandle resolved_klass,
                                          symbolHandle method_name, symbolHandle method_signature, KlassHandle current_klass, 
                                          bool check_access, bool check_null_and_abstract, TRAPS) {
  methodHandle resolved_method;  
  linktime_resolve_interface_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);
  runtime_resolve_interface_method(result, resolved_method, resolved_klass, recv, recv_klass, check_null_and_abstract, CHECK);  
}

// throws linktime exceptions
void LinkResolver::linktime_resolve_interface_method(methodHandle& resolved_method, KlassHandle resolved_klass, symbolHandle method_name, 
						     symbolHandle method_signature, KlassHandle current_klass, bool check_access, TRAPS) {
  // normal interface method resolution
  resolve_interface_method(resolved_method, resolved_klass, method_name, method_signature, current_klass, check_access, CHECK);

  assert(resolved_method->name() != vmSymbols::object_initializer_name(), "should have been checked in verifier");
  assert(resolved_method->name() != vmSymbols::class_initializer_name (), "should have been checked in verifier");  
}

// throws runtime exceptions
void LinkResolver::runtime_resolve_interface_method(CallInfo& result, methodHandle resolved_method, KlassHandle resolved_klass, 
						    Handle recv, KlassHandle recv_klass, bool check_null_and_abstract, TRAPS) {
  // check if receiver exists
  if (check_null_and_abstract && recv.is_null()) {
    THROW(vmSymbols::java_lang_NullPointerException());
  }

  // check if receiver klass implements the resolved interface
  if (!recv_klass->is_subtype_of(resolved_klass())) { 
    THROW(vmSymbols::java_lang_IncompatibleClassChangeError());
  }
  // do lookup based on receiver klass
  methodHandle sel_method;
  lookup_instance_method_in_klasses(sel_method, recv_klass, 
            symbolHandle(THREAD, resolved_method->name()),
            symbolHandle(THREAD, resolved_method->signature()), CHECK);
  // check if method exists
  if (sel_method.is_null()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(recv_klass()),
                                                      resolved_method->name(),
                                                      resolved_method->signature()));
  }
  // check if public
  if (!sel_method->is_public()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_IllegalAccessError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(recv_klass()),
                                                      sel_method->name(),
                                                      sel_method->signature()));
  }
  // check if abstract
  if (check_null_and_abstract && sel_method->is_abstract()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_AbstractMethodError(),
              methodOopDesc::name_and_sig_as_C_string(Klass::cast(recv_klass()),
                                                      sel_method->name(),
                                                      sel_method->signature()));
  }
  // setup result
  result.set(resolved_klass, recv_klass, resolved_method, sel_method, CHECK);
}


methodHandle LinkResolver::linktime_resolve_interface_method_or_null(
                                                 KlassHandle resolved_klass,
                                                 symbolHandle method_name,
                                                 symbolHandle method_signature,
                                                 KlassHandle current_klass,
                                                 bool check_access) {
  EXCEPTION_MARK;
  methodHandle method_result;
  linktime_resolve_interface_method(method_result, resolved_klass, method_name, method_signature, current_klass, check_access, THREAD); 
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return methodHandle();
  } else {
    return method_result;
  }
}

methodHandle LinkResolver::linktime_resolve_virtual_method_or_null(
                                                 KlassHandle resolved_klass,
                                                 symbolHandle method_name,
                                                 symbolHandle method_signature,
                                                 KlassHandle current_klass,
                                                 bool check_access) {
  EXCEPTION_MARK;
  methodHandle method_result;
  linktime_resolve_virtual_method(method_result, resolved_klass, method_name, method_signature, current_klass, check_access, THREAD); 
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return methodHandle();
  } else {
    return method_result;
  }
}

methodHandle LinkResolver::resolve_virtual_call_or_null(
                                                 KlassHandle receiver_klass,
                                                 KlassHandle resolved_klass,
                                                 symbolHandle name,
                                                 symbolHandle signature,
                                                 KlassHandle current_klass) {
  EXCEPTION_MARK;
  CallInfo info;
  resolve_virtual_call(info, Handle(), receiver_klass, resolved_klass, name, signature, current_klass, true, false, THREAD); 
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return methodHandle();
  }
  return info.selected_method();
}

methodHandle LinkResolver::resolve_interface_call_or_null(
                                                 KlassHandle receiver_klass,
                                                 KlassHandle resolved_klass,
                                                 symbolHandle name,
                                                 symbolHandle signature,
                                                 KlassHandle current_klass) {
  EXCEPTION_MARK;
  CallInfo info;
  resolve_interface_call(info, Handle(), receiver_klass, resolved_klass, name, signature, current_klass, true, false, THREAD); 
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return methodHandle();
  }
  return info.selected_method();
}

methodHandle LinkResolver::resolve_static_call_or_null(
                                                  KlassHandle resolved_klass,
                                                  symbolHandle name,
                                                  symbolHandle signature,
                                                  KlassHandle current_klass) {
  EXCEPTION_MARK;
  CallInfo info;
  resolve_static_call(info, resolved_klass, name, signature, current_klass, true, false, THREAD); 
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return methodHandle();
  }
  return info.selected_method();
}

methodHandle LinkResolver::resolve_special_call_or_null(KlassHandle resolved_klass, symbolHandle name, symbolHandle signature, 
							KlassHandle current_klass) {
  EXCEPTION_MARK;
  CallInfo info;
  resolve_special_call(info, resolved_klass, name, signature, current_klass, true, THREAD); 
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return methodHandle();
  }
  return info.selected_method();
}



//------------------------------------------------------------------------------------------------------------------------
// ConstantPool entries

void LinkResolver::resolve_invoke(CallInfo& result, Handle recv, constantPoolHandle pool, int index, Bytecodes::Code byte, TRAPS) {
  switch (byte) {
    case Bytecodes::_invokestatic   : resolve_invokestatic   (result,       pool, index, CHECK); return;
    case Bytecodes::_invokespecial  : resolve_invokespecial  (result,       pool, index, CHECK); return;
    case Bytecodes::_invokevirtual  : resolve_invokevirtual  (result, recv, pool, index, CHECK); return;
    case Bytecodes::_invokeinterface: resolve_invokeinterface(result, recv, pool, index, CHECK); return;
  }
  ShouldNotReachHere();
}

void LinkResolver::resolve_pool(KlassHandle& resolved_klass, symbolHandle& method_name, symbolHandle& method_signature, 
                                KlassHandle& current_klass, constantPoolHandle pool, int index, TRAPS) {
   // resolve klass  
  resolve_klass(resolved_klass, pool, index, CHECK);

  // Get name, signature, and static klass
  method_name      = symbolHandle(THREAD, pool->name_ref_at(index));
  method_signature = symbolHandle(THREAD, pool->signature_ref_at(index));
  current_klass    = KlassHandle(THREAD, pool->pool_holder());
}


void LinkResolver::resolve_invokestatic(CallInfo& result, constantPoolHandle pool, int index, TRAPS) {  
  KlassHandle  resolved_klass;    
  symbolHandle method_name;
  symbolHandle method_signature;
  KlassHandle  current_klass;
  resolve_pool(resolved_klass, method_name,  method_signature, current_klass, pool, index, CHECK);  
  resolve_static_call(result, resolved_klass, method_name, method_signature, current_klass, true, true, CHECK);
}


void LinkResolver::resolve_invokespecial(CallInfo& result, constantPoolHandle pool, int index, TRAPS) {            
  KlassHandle  resolved_klass;    
  symbolHandle method_name;
  symbolHandle method_signature;
  KlassHandle  current_klass;
  resolve_pool(resolved_klass, method_name,  method_signature, current_klass, pool, index, CHECK);  
  resolve_special_call(result, resolved_klass, method_name, method_signature, current_klass, true, CHECK);
}


void LinkResolver::resolve_invokevirtual(CallInfo& result, Handle recv,
					  constantPoolHandle pool, int index,
					  TRAPS) {
        
  KlassHandle  resolved_klass;    
  symbolHandle method_name;
  symbolHandle method_signature;
  KlassHandle  current_klass;
  resolve_pool(resolved_klass, method_name,  method_signature, current_klass, pool, index, CHECK);  
  KlassHandle recvrKlass (THREAD, recv.is_null() ? NULL : recv->klass());
  resolve_virtual_call(result, recv, recvrKlass, resolved_klass, method_name, method_signature, current_klass, true, true, CHECK);
} 


void LinkResolver::resolve_invokeinterface(CallInfo& result, Handle recv, constantPoolHandle pool, int index, TRAPS) {
  KlassHandle  resolved_klass;    
  symbolHandle method_name;
  symbolHandle method_signature;
  KlassHandle  current_klass;
  resolve_pool(resolved_klass, method_name,  method_signature, current_klass, pool, index, CHECK);  
  KlassHandle recvrKlass (THREAD, recv.is_null() ? NULL : recv->klass());
  resolve_interface_call(result, recv, recvrKlass, resolved_klass, method_name, method_signature, current_klass, true, true, CHECK);
}

//------------------------------------------------------------------------------------------------------------------------
#ifndef PRODUCT

void FieldAccessInfo::print() {
  ResourceMark rm;
  tty->print_cr("Field %s@%d", name()->as_C_string(), field_offset());
}

#endif

