#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)constantPoolOop.cpp	1.82 04/02/23 13:36:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_constantPoolOop.cpp.incl"

klassOop constantPoolOopDesc::klass_at_impl(constantPoolHandle this_oop, int which, TRAPS) {   
  // A resolved constantPool entry will contain a klassOop, otherwise a symbolOop. 
  // It is not safe to rely on the tag bit's here, since we don't have a lock, and the entry and
  // tag is not updated atomicly.  
  oop entry = *(this_oop->obj_at_addr(which));
  if (entry->is_klass()) {
    // Already resolved - return entry.
    return (klassOop)entry;
  }  

  // Acquire lock on constant oop while doing update. After we get the lock, we check if another object
  // already has updated the object
  assert(THREAD->is_Java_thread(), "must be a Java thread");
  bool do_resolve = false;
  symbolHandle name;
  Handle       loader;
  { ObjectLocker ol(this_oop, THREAD);
    do_resolve = this_oop->tag_at(which).is_unresolved_klass();
    if (do_resolve) {
      name   = symbolHandle(THREAD, this_oop->unresolved_klass_at(which));
      loader = Handle(THREAD, instanceKlass::cast(this_oop->pool_holder())->class_loader());
    }
  } // unlocking constantPool
    

  if (do_resolve) {
    // this_oop must be unlocked during resolve_or_fail
    oop protection_domain = Klass::cast(this_oop->pool_holder())->protection_domain();
    Handle h_prot (THREAD, protection_domain);
    klassOop k_oop = SystemDictionary::resolve_or_fail(name, loader, h_prot, true, CHECK_0);
    KlassHandle k (THREAD, k_oop);

    // Do access check for klasses
    verify_constant_pool_resolve(this_oop, k, CHECK_0);
    
    if (TraceClassResolution && !k()->klass_part()->oop_is_array()) {
      // skip resolving the constant pool so that this code get's
      // called the next time some bytecodes refer to this class.
      ResourceMark rm;
      int line_number = -1;
      const char * source_file = NULL;
      if (JavaThread::current()->has_last_Java_frame()) {
        // try to identify the method which called this function.
        vframeStream vfst(JavaThread::current());
        if (!vfst.at_end()) {
          line_number = vfst.method()->line_number_from_bci(vfst.bci());
          symbolOop s = instanceKlass::cast(vfst.method()->method_holder())->source_file_name();
          if (s != NULL) {
            source_file = s->as_C_string();
          }
        }
      }
      if (k() != this_oop->pool_holder()) {
        // only print something if the classes are different
        if (source_file != NULL) {
          tty->print("RESOLVE %s %s %s:%d\n",
                     instanceKlass::cast(this_oop->pool_holder())->external_name(),
                     instanceKlass::cast(k())->external_name(), source_file, line_number);
        } else {
          tty->print("RESOLVE %s %s\n",
                     instanceKlass::cast(this_oop->pool_holder())->external_name(),
                     instanceKlass::cast(k())->external_name());
        }
      }
      return k();
    } else {
      ObjectLocker ol (this_oop, THREAD);
      // Only updated constant pool - if it is resolved.
      do_resolve = this_oop->tag_at(which).is_unresolved_klass();
      if (do_resolve) {
        this_oop->klass_at_put(which, k());       
      }
    }
  }
 
  entry = this_oop->resolved_klass_at(which);
  assert(entry->is_klass(), "must be resolved at this point");
  return (klassOop)entry;
}


// Does not update constantPoolOop - to avoid any exception throwing. Used
// by compiler and exception handling.  Also used to avoid classloads for
// instanceof operations. Returns NULL if the class has not been loaded or
// if the verification of constant pool failed
klassOop constantPoolOopDesc::klass_at_if_loaded(constantPoolHandle this_oop, int which) {  
  oop entry = *this_oop->obj_at_addr(which);
  if (entry->is_klass()) {
    return (klassOop)entry;
  } else {
    assert(entry->is_symbol(), "must be either symbol or klass");
    Thread *thread = Thread::current();
    symbolHandle name (thread, (symbolOop)entry);
    oop loader = instanceKlass::cast(this_oop->pool_holder())->class_loader();
    oop protection_domain = Klass::cast(this_oop->pool_holder())->protection_domain();
    Handle h_prot (thread, protection_domain);
    Handle h_loader (thread, loader);
    klassOop k = SystemDictionary::find(name, h_loader, h_prot, thread);    

    if (k != NULL) {
      // Make sure that resolving is legal
      EXCEPTION_MARK;
      KlassHandle klass(THREAD, k);
      // return NULL if verification fails
      verify_constant_pool_resolve(this_oop, klass, THREAD);
      if (HAS_PENDING_EXCEPTION) {
	CLEAR_PENDING_EXCEPTION;
	return NULL;
      }
      return klass();
    } else {
      return k;
    }
  }
}


klassOop constantPoolOopDesc::klass_ref_at_if_loaded(constantPoolHandle this_oop, int which) {
  return klass_at_if_loaded(this_oop, this_oop->klass_ref_index_at(which));
}


// This is an interface for the compiler that allows accessing non-resolved entries
// in the constant pool - but still performs the validations tests. Must be used
// in a pre-parse of the compiler - to determine what it can do and not do.
// Note: We cannot update the ConstantPool from the vm_thread.
klassOop constantPoolOopDesc::klass_ref_at_if_loaded_check(constantPoolHandle this_oop, int index, TRAPS) {
  int which = this_oop->klass_ref_index_at(index);
  oop entry = *this_oop->obj_at_addr(which);
  if (entry->is_klass()) {
    return (klassOop)entry;
  } else {
    assert(entry->is_symbol(), "must be either symbol or klass");
    symbolHandle name (THREAD, (symbolOop)entry);
    oop loader = instanceKlass::cast(this_oop->pool_holder())->class_loader();
    oop protection_domain = Klass::cast(this_oop->pool_holder())->protection_domain();
    Handle h_loader(THREAD, loader);
    Handle h_prot  (THREAD, protection_domain);
    KlassHandle k(THREAD, SystemDictionary::find(name, h_loader, h_prot, THREAD));
  
    // Do access check for klasses
    if( k.not_null() ) verify_constant_pool_resolve(this_oop, k, CHECK_0);
    return k();
  }
}


symbolOop constantPoolOopDesc::uncached_name_ref_at(int which) {
  jint ref_index = name_and_type_at(uncached_name_and_type_ref_index_at(which));
  int name_index = extract_low_short_from_int(ref_index);
  return symbol_at(name_index);
}


symbolOop constantPoolOopDesc::uncached_signature_ref_at(int which) {
  jint ref_index = name_and_type_at(uncached_name_and_type_ref_index_at(which));
  int signature_index = extract_high_short_from_int(ref_index);
  return symbol_at(signature_index);
}


int constantPoolOopDesc::uncached_name_and_type_ref_index_at(int which) {
  jint ref_index = field_or_method_at(which, true);
  return extract_high_short_from_int(ref_index);
}


int constantPoolOopDesc::uncached_klass_ref_index_at(int which) {
  jint ref_index = field_or_method_at(which, true);
  return extract_low_short_from_int(ref_index);
}


void constantPoolOopDesc::verify_constant_pool_resolve(constantPoolHandle this_oop, KlassHandle k, TRAPS) {
 if (k->oop_is_instance() || k->oop_is_objArray()) {        
    instanceKlassHandle holder (THREAD, this_oop->pool_holder());
    klassOop elem_oop = k->oop_is_instance() ? k() : objArrayKlass::cast(k())->bottom_klass();
    KlassHandle element (THREAD, elem_oop);
    
    // The element type could be a typeArray - we only need the access check if it is
    // an reference to another class
    if (element->oop_is_instance()) {
      LinkResolver::check_klass_accessability(holder, element, CHECK);
    }        
  }
}


int constantPoolOopDesc::klass_ref_index_at(int which) {
  jint ref_index = field_or_method_at(which, false);
  return extract_low_short_from_int(ref_index);
}


int constantPoolOopDesc::name_and_type_ref_index_at(int which) {
  jint ref_index = field_or_method_at(which, false);
  return extract_high_short_from_int(ref_index);
}


int constantPoolOopDesc::name_ref_index_at(int which) {
  jint ref_index = name_and_type_at(which);
  return extract_low_short_from_int(ref_index);
}


int constantPoolOopDesc::signature_ref_index_at(int which) {
  jint ref_index = name_and_type_at(which);
  return extract_high_short_from_int(ref_index);
}


klassOop constantPoolOopDesc::klass_ref_at(int which, TRAPS) {
  return klass_at(klass_ref_index_at(which), CHECK_0);
}


symbolOop constantPoolOopDesc::klass_name_at(int which) {
  assert(tag_at(which).is_unresolved_klass() || tag_at(which).is_klass(),
	 "Corrupted constant pool");
  // A resolved constantPool entry will contain a klassOop, otherwise a symbolOop. 
  // It is not safe to rely on the tag bit's here, since we don't have a lock, and the entry and
  // tag is not updated atomicly.  
  oop entry = *(obj_at_addr(which));
  if (entry->is_klass()) {
    // Already resolved - return entry's name.
    return klassOop(entry)->klass_part()->name();
  } else {
    assert(entry->is_symbol(), "must be either symbol or klass");
    return (symbolOop)entry;
  }
}

#ifdef HOTSWAP
symbolOop constantPoolOopDesc::klass_ref_at_noresolve(int which) {
  jint ref_index = klass_ref_index_at(which);
  return klass_at_noresolve(ref_index);  
}
#endif HOTSWAP

char* constantPoolOopDesc::string_at_noresolve(int which) {
  // Test entry type in case string is resolved while in here.
  oop entry = *(obj_at_addr(which));
  if (entry->is_symbol()) {
    return ((symbolOop)entry)->as_C_string();
  } else {
    return java_lang_String::as_utf8_string(entry);
  }
}


symbolOop constantPoolOopDesc::name_ref_at(int which) {
  jint ref_index = name_and_type_at(name_and_type_ref_index_at(which));
  int name_index = extract_low_short_from_int(ref_index);
  return symbol_at(name_index);
}


symbolOop constantPoolOopDesc::signature_ref_at(int which) {
  jint ref_index = name_and_type_at(name_and_type_ref_index_at(which));
  int signature_index = extract_high_short_from_int(ref_index);
  return symbol_at(signature_index);
}


BasicType constantPoolOopDesc::basic_type_for_signature_at(int which) {
  return FieldType::basic_type(symbol_at(which));
}


void constantPoolOopDesc::resolve_string_constants_impl(constantPoolHandle this_oop, TRAPS) {
  for (int index = 1; index < this_oop->length(); index++) { // Index 0 is unused
    if (this_oop->tag_at(index).is_unresolved_string()) {
      this_oop->string_at(index, CHECK);
    }
  }
}

oop constantPoolOopDesc::string_at_impl(constantPoolHandle this_oop, int which, TRAPS) {
  oop entry = *(this_oop->obj_at_addr(which));
  if (entry->is_symbol()) {
    ObjectLocker ol(this_oop, THREAD);
    if (this_oop->tag_at(which).is_unresolved_string()) {
      // Intern string
      symbolOop sym = this_oop->unresolved_string_at(which);
      entry = StringTable::intern(sym, CHECK_(constantPoolOop(NULL)));
      this_oop->string_at_put(which, entry);
    } else {
      // Another thread beat us and interned string, read string from constant pool
      entry = this_oop->resolved_string_at(which);
    }
  }
  assert(java_lang_String::is_instance(entry), "must be string");
  return entry;
}


bool constantPoolOopDesc::klass_name_at_matches(instanceKlassHandle k, 
                                                int which) {
  // Names are interned, so we can compare symbolOops directly
  symbolOop cp_name = klass_name_at(which);
  return (cp_name == k->name());
}


int constantPoolOopDesc::pre_resolve_shared_klasses(TRAPS) {
  ResourceMark rm;
  int count = 0;
  for (int index = 1; index < tags()->length(); index++) { // Index 0 is unused
    if (tag_at(index).is_unresolved_string()) {
      // Intern string
      symbolOop sym = unresolved_string_at(index);
      oop entry = StringTable::intern(sym, CHECK_(-1));
      string_at_put(index, entry);
    }
  }
  return count;
}


// Iterate over symbols which are used as class, field, method names and
// signatures (in preparation for writing to the shared archive).

void constantPoolOopDesc::shared_symbols_iterate(OopClosure* closure) {
  for (int index = 1; index < length(); index++) { // Index 0 is unused
    switch (tag_at(index).value()) {

    case JVM_CONSTANT_UnresolvedClass:
      closure->do_oop(obj_at_addr(index));
      break;

    case JVM_CONSTANT_NameAndType:
      {
        int i = *int_at_addr(index);
        closure->do_oop(obj_at_addr((unsigned)i >> 16));
        closure->do_oop(obj_at_addr((unsigned)i & 0xffff));
      }
      break;

    case JVM_CONSTANT_Class:
    case JVM_CONSTANT_InterfaceMethodref:
    case JVM_CONSTANT_Fieldref:
    case JVM_CONSTANT_Methodref:
    case JVM_CONSTANT_Integer:
    case JVM_CONSTANT_Float:
      // Do nothing!  Not an oop.
      // These constant types do not reference symbols at this point.
      break;

    case JVM_CONSTANT_String:
      // Do nothing!  Not a symbol.
      break;

    case JVM_CONSTANT_UnresolvedString:
    case JVM_CONSTANT_Utf8:
      // These constants are symbols, but unless these symbols are
      // actually to be used for something, we don't want to mark them.
      break;

    case JVM_CONSTANT_Long:
    case JVM_CONSTANT_Double:
      // Do nothing!  Not an oop. (But takes two pool entries.)
      ++index;
      break;

    default:
      ShouldNotReachHere();
      break;
    }
  }
}


// Iterate over the [one] tags array (in preparation for writing to the
// shared archive).

void constantPoolOopDesc::shared_tags_iterate(OopClosure* closure) {
  closure->do_oop(tags_addr());
}


// Iterate over String objects (in preparation for writing to the shared
// archive).

void constantPoolOopDesc::shared_strings_iterate(OopClosure* closure) {
  for (int index = 1; index < length(); index++) { // Index 0 is unused
    switch (tag_at(index).value()) {

    case JVM_CONSTANT_UnresolvedClass:
    case JVM_CONSTANT_NameAndType:
      // Do nothing!  Not a String.
      break;

    case JVM_CONSTANT_Class:
    case JVM_CONSTANT_InterfaceMethodref:
    case JVM_CONSTANT_Fieldref:
    case JVM_CONSTANT_Methodref:
    case JVM_CONSTANT_Integer:
    case JVM_CONSTANT_Float:
      // Do nothing!  Not an oop.
      // These constant types do not reference symbols at this point.
      break;

    case JVM_CONSTANT_String:
      closure->do_oop(obj_at_addr(index));
      break;

    case JVM_CONSTANT_UnresolvedString:
    case JVM_CONSTANT_Utf8:
      // These constants are symbols, but unless these symbols are
      // actually to be used for something, we don't want to mark them.
      break;

    case JVM_CONSTANT_Long:
    case JVM_CONSTANT_Double:
      // Do nothing!  Not an oop. (But takes two pool entries.)
      ++index;
      break;

    default:
      ShouldNotReachHere();
      break;
    }
  }
}


#ifndef PRODUCT

const char* constantPoolOopDesc::printable_name_at(int which) {

  constantTag tag = tag_at(which);

  if (tag.is_unresolved_string() || tag.is_string()) {
    return string_at_noresolve(which);
  } else if (tag.is_klass() || tag.is_unresolved_klass()) {
    return klass_name_at(which)->as_C_string();
  } else if (tag.is_symbol()) {
    return symbol_at(which)->as_C_string();
  }
  return "";
}

#endif // PRODUCT


