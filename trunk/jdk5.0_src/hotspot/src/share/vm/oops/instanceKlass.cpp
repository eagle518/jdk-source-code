#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)instanceKlass.cpp	1.280 04/03/26 09:48:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 * 
 */

# include "incls/_precompiled.incl"
# include "incls/_instanceKlass.cpp.incl"

bool instanceKlass::should_be_initialized() const {
  return !is_initialized();
}

klassVtable* instanceKlass::vtable() const {
  return new klassVtable(as_klassOop(), start_of_vtable(), vtable_length() / vtableEntry::size());
}

klassItable* instanceKlass::itable() const {
  return new klassItable(as_klassOop());
}

void instanceKlass::eager_initialize(Thread *thread) {
#ifndef CORE
  if (!EagerInitialization) return;

  if (this->is_not_initialized()) {
    // abort if the the class has a class initializer
    if (this->class_initializer() != NULL) return;

    // abort if it is java.lang.Object (initialization is handled in genesis)
    klassOop super = this->super();
    if (super == NULL) return;

    // abort if the super class should be initialized
    if (!instanceKlass::cast(super)->is_initialized()) return;

    // call body to expose the this pointer
    instanceKlassHandle this_oop(thread, this->as_klassOop());
    eager_initialize_impl(this_oop);
  }
#endif
}


void instanceKlass::eager_initialize_impl(instanceKlassHandle this_oop) {
  EXCEPTION_MARK;
  ObjectLocker ol(this_oop, THREAD);

  // abort if someone beat us to the initialization
  if (!this_oop->is_not_initialized()) return;  // note: not equivalent to is_initialized()

  ClassState old_state = this_oop->_init_state;
  link_class_impl(this_oop, THREAD);
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    // Abort if linking the class throws an exception.

    // Use a test to avoid redundantly resetting the state if there's
    // no change.  Set_init_state() asserts that state changes make
    // progress, whereas here we might just be spinning in place.
    if( old_state != this_oop->_init_state )
      this_oop->set_init_state (old_state);
  } else {
    // linking successfull, mark class as initialized
    this_oop->set_init_state (fully_initialized);
    // trace 
    if (TraceClassInitialization) {
      ResourceMark rm(THREAD);
      tty->print_cr("[Initialized %s without side effects]", this_oop->external_name());
    }
  }
}


// See "The Virtual Machine Specification" section 2.16.5 for a detailed explanation of the class initialization
// process. The step comments refers to the procedure described in that section.
// Note: implementation moved to static method to expose the this pointer.
void instanceKlass::initialize(TRAPS) {
  if (this->should_be_initialized()) {
    HandleMark hm(THREAD);
    instanceKlassHandle this_oop(THREAD, this->as_klassOop());
    initialize_impl(this_oop, CHECK);
    // Note: at this point the class may be initialized
    //       OR it may be in the state of being initialized
    //       in case of recursive initialization!
  } else {
    assert(is_initialized(), "sanity check");
  }
}


void instanceKlass::verify_code(instanceKlassHandle this_oop, TRAPS) {
  // 1) Verify the bytecodes
  Verifier::verify_byte_codes(this_oop, CHECK);
}

void instanceKlass::add_loader_constraints(instanceKlassHandle this_oop, TRAPS) {
  // Walk over methods looking for methods declared in super-classes (overrides)
  // or methods declared in interfaces, and insist that the names in the 
  // signatures refer to the same classes.  See JLS 5.4.2.
  objArrayHandle methods (THREAD, this_oop->methods());
  int number_of_methods = methods->length();
  Handle this_loader (THREAD, this_oop->class_loader());
  for (int method_index = 0; method_index < number_of_methods; method_index += 1) {
    methodHandle method (THREAD, (methodOop) methods->obj_at(method_index));
    // skip private, static and <init> methods
    if ((!method->is_private()) &&
      (!method->is_static()) &&
      (method->name() != vmSymbols::object_initializer_name())) {
      
      symbolHandle name (THREAD, method->name());
      symbolHandle signature (THREAD, method->signature());
      { 
        // First, look through superclasses of this class for overriding methods
        instanceKlassHandle super_klass (THREAD, this_oop->super());
        while (super_klass.not_null()) {
          // lookup a matching method in the super class hierarchy
          if (super_klass->find_method(name(), signature()) != NULL) {
            Handle super_loader (THREAD, super_klass->class_loader());
            SystemDictionary::check_signature_loaders(signature, this_loader, super_loader, true, CHECK);
            // No need to visit his super, since he and his super
            // have already made any needed loader constraints.
            // Since loader constraints are transitive, it is enough
            // to link to the first super, and we get all the others.
            break;
          }
          // continue to look
          super_klass = instanceKlassHandle(THREAD, super_klass->super());
        }
      }
      {
        // Next, look through interfaces for implemented methods
        objArrayHandle interfaces (THREAD, this_oop->transitive_interfaces());
        int number_of_interfaces = interfaces->length();
        for (int interface_index = 0; interface_index < number_of_interfaces; interface_index += 1) {
          instanceKlassHandle interface_klass (THREAD, klassOop(interfaces->obj_at(interface_index)));
          if (interface_klass->find_method(name(), signature()) != NULL) {
            Handle interface_loader (THREAD, interface_klass->class_loader());
            SystemDictionary::check_signature_loaders(signature, this_loader, interface_loader, true, CHECK);
            // Continue to look, unconditionally, since interfaces are
            // not ordered, and a class can implement any number of them.
          }
        }
      }
    }
  }
  {
    // Make sure the current class links with the system Throwable class. 
    // See bug 4157312.
    klassOop throwable_klass = SystemDictionary::throwable_klass();
    Handle throwable_loader (THREAD, instanceKlass::cast(throwable_klass)->class_loader());
    if (throwable_loader() != this_loader()) {
      symbolHandle throwable_name (THREAD, instanceKlass::cast(throwable_klass)->name());
      SystemDictionary::add_loader_constraint(throwable_name, this_loader, throwable_loader, CHECK);
    }
  }
}


// Used exclusively by the shared spaces dump mechanism to prevent
// classes mapped into the shared regions in new VMs from appearing linked.

void instanceKlass::unlink_class() {
  assert(is_linked(), "must be linked");
  _init_state = loaded;
}

void instanceKlass::link_class(TRAPS) {    
  assert(is_loaded(), "must be loaded");
  if (!is_linked()) {
    instanceKlassHandle this_oop(THREAD, this->as_klassOop());  
    link_class_impl(this_oop, CHECK);
  }
}

void instanceKlass::link_class_impl(instanceKlassHandle this_oop, TRAPS) {
  // check for error state
  if (this_oop->is_in_error_state()) {
    ResourceMark rm(THREAD);
    THROW_MSG(vmSymbols::java_lang_NoClassDefFoundError(), this_oop->external_name());  
  }
  // return if already verified
  if (this_oop->is_linked()) {
    return;
  }
  
  // link super class before linking this class
  instanceKlassHandle super(THREAD, this_oop->super());
  if (super.not_null()) {
    if (super->is_interface()) {  // check if super class is an interface
      ResourceMark rm(THREAD);
      Exceptions::fthrow(  
        THREAD_AND_LOCATION,
        vmSymbolHandles::java_lang_IncompatibleClassChangeError(),
        "class %s has interface %s as super class",
        this_oop->external_name(),
        super->external_name()
        );
    }

    link_class_impl(super, CHECK);
  }
  
  // link all interfaces implemented by this class before linking this class
  objArrayHandle interfaces (THREAD, this_oop->local_interfaces());
  int num_interfaces = interfaces->length();
  for (int index = 0; index < num_interfaces; index++) {
    HandleMark hm(THREAD);
    instanceKlassHandle ih(THREAD, klassOop(interfaces->obj_at(index)));
    link_class_impl(ih, CHECK);
  }
  
  // in case the class is linked in the process of linking its superclasses
  if (this_oop->is_linked()) {
    return;
  }
   
  // verification & rewriting
  {
    ObjectLocker ol(this_oop, THREAD);
    if (!this_oop->is_linked()) {
      {
        assert(THREAD->is_Java_thread(), "non-JavaThread in link_class_impl");
        JavaThread* jt = (JavaThread*)THREAD;
        // Timer includes any side effects of class verification (resolution,
        // etc), but not recursive entry into verify_code().
        PerfTraceTime timer(ClassLoader::perf_class_verify_time(),
                            jt->get_thread_stat()->class_verify_recursion_count_addr());
        verify_code(this_oop, CHECK);
      }

      add_loader_constraints(this_oop, CHECK);
      // Jvmti can redefine a class before it is linked.
      // And it rewrites and init (v/i)tables at safe point.
      if (!this_oop->is_rewritten_by_redefine()) {
        this_oop->rewrite_class(CHECK);
  
        // Initialize the vtable and interface table after
        // methods have been rewritten
        { ResourceMark rm(THREAD);
          // No exception can happen here
          this_oop->vtable()->initialize_vtable(THREAD);
          this_oop->itable()->initialize_itable();
        }
      }
      this_oop->set_rewritten();
      this_oop->set_init_state(linked);
      if (JvmtiExport::should_post_class_prepare()) {
        Thread *thread = THREAD;
        assert(thread->is_Java_thread(), "thread->is_Java_thread()");
        JvmtiExport::post_class_prepare((JavaThread *) thread, this_oop());
      }
    }
  }    
}


// Rewrite the byte codes of all of the methods of a class.
// Three cases:
//    During the link of a newly loaded class.
//    During the preloading of classes to be written to the shared spaces.
//	- Rewrite the methods and update the method entry points.
//
//    During the link of a class in the shared spaces.
//	- The methods were already rewritten, update the metho entry points.
//
// The rewriter must be called exactly once. Rewriting must happen after
// verification but before the first method of the class is executed.

void instanceKlass::rewrite_class(TRAPS) {
  assert(is_loaded(), "must be loaded");
  instanceKlassHandle this_oop(this->as_klassOop());  
  if (this_oop->is_rewritten()) {
    // Class must be in a shared space - just update the entry points.
    assert(this_oop()->is_shared(), "rewriting an unshared class?");
    int i = this_oop->methods()->length();
    while (i-- > 0) {       
      methodOop m = (methodOop) this_oop->methods()->obj_at(i);
      NOT_CORE(m->invocation_counter()->reset();)
        m->update_compiled_code_entry_point(true); // Can block
    }
    return;
  }
  Rewriter::rewrite(this_oop, CHECK); // No exception can happen here
  this_oop->set_rewritten();
}


void instanceKlass::initialize_impl(instanceKlassHandle this_oop, TRAPS) {
  // Make sure klass is linked (verified) before initialization
  // A class could already be verified, since it has been reflected upon.
  this_oop->link_class(CHECK);

  // refer to the JVM book page 47 for description of steps
  // Step 1
  { ObjectLocker ol(this_oop, THREAD);

    Thread *self = THREAD; // it's passed the current thread

    // Step 2
    while(this_oop->is_being_initialized() && !this_oop->is_reentrant_initialization(self)) {      
      ol.wait(CHECK);      
    }

    // Step 3
    if (this_oop->is_being_initialized() && this_oop->is_reentrant_initialization(self))
      return;

    // Step 4 
    if (this_oop->is_initialized())
      return;

    // Step 5
    if (this_oop->is_in_error_state()) {
      THROW(vmSymbols::java_lang_NoClassDefFoundError());  
    }

    // Step 6
    this_oop->set_init_state(being_initialized);
    this_oop->set_init_thread(self);
  }
  
  // Step 7
  klassOop super_klass = this_oop->super();
  if (super_klass != NULL && !this_oop->is_interface() && Klass::cast(super_klass)->should_be_initialized()) {
    Klass::cast(super_klass)->initialize(THREAD);

    if (HAS_PENDING_EXCEPTION) {
      Handle e(THREAD, PENDING_EXCEPTION);
      CLEAR_PENDING_EXCEPTION;
      {
        EXCEPTION_MARK;
        this_oop->set_initialization_state_and_notify(initialization_error, THREAD); // Locks object, set state, and notify all waiting threads
        CLEAR_PENDING_EXCEPTION;   // ignore any exception thrown, superclass initialization error is thrown below
      }
      THROW_OOP(e());
    }
  }

  // Step 8  
  {
    assert(THREAD->is_Java_thread(), "non-JavaThread in initialize_impl");
    JavaThread* jt = (JavaThread*)THREAD;
    // Timer includes any side effects of class initialization (resolution,
    // etc), but not recursive entry into call_class_initializer().
    PerfTraceTimedEvent timer(ClassLoader::perf_class_init_time(),
                              ClassLoader::perf_classes_inited(),
                              jt->get_thread_stat()->class_init_recursion_count_addr());
    this_oop->call_class_initializer(THREAD);
  }

  // Step 9
  if (!HAS_PENDING_EXCEPTION) {    
    this_oop->set_initialization_state_and_notify(fully_initialized, CHECK);
    { ResourceMark rm(THREAD);
      debug_only(this_oop->vtable()->verify(tty, true);)
    }
  }
  else {    
    // Step 10 and 11
    Handle e(THREAD, PENDING_EXCEPTION);
    CLEAR_PENDING_EXCEPTION;
    { 
      EXCEPTION_MARK;
      this_oop->set_initialization_state_and_notify(initialization_error, THREAD);
      CLEAR_PENDING_EXCEPTION;   // ignore any exception thrown, class initialization error is thrown below
    }
    if (e->is_a(SystemDictionary::error_klass())) {
      THROW_OOP(e());
    } else {
      JavaCallArguments args(e);
      THROW_ARG(vmSymbolHandles::java_lang_ExceptionInInitializerError(),
                vmSymbolHandles::throwable_void_signature(),
                &args);      
    }
  }
}
  

// Note: implementation moved to static method to expose the this pointer.
void instanceKlass::set_initialization_state_and_notify(ClassState state, TRAPS) {
  instanceKlassHandle kh(THREAD, this->as_klassOop());
  set_initialization_state_and_notify_impl(kh, state, CHECK);
}

void instanceKlass::set_initialization_state_and_notify_impl(instanceKlassHandle this_oop, ClassState state, TRAPS) {    
  ObjectLocker ol(this_oop, THREAD);   
  this_oop->set_init_state(state);
  ol.notify_all(CHECK);  
}

void instanceKlass::add_implementor(klassOop k) {
  assert(Compile_lock->owned_by_self(), "");
  // Filter out subinterfaces because they're already in the subklass list
  if (instanceKlass::cast(k)->is_interface()) return;

  // Update number of implementors
  _nof_implementors++;

  // Set implementor, if we have one
  if (_nof_implementors == 1) {
    assert(_implementor == NULL, "should be exactly one implementor");
    oop_store_without_check((oop*)&_implementor, k);
  } else {
    oop_store_without_check((oop*)&_implementor, NULL);
  }

  // The implementor also implements the transitive_interfaces
  for (int index = 0; index < local_interfaces()->length(); index++) {
    instanceKlass::cast(klassOop(local_interfaces()->obj_at(index)))->add_implementor(k);
  }
}

void instanceKlass::init_implementor() {
  oop_store_without_check((oop*)&_implementor, NULL);
  _nof_implementors = 0;
}


void instanceKlass::process_interfaces(Thread *thread) {
  // link this class into the implementors list of every interface it implements
  KlassHandle this_as_oop (thread, this->as_klassOop());
  for (int i = local_interfaces()->length() - 1; i >= 0; i--) {
    assert(local_interfaces()->obj_at(i)->is_klass(), "must be a klass");
    instanceKlass* interf = instanceKlass::cast(klassOop(local_interfaces()->obj_at(i)));
    assert(interf->is_interface(), "expected interface");
    interf->add_implementor(this_as_oop()); 
  }
}

bool instanceKlass::can_be_primary_super_slow() const {
  if (is_interface())
    return false;
  else
    return Klass::can_be_primary_super_slow();
}

objArrayOop instanceKlass::compute_secondary_supers(int num_extra_slots, TRAPS) {
  // The secondaries are the implemented interfaces.
  instanceKlass* ik = instanceKlass::cast(as_klassOop());
  objArrayHandle interfaces (THREAD, ik->transitive_interfaces());
  int num_secondaries = num_extra_slots + interfaces->length();
  if (num_secondaries == 0) {
    return Universe::the_empty_system_obj_array();
  } else if (num_extra_slots == 0) {
    return interfaces();
  } else {
    // a mix of both
    objArrayOop secondaries = oopFactory::new_system_objArray(num_secondaries, CHECK_0);
    for (int i = 0; i < interfaces->length(); i++) {
      secondaries->obj_at_put(num_extra_slots+i, interfaces->obj_at(i));
    }
    return secondaries;
  }
}

bool instanceKlass::compute_is_subtype_of(klassOop k) {
  if (Klass::cast(k)->is_interface()) {
    return implements_interface(k);
  } else {
    return Klass::compute_is_subtype_of(k);
  }
}

bool instanceKlass::implements_interface(klassOop k) const {
  if (as_klassOop() == k) return true;
  assert(Klass::cast(k)->is_interface(), "should be an interface class");
  for (int i = 0; i < transitive_interfaces()->length(); i++) {
    if (transitive_interfaces()->obj_at(i) == k) {
      return true;
    }
  }
  return false;
}

objArrayOop instanceKlass::allocate_objArray(int n, int length, TRAPS) {
  if (length < 0) THROW_0(vmSymbols::java_lang_NegativeArraySizeException());
  if (length > arrayOopDesc::max_array_length(T_OBJECT)) {
    THROW_OOP_0(Universe::out_of_memory_error_array_size());
  }
  int size = objArrayOopDesc::object_size(length);
  klassOop ak = array_klass(n, CHECK_0);
  KlassHandle h_ak (THREAD, ak);
  objArrayOop o =
    (objArrayOop)CollectedHeap::array_allocate(h_ak, size, length, CHECK_0);
  return o;
}

instanceOop instanceKlass::register_finalizer(instanceOop i, TRAPS) {
  if (TraceFinalizerRegistration) {
    tty->print("Registered ");
    i->print_value_on(tty);
    tty->print_cr(" (" INTPTR_FORMAT ") as finalizable", i);
  }
  instanceHandle h_i(THREAD, i);
  // Pass the handle as argument, JavaCalls::call expects oop as jobjects
  JavaValue result(T_VOID);
  JavaCallArguments args(h_i);  
  methodHandle mh (THREAD, Universe::finalizer_register_method());
  JavaCalls::call(&result, mh, &args, CHECK_0);
  return h_i();
}

instanceOop instanceKlass::allocate_instance(TRAPS) {
  bool has_finalizer_flag = has_finalizer(); // Query before possible GC
  KlassHandle h_k(THREAD, as_klassOop());
  instanceOop i;
 
  i = (instanceOop)CollectedHeap::obj_allocate(h_k, size_helper(), CHECK_0);
  if (has_finalizer_flag && RegisterFinalizers) {
    i = register_finalizer(i, CHECK_0);
  }
  return i;
}

instanceOop instanceKlass::allocate_permanent_instance(TRAPS) {
  bool has_finalizer_flag = has_finalizer(); // Query before possible GC
  KlassHandle h_k(THREAD, as_klassOop());
  instanceOop i = (instanceOop)
    CollectedHeap::permanent_obj_allocate(h_k, size_helper(), CHECK_0);
  if (has_finalizer_flag && RegisterFinalizers) {
    i = register_finalizer(i, CHECK_0);
  }
  return i;
}

void instanceKlass::check_valid_for_instantiation(bool throwError, TRAPS) {
  if (is_interface() || is_abstract()) {
    ResourceMark rm(THREAD);
    THROW_MSG(throwError ? vmSymbols::java_lang_InstantiationError()
              : vmSymbols::java_lang_InstantiationException(), external_name());
  }
  if (as_klassOop() == SystemDictionary::class_klass()) {
    ResourceMark rm(THREAD);
    THROW_MSG(throwError ? vmSymbols::java_lang_IllegalAccessError()
              : vmSymbols::java_lang_IllegalAccessException(), external_name());
  }
}

klassOop instanceKlass::array_klass_impl(bool or_null, int n, TRAPS) {
  instanceKlassHandle this_oop(THREAD, as_klassOop());
  return array_klass_impl(this_oop, or_null, n, THREAD);
}

klassOop instanceKlass::array_klass_impl(instanceKlassHandle this_oop, bool or_null, int n, TRAPS) {    
  if (this_oop->array_klasses() == NULL) {
    if (or_null) return NULL;

    ResourceMark rm;
    JavaThread *jt = (JavaThread *)THREAD;
    {
      // Atomic creation of array_klasses
      MutexLocker mc(Compile_lock, THREAD);   // for vtables
      MutexLocker ma(MultiArray_lock, THREAD);

      // Check if update has already taken place    
      if (this_oop->array_klasses() == NULL) {
        objArrayKlassKlass* oakk =
          (objArrayKlassKlass*)Universe::objArrayKlassKlassObj()->klass_part();

        // We grab locks above and the allocate_objArray_klass() code
        // path needs to post OBJECT_ALLOC events for the newly
        // allocated objects. We can't post events while holding locks
        // so we store the information on the side until after we
        // release the locks.
        if (Universe::jvmpi_alloc_event_enabled()) {
          jt->set_deferred_obj_alloc_events(
            new GrowableArray<DeferredObjAllocEvent *>(1, true));
        }
        klassOop  k = oakk->allocate_objArray_klass(1, this_oop, CHECK_0);                  
        this_oop->set_array_klasses(k);
      }
    }

    GrowableArray<DeferredObjAllocEvent *>* deferred_list =
      jt->deferred_obj_alloc_events();
    if (deferred_list != NULL) {
      if (deferred_list->length() > 0) {
        Universe::jvmpi_post_deferred_obj_alloc_events(deferred_list);
      }
      jt->set_deferred_obj_alloc_events(NULL);
    }
  }
  // _this will always be set at this point
  objArrayKlass* oak = (objArrayKlass*)this_oop->array_klasses()->klass_part();
  if (or_null) {
    return oak->array_klass_or_null(n);
  }
  return oak->array_klass(n, CHECK_0);
}

klassOop instanceKlass::array_klass_impl(bool or_null, TRAPS) {
  return array_klass_impl(or_null, 1, THREAD);
}

void instanceKlass::call_class_initializer(TRAPS) {
  instanceKlassHandle ik (THREAD, as_klassOop());
  call_class_initializer_impl(ik, THREAD);
}

static int call_class_initializer_impl_counter = 0;   // for debugging

methodOop instanceKlass::class_initializer() {
  return find_method(vmSymbols::class_initializer_name(), vmSymbols::void_method_signature()); 
}

void instanceKlass::call_class_initializer_impl(instanceKlassHandle this_oop, TRAPS) {  
  methodHandle h_method(THREAD, this_oop->class_initializer());
  assert(!this_oop->is_initialized(), "we cannot initialize twice");
  if (TraceClassInitialization) {
    tty->print("%d Initializing ", call_class_initializer_impl_counter++);
    this_oop->name()->print_value();
    tty->print_cr("%s (" INTPTR_FORMAT ")", h_method() == NULL ? "(no method)" : "", this_oop());
  }
  if (h_method() != NULL) {  
    JavaCallArguments args; // No arguments
    JavaValue result(T_VOID);
    JavaCalls::call(&result, h_method, &args, CHECK); // Static call (no args)
  }
}


void instanceKlass::mask_for(methodHandle method, int bci,
  InterpreterOopMap* entry_for) {
  // Dirty read, then double-check under a lock.
  if (_oop_map_cache == NULL) {
    // Otherwise, allocate a new one.
    MutexLocker x(OopMapCacheAlloc_lock);
    // First time use. Allocate a cache in C heap
    if (_oop_map_cache == NULL) {
      _oop_map_cache = new OopMapCache();
    }
  }
  // _oop_map_cache is constant after init; lookup below does is own locking.
  _oop_map_cache->lookup(method, bci, entry_for);
}


bool instanceKlass::find_local_field(symbolOop name, symbolOop sig, fieldDescriptor* fd) const {  
  const int n = fields()->length();
  for (int i = 0; i < n; i += next_offset ) {
    int name_index = fields()->ushort_at(i + name_index_offset);
    int sig_index  = fields()->ushort_at(i + signature_index_offset);
    symbolOop f_name = constants()->symbol_at(name_index);
    symbolOop f_sig  = constants()->symbol_at(sig_index);
    if (f_name == name && f_sig == sig) {
      fd->initialize(as_klassOop(), i);
      return true;
    }
  }
  return false;
}


void instanceKlass::field_names_and_sigs_iterate(OopClosure* closure) {  
  const int n = fields()->length();
  for (int i = 0; i < n; i += next_offset ) {
    int name_index = fields()->ushort_at(i + name_index_offset);
    symbolOop name = constants()->symbol_at(name_index);
    closure->do_oop((oop*)&name);

    int sig_index  = fields()->ushort_at(i + signature_index_offset);
    symbolOop sig = constants()->symbol_at(sig_index);
    closure->do_oop((oop*)&sig);
  }
}


klassOop instanceKlass::find_interface_field(symbolOop name, symbolOop sig, fieldDescriptor* fd) const {
  const int n = local_interfaces()->length();
  for (int i = 0; i < n; i++) {
    klassOop intf1 = klassOop(local_interfaces()->obj_at(i));
    assert(Klass::cast(intf1)->is_interface(), "just checking type");
    // search for field in current interface
    if (instanceKlass::cast(intf1)->find_local_field(name, sig, fd)) {
      assert(fd->is_static(), "interface field must be static");
      return intf1;
    }
    // search for field in direct superinterfaces
    klassOop intf2 = instanceKlass::cast(intf1)->find_interface_field(name, sig, fd);
    if (intf2 != NULL) return intf2;
  }
  // otherwise field lookup fails
  return NULL;
}


klassOop instanceKlass::find_field(symbolOop name, symbolOop sig, fieldDescriptor* fd) const {
  // search order according to newest JVM spec (5.4.3.2, p.167).
  // 1) search for field in current klass
  if (find_local_field(name, sig, fd)) {
    return as_klassOop();
  }
  // 2) search for field recursively in direct superinterfaces
  { klassOop intf = find_interface_field(name, sig, fd);
    if (intf != NULL) return intf;
  }
  // 3) apply field lookup recursively if superclass exists
  { klassOop supr = super();
    if (supr != NULL) return instanceKlass::cast(supr)->find_field(name, sig, fd);
  }
  // 4) otherwise field lookup fails
  return NULL;
}


klassOop instanceKlass::find_field(symbolOop name, symbolOop sig, bool is_static, fieldDescriptor* fd) const {
  // search order according to newest JVM spec (5.4.3.2, p.167).
  // 1) search for field in current klass
  if (find_local_field(name, sig, fd)) {
    if (fd->is_static() == is_static) return as_klassOop();
  }
  // 2) search for field recursively in direct superinterfaces
  if (is_static) {
    klassOop intf = find_interface_field(name, sig, fd);
    if (intf != NULL) return intf;
  }
  // 3) apply field lookup recursively if superclass exists
  { klassOop supr = super();
    if (supr != NULL) return instanceKlass::cast(supr)->find_field(name, sig, is_static, fd);
  }
  // 4) otherwise field lookup fails
  return NULL;
}


bool instanceKlass::find_local_field_from_offset(int offset, bool is_static, fieldDescriptor* fd) const {  
  int length = fields()->length();
  for (int i = 0; i < length; i += next_offset) {
    if (offset_from_fields( i ) == offset) {
      fd->initialize(as_klassOop(), i);      
      if (fd->is_static() == is_static) return true;
    }
  }
  return false;
}


bool instanceKlass::find_field_from_offset(int offset, bool is_static, fieldDescriptor* fd) const {
  klassOop klass = as_klassOop();
  while (klass != NULL) {
    if (instanceKlass::cast(klass)->find_local_field_from_offset(offset, is_static, fd)) {
      return true;
    }
    klass = Klass::cast(klass)->super();
  }
  return false;
}


void instanceKlass::methods_do(void f(methodOop method)) {
  int len = methods()->length();
  for (int index = 0; index < len; index++) {
    methodOop m = methodOop(methods()->obj_at(index));
    assert(m->is_method(), "must be method");
    f(m);
  }
}


void instanceKlass::do_local_static_fields(void f(fieldDescriptor*, oop), oop obj) {  
  fieldDescriptor fd;
  int length = fields()->length();
  for (int i = 0; i < length; i += next_offset) {
    fd.initialize(as_klassOop(), i);
    if (fd.is_static()) f(&fd, obj);
  }
}


void instanceKlass::do_local_static_fields(void f(fieldDescriptor*, TRAPS), TRAPS) {
  instanceKlassHandle h_this(THREAD, as_klassOop());
  do_local_static_fields_impl(h_this, f, CHECK);
}
 

void instanceKlass::do_local_static_fields_impl(instanceKlassHandle this_oop, void f(fieldDescriptor* fd, TRAPS), TRAPS) {
  fieldDescriptor fd;
  int length = this_oop->fields()->length();
  for (int i = 0; i < length; i += next_offset) {
    fd.initialize(this_oop(), i);
    if (fd.is_static()) { f(&fd, CHECK); } // Do NOT remove {}! (CHECK macro expands into several statements)
  }
}


void instanceKlass::do_nonstatic_fields(void f(fieldDescriptor*, oop), oop obj) {
  fieldDescriptor fd;
  instanceKlass* super = superklass();
  if (super != NULL) {
    super->do_nonstatic_fields(f, obj);
  }
  int length = fields()->length();
  for (int i = 0; i < length; i += next_offset) {
    fd.initialize(as_klassOop(), i);
    if (!(fd.is_static())) f(&fd, obj);
  }  
}


void instanceKlass::array_klasses_do(void f(klassOop k)) {
  if (array_klasses() != NULL)
    arrayKlass::cast(array_klasses())->array_klasses_do(f);
}


void instanceKlass::with_array_klasses_do(void f(klassOop k)) {
  f(as_klassOop());
  array_klasses_do(f);
}

#ifdef ASSERT
static int linear_search(objArrayOop methods, symbolOop name, symbolOop signature) {
  int len = methods->length();
  for (int index = 0; index < len; index++) {
    methodOop m = (methodOop)(methods->obj_at(index));
    assert(m->is_method(), "must be method");
    if (m->signature() == signature && m->name() == name) {
       return index;
    }
  }
  return -1;
}
#endif

methodOop instanceKlass::find_method(symbolOop name, symbolOop signature) const {
  return instanceKlass::find_method(methods(), name, signature);
}

methodOop instanceKlass::find_method(objArrayOop methods, symbolOop name, symbolOop signature) {
  int len = methods->length();
  // methods are sorted, so do binary search
  int l = 0;
  int h = len - 1;
  while (l <= h) {
    int mid = (l + h) >> 1;
    methodOop m = (methodOop)methods->obj_at(mid);
    assert(m->is_method(), "must be method");
    int res = m->name()->fast_compare(name);
    if (res == 0) {
      // found matching name; do linear search to find matching signature
      // first, quick check for common case 
      if (m->signature() == signature) return m;
      // search downwards through overloaded methods
      int i;
      for (i = mid - 1; i >= l; i--) {
        methodOop m = (methodOop)methods->obj_at(i);
        assert(m->is_method(), "must be method");
        if (m->name() != name) break;
        if (m->signature() == signature) return m;
      }
      // search upwards
      for (i = mid + 1; i <= h; i++) {
        methodOop m = (methodOop)methods->obj_at(i);
        assert(m->is_method(), "must be method");
        if (m->name() != name) break;
        if (m->signature() == signature) return m;
      }
      // not found
#ifdef ASSERT
      int index = linear_search(methods, name, signature);
      if (index != -1) fatal1("binary search bug: should have found entry %d", index);
#endif
      return NULL;
    } else if (res < 0) {
      l = mid + 1;
    } else {
      h = mid - 1;
    }
  }
#ifdef ASSERT
  int index = linear_search(methods, name, signature);
  if (index != -1) fatal1("binary search bug: should have found entry %d", index);
#endif
  return NULL;
} 

int instanceKlass::method_index_for(methodOop method, TRAPS) const {
  int len = methods()->length();
  for (int index = 0; index < len; index++) {
    if (methodOop(methods()->obj_at(index)) == method) {
      return index;
    }
  }
  THROW_(vmSymbols::java_lang_NoSuchMethodException(), -1);
}


methodOop instanceKlass::uncached_lookup_method(symbolOop name, symbolOop signature) const {  
  klassOop klass = as_klassOop();
  while (klass != NULL) {
    methodOop method = instanceKlass::cast(klass)->find_method(name, signature);
    if (method != NULL) return method;
    klass = instanceKlass::cast(klass)->super();
  }
  return NULL;
}

// lookup a method in all the interfaces that this class implements
methodOop instanceKlass::lookup_method_in_all_interfaces(symbolOop name, 
                                                         symbolOop signature) const {
  objArrayOop all_ifs = instanceKlass::cast(as_klassOop())->transitive_interfaces();
  int num_ifs = all_ifs->length();
  instanceKlass *ik = NULL;
  for (int i = 0; i < num_ifs; i++) {
    ik = instanceKlass::cast(klassOop(all_ifs->obj_at(i)));
    methodOop m = ik->lookup_method(name, signature);
    if (m != NULL) {
      return m;
    }
  }
  return NULL;
}

/* jni_id_for_impl for jfieldIds only */
JNIid* instanceKlass::jni_id_for_impl(instanceKlassHandle this_oop, int offset) {
  MutexLocker ml(JNIIdentifier_lock);
  // Retry lookup after we got the lock
  JNIid* probe = this_oop->jni_ids() == NULL ? NULL : this_oop->jni_ids()->find(offset);
  if (probe == NULL) {
    // Slow case, allocate new static field identifier
    probe = new JNIid(this_oop->as_klassOop(), offset, this_oop->jni_ids());
    this_oop->set_jni_ids(probe);
  }
  return probe;
}


/* jni_id_for for jfieldIds only */
JNIid* instanceKlass::jni_id_for(int offset) {
  JNIid* probe = jni_ids() == NULL ? NULL : jni_ids()->find(offset);
  if (probe == NULL) {
    probe = jni_id_for_impl(this->as_klassOop(), offset);
  }
  return probe;
}

//
// nmethodBucket is used to record dependent nmethods for
// deoptimization.  nmethod dependencies are actually <klass, method>
// pairs but we really only care about the klass part for purposes of
// finding nmethods which might need to be deoptimized.  Instead of
// recording the method, a count of how many times a particular nmethod
// was recorded is kept.  This ensures that any recording errors are
// noticed since an nmethod should be removed as many times are it's
// added.
//
class nmethodBucket {
 private:
  nmethod*       _nmethod;
  int            _count;
  nmethodBucket* _next;

 public:
  nmethodBucket(nmethod* nmethod, nmethodBucket* next) {
    _nmethod = nmethod;
    _next = next;
    _count = 1;
  }
  int increment()                         { _count += 1; return _count; }
  int decrement()                         { _count -= 1; assert(_count >= 0, "don't underflow"); return _count; }
  nmethodBucket* next()                   { return _next; }
  void set_next(nmethodBucket* b)         { _next = b; }
  nmethod* get_nmethod()                  { return _nmethod; }
};


#ifndef CORE

//
// Walk the list of dependent nmethods searching for nmethods which
// are dependent on the klassOop that was passed in and mark them for
// deoptimization.  Returns the number of nmethods found.
//
int instanceKlass::mark_dependent_nmethods(klassOop dependee) {
  assert_locked_or_safepoint(CodeCache_lock);
  int found = 0;
  nmethodBucket* b = _dependencies;
  while (b != NULL) {
    nmethod* nm = b->get_nmethod();
    // since dependencies aren't removed until an nmethod becomes a zombie,
    // the dependency list may contain nmethods which aren't alive.
    if (nm->is_alive() && !nm->is_marked_for_deoptimization() && nm->is_dependent_on(dependee)) {
      if (TraceDependencies) {
        ResourceMark rm;
        tty->print_cr("Marked for deoptimization");
        tty->print_cr("  dependee = %s", this->external_name());
        nm->print();
        nm->print_dependencies();
      }
      nm->mark_for_deoptimization();
      found++;
    }
    b = b->next();
  }
  return found;
}


//
// Add an nmethodBucket to the list of dependencies for this nmethod.
// It's possible that an nmethod has multiple dependencies on this klass
// so a count is kept for each bucket to guarantee that creation and
// deletion of dependencies is consistent.
//
void instanceKlass::add_dependent_nmethod(nmethod* nm) {
  assert_locked_or_safepoint(CodeCache_lock);
  nmethodBucket* b = _dependencies;
  nmethodBucket* last = NULL;
  while (b != NULL) {
    if (nm == b->get_nmethod()) {
      b->increment();
      return;
    }
    b = b->next();
  }
  _dependencies = new nmethodBucket(nm, _dependencies);
}


// 
// Decrement count of the nmethod in the dependency list and remove
// the bucket competely when the count goes to 0.  This method must
// find a corresponding bucket otherwise there's a bug in the
// recording of dependecies.
// 
void instanceKlass::remove_dependent_nmethod(nmethod* nm) {
  assert_locked_or_safepoint(CodeCache_lock);
  nmethodBucket* b = _dependencies;
  nmethodBucket* last = NULL;
  while (b != NULL) {
    if (nm == b->get_nmethod()) {
      if (b->decrement() == 0) {
        if (last == NULL) {
          _dependencies = b->next();
        } else {
          last->set_next(b->next());
        }
        delete b;
      }
      return;
    }
    last = b;
    b = b->next();
  }
#ifdef ASSERT
  tty->print_cr("### %s can't find dependent nmethod:", this->external_name());
  nm->print();
#endif // ASSERT
  ShouldNotReachHere();
}


#endif // !CORE


void instanceKlass::follow_static_fields() {
  oop* start = start_of_static_fields();
  oop* end   = start + static_oop_field_size();
  while (start < end) {
    if (*start != NULL) {
      assert(Universe::heap()->is_in(*start), "should be in heap");
      MarkSweep::mark_and_push(start);
    }
    start++;
  }
}


void instanceKlass::adjust_static_fields() {
  oop* start = start_of_static_fields();
  oop* end   = start + static_oop_field_size();
  while (start < end) {
    MarkSweep::adjust_pointer(start);
    start++;
  }
}


void instanceKlass::oop_follow_contents(oop obj) {
  assert (obj!=NULL, "can't follow the content of NULL object");
  obj->follow_header();
  OopMapBlock* map     = start_of_nonstatic_oop_maps();
  OopMapBlock* end_map = map + nonstatic_oop_map_size();
  while (map < end_map) {
    oop* start = obj->obj_field_addr(map->offset());
    oop* end   = start + map->length();
    while (start < end) {
      if (*start != NULL) {
        assert(Universe::heap()->is_in(*start), "should be in heap");
        MarkSweep::mark_and_push(start);
      }
      start++;
    }
    map++;
  }
}

#define invoke_closure_on(start, closure, nv_suffix) {                          \
  oop obj = *(start);                                                           \
  if (obj != NULL) {                                                            \
    /* We use is_in_reserved() since is_in() is more expensive                  \
       (even though this code is non-product).                  */              \
    assert(Universe::heap()->is_in_reserved(obj), "should be in heap");         \
    (closure)->do_oop##nv_suffix(start);                                        \
  }                                                                             \
}

// closure's do_header() method dicates whether the given closure should be
// applied to the klass ptr in the object header.

#define InstanceKlass_OOP_OOP_ITERATE_DEFN(OopClosureType, nv_suffix)           \
                                                                                \
int instanceKlass::oop_oop_iterate##nv_suffix(oop obj,                          \
                                              OopClosureType* closure) {        \
  SpecializationStats::record_iterate_call##nv_suffix(SpecializationStats::ik); \
  /* header */                                                                  \
  if (closure->do_header()) {                                                   \
    obj->oop_iterate_header(closure);                                           \
  }                                                                             \
  /* instance variables */                                                      \
  OopMapBlock* map     = start_of_nonstatic_oop_maps();                         \
  OopMapBlock* const end_map = map + nonstatic_oop_map_size();                  \
  const intx field_offset    = PrefetchFieldsAhead;                             \
  if (field_offset > 0) {                                                       \
    while (map < end_map) {                                                     \
      oop* start = obj->obj_field_addr(map->offset());                          \
      oop* const end   = start + map->length();                                 \
      while (start < end) {                                                     \
        prefetch_beyond(start, (oop*)end, field_offset,                         \
                        closure->prefetch_style());                             \
        SpecializationStats::                                                   \
          record_do_oop_call##nv_suffix(SpecializationStats::ik);               \
        invoke_closure_on(start, closure, nv_suffix);                           \
        start++;                                                                \
      }                                                                         \
      map++;                                                                    \
    }                                                                           \
  } else {                                                                      \
    while (map < end_map) {                                                     \
      oop* start = obj->obj_field_addr(map->offset());                          \
      oop* const end   = start + map->length();                                 \
      while (start < end) {                                                     \
        SpecializationStats::                                                   \
          record_do_oop_call##nv_suffix(SpecializationStats::ik);               \
        invoke_closure_on(start, closure, nv_suffix);                           \
        start++;                                                                \
      }                                                                         \
      map++;                                                                    \
    }                                                                           \
  }                                                                             \
  return size_helper();                                                         \
}

#define InstanceKlass_OOP_OOP_ITERATE_DEFN_m(OopClosureType, nv_suffix)         \
                                                                                \
int instanceKlass::oop_oop_iterate##nv_suffix##_m(oop obj,                      \
                                                  OopClosureType* closure,      \
                                                  MemRegion mr) {               \
  SpecializationStats::record_iterate_call##nv_suffix(SpecializationStats::ik); \
  /* header */                                                                  \
  if (closure->do_header()) {                                                   \
    obj->oop_iterate_header(closure, mr);                                       \
  }                                                                             \
  /* instance variables */                                                      \
  OopMapBlock* map     = start_of_nonstatic_oop_maps();                         \
  OopMapBlock* const end_map = map + nonstatic_oop_map_size();                  \
  HeapWord* bot = mr.start();                                                   \
  HeapWord* top = mr.end();                                                     \
  oop* start = obj->obj_field_addr(map->offset());                              \
  HeapWord* end = MIN2((HeapWord*)(start + map->length()), top);                \
  /* Find the first map entry that extends onto mr. */                          \
  while (map < end_map && end <= bot) {                                         \
    map++;                                                                      \
    start = obj->obj_field_addr(map->offset());                                 \
    end = MIN2((HeapWord*)(start + map->length()), top);                        \
  }                                                                             \
  if (map != end_map) {                                                         \
    /* The current map's end is past the start of "mr".  Skip up to the first   \
       entry on "mr". */                                                        \
    while ((HeapWord*)start < bot) {                                            \
      start++;                                                                  \
    }                                                                           \
    const intx field_offset = PrefetchFieldsAhead;                              \
    for (;;) {                                                                  \
      if (field_offset > 0) {                                                   \
        while ((HeapWord*)start < end) {                                        \
          prefetch_beyond(start, (oop*)end, field_offset,                       \
                          closure->prefetch_style());                           \
          invoke_closure_on(start, closure, nv_suffix);                         \
          start++;                                                              \
        }                                                                       \
      } else {                                                                  \
        while ((HeapWord*)start < end) {                                        \
          invoke_closure_on(start, closure, nv_suffix);                         \
          start++;                                                              \
        }                                                                       \
      }                                                                         \
      /* Go to the next map. */                                                 \
      map++;                                                                    \
      if (map == end_map) {                                                     \
        break;                                                                  \
      }                                                                         \
      /* Otherwise,  */                                                         \
      start = obj->obj_field_addr(map->offset());                               \
      if ((HeapWord*)start >= top) {                                            \
        break;                                                                  \
      }                                                                         \
      end = MIN2((HeapWord*)(start + map->length()), top);                      \
    }                                                                           \
  }                                                                             \
  return size_helper();                                                         \
}

ALL_OOP_OOP_ITERATE_CLOSURES_1(InstanceKlass_OOP_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_2(InstanceKlass_OOP_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_3(InstanceKlass_OOP_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_1(InstanceKlass_OOP_OOP_ITERATE_DEFN_m)
ALL_OOP_OOP_ITERATE_CLOSURES_2(InstanceKlass_OOP_OOP_ITERATE_DEFN_m)
ALL_OOP_OOP_ITERATE_CLOSURES_3(InstanceKlass_OOP_OOP_ITERATE_DEFN_m)


void instanceKlass::iterate_static_fields(OopClosure* closure) {
  oop* start = start_of_static_fields();
  oop* end   = start + static_oop_field_size();
  while (start < end) {
    assert(Universe::heap()->is_in_or_null(*start), "should be in heap");
    closure->do_oop(start);
    start++;
  }
}

void instanceKlass::iterate_static_fields(OopClosure* closure,
                                          MemRegion mr) {
  oop* start = start_of_static_fields();
  oop* end   = start + static_oop_field_size();
  // I gather that the the static fields of reference types come first,
  // hence the name of "oop_field_size", and that is what makes this safe.
  assert((intptr_t)mr.start() ==
         align_size_up((intptr_t)mr.start(), sizeof(oop)) &&
         (intptr_t)mr.end() == align_size_up((intptr_t)mr.end(), sizeof(oop)),
         "Memregion must be oop-aligned.");
  if ((HeapWord*)start < mr.start()) start = (oop*)mr.start();
  if ((HeapWord*)end   > mr.end())   end   = (oop*)mr.end();
  while (start < end) {
    invoke_closure_on(start, closure,_v);
    start++;
  }
}


int instanceKlass::oop_adjust_pointers(oop obj) {
  int size = size_helper();

  // Compute oopmap block range. The common case is nonstatic_oop_map_size == 1.
  OopMapBlock* map     = start_of_nonstatic_oop_maps();
  OopMapBlock* const end_map = map + nonstatic_oop_map_size();
  // Iterate over oopmap blocks
  while (map < end_map) {
    // Compute oop range for this block
    oop* start = obj->obj_field_addr(map->offset());
    oop* end   = start + map->length();
    // Iterate over oops
    while (start < end) {
      assert(Universe::heap()->is_in_or_null(*start), "should be in heap");
      MarkSweep::adjust_pointer(start);
      start++;
    }
    map++;
  }

  obj->adjust_header();
  return size;
}

void instanceKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  // Compute oopmap block range. The common case is nonstatic_oop_map_size == 1.
  OopMapBlock* map     = start_of_nonstatic_oop_maps();
  OopMapBlock* end_map = map + nonstatic_oop_map_size();

  // Iterate over oopmap blocks
  while (map < end_map) {
    // Compute oop range for this block
    oop* start = obj->obj_field_addr(map->offset());
    oop* end   = start + map->length();
    // Iterate over oops
    while (start < end) {
      if (PSScavenge::should_scavenge(*start)) {
        assert(Universe::heap()->is_in(*start), "should be in heap");
        pm->claim_or_forward(start);
      }
      start++;
    }
    map++;
  }
}

void instanceKlass::copy_static_fields(PSPromotionManager* pm) {
  // Compute oop range
  oop* start = start_of_static_fields();
  oop* end   = start + static_oop_field_size();
  // Iterate over oops
  while (start < end) {
    if (PSScavenge::should_scavenge(*start)) {
      assert(Universe::heap()->is_in(*start), "should be in heap");
      pm->claim_or_forward(start);
    }
    start++;
  }
}

// This klass is alive but the implementor link is not followed/updated.
// Subklass and sibling links are handled by Klass::follow_weak_klass_links

void instanceKlass::follow_weak_klass_links(
  BoolObjectClosure* is_alive, OopClosure* keep_alive) {
  assert(is_alive->do_object_b(as_klassOop()), "this oop should be live");
  if (ClassUnloading) {
    if (implementor() != NULL) {
      if (!is_alive->do_object_b(implementor())) {
        assert(nof_implementors() == 1, "just checking");
        init_implementor();
      }
    }
  } else {
    keep_alive->do_oop(adr_implementor());
  }
  Klass::follow_weak_klass_links(is_alive, keep_alive);
}


void instanceKlass::remove_unshareable_info() {
  Klass::remove_unshareable_info();
  init_implementor();
}


static void clear_all_breakpoints(methodOop m) {
  m->clear_all_breakpoints();
}


void instanceKlass::release_C_heap_structures() {
  // Deallocate oop map cache
  if (_oop_map_cache != NULL) {
    delete _oop_map_cache;
    _oop_map_cache = NULL;
  }

  // Deallocate JNI identifiers for jfieldIDs
  JNIid::deallocate(jni_ids());
  set_jni_ids(NULL);

  // Deallocate map of JNI identifiers (and thus all jmethodIDs) 
  jniIdMapBase* map = jni_id_map();
  set_jni_id_map(NULL);
  if (map != NULL) {
    jniIdMapBase::deallocate(map);
  }

  // release dependencies
  nmethodBucket* b = _dependencies;
  _dependencies = NULL;
  while (b != NULL) {
    nmethodBucket* next = b->next();
    delete b;
    b = next;
  }

  // Deallocate breakpoint records
  if (breakpoints() != 0x0) {
    methods_do(clear_all_breakpoints);
    assert(breakpoints() == 0x0, "should have cleared breakpoints");
  }
}

char* instanceKlass::signature_name() const {
  const char* src = (const char*) (name()->as_C_string());
  const int src_length = (int)strlen(src);
  char* dest = NEW_RESOURCE_ARRAY(char, src_length + 3);
  int src_index = 0;
  int dest_index = 0;
  dest[dest_index++] = 'L';
  while (src_index < src_length) {
    dest[dest_index++] = src[src_index++];
  }
  dest[dest_index++] = ';';
  dest[dest_index] = '\0';
  return dest;
}

// different verisons of is_same_class_package
bool instanceKlass::is_same_class_package(klassOop class2) {
  klassOop class1 = as_klassOop();
  oop classloader1 = instanceKlass::cast(class1)->class_loader();
  symbolOop classname1 = Klass::cast(class1)->name();

  if (Klass::cast(class2)->oop_is_objArray()) {
    class2 = objArrayKlass::cast(class2)->bottom_klass();
  }
  oop classloader2;  
  if (Klass::cast(class2)->oop_is_instance()) {
    classloader2 = instanceKlass::cast(class2)->class_loader();
  } else {
    assert(Klass::cast(class2)->oop_is_typeArray(), "should be type array");
    classloader2 = NULL;
  }
  symbolOop classname2 = Klass::cast(class2)->name();

  return instanceKlass::is_same_class_package(classloader1, classname1,
                                              classloader2, classname2);
}

bool instanceKlass::is_same_class_package(oop classloader2, symbolOop classname2) {
  klassOop class1 = as_klassOop();
  oop classloader1 = instanceKlass::cast(class1)->class_loader();
  symbolOop classname1 = Klass::cast(class1)->name();

  return instanceKlass::is_same_class_package(classloader1, classname1,
                                              classloader2, classname2);
}

// return true if two classes are in the same package, classloader 
// and classname information is enough to determine a class's package
bool instanceKlass::is_same_class_package(oop class_loader1, symbolOop class_name1, 
                                          oop class_loader2, symbolOop class_name2) {
  if (class_loader1 != class_loader2) {
    return false;
  } else {
    ResourceMark rm;

    // The symbolOop's are in UTF8 encoding. Since we only need to check explicitly
    // for ASCII characters ('/', 'L', '['), we can keep them in UTF8 encoding.
    // Otherwise, we just compare jbyte values between the strings.
    jbyte *name1 = class_name1->base();
    jbyte *name2 = class_name2->base();

    jbyte *last_slash1 = UTF8::strrchr(name1, class_name1->utf8_length(), '/');
    jbyte *last_slash2 = UTF8::strrchr(name2, class_name2->utf8_length(), '/');
    
    if ((last_slash1 == NULL) || (last_slash2 == NULL)) {
      // One of the two doesn't have a package.  Only return true
      // if the other one also doesn't have a package.
      return last_slash1 == last_slash2; 
    } else {
      // Skip over '['s
      if (*name1 == '[') {
        do {
          name1++;
        } while (*name1 == '[');
        if (*name1 != 'L') {
          // Something is terribly wrong.  Shouldn't be here.
          return false;
        }
      }
      if (*name2 == '[') {
        do {
          name2++;
        } while (*name2 == '[');
        if (*name2 != 'L') {
          // Something is terribly wrong.  Shouldn't be here.
          return false;
        }
      }

      // Check that package part is identical
      int length1 = last_slash1 - name1;
      int length2 = last_slash2 - name2;

      return UTF8::equal(name1, length1, name2, length2);      
    }
  }
}


jint instanceKlass::compute_modifier_flags(TRAPS) const {
  klassOop k = as_klassOop();
  jint access = access_flags().as_int();

  // But check if it happens to be member class.
  typeArrayOop inner_class_list = inner_classes();
  int length = (inner_class_list == NULL) ? 0 : inner_class_list->length();
  assert (length % instanceKlass::inner_class_next_offset == 0, "just checking");
  if (length > 0) {
    typeArrayHandle inner_class_list_h(THREAD, inner_class_list);
    instanceKlassHandle ik(THREAD, k);
    for (int i = 0; i < length; i += instanceKlass::inner_class_next_offset) {
      int ioff = inner_class_list_h->ushort_at(
                      i + instanceKlass::inner_class_inner_class_info_offset);

      // Inner class attribute can be zero, skip it.
      // Strange but true:  JVM spec. allows null inner class refs.
      if (ioff == 0) continue;

      // only look at classes that are already loaded
      // since we are looking for the flags for our self.
      symbolOop inner_name = ik->constants()->klass_name_at(ioff);
      if ((ik->name() == inner_name)) {
        // This is really a member class.
        access = inner_class_list_h->ushort_at(i + instanceKlass::inner_class_access_flags_offset);
        break;
      }
    }
  }
  // Remember to strip ACC_SUPER bit
  return (access & (~JVM_ACC_SUPER)) & JVM_ACC_WRITTEN_FLAGS;
}

jint instanceKlass::jvmti_class_status() const {
  jint result = 0;

  if (is_linked()) {
    result |= JVMTI_CLASS_STATUS_VERIFIED | JVMTI_CLASS_STATUS_PREPARED;
  }

  if (is_initialized()) {
    assert(is_linked(), "Class status is not consistent");
    result |= JVMTI_CLASS_STATUS_INITIALIZED;
  }
  if (is_in_error_state()) {
    result |= JVMTI_CLASS_STATUS_ERROR;
  }
  return result;
}

methodOop instanceKlass::method_at_itable(klassOop holder, int index, TRAPS) {
  itableOffsetEntry* ioe = (itableOffsetEntry*)start_of_itable();
  int method_table_offset_in_words = ioe->offset()/wordSize;
  int nof_interfaces = (method_table_offset_in_words - itable_offset_in_words())
                       / itableOffsetEntry::size();

  for (int cnt = 0 ; ; cnt ++, ioe ++) {
    // If the interface isn't implemented by the reciever class,
    // the VM should throw IncompatibleClassChangeError.
    if (cnt >= nof_interfaces) {
      THROW_OOP_0(vmSymbols::java_lang_IncompatibleClassChangeError());
    }

    klassOop ik = ioe->interface_klass();
    if (ik == holder) break;
  }

  itableMethodEntry* ime = ioe->first_method_entry(as_klassOop());
  methodOop m = ime[index].method();
  if (m == NULL) {
    THROW_OOP_0(vmSymbols::java_lang_AbstractMethodError());
  }
  return m;
}

// On-stack replacement stuff
#ifndef CORE

void instanceKlass::add_osr_nmethod(nmethod* n) {
  // only one compilation can be active
  NEEDS_CLEANUP
  //COMPILER1_ONLY(Unimplemented(); /*assert(C1Compiler::is_active(), "");*/)
  // This is a short non-blocking critical region, so the no safepoint check is ok. 
  OsrList_lock->lock_without_safepoint_check();
  assert(n->is_osr_method(), "wrong kind of nmethod");  
  n->set_link(osr_nmethods_head());
  set_osr_nmethods_head(n);  
  // Remember to unlock again
  OsrList_lock->unlock();
}


void instanceKlass::remove_osr_nmethod(nmethod* n) {
  // This is a short non-blocking critical region, so the no safepoint check is ok. 
  OsrList_lock->lock_without_safepoint_check();
  assert(n->is_osr_method(), "wrong kind of nmethod");
  nmethod* last = NULL;
  nmethod* cur  = osr_nmethods_head();
  // Search for match
  while(cur != NULL && cur != n) {
    last = cur;
    cur = cur->link();    
  }   
  if (cur == n) {
    if (last == NULL) {
      // Remove first element
      set_osr_nmethods_head(osr_nmethods_head()->link());      
    } else {
      last->set_link(cur->link());
    }
  }
  n->set_link(NULL);  
  // Remember to unlock again
  OsrList_lock->unlock();
}

nmethod* instanceKlass::lookup_osr_nmethod(const methodOop m, int bci) const {
  // This is a short non-blocking critical region, so the no safepoint check is ok. 
  OsrList_lock->lock_without_safepoint_check();
  nmethod* osr = osr_nmethods_head();
  while (osr != NULL) {
    assert(osr->is_osr_method(), "wrong kind of nmethod found in chain");
    if (osr->method() == m && 
        (bci == InvocationEntryBci || osr->osr_entry_bci() == bci)) {
      // Found a match - return it.        
      OsrList_lock->unlock();
      return osr;
    }    
    osr = osr->link();
  }
  OsrList_lock->unlock();
  return NULL;
}
#endif

// -----------------------------------------------------------------------------------------------------

#ifndef PRODUCT

// Printing

static outputStream* printing_stream = NULL;

static void print_nonstatic_field(fieldDescriptor* fd, oop obj) {
  assert(printing_stream != NULL, "Invalid printing stream");
  printing_stream->print("   - ");
  fd->print_on_for(printing_stream, obj);
  printing_stream->cr();
}


void instanceKlass::oop_print_on(oop obj, outputStream* st) {
  Klass::oop_print_on(obj, st);

  if (as_klassOop() == SystemDictionary::string_klass()) {
    typeArrayOop value  = java_lang_String::value(obj);
    juint        offset = java_lang_String::offset(obj);
    juint        length = java_lang_String::length(obj);
    if (value != NULL &&
        value->is_typeArray() &&
        offset          <= (juint) value->length() &&
        offset + length <= (juint) value->length()) {
      st->print("string: ");
      Handle h_obj(obj);
      java_lang_String::print(h_obj, st);
      st->cr();
      if (!WizardMode)  return;  // that is enough
    }
  }

  if (WizardMode) {
    st->print("header: ");
    obj->mark()->print_on(tty);
    tty->cr();
  }

  st->print_cr("fields:");
  printing_stream = st;
  do_nonstatic_fields(print_nonstatic_field, obj);
  printing_stream = NULL;

  if (as_klassOop() == SystemDictionary::class_klass()) {
    klassOop mirrored_klass = java_lang_Class::as_klassOop(obj);
    st->print("   - fake entry for mirror: ");
    mirrored_klass->print_value_on(st);
    st->cr();
  }
}

void instanceKlass::oop_print_value_on(oop obj, outputStream* st) {
  st->print("a ");
  name()->print_value_on(st);
  obj->print_address_on(st);
}

const char* instanceKlass::internal_name() const {
  return external_name();
}



// Verification

class VerifyFieldClosure: public OopClosure {
 public:
  void do_oop(oop* p) {
    guarantee(Universe::heap()->is_in(p), "should be in heap");
    if (!(*p)->is_oop_or_null()) {
      tty->print_cr("Failed: %p -> %p",p,*p);
      Universe::print();
      guarantee(false, "boom");
    }
    //guarantee((*p)->is_oop_or_null(), "should be in heap");
  }
};


void instanceKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  VerifyFieldClosure blk;
  oop_oop_iterate(obj, &blk);
}


void instanceKlass::verify_class_klass_nonstatic_oop_maps(klassOop k) {
  // This verification code is disabled.  Universe::is_gte_jdk14x_version()
  // cannot be called since this function is called before the VM is
  // able to determine what JDK version is running with.
  // The check below always is false since 1.4.
  return;

  // This verification code temporarily disabled for the 1.4
  // reflection implementation since java.lang.Class now has
  // Java-level instance fields. Should rewrite this to handle this
  // case.
  if (!(Universe::is_gte_jdk14x_version() && UseNewReflection)) {
    // Verify that java.lang.Class instances have a fake oop field added.
    instanceKlass* ik = instanceKlass::cast(k);

    // Check that we have the right class
    static bool first_time = true;
    guarantee(k == SystemDictionary::class_klass() && first_time, "Invalid verify of maps");
    first_time = false;
    const int extra = java_lang_Class::number_of_fake_oop_fields;
    guarantee(ik->nonstatic_field_size() == extra, "just checking");
    guarantee(ik->nonstatic_oop_map_size() == 1, "just checking");
    guarantee(ik->size_helper() == align_object_size(instanceOopDesc::header_size() + extra), "just checking");

    // Check that the map is (2,extra)
    int offset = java_lang_Class::klass_offset;

    OopMapBlock* map = ik->start_of_nonstatic_oop_maps();
    guarantee(map->offset() == offset && map->length() == extra, "just checking");
  }
}

#endif

 
/* JNIid class for jfieldIDs only */
 JNIid::JNIid(klassOop holder, int offset, JNIid* next) {
   _holder = holder;
   _offset = offset;
   _next = next;
   debug_only(_is_static_field_id = false;)
 }
 
 
 JNIid* JNIid::find(int offset) {
   JNIid* current = this;
   while (current != NULL) {
     if (current->offset() == offset) return current;
     current = current->next();
   }
   return NULL;
 }
 
 void JNIid::oops_do(OopClosure* f) {
   JNIid* current = this;
   while (current != NULL) {
     f->do_oop((oop*)&(current->_holder));
     current = current->next();
   }
 }
 
 
 void JNIid::deallocate(JNIid* current) {
   while (current != NULL) {
     JNIid* next = current->next();
     delete current;
     current = next;
   }
 }
 
 
 #ifndef PRODUCT
 
 void JNIid::verify(klassOop holder) {
   int first_field_offset  = instanceKlass::cast(holder)->offset_of_static_fields();
   int end_field_offset;
   end_field_offset = first_field_offset + (instanceKlass::cast(holder)->static_field_size() * wordSize);
 
   JNIid* current = this;
   while (current != NULL) {
     guarantee(current->holder() == holder, "Invalid klass in JNIid");
 #ifdef ASSERT
     int o = current->offset();
     if (current->is_static_field_id()) {
       guarantee(o >= first_field_offset  && o < end_field_offset,  "Invalid static field offset in JNIid");
     }
 #endif
     current = current->next();
   }
 }
 
 #endif


#ifdef ASSERT
  void instanceKlass::set_init_state(ClassState state) { 
    bool good_state = as_klassOop()->is_shared() ? (_init_state <= state)
                                                 : (_init_state < state);
    assert(good_state || state == allocated, "illegal state transition");
    _init_state = state; 
  }
#endif
