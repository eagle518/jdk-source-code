#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compile.cpp	1.581 04/04/30 16:42:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_compile.cpp.incl"

/// Support for intrinsics.

// Return the index at which m must be inserted (or already exists).
// The sort order is by the address of the ciMethod, with is_virtual as minor key.
int Compile::intrinsic_insertion_index(ciMethod* m, bool is_virtual) {
#ifdef ASSERT
  for (int i = 1; i < _intrinsics->length(); i++) {
    CallGenerator* cg1 = _intrinsics->at(i-1);
    CallGenerator* cg2 = _intrinsics->at(i);
    assert(cg1->method() != cg2->method()
           ? cg1->method()     < cg2->method()
           : cg1->is_virtual() < cg2->is_virtual(),
           "compiler intrinsics list must stay sorted");
  }
#endif
  // Binary search sorted list, in decreasing intervals [lo, hi].
  int lo = 0, hi = _intrinsics->length()-1;
  while (lo <= hi) {
    int mid = (uint)(hi + lo) / 2;
    ciMethod* mid_m = _intrinsics->at(mid)->method();
    if (m < mid_m) {
      hi = mid-1;
    } else if (m > mid_m) {
      lo = mid+1;
    } else {
      // look at minor sort key
      bool mid_virt = _intrinsics->at(mid)->is_virtual();
      if (is_virtual < mid_virt) {
        hi = mid-1;
      } else if (is_virtual > mid_virt) {
        lo = mid+1;
      } else {
        return mid;  // exact match
      }
    }
  }
  return lo;  // inexact match
}

void Compile::register_intrinsic(CallGenerator* cg) {
  if (_intrinsics == NULL) {
    _intrinsics = new GrowableArray<CallGenerator*>(60);
  }
  // This code is stolen from ciObjectFactory::insert.
  // Really, GrowableArray should have methods for
  // insert_at, remove_at, and binary_search.
  int len = _intrinsics->length();
  int index = intrinsic_insertion_index(cg->method(), cg->is_virtual());
  if (index == len) {
    _intrinsics->append(cg);
  } else {
#ifdef ASSERT
    CallGenerator* oldcg = _intrinsics->at(index);
    assert(oldcg->method() != cg->method() || oldcg->is_virtual() != cg->is_virtual(), "don't register twice");
#endif
    _intrinsics->append(_intrinsics->at(len-1));
    int pos;
    for (pos = len-2; pos >= index; pos--) {
      _intrinsics->at_put(pos+1,_intrinsics->at(pos));
    }
    _intrinsics->at_put(index, cg);
  }
  assert(find_intrinsic(cg->method(), cg->is_virtual()) == cg, "registration worked");
}

CallGenerator* Compile::find_intrinsic(ciMethod* m, bool is_virtual) {
  assert(m->is_loaded(), "don't try this on unloaded methods");
  if (_intrinsics != NULL) {
    int index = intrinsic_insertion_index(m, is_virtual);
    if (index < _intrinsics->length()
        && _intrinsics->at(index)->method() == m) {
      return _intrinsics->at(index);
    }
  }
  // Lazily create intrinsics for intrinsic IDs well-known in the runtime.
  if (m->intrinsic_id() != ciMethod::_none) {
    CallGenerator* cg = make_vm_intrinsic(m, is_virtual);
    if (cg != NULL) {
      // Save it for next time:
      register_intrinsic(cg);
      return cg;
    }
  }
  return NULL;
}

// Compile:: register_library_intrinsics and make_vm_intrinsic are defined
// in library_call.cpp.


ciMethod* Compile::get_Method_invoke() {
  assert(Universe::is_gte_jdk14x_version() && UseNewReflection, "Should not reach here otherwise");

  if (_Method_invoke == NULL) {
    ciInstanceKlass* _Method =
        env()->find_system_klass(ciSymbol::make("java/lang/reflect/Method"))->as_instance_klass();
    if (!_Method->is_loaded())  return NULL;
    _Method_invoke =
      _Method->find_method(get_invoke_name(),
                           ciSymbol::make("(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"));
    assert(_Method_invoke != NULL, "must be able to find");
  }
  return _Method_invoke;
}


ciSymbol* Compile::get_invoke_name() {
  assert(Universe::is_gte_jdk14x_version() && UseNewReflection, "Should not reach here otherwise");

  if (_invoke_name == NULL) {
    _invoke_name = ciSymbol::make("invoke");
  }
  return _invoke_name;
}

ciInstanceKlass* Compile::get_MethodAccessorImpl() {
  assert(Universe::is_gte_jdk14x_version() && UseNewReflection, "Should not reach here otherwise");

  if (_MethodAccessorImpl == NULL) {
    _MethodAccessorImpl = env()->find_system_klass(ciSymbol::make("sun/reflect/MethodAccessorImpl"))->as_instance_klass();
  }
  return _MethodAccessorImpl;
}

// Support for bundling info
Bundle* Compile::node_bundling(const Node *n) {
  assert(valid_bundle_info(n), "oob");
  return &_node_bundling_base[n->_idx];
}

bool Compile::valid_bundle_info(const Node *n) {
  return (_node_bundling_limit > n->_idx);
}


// Identify all nodes that are reachable from below, useful.
// Use breadth-first pass that records state in a Unique_Node_List,
// recursive traversal is slower.
void Compile::identify_useful_nodes(Unique_Node_List &useful) {
  int estimated_worklist_size = unique();
  useful.map( estimated_worklist_size, NULL );  // preallocate space

  // Initialize worklist
  if (root() != NULL)     { useful.push(root()); }
  // If 'top' is cached, declare it useful to preserve cached node
  if( cached_top_node() ) { useful.push(cached_top_node()); }

  // Push all useful nodes onto the list, breadthfirst
  for( uint next = 0; next < useful.size(); ++next ) {
    assert( next < unique(), "Unique useful nodes < total nodes");
    Node *n  = useful.at(next);
    uint max = n->len();
    for( uint i = 0; i < max; ++i ) {
      Node *m = n->in(i);
      if( m == NULL ) continue;
      useful.push(m);
    }
  }
}

// Disconnect all useless nodes by disconnecting those at the boundary.
void Compile::remove_useless_nodes(Unique_Node_List &useful) {
  uint next = 0;
  while( next < useful.size() ) {
    Node *n = useful.at(next++);
    // Use raw traversal of out edges since this code removes out edges 
    int max = n->outcnt();
    for (int j = 0; j < max; ++j ) {
      Node* child = n->raw_out(j);
      if( ! useful.member(child) ) {
        assert( !child->is_top() || child != top(),
                "If top is cached in Compile object it is in useful list");
        // Only need to remove this out-edge to the useless node
        n->raw_del_out(j);
        --j;
        --max;
      }
    }
  }
  debug_only(verify_graph_edges(true/*check for no_dead_code*/);)
}

//------------------------------frame_size_in_words-----------------------------
// frame_slots in units of words
int Compile::frame_size_in_words() const { 
  // shift is 0 in LP32 and 1 in LP64
  const int shift = (LogBytesPerWord - LogBytesPerInt);
  int words = _frame_slots >> shift;
  assert( words << shift == _frame_slots, "frame size must be properly aligned in LP64" );
  return words;
}

// ============================================================================
//------------------------------CompileWrapper---------------------------------
class CompileWrapper : public StackObj {
  Compile *const _compile;
 public:
  CompileWrapper(Compile* compile);

  ~CompileWrapper();
};
  
CompileWrapper::CompileWrapper(Compile* compile) : _compile(compile) {
  COMPILER1_ONLY(ShouldNotReachHere());
  ciEnv* env = compile->env();
  assert(env->data() == NULL, "compile already active?");
  ciEnv::set_data(compile);

  compile->set_type_dict(NULL);
  compile->set_type_hwm(NULL);
  compile->set_type_last_size(0);
  compile->set_last_tf(NULL, NULL);
  compile->set_indexSet_arena(NULL);
  compile->set_indexSet_free_block_list(NULL);
  compile->init_type_arena();
  Type::Initialize(compile);
  _compile->set_scratch_buffer_blob(NULL);
}
CompileWrapper::~CompileWrapper() {
  if (_compile->scratch_buffer_blob() != NULL)
    BufferBlob::free(_compile->scratch_buffer_blob());
  ciEnv::set_data(NULL);
}


//----------------------------print_compile_messages---------------------------
void Compile::print_compile_messages() {
#ifndef PRODUCT
  // Check if recompiling
  if (_subsume_loads == false && PrintOpto) {
    // Recompiling without allowing machine instructions to subsume loads
    tty->print_cr("*********************************************************");
    tty->print_cr("** Bailout: Recompile without subsuming loads          **");
    tty->print_cr("*********************************************************");
  }
  if (env()->break_at_compile()) {
    // Open the debugger when compiing this method.
    tty->print("### Breaking when compiling: ");
    method()->print_short_name();
    tty->cr();
    BREAKPOINT;
  }

  if( PrintOpto ) {
    if (is_osr_compilation()) {
      tty->print("[OSR]%3d", _compile_id);
    } else {
      tty->print("%3d", _compile_id);
    }
  }
#endif
}


void Compile::init_scratch_buffer_blob() {
  if( scratch_buffer_blob() != NULL )  return;

  // Construct a temporary CodeBuffer to have it construct a BufferBlob
  // Cache this BufferBlob for this compile.
  ResourceMark rm;
  BufferBlob *blob = NULL;
  init_scratch_locs_memory();
  OopRecorder oop_recorder;
  CodeBuffer buf(MAX_inst_size, MAX_locs_size,
                 MAX_stubs_size, MAX_const_size, MAX_locs_stub_size, true, blob,
                 scratch_locs_memory(), scratch_locs_stub_memory(), false /*auto_free_blob*/,
                 (OopRecorder*)&oop_recorder, NULL/*name*/);
  // Record the buffer blob for next time.
  set_scratch_buffer_blob(buf.get_blob());
  guarantee(scratch_buffer_blob() != NULL, "Need BufferBlob for code generation");
}

// Initialize the relocation buffers
void Compile::init_scratch_locs_memory() {
  // Round up the sizes to the next full size
  set_scratch_locs_memory(
    NEW_RESOURCE_ARRAY(relocInfo,      1 + (MAX_locs_size-1)      / sizeof(relocInfo) ) );
  set_scratch_locs_stub_memory(
    NEW_RESOURCE_ARRAY(RelocateBuffer, 1 + (MAX_locs_stub_size-1) / sizeof(RelocateBuffer) ) );
}


// ============================================================================
//------------------------------Compile standard-------------------------------
debug_only( int Compile::_debug_idx = 100000; )

// Compile a method.  entry_bci is -1 for normal compilations and indicates
// the continuation bci for on stack replacement.


Compile::Compile( ciEnv* ci_env, C2Compiler* compiler, ciMethod* target, int osr_bci, bool subsume_loads )
                : Phase(Compiler),
                  _env(ci_env),
                  _log(ci_env->log()),
                  _compile_id(ci_env->compile_id()),
                  _save_argument_registers(false),
                  _stub_name(NULL),
                  _stub_function(NULL),
                  _stub_entry_point(NULL),
                  _method(target),
                  _entry_bci(osr_bci),
                  _initial_gvn(NULL),
                  _for_igvn(NULL),
                  _warm_calls(NULL),
                  _subsume_loads(subsume_loads),
                  _failure_reason(NULL),
                  _node_bundling_limit(0),
                  _node_bundling_base(NULL) {
  CompileWrapper cw(this);
#ifndef PRODUCT
  if (TimeCompiler2) {
    tty->print(" ");
    target->holder()->name()->print();
    tty->print(".");
    target->print_short_name();
    tty->print("  ");
  }
#endif
  TraceTime t1("Total compilation time", &_t_totalCompilation, TimeCompiler, TimeCompiler2);
  TraceTime t2(NULL, &_t_methodCompilation, TimeCompiler, false);
  set_print_assembly(PrintOptoAssembly NOT_PRODUCT( || _method->should_print_assembly()) );

  if (ProfileTraps) {
    // Make sure the method being compiled gets its own MDO,
    // so we can at least track the decompile_count().
    method()->build_method_data();
  }

  Init(::AliasLevel);

  print_compile_messages();

  if (UseOldInlining || PrintCompilation || PrintOpto)
    _ilt = InlineTree::build_inline_tree_root();
  else
    _ilt = NULL;

  // Even if NO memory addresses are used, MergeMem nodes must have at least 1 slice
  assert(num_alias_types() >= AliasIdxRaw, "");

#define MINIMUM_NODE_HASH  1023
  // Node list that Iterative GVN will start with
  Unique_Node_List for_igvn(comp_arena());
  set_for_igvn(&for_igvn);
  
  // GVN that will be run immediately on new nodes
  uint estimated_size = method()->code_size()*4+64;
  estimated_size = (estimated_size < MINIMUM_NODE_HASH ? MINIMUM_NODE_HASH : estimated_size);
  PhaseGVN gvn(node_arena(), estimated_size);
  set_initial_gvn(&gvn);

  { // Scope for timing the parser
    TracePhase t3("parse", &_t_parser, true);

    // Put top into the hash table ASAP.
    initial_gvn()->transform_no_reclaim(top());

    // Set up tf(), start(), and find a CallGenerator.
    CallGenerator* cg;
    if (is_osr_compilation()) {
      const TypeTuple *domain = StartOSRNode::osr_domain();
      const TypeTuple *range = TypeTuple::make_range(method()->signature());
      init_tf(TypeFunc::make(domain, range));
      StartNode* s = new StartOSRNode(root(), domain);
      initial_gvn()->set_type_bottom(s);
      init_start(s);
      cg = CallGenerator::for_osr(method(), entry_bci());
    } else {
      // Normal case.
      init_tf(TypeFunc::make(method()));
      StartNode* s = new StartNode(root(), tf()->domain());
      initial_gvn()->set_type_bottom(s);
      init_start(s);
      float past_uses = method()->interpreter_invocation_count();
      float expected_uses = past_uses;
      cg = CallGenerator::for_inline(method(), expected_uses);
    }
    if (failing())  return;
    if (cg == NULL) {
      record_method_not_compilable_all_tiers("cannot parse method");
      return;
    }
    JVMState* jvms = build_start_state(start(), tf());
    if ((jvms = cg->generate(jvms)) == NULL) {
      record_method_not_compilable("method parse failed");
      return;
    }
    GraphKit kit(jvms);

    if (!kit.stopped()) {
      // Accept return values, and transfer control we know not where.
      // This is done by a special, unique ReturnNode bound to root.
      return_values(kit.jvms());
    }

    if (kit.has_exceptions()) {
      // Any exceptions that escape from this call must be rethrown
      // to whatever caller is dynamically above us on the stack.
      // This is done by a special, unique RethrowNode bound to root.
      rethrow_exceptions(kit.transfer_exceptions_into_jvms());
    }

    // Remove clutter produced by parsing.
    if (!failing()) {
      ResourceMark rm;
      PhaseRemoveUseless pru(initial_gvn(), &for_igvn);
    }
  }

  // Note:  Large methods are capped off in do_one_bytecode().
  if (failing())  return;

  for (;;) {
    int successes = Inline_Warm();
    if (failing())  return;
    if (successes == 0)  break;
  }

  // Drain the list.
  Finish_Warm();
  if (failing())  return;
  NOT_PRODUCT( verify_graph_edges(); )

  // Now optimize
  Optimize();
  if (failing())  return;
  NOT_PRODUCT( verify_graph_edges(); )

  // Now generate code
  Code_Gen();
  if (failing())  return;

  // Check if we want to skip execution of all compiled code.  
  if (OptoNoExecute) {
    record_method_not_compilable("+OptoNoExecute");  // Flag as failed
  } else {
    TracePhase t2("install_code", &_t_registerMethod, TimeCompiler);

    const int ep_offset   = 0;
    const int vep_offset  = is_osr_compilation() ? 0 : _first_block_size;
    const int code_offset = _first_block_size;
    const int osr_offset  = is_osr_compilation() ? _first_block_size : 0;
    env()->register_method(_method, 
                           _entry_bci,
                           0/*dummy arg for C1*/, ep_offset, vep_offset, code_offset, osr_offset,
                           _code_buffer, 
                           frame_size_in_words(), _oop_map_set, 
                           &_handler_table, &_inc_table, NULL,
                           compiler,
                           true, /*has_debug_info*/
                           has_unsafe_access()
                           );
  }
}

//------------------------------Compile----------------------------------------
// Compile a runtime stub
Compile::Compile( ciEnv* ci_env,
                  TypeFunc_generator generator,
                  address stub_function,
                  const char *stub_name,
                  int is_fancy_jump,
                  bool pass_tls,
                  bool save_arg_registers,
                  bool return_pc )
  : Phase(Compiler),
    _env(ci_env),
    _log(ci_env->log()),
    _compile_id(-1),
    _save_argument_registers(save_arg_registers),
    _method(NULL),
    _stub_name(stub_name),
    _stub_function(stub_function),
    _stub_entry_point(NULL),
    _entry_bci(InvocationEntryBci),
    _initial_gvn(NULL),
    _for_igvn(NULL),
    _warm_calls(NULL),
    _subsume_loads(true),
    _failure_reason(NULL),
    _node_bundling_limit(0),
    _node_bundling_base(NULL) {

  TraceTime t1(NULL, &_t_totalCompilation, TimeCompiler, false);
  TraceTime t2(NULL, &_t_stubCompilation, TimeCompiler, false);
  CompileWrapper cw(this);
  set_print_assembly(PrintFrameConverterAssembly);
  Init(/*AliasLevel=*/ 0);
  init_tf((*generator)());

  {
    // The following is a dummy for the sake of GraphKit::gen_stub
    Unique_Node_List for_igvn(comp_arena());
    set_for_igvn(&for_igvn);  // not used, but some GraphKit guys push on this
    PhaseGVN gvn(Thread::current()->resource_area(),255);
    set_initial_gvn(&gvn);    // not significant, but GraphKit guys use it pervasively
    gvn.transform_no_reclaim(top());

    GraphKit kit;
    kit.gen_stub(stub_function, stub_name, is_fancy_jump, pass_tls, return_pc);
  }

  NOT_PRODUCT( verify_graph_edges(); )
  Code_Gen();
  if (failing())  return;


  // Entry point will be accessed using compile->stub_entry_point();
  if (code_buffer() == NULL) {
    Matcher::soft_match_failure();
  } else {
    if (PrintAssembly) tty->print_cr("### Stub::%s", stub_name);

    if (!failing()) {
      assert(_sync == 0, "no synchronization is used for runtime stubs");

      // Make the NMethod
      RuntimeStub *rs = RuntimeStub::new_runtime_stub(stub_name,
                                                      _code_buffer,
                                                      frame_size_in_words(),
                                                      _oop_map_set,
                                                      save_arg_registers);
      assert(rs != NULL && rs->is_runtime_stub(), "sanity check");

      _stub_entry_point = rs->entry_point();
    }
  }
}

#ifndef PRODUCT
void print_opto_verbose_signature( const TypeFunc *j_sig, const char *stub_name ) {
  if(PrintOpto && Verbose) {
    tty->print("%s   ", stub_name); j_sig->print_flattened(); tty->cr();
  }
}
#endif

//------------------------------Compile an adapter----------------------------
// Compile an adapter routine.
Compile::Compile( ciEnv* ci_env, ciMethod* method,
                  int /*AdapterKind*/ kind)
  : Phase(Compiler),
    _env(ci_env),
    _log(ci_env->log()),
    _compile_id(ci_env->compile_id()),
    _save_argument_registers(false),
    _method(NULL),
    _stub_name(NULL),
    _stub_function(NULL),
    _stub_entry_point(NULL),
    _entry_bci(InvocationEntryBci),
    _initial_gvn(NULL),
    _for_igvn(NULL),
    _warm_calls(NULL),
    _subsume_loads(true),
    _failure_reason(NULL),
    _node_bundling_limit(0),
    _node_bundling_base(NULL) {
  TraceTime t1(NULL, &_t_totalCompilation, TimeCompiler, false);
  TraceTime t2(NULL, &_t_adapterCompilation, TimeCompiler, false);
  CompileWrapper cw(this);
  set_print_assembly(PrintFrameConverterAssembly);
  Init(/*AliasLevel=*/ 0);
  // Check for too many adapter arguments.

  const TypeFunc* j_sig = TypeFunc::make(method);
  if (kind == ciEnv::i2c) {
    _stub_name = "i2c";
    NOT_PRODUCT( print_opto_verbose_signature(j_sig, _stub_name); )
    Generate_Interpreter_To_Compiled_Graph(j_sig);
    NOT_PRODUCT( verify_graph_edges(); )
    Code_Gen();
    if (failing())  return;
    env()->register_i2c_adapter(method,
                                _oop_map_set,
                                _code_buffer,
                                frame_size_in_words());
  } else {
    assert (kind == ciEnv::c2i, "unknown adapter kind");
    _stub_name = "c2i";
    NOT_PRODUCT( print_opto_verbose_signature(j_sig, _stub_name); )
    Generate_Compiled_To_Interpreter_Graph(j_sig, method->interpreter_entry());
    NOT_PRODUCT( verify_graph_edges(); )
    Code_Gen();
    if (failing())  return;
    env()->register_c2i_adapter(method,
                                _oop_map_set,
                                _code_buffer,
                                _first_block_size,
                                frame_size_in_words());
  }
}

//------------------------------Compile native---------------------------------
// Compile a native method
Compile::Compile( ciEnv* ci_env, C2Compiler* compiler, ciMethod* method )
                : Phase(Compiler),
                  _env(ci_env),
                  _log(ci_env->log()),
                  _compile_id(ci_env->compile_id()),
                  _save_argument_registers(false),
                  _method(method),
                  _stub_name(NULL),
                  _stub_function(NULL),
                  _stub_entry_point(NULL),
                  _entry_bci(InvocationEntryBci),
                  _initial_gvn(NULL),
                  _for_igvn(NULL),
                  _warm_calls(NULL),
                  _subsume_loads(true),
                  _failure_reason(NULL),
                  _node_bundling_limit(0),
                  _node_bundling_base(NULL) {
  TraceTime t1(NULL, &_t_totalCompilation, TimeCompiler, false);
  TraceTime t2(NULL, &_t_nativeCompilation, TimeCompiler, false);
  CompileWrapper cw(this);
  set_print_assembly(PrintOptoAssembly NOT_PRODUCT( || _method->should_print_assembly()) );
  Init(/*AliasLevel=*/ 0);
  assert( method->is_native(), "sanity check" );

#ifndef PRODUCT
  if (env()->break_at_compile()) {
    // Open the debugger when compiling this method.
    tty->print("### Breaking when compiling: ");
    method->print_short_name();
    tty->cr();
    BREAKPOINT;
  }

  if( PrintOpto ) {             // Print when we attempt compilation
    tty->print("%3d*%c", _compile_id,
                 method->flags().is_synchronized() ? 's' : ' ');
    method->print_short_name();
    tty->cr();    
  }  
#endif

  // Compute Java signature
  init_tf(TypeFunc::make(method));

  {
    // The following is a dummy for the sake of GraphKit::shared_lock, etc.
    Unique_Node_List for_igvn(comp_arena());
    set_for_igvn(&for_igvn);  // not used, but some GraphKit guys push on this
    PhaseGVN gvn(Thread::current()->resource_area(),255);
    set_initial_gvn(&gvn);    // not significant, but GraphKit guys use it pervasively
    gvn.transform_no_reclaim(top());

    GraphKit kit;
    kit.gen_native_wrapper(method);

    set_initial_gvn(NULL);
  } // End of gvn, etc.

  NOT_PRODUCT( verify_graph_edges(); )
  Code_Gen();
  if (failing())  return;

  const int ep_offset   = 0;
  const int vep_offset  = is_osr_compilation() ? 0 : _first_block_size;
  const int code_offset = _first_block_size;
  const int osr_offset  = is_osr_compilation() ? _first_block_size : 0;
  env()->register_method(method, 
                         _entry_bci,
                         0/*dummy arg for C1*/, ep_offset, vep_offset, code_offset, osr_offset,
                         _code_buffer, 
                         frame_size_in_words(), _oop_map_set, 
                           &_handler_table, &_inc_table, NULL,
                         compiler);
}

void Compile::print_codes() { 
}

//------------------------------Init-------------------------------------------
// Prepare for a single compilation
void Compile::Init(int aliaslevel) {
  _unique  = 0;
  _regalloc = NULL;

  _tf      = NULL;  // filled in later
  _top     = NULL;  // cached later
  _matcher = NULL;  // filled in later
  _cfg     = NULL;  // filled in later

  set_24_bit_selection_and_mode(Use24BitFP, false);

  // Capture status of JVMPI once for the compilation
  _need_jvmpi_compiled_method_event = jvmpi::is_event_enabled(JVMPI_EVENT_COMPILED_METHOD_LOAD);
  _need_jvmpi_method_entry_event    = jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY);
  _need_jvmpi_method_entry2_event   = jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY2);
  _need_jvmpi_method_exit_event     = jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_EXIT);

  // Globally visible Nodes
  // First set TOP to NULL to give safe behavior during creation of RootNode
  set_cached_top_node(NULL);
  set_root(new RootNode());
  // Now that you have a Root to point to, create the real TOP
  set_cached_top_node( new (1) ConNode(Type::TOP) );

  // Create Debug Information Recorder to record scopes, oopmaps, etc.
  set_recorder(new DebugInformationRecorder(new OopRecorder(env()->arena())));

  _sync = 0;
  set_has_split_ifs(false);
  set_has_loops(has_method() && method()->has_loops()); // first approximation
  _deopt_happens = true;  // start out assuming the worst
  _trap_can_recompile = false;  // no traps emitted yet
  _major_progress = true; // start out assuming good things will happen
  set_has_unsafe_access(false);
  Copy::zero_to_bytes(_trap_hist, sizeof(_trap_hist));
  set_decompile_count(0);

  // Compilation level related initialization
  if (env()->comp_level() == CompLevel_fast_compile) {
    set_num_loop_opts(Tier1LoopOptsCount);
    set_do_inlining(Tier1Inline);
    set_max_inline_size(Tier1MaxInlineSize);
    set_freq_inline_size(Tier1FreqInlineSize);
    set_do_scheduling(false);
    set_do_count_invocations(Tier1CountInvocations);
    set_do_method_data_update(Tier1UpdateMethodData);
  } else {
    assert(env()->comp_level() == CompLevel_full_optimization, "unknown comp level");
    set_num_loop_opts(LoopOptsCount);
    set_do_inlining(Inline);
    set_max_inline_size(MaxInlineSize);
    set_freq_inline_size(FreqInlineSize);
    set_do_scheduling(OptoScheduling);  
    set_do_count_invocations(false);
    set_do_method_data_update(false);
  }

  // // -- Initialize types before each compile --
  // // Update cached type information
  // if( _method && _method->constants() ) 
  //   Type::update_loaded_types(_method, _method->constants());

  // Init alias_type map.
  _AliasLevel = aliaslevel;
  const int grow_ats = 16;
  _max_alias_types = grow_ats;
  _alias_types   = NEW_ARENA_ARRAY(comp_arena(), AliasType*, grow_ats);
  AliasType* ats = NEW_ARENA_ARRAY(comp_arena(), AliasType,  grow_ats);
  Copy::zero_to_bytes(ats, sizeof(AliasType)*grow_ats);
  {
    for (int i = 0; i < grow_ats; i++)  _alias_types[i] = &ats[i];
  }
  // Initialize the first few types.
  _alias_types[AliasIdxTop]->Init(AliasIdxTop, NULL);
  _alias_types[AliasIdxBot]->Init(AliasIdxBot, TypePtr::BOTTOM);
  _alias_types[AliasIdxRaw]->Init(AliasIdxRaw, TypeRawPtr::BOTTOM);
  _num_alias_types = AliasIdxRaw+1;
  // Zero out the alias type cache.
  Copy::zero_to_bytes(_alias_cache, sizeof(_alias_cache));
  // A NULL adr_type hits in the cache right away.  Preload the right answer.
  probe_alias_cache(NULL)->_index = AliasIdxTop;

  _Method_invoke = NULL;
  _invoke_name = NULL;
  _MethodAccessorImpl = NULL;
  _intrinsics = NULL;
  register_library_intrinsics();
}

//---------------------------init_start----------------------------------------
// Install the StartNode on this compile object.
void Compile::init_start(StartNode* s) {
  if (failing())
    return; // already failing
  assert(s == start(), "");
}

StartNode* Compile::start() const {
  assert(!failing(), "");
  for (DUIterator_Fast imax, i = root()->fast_outs(imax); i < imax; i++) {
    StartNode* start = root()->fast_out(i)->is_Start();
    if( start ) 
      return start;
  }
  ShouldNotReachHere();
  return NULL;
}

//----------------------set_cached_top_node------------------------------------
// Install the cached top node, and make sure Node::is_top works correctly.
void Compile::set_cached_top_node(Node* tn) {
  if (tn != NULL)  verify_top(tn);
  Node* old_top = _top;
  _top = tn;
  // Calling Node::setup_is_top allows the nodes the chance to adjust
  // their _out arrays.
  if (_top != NULL)     _top->setup_is_top();
  if (old_top != NULL)  old_top->setup_is_top();
  assert(_top == NULL || top()->is_top(), "");
}


#ifndef PRODUCT
void Compile::verify_top(Node* tn) const {
  if (tn != NULL) {
    assert(tn->is_Con(), "top node must be a constant");
    assert(((ConNode*)tn)->type() == Type::TOP, "top node must have correct type");
    assert(tn->in(0) != NULL, "must have live top node");
  }
}
#endif


//--------------------------allow_range_check_smearing-------------------------
// Gating condition for coalescing similar range checks.
// Sometimes we try 'speculatively' replacing a series of a range checks by a
// single covering check that is at least as strong as any of them.
// If the optimization succeeds, the simplified (strengthened) range check
// will always succeed.  If it fails, we will deopt, and then give up
// on the optimization.
bool Compile::allow_range_check_smearing() const {
  // If this method has already thrown a range-check,
  // assume it was because we already tried range smearing
  // and it failed.
  uint already_trapped = trap_count(Deoptimization::Reason_range_check);
  return !already_trapped;
}


//------------------------------flatten_alias_type-----------------------------
const TypePtr *Compile::flatten_alias_type( const TypePtr *tj ) const {
  int offset = tj->offset();
  TypePtr::PTR ptr = tj->ptr();

  // Process weird unsafe references.
  if (offset == Type::OffsetBot && (tj->isa_instptr() /*|| tj->isa_klassptr()*/)) {
    assert(InlineUnsafeOps, "indeterminate pointers come only from unsafe ops");
    tj = TypeOopPtr::BOTTOM;
    ptr = tj->ptr();
    offset = tj->offset();
  }

  // Array pointers need some flattening
  const TypeAryPtr *ta = tj->isa_aryptr();
  if( ta && _AliasLevel >= 2 ) {
    // For arrays indexed by constant indices, we flatten the alias
    // space to include all of the array body.  Only the header, klass
    // and array length can be accessed un-aliased.
    if( offset != Type::OffsetBot ) {
      if( ta->const_oop() ) { // methodDataOop or methodOop
        offset = Type::OffsetBot;   // Flatten constant access into array body
        tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),ta->ary(),ta->klass(),false,Type::OffsetBot);
      } else if( offset == arrayOopDesc::length_offset_in_bytes() ) {
        // range is OK as-is.
        tj = ta = TypeAryPtr::RANGE; 
      } else if( offset == oopDesc::klass_offset_in_bytes() ) {
        tj = TypeInstPtr::KLASS; // all klass loads look alike
        ta = TypeAryPtr::RANGE; // generic ignored junk
        ptr = TypePtr::BotPTR;
      } else if( offset == oopDesc::mark_offset_in_bytes() ) {
        tj = TypeInstPtr::MARK;
        ta = TypeAryPtr::RANGE; // generic ignored junk
        ptr = TypePtr::BotPTR;
      } else {                  // Random constant offset into array body 
        offset = Type::OffsetBot;   // Flatten constant access into array body
        tj = ta = TypeAryPtr::make(ptr,ta->ary(),ta->klass(),false,Type::OffsetBot);
      }
    }
    // Arrays of fixed size alias with arrays of unknown size.
    if (ta->size() != TypeInt::POS) {
      const TypeAry *tary = TypeAry::make(ta->elem(), TypeInt::POS);
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,ta->klass(),false,offset);
    }
    // Arrays of known objects become arrays of unknown objects.
    if (ta->elem()->isa_oopptr() && ta->elem() != TypeInstPtr::BOTTOM) {
      const TypeAry *tary = TypeAry::make(TypeInstPtr::BOTTOM, ta->size());
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,NULL,false,offset);
    }
    // Arrays of bytes and of booleans both use 'bastore' and 'baload' so
    // cannot be distinguished by bytecode alone.
    if (ta->elem() == TypeInt::BOOL) {
      const TypeAry *tary = TypeAry::make(TypeInt::BYTE, ta->size());
      ciKlass* aklass = ciTypeArrayKlass::make(T_BYTE);
      tj = ta = TypeAryPtr::make(ptr,ta->const_oop(),tary,aklass,false,offset);
    }
    // During the 2nd round of IterGVN, NotNull castings are removed.
    // Make sure the Bottom and NotNull variants alias the same.
    // Also, make sure exact and non-exact variants alias the same.
    if( ptr == TypePtr::NotNull || ta->klass_is_exact() ) {
      if (ta->const_oop()) {
        tj = ta = TypeAryPtr::make(TypePtr::Constant,ta->const_oop(),ta->ary(),ta->klass(),false,offset);
      } else {
        tj = ta = TypeAryPtr::make(TypePtr::BotPTR,ta->ary(),ta->klass(),false,offset);
      }
    }
  }

  // Oop pointers need some flattening
  const TypeInstPtr *to = tj->isa_instptr();
  if( to && _AliasLevel >= 2 && to != TypeOopPtr::BOTTOM ) {
    if( ptr == TypePtr::Constant ) {
      // No constant oop pointers (such as Strings); they alias with
      // unknown strings.
      tj = to = TypeInstPtr::make(TypePtr::BotPTR,to->klass(),false,0,offset);
    } else if( ptr == TypePtr::NotNull || to->klass_is_exact() ) {
      // During the 2nd round of IterGVN, NotNull castings are removed.
      // Make sure the Bottom and NotNull variants alias the same.
      // Also, make sure exact and non-exact variants alias the same.
      tj = to = TypeInstPtr::make(TypePtr::BotPTR,to->klass(),false,0,offset);
    }
    // Canonicalize the holder of this field
    ciInstanceKlass *k = to->klass()->as_instance_klass();
    if (offset >= 0 && offset < oopDesc::header_size() * wordSize) {
      // First handle header references such as a LoadKlassNode, even if the
      // object's klass is unloaded at compile time (4965979).
      tj = to = TypeInstPtr::make(TypePtr::BotPTR, env()->Object_klass(), false, NULL, offset);
    } else if (offset < 0 || offset >= k->size_helper() * wordSize) {
      to = NULL;
      tj = TypeOopPtr::BOTTOM;
      offset = tj->offset();
    } else {
      ciInstanceKlass *canonical_holder = k->get_canonical_holder(offset);
      if (!k->equals(canonical_holder) || tj->offset() != offset) {
        tj = to = TypeInstPtr::make(TypePtr::BotPTR, canonical_holder, false, NULL, offset);
      }
    }
  }

  // Klass pointers to object array klasses need some flattening
  const TypeKlassPtr *tk = tj->isa_klassptr();
  if( tk ) {
    // If we are referencing a field within a Klass, we need
    // to assume the worst case of an Object.  Both exact and
    // inexact types must flatten to the same alias class.
    // Since the flattened result for a klass is defined to be 
    // precisely java.lang.Object, use a constant ptr.
    if ( offset == Type::OffsetBot || offset < sizeof(Klass) ) {
      tj = tk = TypeKlassPtr::make(TypePtr::Constant,
                                   TypeKlassPtr::OBJECT->klass(),
                                   offset);
    }

    ciKlass* klass = tk->klass();
    if( klass->is_obj_array_klass() ) {
      ciKlass* k = TypeAryPtr::OOPS->klass();
      if( !k || !k->is_loaded() )                  // Only fails for some -Xcomp runs
        k = TypeInstPtr::BOTTOM->klass();
      tj = tk = TypeKlassPtr::make( TypePtr::NotNull, k, offset );
    }

    // Check for precise loads from the primary supertype array and force them
    // to the supertype cache alias index.  Check for generic array loads from
    // the primary supertype array and also force them to the supertype cache
    // alias index.  Since the same load can reach both, we need to merge
    // these 2 disparate memories into the same alias class.  Since the
    // primary supertype array is read-only, there's no chance of confusion
    // where we bypass an array load and an array store.
    uint off2 = offset - Klass::primary_supers_offset_in_bytes();
    if( offset == Type::OffsetBot ||
        off2 < Klass::primary_super_limit()*wordSize ) {
      offset = sizeof(oopDesc) +Klass::secondary_super_cache_offset_in_bytes();
      tj = tk = TypeKlassPtr::make( TypePtr::NotNull, tk->klass(), offset );
    }
  }

  // Flatten all Raw pointers together.
  if (tj->base() == Type::RawPtr)
    tj = TypeRawPtr::BOTTOM;

  if (tj->base() == Type::AnyPtr)
    tj = TypePtr::BOTTOM;      // An error, which the caller must check for.

  // Flatten all to bottom for now
  switch( _AliasLevel ) {
  case 0: 
    tj = TypePtr::BOTTOM; 
    break;
  case 1:                       // Flatten to: oop, static, field or array
    switch (tj->base()) {
    //case Type::AryPtr: tj = TypeAryPtr::RANGE;    break;
    case Type::RawPtr:   tj = TypeRawPtr::BOTTOM;   break;
    case Type::AryPtr:   // do not distinguish arrays at all
    case Type::InstPtr:  tj = TypeInstPtr::BOTTOM;  break;
    case Type::KlassPtr: tj = TypeKlassPtr::OBJECT; break;
    case Type::AnyPtr:   tj = TypePtr::BOTTOM;      break;  // caller checks it
    default: ShouldNotReachHere();
    }
    break;
  case 2:                       // No collasping at level 2; keep all splits
    break;
  default:
    Unimplemented();
  } 

  offset = tj->offset();
  assert( offset != Type::OffsetTop, "Offset has fallen from constant" );
  
  assert( (offset != Type::OffsetBot && tj->base() != Type::AryPtr) ||
          (offset == Type::OffsetBot && tj->base() == Type::AryPtr) ||
          (offset == Type::OffsetBot && tj == TypeOopPtr::BOTTOM) ||
          (offset == Type::OffsetBot && tj == TypePtr::BOTTOM) ||
          (offset == oopDesc::mark_offset_in_bytes() && tj->base() == Type::AryPtr) ||
          (offset == oopDesc::klass_offset_in_bytes() && tj->base() == Type::AryPtr) ||
          (offset == arrayOopDesc::length_offset_in_bytes() && tj->base() == Type::AryPtr)  , 
          "For oops, klasses, raw offset must be constant; for arrays the offset is never known" );
  assert( tj->ptr() != TypePtr::TopPTR &&
          tj->ptr() != TypePtr::AnyNull &&          
          tj->ptr() != TypePtr::Null, "No imprecise addresses" );
//    assert( tj->ptr() != TypePtr::Constant ||
//            tj->base() == Type::RawPtr ||
//            tj->base() == Type::KlassPtr, "No constant oop addresses" );

  return tj;
}

//---------------------------------print_on------------------------------------
#ifndef PRODUCT
void Compile::AliasType::print_on(outputStream* st) {
  if (index() < 10)
        st->print("@ <%d> ", index());
  else  st->print("@ <%d>",  index());
  st->print(is_rewritable() ? "   " : " RO");
  int offset = adr_type()->offset();
  if (offset == Type::OffsetBot)
        st->print(" +any");
  else  st->print(" +%-3d", offset);
  st->print(" in ");
  adr_type()->dump();
  const TypeOopPtr* tjp = adr_type()->isa_oopptr();
  if (field() != NULL && tjp) {
    if (tjp->klass()  != field()->holder() ||
        tjp->offset() != field()->offset_in_bytes()) {
      st->print(" != ");
      field()->print();
      st->print(" ***");
    }
  }
}

void print_alias_types() {
  Compile* C = Compile::current();
  tty->print_cr("--- Alias types, AliasIdxBot .. %d", C->num_alias_types()-1);
  for (int idx = Compile::AliasIdxBot; idx < C->num_alias_types(); idx++) {
    C->alias_type(idx)->print_on(tty);
    tty->cr();
  }
}
#endif


//----------------------------probe_alias_cache--------------------------------
Compile::AliasCacheEntry* Compile::probe_alias_cache(const TypePtr* adr_type) {
  intptr_t key = (intptr_t) adr_type;
  key ^= key >> logAliasCacheSize;
  return &_alias_cache[key & right_n_bits(logAliasCacheSize)];
}


//-----------------------------grow_alias_types--------------------------------
void Compile::grow_alias_types() {
  const int old_ats  = _max_alias_types; // how many before?
  const int new_ats  = old_ats;          // how many more?
  const int grow_ats = old_ats+new_ats;  // how many now?
  _max_alias_types = grow_ats;
  _alias_types =  REALLOC_ARENA_ARRAY(comp_arena(), AliasType*, _alias_types, old_ats, grow_ats);
  AliasType* ats =    NEW_ARENA_ARRAY(comp_arena(), AliasType, new_ats);
  Copy::zero_to_bytes(ats, sizeof(AliasType)*new_ats);
  for (int i = 0; i < new_ats; i++)  _alias_types[old_ats+i] = &ats[i];
}


//--------------------------------find_alias_type------------------------------
Compile::AliasType* Compile::find_alias_type(const TypePtr* adr_type, bool no_create) {
  if (_AliasLevel == 0)
    return alias_type(AliasIdxBot);

  AliasCacheEntry* ace = probe_alias_cache(adr_type);
  if (ace->_adr_type == adr_type) {
    return alias_type(ace->_index);
  }

  // Handle special cases.
  if (adr_type == NULL)             return alias_type(AliasIdxTop);
  if (adr_type == TypePtr::BOTTOM)  return alias_type(AliasIdxBot);

  // Do it the slow way.
  const TypePtr* flat = flatten_alias_type(adr_type);

#ifdef ASSERT
  assert(flat == flatten_alias_type(flat), "idempotent");
  assert(flat != TypePtr::BOTTOM,     "cannot alias-analyze an untyped ptr");
  if (flat->isa_oopptr() && !flat->isa_klassptr()) {
    const TypeOopPtr* foop = flat->is_oopptr();
    const TypePtr* xoop = foop->cast_to_exactness(!foop->klass_is_exact())->is_ptr();
    assert(foop == flatten_alias_type(xoop), "exactness must not affect alias type");
  }
  assert(flat == flatten_alias_type(flat), "exact bit doesn't matter");
#endif

  int idx = AliasIdxTop;
  for (int i = 0; i < num_alias_types(); i++) {
    if (alias_type(i)->adr_type() == flat) {
      idx = i;
      break;
    }
  }

  if (idx == AliasIdxTop) {
    if (no_create)  return NULL;
    // Grow the array if necessary.
    if (_num_alias_types == _max_alias_types)  grow_alias_types();
    // Add a new alias type.
    idx = _num_alias_types++;
    _alias_types[idx]->Init(idx, flat);
    if (flat == TypeInstPtr::KLASS)  alias_type(idx)->set_rewritable(false);
    if (flat == TypeAryPtr::RANGE)   alias_type(idx)->set_rewritable(false);
    if (flat->isa_instptr()) {
      if (flat->offset() == java_lang_Class::klass_offset_in_bytes()
          && flat->is_instptr()->klass() == env()->Class_klass())
        alias_type(idx)->set_rewritable(false);
    }
    if (flat->isa_klassptr()) {
      if (flat->offset() == Klass::super_check_offset_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
      if (flat->offset() == Klass::modifier_flags_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
      if (flat->offset() == Klass::access_flags_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
      if (flat->offset() == Klass::java_mirror_offset_in_bytes() + (int)sizeof(oopDesc))
        alias_type(idx)->set_rewritable(false);
    }
    // %%% (We would like to finalize JavaThread::threadObj_offset(),
    // but the base pointer type is not distinctive enough to identify
    // references into JavaThread.)

    // Check for final instance fields.
    const TypeInstPtr* tinst = flat->isa_instptr();
    if (tinst && tinst->offset() >= oopDesc::header_size() * wordSize) {
      ciInstanceKlass *k = tinst->klass()->as_instance_klass();
      ciField* field = k->get_field_by_offset(tinst->offset(), false);
      // Set field() and is_rewritable() attributes.
      if (field != NULL)  alias_type(idx)->set_field(field);
    }
    const TypeKlassPtr* tklass = flat->isa_klassptr();
    // Check for final static fields.
    if (tklass && tklass->klass()->is_instance_klass()) {
      ciInstanceKlass *k = tklass->klass()->as_instance_klass();
      ciField* field = k->get_field_by_offset(tklass->offset(), true);
      // Set field() and is_rewritable() attributes.
      if (field != NULL)   alias_type(idx)->set_field(field);
    }
  }

  // Fill the cache for next time.
  ace->_adr_type = adr_type;
  ace->_index    = idx;
  assert(alias_type(adr_type) == alias_type(idx),  "type must be installed");

  // Might as well try to fill the cache for the flattened version, too.
  AliasCacheEntry* face = probe_alias_cache(flat);
  if (face->_adr_type == NULL) {
    face->_adr_type = flat;
    face->_index    = idx;
    assert(alias_type(flat) == alias_type(idx), "flat type must work too");
  }

  return alias_type(idx);
}


Compile::AliasType* Compile::alias_type(ciField* field) {
  const TypeOopPtr* t;
  if (field->is_static())
    t = TypeKlassPtr::make(field->holder());
  else
    t = TypeOopPtr::make_from_klass_raw(field->holder());
  AliasType* atp = alias_type(t->add_offset(field->offset_in_bytes()));
  assert(field->is_final() == !atp->is_rewritable(), "must get the rewritable bits correct");
  return atp;
}


//------------------------------have_alias_type--------------------------------
bool Compile::have_alias_type(const TypePtr* adr_type) {
  AliasCacheEntry* ace = probe_alias_cache(adr_type);
  if (ace->_adr_type == adr_type) {
    return true;
  }

  // Handle special cases.
  if (adr_type == NULL)             return true;
  if (adr_type == TypePtr::BOTTOM)  return true;

  return find_alias_type(adr_type, true) != NULL;
}

//-----------------------------must_alias--------------------------------------
// True if all values of the given address type are in the given alias category.
bool Compile::must_alias(const TypePtr* adr_type, int alias_idx) {
  if (alias_idx == AliasIdxBot)         return true;  // the universal category
  if (adr_type == NULL)                 return true;  // NULL serves as TypePtr::TOP
  if (alias_idx == AliasIdxTop)         return false; // the empty category
  if (adr_type->base() == Type::AnyPtr) return false; // TypePtr::BOTTOM or its twins

  // the only remaining possible overlap is identity
  int adr_idx = get_alias_index(adr_type);
  assert(adr_idx != AliasIdxBot && adr_idx != AliasIdxTop, "");
  assert(adr_idx == alias_idx ||
         (alias_type(alias_idx)->adr_type() != TypeOopPtr::BOTTOM
          && adr_type                       != TypeOopPtr::BOTTOM),
         "should not be testing for overlap with an unsafe pointer");
  return adr_idx == alias_idx;
}

//------------------------------can_alias--------------------------------------
// True if any values of the given address type are in the given alias category.
bool Compile::can_alias(const TypePtr* adr_type, int alias_idx) {
  if (alias_idx == AliasIdxTop)         return false; // the empty category
  if (adr_type == NULL)                 return false; // NULL serves as TypePtr::TOP
  if (alias_idx == AliasIdxBot)         return true;  // the universal category
  if (adr_type->base() == Type::AnyPtr) return true;  // TypePtr::BOTTOM or its twins

  // the only remaining possible overlap is identity
  int adr_idx = get_alias_index(adr_type);
  assert(adr_idx != AliasIdxBot && adr_idx != AliasIdxTop, "");
#if 0
  // This assert triggers with MemBarVolatileNodes generated by
  // inline_unsafe_access.  It is apparently harmless there though. Is
  // there a way to let only this use through? -dl
  assert(adr_idx == alias_idx ||
         (alias_type(alias_idx)->adr_type() != TypeOopPtr::BOTTOM
          && adr_type                       != TypeOopPtr::BOTTOM),
         "should not be testing for overlap with an unsafe pointer");
#endif
  return adr_idx == alias_idx;
}



//---------------------------pop_warm_call-------------------------------------
WarmCallInfo* Compile::pop_warm_call() {
  WarmCallInfo* wci = _warm_calls;
  if (wci != NULL)  _warm_calls = wci->remove_from(wci);
  return wci;
}

//----------------------------Inline_Warm--------------------------------------
int Compile::Inline_Warm() {
  // If there is room, try to inline some more warm call sites.
  // %%% Do a graph index compaction pass when we think we're out of space?
  if (!InlineWarmCalls)  return 0;

  int calls_made_hot = 0;
  int room_to_grow   = NodeCountInliningCutoff - unique();
  int amount_to_grow = MIN2(room_to_grow, (int)NodeCountInliningStep);
  int amount_grown   = 0;
  WarmCallInfo* call;
  while (amount_to_grow > 0 && (call = pop_warm_call()) != NULL) {
    int est_size = call->size();
    if (est_size > (room_to_grow - amount_grown)) {
      // This one won't fit anyway.  Get rid of it.
      call->make_cold();
      continue;
    }
    call->make_hot();
    calls_made_hot++;
    amount_grown   += est_size;
    amount_to_grow -= est_size;
  }

  if (calls_made_hot > 0)  set_major_progress();
  return calls_made_hot;
}


//----------------------------Finish_Warm--------------------------------------
void Compile::Finish_Warm() {
  if (!InlineWarmCalls)  return;
  if (failing())  return;
  if (warm_calls() == NULL)  return;

  // Clean up loose ends, if we are out of space for inlining.
  WarmCallInfo* call;
  while ((call = pop_warm_call()) != NULL) {
    call->make_cold();
  }
}


//------------------------------Optimize---------------------------------------
// Given a graph, optimize it.
void Compile::Optimize() {
  TracePhase t1("optimizer", &_t_optimizer, true);

#ifndef PRODUCT
  if (env()->break_at_compile()) {
    BREAKPOINT;
  }

#endif

  ResourceMark rm;
  int          loop_opts_cnt;

  NOT_PRODUCT( verify_graph_edges(); )

 {
  // Iterative Global Value Numbering, including ideal transforms
  // Initialize IterGVN with types and values from parse-time GVN
  PhaseIterGVN igvn(initial_gvn());
  {
    TracePhase t2("iterGVN", &_t_iterGVN, TimeCompiler);
    igvn.optimize();
  }
  if (failing())  return;

  // Loop transforms on the ideal graph.  Range Check Elimination,
  // peeling, unrolling, etc.

  // Set loop opts counter
  loop_opts_cnt = num_loop_opts();
  if((loop_opts_cnt > 0) && (has_loops() || has_split_ifs())) {
    {
      TracePhase t2("idealLoop", &_t_idealLoop, true);
      PhaseIdealLoop ideal_loop( igvn, NULL, true );
      loop_opts_cnt--;                            // Decrement iteration counter for loop opts
      if (failing())  return;
    }
    if(major_progress() && (loop_opts_cnt > 0)) { // Do a loop-unroll pass before CCP
      TracePhase t3("idealLoop", &_t_idealLoop, true);
      PhaseIdealLoop ideal_loop( igvn, NULL, false );
      loop_opts_cnt--;                            // Decrement iteration counter for loop opts
    }
  }
  if (failing())  return;

  // Conditional Constant Propagation;
  PhaseCCP ccp( &igvn ); 
  assert( true, "Break here to ccp.dump_nodes_and_types(_root,999,1)");
  {
    TracePhase t2("ccp", &_t_ccp, true);
    ccp.do_transform();
  }
  assert( true, "Break here to ccp.dump_old2new_map()");
    
  // Iterative Global Value Numbering, including ideal transforms
  {
    TracePhase t2("iterGVN2", &_t_iterGVN2, TimeCompiler);
    igvn = ccp;
    igvn.optimize();
  }
  if (failing())  return;

  // Loop transforms on the ideal graph.  Range Check Elimination,
  // peeling, unrolling, etc.
  if(loop_opts_cnt > 0) {
    debug_only( int cnt = 0; );
    while(major_progress() && (loop_opts_cnt > 0)) {      
      TracePhase t2("idealLoop", &_t_idealLoop, true);
      assert( cnt++ < 40, "infinite cycle in loop optimization" );
      PhaseIdealLoop ideal_loop( igvn, NULL, true );
      loop_opts_cnt--;                          // Decrement iteration counter for loop opts
      if (failing())  return;
    }
  }
 } // (End scope of igvn; run destructor if necessary for asserts.)

  // A method with only infinite loops has no edges entering loops from root
  {
    TracePhase t2("graphReshape", &_t_graphReshaping, TimeCompiler);
    if (final_graph_reshaping()) {
      assert(failing(), "must bail out w/ explicit message");
      return;
    }
  }
}


//------------------------------Code_Gen---------------------------------------
// Given a graph, generate code for it
void Compile::Code_Gen() {
  if (failing())  return;

  // Perform instruction selection.  You might think we could reclaim Matcher
  // memory PDQ, but actually the Matcher is used in generating spill code.
  // Internals of the Matcher (including some VectorSets) must remain live
  // for awhile - thus I cannot reclaim Matcher memory lest a VectorSet usage
  // set a bit in reclaimed memory.

  // In debug mode can dump m._nodes.dump() for mapping of ideal to machine
  // nodes.  Mapping is only valid at the root of each matched subtree.
  NOT_PRODUCT( verify_graph_edges(); )

  Node_List proj_list;
  Matcher m(proj_list);
  _matcher = &m;
  {
    TracePhase t2("matcher", &_t_matcher, true);
    m.match();
  }
  // In debug mode can dump m._nodes.dump() for mapping of ideal to machine
  // nodes.  Mapping is only valid at the root of each matched subtree.
  NOT_PRODUCT( verify_graph_edges(); )

  // If you have too many nodes, or if matching has failed, bail out
  check_node_count(0, "out of nodes matching instructions");
  if (failing())  return;

  // Build a proper-looking CFG
  PhaseCFG cfg(node_arena(), root(), m);
  _cfg = &cfg;
  {
    TracePhase t2("scheduler", &_t_scheduler, TimeCompiler);
    cfg.Dominators();
    if (failing())  return;

    cfg.Find_Inner_Loops();
    NOT_PRODUCT( verify_graph_edges(); )

    cfg.Estimate_Block_Frequency();
    cfg.GlobalCodeMotion(m,unique(),proj_list);
    if (failing())  return;
    NOT_PRODUCT( verify_graph_edges(); )

    debug_only( cfg.verify(); )
  }
  NOT_PRODUCT( verify_graph_edges(); )

  PhaseChaitin regalloc(unique(),cfg,m);
  _regalloc = &regalloc;
  {
    TracePhase t2("regalloc", &_t_registerAllocation, true);
    // Perform any platform dependent preallocation actions.  This is used,
    // for example, to avoid taking an implicit null pointer exception
    // using the frame pointer on win95.
    _regalloc->pd_preallocate_hook();

    // Perform register allocation.  After Chaitin, use-def chains are
    // no longer accurate (at spill code) and so must be ignored.
    // Node->LRG->reg mappings are still accurate.
    _regalloc->Register_Allocate();

    // Bail out if the allocator builds too many nodes
    if (failing())  return;
  }

  // Prior to register allocation we kept empty basic blocks in case the
  // the allocator needed a place to spill.  After register allocation we 
  // are not adding any new instructions.  If any basic block is empty, we 
  // can now safely remove it.
  {
    TracePhase t2("removeEmpty", &_t_removeEmptyBlocks, TimeCompiler);
    cfg.RemoveEmpty();
  }

  // Perform any platform dependent postallocation verifications.
  debug_only( _regalloc->pd_postallocate_verify_hook(); )

  // Apply peephole optimizations
  if( OptoPeephole ) {
    TracePhase t2("peephole", &_t_peephole, TimeCompiler);
    PhasePeephole peep( _regalloc, cfg);
    peep.do_transform();
  }

  // Convert Nodes to instruction bits in a buffer
  {
    // %%%% workspace merge brought two timers together for one job
    TracePhase t2a("output", &_t_output, true);
    TraceTime t2b(NULL, &_t_codeGeneration, TimeCompiler, false);
    Output();
  }

  // He's dead, Jim.
  _cfg     = (PhaseCFG*)0xdeadbeef;
  _regalloc = (PhaseChaitin*)0xdeadbeef;
}


//------------------------------dump_asm---------------------------------------
// Dump formatted assembly
#ifndef PRODUCT
void Compile::dump_asm(int *pcs, uint pc_limit) { 
  tty->print_cr("#");
  tty->print("#  ");  _tf->dump();  tty->cr();
  tty->print_cr("#");

  // For all blocks
  int pc = 0x0;                 // Program counter
  char starts_bundle = ' ';
  _regalloc->dump_frame();

  Node *n = NULL;
  for( uint i=0; i<_cfg->_num_blocks; i++ ) {
    Block *b = _cfg->_blocks[i];
    n = b->_nodes[0];
    if (pcs && n->_idx < pc_limit)
      tty->print("%3.3x   ", pcs[n->_idx]);
    else
      tty->print("      ");
    b->dump_head( &_cfg->_bbs );

    // For all instructions
    Node *delay = NULL;
    for( uint j = 0; j<b->_nodes.size(); j++ ) {
      n = b->_nodes[j];
      if (valid_bundle_info(n)) {
        Bundle *bundle = node_bundling(n);
        if (bundle->used_in_unconditional_delay()) {
          delay = n;
          continue;
        }
        if (bundle->starts_bundle())
          starts_bundle = '+';
      }

      if( !n->is_Region() &&    // Dont print in the Assembly
          !n->is_Phi() &&       // a few noisely useless nodes
          !n->is_Proj() &&
          !n->is_Catch() &&     // Would be nice to print exception table targets
          !n->is_MergeMem() &&  // Not very interesting
          !n->is_top() &&       // Debug info table constants
          !(n->is_Con() && !n->is_Mach())// Debug info table constants
          ) {
        if (pcs && n->_idx < pc_limit)
          tty->print("%3.3x", pcs[n->_idx]);
        else
          tty->print("   ");
        tty->print(" %c ", starts_bundle);
        starts_bundle = ' ';
        tty->print("\t");
        n->format(_regalloc);
        tty->cr();
      }

      // If we have an instruction with a delay slot, and have seen a delay,
      // then back up and print it
      if (valid_bundle_info(n) && node_bundling(n)->use_unconditional_delay()) {
        assert(delay != NULL, "no unconditional delay instruction");
        if (node_bundling(delay)->starts_bundle())
          starts_bundle = '+';
        if (pcs && n->_idx < pc_limit)
          tty->print("%3.3x", pcs[n->_idx]);
        else
          tty->print("   ");
        tty->print(" %c ", starts_bundle);
        starts_bundle = ' ';
        tty->print("\t");
        delay->format(_regalloc);
        tty->print_cr("");
        delay = NULL;
      }

      // Dump the exception table as well
      if( n->is_Catch() && (Verbose || WizardMode) ) {
        // Print the exception table for this offset
        _handler_table.print_subtable_for(pc);
      }
    }
    if (pcs && n->_idx < pc_limit)
      tty->print_cr("%3.3x", pcs[n->_idx]);
    else
      tty->print_cr("");

    assert(delay == NULL, "no unconditional delay branch");

  } // End of per-block dump
  tty->print_cr("");
}
#endif

// register information defined by ADLC
extern const char register_save_policy[];
extern const int  register_save_type[];


// !!!!! The following method initializes the runtime stubs used by compiler2.
// The code here should be moved to an initialization routine in C2Compiler.
void compiler2_init() {
  // No initialization needed if interpreter only
  if (Arguments::mode() == Arguments::_int) return;

  // Check assumptions used while running ADLC
  Compile::adlc_verification();

  // Check that runtime and architecture description agree on callee-saved-floats
  bool callee_saved_floats = false;
  for( OptoReg::Name i=OptoReg::Name(0); i<OptoReg::Name(_last_Mach_Reg); i = OptoReg::add(i,1) ) {
    // Is there a callee-saved float or double?
    if( register_save_policy[i] == 'E' /* callee-saved */ && 
       (register_save_type[i] == Op_RegF || register_save_type[i] == Op_RegD) ) {
      callee_saved_floats = true;
    }
  }

  Compile::pd_compiler2_init();

  int num_mods = 0;
  {
    MutexLocker locker(Compile_lock);
    num_mods = SystemDictionary::number_of_modifications();
  }

  OptoRuntime::generate_arraycopy_stubs();

  JavaThread *thread = JavaThread::current();
  JNIEnv* jni_env = thread->jni_environment();
  { ThreadToNativeFromVM ttn(thread);

    HandleMark  handle_mark(thread);

    ciEnv ci_env(jni_env, num_mods, false, CompLevel_full_optimization);

    // -- Also Initialize types before each compile --
    // Compile all the Runtime stubs
    OptoRuntime::generate(&ci_env);
  }
}

//------------------------------Final_Reshape_Counts---------------------------
// This class defines counters to help identify when a method 
// may/must be executed using hardware with only 24-bit precision.
struct Final_Reshape_Counts : public StackObj {
  int  _call_count;             // count non-inlined 'common' calls 
  int  _float_count;            // count float ops requiring 24-bit precision
  int  _double_count;           // count double ops requiring more precision
  int  _java_call_count;        // count non-inlined 'java' calls 
  VectorSet _visited;           // Visitation flags
  Node_List _tests;             // Set of IfNodes & PCTableNodes

  Final_Reshape_Counts() : 
    _call_count(0), _float_count(0), _double_count(0), _java_call_count(0),
    _visited( Thread::current()->resource_area() ) { }

  void inc_call_count  () { _call_count  ++; }
  void inc_float_count () { _float_count ++; }
  void inc_double_count() { _double_count++; }
  void inc_java_call_count() { _java_call_count++; }

  int  get_call_count  () const { return _call_count  ; }
  int  get_float_count () const { return _float_count ; }
  int  get_double_count() const { return _double_count; }
  int  get_java_call_count() const { return _java_call_count; }
};

static bool oop_offset_is_sane(const TypeInstPtr* tp) {
  ciInstanceKlass *k = tp->klass()->as_instance_klass();
  // Make sure the offset goes inside the instance layout.
  return (uint)tp->offset() < (uint)(oopDesc::header_size() + k->nonstatic_field_size())*wordSize;
  // Note that OffsetBot and OffsetTop are very negative.
}

//------------------------------final_graph_reshaping_impl----------------------
// Implement items 1-5 from final_graph_reshaping below.
static void final_graph_reshaping_impl( Node *n, Final_Reshape_Counts &fpu ) {

  uint nop = n->Opcode();

  // Check for 2-input instruction with "last use" on right input.
  // Swap to left input.  Implements item (2).
  if( n->req() == 3 &&          // two-input instruction
      n->in(1)->outcnt() > 1 && // left use is NOT a last use
      n->in(2)->outcnt() == 1 &&// right use IS a last use
      !n->in(2)->is_Con() ) {   // right use is not a constant
    // Check for commutative opcode
    switch( nop ) {
    case Op_AddI:  case Op_AddF:  case Op_AddD:  case Op_AddL:
    case Op_MulI:  case Op_MulF:  case Op_MulD:  case Op_MulL:
    case Op_AndL:  case Op_XorL:  case Op_OrL: 
    case Op_AndI:  case Op_XorI:  case Op_OrI: {
      // Move "last use" input to left by swapping inputs
      n->swap_edges(1, 2);
      break;
    }
    default:
      break;
    }
  }

  // Count FPU ops and common calls, implements item (3)
  switch( nop ) {
  // Count all float operations that may use FPU
  case Op_AddF:
  case Op_SubF:
  case Op_MulF:
  case Op_DivF:
  case Op_NegF:
  case Op_ModF:
  case Op_ConvI2F:
  case Op_ConF:
  case Op_CmpF:
  case Op_CmpF3:
  // case Op_ConvL2F: // longs are split into 32-bit halves
    fpu.inc_float_count();
    break;

  case Op_ConvF2D:
  case Op_ConvD2F:
    fpu.inc_float_count();
    fpu.inc_double_count();
    break;

  // Count all double operations that may use FPU
  case Op_AddD:
  case Op_SubD:
  case Op_MulD:
  case Op_DivD:
  case Op_NegD:
  case Op_ModD:
  case Op_ConvI2D:
  case Op_ConvD2I:
  // case Op_ConvL2D: // handled by leaf call
  // case Op_ConvD2L: // handled by leaf call
  case Op_ConD:
  case Op_CmpD:
  case Op_CmpD3:
    fpu.inc_double_count();
    break;
  case Op_Opaque1:              // Remove Opaque Nodes before matching
  case Op_Opaque2:              // Remove Opaque Nodes before matching
    n->replace_by(n->in(1));
    break;
  case Op_Phi:                  // Clone initializing constants to loops
    {
      PhiNode* nphi = (PhiNode*)n;
      Node* x = nphi->is_copy();
      if (x == n)  x = Compile::current()->top();  // degenerate self-loop
      if (x != NULL)  n->replace_by(x);
      break;
    }
    if( n->in(0)->is_Loop() &&  // These will split & rematerialize anyways
        n->in(1)->is_Con() &&   // so we save the allocator some work here.
        n->in(1)->outcnt() > 1 ) {
      n->set_req( 1, n->in(1)->clone() );
    }
    break;

  case Op_CallStaticJava:
  case Op_CallJava:
  case Op_CallDynamicJava:
  case Op_CallCompiledJava:
  case Op_CallInterpreter:
    fpu.inc_java_call_count(); // Count java call site;
  case Op_CallRuntime:
  case Op_CallLeaf:
  case Op_CallLeafNoFP:
  case Op_CallNative: {
    assert( n->is_Call(), "" );
    CallNode *call = (CallNode*)n;
    // Count call sites where the FP mode bit would have to be flipped.
    // Do not count uncommon runtime calls:
    // uncommon_trap, _complete_monitor_locking, _complete_monitor_unlocking, 
    // _new_Java, _new_typeArray, _new_objArray, _rethrow_Java, ...
    const CallStaticJavaNode *static_call = call->is_CallStaticJava();
    if( !static_call || !static_call->_name ) {
      fpu.inc_call_count();   // Count the call site
    } else {                  // See if uncommon argument is shared
      Node *n = call->in(TypeFunc::Parms);
      int nop = n->Opcode();
      // Clone shared simple arguments to uncommon calls, item (1).
      if( n->outcnt() > 1 && 
          !n->is_Proj() && 
          nop != Op_CreateEx && 
          nop != Op_CheckCastPP && 
          !n->is_Mem() ) {
        Node *x = n->clone();
        call->set_req( TypeFunc::Parms, x );
      }
    }
    break;
  }

  case Op_StoreD:
  case Op_LoadD:
  case Op_LoadD_unaligned:
    fpu.inc_double_count();
    goto handle_mem;
  case Op_StoreF:
  case Op_LoadF:
    fpu.inc_float_count();
    goto handle_mem;

  case Op_StoreB:
  case Op_StoreC:
  case Op_StoreCM:
  case Op_StorePConditional:
  case Op_StoreI:
  case Op_StoreL:
  case Op_StoreLConditional:
  case Op_CompareAndSwapI:
  case Op_CompareAndSwapL:
  case Op_CompareAndSwapP:
  case Op_StoreP:
  case Op_LoadB:
  case Op_LoadC:
  case Op_LoadI:
  case Op_LoadKlass:
  case Op_LoadL:
  case Op_LoadL_unaligned:
  case Op_LoadPLocked:
  case Op_LoadLLocked:
  case Op_LoadP:
  case Op_LoadRange:
  case Op_LoadS: {
  handle_mem:
#ifdef ASSERT
    if( VerifyOptoOopOffsets ) {
      assert( n->is_Mem(), "" );
      MemNode *mem  = (MemNode*)n;
      // Check to see if address types have grounded out somehow.
      const TypeInstPtr *tp = mem->in(MemNode::Address)->bottom_type()->isa_instptr();
      assert( !tp || oop_offset_is_sane(tp), "" );
    }
#endif
    break;
  }
  case Op_If:
  case Op_CountedLoopEnd:
    fpu._tests.push(n);         // Collect CFG split points
    break;

  case Op_AddP: {               // Assert sane base pointers
    const AddPNode *addp = n->in(AddPNode::Address)->is_AddP();
    assert( !addp || 
            addp->in(AddPNode::Base)->is_top() || // Top OK for allocation
            addp->in(AddPNode::Base) == n->in(AddPNode::Base), 
            "Base pointers must match" );
  }

  default: 
    assert( !n->is_Call(), "" );
    assert( !n->is_Mem(), "" );
    if( n->is_If() || n->is_PCTable() ) 
      fpu._tests.push(n);       // Collect CFG split points
    break;
  }
}

//------------------------------final_graph_reshaping_walk---------------------
// Replacing Opaque nodes with their input in final_graph_reshaping_impl(),
// requires that the walk visits a node's inputs before visiting the node.
static void final_graph_reshaping_walk( Node_Stack &nstack, Node *root, Final_Reshape_Counts &fpu ) {
  fpu._visited.set(root->_idx); // first, mark node as visited
  uint cnt = root->req();
  Node *n = root;
  uint  i = 0;
  while (true) {
    if (i < cnt) {
      // Place all non-visited non-null inputs onto stack
      Node* m = n->in(i);
      ++i;
      if (m != NULL && !fpu._visited.test_set(m->_idx)) {
        cnt = m->req();
        nstack.push(n, i); // put on stack parent and next input's index 
        n = m;
        i = 0;
      }
    } else {
      // Now do post-visit work
      final_graph_reshaping_impl( n, fpu );
      if (nstack.is_empty())
        break;             // finished
      n = nstack.node();   // Get node from stack 
      cnt = n->req();
      i = nstack.index();
      nstack.pop();        // Shift to the next node on stack
    }
  }
}

//------------------------------final_graph_reshaping--------------------------
// Final Graph Reshaping.  
//
// (1) Clone simple inputs to uncommon calls, so they can be scheduled late
//     and not commoned up and forced early.  Must come after regular 
//     optimizations to avoid GVN undoing the cloning.  Clone constant
//     inputs to Loop Phis; these will be split by the allocator anyways.
//     Remove Opaque nodes.
// (2) Move last-uses by commutative operations to the left input to encourage
//     Intel update-in-place two-address operations and better register usage
//     on RISCs.  Must come after regular optimizations to avoid GVN Ideal
//     calls canonicalizing them back.
// (3) Count the number of double-precision FP ops, single-precision FP ops
//     and call sites.  On Intel, we can get correct rounding either by
//     forcing singles to memory (requires extra stores and loads after each
//     FP bytecode) or we can set a rounding mode bit (requires setting and
//     clearing the mode bit around call sites).  The mode bit is only used
//     if the relative frequency of single FP ops to calls is low enough.
//     This is a key transform for SPEC mpeg_audio.
// (4) Detect infinite loops; blobs of code reachable from above but not 
//     below.  Several of the Code_Gen algorithms fail on such code shapes,
//     so we simply bail out.  Happens a lot in ZKM.jar, but also happens
//     from time to time in other codes (such as -Xcomp finalizer loops, etc).
//     Detection is by looking for IfNodes where only 1 projection is
//     reachable from below or CatchNodes missing some targets.
// (5) Assert for insane oop offsets in debug mode.

bool Compile::final_graph_reshaping() {
  // an infinite loop may have been eliminated by the optimizer,
  // in which case the graph will be empty.
  if (root()->req() == 1) {
    record_method_not_compilable("trivial infinite loop");
    return true;
  }

  Final_Reshape_Counts fpu;

  // Visit everybody reachable!
  // Allocate stack of size C->unique()/2 to avoid frequent realloc
  Node_Stack nstack(unique() >> 1);
  final_graph_reshaping_walk(nstack, root(), fpu);

  // Check for unreachable (from below) code (i.e., infinite loops).
  for( uint i = 0; i < fpu._tests.size(); i++ ) {
    Node *n = fpu._tests[i];
    const PCTableNode *pct = n->is_PCTable();
    assert( pct || n->is_If(), "either PCTables or IfNodes" );
    // Get number of CFG targets; 2 for IfNodes or _size for PCTables.
    // Note that PCTables include exception targets after calls.
    uint expected_kids = pct ? pct->_size : 2;
    if (n->outcnt() != expected_kids) {
      // Check for a few special cases.  Rethrow Nodes never take the
      // 'fall-thru' path, so expected kids is 1 less.
      if( pct && pct->in(0) && pct->in(0)->in(0) ) {
        CallNode *call = pct->in(0)->in(0)->is_Call();
        if (call && call->entry_point() == OptoRuntime::rethrow_stub())
          expected_kids--;      // Rethrow always has 1 less kid
      }
      // Recheck with a better notion of 'expected_kids'
      if (n->outcnt() != expected_kids) {
        record_method_not_compilable("uncaught exceptions in call");
        return true;            // Not all targets reachable!
      }
    }
    // Check that I actually visited all kids.  Unreached kids
    // must be infinite loops.
    for (DUIterator_Fast jmax, j = n->fast_outs(jmax); j < jmax; j++)
      if (!fpu._visited.test(n->fast_out(j)->_idx)) {
        record_method_not_compilable("infinite loop");
        return true;            // Found unvisited kid; must be unreach
      }
  }

  // If original bytecodes contained a mixture of floats and doubles
  // check if the optimizer has made it homogenous, item (3).
  if( Use24BitFPMode && Use24BitFP && 
      fpu.get_float_count() > 32 && 
      fpu.get_double_count() == 0 &&
      (10 * fpu.get_call_count() < fpu.get_float_count()) ) {
    set_24_bit_selection_and_mode( false,  true );
  }

  set_has_java_calls(fpu.get_java_call_count() > 0);

  // No infinite loops, no reason to bail out.
  return false;
}

#ifndef PRODUCT
//------------------------------verify_graph_edges---------------------------
// Walk the Graph and verify that there is a one-to-one correspondence
// between Use-Def edges and Def-Use edges in the graph.
void Compile::verify_graph_edges(bool no_dead_code) {
  if (VerifyGraphEdges) {
    ResourceArea *area = Thread::current()->resource_area();
    Unique_Node_List visited(area);
    // Call recursive graph walk to check edges
    _root->verify_edges(visited);
    if (no_dead_code) {
      // Now make sure that no visited node is used by an unvisited node.
      bool dead_nodes = 0;
      Unique_Node_List checked(area);
      while (visited.size() > 0) {
        Node* n = visited.pop();
        checked.push(n);
        for (uint i = 0; i < n->outcnt(); i++) {
          Node* use = n->raw_out(i);
          if (checked.member(use))  continue;  // already checked
          if (visited.member(use))  continue;  // already in the graph
          if (use->is_Con())        continue;  // a dead ConNode is OK
          // At this point, we have found a dead node which is DU-reachable.
          if (dead_nodes++ == 0)
            tty->print_cr("*** Dead nodes reachable via DU edges:");
          use->dump(2);
          tty->print_cr("---");
          checked.push(use);  // No repeats; pretend it is now checked.
        }
      }
      assert(dead_nodes == 0, "using nodes must be reachable from root");
    }
  }
}
#endif

// The Compile object keeps track of failure reasons separately from the ciEnv.
// This is required because there is not quite a 1-1 relation between the
// ciEnv and its compilation task and the Compile object.  Note that one
// ciEnv might use two Compile objects, if C2Compiler::compile_method decides
// to backtrack and retry without subsuming loads.  Other than this backtracking
// behavior, the Compile's failure reason is quietly copied up to the ciEnv
// by the logic in C2Compiler.
void Compile::record_failure(const char* reason) {
  if (log() != NULL) {
    log()->elem("failure reason='%s' phase='compile'", reason);
  }
  if (_failure_reason == NULL) {
    // Record the first failure reason.
    _failure_reason = reason;
  }
  _root = NULL;  // flush the graph, too
}

Compile::TracePhase::TracePhase(const char* name, elapsedTimer* accumulator, bool dolog)
  : TraceTime(NULL, accumulator, TimeCompiler, false)
{
  if (dolog) {
    C = Compile::current();
    _log = C->log();
  } else {
    C = NULL;
    _log = NULL;
  }
  if (_log != NULL) {
    _log->begin_head("phase name='%s' nodes='%d'", name, C->unique());
    _log->stamp();
    _log->end_head();
  }
}

Compile::TracePhase::~TracePhase() {
  if (_log != NULL) {
    _log->done("phase nodes='%d'", C->unique());
  }
}
