#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)universe.cpp	1.320 04/06/15 12:17:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_universe.cpp.incl"

// Known objects 
klassOop Universe::_boolArrayKlassObj                 = NULL;
klassOop Universe::_byteArrayKlassObj                 = NULL;
klassOop Universe::_charArrayKlassObj                 = NULL;
klassOop Universe::_intArrayKlassObj                  = NULL;
klassOop Universe::_shortArrayKlassObj                = NULL;
klassOop Universe::_longArrayKlassObj                 = NULL;
klassOop Universe::_singleArrayKlassObj               = NULL;
klassOop Universe::_doubleArrayKlassObj               = NULL;
klassOop Universe::_typeArrayKlassObjs[T_VOID+1]      = { NULL /*, NULL...*/ };
klassOop Universe::_symbolKlassObj                    = NULL;
klassOop Universe::_methodKlassObj                    = NULL;
klassOop Universe::_constMethodKlassObj               = NULL;
#ifndef CORE
klassOop Universe::_methodDataKlassObj                = NULL;
#endif // !CORE
klassOop Universe::_klassKlassObj                     = NULL;
klassOop Universe::_arrayKlassKlassObj                = NULL;
klassOop Universe::_objArrayKlassKlassObj             = NULL;
klassOop Universe::_typeArrayKlassKlassObj            = NULL;
klassOop Universe::_instanceKlassKlassObj             = NULL;
klassOop Universe::_constantPoolKlassObj              = NULL;
klassOop Universe::_constantPoolCacheKlassObj         = NULL;
#ifndef CORE
klassOop Universe::_compiledICHolderKlassObj          = NULL;
#endif
klassOop Universe::_systemObjArrayKlassObj            = NULL;
oop      Universe::_main_thread_group                 = NULL;
oop      Universe::_system_thread_group               = NULL;
typeArrayOop Universe::_the_empty_byte_array          = NULL;
typeArrayOop Universe::_the_empty_short_array         = NULL;
typeArrayOop Universe::_the_empty_int_array           = NULL;
objArrayOop Universe::_the_empty_system_obj_array     = NULL;
objArrayOop Universe::_the_empty_class_klass_array    = NULL;
objArrayOop Universe::_the_array_interfaces_array     = NULL;
methodOop Universe::_finalizer_register_method        = NULL;
methodOop Universe::_reflect_invoke_method            = NULL;
methodOop Universe::_loader_addClass_method           = NULL;
oop Universe::_out_of_memory_error_java_heap          = NULL;
oop Universe::_out_of_memory_error_perm_gen           = NULL;
oop Universe::_out_of_memory_error_array_size         = NULL;
oop Universe::_null_ptr_exception_instance            = NULL;
oop Universe::_arithmetic_exception_instance          = NULL;
oop Universe::_vm_exception                           = NULL;
oop Universe::_emptySymbol                            = NULL;
Universe::JVMPIState Universe::_jvmpi_alloc_event_enabled       = _jvmpi_disabled;
bool Universe::_jvmpi_move_event_enabled              = false;
bool Universe::_jvmpi_jni_global_alloc_event_enabled      = false;
bool Universe::_jvmpi_jni_global_free_event_enabled       = false;
bool Universe::_jvmpi_jni_weak_global_alloc_event_enabled = false;
bool Universe::_jvmpi_jni_weak_global_free_event_enabled  = false;

// These variables are guarded by FullGCALot_lock.
debug_only(objArrayOop Universe::_fullgc_alot_dummy_array = NULL;)
debug_only(int Universe::_fullgc_alot_dummy_next      = 0;)


// Heap  
int             Universe::_verify_count = 0;

bool            Universe::_is_jdk12x_version = false;
bool            Universe::_is_jdk13x_version = false;
bool            Universe::_is_jdk14x_version = false;
bool            Universe::_is_jdk15x_version = false;

int             Universe::_base_vtable_size = 0;
bool            Universe::_bootstrapping = false;
bool            Universe::_fully_initialized = false;

size_t          Universe::_heap_capacity_at_last_gc;
size_t          Universe::_heap_used_at_last_gc;

CollectedHeap*  Universe::_collectedHeap = NULL;


void Universe::basic_type_classes_do(void f(klassOop)) {
  f(boolArrayKlassObj());
  f(byteArrayKlassObj());
  f(charArrayKlassObj());
  f(intArrayKlassObj());
  f(shortArrayKlassObj());
  f(longArrayKlassObj());
  f(singleArrayKlassObj());
  f(doubleArrayKlassObj());
}


void Universe::system_classes_do(void f(klassOop)) {
  f(symbolKlassObj());
  f(methodKlassObj());
  f(constMethodKlassObj());
#ifndef CORE
  f(methodDataKlassObj());
#endif // !CORE
  f(klassKlassObj());
  f(arrayKlassKlassObj());
  f(objArrayKlassKlassObj());
  f(typeArrayKlassKlassObj());
  f(instanceKlassKlassObj());
  f(constantPoolKlassObj());
  f(systemObjArrayKlassObj());
}

void Universe::oops_do(OopClosure* f, bool do_all) {
  f->do_oop((oop*)&_boolArrayKlassObj);
  f->do_oop((oop*)&_byteArrayKlassObj);
  f->do_oop((oop*)&_charArrayKlassObj);
  f->do_oop((oop*)&_intArrayKlassObj);
  f->do_oop((oop*)&_shortArrayKlassObj);
  f->do_oop((oop*)&_longArrayKlassObj);
  f->do_oop((oop*)&_singleArrayKlassObj);
  f->do_oop((oop*)&_doubleArrayKlassObj);
  {
    for (int i = 0; i < T_VOID+1; i++) {
      if (_typeArrayKlassObjs[i] != NULL) {
	assert(i >= T_BOOLEAN, "checking");
	f->do_oop((oop*)&_typeArrayKlassObjs[i]);
      } else if (do_all) {
        f->do_oop((oop*)&_typeArrayKlassObjs[i]);
      }
    }
  }
  f->do_oop((oop*)&_symbolKlassObj);
  f->do_oop((oop*)&_methodKlassObj);
  f->do_oop((oop*)&_constMethodKlassObj);
#ifndef CORE
  f->do_oop((oop*)&_methodDataKlassObj);
#endif // !CORE
  f->do_oop((oop*)&_klassKlassObj);
  f->do_oop((oop*)&_arrayKlassKlassObj);
  f->do_oop((oop*)&_objArrayKlassKlassObj);
  f->do_oop((oop*)&_typeArrayKlassKlassObj);
  f->do_oop((oop*)&_instanceKlassKlassObj);
  f->do_oop((oop*)&_constantPoolKlassObj);
  f->do_oop((oop*)&_constantPoolCacheKlassObj);
#ifndef CORE
  f->do_oop((oop*)&_compiledICHolderKlassObj);
#endif
  f->do_oop((oop*)&_systemObjArrayKlassObj);
  f->do_oop((oop*)&_the_empty_byte_array);
  f->do_oop((oop*)&_the_empty_short_array);
  f->do_oop((oop*)&_the_empty_int_array);
  f->do_oop((oop*)&_the_empty_system_obj_array);    
  f->do_oop((oop*)&_the_empty_class_klass_array);    
  f->do_oop((oop*)&_the_array_interfaces_array);    
  f->do_oop((oop*)&_finalizer_register_method);  
  f->do_oop((oop*)&_reflect_invoke_method);
  f->do_oop((oop*)&_loader_addClass_method);
  f->do_oop((oop*)&_out_of_memory_error_java_heap);
  f->do_oop((oop*)&_out_of_memory_error_perm_gen);
  f->do_oop((oop*)&_out_of_memory_error_array_size);
  f->do_oop((oop*)&_null_ptr_exception_instance);
  f->do_oop((oop*)&_arithmetic_exception_instance);
  f->do_oop((oop*)&_main_thread_group);
  f->do_oop((oop*)&_system_thread_group);
  f->do_oop((oop*)&_vm_exception);
  f->do_oop((oop*)&_emptySymbol);
  debug_only(f->do_oop((oop*)&_fullgc_alot_dummy_array);)
}


void Universe::check_alignment(uintx size, uintx alignment, const char* name) {
  if (size < alignment || size % alignment != 0) {
    ResourceMark rm;
    stringStream st;
    st.print("Size of %s (%ld bytes) must be aligned to %ld bytes", name, size, alignment);
    char* error = st.as_string();
    vm_exit_during_initialization(error);
  }
}


void Universe::genesis(TRAPS) {
  ResourceMark rm;
  { FlagSetting fs(_bootstrapping, true);
      
    { MutexLocker mc(Compile_lock);

      // determine base vtable size; without that we cannot create the array klasses
      compute_base_vtable_size();

      if (!UseSharedSpaces) {
        _klassKlassObj          = klassKlass::create_klass(CHECK);
        _arrayKlassKlassObj     = arrayKlassKlass::create_klass(CHECK);

        _objArrayKlassKlassObj  = objArrayKlassKlass::create_klass(CHECK);
        _instanceKlassKlassObj  = instanceKlassKlass::create_klass(CHECK);
        _typeArrayKlassKlassObj = typeArrayKlassKlass::create_klass(CHECK);

        _symbolKlassObj         = symbolKlass::create_klass(CHECK);

        _emptySymbol            = oopFactory::new_symbol("", CHECK);

        _boolArrayKlassObj      = typeArrayKlass::create_klass(T_BOOLEAN, sizeof(jboolean), CHECK);

        _charArrayKlassObj      = typeArrayKlass::create_klass(T_CHAR,    sizeof(jchar),    CHECK);
        _singleArrayKlassObj    = typeArrayKlass::create_klass(T_FLOAT,   sizeof(jfloat),   CHECK);
        _doubleArrayKlassObj    = typeArrayKlass::create_klass(T_DOUBLE,  sizeof(jdouble),  CHECK);
        _byteArrayKlassObj      = typeArrayKlass::create_klass(T_BYTE,    sizeof(jbyte),    CHECK);
        _shortArrayKlassObj     = typeArrayKlass::create_klass(T_SHORT,   sizeof(jshort),   CHECK);
        _intArrayKlassObj       = typeArrayKlass::create_klass(T_INT,     sizeof(jint),     CHECK);
        _longArrayKlassObj      = typeArrayKlass::create_klass(T_LONG,    sizeof(jlong),    CHECK);

        _typeArrayKlassObjs[T_BOOLEAN] = _boolArrayKlassObj;
        _typeArrayKlassObjs[T_CHAR]    = _charArrayKlassObj;
        _typeArrayKlassObjs[T_FLOAT]   = _singleArrayKlassObj;
        _typeArrayKlassObjs[T_DOUBLE]  = _doubleArrayKlassObj;
        _typeArrayKlassObjs[T_BYTE]    = _byteArrayKlassObj;
        _typeArrayKlassObjs[T_SHORT]   = _shortArrayKlassObj;
        _typeArrayKlassObjs[T_INT]     = _intArrayKlassObj;
        _typeArrayKlassObjs[T_LONG]    = _longArrayKlassObj;

        _methodKlassObj         = methodKlass::create_klass(CHECK);
        _constMethodKlassObj    = constMethodKlass::create_klass(CHECK);
#ifndef CORE
        _methodDataKlassObj     = methodDataKlass::create_klass(CHECK);
#endif // !CORE
        _constantPoolKlassObj       = constantPoolKlass::create_klass(CHECK);
        _constantPoolCacheKlassObj  = constantPoolCacheKlass::create_klass(CHECK);

#ifndef CORE
        _compiledICHolderKlassObj   = compiledICHolderKlass::create_klass(CHECK);
#endif
        _systemObjArrayKlassObj     = objArrayKlassKlass::cast(objArrayKlassKlassObj())->allocate_system_objArray_klass(CHECK);

        _the_empty_byte_array      = oopFactory::new_permanent_byteArray(0, CHECK);
        _the_empty_short_array      = oopFactory::new_permanent_shortArray(0, CHECK);
        _the_empty_int_array        = oopFactory::new_permanent_intArray(0, CHECK);
        _the_empty_system_obj_array = oopFactory::new_system_objArray(0, CHECK);

        _the_array_interfaces_array = oopFactory::new_system_objArray(2, CHECK);
        _vm_exception               = oopFactory::new_symbol("vm exception holder", CHECK);
      } else {

        FileMapInfo *mapinfo = FileMapInfo::current_info();
        char* buffer = mapinfo->region_base(CompactingPermGenGen::md);
        void** vtbl_list = (void**)buffer;
        init_self_patching_vtbl_list(vtbl_list, 
                                     CompactingPermGenGen::vtbl_list_size);
      }
    }

    vmSymbols::initialize(CHECK);

    SystemDictionary::initialize(CHECK);

    klassOop ok = SystemDictionary::object_klass();

    if (UseSharedSpaces) {
      // Verify shared interfaces array.
      assert(_the_array_interfaces_array->obj_at(0) ==
             SystemDictionary::cloneable_klass(), "u3");
      assert(_the_array_interfaces_array->obj_at(1) ==
             SystemDictionary::serializable_klass(), "u3");

      // Verify element klass for system obj array klass
      assert(objArrayKlass::cast(_systemObjArrayKlassObj)->element_klass() == ok, "u1");
      assert(objArrayKlass::cast(_systemObjArrayKlassObj)->bottom_klass() == ok, "u2");

      // Verify super class for the classes created above
      assert(Klass::cast(boolArrayKlassObj()     )->super() == ok, "u3");
      assert(Klass::cast(charArrayKlassObj()     )->super() == ok, "u3");
      assert(Klass::cast(singleArrayKlassObj()   )->super() == ok, "u3");
      assert(Klass::cast(doubleArrayKlassObj()   )->super() == ok, "u3");
      assert(Klass::cast(byteArrayKlassObj()     )->super() == ok, "u3");
      assert(Klass::cast(shortArrayKlassObj()    )->super() == ok, "u3");
      assert(Klass::cast(intArrayKlassObj()      )->super() == ok, "u3");
      assert(Klass::cast(longArrayKlassObj()     )->super() == ok, "u3");
      assert(Klass::cast(constantPoolKlassObj()  )->super() == ok, "u3");
      assert(Klass::cast(systemObjArrayKlassObj())->super() == ok, "u3");
    } else {
      // Set up shared interfaces array.  (Do this before supers are set up.)
      _the_array_interfaces_array->obj_at_put(0, SystemDictionary::cloneable_klass());
      _the_array_interfaces_array->obj_at_put(1, SystemDictionary::serializable_klass());

      // Set element klass for system obj array klass
      objArrayKlass::cast(_systemObjArrayKlassObj)->set_element_klass(ok);
      objArrayKlass::cast(_systemObjArrayKlassObj)->set_bottom_klass(ok);

      // Set super class for the classes created above
      Klass::cast(boolArrayKlassObj()     )->initialize_supers(ok, CHECK);
      Klass::cast(charArrayKlassObj()     )->initialize_supers(ok, CHECK);
      Klass::cast(singleArrayKlassObj()   )->initialize_supers(ok, CHECK);
      Klass::cast(doubleArrayKlassObj()   )->initialize_supers(ok, CHECK);
      Klass::cast(byteArrayKlassObj()     )->initialize_supers(ok, CHECK);
      Klass::cast(shortArrayKlassObj()    )->initialize_supers(ok, CHECK);
      Klass::cast(intArrayKlassObj()      )->initialize_supers(ok, CHECK);
      Klass::cast(longArrayKlassObj()     )->initialize_supers(ok, CHECK);
      Klass::cast(constantPoolKlassObj()  )->initialize_supers(ok, CHECK);
      Klass::cast(systemObjArrayKlassObj())->initialize_supers(ok, CHECK);
      Klass::cast(boolArrayKlassObj()     )->set_super(ok);
      Klass::cast(charArrayKlassObj()     )->set_super(ok);
      Klass::cast(singleArrayKlassObj()   )->set_super(ok);
      Klass::cast(doubleArrayKlassObj()   )->set_super(ok);
      Klass::cast(byteArrayKlassObj()     )->set_super(ok);
      Klass::cast(shortArrayKlassObj()    )->set_super(ok);
      Klass::cast(intArrayKlassObj()      )->set_super(ok);
      Klass::cast(longArrayKlassObj()     )->set_super(ok);
      Klass::cast(constantPoolKlassObj()  )->set_super(ok);
      Klass::cast(systemObjArrayKlassObj())->set_super(ok);
    }

    Klass::cast(boolArrayKlassObj()     )->append_to_sibling_list();
    Klass::cast(charArrayKlassObj()     )->append_to_sibling_list();
    Klass::cast(singleArrayKlassObj()   )->append_to_sibling_list();
    Klass::cast(doubleArrayKlassObj()   )->append_to_sibling_list();
    Klass::cast(byteArrayKlassObj()     )->append_to_sibling_list();
    Klass::cast(shortArrayKlassObj()    )->append_to_sibling_list();
    Klass::cast(intArrayKlassObj()      )->append_to_sibling_list();
    Klass::cast(longArrayKlassObj()     )->append_to_sibling_list();
    Klass::cast(constantPoolKlassObj()  )->append_to_sibling_list();
    Klass::cast(systemObjArrayKlassObj())->append_to_sibling_list();
  } // end of core bootstrapping
  
  // Compute is_jdk version flags. 
  // Only 1.3 or later has the java.lang.Shutdown class.
  // Only 1.4 or later has the java.lang.CharSequence interface.
  // Only 1.5 or later has the java.lang.management.MemoryUsage class.
  klassOop k = SystemDictionary::resolve_or_null(vmSymbolHandles::java_lang_management_MemoryUsage(), NULL, NULL, THREAD);
  CLEAR_PENDING_EXCEPTION; // ignore exceptions
  if (k == NULL) {
    k = SystemDictionary::resolve_or_null(vmSymbolHandles::java_lang_CharSequence(), NULL, NULL, THREAD);
    CLEAR_PENDING_EXCEPTION; // ignore exceptions
    if (k == NULL) {
      k = SystemDictionary::resolve_or_null(vmSymbolHandles::java_lang_Shutdown(), NULL, NULL, THREAD);
      CLEAR_PENDING_EXCEPTION; // ignore exceptions
      if (k == NULL) {
        Universe::_is_jdk12x_version = true;
      } else {
        Universe::_is_jdk13x_version = true;
      }
    } else {
        Universe::_is_jdk14x_version = true;
    }
  } else {
    Universe::_is_jdk15x_version = true;
  }

  #ifdef ASSERT
  if (FullGCALot) {
    // Allocate an array of dummy objects.
    // We'd like these to be at the bottom of the old generation,
    // so that when we free one and then collect,
    // (almost) the whole heap moves
    // and we find out if we actually update all the oops correctly.
    // But we can't allocate directly in the old generation,
    // so we allocate wherever, and hope that the first collection
    // moves these objects to the bottom of the old generation.
    // We can allocate directly in the permanent generation, so we do.
    int size;
    if (UseTrainGC || UseConcMarkSweepGC) {
      warning("Using +FullGCALot with train gc or conc mark sweep gc "
              "will not force all objects to relocate");
      size = FullGCALotDummies;
    } else {
      size = FullGCALotDummies * 2;
    }
    objArrayOop    naked_array = oopFactory::new_system_objArray(size, CHECK);
    objArrayHandle dummy_array(THREAD, naked_array);
    int i = 0;
    while (i < size) {
      if (!UseTrainGC && !UseConcMarkSweepGC) {
        // Allocate dummy in old generation
        oop dummy = instanceKlass::cast(SystemDictionary::object_klass())->allocate_instance(CHECK);
        dummy_array->obj_at_put(i++, dummy);
      }
      // Allocate dummy in permanent generation
      oop dummy = instanceKlass::cast(SystemDictionary::object_klass())->allocate_permanent_instance(CHECK);
      dummy_array->obj_at_put(i++, dummy);
    }
    {
      // Only modify the global variable inside the mutex.
      // If we had a race to here, the other dummy_array instances
      // and their elements just get dropped on the floor, which is fine.
      MutexLocker ml(FullGCALot_lock);
      if (_fullgc_alot_dummy_array == NULL) {
        _fullgc_alot_dummy_array = dummy_array();
      }
    }
    assert(i == _fullgc_alot_dummy_array->length(), "just checking");
  }
  #endif
}    


static inline void add_vtable(void** list, int* n, Klass* o, int count) {
  list[(*n)++] = *(void**)&o->vtbl_value();
  guarantee((*n) <= count, "vtable list too small.");
}


void Universe::init_self_patching_vtbl_list(void** list, int count) {
  int n = 0;
  { klassKlass o;             add_vtable(list, &n, &o, count); }
  { arrayKlassKlass o;        add_vtable(list, &n, &o, count); }
  { objArrayKlassKlass o;     add_vtable(list, &n, &o, count); }
  { instanceKlassKlass o;     add_vtable(list, &n, &o, count); }
  { instanceKlass o;          add_vtable(list, &n, &o, count); }
  { instanceRefKlass o;       add_vtable(list, &n, &o, count); }
  { typeArrayKlassKlass o;    add_vtable(list, &n, &o, count); }
  { symbolKlass o;            add_vtable(list, &n, &o, count); }
  { typeArrayKlass o;         add_vtable(list, &n, &o, count); }
  { methodKlass o;            add_vtable(list, &n, &o, count); }
  { constMethodKlass o;       add_vtable(list, &n, &o, count); }
  { constantPoolKlass o;      add_vtable(list, &n, &o, count); }
  { constantPoolCacheKlass o; add_vtable(list, &n, &o, count); }
  { objArrayKlass o;          add_vtable(list, &n, &o, count); }
#ifndef CORE
  { methodDataKlass o;        add_vtable(list, &n, &o, count); }
  { compiledICHolderKlass o;  add_vtable(list, &n, &o, count); }
#endif
}


class FixupMirrorClosure: public ObjectClosure {
 public:
  void do_object(oop obj) {
    if (obj->is_klass()) {
      EXCEPTION_MARK;
      KlassHandle k(THREAD, klassOop(obj));
      // We will never reach the CATCH below since Exceptions::_throw will cause
      // the VM to exit if an exception is thrown during initialization
      java_lang_Class::create_mirror(k, CATCH);
    }
  }
};


void Universe::fixup_mirrors(TRAPS) {
  // Bootstrap problem: all classes gets a mirror (java.lang.Class instance) assigned eagerly,
  // but we cannot do that for classes created before java.lang.Class is loaded. Here we simply
  // walk over permanent objects created so far (mostly classes) and fixup their mirrors. Note
  // that the number of objects allocated at this point is very small.
  assert(SystemDictionary::class_klass_loaded(), "java.lang.Class should be loaded");
  FixupMirrorClosure blk;
  Universe::heap()->permanent_object_iterate(&blk);
}


static bool has_run_finalizers_on_exit = false;

void Universe::run_finalizers_on_exit() {
  if (has_run_finalizers_on_exit) return;
  has_run_finalizers_on_exit = true;

  // Called on VM exit. This ought to be run in a separate thread.
  if (TraceReferenceGC) tty->print_cr("Callback to run finalizers on exit");
  { 
    PRESERVE_EXCEPTION_MARK;
    KlassHandle finalizer_klass(THREAD, SystemDictionary::finalizer_klass());
    JavaValue result(T_VOID);
    JavaCalls::call_static(
      &result, 
      finalizer_klass, 
      vmSymbolHandles::run_finalizers_on_exit_name(), 
      vmSymbolHandles::void_method_signature(),
      THREAD
    );
    // Ignore any pending exceptions
    CLEAR_PENDING_EXCEPTION;
  }
}


void Universe::reinitialize_vtable_of(Klass* k, TRAPS) {  
  // init vtable of k and all subclasses
  klassVtable* vt = k->vtable();
  if (vt) vt->initialize_vtable(THREAD);
  if (k->oop_is_instance()) {
    instanceKlass* ik = (instanceKlass*)k;
    for (Klass* s = ik->subklass(); s != NULL; s = s->next_sibling()) {
      reinitialize_vtable_of(s, CHECK);
    }
  }
}


void initialize_itable_for_klass(klassOop k) {
  instanceKlass::cast(k)->itable()->initialize_itable();        
}


void Universe::reinitialize_itables() {
  SystemDictionary::classes_do(initialize_itable_for_klass);

}


bool Universe::on_page_boundary(void* addr) {
  return ((uintptr_t) addr) % os::vm_page_size() == 0;
}

bool Universe::is_out_of_memory_error(Handle throwable) {
  return ((throwable == Universe::out_of_memory_error_java_heap()) ||
          (throwable == Universe::out_of_memory_error_perm_gen())  ||
          (throwable == Universe::out_of_memory_error_array_size()));
}

static intptr_t non_oop_bits = 0;

void* Universe::non_oop_word() {
  // Neither the high bits nor the low bits of this value is allowed
  // to look like (respectively) the high or low bits of a real oop.
  //
  // High and low are CPU-specific notions, but low always includes
  // the low-order bit.  Since oops are always aligned at least mod 4,
  // setting the low-order bit will ensure that the low half of the
  // word will never look like that of a real oop.
  //
  // Using the OS-supplied non-memory-address word (usually 0 or -1)
  // will take care of the high bits, however many there are.

  if (non_oop_bits == 0) {
    non_oop_bits = (intptr_t)os::non_memory_address_word() | 1;
  }

  return (void*)non_oop_bits;
}

jint universe_init() {
  assert(!Universe::_fully_initialized, "called after initialize_vtables");
  guarantee(1 << LogHeapWordSize == sizeof(HeapWord),
	 "LogHeapWordSize is incorrect.");
  guarantee(sizeof(oop) >= sizeof(HeapWord), "HeapWord larger than oop?");
  guarantee(sizeof(oop) % sizeof(HeapWord) == 0,
	 "oop size is not not a multiple of HeapWord size");
  TraceTime timer("Genesis", TraceStartupTime);
  GC_locker::lock();  // do not allow gc during bootstrapping
  JavaClasses::compute_hard_coded_offsets();

  // Get map info from shared archive file.
  if (DumpSharedSpaces)
    UseSharedSpaces = false;

  FileMapInfo* mapinfo = NULL;
  if (UseSharedSpaces) {
    mapinfo = NEW_C_HEAP_OBJ(FileMapInfo);
    memset(mapinfo, 0, sizeof(FileMapInfo));

    // Open the shared archive file, read and validate the header. If
    // initialization files, shared spaces [UseSharedSpaces] are
    // disabled and the file is closed.

    if (mapinfo->initialize()) {
      FileMapInfo::set_current_info(mapinfo);
    } else {
      assert(!mapinfo->is_open() && !UseSharedSpaces,
             "archive file not closed or shared spaces not disabled.");
    }
  }

  jint status = Universe::initialize_heap();
  if (status != JNI_OK) {
    return status;
  }

  if (UseSharedSpaces) {

    // Read the data structures supporting the shared spaces (shared
    // system dictionary, symbol table, etc.).  After that, access to
    // the file (other than the mapped regions) is no longer needed, and
    // the file is closed. Closing the file does not affect the
    // currently mapped regions.

    CompactingPermGenGen::initialize_oops();
    mapinfo->close();

  } else {
    SymbolTable::create_table();
    StringTable::create_table();
    ClassLoader::create_package_info_table();
  }

  return JNI_OK;
}

jint Universe::initialize_heap() {

  if (UseParallelGC) {
    Universe::_collectedHeap = new ParallelScavengeHeap();
  } else {
    CollectorPolicy *gc_policy;
    
    if (UseSerialGC) {
      gc_policy = new MarkSweepPolicy();
    } else if (UseTrainGC) {
      gc_policy = new TrainPolicy();
    } else if (UseConcMarkSweepGC) {
      gc_policy = new ConcurrentMarkSweepPolicy();
    } else { // default old generation
      gc_policy = new MarkSweepPolicy();
    }
    
    Universe::_collectedHeap = new GenCollectedHeap(gc_policy);
  }

  jint status = Universe::heap()->initialize();
  if (status != JNI_OK) {
    return status;
  }

  // We will never reach the CATCH below since Exceptions::_throw will cause
  // the VM to exit if an exception is thrown during initialization

  if (UseTLAB) {
    assert(Universe::heap()->supports_tlab_allocation(),
           "Should support thread-local allocation buffers");
    ThreadLocalAllocBuffer::startup_initialization();
  }
  return JNI_OK;
}

// It's the caller's repsonsibility to ensure glitch-freedom
// (if required).
void Universe::update_heap_info_at_gc() {
  _heap_capacity_at_last_gc = heap()->capacity();
  _heap_used_at_last_gc     = heap()->used();
}



void universe2_init() {
  EXCEPTION_MARK;
  Universe::genesis(CATCH);
  // Although we'd like to verify here that the state of the heap
  // is good, we can't because the main thread has not yet added
  // itself to the threads list (so, using current interfaces
  // we can't "fill" its TLAB), unless TLABs are disabled.
  if (VerifyBeforeGC && !UseTLAB &&
      Universe::heap()->total_collections() >= VerifyGCStartAt) { 
     Universe::heap()->prepare_for_verify();
     Universe::verify();   // make sure we're starting with a clean slate
  }
}


// This function is defined in JVM.cpp
extern void initialize_converter_functions();

void universe_post_init() {
  Universe::_fully_initialized = true;
  EXCEPTION_MARK;
  { ResourceMark rm;
    Interpreter::initialize();      // needed for interpreter entry points
    if (!UseSharedSpaces) {
      Klass* k = Klass::cast(SystemDictionary::object_klass());
      Universe::reinitialize_vtable_of(k, CHECK);
      Universe::reinitialize_itables();
    }
  }

  klassOop k;
  instanceKlassHandle k_h;
  if (!UseSharedSpaces) {
    // Setup preallocated empty java.lang.Class array
    Universe::_the_empty_class_klass_array = oopFactory::new_objArray(SystemDictionary::class_klass(), 0, CHECK);
    // Setup preallocated OutOfMemoryError errors
    k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_OutOfMemoryError(), NULL, NULL, true, CHECK);
    k_h = instanceKlassHandle(THREAD, k);
    Universe::_out_of_memory_error_java_heap = k_h->allocate_permanent_instance(CHECK);
    Universe::_out_of_memory_error_perm_gen = k_h->allocate_permanent_instance(CHECK);
    Universe::_out_of_memory_error_array_size = k_h->allocate_permanent_instance(CHECK);

    // Setup preallocated NullPointerException
    // (this is currently used for a cheap & dirty solution in compiler exception handling)
    k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_NullPointerException(), NULL, NULL, true, CHECK);
    Universe::_null_ptr_exception_instance = instanceKlass::cast(k)->allocate_permanent_instance(CHECK);
    // Setup preallocated ArithmeticException
    // (this is currently used for a cheap & dirty solution in compiler exception handling)
    k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_ArithmeticException(), NULL, NULL, true, CHECK);
    Universe::_arithmetic_exception_instance = instanceKlass::cast(k)->allocate_permanent_instance(CHECK);
  }
  if (!DumpSharedSpaces) {
    // These are the only Java fields that are currently set during shared space dumping.
    // We prefer to not handle this generally, so we always reinitialize these detail messages.
    Handle msg = java_lang_String::create_from_str("Java heap space", CHECK);
    java_lang_Throwable::set_message(Universe::_out_of_memory_error_java_heap, msg());

    msg = java_lang_String::create_from_str("PermGen space", CHECK);
    java_lang_Throwable::set_message(Universe::_out_of_memory_error_perm_gen, msg());

    msg = java_lang_String::create_from_str("Requested array size exceeds VM limit", CHECK);
    java_lang_Throwable::set_message(Universe::_out_of_memory_error_array_size, msg());

    msg = java_lang_String::create_from_str("/ by zero", CHECK);
    java_lang_Throwable::set_message(Universe::_arithmetic_exception_instance, msg());
  }

  if (PrintHeapUsageOverTime) Universe::print_heap_usage_header();
  
  if (!UseSharedSpaces) {
    // Setup static method for registering finalizers
    // The finalizer klass must be linked before looking up the method, in
    // case it needs to get rewritten.
    instanceKlass::cast(SystemDictionary::finalizer_klass())->link_class(CHECK);
    methodOop m = instanceKlass::cast(SystemDictionary::finalizer_klass())->find_method(
                                    vmSymbols::register_method_name(), 
                                    vmSymbols::register_method_signature());
    if (m == NULL || !m->is_static()) {
      THROW_MSG(vmSymbols::java_lang_NoSuchMethodException(), "java.lang.ref.Finalizer.register");
    }
    Universe::_finalizer_register_method = m;

    // Resolve on first use and initialize class. 
    // Note: No race-condition here, since a resolve will always return the same result

    // Setup method for security checks 
    k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_reflect_Method(), NULL, NULL, true, CHECK);  
    instanceKlass::cast(k)->link_class(CHECK);
    m = instanceKlass::cast(k)->find_method(vmSymbols::invoke_name(), vmSymbols::object_array_object_object_signature());
    if (m == NULL || m->is_static()) {
      THROW_MSG(vmSymbols::java_lang_NoSuchMethodException(), "java.lang.reflect.Method.invoke");
    }
    Universe::_reflect_invoke_method = m;  

    // Setup method for registering loaded classes in class loader vector 
    instanceKlass::cast(SystemDictionary::classloader_klass())->link_class(CHECK);
    m = instanceKlass::cast(SystemDictionary::classloader_klass())->find_method(vmSymbols::addClass_name(), vmSymbols::class_void_signature());
    if (m == NULL || m->is_static()) {
      THROW_MSG(vmSymbols::java_lang_NoSuchMethodException(), "java.lang.ClassLoader.addClass");
    }
    Universe::_loader_addClass_method = m;  
  }

  // Ideally this should be done more lazily as it preloads the StrictMath
  // class in order to get the StrictMath sine & cosine native entry points.
  // Note: StrictMath doesn't exist for JDK 1.2!
  if (!Universe::is_jdk12x_version()) SharedRuntime::initialize_StrictMath_entry_points();

  // The folowing is initializing converter functions for serialization in
  // JVM.cpp. If we clean up the StrictMath code above we may want to find
  // a better solution for this as well.
  initialize_converter_functions();

  // This needs to be done before the first scavenge/gc, since
  // it's an input to soft ref clearing policy.
  Universe::update_heap_info_at_gc();

  // ("weak") refs processing infrastructure initialization
  Universe::heap()->post_initialize();

  GC_locker::unlock();  // allow gc after bootstrapping

  MemoryService::set_universe_heap(Universe::_collectedHeap);
}


void Universe::compute_base_vtable_size() {
  _base_vtable_size = ClassLoader::compute_Object_vtable();
}


#ifndef CORE
// Flushes compiled methods dependent on dependee.
void Universe::flush_dependents_on(instanceKlassHandle dependee) {  
  assert_lock_strong(Compile_lock);  
  if (CodeCache::number_of_nmethods_with_dependencies() == 0) return;

  // CodeCache can only be updated by a thread_in_VM and they will all be
  // stopped dring the safepoint so CodeCache will be safe to update without
  // holding the CodeCache_lock.
  
  // Mark all dependee and all its superclasses
  for (klassOop d = dependee(); d != NULL; d = instanceKlass::cast(d)->super()) {
    assert(!instanceKlass::cast(d)->is_marked_dependent(), "checking");
    instanceKlass::cast(d)->set_is_marked_dependent(true);
  }
  // Mark transitive interfaces
  int i;
  for (i = 0; i < dependee->transitive_interfaces()->length(); i++) {
    instanceKlass* klass = instanceKlass::cast((klassOop)dependee->transitive_interfaces()->obj_at(i));
    assert(!klass->is_marked_dependent(), "checking");
    klass->set_is_marked_dependent(true);
  }

  // Compute the dependent nmethods
  if (CodeCache::mark_for_deoptimization(dependee()) > 0) {
    // At least one nmethod has been marked for deoptimization 
    VM_Deoptimize op;  
    VMThread::execute(&op);    
  }

  // Unmark all dependee and all its superclasses
  for (klassOop e = dependee(); e != NULL; e = instanceKlass::cast(e)->super()) {
    instanceKlass::cast(e)->set_is_marked_dependent(false);
  }
  // Unmark transitive interfaces
  for (i = 0; i < dependee->transitive_interfaces()->length(); i++) {
    instanceKlass* klass = instanceKlass::cast((klassOop)dependee->transitive_interfaces()->obj_at(i));
    klass->set_is_marked_dependent(false);
  }
}

#ifdef HOTSWAP
// Flushes compiled methods dependent on dependee in the evolutionary sense
void Universe::flush_evol_dependents_on(instanceKlassHandle ev_k_h) {
  // --- Compile_lock is not held. However we are at a safepoint.
  assert_locked_or_safepoint(Compile_lock);
  if (CodeCache::number_of_nmethods_with_dependencies() == 0) return;

  // CodeCache can only be updated by a thread_in_VM and they will all be
  // stopped dring the safepoint so CodeCache will be safe to update without
  // holding the CodeCache_lock.
  
  // Compute the dependent nmethods
  if (CodeCache::mark_for_evol_deoptimization(ev_k_h) > 0) {
    // At least one nmethod has been marked for deoptimization 
    
    // All this already happens inside a VM_Operation, so we'll do all the work here.
    // Stuff copied from VM_Deoptimize and modified slightly.

    // We do not want any GCs to happen while we are in the middle of this VM operation
    ResourceMark rm;
    DeoptimizationMarker dm;

    // Deoptimize all activations depending on marked nmethods  
    Deoptimization::deoptimize_dependents();

    // Make the dependent methods not entrant (in VM_Deoptimize they are made zombies)
    CodeCache::make_marked_nmethods_not_entrant(); 
  }
}
#endif HOTSWAP


// Flushes compiled methods dependent on dependee
void Universe::flush_dependents_on_method(methodHandle m_h) {
  // --- Compile_lock is not held. However we are at a safepoint.
  assert_locked_or_safepoint(Compile_lock);

  // CodeCache can only be updated by a thread_in_VM and they will all be
  // stopped dring the safepoint so CodeCache will be safe to update without
  // holding the CodeCache_lock.
  
  // Compute the dependent nmethods
  if (CodeCache::mark_for_deoptimization(m_h()) > 0) {
    // At least one nmethod has been marked for deoptimization 
    
    // All this already happens inside a VM_Operation, so we'll do all the work here.
    // Stuff copied from VM_Deoptimize and modified slightly.

    // We do not want any GCs to happen while we are in the middle of this VM operation
    ResourceMark rm;
    DeoptimizationMarker dm;

    // Deoptimize all activations depending on marked nmethods  
    Deoptimization::deoptimize_dependents();

    // Make the dependent methods not entrant (in VM_Deoptimize they are made zombies)
    CodeCache::make_marked_nmethods_not_entrant(); 
  }
}

#endif /* CORE */


bool Universe::is_non_oop(oop x) {
  // return true if the low bits are odd:
  if (((intptr_t)x&1) == 1)  return true;

  // This choice should be safe; hopefully we don't have to factor across CPUs.
  const jint high_bits_to_check = 8;

  // return true if the high bits match:
  return ((((intptr_t)x^non_oop_bits) >> (BitsPerWord-high_bits_to_check)) == 0);
}


void Universe::print() { print_on(gclog_or_tty); }

void Universe::print_on(outputStream* st) {
  st->print_cr("Heap");
  heap()->print_on(st);
}

//---------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void Universe::verify(bool allow_dirty, bool silent) {
  if (SharedSkipVerify) {
    return;
  }
  COMPILER2_ONLY(
    assert(!DerivedPointerTable::is_active(),
         "DPT should not be active during verification "
         "(of thread stacks below)");
  )

  ResourceMark rm;
  HandleMark hm;  // Handles created during verification can be zapped
  _verify_count++;

  if (!silent) gclog_or_tty->print("[Verifying ");
  if (!silent) gclog_or_tty->print("threads ");     
  Threads::verify();
  heap()->verify(allow_dirty, silent);

  if (!silent) gclog_or_tty->print("syms ");        
  SymbolTable::verify();
  if (!silent) gclog_or_tty->print("strs ");        
  StringTable::verify();
#ifndef CORE
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    if (!silent) gclog_or_tty->print("zone ");      
    CodeCache::verify();
  }
#endif
  if (!silent) gclog_or_tty->print("dict ");        
  SystemDictionary::verify();
  if (!silent) gclog_or_tty->print("hand ");        
  JNIHandles::verify();
  if (!silent) gclog_or_tty->print("C-heap ");      
  os::check_heap();
  if (!silent) gclog_or_tty->print_cr("]");
}


void Universe::print_heap_usage_header() {
  gclog_or_tty->print("elapsed_time,mem_used,prev_mem_used,mem_capacity,");
  int levels = GenCollectedHeap::heap()->n_gens();
  for (int i = 0; i < levels; i++) {
    gclog_or_tty->print("gen_%d_capacity,", i);
  }
  gclog_or_tty->print_cr("level");
}


void Universe::print_heap_usage_stamp(size_t prev_mem_used, int level) {
  GenCollectedHeap* ch = GenCollectedHeap::heap();
  int levels = ch->n_gens();
  gclog_or_tty->print("%.2lf,%ld,%ld,%ld,", 
    os::elapsedTime(), 
    ch->used() / K, 
    prev_mem_used / K,
    ch->capacity() / K);
  for (int i = 0; i < levels; i++) {
    gclog_or_tty->print("%ld,", ch->get_gen(i)->capacity() / K);
  }
  gclog_or_tty->print_cr("%ld", level);
}


#endif // PRODUCT


// Oop verification (see MacroAssembler::verify_oop)

static uintptr_t _verify_oop_data[2]   = {0,-1};
static uintptr_t _verify_klass_data[2] = {0,-1};


static void calculate_verify_data(uintptr_t verify_data[2],
				  HeapWord* low_boundary,
				  HeapWord* high_boundary) {
  assert(low_boundary < high_boundary, "bad interval");

  // decide which low-order bits we require to be clear:
  size_t alignSize = MinObjAlignmentInBytes;
  size_t min_object_size = oopDesc::header_size();

  // make an inclusive limit:
  uintptr_t max = (uintptr_t)high_boundary - min_object_size*wordSize;
  uintptr_t min = (uintptr_t)low_boundary;
  assert(min < max, "bad interval");
  uintptr_t diff = max ^ min;

  // throw away enough low-order bits to make the diff vanish
  uintptr_t mask = (intptr_t)(-1);
  while ((mask & diff) != 0)
    mask <<= 1;
  uintptr_t bits = (min & mask);
  assert(bits == (max & mask), "correct mask");
  // check an intermediate value between min and max, just to make sure:
  assert(bits == ((min + (max-min)/2) & mask), "correct mask");

  // require address alignment, too:
  mask |= (alignSize - 1);

  if (!(verify_data[0] == 0 && verify_data[1] == -1)) {
    assert(verify_data[0] == mask && verify_data[1] == bits, "mask stability");
  }
  verify_data[0] = mask;
  verify_data[1] = bits;
}


// Oop verification (see MacroAssembler::verify_oop)
#ifndef PRODUCT

uintptr_t Universe::verify_oop_mask() {
  MemRegion m = heap()->reserved_region();
  calculate_verify_data(_verify_oop_data,
			m.start(),
			m.end());
  return _verify_oop_data[0];
}



uintptr_t Universe::verify_oop_bits() {
  verify_oop_mask();
  return _verify_oop_data[1];
}


uintptr_t Universe::verify_klass_mask() {
  /* $$$
  // A klass can never live in the new space.  Since the new and old
  // spaces can change size, we must settle for bounds-checking against
  // the bottom of the world, plus the smallest possible new and old
  // space sizes that may arise during execution.
  size_t min_new_size = Universe::new_size();   // in bytes
  size_t min_old_size = Universe::old_size();   // in bytes
  calculate_verify_data(_verify_klass_data,
	  (HeapWord*)((uintptr_t)_new_gen->low_boundary + min_new_size + min_old_size),
	  _perm_gen->high_boundary);
			*/
  // Why doesn't the above just say that klass's always live in the perm
  // gen?  I'll see if that seems to work...
  MemRegion permanent_reserved;
  switch (Universe::heap()->kind()) {
  default:
    // ???: What if a CollectedHeap doesn't have a permanent generation?
    ShouldNotReachHere();
    break;
  case CollectedHeap::GenCollectedHeap: {
    GenCollectedHeap* gch = (GenCollectedHeap*) Universe::heap();
    permanent_reserved = gch->perm_gen()->reserved();
    break;
  }
  case CollectedHeap::ParallelScavengeHeap: {
    ParallelScavengeHeap* psh = (ParallelScavengeHeap*) Universe::heap();
    permanent_reserved = psh->perm_gen()->reserved();
    break;
  }
  }
  calculate_verify_data(_verify_klass_data,
                        permanent_reserved.start(), 
                        permanent_reserved.end());
  
  return _verify_klass_data[0];
}



uintptr_t Universe::verify_klass_bits() {
  verify_klass_mask();
  return _verify_klass_data[1];
}


uintptr_t Universe::verify_mark_mask() {
  return markOopDesc::lock_mask_in_place;
}



uintptr_t Universe::verify_mark_bits() {
  intptr_t mask = verify_mark_mask();
  intptr_t bits = (intptr_t)markOopDesc::prototype();
  assert((bits & ~mask) == 0, "no stray header bits");
  return bits;
}
#endif // PRODUCT


void Universe::compute_verify_oop_data() {
  verify_oop_mask();
  verify_oop_bits();
  verify_mark_mask();
  verify_mark_bits();
  verify_klass_mask();
  verify_klass_bits();
}


void Universe::jvmpi_object_alloc(oop obj, size_t bytesize) {
  JavaThread *calling_thread = JavaThread::active();
  assert(calling_thread != NULL, "must be posting for another thread");
  if (calling_thread == NULL) return; // robustness

  GrowableArray<DeferredObjAllocEvent *>* deferred_list =
    calling_thread->deferred_obj_alloc_events();

  if (deferred_list == NULL) {
    jvmpi::post_object_alloc_event(obj, bytesize,
      heap()->addr_to_arena_id(obj), 0);
  } else {
    DeferredObjAllocEvent *node =
      new DeferredObjAllocEvent(obj, bytesize, heap()->addr_to_arena_id(obj));
    deferred_list->append(node);
  }
}


void Universe::jvmpi_object_move(oop from, oop to) {
  CollectedHeap* ch = heap();
  jvmpi::post_object_move_event(from, ch->addr_to_arena_id(from),
				to, ch->addr_to_arena_id(to));
}


void Universe::jvmpi_post_deferred_obj_alloc_events(
  GrowableArray<DeferredObjAllocEvent *>* deferred_list) {

  for (int i = 0; i < deferred_list->length(); i++) {
    DeferredObjAllocEvent *node = deferred_list->at(i);
    assert(node != NULL, "expected DeferredObjAllocEvent node");
    jvmpi::post_object_alloc_event(node->get_oop(), node->bytesize(),
      node->arena_id(), 0);

    delete node;
  }
}

#ifdef ASSERT
// Release dummy object(s) at bottom of heap
bool Universe::release_fullgc_alot_dummy() {
  MutexLocker ml(FullGCALot_lock);
  if (_fullgc_alot_dummy_array != NULL) {
    if (_fullgc_alot_dummy_next >= _fullgc_alot_dummy_array->length()) {
      // No more dummies to release, release entire array instead
      _fullgc_alot_dummy_array = NULL;
      return false;
    }
    if (!UseTrainGC && !UseConcMarkSweepGC) {
      // Release dummy at bottom of old generation
      _fullgc_alot_dummy_array->obj_at_put(_fullgc_alot_dummy_next++, NULL);
    }
    // Release dummy at bottom of permanent generation
    _fullgc_alot_dummy_array->obj_at_put(_fullgc_alot_dummy_next++, NULL);
  }
  return true;
}

#endif // ASSERT
