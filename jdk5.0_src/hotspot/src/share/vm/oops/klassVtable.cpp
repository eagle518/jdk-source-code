#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)klassVtable.cpp	1.115 04/05/04 14:59:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_klassVtable.cpp.incl"

inline instanceKlass* klassVtable::ik() const {
  Klass* k = _klass()->klass_part();
  assert(k->oop_is_instance(), "not an instanceKlass");
  return (instanceKlass*)k;
}

inline bool needs_vtable_entry(methodHandle m, symbolHandle init, bool is_final_class) {
  if (is_final_class || m->is_final()
    // (note: can't call m->is_final_method() because class may not exist yet -- called from loader)
    // if m overrides a superclass method, we don't need a new vtable entry because it's there already;
    // if m is newly introduced here, we also don't need one because it's called statically
    || m->is_static()
    // static methods don't need to be in vtable
    || m->name() == init()
    // <init> is never called dynamically-bound
    ) {
    return false;
  }
  return true;
}


// this function computes the vtable size (including the size needed for miranda
// methods) and the number of miranda methods in this class
// Note on Miranda methods: Let's say there is a class C that implements
// interface I.  Let's say there is a method m in I that neither C nor any 
// of its super classes implement (i.e there is no method of any access, with
// the same name and signature as m), then m is a Miranda method which is
// entered as a public abstract method in C's vtable.  From then on it should
// treated as any other public method in C for method over-ride purposes.
void klassVtable::compute_vtable_size_and_num_mirandas(int &vtable_length,
						       int &num_miranda_methods,
						       klassOop super,
						       objArrayOop methods, 
						       AccessFlags class_flags, 
						       oop classloader,
						       symbolOop classname,
						       objArrayOop local_interfaces
						       ) {

  No_Safepoint_Verifier nsv;

  // set up default result values
  vtable_length = 0;
  num_miranda_methods = 0;

  // start off with super's vtable length
  instanceKlass* sk = (instanceKlass*)super->klass_part();
  vtable_length = super == NULL ? 0 : sk->vtable_length();
  
  // go thru each method in the methods table to see if it needs a new entry
  int len = methods->length();
  for (int i = 0; i < len; i++) {
    assert(methods->obj_at(i)->is_method(), "must be a methodOop");
    methodOop m = methodOop(methods->obj_at(i));
    
    if (needs_new_vtable_entry(m, super, classloader, classname, class_flags)) {
      vtable_length += vtableEntry::size(); // we need a new entry      
    }
  }
  
  // compute the number of mirandas methods that must be added to the end
  num_miranda_methods = get_num_mirandas(super, methods, local_interfaces);
  vtable_length += (num_miranda_methods * vtableEntry::size());
  
  if (Universe::is_bootstrapping() && vtable_length == 0) {
    // array classes don't have their superclass set correctly during 
    // bootstrapping
    vtable_length = Universe::base_vtable_size();
  }

  assert(super != NULL || vtable_length == Universe::base_vtable_size(),
         "bad vtable size for class Object");
  assert(vtable_length % vtableEntry::size() == 0, "bad vtable length");
  assert(vtable_length >= Universe::base_vtable_size(), "vtable too small");
}

int klassVtable::index_of(methodOop m, int len) const {
  return m->vtable_index();
}

int klassVtable::initialize_from_super(KlassHandle super) {
  if (super == NULL) {
    return 0;
  } else {
    // copy methods from superKlass
    // can't inherit from array class, so must be instanceKlass
    assert(super->oop_is_instance(), "must be instance klass");
    instanceKlass* sk = (instanceKlass*)super()->klass_part();
    klassVtable* superVtable = sk->vtable();
    assert(superVtable->length() <= _length, "vtable too short");
#ifdef ASSERT
    superVtable->verify(tty, true);
#endif
    superVtable->copy_vtable_to(table());
#ifndef PRODUCT
    if (PrintVtables && Verbose) {  
      tty->print_cr("copy vtable from %s to %s size %d", sk->internal_name(), klass()->internal_name(), _length);
    }
#endif
    return superVtable->length();
  }
}

// Revised lookup semantics   introduced 1.3 (Kestral beta)
void klassVtable::initialize_vtable(Thread *thread) {  
      
  // Note:  Arrays can have intermediate array supers.  Use java_super to skip them.
  KlassHandle super (thread, klass()->java_super());
  int nofNewEntries = 0;
 

  if (PrintVtables && !klass()->oop_is_array()) {
    ResourceMark rm(thread);      
    tty->print_cr("Initializing: %s", _klass->name()->as_C_string());    
  }  

#ifdef ASSERT
  oop* end_of_obj = (oop*)_klass() + _klass()->size();
  oop* end_of_vtable = (oop*)&table()[_length];
  assert(end_of_vtable <= end_of_obj, "vtable extends beyond end");
#endif

  if (Universe::is_bootstrapping()) {    
    // just clear everything
    for (int i = 0; i < _length; i++) table()[i].clear();
    return;
  }

  int super_vtable_len = initialize_from_super(super);
  if (klass()->oop_is_array()) {
    assert(super_vtable_len == _length, "arrays shouldn't introduce new methods");
  } else {
    assert(_klass->oop_is_instance(), "must be instanceKlass");
    
    objArrayOop methods = ik()->methods();    
    int len = methods->length();
    int initialized = super_vtable_len;    

    for (int i = 0; i < len; i++) {
      HandleMark hm(thread);
      assert(methods->obj_at(i)->is_method(), "must be a methodOop");    
      methodOop m = (methodOop)methods->obj_at(i);

      bool needs_new_entry = update_super_vtable(ik(), m, super_vtable_len);

      if (needs_new_entry) {
        put_method_at(m, initialized);
	m->set_vtable_index(initialized); // set primary vtable index
	initialized++;        
      } 
    }

    // add miranda methods; it will also update the value of initialized
    fill_in_mirandas(initialized);
  
    // In class hierachieswhere the accesibility is not increasing (i.e., going from private -> 
    // package_private -> publicprotected), the vtable might actually be smaller than our initial
    // calculation. 
    assert(initialized <= _length, "vtable initialization failed");    
    for(;initialized < _length; initialized++) {
      put_method_at(NULL, initialized);
    }
    verify(tty, true);
  }
}

// Interates through the vtables to find the broadest access level. This
// will always be monotomic for valid Java programs - but not neccesarily
// for incompatible class files.
klassVtable::AccessType klassVtable::vtable_accessibility_at(int i) {
  // This vtable is not implementing the specific method
  if (i >= length()) return acc_private;
      
  // Compute AccessType for current method. public or protected we are done.
  methodOop m = method_at(i);
  if (m->is_protected() || m->is_public()) return acc_publicprotected;
  
  AccessType acc = m->is_package_private() ? acc_package_private : acc_private;

  // Compute AccessType for method in super classes
  klassOop super = klass()->super();  
  AccessType super_acc = (super != NULL) ? instanceKlass::cast(klass()->super())->vtable()->vtable_accessibility_at(i)
                                         : acc_private;
  
  // Merge
  return (AccessType)MAX2((int)acc, (int)super_acc);
}


bool klassVtable::update_super_vtable(instanceKlass* klass, methodOop target_method, int super_vtable_len) {    
  ResourceMark rm;
  bool allocate_new = true;

  // Static and <init> methods are never in 
  if (target_method->is_static() || target_method->name() ==  vmSymbols::object_initializer_name()) {        
    return false;
  }

  if (klass->is_final() || target_method->is_final()) {
    // a final method never needs a new entry; final methods can be statically
    // resolved and they have to be present in the vtable only if they override
    // a super's method, in which case they re-use its entry          
    allocate_new = false;
  }      

  // we need a new entry if there is no superclass
  if (klass->super() == NULL) {
    return allocate_new;
  }

  // private methods always have a new entry in the vtable
  if (target_method->is_private()) {
    return allocate_new;
  }
  
  // search through the vtable and update overridden entries
  symbolOop name = target_method->name();
  symbolOop signature = target_method->signature();    
  for(int i = 0; i < super_vtable_len; i++) {
    methodOop match_method = method_at(i);
    // Check if method name matches
    if (match_method->name() == name && match_method->signature() == signature) {
      
      instanceKlass* holder = instanceKlass::cast(match_method->method_holder());

      // Check if the match_method is accessable from current class
      
      bool same_package_init = false;
      bool same_package_flag = false;
      bool simple_match = match_method->is_public()  || match_method->is_protected();
      if (!simple_match) {
        same_package_init = true;
        same_package_flag = holder->is_same_class_package(klass->class_loader(), klass->name());

        simple_match = match_method->is_package_private() && same_package_flag;
      }
      // A simple form of this statement is:
      // if ( (match_method->is_public()  || match_method->is_protected()) ||
      //    (match_method->is_package_private() && holder->is_same_class_package(klass->class_loader(), klass->name()))) {
      //
      // The complexity is introduced it avoid recomputing 'is_same_class_package' which is expensive.
      if (simple_match) {      
        // Check if target_method and match_method has same level of accessibility. The accesibility of the
        // match method is the "most-general" visibility of all entries at it's particular vtable index for
        // all superclasses. This check must be done before we override the current entry in the vtable.
        AccessType at = vtable_accessibility_at(i);        

        // Method is accessible, override with target method
        put_method_at(target_method, i);

        if (  (at == acc_publicprotected && (target_method->is_public() || target_method->is_protected()) 
           || (at == acc_package_private && (target_method->is_package_private() &&
                                            (( same_package_init && same_package_flag) ||
                                             (!same_package_init && holder->is_same_class_package(klass->class_loader(), klass->name()))))))) {

          // target and match has same accessiblity - share entry  
          allocate_new = false;
          target_method->set_vtable_index(i);            
        }
       
      }
    }
  }
  return allocate_new;
}
      


void klassVtable::put_method_at(methodOop m, int index) {
#ifndef PRODUCT
  if (PrintVtables && Verbose) {  
    tty->print_cr("adding %s::%s at index %d", _klass->internal_name(), 
      (m != NULL) ? m->name()->as_C_string() : "<NULL>", index);
  }
#endif
  table()[index].set(m);
}

// Find out if a method "m" with superclass "super", loader "classloader" and 
// name "classname" needs a new vtable entry.  Let P be a class package defined
// by "classloader" and "classname".
// NOTE: The logic used here is very similar to the one used for computing
// the vtables indices for a method. We cannot directly use that function because,
// when the Universe is boostrapping, a super's vtable might not be initialized.
bool klassVtable::needs_new_vtable_entry(methodOop target_method, 
					 klassOop super, 
					 oop classloader,
					 symbolOop classname,
					 AccessFlags class_flags) {
  if ((class_flags.is_final() || target_method->is_final()) ||
      // a final method never needs a new entry; final methods can be statically
      // resolved and they have to be present in the vtable only if they override
      // a super's method, in which case they re-use its entry
      (target_method->is_static()) ||
      // static methods don't need to be in vtable
      (target_method->name() ==  vmSymbols::object_initializer_name())
      // <init> is never called dynamically-bound
      ) {
    return false;
  }

  // we need a new entry if there is no superclass
  if (super == NULL) {
    return true;
  }
  
  // private methods always have a new entry in the vtable
  if (target_method->is_private()) {
    return true;
  }

  // search through the super class hierarchy to see if we need
  // a new entry
  symbolOop name = target_method->name();
  symbolOop signature = target_method->signature();
  klassOop k = super;
  methodOop match_method = NULL;
  instanceKlass *holder = NULL;
  while (k != NULL) {
    // lookup through the hierarchy for a method with matching name and sign.
    match_method = instanceKlass::cast(k)->lookup_method(name, signature);
    if (match_method == NULL) {
      break; // we still have to search for a matching miranda method
    }
    // get the class holding the matching method
    holder = instanceKlass::cast(match_method->method_holder());

    if (!match_method->is_static()) { // we want only instance method matches
      if ((target_method->is_public() || target_method->is_protected()) &&
	  (match_method->is_public()  || match_method->is_protected())) {
	// target and match are public/protected; we do not need a new entry
        return false;
      }

      if (target_method->is_package_private() &&
	  match_method->is_package_private() &&
	  holder->is_same_class_package(classloader, classname)) {
	// target and match are P private; we do not need a new entry
        return false;
      }
    }
    
    k = holder->super(); // haven't found a match yet; continue to look
  }
  
  // if the target method is public or protected it may have a matching
  // miranda method in the super, whose entry it should re-use.
  if (target_method->is_public() || target_method->is_protected()) {
    instanceKlass *sk = instanceKlass::cast(super);
    if (sk->has_miranda_methods()) {
      if (sk->lookup_method_in_all_interfaces(name, signature) != NULL) {
	return false;  // found a matching miranda; we do not need a new entry
      }
    }
  }
  
  return true; // found no match; we need a new entry
}

// Support for miranda methods

// get the vtable index of a miranda method with matching "name" and "signature"
int klassVtable::index_of_miranda(symbolOop name, symbolOop signature) {
  // search from the bottom, might be faster
  for (int i = (length() - 1); i >= 0; i--) {
    methodOop m = table()[i].method();
    if (is_miranda_entry_at(i) &&
        m->name() == name && m->signature() == signature) {
      return i;
    }
  }
  return -1;
}

// check if an entry is miranda
bool klassVtable::is_miranda_entry_at(int i) {
  methodOop m = method_at(i);
  klassOop method_holder = m->method_holder();
  instanceKlass *mhk = instanceKlass::cast(method_holder);
  
  // miranda methods are interface methods in a class's vtable
  if (mhk->is_interface()) {  
    assert(m->is_public() && m->is_abstract(), "should be public and abstract");
    assert(ik()->implements_interface(method_holder) , "this class should implement the interface");
    assert(is_miranda(m, ik()->methods(), ik()->super()), "should be a miranda_method");
    return true;
  }
  return false;
}

// check if a method is a miranda method, given a class's methods table and it's super
// the caller must make sure that the method belongs to an interface implemented by the class
bool klassVtable::is_miranda(methodOop m, objArrayOop class_methods, klassOop super) {
  symbolOop name = m->name();
  symbolOop signature = m->signature();
  if (instanceKlass::find_method(class_methods, name, signature) == NULL) {
     // did not find it in the method table of the current class
    if (super == NULL) {
      // super doesn't exist
      return true;
    } else {
      if (instanceKlass::cast(super)->lookup_method(name, signature) == NULL) {
	// super class hierarchy does not implement it
	return true;
      }
    } 
  }
  return false; 
}

void klassVtable::add_new_mirandas_to_list(GrowableArray<methodOop>* list_of_current_mirandas,
					   objArrayOop current_interface_methods,
					   objArrayOop class_methods,
					   klassOop super) {
  // iterate thru the current interface's method to see if it a miranda
  int num_methods = current_interface_methods->length();
  for (int i = 0; i < num_methods; i++) {
    methodOop im = methodOop(current_interface_methods->obj_at(i));
    bool is_duplicate = false;
    int num_of_current_mirandas = list_of_current_mirandas->length();
    // check for duplicate mirandas in different interfaces we implement
    for (int j = 0; j < num_of_current_mirandas; j++) {
      methodOop miranda = list_of_current_mirandas->at(j);
      if ((im->name() == miranda->name()) &&
	  (im->signature() == miranda->signature())) {
	is_duplicate = true;
	break;
      }
    }
    
    if (!is_duplicate) { // we don't want duplicate miranda entries in the vtable
      if (is_miranda(im, class_methods, super)) { // is it a miranda at all?
	instanceKlass *sk = instanceKlass::cast(super);
	// check if it is a duplicate of a super's miranda
	if (sk->lookup_method_in_all_interfaces(im->name(), im->signature()) == NULL) {
	  list_of_current_mirandas->append(im);
	}
      }
    }
  }
}

void klassVtable::get_mirandas(GrowableArray<methodOop>* mirandas,
			       klassOop super, objArrayOop class_methods, 
			       objArrayOop local_interfaces) {
  assert((mirandas->length() == 0) , "current mirandas must be 0");
  
  // iterate thru the local interfaces looking for a miranda
  int num_local_ifs = local_interfaces->length();
  for (int i = 0; i < num_local_ifs; i++) {
    instanceKlass *ik = instanceKlass::cast(klassOop(local_interfaces->obj_at(i)));
    add_new_mirandas_to_list(mirandas, ik->methods(), class_methods, super);
    // iterate thru each local's super interfaces
    objArrayOop super_ifs = ik->transitive_interfaces();
    int num_super_ifs = super_ifs->length();
    for (int j = 0; j < num_super_ifs; j++) {
      instanceKlass *sik = instanceKlass::cast(klassOop(super_ifs->obj_at(j)));
      add_new_mirandas_to_list(mirandas, sik->methods(), class_methods, super);
    }
  }
}

// get number of mirandas
int klassVtable::get_num_mirandas(klassOop super, objArrayOop class_methods, objArrayOop local_interfaces) {
  ResourceMark rm;
  GrowableArray<methodOop>* mirandas = new GrowableArray<methodOop>(20);
  get_mirandas(mirandas, super, class_methods, local_interfaces);
  return mirandas->length();
}

// fill in mirandas
void klassVtable::fill_in_mirandas(int& initialized) {
  ResourceMark rm;
  GrowableArray<methodOop>* mirandas = new GrowableArray<methodOop>(20);
  instanceKlass *this_ik = ik();
  get_mirandas(mirandas, this_ik->super(), this_ik->methods(), this_ik->local_interfaces());
  int num_mirandas = mirandas->length();
  for (int i = 0; i < num_mirandas; i++) {
    put_method_at(mirandas->at(i), initialized);
    initialized++;
  }
}

void klassVtable::copy_vtable_to(vtableEntry* start) {
  Copy::disjoint_words((HeapWord*)table(), (HeapWord*)start, _length * vtableEntry::size());
}

#ifdef HOTSWAP
void klassVtable::adjust_entries(objArrayOop old_methods, objArrayOop new_methods) {
  for (int j = 0; j < old_methods->length(); j++) {
    methodOop old_method = (methodOop) old_methods->obj_at(j);
    if (! old_method->is_old_version())
      continue;
    // check matching index first
    if (j < length() && unchecked_method_at(j) == old_method) {
      put_method_at((methodOop) new_methods->obj_at(j), j);
    } else {
      for (int i = 0; i < length(); i++) {
        if (unchecked_method_at(i) == old_method) {
          put_method_at((methodOop) new_methods->obj_at(j), i);
          break;
        }
      }
    }
  }
}
#endif HOTSWAP


// Garbage collection
void klassVtable::oop_follow_contents() {
  int len = length();
  for (int i = 0; i < len; i++) {
    MarkSweep::mark_and_push((oop*)&table()[i]._method);
  }
}

void klassVtable::oop_adjust_pointers() {
  int len = length();
  for (int i = 0; i < len; i++) {
    MarkSweep::adjust_pointer((oop*)&table()[i]._method);
  }
}

// Iterators
void klassVtable::oop_oop_iterate(OopClosure* blk) {
  int len = length();
  for (int i = 0; i < len; i++) {
    blk->do_oop((oop*)&table()[i]._method);
  }
}

void klassVtable::oop_oop_iterate_m(OopClosure* blk, MemRegion mr) {
  int len = length();
  int i;
  for (i = 0; i < len; i++) {
    if ((HeapWord*)&table()[i]._method >= mr.start()) break;
  }
  for (; i < len; i++) {  
    oop* adr = (oop*)&table()[i]._method;
    if ((HeapWord*)adr < mr.end()) blk->do_oop(adr);
  }
}

//-----------------------------------------------------------------------------------------
// Itable code

// Initialize a itableMethodEntry
void itableMethodEntry::initialize(methodOop m) {
  if (m == NULL) return;

  _method = m;
}

klassItable::klassItable(instanceKlassHandle klass) {
  _klass = klass;

  if (klass->itable_length() > 0) {
    itableOffsetEntry* offset_entry = (itableOffsetEntry*)klass->start_of_itable();
    if (offset_entry  != NULL && offset_entry->interface_klass() != NULL) { // Check that itable is initialized
      // First offset entry points to the first method_entry
      intptr_t* method_entry  = (intptr_t *)(((address)klass->as_klassOop()) + offset_entry->offset());
      intptr_t* end         = klass->end_of_itable();
  
      _table_offset      = (intptr_t*)offset_entry - (intptr_t*)klass->as_klassOop();
      _size_offset_table = (method_entry - ((intptr_t*)offset_entry)) / itableOffsetEntry::size();
      _size_method_table = (end - method_entry)                  / itableMethodEntry::size();
      assert(_table_offset >= 0 && _size_offset_table >= 0 && _size_method_table >= 0, "wrong computation");
      return;
    }    
  }

  // This lenght of the itable was either zero, or it has not yet been initialized.
  _table_offset      = 0;
  _size_offset_table = 0;
  _size_method_table = 0;
}

// Garbage Collection

void klassItable::oop_follow_contents() {
  // offset table
  itableOffsetEntry* ioe = offset_entry(0);
  for(int i = 0; i < _size_offset_table; i++) {
    MarkSweep::mark_and_push((oop*)&ioe->_interface);
    ioe++;
  }

  // method table
  itableMethodEntry* ime = method_entry(0);
  for(int j = 0; j < _size_method_table; j++) {
    MarkSweep::mark_and_push((oop*)&ime->_method);
    ime++;
  }
}


void klassItable::oop_adjust_pointers() {
  // offset table
  itableOffsetEntry* ioe = offset_entry(0);
  for(int i = 0; i < _size_offset_table; i++) {
    MarkSweep::adjust_pointer((oop*)&ioe->_interface);
    ioe++;
  }

  // method table
  itableMethodEntry* ime = method_entry(0);
  for(int j = 0; j < _size_method_table; j++) {
    MarkSweep::adjust_pointer((oop*)&ime->_method);
    ime++;
  }
}


// Iterators
void klassItable::oop_oop_iterate(OopClosure* blk) {
  // offset table
  itableOffsetEntry* ioe = offset_entry(0);
  for(int i = 0; i < _size_offset_table; i++) {
    blk->do_oop((oop*)&ioe->_interface);
    ioe++;
  }

  // method table
  itableMethodEntry* ime = method_entry(0);
  for(int j = 0; j < _size_method_table; j++) {
    blk->do_oop((oop*)&ime->_method);
    ime++;
  }
}

void klassItable::oop_oop_iterate_m(OopClosure* blk, MemRegion mr) {
  // offset table
  itableOffsetEntry* ioe = offset_entry(0);
  for(int i = 0; i < _size_offset_table; i++) {
    oop* adr = (oop*)&ioe->_interface;
    if (mr.contains(adr)) blk->do_oop(adr);
    ioe++;
  }

  // method table
  itableMethodEntry* ime = method_entry(0);
  for(int j = 0; j < _size_method_table; j++) {
    oop* adr = (oop*)&ime->_method;
    if (mr.contains(adr)) blk->do_oop(adr);
    ime++;
  }
}


static int initialize_count = 0;

// Initialization
void klassItable::initialize_itable() {
  // Cannot be setup doing bootstrapping
  if (Universe::is_bootstrapping()) return;

  int num_interfaces = nof_interfaces();
  if (num_interfaces > 0) {
    if (TraceItables) tty->print_cr("%3d: Initializing itables for %s", ++initialize_count, _klass->name()->as_C_string());
    
    // In debug mode, we got an extra NULL/NULL entry
    debug_only(num_interfaces--);
    assert(num_interfaces > 0, "to few interfaces in offset itable");

    // Interate through all interfaces
    int i;
    for(i = 0; i < num_interfaces; i++) {
      itableOffsetEntry* ioe = offset_entry(i);
      klassOop interf = ioe->interface_klass();      
      itableMethodEntry* ime = ioe->first_method_entry(_klass());
      assert(interf != NULL && ioe->offset() != 0, "bad offet entry in itable");      
      initialize_itable_for_interface(interf, ime);      
    } 

#ifdef ASSERT
    // Check that the last entry is empty
    itableOffsetEntry* ioe = offset_entry(i);
    assert(ioe->interface_klass() == NULL && ioe->offset() == 0, "terminator entry missing");
#endif
  }  
}


void klassItable::initialize_itable_for_interface(klassOop interf, itableMethodEntry* ime) {
  objArrayOop methods = instanceKlass::cast(interf)->methods();
  int nof_methods = methods->length();
  HandleMark hm;
  KlassHandle klass = _klass;
  assert(nof_methods > 0, "at least one method must exist for interface to be in vtable")

  // Skip first methodOop if it is a class initializer
  int i = ((methodOop)methods->obj_at(0))->name() != vmSymbols::class_initializer_name() ? 0 : 1;

  for(; i < nof_methods; i++) {
    methodOop m = (methodOop)methods->obj_at(i);    
    symbolOop method_name      = m->name();
    symbolOop method_signature = m->signature();
      
    // This is same code as in Linkresolver::lookup_instance_method_in_klasses
    methodOop  target = klass->uncached_lookup_method(method_name, method_signature);
    while (target != NULL && target->is_static()) {
      // continue with recursive lookup through the superclass
      klassOop klass = Klass::cast(target->method_holder())->super();
      target = (klass == NULL) ? methodOop(NULL) : Klass::cast(klass)->uncached_lookup_method(method_name, method_signature);
    }      
    if (target == NULL || !target->is_public() || target->is_abstract()) {        
      // Entry do not resolve. Leave it empty
    } else {            
      ime->initialize(target);
    }
    // Progress to next entry
    ime++;    
  }  
}

// Update entry for specic methodOop
void klassItable::initialize_with_method(methodOop m) {
  itableMethodEntry* ime = method_entry(0);
  for(int i = 0; i < _size_method_table; i++) {    
    if (ime->method() == m) {      
      ime->initialize(m);      
    }
    ime++;
  }  
}

#ifdef HOTSWAP
void klassItable::adjust_method_entries(objArrayOop old_methods, objArrayOop new_methods) {
  for (int j = 0; j < old_methods->length(); j++) {
    methodOop old_method = (methodOop) old_methods->obj_at(j);
    if (! old_method->is_old_version())
      continue;
    itableMethodEntry* ime = method_entry(0);
    for (int i = 0; i < _size_method_table; i++) {
      if (ime->method() == old_method) {
	ime->initialize((methodOop) new_methods->obj_at(j));
	break;
      }
      ime++;
    }
  } 
}
#endif HOTSWAP


// Setup
class InterfaceVisiterClosure : public StackObj {
 public:
  virtual void doit(klassOop intf, int method_count) = 0;
};

// Visit all interfaces with at-least one method (excluding <clinit>)
void visit_all_interfaces(objArrayOop transitive_intf, InterfaceVisiterClosure *blk) {  
  // Handle array argument  
  for(int i = 0; i < transitive_intf->length(); i++) {
    klassOop intf = (klassOop)transitive_intf->obj_at(i);
    assert(Klass::cast(intf)->is_interface(), "sanity check");
    
    // Find no. of methods excluding a <clinit>
    int method_count = instanceKlass::cast(intf)->methods()->length();    
    if (method_count > 0) {
      methodOop m = (methodOop)instanceKlass::cast(intf)->methods()->obj_at(0);
      assert(m != NULL && m->is_method(), "sanity check");
      if (m->name() == vmSymbols::object_initializer_name()) {
        method_count--;
      }
    }

    // Only count interfaces with at least one method
    if (method_count > 0) {
      blk->doit(intf, method_count);       
    }    
  }
}

class CountInterfacesClosure : public InterfaceVisiterClosure {
 private:
  int _nof_methods;
  int _nof_interfaces;
 public:
   CountInterfacesClosure() { _nof_methods = 0; _nof_interfaces = 0; }

   int nof_methods() const    { return _nof_methods; }
   int nof_interfaces() const { return _nof_interfaces; }

   void doit(klassOop intf, int method_count) { _nof_methods += method_count; _nof_interfaces++; }
};

class SetupItableClosure : public InterfaceVisiterClosure  {
 private:
  itableOffsetEntry* _offset_entry;
  itableMethodEntry* _method_entry;
  address            _klass_begin;
 public:
  SetupItableClosure(address klass_begin, itableOffsetEntry* offset_entry, itableMethodEntry* method_entry) {
    _klass_begin  = klass_begin;
    _offset_entry = offset_entry;
    _method_entry = method_entry;    
  }

  itableMethodEntry* method_entry() const { return _method_entry; }

  void doit(klassOop intf, int method_count) {
    int offset = ((address)_method_entry) - _klass_begin;
    _offset_entry->initialize(intf, offset);      
    _offset_entry++;
    _method_entry += method_count;
  }
};

int klassItable::compute_itable_size(objArrayHandle transitive_interfaces) {    
  // Count no of interfaces and total number of interface methods
  CountInterfacesClosure cic;  
  visit_all_interfaces(transitive_interfaces(), &cic);
    
  // Add one extra entry in debug mode, so we can null-terminate the table
  int nof_methods    = cic.nof_methods();
  int nof_interfaces = cic.nof_interfaces();
  debug_only(if (nof_interfaces > 0) nof_interfaces++);

  int itable_size = calc_itable_size(nof_interfaces, nof_methods); 

  // Statistics
  update_stats(itable_size * HeapWordSize);  
  
  return itable_size;
}


// Fill out offset table and interface klasses into the itable space
void klassItable::setup_itable_offset_table(instanceKlassHandle klass) {  
  if (klass->itable_length() == 0) return;
  assert(!klass->is_interface(), "Should have zero length itable");
    
  // Count no of interfaces and total number of interface methods  
  CountInterfacesClosure cic;  
  visit_all_interfaces(klass->transitive_interfaces(), &cic);
  int nof_methods    = cic.nof_methods();
  int nof_interfaces = cic.nof_interfaces();
  
  // Add one extra entry in debug mode, so we can null-terminate the table
  debug_only(if (nof_interfaces > 0) nof_interfaces++);
  
  assert(compute_itable_size(objArrayHandle(klass->transitive_interfaces())) ==
         calc_itable_size(nof_interfaces, nof_methods),
         "mismatch calculation of itable size");

  // Fill-out offset table
  itableOffsetEntry* ioe = (itableOffsetEntry*)klass->start_of_itable();
  itableMethodEntry* ime = (itableMethodEntry*)(ioe + nof_interfaces);
  intptr_t* end               = klass->end_of_itable();
  assert((oop*)(ime + nof_methods) <= klass->start_of_static_fields(), "wrong offset calculation (1)");
  assert((oop*)(end) == (oop*)(ime + nof_methods),                     "wrong offset calculation (2)");
  
  // Visit all interfaces and initialize itable offset table
  SetupItableClosure sic((address)klass->as_klassOop(), ioe, ime);
  visit_all_interfaces(klass->transitive_interfaces(), &sic);
    
#ifdef ASSERT
  ime  = sic.method_entry();
  oop* v = (oop*) klass->end_of_itable();
  assert( (oop*)(ime) == v, "wrong offset calculation (2)");
#endif 
}


// m must be a method in an interface
int klassItable::compute_itable_index(methodOop m) {  
  klassOop intf = m->method_holder();
  assert(instanceKlass::cast(intf)->is_interface(), "sanity check");
  objArrayOop methods = instanceKlass::cast(intf)->methods();  
  int index = 0;  
  while(methods->obj_at(index) != m) {
    index++;
    assert(index < methods->length(), "should find index for resolve_invoke");
  }  
  // Adjust for <clinit>, which is left out of table if first method
  if (methods->length() > 0 && ((methodOop)methods->obj_at(0))->name() == vmSymbols::class_initializer_name()) {
    index--;
  }
  return index;
}

//-----------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void klassVtable::verify(outputStream* st, bool forced) {
  // make sure table is initialized
  if (!Universe::is_fully_initialized()) return;
  // avoid redundant verifies
  if (!forced && _verify_count == Universe::verify_count()) return;
  _verify_count = Universe::verify_count();

  oop* end_of_obj = (oop*)_klass() + _klass()->size();
  oop* end_of_vtable = (oop*)&table()[_length];
  if (end_of_vtable > end_of_obj) {
    fatal1("klass %s: klass object too short (vtable extends beyond end)",
          _klass->internal_name());
  }

  for (int i = 0; i < _length; i++) table()[i].verify(this, st);
  // verify consistency with superKlass vtable
  klassOop super = _klass->super();
  if (super) {
    instanceKlass* sk = instanceKlass::cast(super);
    klassVtable* vt = sk->vtable();
    for (int i = 0; i < vt->length(); i++) {
      verify_against(st, vt, i);
    }
  }
}

void klassVtable::verify_against(outputStream* st, klassVtable* vt, int index) {
  vtableEntry* vte = &vt->table()[index];
  if (vte->method()->name()      != table()[index].method()->name() ||
      vte->method()->signature() != table()[index].method()->signature()) {
    fatal("mismatched name/signature of vtable entries");
  }
}

void klassVtable::print() {
  ResourceMark rm;
  tty->print("klassVtable for klass %s (length %d):\n", _klass->internal_name(), length());
  for (int i = 0; i < length(); i++) {
    table()[i].print();
    tty->cr();
  }
}

void vtableEntry::verify(klassVtable* vt, outputStream* st) {
  FlagSetting fs(IgnoreLockingAssertions, true);  
  assert(method() != NULL, "must have set method");
  method()->verify();
  // we sub_type, because it could be a miranda method
  if (!vt->klass()->is_subtype_of(method()->method_holder())) { 
    print();
    fatal1("vtableEntry %#lx: method is from subclass", this);
  }
}

void vtableEntry::print() {
  ResourceMark rm;
  tty->print("vtableEntry %s: ", method()->name()->as_C_string());
  if (Verbose) {
    tty->print("m %#lx ", method());
  }
}

class VtableStats : AllStatic {
 public:
  static int no_klasses;                // # classes with vtables
  static int no_array_klasses;          // # array classes
  static int no_instance_klasses;       // # instanceKlasses
  static int sum_of_vtable_len;         // total # of vtable entries
  static int sum_of_array_vtable_len;   // total # of vtable entries in array klasses only
  static int fixed;                     // total fixed overhead in bytes
  static int filler;                    // overhead caused by filler bytes
  static int entries;                   // total bytes consumed by vtable entries
  static int array_entries;             // total bytes consumed by array vtable entries

  static void do_class(klassOop k) {
    Klass* kl = k->klass_part();
    klassVtable* vt = kl->vtable();
    if (vt == NULL) return;
    no_klasses++;
    if (kl->oop_is_instance()) {
      no_instance_klasses++;
      kl->array_klasses_do(do_class);
    }
    if (kl->oop_is_array()) {
      no_array_klasses++;
      sum_of_array_vtable_len += vt->length();
    }
    sum_of_vtable_len += vt->length();
  }

  static void compute() {
    SystemDictionary::classes_do(do_class);
    fixed  = no_klasses * oopSize;      // vtable length
    // filler size is a conservative approximation
    filler = oopSize * (no_klasses - no_instance_klasses) * (sizeof(instanceKlass) - sizeof(arrayKlass) - 1);
    entries = sizeof(vtableEntry) * sum_of_vtable_len;
    array_entries = sizeof(vtableEntry) * sum_of_array_vtable_len;
  }
};

int VtableStats::no_klasses = 0;         
int VtableStats::no_array_klasses = 0;   
int VtableStats::no_instance_klasses = 0;
int VtableStats::sum_of_vtable_len = 0; 
int VtableStats::sum_of_array_vtable_len = 0; 
int VtableStats::fixed = 0;     
int VtableStats::filler = 0; 
int VtableStats::entries = 0;
int VtableStats::array_entries = 0;

void klassVtable::print_statistics() {
  ResourceMark rm;
  HandleMark hm;
  VtableStats::compute();
  tty->print_cr("vtable statistics:");
  tty->print_cr("%6d classes (%d instance, %d array)", VtableStats::no_klasses, VtableStats::no_instance_klasses, VtableStats::no_array_klasses);
  int total = VtableStats::fixed + VtableStats::filler + VtableStats::entries;
  tty->print_cr("%6d bytes fixed overhead (refs + vtable object header)", VtableStats::fixed);
  tty->print_cr("%6d bytes filler overhead", VtableStats::filler);
  tty->print_cr("%6d bytes for vtable entries (%d for arrays)", VtableStats::entries, VtableStats::array_entries);
  tty->print_cr("%6d bytes total", total);
}

int  klassItable::_total_classes;   // Total no. of classes with itables
long klassItable::_total_size;      // Total no. of bytes used for itables

void klassItable::print_statistics() {
 tty->print_cr("itable statistics:");
 tty->print_cr("%6d classes with itables", _total_classes);
 tty->print_cr("%6d K uses for itables (average by class: %d bytes)", _total_size / K, _total_size / _total_classes); 
}

#endif // PRODUCT


