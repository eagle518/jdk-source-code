#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)placeholders.hpp	1.5 03/12/23 16:41:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PlaceholderEntry;

// Placeholder objects. These represent classes currently
// being loaded, as well as arrays of primitives.
//

class PlaceholderTable : public TwoOopHashtable {
  friend class VMStructs;

public:
  PlaceholderTable(int table_size);

  PlaceholderEntry* new_entry(int hash, symbolOop name, oop loader);

  PlaceholderEntry* bucket(int i) {
    return (PlaceholderEntry*)Hashtable::bucket(i);
  }

  PlaceholderEntry** bucket_addr(int i) {
    return (PlaceholderEntry**)Hashtable::bucket_addr(i);
  }

  void add_entry(int index, PlaceholderEntry* new_entry) {
    Hashtable::add_entry(index, (HashtableEntry*)new_entry);
  }

  void add_entry(int index, unsigned int hash,
                 symbolHandle name, Handle loader);

  symbolOop find_entry(int index, unsigned int hash,
                       symbolHandle name, Handle loader);


  symbolOop find_and_add(int index, unsigned int hash,
                         symbolHandle name, Handle loader) {
    symbolOop ph_check = find_entry(index, hash, name, loader);
    if (ph_check == NULL) {
      // Nothing found, add place holder
      add_entry(index, hash, name, loader);
    }
    return ph_check;
  }

  void remove_entry(int index, unsigned int hash,
                    symbolHandle name, Handle loader);


  void find_and_remove(int index, unsigned int hash,
                       symbolHandle name, Handle loader) {
    // Check for placeholder and remove if there.
    // This method is typically called for failure cleanups.
    symbolOop check = find_entry(index, hash, name, loader);
    if (check != NULL) {
      remove_entry(index, hash, name, loader);
    }
  }


  // GC support.

  void oops_do(OopClosure* f);
  void prim_array_classes_do(void f(klassOop, oop));

#ifndef PRODUCT
  void print();
  void verify();
#endif
};

// Placeholder objects represent classes currently being loaded.
// All threads examining the placeholder table must hold the
// SystemDictionary_lock, so we don't need special precautions
// on store ordering here.
// The system dictionary is the only user of this class.

class PlaceholderEntry : public HashtableEntry {
  friend class VMStructs;
 private:
  oop               _loader;

 public:
  // Simple accessors, used only by SystemDictionary
  symbolOop          klass()       const { return (symbolOop)literal(); }
  symbolOop*         klass_addr()  { return (symbolOop*)literal_addr(); }

  oop                loader()      const { return _loader; }
  void               set_loader(oop loader) { _loader = loader; }
  oop*               loader_addr() { return &_loader; }

  PlaceholderEntry* next() const {
    return (PlaceholderEntry*)HashtableEntry::next();
  }

  PlaceholderEntry** next_addr() {
    return (PlaceholderEntry**)HashtableEntry::next_addr();
  }

  // Test for equality
  bool equals(symbolOop class_name, oop class_loader) const {
    return (klass() == class_name && loader() == class_loader);
  }

  // GC support
  // Applies "f->do_oop" to all root oops in the placeholder table.
  void oops_do(OopClosure* blk);

  // Print method doesn't append a cr
  void print() const  PRODUCT_RETURN;
  void verify() const PRODUCT_RETURN;
};

