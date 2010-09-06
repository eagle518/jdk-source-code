#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Compilation.cpp	1.135 04/03/31 18:13:10 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_Compilation.cpp.incl"

static elapsedTimer _t_setup;
static elapsedTimer _t_optimizeIR;
static elapsedTimer _t_loopRecognition;
static elapsedTimer _t_buildIR;
static elapsedTimer _t_emit_lir;
static elapsedTimer _t_cache_locals;
static elapsedTimer _t_lir_optimize;
static elapsedTimer _t_codeemit;
static elapsedTimer _t_codeinstall;
static int totalInstructionNodes = 0;

Arena* Compilation::_arena = NULL;
Compilation* Compilation::_compilation = NULL;

// Implementation of Compilation


#ifndef PRODUCT

void Compilation::maybe_print_current_instruction() {
  if (_current_instruction != NULL && _last_instruction_printed != _current_instruction) {
    _last_instruction_printed = _current_instruction;
    _current_instruction->print();
  }
}
#endif // PRODUCT


CodeBuffer* Compilation::code() const {
  return _masm->code();
}


DebugInformationRecorder* Compilation::debug_info_recorder() const {
  return _env->recorder();
}


void Compilation::initialize() {
  _env->set_recorder(new DebugInformationRecorder(code()->oop_recorder()));
  debug_info_recorder()->set_oopmaps(new OopMapSet());
}


void Compilation::initialize_oop_maps() {
  _oop_map = method()->all_oop_maps();
  if (PrintOopMap) {
    if (_oop_map == NULL) tty->print_cr("NULL oop_map");
    else _oop_map->print();
  }
}


void Compilation::build_hir() {
  assert(!method()->is_native(), "should not reach here");
  if (bailed_out()) return;

  // setup ir
  _hir = new IR(this, method(), osr_bci());
  if (!_hir->is_valid()) {
    bailout("invalid parsing");
    return;
  }
  if (PrintCFG || PrintCFG0) { tty->print_cr("CFG after parsing"); _hir->print(true); }
  if (PrintIR  || PrintIR0 ) { tty->print_cr("IR after parsing"); _hir->print(false); }

  if (UseC1Optimizations) {
    NEEDS_CLEANUP
    // optimization
    _t_optimizeIR.start();
    _hir->optimize();
    _t_optimizeIR.stop();
  }

  // compute space for locals
  _hir->compute_locals_size();

  if (UseC1Optimizations) {
    // loop detection
    // note: after this, no block introduction/elimination must happen
    //       otherwise the loop information is going to be incorrect
    _t_loopRecognition.start();
    _hir->compute_loops();
    _t_loopRecognition.stop();
  }

  // compute block ordering for code generation
  _hir->compute_code();
  if (PrintCFG || PrintCFG2) { tty->print_cr("CFG before code generation"); _hir->code()->print(true); }
  if (PrintIR  || PrintIR2 ) { tty->print_cr("IR before code generation"); _hir->code()->print(false); }
}


void Compilation::init_framemap(FrameMap* map) {
  map->set_size_arguments(method()->arg_size());
  int size_locals = MAX2(method()->arg_size(), 
                         hir() == NULL ? method()->max_locals() : // for libraries
                                         hir()->locals_size_in_words());
  map->set_size_locals(size_locals);
  if (hir() == NULL) {
    // optimized library call: no monitors
    map->set_size_monitors(0);
  } else {
    map->set_num_monitors(hir()->number_of_locks()); 
    map->set_local_name_to_offset_map(hir()->local_name_to_offset_map());
  }
}


void Compilation::emit_lir() {
  assert(!method()->is_native(), "should not reach here");
  if (bailed_out()) return;
  FrameMap map(-1); // number of spills is not yet computed
  init_framemap(&map);
  debug_only(map.set_is_java_method(););
  
  set_frame_map(&map);

  LIR_Emitter* lir_emitter = new LIR_Emitter(this);
  ValueGen::init_value_gen();
  RegAlloc reg_alloc;
  ValueGenInvariant vgi(method(), &reg_alloc,  lir_emitter);
  ValueGen visitor(this, &vgi); // no code for the moment
  CodeGenerator gen(&visitor, &vgi);
  hir()->code()->iterate_forward(&gen);
  _max_spills = gen.max_spills();

  if (lir_emitter->must_bailout()) {
    bailout("LIR emission bailout");
  }

  // clear items
  hir()->code()->blocks_do(CodeGenerator::clear_instruction_items);

  // This frame map is dead at this point, do not try to use
  set_frame_map(NULL);
}


void Compilation::emit_code_prolog_native(FrameMap* map) {
  assert(method()->is_native(), "this method is only for native methods");
  map->set_size_arguments(method()->arg_size());
  int size_locals = MAX2(method()->arg_size(), method()->max_locals());
  map->set_size_locals(size_locals);
  int nofm = method()->is_synchronized() ? 1 : 0;
  map->set_num_monitors(nofm);
  // the reserved_argument_area holds the arguments to the native call + 
  // the jni environment + a handle to the klass mirror (if static)
  int reserved_argument_area_size = method()->arg_size() + 1 + (method()->is_static() ? 1 : 0);
  // substract the number of arguments being passed in registers
  reserved_argument_area_size = MAX2(reserved_argument_area_size - Argument::n_register_parameters, 0);
  map->set_reserved_argument_area_size(reserved_argument_area_size);
  set_frame_map(map);
}


void Compilation::emit_code_prolog_non_native(FrameMap* map) {
  assert(!method()->is_native(), "this method is only for non-native methods");
  init_framemap(map);
  NEEDS_CLEANUP
  // because some slow paths require at least 4 words in the argument area,
  // we will allocate at least 4 words in the outgoing arguments area
  // Note that intrinsics have no IR; for all others, must handle inlining properly
  int reserved_argument_area_size = MAX2(4, hir() ? hir()->max_stack() : method()->max_stack());
  map->set_reserved_argument_area_size(reserved_argument_area_size);
  set_frame_map(map);
}


void Compilation::emit_code_epilog(LIR_Assembler* assembler) {
  if (bailed_out()) return;
  // generate code for osr, if any
  if (UseOnStackReplacement && is_osr_compile()) {
    BlockBegin* osr_entry = hir()->osr_entry();
    assert(osr_entry != NULL && osr_entry->is_set(BlockBegin::osr_entry_flag), "osr entry block must exist");
    assembler->emit_osr_entry(osr_entry->scope(), osr_entry->end()->state()->locks_size(), osr_entry->label(), osr_bci());
  }
  // generate code or slow cases
  assembler->emit_slow_case_stubs();
  // generate code for exception handler
  int offset = assembler->emit_exception_handler();
  code()->set_exception_offset(offset);
  assembler->emit_call_stubs();
  // done
  masm()->flush();
}


intStack* Compilation::get_init_vars() {
  intStack* vars = new intStack();
  if (oop_map() != NULL) {
    for (int i = 0; i < oop_map()->nof_locals_to_initialize(); i++) {
      vars->append(oop_map()->local_to_initialize(i));
    }
  }
  return vars;
}


int Compilation::emit_code_body(CodeOffsets* offsets) {
  // emit code
  LIR_Assembler lir_asm(this, offsets);
  LIR_Emitter emit(this);

  lir_asm.emit_code(hir()->code());

  emit_code_epilog(&lir_asm);

  generate_exception_range_table();

#ifndef PRODUCT
  if (PrintExceptionHandlers && Verbose) {
    tty->print_cr("Exception handlers:");
    for (int i = 0; i < exception_range_table()->length(); i++) {
      ExceptionRangeEntry* entry = exception_range_table()->entry_at(i);
      tty->print_cr("  start PC offset 0x%x, end PC offset 0x%x, scope count %d, exception type %d, handler PC offset 0x%x",
                    entry->start_pco(), entry->end_pco(), entry->scope_count(), entry->exception_type(), entry->handler_pco());
    }
  }
#endif /* PRODUCT */

  // return
  if (emit.must_bailout()) {
    bailout("LIR emission bailout");
  } else if (lir_asm.must_bailout()) {
    bailout("LIR assembler bailout");
  }
  return emit.frame_size();
}


int Compilation::compile_java_method(CodeOffsets* offsets) {
#ifdef _LP64
  bailout("LP64 not supported");
  if (bailed_out()) return no_frame_size;
#endif // _LP64

  assert(!method()->is_native(), "should not reach here");
  if (bailed_out()) return no_frame_size;
  _t_setup.start();

  initialize_oop_maps();

  _t_setup.stop(); _t_buildIR.start();

  build_hir();

  if (BailoutAfterHIR) {
    bailout("Bailing out because of -XX:+BailoutAfterHIR");
    return no_frame_size;
  }

  _t_buildIR.stop(); _t_emit_lir.start();

  emit_lir();

   _t_emit_lir.stop(); _t_cache_locals.start();

  if (bailed_out()) return no_frame_size;

  if (LIRCacheLocals && !jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY) && !jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY2)) {
    LIR_LocalCaching c(hir());
    c.compute_cached_locals();
  }

  assert(!method()->is_native(), "should not reach here");
  FrameMap map(max_spills());
  debug_only(map.set_is_java_method();)
  emit_code_prolog_non_native(&map);

   _t_cache_locals.stop(); _t_lir_optimize.start();

  if (LIROptimize || LIRCacheLocals || LIRFillDelaySlots) {
    // perform various backend optimizations on the LIR
    LIR_Optimizer lopt(hir());
    lopt.optimize();
    if (lopt.saw_fpu2cpu_moves()) {
      map.add_spill_slots(2); 
    }
  }

  _t_lir_optimize.stop(); _t_codeemit.start();

  int frame_size = emit_code_body(offsets);

  _t_codeemit.stop();

#ifdef _LP64
  NEEDS_CLEANUP;
  // check that these are still off
  assert(!LIROptimize && !LIRCacheLocals, "these shouldn't magically turn on");
#endif // _LP64

  return frame_size;
}


void Compilation::emit_code_for_native(address native_entry, CodeOffsets* offsets) {
  assert(method()->is_native(), "should not reach here");
  assert(!is_osr_compile(), "no OSR for natives");
  if (bailed_out()) return;

  // The spill area is used for two purposes:
  //    1. to store the method holder mirror (only if the method is static)
  //    2. to preserve the return value (eax/edx) if the method is synchronized (intel only)
  NEEDS_CLEANUP;
  int nof_spills = 1; // Must always have the slot allocated since we refer to them by absolute index
#ifndef SPARC
  nof_spills += (method()->is_synchronized() ? 2 : 0);
#endif
  // must preserve floating point arguments
  switch (method()->return_type()->basic_type()) { 
    case T_FLOAT : nof_spills += 1; break; 
    case T_DOUBLE: nof_spills += 2; break; 
  } 
  FrameMap map(nof_spills);
  emit_code_prolog_native(&map);

  intStack* spilled = NULL;
  if (method()->is_static()) {
    spilled = new intStack();
    spilled->append(0); // stack slot for the mirror class
  }
  IRScope scope(this, NULL, -1, method(), -1, false);
  CodeEmitInfo* info = new CodeEmitInfo(&scope, 0, spilled);

  LIR_Assembler lir_asm(this, offsets);
  LIR_Emitter   emit (this);

  lir_asm.emit_method_entry(&emit, &scope);
  lir_asm.emit_native_call(native_entry, info);
  lir_asm.emit_native_method_exit(info);
  emit_code_epilog(&lir_asm);
}


int Compilation::compile_native_method(CodeOffsets* offsets) {
  address entry = method()->native_entry();
  assert(entry != NULL, "native entry must have been precomputed");
  emit_code_for_native(entry, offsets);
  return no_frame_size;
}


bool Compilation::is_optimized_library_method() const {
  if (!jvmpi_event_method_enabled()) {
    ciMethod::IntrinsicId id = method()->intrinsic_id();
    // optimizable library methods
    switch (id) {
    case ciMethod::_dsin     : // fall through
    case ciMethod::_dcos     : // fall through
    case ciMethod::_compareTo: // fall through
      return true;
    }
  }
  // default
  return false;
}


int Compilation::compile_library_method(CodeOffsets* offsets) {
  ciMethod::IntrinsicId id = method()->intrinsic_id();
  assert(is_optimized_library_method(), "only optimized library methods");

  FrameMap map(0);
  emit_code_prolog_non_native(&map);

  IRScope scope(this, NULL, -1, method(), -1, false);
  LIR_Assembler lir_asm(this, offsets);
  LIR_Emitter   emit (this);

  lir_asm.emit_method_entry(&emit, &scope);
  switch (id) {
    case methodOopDesc::_dsin     : // fall through
    case methodOopDesc::_dcos     : lir_asm.emit_triglib(id);            break;
    case methodOopDesc::_compareTo: lir_asm.emit_string_compare(&scope); break;
    default                       : ShouldNotReachHere();
  };
  emit_code_epilog(&lir_asm);
  return no_frame_size;
}


void Compilation::install_code(CodeOffsets* offsets, int frame_size) {
  _env->register_method(
    method(),
    osr_bci(),
    offsets->_iep_offset,
    offsets->_ep_offset,
    offsets->_vep_offset,
    offsets->_code_offset,
    offsets->_osr_offset,
    code(),
    frame_size,
    debug_info_recorder()->_oopmaps,
    NULL, // no exception table
    null_check_table(),
    exception_range_table(),
    compiler(),
    needs_debug_information(),
    has_unsafe_access()
  );
}


void Compilation::compile_method() {
  // setup compilation
  initialize();

  // compile method
  CodeOffsets offsets;
  int frame_size;
  if (OptimizeLibraryCalls && !is_osr_compile() && is_optimized_library_method()) {
    // note: we cannot produce an osr method for libray calls
    //       because these methods are hand-coded and no osr
    //       entry point is provided!
    frame_size = compile_library_method(&offsets);
  } else if (method()->is_native()) {
    frame_size = compile_native_method(&offsets);
  } else {
    frame_size = compile_java_method(&offsets);
  }

  // bailout if method couldn't be compiled
  // Note: make sure we mark the method as not compilable!
  if (bailed_out()) return;

  _t_codeinstall.start();
  if (InstallMethods) {
    // install code
    install_code(&offsets, frame_size);
  }
  _t_codeinstall.stop();
  totalInstructionNodes += Instruction::number_of_instructions();
}


void Compilation::generate_exception_range_table() {
  // Generate an ExceptionRangeTable from the exception handler
  // information accumulated during the compilation. We want to
  // generate program counter ranges from the discrete PC offsets we
  // have in the ExceptionInfoList.
  ExceptionInfoList* info_list = exception_info_list();
  ExceptionScope* last_recorded_scope = NULL;
  int last_pco = -1;
  bool extend = false;

  for (int i = 0; i < info_list->length(); i++) {
    ExceptionInfo* info = info_list->at(i);
    ExceptionScope* scope = info->exception_scope();
    // Be conservative -- seems to be tricky to figure out when it is
    // legal to extend a PC range for an exception handler to cover the
    // current one. To be safe, we will only do this when the entire set
    // of exception handlers for the current PC is equivalent to that
    // for the last PC where we added exception handlers.
    extend = scope->equals(last_recorded_scope);
    add_exception_range_entries(info->pco(), info->at_call(), scope, extend, &last_recorded_scope, &last_pco);
  }
}


void Compilation::add_exception_range_entries(int pco, bool at_call, ExceptionScope* scope, bool extend, ExceptionScope** last_recorded_scope, int* last_pco) {
  int entry_index;
  ExceptionRangeTable* table = exception_range_table();
  ExceptionScope* top_scope = scope;

  pco = ExceptionRangeTable::compute_modified_at_call_pco(pco, at_call);

  assert((*last_pco) < 0 || (pco > *last_pco), "PC offsets must be monotonically increasing");
  debug_only(*last_pco = pco);

  // Ensure we do not accidentally extend ranges to cover PCs we should not
  // if we don't have exception handlers for a given PC
  *last_recorded_scope = NULL;

  if (extend) {
    entry_index = table->entry_index_for_pco(pco);
    assert(entry_index != -1, "must be found");
  }

  int scope_count = 0;
  while (scope != NULL) {
    for (int i = 0; i < scope->length(); i++) {
      XHandler* handler = scope->handler_at(i);
      if (extend) {
        ExceptionRangeEntry* entry = table->entry_at(entry_index);
        assert((entry->exception_type() == handler->catch_type() &&
                entry->handler_pco()    == handler->start_pco() &&
                entry->scope_count()    == scope_count), "incorrect computation of extend flag");
        // Can extend this exception handler's range
        entry->set_end_pco(pco + 1);
        ++entry_index;
      } else {
        // Create a new ExceptionRangeEntry
        table->add_entry(pco, pco + 1, scope_count,
                         handler->catch_type(), handler->start_pco(), handler->handler_bci());
      }
      *last_recorded_scope = top_scope;
    }

    scope = scope->caller_scope();
    ++scope_count;
  }
}


Compilation::Compilation(AbstractCompiler* compiler, ciEnv* env, ciMethod* method, int osr_bci, C1_MacroAssembler* masm)
: _compiler(compiler)
, _env(env)
, _method(method)
, _osr_bci(osr_bci)
, _hir(NULL)
, _max_spills(-1)
, _oop_map(NULL)
, _frame_map(NULL)
, _masm(masm)
, _has_exception_handlers(false)
, _has_unsafe_access(false)
, _bailout_msg(NULL)
, _exception_info_list(NULL)
, _jvmpi_event_compiled_method_enabled(jvmpi::is_event_enabled(JVMPI_EVENT_COMPILED_METHOD_LOAD))
, _jvmpi_event_method_entry_enabled   (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY))
, _jvmpi_event_method_entry2_enabled  (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY2))
, _jvmpi_event_method_exit_enabled    (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_EXIT))
#ifndef PRODUCT
, _current_instruction(NULL)
, _last_instruction_printed(NULL)
#endif // PRODUCT
{
  assert(_arena == NULL, "shouldn't only one instance of Compilation in existence at a time");
  _arena = new Arena();
  _compilation = this;
  _needs_debug_information = jvmpi::enabled() || JvmtiExport::can_examine_or_deopt_anywhere() || 
                               JavaMonitorsInStackTrace || AlwaysEmitDebugInfo || DeoptimizeALot;
  _exception_info_list = new ExceptionInfoList();
  _null_check_table.set_size(0);
  compile_method();
}

Compilation::~Compilation() {
  delete _arena;
  _arena = NULL;
  _compilation = NULL;
}


void Compilation::add_exception_handlers_for_pco(int pco, bool at_call, ExceptionScope* exception_scope) {
  if (PrintExceptionHandlers && Verbose) {
    tty->print_cr("  added exception scope for pco %d %s", pco, (at_call ? "(at_call)" : ""));
  }
  // Note: we do not have program counters for these exception handlers yet
  exception_info_list()->push(new ExceptionInfo(pco, at_call, exception_scope));
}


void Compilation::notice_inlined_method(ciMethod* method) {
  _env->notice_inlined_method(method);
}


void Compilation::bailout(const char* msg) {
  assert(msg != NULL, "bailout message must exist");
  if (!bailed_out()) {
    // keep first bailout message
    if (PrintBailouts) tty->print_cr("compilation bailout: %s", msg);
    _bailout_msg = msg;
  }
}


void Compilation::print_timers() {
  // tty->print_cr("    Native methods         : %6.3f s, Average : %2.3f", CompileBroker::_t_native_compilation.seconds(), CompileBroker::_t_native_compilation.seconds() / CompileBroker::_total_native_compile_count);
  float total = _t_setup.seconds() + _t_buildIR.seconds() + _t_emit_lir.seconds() + _t_cache_locals.seconds() + _t_lir_optimize.seconds() + _t_codeemit.seconds() + _t_codeinstall.seconds();


  tty->print_cr("    Detailed C1 Timings");
  tty->print_cr("       Setup time:        %6.3f s (%4.1f%%)",    _t_setup.seconds(),           (_t_setup.seconds() / total) * 100.0);
  tty->print_cr("       Build IR:          %6.3f s (%4.1f%%)",    _t_buildIR.seconds(),         (_t_buildIR.seconds() / total) * 100.0);
  tty->print_cr("         Optimize:           %6.3f s (%4.1f%%)", _t_optimizeIR.seconds(),      (_t_optimizeIR.seconds() / total) * 100.0);
  tty->print_cr("         Loop Comp.:         %6.3f s (%4.1f%%)", _t_loopRecognition.seconds(), (_t_loopRecognition.seconds() / total) * 100.0);
  tty->print_cr("       Emit LIR:          %6.3f s (%4.1f%%)",    _t_emit_lir.seconds(),        (_t_emit_lir.seconds() / total) * 100.0);
  tty->print_cr("       Cache Locals:      %6.3f s (%4.1f%%)",    _t_cache_locals.seconds(),    (_t_cache_locals.seconds() / total) * 100.0);
  tty->print_cr("       LIR Optimize:      %6.3f s (%4.1f%%)",    _t_lir_optimize.seconds(),    (_t_lir_optimize.seconds() / total) * 100.0);
  tty->print_cr("       Code Emission:     %6.3f s (%4.1f%%)",    _t_codeemit.seconds(),        (_t_codeemit.seconds() / total) * 100.0);
  tty->print_cr("       Code Installation: %6.3f s (%4.1f%%)",    _t_codeinstall.seconds(),     (_t_codeinstall.seconds() / total) * 100.0);
  tty->print_cr("       Instruction Nodes: %6d nodes",    totalInstructionNodes);
}


#ifndef PRODUCT
void Compilation::compile_only_this_method() {
  ResourceMark rm;
  fileStream stream(fopen("c1_compile_only", "wt"));
  stream.print_cr("# c1 compile only directives");
  compile_only_this_scope(&stream, hir()->top_scope());
}


void Compilation::compile_only_this_scope(outputStream* st, IRScope* scope) {
  st->print("CompileOnly=");
  scope->method()->holder()->name()->print_symbol_on(st);
  st->print(".");
  scope->method()->name()->print_symbol_on(st);
  st->cr();
}
#endif


LIR_Opr Compilation::lir_opr_for_instruction(Value v) {
  Item* item = v->item();
  LIR_Opr res = NULL;
  if (UseNewCodeGen && v->operand()->is_valid()) {
    return v->operand();
  }
  if (item == NULL) {
    // instruction was not visited yet, use instruction itself
    if (v->as_Constant() != NULL) {
      // alway produce a real LIR_Opr for constants even if they are unused
      // because it's possible that the IR has optimized away their use but
      // when deoptimizing the value need to be rematerialized.
      res = LIR_OprFact::value_type(v->as_Constant()->type());
    } else if (v->use_count() == 0) {
      // produce a dummy LIR_Opr for all other unused instructions
      res = LIR_OprFact::dummy_value_type(v->type());
    } else if (v->as_LoadLocal() != NULL) {
      assert(v->as_LoadLocal()->has_local_name(), "must have local name assigned");
      int pos = v->as_LoadLocal()->local_name();
      if (v->type()->is_double_word() ) {
        res = LIR_OprFact::double_stack(pos, as_BasicType(v->type()));
      } else {
        res = LIR_OprFact::single_stack(pos, as_BasicType(v->type()));
      }
    } else {
      ShouldNotReachHere();
    }
  } else {
    if (item->is_valid()) {
      res = item2lir(item);
    } else {
      res = LIR_OprFact::dummy_value_type(v->type());
    }
  }
  return res;
}


GrowableArray<LIR_Opr>* Compilation::value_stack2lir_stack(ValueStack* value_stack) {
  GrowableArray<LIR_Opr>* expressions = NULL;
  if (value_stack != NULL) {
    expressions = new GrowableArray<LIR_Opr>(value_stack->stack_size());
    for (int i = 0; i < value_stack->stack_size();) {
      // create a LIR operand for each Item
      Value value = value_stack->stack_at_inc(i); 
      expressions->append(lir_opr_for_instruction(value));
    }
  }
  return expressions;
}


LIR_Opr Compilation::item2lir(const Item* i) {
  LIR_Opr res;
  if (i->is_register()) {
    res = LIR_OprFact::rinfo(i->get_register(), as_BasicType(i->type()));
  } else if (i->is_constant()) {
    res = LIR_OprFact::value_type(i->type());
  } else if (i->is_stack()) {
    if (i->type()->is_double_word() ) {
      res = LIR_OprFact::double_stack(item2stack(i), as_BasicType(i->type()));
    } else {
      res = LIR_OprFact::single_stack(item2stack(i), as_BasicType(i->type()));
    }
  } else if (!i->is_valid()) {
    res = LIR_OprFact::illegalOpr;
  } else {
    ShouldNotReachHere();
  }
  if (i->destroys_register() && i->is_register()) {
    res = res->make_destroyed();
  }
  return res;
}


int Compilation::item2stack(const Item* item) {
  assert(item->is_stack(),"must be stack item");
  if (item->is_spilled()) {
    return frame_map()->spill_name(item->get_spilled_index());
  } else {
    int local_name = item->get_stack();
    if (item->type()->is_double()) {
      if (!frame_map()->are_adjacent_indeces(local_name, local_name+1)) {
        bailout("double elements not adjacent" );
      }
    }
    // index adjustment will be done later on
    return local_name;
  }
}


// Implementation of ExceptionInfo

ExceptionInfo::ExceptionInfo(int pco, bool at_call, ExceptionScope* exception_scope)
  : _pco(pco)
  , _at_call(at_call)
  , _exception_scope(exception_scope)
{
}

