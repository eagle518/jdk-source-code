#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)objArrayKlassKlass.cpp	1.68 03/12/23 16:42:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_objArrayKlassKlass.cpp.incl"

klassOop objArrayKlassKlass::create_klass(TRAPS) {
  objArrayKlassKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());  
  KlassHandle k = base_create_klass(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  assert(k()->size() == align_object_size(header_size()), "wrong size for object");
  java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}

klassOop objArrayKlassKlass::allocate_system_objArray_klass(TRAPS) {
  // system_objArrays have no instance klass, so allocate with fake class, then reset to NULL
  KlassHandle kk(THREAD, Universe::intArrayKlassObj());
  klassOop k = allocate_objArray_klass(1, kk, CHECK_0);
  objArrayKlass* tk = (objArrayKlass*) k->klass_part();
  tk->set_element_klass(NULL);
  tk->set_bottom_klass(NULL);
  return k;
}


klassOop objArrayKlassKlass::allocate_objArray_klass(int n, KlassHandle element_klass, TRAPS) {
  objArrayKlassKlassHandle this_oop(THREAD, as_klassOop());
  return allocate_objArray_klass_impl(this_oop, n, element_klass, THREAD);
}

klassOop objArrayKlassKlass::allocate_objArray_klass_impl(objArrayKlassKlassHandle this_oop, 
                                                          int n, KlassHandle element_klass, TRAPS) {

  // Eagerly allocate the direct array supertype.
  KlassHandle super_klass = KlassHandle();
  if (!Universe::is_bootstrapping()) {
    KlassHandle element_super (THREAD, element_klass->super());
    if (element_super.not_null()) {
      // The element type has a direct super.  E.g., String[] has direct super of Object[].
      super_klass = KlassHandle(THREAD, element_super->array_klass_or_null());
      bool supers_exist = super_klass.not_null();
      // Also, see if the element has secondary supertypes.
      // We need an array type for each.
      objArrayHandle element_supers = objArrayHandle(THREAD, 
                                            element_klass->secondary_supers());
      for( int i = element_supers->length()-1; i >= 0; i-- ) {
	klassOop elem_super = (klassOop) element_supers->obj_at(i);
	if (Klass::cast(elem_super)->array_klass_or_null() == NULL) {
	  supers_exist = false;
	  break;
	}
      }
      if (!supers_exist) {
	// Oops.  Not allocated yet.  Back out, allocate it, and retry.
#ifndef PRODUCT
	if (WizardMode) {
          tty->print_cr("Must retry array klass creation for depth %d",n);
        }
#endif
        KlassHandle ek;
        {
	  MutexUnlocker mu(MultiArray_lock);
	  MutexUnlocker mc(Compile_lock);   // for vtables
	  klassOop sk = element_super->array_klass(CHECK_0);
	  super_klass = KlassHandle(THREAD, sk);
	  for( int i = element_supers->length()-1; i >= 0; i-- ) {
	    KlassHandle elem_super (THREAD, element_supers->obj_at(i));
	    elem_super->array_klass(CHECK_0);
	  }
          // Now retry from the beginning
          klassOop klass_oop = element_klass->array_klass(n, CHECK_0);
          // Create a handle because the enclosing brace, when locking
          // can cause a gc.  Better to have this function return a Handle.
          ek = KlassHandle(THREAD, klass_oop);
        }  // re-lock
        return ek();
      }
    } else {
      // The element type is already Object.  Object[] has direct super of Object.
      super_klass = KlassHandle(THREAD, SystemDictionary::object_klass());
    }
  }

  // Create type name for klass (except for symbol arrays, since symbolKlass
  // does not have a name).  This will potentially allocate an object, cause
  // GC, and all other kinds of things.  Hence, this must be done before we
  // get a handle to the new objArrayKlass we want to construct.  We cannot
  // block while holding a handling to a partly initialized object.
  symbolHandle name = symbolHandle();
  
  if (!element_klass->oop_is_symbol()) {
    ResourceMark rm(THREAD);
    char *name_str = element_klass->name()->as_C_string();
    int len = element_klass->name()->utf8_length();
    char *new_str = NEW_RESOURCE_ARRAY(char, len + 4);
    int idx = 0;
    new_str[idx++] = '[';
    if (element_klass->oop_is_instance()) { // it could be an array or simple type
      new_str[idx++] = 'L';
    } 
    memcpy(&new_str[idx], name_str, len * sizeof(char));
    idx += len;
    if (element_klass->oop_is_instance()) {
      new_str[idx++] = ';';
    }
    new_str[idx++] = '\0';
    name = oopFactory::new_symbol_handle(new_str, CHECK_0);    
  } 

  objArrayKlass o;  
  arrayKlassHandle k = arrayKlass::base_create_array_klass(o.vtbl_value(), 
                                                           objArrayKlass::header_size(), 
                                                          this_oop, 
                                                           CHECK_0);


  // Initialize instance variables  
  objArrayKlass* oak = objArrayKlass::cast(k());
  oak->set_dimension(n);
  oak->set_element_klass(element_klass());
  oak->set_name(name());
  oak->set_is_objArray(true);

  klassOop bk;
  if (element_klass->oop_is_objArray()) {
    bk = objArrayKlass::cast(element_klass())->bottom_klass();
  } else {
    bk = element_klass();
  }
  assert(bk != NULL && (Klass::cast(bk)->oop_is_instance() || Klass::cast(bk)->oop_is_typeArray()), "invalid bottom klass");
  oak->set_bottom_klass(bk);

  oak->set_array_header_in_bytes(objArrayOopDesc::header_size() * wordSize);
  oak->set_size_helper(-1-exact_log2(wordSize));

  // Call complete_create_array_klass after all instance variables has been initialized.
  arrayKlass::complete_create_array_klass(k, super_klass, CHECK_0);

  return k();
}


void objArrayKlassKlass::oop_follow_contents(oop obj) {
  assert(obj->is_klass(), "must be klass");
  assert(klassOop(obj)->klass_part()->oop_is_objArray(), "must be obj array");

  objArrayKlass* oak = objArrayKlass::cast((klassOop)obj);
  MarkSweep::mark_and_push(oak->element_klass_addr());
  MarkSweep::mark_and_push(oak->bottom_klass_addr());

  arrayKlassKlass::oop_follow_contents(obj);
}



int objArrayKlassKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_klass(), "must be klass");
  assert(klassOop(obj)->klass_part()->oop_is_objArray(), "must be obj array");

  objArrayKlass* oak = objArrayKlass::cast((klassOop)obj);
  MarkSweep::adjust_pointer(oak->element_klass_addr());
  MarkSweep::adjust_pointer(oak->bottom_klass_addr());

  return arrayKlassKlass::oop_adjust_pointers(obj);
}



int objArrayKlassKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert(obj->is_klass(), "must be klass");
  assert(klassOop(obj)->klass_part()->oop_is_objArray(), "must be obj array");

  objArrayKlass* oak = objArrayKlass::cast((klassOop)obj);
  blk->do_oop(oak->element_klass_addr());
  blk->do_oop(oak->bottom_klass_addr());

  return arrayKlassKlass::oop_oop_iterate(obj, blk);
}


int objArrayKlassKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert(obj->is_klass(), "must be klass");
  assert(klassOop(obj)->klass_part()->oop_is_objArray(), "must be obj array");

  objArrayKlass* oak = objArrayKlass::cast((klassOop)obj);
  oop* addr;
  addr = oak->element_klass_addr();
  if (mr.contains(addr)) blk->do_oop(addr);
  addr = oak->bottom_klass_addr();
  if (mr.contains(addr)) blk->do_oop(addr);

  return arrayKlassKlass::oop_oop_iterate(obj, blk);
}

void objArrayKlassKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->blueprint()->oop_is_objArrayKlass(),"must be an obj array klass");
}

#ifndef PRODUCT

// Printing

void objArrayKlassKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_klass(), "must be klass");
  objArrayKlass* oak = (objArrayKlass*) klassOop(obj)->klass_part();
  klassKlass::oop_print_on(obj, st);
  st->print(" - instance klass: ");
  oak->element_klass()->print_value_on(st);
  st->cr();
}


void objArrayKlassKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_klass(), "must be klass");
  objArrayKlass* oak = (objArrayKlass*) klassOop(obj)->klass_part();

  oak->element_klass()->print_value_on(st);
  st->print("[]");
}


const char* objArrayKlassKlass::internal_name() const {
  return "{object array class}";
}


// Verification

void objArrayKlassKlass::oop_verify_on(oop obj, outputStream* st) {
  klassKlass::oop_verify_on(obj, st);
  objArrayKlass* oak = objArrayKlass::cast((klassOop)obj);
  guarantee(oak->element_klass()->is_perm(),  "should be in permspace");
  guarantee(oak->element_klass()->is_klass(), "should be klass");
  guarantee(oak->bottom_klass()->is_perm(),   "should be in permspace");
  guarantee(oak->bottom_klass()->is_klass(),  "should be klass");
  Klass* bk = Klass::cast(oak->bottom_klass());
  guarantee(bk->oop_is_instance() || bk->oop_is_typeArray(),  "invalid bottom klass");
}
#endif


