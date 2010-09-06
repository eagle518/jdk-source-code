#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)placeholders.cpp	1.5 03/12/23 16:41:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

# include "incls/_precompiled.incl"
# include "incls/_placeholders.cpp.incl"

// Placeholder methods

PlaceholderEntry* PlaceholderTable::new_entry(int hash, symbolOop name,
                                              oop loader) {
  PlaceholderEntry* entry = (PlaceholderEntry*)Hashtable::new_entry(hash, name);
  entry->set_loader(loader);
  return entry;
}


// Placeholder objects represent classes currently being loaded.
// All threads examining the placeholder table must hold the
// SystemDictionary_lock, so we don't need special precautions
// on store ordering here.
void PlaceholderTable::add_entry(int index, unsigned int hash,
                                 symbolHandle class_name, Handle class_loader){
  assert_locked_or_safepoint(SystemDictionary_lock);
  assert(!class_name.is_null(), "adding NULL obj");

  // Both readers and writers are locked so it's safe to just
  // create the placeholder and insert it in the list without a membar.
  PlaceholderEntry* entry = new_entry(hash, class_name(), class_loader());
  add_entry(index, entry);
}


// Remove a placeholder object. 
void PlaceholderTable::remove_entry(int index, unsigned int hash,
                                    symbolHandle class_name, 
                                    Handle class_loader) {
  assert_locked_or_safepoint(SystemDictionary_lock);
  PlaceholderEntry** p = bucket_addr(index);
  while (*p) {
    PlaceholderEntry *probe = *p;
    if (probe->hash() == hash && probe->equals(class_name(), class_loader())) {
      // Delete entry
      *p = probe->next();
      free_entry(probe);
      return;
    }
    p = probe->next_addr();
  }
}


symbolOop PlaceholderTable::find_entry(int index, unsigned int hash,
                                       symbolHandle class_name,
                                       Handle class_loader) {
  assert_locked_or_safepoint(SystemDictionary_lock);

  symbolOop class_name_ = class_name();
  oop class_loader_ = class_loader();

  for (PlaceholderEntry *place_probe = bucket(index);
                         place_probe != NULL;
                         place_probe = place_probe->next()) {
    if (place_probe->hash() == hash &&
        place_probe->equals(class_name_, class_loader_)) {
      return place_probe->klass();
    }
  }
  return NULL;
}


PlaceholderTable::PlaceholderTable(int table_size)
    : TwoOopHashtable(table_size, sizeof(PlaceholderEntry)) {
}


void PlaceholderTable::oops_do(OopClosure* f) {
  for (int index = 0; index < table_size(); index++) {
    for (PlaceholderEntry* probe = bucket(index); 
                           probe != NULL; 
                           probe = probe->next()) {
      probe->oops_do(f);
    }
  }
}


void PlaceholderEntry::oops_do(OopClosure* blk) {
  assert(klass() != NULL, "should have a non-null klass");
  blk->do_oop((oop*)klass_addr());
  if (_loader != NULL) {
    blk->do_oop(loader_addr());
  }
}


//   Do all arrays of primitive types
//  These are stored in the system dictionary as placeholders
//  and are used by jvmti
void PlaceholderTable::prim_array_classes_do(void f(klassOop, oop)) {
  for (int index = 0; index < table_size(); index++) {
    for (PlaceholderEntry* probe = bucket(index); 
                           probe != NULL; 
                           probe = probe->next()) {
      symbolOop klass_name = probe->klass();
      // array of primitive arrays are stored in system dictionary as placeholders
      if (FieldType::is_array(klass_name)) {
        jint dimension;
        Thread *thread = Thread::current();
        symbolOop object_key;

        BasicType t = FieldType::get_array_info(klass_name,
                                                &dimension, 
                                                &object_key, 
                                                thread);
        if (t != T_OBJECT) {
          klassOop array_klass = Universe::typeArrayKlassObj(t);
          array_klass = typeArrayKlass::cast(array_klass)->array_klass_or_null(dimension);
          f(array_klass, probe->loader());
        }
      }
    }
  }
}


#ifndef PRODUCT
// Note, doesn't append a cr
void PlaceholderEntry::print() const {
  klass()->print_value();
  if (loader() != NULL) {
    tty->print(", loader ");
    loader()->print_value();
  }
}


void PlaceholderEntry::verify() const {
  guarantee(loader() == NULL || loader()->is_instance(), 
            "checking type of _loader");
  klass()->verify();
}


void PlaceholderTable::verify() {
  int element_count = 0;
  for (int pindex = 0; pindex < table_size(); pindex++) {
    for (PlaceholderEntry* probe = bucket(pindex); 
                           probe != NULL; 
                           probe = probe->next()) {
      probe->verify();
      element_count++;  // both klasses and place holders count
    }
  }
  guarantee(number_of_entries() == element_count,
            "Verify of system dictionary failed");
}


void PlaceholderTable::print() {
  for (int pindex = 0; pindex < table_size(); pindex++) {    
    for (PlaceholderEntry* probe = bucket(pindex);
                           probe != NULL; 
                           probe = probe->next()) {
      if (Verbose) tty->print("%4d: ", pindex);
      tty->print(" place holder ");

      probe->print();
      tty->cr();
    }
  }
}
#endif


