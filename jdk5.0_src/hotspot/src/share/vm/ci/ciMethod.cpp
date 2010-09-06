#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciMethod.cpp	1.68 04/03/02 02:08:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciMethod.cpp.incl"

// ciMethod
//
// This class represents a methodOop in the HotSpot virtual
// machine.


// ------------------------------------------------------------------
// ciMethod::ciMethod
//
// Loaded method.
ciMethod::ciMethod(methodHandle h_m) : ciObject(h_m) {
  assert(h_m() != NULL, "no null method");

  // These fields are always filled in in loaded methods.
  _flags = ciFlags(h_m()->access_flags());

  // Easy to compute, so fill them in now.
  _max_stack          = h_m()->max_stack();
  _max_locals         = h_m()->max_locals();  
  _code_size          = h_m()->code_size();
  _intrinsic_id       = (IntrinsicId) h_m()->intrinsic_id();
  _handler_count      = h_m()->exception_table()->length() / 4;
  _uses_monitors      = h_m()->access_flags().has_monitor_bytecodes();
  _balanced_monitors  = !_uses_monitors || h_m()->access_flags().is_monitor_matching();
  _is_compilable      = !h_m()->is_not_compilable();
  // Lazy fields, filled in on demand.  Require allocation.
  _code               = NULL;
  _exception_handlers = NULL;
#ifdef COMPILER1
  _all_oop_maps       = NULL;
#endif // COMPILER1
#ifdef COMPILER2
  _liveness           = NULL;
  _flow               = NULL;
#endif // COMPILER2

  ciEnv *env = CURRENT_ENV;
  // generating _signature may allow GC and therefore move m.
  // These fields are always filled in.
  _name = env->get_object(h_m()->name())->as_symbol();
  _holder = env->get_object(h_m()->method_holder())->as_instance_klass();
  ciSymbol* sig_symbol = env->get_object(h_m()->signature())->as_symbol();
  _signature = new (env->arena()) ciSignature(_holder, sig_symbol);
  _method_data = NULL;
#ifdef COMPILER2
  if (Tier1UpdateMethodData && is_tier1_compile(env->comp_level())) {
    build_method_data(h_m);
  }
  if (h_m()->method_data() != NULL) {
    _method_data = env->get_object(h_m()->method_data())->as_method_data();
  } else {
    _method_data = env->get_empty_methodData();
  }
#endif // COMPILER2
}


// ------------------------------------------------------------------
// ciMethod::ciMethod
//
// Unloaded method.
ciMethod::ciMethod(ciInstanceKlass* holder,
                   ciSymbol* name,
                   ciSymbol* signature) : ciObject(ciMethodKlass::make()) {
  // These fields are always filled in.
  _name = name;
  _holder = holder;
  _signature = new (CURRENT_ENV->arena()) ciSignature(_holder, signature);
  _intrinsic_id = _none;
#ifdef COMPILER1
  _all_oop_maps = NULL;
#endif // COMPILER1
#ifdef COMPILER2
  _liveness = NULL;
  _flow = NULL;
  _method_data = NULL;
#endif // COMPILER2
}


// ------------------------------------------------------------------
// ciMethod::load_code
//
// Load the bytecodes and exception handler table for this method.
void ciMethod::load_code() {
  VM_ENTRY_MARK;
  assert(is_loaded(), "only loaded methods have code");

  methodOop me = get_methodOop();
  Arena* arena = CURRENT_THREAD_ENV->arena();

  // Load the bytecodes.
  _code = (address)arena->Amalloc(code_size());
  memcpy(_code, me->code_base(), code_size());

  // Revert any breakpoint bytecodes in ci's copy
  if (_is_compilable && me->number_of_breakpoints() > 0) {
    BreakpointInfo* bp = instanceKlass::cast(me->method_holder())->breakpoints();
    for (; bp != NULL; bp = bp->next()) {
      if (bp->match(me)) {
        code_at_put(bp->bci(), bp->orig_bytecode());
      }
    }
  }

  // And load the exception table.
  typeArrayOop exc_table = me->exception_table();

  // Allocate one extra spot in our list of exceptions.  This
  // last entry will be used to represent the possibility that
  // an exception escapes the method.  See ciExceptionHandlerStream
  // for details.
  _exception_handlers =
    (ciExceptionHandler**)arena->Amalloc(sizeof(ciExceptionHandler*)
                                         * (_handler_count + 1));
  if (_handler_count > 0) {
    for (int i=0; i<_handler_count; i++) {
      int base = i*4;
      _exception_handlers[i] = new (arena) ciExceptionHandler(
                                holder(),
            /* start    */      exc_table->int_at(base),
            /* limit    */      exc_table->int_at(base+1),
            /* goto pc  */      exc_table->int_at(base+2),
            /* cp index */      exc_table->int_at(base+3));
    }
  }

  // Put an entry at the end of our list to represent the possibility
  // of exceptional exit.
  _exception_handlers[_handler_count] =
    new (arena) ciExceptionHandler(holder(), 0, code_size(), -1, 0);

  // Fill in the collected data. 
  if (method_data() != NULL) {
    method_data()->load_data();
  }

  if (CIPrintMethodCodes) {
    print_codes();
  }
}


// ------------------------------------------------------------------
// ciMethod::has_linenumber_table
//
// length unknown until decompression
bool    ciMethod::has_linenumber_table() const {
  check_is_loaded();
  VM_ENTRY_MARK;
  return get_methodOop()->has_linenumber_table();
}


// ------------------------------------------------------------------
// ciMethod::compressed_linenumber_table
u_char* ciMethod::compressed_linenumber_table() const {
  check_is_loaded();
  VM_ENTRY_MARK;
  return get_methodOop()->compressed_linenumber_table();
}


// ------------------------------------------------------------------
// ciMethod::vtable_index
//
// Get the position of this method's entry in the vtable, if any.
int ciMethod::vtable_index() {
  check_is_loaded();
  VM_ENTRY_MARK;
  return get_methodOop()->vtable_index();
}


// ------------------------------------------------------------------
// ciMethod::native_entry
//
// Get the address of this method's native code, if any.
address ciMethod::native_entry() {
  check_is_loaded();
  assert(flags().is_native(), "must be native method");
  VM_ENTRY_MARK;
  methodOop method = get_methodOop();
  address entry = method->native_function();
  assert(entry != NULL, "must be valid entry point");
  return entry;
}


// ------------------------------------------------------------------
// ciMethod::interpreter_entry
//
// Get the entry point for running this method in the interpreter.
address ciMethod::interpreter_entry() {
  check_is_loaded();
  VM_ENTRY_MARK;
  methodHandle mh(THREAD, get_methodOop());
  return AbstractInterpreter::entry_for_method(mh);
}


// ------------------------------------------------------------------
// ciMethod::uses_balanced_monitors
//
// Does this method use monitors in a strict stack-disciplined manner?
bool ciMethod::has_balanced_monitors() {
  check_is_loaded();
  if (_balanced_monitors) return true;

  // Analyze the method to see if monitors are used properly.
  VM_ENTRY_MARK;
  methodHandle method(THREAD, get_methodOop());
  assert(method->has_monitor_bytecodes(), "should have checked this");

  // Check to see if a previous compilation computed the
  // monitor-matching analysis.
  if (method->guaranteed_monitor_matching()) {
    _balanced_monitors = true;
    return true;
  }

  {
    EXCEPTION_MARK;
    ResourceMark rm(THREAD);
    GeneratePairingInfo gpi(method);
    gpi.compute_map(CATCH);
    if (!gpi.monitor_safe()) {
      return false;
    }
    method->set_guaranteed_monitor_matching();
    _balanced_monitors = true;
  }
  return true;
}


#ifdef COMPILER2
class GenerateOopMapForBCI : public GenerateOopMap {
 private:
  int          _bci;
  BitMap*      _result;
  
 public:
  GenerateOopMapForBCI(methodHandle method, int bci, BitMap* result)
  : GenerateOopMap(method) {
    _bci = bci;
    _result = result;
  }

  void compute_map(TRAPS);
  bool possible_gc_point(BytecodeStream* bcs) { return false; }
  bool report_results() const { return false; }
  void fill_stackmap_prolog(int nof_gc_points) { /* do nothing */ }
  void fill_stackmap_epilog() { /* do nothing */ }
  void fill_stackmap_for_opcodes(BytecodeStream* bcs,
                                 CellTypeState* vars,
                                 CellTypeState* stack,
                                 int stack_top);
  void fill_init_vars(GrowableArray<intptr_t>* init_vars) { /* do nothing */ }
};


void GenerateOopMapForBCI::compute_map(TRAPS) {
  ResourceMark rm;
  GenerateOopMap::compute_map(CATCH);
  result_for_basicblock(_bci);
}


void GenerateOopMapForBCI::fill_stackmap_for_opcodes(BytecodeStream* bcs,
                                                     CellTypeState* vars,
                                                     CellTypeState* stack,
                                                     int stack_top) {
  if (bcs->bci() == _bci) {
    assert(stack_top == 0, "empty stack");
    int max_locals = _method->max_locals();
    
    for (int i=0; i<max_locals; i++) {
      _result->at_put(i, vars[i].is_reference());
    }
  }
}


#endif // COMPILER2


#ifdef COMPILER1
ciLocalMap* ciMethod::all_oop_maps() {
  assert(!is_abstract() && !is_native(), "cannot compute oop maps for abstract or native methods");
  // do nothing if possible
  if (code_size() == 0 || max_locals() == 0) return NULL;
  // compute oop maps
  check_is_loaded();
  VM_ENTRY_MARK;
  if (_all_oop_maps == NULL) {
    Arena* arena = CURRENT_THREAD_ENV->arena();
    ciGenerateLocalMap omg(arena, methodHandle(THREAD, get_methodOop()));
    omg.compute_map(CATCH);  
    ciLocalMap* map = omg.get_oop_maps();
    assert(map != NULL, "map must exist");
    _all_oop_maps = map;
  }
  return _all_oop_maps;
}


#endif // COMPILER1


// ------------------------------------------------------------------
// ciMethod::get_flow_analysis
ciTypeFlow* ciMethod::get_flow_analysis() {
#ifdef COMPILER2
  if (_flow == NULL) {
    ciEnv* env = CURRENT_ENV;
    _flow = new (env->arena()) ciTypeFlow(env, this);
    _flow->do_flow();
  }
  return _flow;
#else // COMPILER2
  ShouldNotReachHere();
  return NULL;
#endif // COMPILER2
}


// ------------------------------------------------------------------
// ciMethod::get_osr_flow_analysis
ciTypeFlow* ciMethod::get_osr_flow_analysis(int osr_bci) {
#ifdef COMPILER2
  // OSR entry points are always place after a call bytecode of some sort
  assert(osr_bci >= 0, "must supply valid OSR entry point");
  ciEnv* env = CURRENT_ENV;
  ciTypeFlow* flow = new (env->arena()) ciTypeFlow(env, this, osr_bci);
  flow->do_flow();
  return flow;
#else // COMPILER2
  ShouldNotReachHere();
  return NULL;
#endif // COMPILER2
}

// ------------------------------------------------------------------
// ciMethod::liveness_at_bci
//
// Which local variables are live at a specific bci?
#ifdef COMPILER2
BitMap ciMethod::liveness_at_bci(int bci) {
  check_is_loaded();
  if (_liveness == NULL) {
    // Create the liveness analyzer.
    Arena* arena = CURRENT_ENV->arena();
    _liveness = new (arena) MethodLiveness(arena, this);
    _liveness->compute_liveness();
  }
  BitMap result = _liveness->get_liveness_at(bci);
  if (JvmtiExport::can_access_local_variables()) {
    // Keep all locals live for the user's edification and amusement.
    result.at_put_range(0, result.size(), true);
    // Also, to support the popframe command, the incoming arguments
    // must be preserved throughout the execution of the method.
    //result.at_put_range(0, arg_size(), true);
  }
  return result;
}
#endif // COMPILER2

// ------------------------------------------------------------------
// ciMethod::init_vars
//
// Which local variables need to be initialized to zero?
#ifdef COMPILER2
BitMap ciMethod::init_vars() {
  check_is_loaded();
  Unimplemented();
  return BitMap(NULL,0);
}
#endif // COMPILER2

// ------------------------------------------------------------------
// ciMethod::call_profile_at_bci
//
// Get the ciCallProfile for the invocation of this method.
ciCallProfile ciMethod::call_profile_at_bci(int bci) {
  ResourceMark rm;
  ciCallProfile result;
  if (method_data() != NULL && method_data()->is_mature()) {
    ciProfileData* data = method_data()->bci_to_data(bci);
    if (data != NULL && data->is_CounterData()) {
      // Every profiled call site has a counter.
      int raw_count = data->as_CounterData()->count();
      result._count = scale_count(raw_count);

      if (!data->is_VirtualCallData()) {
        result._receiver_count = 0;  // that's a definite zero
      } else { // VirtualCallData is a subclass of CounterData
        ciVirtualCallData* call = (ciVirtualCallData*)data->as_VirtualCallData();
        // In addition, virtual call sites have receiver type information
        ciKlass* best_receiver = NULL;
        int best_receiver_count = -1;
        for (uint i = 0; i < call->row_limit(); i++) {
          ciKlass* receiver = call->receiver(i);
          if (receiver == NULL)  continue;
          int receiver_count = call->receiver_count(i);
          if (best_receiver_count < receiver_count) {
            // Find the most common receiver so far.
            best_receiver = receiver;
            best_receiver_count = receiver_count;
          }
        }
        if (best_receiver != NULL) {
          // Report results:
          result._receiver = best_receiver;
          result._receiver_count = scale_count(best_receiver_count);
          // If we extend profiling to record methods,
          // we will set result._method also.
        }

        #ifdef COMPILER2
        // Do not allow a monomorphic result if there have been
        // the wrong sort of deoptimizations here, either once at
        // this BCI, or enough times in the whole method.
        if (result.is_monomorphic()) {
          if (method_data()->trap_count(Deoptimization::Reason_class_check)
              >= (uint)PerMethodTrapLimit) {
            // Force not monomorphic.
            result._receiver_count = result._count-1;
            assert(!result.is_monomorphic(), "mono forced off");
          }
        }
        #endif
      }
    }
  }
  return result;
}

// ------------------------------------------------------------------
// ciCallProfile::apply_prof_factor
void ciCallProfile::apply_prof_factor(float prof_factor) {
  if (_count > 0) {
    int pfc = (_count * prof_factor + 0.5);
    _count = (pfc > 1)? pfc: 1;
  }
  if (_receiver_count > 0) {
    int pfc = (_receiver_count * prof_factor + 0.5);
    _receiver_count = (pfc > 1)? pfc: 1;
  }
}

// ------------------------------------------------------------------
// ciMethod::find_monomorphic_target
//
// Given a certain calling environment, find the monomorphic target
// for the call.  Return NULL if the call is not monomorphic in
// its calling environment.
ciMethod* ciMethod::find_monomorphic_target(ciKlass* caller,
                                            ciKlass* callee_holder,
                                            ciKlass* actual_recv) {
  check_is_loaded();
  VM_ENTRY_MARK;

  KlassHandle caller_klass (THREAD, caller->get_klassOop());
  KlassHandle callee_klass (THREAD, callee_holder->get_klassOop());
  KlassHandle h_recv       (THREAD, actual_recv->get_klassOop());
  symbolHandle h_name      (THREAD, name()->get_symbolOop());
  symbolHandle h_signature (THREAD, signature()->get_symbolOop());

  CHAResult* result =
    CHA::analyze_call(caller_klass, callee_klass, h_recv, h_name, h_signature);
  if (!result->is_monomorphic()) {
    return NULL;
  }
  methodHandle target = result->monomorphic_target();
  if (target() == NULL) {
    return NULL;
  }
  if (target() == get_methodOop()) {
    return this;
  }
  return CURRENT_THREAD_ENV->get_object(target())->as_method();
}

// ------------------------------------------------------------------
// ciMethod::interpreter_invocation_count
int ciMethod::interpreter_invocation_count() {
  VM_ENTRY_MARK;
#ifdef COMPILER2
  int invcnt = (ProfileInterpreter) ?  get_methodOop()->interpreter_invocation_count() : 1;
  return (invcnt == 0) ? 1 : invcnt;
#else // COMPILER2
  ShouldNotReachHere();
  return 1;
#endif // COMPILER2
}

// ------------------------------------------------------------------
// ciMethod::interpreter_call_site_count
int ciMethod::interpreter_call_site_count(int bci) {
  if (method_data() != NULL) {
    ResourceMark rm;
    ciProfileData* data = method_data()->bci_to_data(bci);
    if (data != NULL && data->is_CounterData()) {
      return scale_count(data->as_CounterData()->count());
    }
  }
  return -1;  // unknown
}

// ------------------------------------------------------------------
// ciMethod::interpreter_throwout_count
int ciMethod::interpreter_throwout_count() const {
  VM_ENTRY_MARK;
#ifdef COMPILER2
  return get_methodOop()->interpreter_throwout_count();
#else // COMPILER2
  ShouldNotReachHere();
  return 0;
#endif // COMPILER2
}

// ------------------------------------------------------------------
// Adjust a CounterData count to be commensurate with
// interpreter_invocation_count.  If the MDO exists for
// only 25% of the time the method exists, then the
// counts in the MDO should be scaled by 4X, so that
// they can be usefully and stably compared against the
// invocation counts in methods.
int ciMethod::scale_count(int count) {
  if (count > 0 && method_data() != NULL) {
    int current_mileage = method_data()->current_mileage();
    int creation_mileage = method_data()->creation_mileage();
    int counter_life = current_mileage - creation_mileage;
    int method_life = interpreter_invocation_count();
    if (0 < counter_life && counter_life < method_life) {
      return (int)((double)count * method_life / counter_life);
    }
  }
  return count;
}

// ------------------------------------------------------------------
// ciMethod::build_method_data
//
// Generate new methodDataOop objects at compile time.
void ciMethod::build_method_data(methodHandle h_m) {
#ifdef COMPILER2
  EXCEPTION_CONTEXT;
  if (is_native() || is_abstract() || h_m()->is_accessor()) return;
  if (h_m()->method_data() == NULL) {
    methodOopDesc::build_interpreter_method_data(h_m, THREAD);
    if (HAS_PENDING_EXCEPTION) {
      assert((PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())), "we expect only an OOM error here");
      CLEAR_PENDING_EXCEPTION;
    }
  }
  if (h_m()->method_data() != NULL) {
    _method_data = CURRENT_ENV->get_object(h_m()->method_data())->as_method_data();
  }
#else // COMPILER2
  ShouldNotReachHere();
#endif // COMPILER2
}

// public, retroactive version
void ciMethod::build_method_data() {
  assert(_method_data != NULL, "mdo already built");
  if (_method_data->is_empty()) {
    GUARDED_VM_ENTRY({
      build_method_data(get_methodOop());
    });
  }
}

// ------------------------------------------------------------------
// ciMethod::will_link
//
// Will this method link in a specific calling context?
bool ciMethod::will_link(ciKlass* accessing_klass,
                         ciKlass* declared_method_holder,
                         Bytecodes::Code bc) {
  if (!is_loaded()) {
    // Method lookup failed.
    return false;
  }

  // The link checks have been front-loaded into the get_method
  // call.  This method (ciMethod::will_link()) will be removed
  // in the future.

  return true;
}

// ------------------------------------------------------------------
// ciMethod::should_exclude
//
// Should this method be excluded from compilation?
bool ciMethod::should_exclude() {
  check_is_loaded();
  VM_ENTRY_MARK;
  {
    EXCEPTION_MARK;
    methodHandle mh(THREAD, get_methodOop());
    bool exclude = CompilerOracle::should_exclude(mh);
    return exclude;
  }
}

// ------------------------------------------------------------------
// ciMethod::should_inline
//
// Should this method be inlined during compilation?
bool ciMethod::should_inline() {
  check_is_loaded();
  VM_ENTRY_MARK;
  methodHandle mh(THREAD, get_methodOop());
  return CompilerOracle::should_inline(mh);
}

// ------------------------------------------------------------------
// ciMethod::should_print_assembly
//
// Should the compiler print the generated code for this method?
bool ciMethod::should_print_assembly() {
  check_is_loaded();
  VM_ENTRY_MARK;
  methodHandle mh(THREAD, get_methodOop());
  return CompilerOracle::should_print(mh);
}

// ------------------------------------------------------------------
// ciMethod::break_at_execute
//
// Should the compiler insert a breakpoint into the generated code
// method?
bool ciMethod::break_at_execute() {
  check_is_loaded();
  VM_ENTRY_MARK;
  methodHandle mh(THREAD, get_methodOop());
  return CompilerOracle::should_break_at(mh);
}

// ------------------------------------------------------------------
// ciMethod::can_be_compiled
//
// Have previous compilations of this method succeeded?
bool ciMethod::can_be_compiled() {
  check_is_loaded();
  return _is_compilable;
}

// ------------------------------------------------------------------
// ciMethod::set_not_compilable
//
// Tell the VM that this method cannot be compiled at all.
void ciMethod::set_not_compilable() {
  check_is_loaded();
  VM_ENTRY_MARK;
  _is_compilable = false;
  get_methodOop()->set_not_compilable();
}

// ------------------------------------------------------------------
// ciMethod::can_be_osr_compiled
//
// Have previous compilations of this method succeeded?
//
// Implementation note: the VM does not currently keep track
// of failed OSR compilations per bci.  The entry_bci parameter
// is currently unused.
bool ciMethod::can_be_osr_compiled(int entry_bci) {
  check_is_loaded();
  VM_ENTRY_MARK;
  return !get_methodOop()->access_flags().is_not_osr_compilable();
}

// ------------------------------------------------------------------
// ciMethod::has_compiled_code
bool ciMethod::has_compiled_code() {
  VM_ENTRY_MARK;
  return get_methodOop()->code() != NULL;
}

// ------------------------------------------------------------------
// ciMethod::instructions_size
int ciMethod::instructions_size() {
  GUARDED_VM_ENTRY(
    nmethod* code = get_methodOop()->code();
    if (code != NULL) {
      return code->instructions_size();
    }
    return 0;
  )
}

// ------------------------------------------------------------------
// ciMethod::is_not_reached
bool ciMethod::is_not_reached(int bci) {
  check_is_loaded();
  VM_ENTRY_MARK;
  return AbstractInterpreter::is_not_reached(
               methodHandle(THREAD, get_methodOop()), bci);
}

// ------------------------------------------------------------------
// ciMethod::was_never_executed
int ciMethod::was_executed_more_than(int times) {
  VM_ENTRY_MARK;
  return get_methodOop()->was_executed_more_than(times);
}

// ------------------------------------------------------------------
// ciMethod::has_unloaded_classes_in_signature
bool ciMethod::has_unloaded_classes_in_signature() {
  VM_ENTRY_MARK;
  {
    EXCEPTION_MARK;
    methodHandle m(THREAD, get_methodOop());
    bool has_unloaded = methodOopDesc::has_unloaded_classes_in_signature(m, (JavaThread *)THREAD);
    if( PENDING_EXCEPTION ) {
      CLEAR_PENDING_EXCEPTION;
      return true;     // Declare that we may have unloaded classes
    }
    return has_unloaded;
  }
}

// ------------------------------------------------------------------
// ciMethod::is_klass_loaded
bool ciMethod::is_klass_loaded(int refinfo_index, bool must_be_resolved) const {
  VM_ENTRY_MARK;
  return get_methodOop()->is_klass_loaded(refinfo_index, must_be_resolved);
}

// ------------------------------------------------------------------
// ciMethod::check_call
bool ciMethod::check_call(int refinfo_index, bool is_static) const {
  VM_ENTRY_MARK;
  {
    EXCEPTION_MARK;
    HandleMark hm(THREAD);
    constantPoolHandle pool (THREAD, get_methodOop()->constants());
    methodHandle spec_method;
    KlassHandle  spec_klass;
    LinkResolver::resolve_method(spec_method, spec_klass, pool, refinfo_index, THREAD);
    if (HAS_PENDING_EXCEPTION) {
      CLEAR_PENDING_EXCEPTION;
      return false;
    } else {
      return (spec_method->is_static() == is_static);    
    }
  }
  return false;
}

// ------------------------------------------------------------------
// ciMethod::print_codes
//
// Print the bytecodes for this method.
void ciMethod::print_codes() {
  check_is_loaded();
  GUARDED_VM_ENTRY(get_methodOop()->print_codes();)
}


#define FETCH_FLAG_FROM_VM(flag_accessor) { \
  check_is_loaded(); \
  VM_ENTRY_MARK; \
  return get_methodOop()->flag_accessor(); \
}

bool ciMethod::is_empty_method() const {         FETCH_FLAG_FROM_VM(is_empty_method); }
bool ciMethod::is_vanilla_constructor() const {  FETCH_FLAG_FROM_VM(is_vanilla_constructor); }
bool ciMethod::has_loops      () const {         FETCH_FLAG_FROM_VM(has_loops); }
bool ciMethod::has_jsrs       () const {         FETCH_FLAG_FROM_VM(has_jsrs);  }
bool ciMethod::is_accessor    () const {         FETCH_FLAG_FROM_VM(is_accessor); }
bool ciMethod::is_initializer () const {         FETCH_FLAG_FROM_VM(is_initializer); }

#undef FETCH_FLAG_FROM_VM


// ------------------------------------------------------------------
// ciMethod::print_codes
//
// Print a range of the bytecodes for this method.
void ciMethod::print_codes(int from, int to) {
  check_is_loaded();
  GUARDED_VM_ENTRY(get_methodOop()->print_codes(from, to);)
}

// ------------------------------------------------------------------
// ciMethod::print_name
//
// Print the name of this method, including signature and some flags.
void ciMethod::print_name() {
  check_is_loaded();
  GUARDED_VM_ENTRY(get_methodOop()->print_name(tty);)
}

// ------------------------------------------------------------------
// ciMethod::print_short_name
//
// Print the name of this method, without signature.
void ciMethod::print_short_name() {
  check_is_loaded();
  GUARDED_VM_ENTRY(get_methodOop()->print_short_name(tty);)
}

// ------------------------------------------------------------------
// ciMethod::print_impl
//
// Implementation of the print method.
void ciMethod::print_impl() {
  ciObject::print_impl();
  tty->print(" name=");
  name()->print_symbol();
  tty->print(" holder=");
  holder()->print_name();
  tty->print(" signature=");
  signature()->print_signature();
  if (is_loaded()) {
    tty->print(" loaded=true flags=");
    flags().print_member_flags();
  } else {
    tty->print(" loaded=false");
  }
}


