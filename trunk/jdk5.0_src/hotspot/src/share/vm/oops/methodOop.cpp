#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)methodOop.cpp	1.270 04/03/31 18:13:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_methodOop.cpp.incl"


// Implementation of methodOopDesc

char* methodOopDesc::name_and_sig_as_C_string() {
  return name_and_sig_as_C_string(Klass::cast(constants()->pool_holder()), name(), signature());
}

char* methodOopDesc::name_and_sig_as_C_string(char* buf, int size) {
  return name_and_sig_as_C_string(Klass::cast(constants()->pool_holder()), name(), signature(), buf, size);
}

char* methodOopDesc::name_and_sig_as_C_string(Klass* klass, symbolOop method_name, symbolOop signature) {
  const char* klass_name = klass->external_name();
  int klass_name_len  = (int)strlen(klass_name);
  int method_name_len = method_name->utf8_length();
  int len             = klass_name_len + 1 + method_name_len + signature->utf8_length();
  char* dest          = NEW_RESOURCE_ARRAY(char, len + 1);
  strcpy(dest, klass_name);
  dest[klass_name_len] = '.';
  strcpy(&dest[klass_name_len + 1], method_name->as_C_string());
  strcpy(&dest[klass_name_len + 1 + method_name_len], signature->as_C_string());
  dest[len] = 0;
  return dest;
}

char* methodOopDesc::name_and_sig_as_C_string(Klass* klass, symbolOop method_name, symbolOop signature, char* buf, int size) {
  symbolOop klass_name = klass->name();
  klass_name->as_klass_external_name(buf, size);
  int len = (int)strlen(buf);

  if (len < size - 1) {
    buf[len++] = '.';

    method_name->as_C_string(&(buf[len]), size - len);
    len = (int)strlen(buf);

    signature->as_C_string(&(buf[len]), size - len);
  }

  return buf;
}

int  methodOopDesc::fast_exception_handler_bci_for(KlassHandle ex_klass, int throw_bci, TRAPS) {
  // exception table holds quadruple entries of the form (beg_bci, end_bci, handler_bci, klass_index)
  const int beg_bci_offset     = 0;
  const int end_bci_offset     = 1;
  const int handler_bci_offset = 2;
  const int klass_index_offset = 3;
  const int entry_size         = 4;
  // access exception table
  typeArrayHandle table (THREAD, constMethod()->exception_table());
  int length = table->length();
  assert(length % entry_size == 0, "exception table format has changed");
  // iterate through all entries sequentially
  constantPoolHandle pool(THREAD, constants());
  for (int i = 0; i < length; i += entry_size) {
    int beg_bci = table->int_at(i + beg_bci_offset);
    int end_bci = table->int_at(i + end_bci_offset);
    assert(beg_bci <= end_bci, "inconsistent exception table");
    if (beg_bci <= throw_bci && throw_bci < end_bci) {
      // exception handler bci range covers throw_bci => investigate further
      int handler_bci = table->int_at(i + handler_bci_offset);
      int klass_index = table->int_at(i + klass_index_offset);
      if (klass_index == 0) {
        return handler_bci;
      } else if (ex_klass == NULL) {
        return handler_bci;
      } else {
        // we know the exception class => get the constraint class
        // this may require loading of the constraint class; if verification
        // fails or some other exception occurs, return handler_bci
	klassOop k = pool->klass_at(klass_index, CHECK_(handler_bci));
	KlassHandle klass = KlassHandle(THREAD, k);
        assert(klass.not_null(), "klass not loaded");
        if (ex_klass->is_subtype_of(klass())) {
          return handler_bci;
        }
      }
    }
  }

  return -1;
}

methodOop methodOopDesc::method_from_bcp(address bcp) {
  debug_only(static int count = 0; count++);
  assert(Universe::heap()->is_in_permanent(bcp), "bcp not in perm_gen");
  // TO DO: this may be unsafe in some configurations
  HeapWord* p = Universe::heap()->block_start(bcp);
  assert(Universe::heap()->block_is_obj(p), "must be obj");
  assert(oop(p)->is_constMethod(), "not a method");
  return constMethodOop(p)->method();
}


void methodOopDesc::mask_for(int bci, InterpreterOopMap* mask) {
    
  Thread* myThread    = Thread::current();
  methodHandle h_this(myThread, this);
#ifdef ASSERT
  bool has_capability = myThread->is_VM_thread() ||
                        myThread->is_ConcurrentMarkSweep_thread() ||
                        myThread->is_GC_task_thread();

  if (!has_capability) {
    if (!VerifyStack && !VerifyLastFrame) {
      // verify stack calls this outside VM thread
      warning("oopmap should only be accessed by the "
              "VM, GC task or CMS threads (or during debugging)");  
      InterpreterOopMap local_mask;
      instanceKlass::cast(method_holder())->mask_for(h_this, bci, &local_mask);
      local_mask.print();
    }
  }
#endif
  instanceKlass::cast(method_holder())->mask_for(h_this, bci, mask);
  return;
}


int methodOopDesc::bci_from(address bcp) const {  
  assert(is_native() && bcp == code_base() || contains(bcp), "bcp doesn't belong to this method");
  return bcp - code_base();
}


address methodOopDesc::bcp_from(int bci) const {
  assert((is_native() && bci == 0)  || (!is_native() && 0 <= bci && bci < code_size()), "illegal bci");
  address bcp = code_base() + bci;
  assert(is_native() && bcp == code_base() || contains(bcp), "bcp doesn't belong to this method");
  return bcp;
}


int methodOopDesc::object_size(bool is_native) {
  // If native, then include pointers for native_function and signature_handler
  int extra_bytes = (is_native) ? 2*sizeof(address*) : 0;
  int extra_words = align_size_up(extra_bytes, BytesPerWord) / BytesPerWord;
  return align_object_size(header_size() + extra_words);
}


symbolOop methodOopDesc::klass_name() const {
  klassOop k = method_holder();
  assert(k->is_klass(), "must be klass");
  instanceKlass* ik = (instanceKlass*) k->klass_part();
  return ik->name();
}


void methodOopDesc::set_interpreter_kind() {
  int kind = AbstractInterpreter::method_kind(this);
  assert(kind != AbstractInterpreter::invalid,
         "interpreter entry must be valid");
  set_interpreter_kind(kind);
}


// Attempt to return method oop to original state.  Clear any pointers
// (to objects outside the shared spaces).  We won't be able to predict
// where they should point in a new JVM.  Further initialize some
// entries now in order allow them to be write protected later.

void methodOopDesc::remove_unshareable_info() {
  _code = NULL;
#ifndef CORE
  NOT_PRODUCT(set_compiled_invocation_count(0);)
  set_from_compiled_code_entry_point(NULL);
#endif
  if (is_native()) {
    *native_function_addr() = NULL;
    set_signature_handler(NULL);
  }
  set_interpreter_kind();
  set_interpreter_entry(NULL);
#ifdef COMPILER2
  assert(_method_data == NULL, "unexpected method data?");
  set_method_data(NULL);
  set_interpreter_throwout_count(0);
  set_interpreter_invocation_count(0);
  assert(_tier1_compile_done == 0, "unexpected compile?");
  _tier1_compile_done = 0;
#endif
}


#ifndef CORE

bool methodOopDesc::was_executed_more_than(int n) const {
  // Invocation counter is reset when the methodOop is compiled.
  // If the method has compiled code we therefore assume it has
  // be excuted more than n times.
  if (is_accessor() || is_empty_method() || (code() != NULL)) {
    // interpreter doesn't bump invocation counter of trivial methods
    // compiler does not bump invocation counter of compiled methods
    return true;
  } else if (_invocation_counter.carry()) {
    // The carry bit is set when the counter overflows and causes
    // a compilation to occur.  We don't know how many times
    // the counter has been reset, so we simply assume it has
    // been executed more than n times.
    return true;
  } else {
    return invocation_count() > n;
  }
}

#ifndef PRODUCT
void methodOopDesc::print_invocation_count() const {
  tty->print ("%8d ", invocation_count());
  tty->print ("%8d ", compiled_invocation_count());
  if (is_static()) tty->print("static ");
  if (is_final()) tty->print("final ");
  if (is_synchronized()) tty->print("synchronized ");
  if (is_native()) tty->print("native ");
  name()->print_value();
  tty->print(" ");
  tty->fill_to(48);
  method_holder()->print_value();
  if (WizardMode) {
    // dump the size of the byte codes
    tty->print(" {%d}", code_size());
  }
  tty->cr();
}
#endif

// Build a methodDataOop object to hold information about this method
// collected in the interpreter.
void methodOopDesc::build_interpreter_method_data(methodHandle method, TRAPS) {
#ifndef COMPILER2
  ShouldNotReachHere();
#else // COMPILER2
  // Grab a lock here to prevent multiple
  // methodDataOops from being created.
  MutexLocker ml(MethodData_lock, THREAD);
  if (method->method_data() == NULL) {
    methodDataOop method_data = oopFactory::new_methodData(method, CHECK);
    method->set_method_data(method_data);
    if (PrintMethodData && (Verbose || WizardMode)) {
      ResourceMark rm(THREAD);
      tty->print("build_interpreter_method_data for ");
      method->print_name(tty);
      tty->cr();
      // At the end of the run, the MDO, full of data, will be dumped.
    }
  }
#endif // !COMPILER2
}
#endif //!CORE


void methodOopDesc::cleanup_inline_caches() {
  // The current system doesn't use inline caches in the interpreter
  // => nothing to do (keep this method around for future use)
}


void methodOopDesc::compute_size_of_parameters(Thread *thread) {
  symbolHandle h_signature(thread, signature());
  ArgumentSizeComputer asc(h_signature);
  set_size_of_parameters(asc.size() + (is_static() ? 0 : 1));
}

typeArrayOop methodOopDesc::annotations() const {
  instanceKlass* ik = instanceKlass::cast(method_holder());
  objArrayOop md = ik->methods_annotations();
  if (md == NULL)
    return NULL;
  return typeArrayOop(md->obj_at(method_index()));
}

typeArrayOop methodOopDesc::parameter_annotations() const {
  instanceKlass* ik = instanceKlass::cast(method_holder());
  objArrayOop md = ik->methods_parameter_annotations();
  if (md == NULL)
    return NULL;
  return typeArrayOop(md->obj_at(method_index()));
}

typeArrayOop methodOopDesc::annotation_default() const {
  instanceKlass* ik = instanceKlass::cast(method_holder());
  objArrayOop md = ik->methods_default_annotations();
  if (md == NULL)
    return NULL;
  return typeArrayOop(md->obj_at(method_index()));
}

#ifdef CC_INTERP
void methodOopDesc::set_result_index(BasicType type)          { 
  _result_index = AbstractInterpreter::BasicType_as_index(type); 
}
#endif

BasicType methodOopDesc::result_type() const {
  ResultTypeFinder rtf(signature());
  return rtf.type();
}


bool methodOopDesc::is_empty_method() const {
  return  code_size() == 1
      && *code_base() == Bytecodes::_return;
}


bool methodOopDesc::is_vanilla_constructor() const {
  // Returns true if this method is a vanilla constructor, i.e. an "<init>" "()V" method
  // which only calls the superclass vanilla constructor and possibly does stores of
  // zero constants to local fields:
  //
  //   aload_0
  //   invokespecial
  //   indexbyte1
  //   indexbyte2
  // 
  // followed by an (optional) sequence of:
  //
  //   aload_0
  //   aconst_null / iconst_0 / fconst_0 / dconst_0
  //   putfield
  //   indexbyte1
  //   indexbyte2
  //
  // followed by:
  //
  //   return

  assert(name() == vmSymbols::object_initializer_name(),    "Should only be called for default constructors");
  assert(signature() == vmSymbols::void_method_signature(), "Should only be called for default constructors");
  int size = code_size();
  // Check if size match
  if (size == 0 || size % 5 != 0) return false;
  address cb = code_base();
  int last = size - 1;
  if (cb[0] != Bytecodes::_aload_0 || cb[1] != Bytecodes::_invokespecial || cb[last] != Bytecodes::_return) {
    // Does not call superclass default constructor
    return false;
  }
  // Check optional sequence
  for (int i = 4; i < last; i += 5) {
    if (cb[i] != Bytecodes::_aload_0) return false;
    if (!Bytecodes::is_zero_const(Bytecodes::cast(cb[i+1]))) return false;
    if (cb[i+2] != Bytecodes::_putfield) return false;
  }
  return true;
}


bool methodOopDesc::compute_has_loops_flag() {  
  BytecodeStream bcs(this);
  Bytecodes::Code bc;
    
  while ((bc = bcs.next()) >= 0) {    
    switch( bc ) {        
      case Bytecodes::_ifeq: 
      case Bytecodes::_ifnull: 
      case Bytecodes::_iflt: 
      case Bytecodes::_ifle: 
      case Bytecodes::_ifne: 
      case Bytecodes::_ifnonnull: 
      case Bytecodes::_ifgt: 
      case Bytecodes::_ifge: 
      case Bytecodes::_if_icmpeq: 
      case Bytecodes::_if_icmpne: 
      case Bytecodes::_if_icmplt: 
      case Bytecodes::_if_icmpgt: 
      case Bytecodes::_if_icmple: 
      case Bytecodes::_if_icmpge: 
      case Bytecodes::_if_acmpeq:
      case Bytecodes::_if_acmpne:
      case Bytecodes::_goto: 
      case Bytecodes::_jsr:     
        if( bcs.dest() < bcs.next_bci() ) _access_flags.set_has_loops();
        break;

      case Bytecodes::_goto_w:       
      case Bytecodes::_jsr_w:        
        if( bcs.dest_w() < bcs.next_bci() ) _access_flags.set_has_loops(); 
        break;
    }  
  }
  _access_flags.set_loops_flag_init();
  return _access_flags.has_loops(); 
}


bool methodOopDesc::is_final_method() const {
  return is_final() || Klass::cast(method_holder())->is_final();
}


bool methodOopDesc::is_strict_method() const {
  return is_strict();
}


bool methodOopDesc::is_accessor() const {
  if (code_size() != 5) return false;
  if (size_of_parameters() != 1) return false;
  if (Bytecodes::java_code(Bytecodes::cast(code_base()[0])) != Bytecodes::_aload_0 ) return false;
  if (Bytecodes::java_code(Bytecodes::cast(code_base()[1])) != Bytecodes::_getfield) return false;
  Bytecodes::Code ret_bc = Bytecodes::java_code(Bytecodes::cast(code_base()[4]));
  if (Bytecodes::java_code(Bytecodes::cast(code_base()[4])) != Bytecodes::_areturn &&
      Bytecodes::java_code(Bytecodes::cast(code_base()[4])) != Bytecodes::_ireturn ) return false;
  return true;
}


bool methodOopDesc::is_initializer() const {
  return name() == vmSymbols::object_initializer_name() || name() == vmSymbols::class_initializer_name();
}


objArrayHandle methodOopDesc::resolved_checked_exceptions_impl(methodOop this_oop, TRAPS) {
  int length = this_oop->checked_exceptions_length();
  if (length == 0) {  // common case
    return objArrayHandle(THREAD, Universe::the_empty_class_klass_array());
  } else {
    methodHandle h_this(THREAD, this_oop);
    objArrayOop m_oop = oopFactory::new_objArray(SystemDictionary::class_klass(), length, CHECK_(objArrayHandle()));
    objArrayHandle mirrors (THREAD, m_oop);
    for (int i = 0; i < length; i++) {
      CheckedExceptionElement* table = h_this->checked_exceptions_start(); // recompute on each iteration, not gc safe
      klassOop k = h_this->constants()->klass_at(table[i].class_cp_index, CHECK_(objArrayHandle()));
      assert(Klass::cast(k)->is_subclass_of(SystemDictionary::throwable_klass()), "invalid exception class");
      mirrors->obj_at_put(i, Klass::cast(k)->java_mirror());
    }
    return mirrors;
  }
};


int methodOopDesc::line_number_from_bci(int bci) const {
  NOT_CORE(if (bci == SynchronizationEntryBCI) bci = 0;)
  assert(bci == 0 || 0 <= bci && bci < code_size(), "illegal bci");
  int best_bci  =  0;
  int best_line = -1;

  if (has_linenumber_table()) {
    // The line numbers are a short array of 2-tuples [start_pc, line_number].
    // Not necessarily sorted and not necessarily one-to-one.
    CompressedLineNumberReadStream stream(compressed_linenumber_table());
    while (stream.read_pair()) {
      if (stream.bci() == bci) {
        // perfect match
        return stream.line();
      } else {
        // update best_bci/line
        if (stream.bci() < bci && stream.bci() >= best_bci) {
          best_bci  = stream.bci();
          best_line = stream.line();
        }
      }
    }
  }
  return best_line;
}


bool methodOopDesc::is_klass_loaded_by_klass_index(int klass_index) const {
  if( _constants->tag_at(klass_index).is_unresolved_klass() ) {
    Thread *thread = Thread::current();
    symbolHandle klass_name(thread, _constants->klass_name_at(klass_index));
    Handle loader(thread, instanceKlass::cast(method_holder())->class_loader());
    Handle prot  (thread, Klass::cast(method_holder())->protection_domain());
    return SystemDictionary::find(klass_name, loader, prot, thread) != NULL;
  } else {
    return true;
  }
}


bool methodOopDesc::is_klass_loaded(int refinfo_index, bool must_be_resolved) const {
  int klass_index = _constants->klass_ref_index_at(refinfo_index);
  if (must_be_resolved) {
    // Make sure klass is resolved in constantpool.   
    if (constants()->tag_at(klass_index).is_unresolved_klass()) return false;
  }  
  return is_klass_loaded_by_klass_index(klass_index);
}


void methodOopDesc::set_native_function(address function) {
  assert(function != NULL, "use clear_native_function to unregister natives");
  address* native_function = native_function_addr();
  if (JvmtiExport::should_post_native_method_bind() &&
      function != NULL && function != SharedRuntime::native_method_throw_unsatisfied_link_error_entry()) {
    // post the bind event, and possible change the bind function
    JvmtiExport::post_native_method_bind(this, &function);
  }
  *native_function = function;
#ifndef CORE
  // This function can be called more than once. We must make sure that we always
  // use the latest registered method -> check if a stub already has been generated.
  // If so, we have to make it not_entrant.
  nmethod* nm = code(); // Put it into local variable to guard against concurrent updates
  if (nm != NULL) {
    nm->make_not_entrant();
  }  
#endif
}


bool methodOopDesc::has_native_function() const {
  address func = native_function();
  return (func != NULL && func != SharedRuntime::native_method_throw_unsatisfied_link_error_entry());
}


void methodOopDesc::clear_native_function() {
  set_native_function(SharedRuntime::native_method_throw_unsatisfied_link_error_entry());
}


void methodOopDesc::set_signature_handler(address handler) { 
  address* signature_handler =  signature_handler_addr();
  *signature_handler = handler;
}


bool methodOopDesc::is_not_compilable(int comp_level) const {
#ifndef CORE
#ifdef COMPILER2  // update MDO only in C2 system
  methodDataOop mdo = method_data();
  if (mdo != NULL
      && (uint)mdo->decompile_count() > (uint)PerMethodRecompilationCutoff) {
    // Since (uint)-1 is large, -1 really means 'no cutoff'.
    return true;
  }
  if (is_tier1_compile(comp_level)) {
    if (is_not_tier1_compilable()) {
      return true;
    }
  }
#endif //COMPILER2
  return (_invocation_counter.state() == InvocationCounter::wait_for_nothing)
          || (number_of_breakpoints() > 0);
#else //CORE
  return (number_of_breakpoints() > 0);
#endif //CORE
}

// call this when compiler finds that this method is not compilable
void methodOopDesc::set_not_compilable(int comp_level) {
#ifndef CORE
  if ((TraceDeoptimization || LogCompilation) && (xtty != NULL)) {
    ttyLocker ttyl;
    xtty->begin_elem("make_not_compilable thread='%d'", (int) os::current_thread_id());
    xtty->method(this);
    xtty->stamp();
    xtty->end_elem();
  }
#ifdef COMPILER2
  if (is_tier1_compile(comp_level)) {
    set_not_tier1_compilable();
    return;
  }
#endif
  assert(comp_level == CompLevel_highest_tier, "unexpected compilation level");
  invocation_counter()->set_state(InvocationCounter::wait_for_nothing);
  backedge_counter()->set_state(InvocationCounter::wait_for_nothing);
#endif
}

// This is called before the access flags have been set, i.e.,during
// method allocation The _from_compiled_code_entry_point is updated when
// a class is linked.
void methodOopDesc::init_code() {
  _code = NULL;  
  NOT_CORE(_from_compiled_code_entry_point = NULL;)
}

// Called when the method_holder is getting linked. Setup entrypoints so the method
// is ready to be called from interpreter, compiler, and vtables.
void methodOopDesc::link_method(methodHandle h_method) {
  NOT_CORE(assert(h_method->_from_compiled_code_entry_point == NULL, "should only be called once");)
  assert(h_method->_interpreter_entry              == NULL, "should only be called once");

  // Setup interpreter entrypoint
  address entry = AbstractInterpreter::entry_for_method(h_method);
  assert(entry != NULL, "interpreter entry must be non-null");
  h_method->set_interpreter_entry(entry);

  // Setup compiler entrypoint. Default entry is lazy, so we do
  // not eagerly create I2C adapters.
  h_method->update_compiled_code_entry_point(true);
}

// The verified_code_entry() must be called when a invoke is resolved
// on this method.

// It returns the compiled code entry point, after asserting not null.
// This function is called after potential safepoints so that nmethod
// or adapter that it points to is still live and valid.
// This function must not hit a safepoint!
#ifndef CORE
address methodOopDesc::verified_code_entry() {
  debug_only(No_Safepoint_Verifier nsv;)
  assert(_from_compiled_code_entry_point != NULL, "must be set");
  return _from_compiled_code_entry_point;
}
#endif

#ifndef CORE
// Check that if an nmethod ref exists, it has a backlink to this.
// Not inline to avoid circular ref.
bool methodOopDesc::check_code() const {
  // cached in a register or local.  There's a race on the value of the field.
  nmethod *code = (nmethod *)OrderAccess::load_ptr_acquire(&_code);
  return code == NULL || code->method() == this; 
}
#endif

// Install compiled code for this nmethod
void methodOopDesc::set_code(nmethod* code) {
#ifdef COMPILER2
  NOT_CORE(assert(code == NULL || !code->is_osr_method(), "osr code should not be used here");)
#endif
  _code = code;
  // Update compiler entrypoint
  update_compiled_code_entry_point(true);
}

void methodOopDesc::update_compiled_code_entry_point(bool lazy) {
#ifndef CORE
  // Cache code in a register.  There is a race between uncommon trap code
  // setting the code to NULL and the compiler setting the code to a newly
  // compiled nmethod (bugid 4947125).  The race is benign if the fields do
  // not match.  If the code points to an nmethod but the compiled code entry
  // points to an c2i adapter, it will call the compiled code through the
  // interpreter, which is slower but correct (and unlikely).
  // If the code points to NULL and the compiled code entry points to a
  // verified entry point, the vep() has already been patched to call
  // handle_wrong_method and come back here.
  // If the nmethod is flushed before getting back here, the make_zombie() code
  //  will clean up the vep from the compiled code entry field.
  nmethod *code = (nmethod *)OrderAccess::load_ptr_acquire(&_code);
  if (code != NULL) {   // cached in a register or local
    _from_compiled_code_entry_point = code->verified_entry_point();
    return;
  }

  if (is_abstract()) {
    COMPILER1_ONLY(_from_compiled_code_entry_point = StubRoutines::throw_AbstractMethodError_entry();)
    COMPILER2_ONLY(_from_compiled_code_entry_point = OptoRuntime::handle_abstract_method_stub();)
    return;
  }  

#ifdef COMPILER1       
  // There is no lazy entrypoints for Compiler1 since it does not
  // use adapter frames.
  methodHandle m(methodOop(this));
  _from_compiled_code_entry_point = is_static()
    ? Runtime1::ientries_for(m)->static_call_entry()
    : Runtime1::ientries_for(m)->optimized_call_entry();    
#endif

#ifdef COMPILER2
  methodHandle m(methodOop(this));  
  // This temporary assignment to "entry" is necessary to guarantee that
  // lazy_std/std_verified_entry() is called before dereferencing m
  // through methodHandle::operator->(). See 4801752 for details.
  address entry = lazy ? C2IAdapterGenerator::lazy_std_verified_entry(m)
                       : C2IAdapterGenerator::std_verified_entry(m);
  m->_from_compiled_code_entry_point = entry;
#endif
#endif
}


bool methodOopDesc::is_overridden_in(klassOop k) const {
  instanceKlass* ik = instanceKlass::cast(k);

  if (ik->is_interface()) return false;

  // If method is an interface, we skip it - except if it
  // is a miranda method
  if (instanceKlass::cast(method_holder())->is_interface()) {
    // Check that method is not a miranda method
    if (ik->lookup_method(name(), signature()) == NULL) {
      // No implementation exist - so miranda method
      return false;
    }
    return true;
  }  

  assert(ik->is_subclass_of(method_holder()), "should be subklass");
  assert(ik->vtable() != NULL, "vtable should exist");
  if (_vtable_index == -1) {
    return false;
  } else {
    methodOop vt_m = ik->method_at_vtable(_vtable_index);
    return vt_m != this;
  }
}


methodHandle methodOopDesc:: clone_with_new_data(methodHandle m, u_char* new_code, int new_code_length, 
                                                u_char* new_compressed_linenumber_table, int new_compressed_linenumber_size, TRAPS) {
  // Code below does not work for native methods - they should never get rewritten anyway
  assert(!m->is_native(), "cannot rewrite native methods");
  // Allocate new methodOop
  AccessFlags flags = m->access_flags();
  int checked_exceptions_len = m->checked_exceptions_length();
  int localvariable_len = m->localvariable_table_length();
  methodOop newm_oop = oopFactory::new_method(new_code_length, flags, new_compressed_linenumber_size, localvariable_len, checked_exceptions_len, CHECK_(methodHandle()));
  methodHandle newm (THREAD, newm_oop);
  int new_method_size = newm->method_size();
  // Create a shallow copy of methodOopDesc part, but be careful to preserve the new constMethodOop
  constMethodOop newcm = newm->constMethod();
  int new_const_method_size = newm->constMethod()->object_size();
  memcpy(newm(), m(), sizeof(methodOopDesc));
  // Create shallow copy of constMethodOopDesc, but be careful to preserve the methodOop
  memcpy(newcm, m->constMethod(), sizeof(constMethodOopDesc));
  // Reset correct method/const method, method size, and parameter info
  newcm->set_method(newm());
  newm->set_constMethod(newcm);
  assert(newcm->method() == newm(), "check");
  newm->constMethod()->set_code_size(new_code_length);
  newm->constMethod()->set_constMethod_size(new_const_method_size);
  newm->set_method_size(new_method_size);
  newm->set_parameter_info(0);
  assert(newm->code_size() == new_code_length, "check");
  assert(newm->checked_exceptions_length() == checked_exceptions_len, "check");
  assert(newm->localvariable_table_length() == localvariable_len, "check");
  // Copy new byte codes
  memcpy(newm->code_base(), new_code, new_code_length);
  // Copy line number table
  if (new_compressed_linenumber_size > 0) {
    memcpy(newm->compressed_linenumber_table(),
           new_compressed_linenumber_table,
           new_compressed_linenumber_size);
  }
  // Copy checked_exceptions
  if (checked_exceptions_len > 0) {
    memcpy(newm->checked_exceptions_start(),
           m->checked_exceptions_start(),
           checked_exceptions_len * sizeof(CheckedExceptionElement));
  }
  // Copy local variable number table
  if (localvariable_len > 0) {
    memcpy(newm->localvariable_table_start(),
           m->localvariable_table_start(),
           localvariable_len * sizeof(LocalVariableTableElement));
  }
  return newm;
}

// Lookup the jmethodID for this method.  Return NULL if not found.
// NOTE that this function can be called from a signal handler
// (see AsyncGetCallTrace support for Forte Analyzer) and this
// needs to be async-safe. No allocation should be done and
// so handles are not used to avoid deadlock.
jmethodID methodOopDesc::find_jmethod_id_or_null() {
  return jniIdSupport::to_jmethod_id_or_null(this);
}

jmethodID methodOopDesc::jmethod_id() {
  return jniIdSupport::to_jmethod_id(this);
}

// This function must be factored out from methodOopDesc::intrinsic_id,
// or else some compilers will take the resulting huge combined function
// and miscompile it.  (The solx86 fastdebug build was compiling this
// with a huge stack frame, which was defeating overflow detection!)
methodOopDesc::IntrinsicId unsafe_intrinsic_id(symbolOop name, symbolOop signature) {
  if        (name == vmSymbols::sun_misc_Unsafe_getObject_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getObject_obj32_signature ()) return methodOopDesc::_getObject_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getObject_obj_signature   ()) return methodOopDesc::_getObject_obj;
    // no 'raw' version of getObject

  } else if (name == vmSymbols::sun_misc_Unsafe_getBoolean_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getBoolean_obj32_signature()) return methodOopDesc::_getBoolean_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getBoolean_obj_signature  ()) return methodOopDesc::_getBoolean_obj;
    // no 'raw' version of getBoolean

  } else if (name == vmSymbols::sun_misc_Unsafe_getByte_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getByte_obj32_signature   ()) return methodOopDesc::_getByte_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getByte_obj_signature     ()) return methodOopDesc::_getByte_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getByte_raw_signature     ()) return methodOopDesc::_getByte_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getShort_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getShort_obj32_signature  ()) return methodOopDesc::_getShort_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getShort_obj_signature    ()) return methodOopDesc::_getShort_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getShort_raw_signature    ()) return methodOopDesc::_getShort_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getChar_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getChar_obj32_signature   ()) return methodOopDesc::_getChar_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getChar_obj_signature     ()) return methodOopDesc::_getChar_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getChar_raw_signature     ()) return methodOopDesc::_getChar_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getInt_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getInt_obj32_signature    ()) return methodOopDesc::_getInt_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getInt_obj_signature      ()) return methodOopDesc::_getInt_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getInt_raw_signature      ()) return methodOopDesc::_getInt_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getLong_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getLong_obj32_signature   ()) return methodOopDesc::_getLong_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getLong_obj_signature     ()) return methodOopDesc::_getLong_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getLong_raw_signature     ()) return methodOopDesc::_getLong_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getFloat_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getFloat_obj32_signature  ()) return methodOopDesc::_getFloat_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getFloat_obj_signature    ()) return methodOopDesc::_getFloat_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getFloat_raw_signature    ()) return methodOopDesc::_getFloat_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getDouble_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getDouble_obj32_signature ()) return methodOopDesc::_getDouble_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_getDouble_obj_signature   ()) return methodOopDesc::_getDouble_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_getDouble_raw_signature   ()) return methodOopDesc::_getDouble_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getAddress_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getAddress_raw_signature  ()) return methodOopDesc::_getAddress_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putObject_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putObject_obj32_signature ()) return methodOopDesc::_putObject_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putObject_obj_signature   ()) return methodOopDesc::_putObject_obj;
    // no 'raw' version of putObject

  } else if (name == vmSymbols::sun_misc_Unsafe_putBoolean_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putBoolean_obj32_signature()) return methodOopDesc::_putBoolean_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putBoolean_obj_signature  ()) return methodOopDesc::_putBoolean_obj;
    // no 'raw' version of putBoolean

  } else if (name == vmSymbols::sun_misc_Unsafe_putByte_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putByte_obj32_signature   ()) return methodOopDesc::_putByte_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putByte_obj_signature     ()) return methodOopDesc::_putByte_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putByte_raw_signature     ()) return methodOopDesc::_putByte_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putShort_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putShort_obj32_signature  ()) return methodOopDesc::_putShort_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putShort_obj_signature    ()) return methodOopDesc::_putShort_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putShort_raw_signature    ()) return methodOopDesc::_putShort_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putChar_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putChar_obj32_signature   ()) return methodOopDesc::_putChar_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putChar_obj_signature     ()) return methodOopDesc::_putChar_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putChar_raw_signature     ()) return methodOopDesc::_putChar_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putInt_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putInt_obj32_signature    ()) return methodOopDesc::_putInt_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putInt_obj_signature      ()) return methodOopDesc::_putInt_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putInt_raw_signature      ()) return methodOopDesc::_putInt_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putLong_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putLong_obj32_signature   ()) return methodOopDesc::_putLong_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putLong_obj_signature     ()) return methodOopDesc::_putLong_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putLong_raw_signature     ()) return methodOopDesc::_putLong_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putFloat_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putFloat_obj32_signature  ()) return methodOopDesc::_putFloat_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putFloat_obj_signature    ()) return methodOopDesc::_putFloat_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putFloat_raw_signature    ()) return methodOopDesc::_putFloat_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putDouble_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putDouble_obj32_signature ()) return methodOopDesc::_putDouble_obj32;
    if (signature == vmSymbols::sun_misc_Unsafe_putDouble_obj_signature   ()) return methodOopDesc::_putDouble_obj;
    if (signature == vmSymbols::sun_misc_Unsafe_putDouble_raw_signature   ()) return methodOopDesc::_putDouble_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_putAddress_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putAddress_raw_signature  ()) return methodOopDesc::_putAddress_raw;

  } else if (name == vmSymbols::sun_misc_Unsafe_getObjectVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getObjectVolatile_obj_signature   ()) return methodOopDesc::_getObjectVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getBooleanVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getBooleanVolatile_obj_signature  ()) return methodOopDesc::_getBooleanVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getByteVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getByteVolatile_obj_signature     ()) return methodOopDesc::_getByteVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getShortVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getShortVolatile_obj_signature    ()) return methodOopDesc::_getShortVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getCharVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getCharVolatile_obj_signature     ()) return methodOopDesc::_getCharVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getIntVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getIntVolatile_obj_signature      ()) return methodOopDesc::_getIntVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getLongVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getLongVolatile_obj_signature     ()) return methodOopDesc::_getLongVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getFloatVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getFloatVolatile_obj_signature    ()) return methodOopDesc::_getFloatVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_getDoubleVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_getDoubleVolatile_obj_signature   ()) return methodOopDesc::_getDoubleVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putObjectVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putObjectVolatile_obj_signature   ()) return methodOopDesc::_putObjectVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putBooleanVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putBooleanVolatile_obj_signature  ()) return methodOopDesc::_putBooleanVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putByteVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putByteVolatile_obj_signature     ()) return methodOopDesc::_putByteVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putShortVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putShortVolatile_obj_signature    ()) return methodOopDesc::_putShortVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putCharVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putCharVolatile_obj_signature     ()) return methodOopDesc::_putCharVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putIntVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putIntVolatile_obj_signature      ()) return methodOopDesc::_putIntVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putLongVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putLongVolatile_obj_signature     ()) return methodOopDesc::_putLongVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putFloatVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putFloatVolatile_obj_signature    ()) return methodOopDesc::_putFloatVolatile_obj;
  } else if (name == vmSymbols::sun_misc_Unsafe_putDoubleVolatile_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_putDoubleVolatile_obj_signature   ()) return methodOopDesc::_putDoubleVolatile_obj;

  } else if (name == vmSymbols::sun_misc_Unsafe_allocateInstance_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_allocateInstance_signature()) return methodOopDesc::_allocateInstance;

  } else if (name == vmSymbols::sun_misc_Unsafe_compareAndSwapObject_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_compareAndSwapObject_obj_signature()) return methodOopDesc::_compareAndSwapObject_obj;

  } else if (name == vmSymbols::sun_misc_Unsafe_compareAndSwapInt_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_compareAndSwapInt_obj_signature()) return methodOopDesc::_compareAndSwapInt_obj;

  } else if (name == vmSymbols::sun_misc_Unsafe_compareAndSwapLong_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_compareAndSwapLong_obj_signature()) return methodOopDesc::_compareAndSwapLong_obj;

  } else if (name == vmSymbols::sun_misc_Unsafe_park_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_park_signature()) return methodOopDesc::_park;
  } else if (name == vmSymbols::sun_misc_Unsafe_unpark_name()) {
    if (signature == vmSymbols::sun_misc_Unsafe_unpark_signature()) return methodOopDesc::_unpark;
  }


  return methodOopDesc::_none;
}


methodOopDesc::IntrinsicId methodOopDesc::intrinsic_id() const {
  assert(vmIntrinsics::_none == 0, "correct coding of default case");
  // if loader is not the default loader (i.e., != NULL), we can't know the intrinsics
  // because we are not loading from core libraries
  if (instanceKlass::cast(method_holder())->class_loader() != NULL) return _none;
  // return intrinsic id if any
  symbolOop klass_name = instanceKlass::cast(method_holder())->name();
  if (klass_name == vmSymbols::java_lang_Object() && !is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_lang_Object_hashCode_name()) {
      if (signature() == vmSymbols::java_lang_Object_hashCode_signature()) return _hash;
    
    } else if (name() == vmSymbols::java_lang_Object_getClass_name()) {
      if (signature() == vmSymbols::java_lang_Object_getClass_signature()) return _getClass;
    }

  } else if (klass_name == vmSymbols::java_lang_Math() && is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_lang_Math_sin_name()) {
      if (signature() == vmSymbols::java_lang_Math_sin_signature()) return _dsin;

    } else if (name() == vmSymbols::java_lang_Math_cos_name()) {
      if (signature() == vmSymbols::java_lang_Math_cos_signature()) return _dcos;

    } else if (name() == vmSymbols::java_lang_Math_tan_name()) {
      if (signature() == vmSymbols::java_lang_Math_tan_signature()) return _dtan;

    } else if (name() == vmSymbols::java_lang_Math_atan2_name()) {
      if (signature() == vmSymbols::java_lang_Math_atan2_signature()) return _datan2;

    } else if (name() == vmSymbols::java_lang_Math_sqrt_name()) {
      if (signature() == vmSymbols::java_lang_Math_sqrt_signature()) return _dsqrt;

    } else if (name() == vmSymbols::java_lang_Math_pow_name()) {
      if (signature() == vmSymbols::java_lang_Math_pow_signature()) return _dpow;

    }

  } else if (klass_name == vmSymbols::java_lang_Double() && is_static() && !is_synchronized()) {
    if (name() == vmSymbols::java_lang_Double_longBitsToDouble_name()) {
      if (signature() == vmSymbols::long_double_signature()) return _longBitsToDouble;
    } else if (name() == vmSymbols::java_lang_Double_doubleToRawLongBits_name()) {
      if (signature() == vmSymbols::double_long_signature()) return _doubleToRawLongBits;
    } else if (name() == vmSymbols::java_lang_Double_doubleToLongBits_name()) {
      if (signature() == vmSymbols::double_long_signature()) return _doubleToLongBits;
    }
  } else if (klass_name == vmSymbols::java_lang_Float() && is_static() && !is_synchronized()) {
    if (name() == vmSymbols::java_lang_Float_intBitsToFloat_name()) {
      if (signature() == vmSymbols::int_float_signature()) return _intBitsToFloat;
    } else if (name() == vmSymbols::java_lang_Float_floatToRawIntBits_name()) {
      if (signature() == vmSymbols::float_int_signature()) return _floatToRawIntBits;
    } else if (name() == vmSymbols::java_lang_Float_floatToIntBits_name()) {
      if (signature() == vmSymbols::float_int_signature()) return _floatToIntBits;
    }
  } else if (klass_name == vmSymbols::java_lang_System() && is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_lang_System_arraycopy_name()) {
      if (signature() == vmSymbols::java_lang_System_arraycopy_signature()) return _arraycopy;
    } else if (name() == vmSymbols::java_lang_System_identityHashCode_name()) {
      if (signature() == vmSymbols::java_lang_System_identityHashCode_signature()) return _identityHash;
    } else if (name() == vmSymbols::java_lang_System_currentTimeMillis_name()) {
      if (signature() == vmSymbols::java_lang_System_currentTimeMillis_signature()) return _currentTimeMillis;
    } else if (name() == vmSymbols::java_lang_System_nanoTime_name()) {
      if (signature() == vmSymbols::java_lang_System_nanoTime_signature()) return _nanoTime;
    }

  } else if (klass_name == vmSymbols::java_lang_Thread() && is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_lang_Thread_currentThread_name()) {
      if (signature() == vmSymbols::java_lang_Thread_currentThread_signature()) return _currentThread;
    }

  } else if (klass_name == vmSymbols::java_lang_Thread() && !is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_lang_Thread_isInterrupted_name()) {
      if (signature() == vmSymbols::java_lang_Thread_isInterrupted_signature()) return _isInterrupted;
    }

  } else if (klass_name == vmSymbols::java_lang_Class() && !is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_lang_Class_isInstance_name()) {
      if (signature() == vmSymbols::java_lang_Class_isInstance_signature()) return _isInstance;
    } else if (name() == vmSymbols::java_lang_Class_getModifiers_name()) {
      if (signature() == vmSymbols::java_lang_Class_getModifiers_signature()) return _getModifiers;
    }

  } else if (klass_name == vmSymbols::sun_reflect_Reflection() && is_static() && !is_synchronized() && is_native()) {
    if        (name() == vmSymbols::getClassAccessFlags_name()) {
      if (signature() == vmSymbols::getClassAccessFlags_signature()) return _getClassAccessFlags;
    }

  } else if (klass_name == vmSymbols::java_lang_String() && !is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::compareTo_name()) {
      if (signature() == vmSymbols::string_int_signature()) return _compareTo;
    }

  } else if (klass_name == vmSymbols::java_nio_Buffer() && !is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::java_nio_Buffer_checkIndex_name()) {
      if (signature() == vmSymbols::java_nio_Buffer_checkIndex_signature ()) return _checkIndex;
    }

  } else if (klass_name == vmSymbols::sun_misc_AtomicLongCSImpl() && !is_static() && !is_synchronized()) {
    if        (name() == vmSymbols::sun_misc_AtomicLongCSImpl_attemptUpdate_name()) {
      if (signature() == vmSymbols::sun_misc_AtomicLongCSImpl_attemptUpdate_signature()) return _attemptUpdate;
    }

  } else if (klass_name == vmSymbols::sun_misc_Unsafe() && !is_static() && !is_synchronized() && is_native()) {
    IntrinsicId id = unsafe_intrinsic_id(name(), signature());
    if (id != _none)  return id;
  }
  return _none;
}


#ifndef CORE
// These two methods are static since a GC may move the methodOopDesc
bool methodOopDesc::load_signature_classes(methodHandle m, TRAPS) {
  bool sig_is_loaded = true;
  Handle class_loader(THREAD, instanceKlass::cast(m->method_holder())->class_loader());
  Handle protection_domain(THREAD, Klass::cast(m->method_holder())->protection_domain());
  symbolHandle signature(THREAD, m->signature());
  for(SignatureStream ss(signature); !ss.is_done(); ss.next()) {
    if (ss.is_object()) {
      symbolOop sym = ss.as_symbol(CHECK_(false));
      symbolHandle name (THREAD, sym);
      klassOop klass = SystemDictionary::resolve_or_null(name, class_loader,
                                             protection_domain, THREAD);
      // We are loading classes eagerly. If a ClassNotFoundException was generated,
      // be sure to ignore it.
      if (HAS_PENDING_EXCEPTION) {
        if (PENDING_EXCEPTION->is_a(SystemDictionary::classNotFoundException_klass())) {
          CLEAR_PENDING_EXCEPTION;
        } else {
          return false;
        }
      }
      if( klass == NULL) { sig_is_loaded = false; }
    }
  }
  return sig_is_loaded;
}

bool methodOopDesc::has_unloaded_classes_in_signature(methodHandle m, TRAPS) {
  Handle class_loader(THREAD, instanceKlass::cast(m->method_holder())->class_loader());
  Handle protection_domain(THREAD, Klass::cast(m->method_holder())->protection_domain());
  symbolHandle signature(THREAD, m->signature());
  for(SignatureStream ss(signature); !ss.is_done(); ss.next()) {
    if (ss.type() == T_OBJECT) {
      symbolHandle name(THREAD, ss.as_symbol_or_null());
      if (name() == NULL) return true;   
      klassOop klass = SystemDictionary::find(name, class_loader, protection_domain, THREAD);
      if (klass == NULL) return true;
    }
  }
  return false;
}
#endif // not CORE


// Exposed so field engineers can debug VM
void methodOopDesc::print_short_name(outputStream* st) {
  ResourceMark rm;
#ifdef PRODUCT
  st->print(" %s::", method_holder()->klass_part()->external_name());
#else
  st->print(" %s::", method_holder()->klass_part()->internal_name());
#endif
  name()->print_symbol_on(st);
  if (WizardMode) signature()->print_symbol_on(st);
}


extern "C" {
  static int method_compare(methodOop* a, methodOop* b) {
    return (*a)->name()->fast_compare((*b)->name());
  }

  typedef int (*compareFn)(const void*, const void*);
}


static void reorder_based_on_method_index(objArrayOop methods,
                                          objArrayOop annotations,
                                          oop* temp_array) {
  if (annotations == NULL) {
    return;
  }

  int length = methods->length();
  int i;
  // Copy to temp array
  memcpy(temp_array, annotations->obj_at_addr(0), length * sizeof(oop));

  // Copy back using old method indices
  for (i = 0; i < length; i++) {
    methodOop m = (methodOop) methods->obj_at(i);
    annotations->obj_at_put(i, temp_array[m->method_index()]);
  }
}


void methodOopDesc::sort_methods(objArrayOop methods,
                                 objArrayOop methods_annotations,
                                 objArrayOop methods_parameter_annotations,
                                 objArrayOop methods_default_annotations) {
  int length = methods->length();
  if (length > 1) {
    // Remember current method ordering so we can reorder annotations
    int i;
    for (i = 0; i < length; i++) {
      methodOop m = (methodOop) methods->obj_at(i);
      m->set_method_index(i);
    }

    qsort(methods->obj_at_addr(0), length, oopSize, (compareFn) method_compare);
    
    // Sort annotations if necessary
    assert(methods_annotations == NULL           || methods_annotations->length() == methods->length(), "");
    assert(methods_parameter_annotations == NULL || methods_parameter_annotations->length() == methods->length(), "");
    assert(methods_default_annotations == NULL   || methods_default_annotations->length() == methods->length(), "");
    if (methods_annotations != NULL ||
        methods_parameter_annotations != NULL ||
        methods_default_annotations != NULL) {
      // Allocate temporary storage
      oop* temp_array = NEW_RESOURCE_ARRAY(oop, length);
      reorder_based_on_method_index(methods, methods_annotations, temp_array);
      reorder_based_on_method_index(methods, methods_parameter_annotations, temp_array);
      reorder_based_on_method_index(methods, methods_default_annotations, temp_array);
    }

    // Reset method ordering
    for (i = 0; i < length; i++) {
      methodOop m = (methodOop) methods->obj_at(i);
      m->set_method_index(i);
    }
  }
}


//-----------------------------------------------------------------------------------
// Non-product code

#ifndef PRODUCT
class SignatureTypePrinter : public SignatureTypeNames {
 private:
  outputStream* _st;
  bool _use_separator;

  void type_name(const char* name) {
    if (_use_separator) _st->print(", ");
    _st->print(name);
    _use_separator = true;
  }

 public:
  SignatureTypePrinter(symbolHandle signature, outputStream* st) : SignatureTypeNames(signature) {
    _st = st;
    _use_separator = false;
  }

  void print_parameters()              { _use_separator = false; iterate_parameters(); }
  void print_returntype()              { _use_separator = false; iterate_returntype(); }
};


void methodOopDesc::print_name(outputStream* st) {
  SignatureTypePrinter sig(signature(), st);
  st->print("%s ", is_static() ? "static" : "virtual");
  sig.print_returntype();
  st->print(" %s.", method_holder()->klass_part()->internal_name());
  name()->print_symbol_on(st);
  st->print("(");
  sig.print_parameters();
  st->print(")");
}


void methodOopDesc::print_codes() const {
  print_codes(0, code_size());
}

void methodOopDesc::print_codes(int from, int to) const {
  Thread *thread = Thread::current();
  ResourceMark rm(thread);
  methodHandle mh (thread, (methodOop)this);
  BytecodeStream s(mh);
  s.set_interval(from, to);
  BytecodeTracer::set_closure(BytecodeTracer::std_closure());
  while (s.next() >= 0) BytecodeTracer::trace(mh, s.bcp());
}
#endif // not PRODUCT


// Simple compression of line number tables. We use a regular compressed stream, except that we compress deltas
// between (bci,line) pairs since they are smaller. If (bci delta, line delta) fits in (5-bit unsigned, 3-bit unsigned)
// we save it as one byte, otherwise we write a 0xFF escape character and use regular compression. 0x0 is used
// as end-of-stream terminator.


CompressedLineNumberWriteStream::CompressedLineNumberWriteStream(int initial_size) : CompressedWriteStream(initial_size) {
  _bci = 0;
  _line = 0;
};


void CompressedLineNumberWriteStream::write_pair(int bci, int line) {
  int bci_delta = bci - _bci;
  int line_delta = line - _line;
  _bci = bci;
  _line = line;
  // Skip (0,0) deltas - they do not add information and conflict with terminator.
  if (bci_delta == 0 && line_delta == 0) return;
  // Check if bci is 5-bit and line number 3-bit unsigned.
  if (((bci_delta & ~0x1F) == 0) && ((line_delta & ~0x7) == 0)) {
    // Compress into single byte.
    jubyte value = ((jubyte) bci_delta << 3) | (jubyte) line_delta;
    // Check that value doesn't match escape character.
    if (value != 0xFF) {
      write_byte(value);
      return;
    }
  }
  // bci and line number does not compress into single byte.
  // Write out escape character and use regular compression for bci and line number.
  write_byte((jubyte)0xFF);
  write_int(bci_delta);
  write_int(line_delta);
}


CompressedLineNumberReadStream::CompressedLineNumberReadStream(u_char* buffer) : CompressedReadStream(buffer) {
  _bci = 0;
  _line = 0;
};


bool CompressedLineNumberReadStream::read_pair() {
  jubyte next = read_byte();
  // Check for terminator
  if (next == 0) return false;
  if (next == 0xFF) {
    // Escape character, regular compression used
    _bci  += read_int();
    _line += read_int();
  } else {
    // Single byte compression used
    _bci  += next >> 3;
    _line += next & 0x7;
  }
  return true;
}


Bytecodes::Code methodOopDesc::orig_bytecode_at(int bci) {
  BreakpointInfo* bp = instanceKlass::cast(method_holder())->breakpoints();
  for (; bp != NULL; bp = bp->next()) {
    if (bp->match(this, bci)) {
      return bp->orig_bytecode();
    }
  }
  ShouldNotReachHere();
  return Bytecodes::_shouldnotreachhere;
}

void methodOopDesc::set_orig_bytecode_at(int bci, Bytecodes::Code code) {
  assert(code != Bytecodes::_breakpoint, "cannot patch breakpoints this way");
  BreakpointInfo* bp = instanceKlass::cast(method_holder())->breakpoints();
  for (; bp != NULL; bp = bp->next()) {
    if (bp->match(this, bci)) {
      bp->set_orig_bytecode(code);
      // and continue, in case there is more than one
    }
  }
}

void methodOopDesc::set_breakpoint(int bci) {
  instanceKlass* ik = instanceKlass::cast(method_holder());
  BreakpointInfo *bp = new BreakpointInfo(this, bci);
  bp->set_next(ik->breakpoints());
  ik->set_breakpoints(bp);
  // do this last:
  bp->set(this);
}

static void clear_matches(methodOop m, int bci) {
  instanceKlass* ik = instanceKlass::cast(m->method_holder());
  BreakpointInfo* prev_bp = NULL;
  BreakpointInfo* next_bp;
  for (BreakpointInfo* bp = ik->breakpoints(); bp != NULL; bp = next_bp) {
    next_bp = bp->next();
    // bci value of -1 is used to delete all breakpoints in method m (ex: clear_all_breakpoint).
    if (bci >= 0 ? bp->match(m, bci) : bp->match(m)) {
      // do this first:
      bp->clear(m);
      // unhook it
      if (prev_bp != NULL)
        prev_bp->set_next(next_bp);
      else
        ik->set_breakpoints(next_bp);
      delete bp;
      // When class is redefined JVMDI sets breakpoint in all versions of EMCP methods
      // at same location. So we have multiple matching (method_index and bci)
      // BreakpointInfo nodes in BreakpointInfo list. We should just delete one
      // breakpoint for clear_breakpoint request and keep all other method versions
      // BreakpointInfo for future clear_breakpoint request.
      // bcivalue of -1 is used to clear all breakpoints (see clear_all_breakpoints)
      // which is being called when class is unloaded. We delete all the Breakpoint
      // information for all versions of method. We may not correctly restore the original
      // bytecode in all method versions, but that is ok. Because the class is being unloaded
      // so these methods won't be used anymore.
      if (bci >= 0) {
        break;
      }
    } else {
      // This one is a keeper.
      prev_bp = bp;
    }
  }
}

void methodOopDesc::clear_breakpoint(int bci) {
  assert(bci >= 0, "");
  clear_matches(this, bci);
}

void methodOopDesc::clear_all_breakpoints() {
  clear_matches(this, -1);
}


BreakpointInfo::BreakpointInfo(methodOop m, int bci) {
  _bci = bci;
  _name_index = m->name_index();
  _signature_index = m->signature_index();
  _orig_bytecode = (Bytecodes::Code) *m->bcp_from(_bci);
  if (_orig_bytecode == Bytecodes::_breakpoint)
    _orig_bytecode = m->orig_bytecode_at(_bci);
  _next = NULL;
}

void BreakpointInfo::set(methodOop method) {
#ifdef ASSERT
  {
    Bytecodes::Code code = (Bytecodes::Code) *method->bcp_from(_bci);
    if (code == Bytecodes::_breakpoint)
      code = method->orig_bytecode_at(_bci);
    assert(orig_bytecode() == code, "original bytecode must be the same");
  }
#endif
  *method->bcp_from(_bci) = Bytecodes::_breakpoint;
  method->incr_number_of_breakpoints();
  SystemDictionary::notice_modification();
#ifndef CORE
  {
    // Deoptimize all dependents on this method
    Thread *thread = Thread::current();
    HandleMark hm(thread);
    methodHandle mh(thread, method);
    Universe::flush_dependents_on_method(mh);
  }
#endif /* CORE */
}

void BreakpointInfo::clear(methodOop method) {
  *method->bcp_from(_bci) = orig_bytecode();
  method->decr_number_of_breakpoints();
  assert(method->number_of_breakpoints() >= 0, "must not go negative");
}
