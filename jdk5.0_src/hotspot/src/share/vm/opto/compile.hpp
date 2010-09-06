#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compile.hpp	1.199 04/03/10 15:53:02 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Block;
class Bundle;
class C2Compiler;
class CallGenerator;
class InlineTree;
class Int_Array;
class Matcher;
class MachNode;
class Node;
class Node_Array;
class OptoReg;
class PhaseCFG;
class PhaseGVN;
class PhaseRegAlloc;
class PhaseCCP;
class PhaseCCP_DCE;
class RootNode;
class relocInfo;
class RelocateBuffer;
class Scope;
class StartNode;
class SafePointNode;
class JVMState;
class TypeData;
class TypePtr;
class TypeFunc;
class Unique_Node_List;
class nmethod;
class WarmCallInfo;

//------------------------------Compile----------------------------------------
// This class defines a top-level Compiler invocation. 

class Compile : public Phase {
 public:
  // Fixed alias indexes.  (See also MergeMemNode.)
  enum {
    AliasIdxTop = 1,  // pseudo-index, aliases to nothing (used as sentinel value)
    AliasIdxBot = 2,  // pseudo-index, aliases to everything
    AliasIdxRaw = 3   // hard-wired index for TypeRawPtr::BOTTOM
  };

  // Variant of TraceTime(NULL, &_t_accumulator, TimeCompiler);
  // Integrated with logging.  If logging is turned on, and dolog is true,
  // then brackets are put into the log, with time stamps and node counts.
  // (The time collection itself is always conditionalized on TimeCompiler.)
  class TracePhase : public TraceTime {
   private:
    Compile*    C;
    CompileLog* _log;
   public:
    TracePhase(const char* name, elapsedTimer* accumulator, bool dolog);
    ~TracePhase();
  };

  // Information per category of alias (memory slice)
  class AliasType {
   private:
    friend class Compile;

    int             _index;         // unique index, used with MergeMemNode
    const TypePtr*  _adr_type;      // normalized address type
    ciField*        _field;         // relevant instance field, or null if none
    bool            _is_rewritable; // false if the memory is write-once only
    bool            _force_volatile;// true if manually set. If false, check _field before claiming false

    void Init(int i, const TypePtr* at) {
      _index = i;
      _adr_type = at;
      _field = NULL;
      _is_rewritable = true; // default
      _force_volatile = false;
    }

   public:
    int             index()         const { return _index; }
    const TypePtr*  adr_type()      const { return _adr_type; }
    ciField*        field()         const { return _field; }
    bool            is_rewritable() const { return _is_rewritable; }
    bool            is_volatile()   const { return (_force_volatile)? true : (_field ? _field->is_volatile() : false); }
    void set_volatile(bool z) { _force_volatile = z; assert(is_volatile() == z, "cannot clear volatile attribute of a volatile field.");}

    void set_rewritable(bool z) { _is_rewritable = z; }
    void set_field(ciField* f) {
      assert(!_field,"");
      _field = f;
      if (f->is_final())  _is_rewritable = false;
    }

    void print_on(outputStream* st) PRODUCT_RETURN;
  };

  enum {
    logAliasCacheSize = 6,
    AliasCacheSize = (1<<logAliasCacheSize)
  };
  struct AliasCacheEntry { const TypePtr* _adr_type; int _index; };  // simple duple type
  enum {
    trapHistLength = methodDataOopDesc::_trap_hist_limit
  };

 private:
  // Fixed parameters to this compilation.
  const int             _compile_id;
  const bool            _save_argument_registers; // save/restore arg regs for trampolines
  const bool            _subsume_loads;         // Load can be matched as part of a larger op.
  ciMethod*             _method;                // The method being compiled.
  int                   _entry_bci;             // entry bci for osr methods.
  const TypeFunc*       _tf;                    // My kind of signature
  InlineTree*           _ilt;                   // Ditto (temporary).
  address               _stub_function;         // VM entry for stub being compiled, or NULL
  const char*           _stub_name;             // Name of stub or adapter being compiled, or NULL
  address               _stub_entry_point;      // Compile code entry for generated stub, or NULL

  // Control of this compilation.
  int                   _num_loop_opts;         // Number of iterations for doing loop optimiztions
  int                   _max_inline_size;       // Max inline size for this compilation
  int                   _freq_inline_size;      // Max hot method inline size for this compilation
  int                   _sync;                  // Largest number of simultaneously live monitors
  int                   _major_progress;        // Count of something big happening
  bool                  _deopt_happens;         // TRUE if de-optimization CAN happen
  bool                  _has_loops;             // True if the method _may_ have some loops
  bool                  _has_split_ifs;         // True if the method _may_ have some split-if
  bool                  _has_unsafe_access;     // True if the method _may_ produce faults in unsafe loads or stores.
  uint                  _trap_hist[trapHistLength];  // Cumulative traps
  bool                  _trap_can_recompile;    // Have we emitted a recompiling trap?
  uint                  _decompile_count;       // Cumulative decompilation counts.
  bool                  _do_inlining;           // True if we intend to do inlining
  bool                  _do_scheduling;         // True if we intend to do scheduling
  bool                  _do_count_invocations;  // True if we generate code to count invocations
  bool                  _do_method_data_update; // True if we generate code to update methodDataOops
  int                   _AliasLevel;            // Locally-adjusted version of AliasLevel flag.
  bool                  _print_assembly;        // True if we should dump assembly code for this compilation

  // Compilation environment.
  Arena                 _comp_arena;            // Arena with lifetime equivalent to Compile
  ciEnv*                _env;                   // CI interface
  CompileLog*           _log;                   // from CompilerThread
  const char*           _failure_reason;        // for record_failure/failing pattern
  ciMethod*             _Method_invoke;         // Method.invoke(Object, Object[])
  ciSymbol*             _invoke_name;           // "invoke"
  ciInstanceKlass*      _MethodAccessorImpl;    // sun.reflect.MethodAccessorImpl
  GrowableArray<CallGenerator*>* _intrinsics;   // List of intrinsics.

  //                Capture JVMPI status once for the compilation.
  bool                  _need_jvmpi_compiled_method_event;
  bool                  _need_jvmpi_method_entry_event;
  bool                  _need_jvmpi_method_entry2_event;
  bool                  _need_jvmpi_method_exit_event;

  // Node management
  uint                  _unique;                // Counter for unique Node indices
  debug_only(static int _debug_idx;)            // Monotonic counter (not reset), use -XX:BreakAtNode=<idx>
  Arena                 _node_arena;            // Arena for new-space Nodes
  Arena                 _old_arena;             // Arena for old-space Nodes, lifetime during xform
  RootNode*             _root;                  // Unique root of compilation, or NULL after bail-out.
  Node*                 _top;                   // Unique top node.  (Reset by various phases.)

  // After parsing and every bulk phase we hang onto the Root instruction.
  // The RootNode instruction is where the whole program begins.  It produces
  // the initial Control and BOTTOM for everybody else.

  // Type management
  Arena                 _Compile_types;         // Arena for all types
  Arena*                _type_arena;            // Alias for _Compile_types except in Initialize_shared()
  Dict*                 _type_dict;             // Intern table
  void*                 _type_hwm;              // Last allocation (see Type::operator new/delete)
  size_t                _type_last_size;        // Last allocation size (see Type::operator new/delete)
  ciMethod*             _last_tf_m;             // Cache for
  const TypeFunc*       _last_tf;               //  TypeFunc::make
  AliasType**           _alias_types;           // List of alias types seen so far.
  int                   _num_alias_types;       // Logical length of _alias_types
  int                   _max_alias_types;       // Physical length of _alias_types
  AliasCacheEntry       _alias_cache[AliasCacheSize]; // Gets aliases w/o data structure walking

  // Parsing, optimization
  PhaseGVN*             _initial_gvn;           // Results of parse-time PhaseGVN
  Unique_Node_List*     _for_igvn;              // Initial work-list for next round of Iterative GVN
  WarmCallInfo*         _warm_calls;            // Sorted work-list for heat-based inlining.

  // Matching, CFG layout, allocation, code generation
  PhaseCFG*             _cfg;                   // Results of CFG finding
  bool                  _select_24_bit_instr;   // We selected an instruction with a 24-bit result
  bool                  _in_24_bit_fp_mode;     // We are emitting instructions with 24-bit results
  bool                  _has_java_calls;        // True if the method has java calls 
  Matcher*              _matcher;               // Engine to map ideal to machine instructions
  PhaseRegAlloc*        _regalloc;              // Results of register allocation.
  int                   _frame_slots;           // Size of total frame in stack slots
  RegMask               _FIRST_STACK_mask;      // All stack slots usable for spills (depends on frame layout)
  Arena*                _indexSet_arena;        // control IndexSet allocation within PhaseChaitin
  void*                 _indexSet_free_block_list; // free list of IndexSet bit blocks

  uint                  _node_bundling_limit;
  Bundle*               _node_bundling_base;    // Information for instruction bundling

  // Instruction bits passed off to the VM
  int                   _method_size;           // Size of nmethod code segment in bytes
  CodeBuffer*           _code_buffer;           // Where the code is assembled
  address               _code_base;             // Handy base address in code_buffer
  int                   _first_block_size;      // Size of unvalidated entry point code / OSR poison code
  ExceptionHandlerTable _handler_table;         // Table of native-code exception handlers
  ImplicitExceptionTable _inc_table;            // Table of implicit null checks in native code
  OopMapSet*            _oop_map_set;           // Table of oop maps (one for each safepoint location)
  static int            _CompiledZap_count;     // counter compared against CompileZap[First/Last]
  BufferBlob*           _scratch_buffer_blob;   // For temporary code buffers.
  relocInfo*            _scratch_locs_memory;   // For temporary code buffers.
  RelocateBuffer*       _scratch_locs_stub_memory; // For temporary code buffers.

 public:
  // Accessors

  // The Compile instance currently active in this (compiler) thread.
  static Compile*   current() {
    Compile* current = (Compile*) ciEnv::data();
    assert(current != NULL && current->env() == ciEnv::current(), "no active Compile");
    return current;
  }

  // ID for this compilation.  Useful for setting breakpoints in the debugger.
  int               compile_id() const          { return _compile_id; }

  // Does this compilation allow instructions to subsume loads?  User
  // instructions that subsume a load may result in an unschedulable
  // instruction sequence.
  bool              subsume_loads() const       { return _subsume_loads; }
  bool              save_argument_registers() const { return _save_argument_registers; }


  // Other fixed compilation parameters.
  ciMethod*         method() const              { return _method; }
  int               entry_bci() const           { return _entry_bci; }
  bool              is_osr_compilation() const  { return _entry_bci != InvocationEntryBci; }
  bool              is_method_compilation() const { return (_method != NULL && !_method->flags().is_native()); }
  bool              is_native_compilation() const { return (_method != NULL &&  _method->flags().is_native()); }
  const TypeFunc*   tf() const                  { assert(_tf!=NULL, ""); return _tf; }
  void         init_tf(const TypeFunc* tf)      { assert(_tf==NULL, ""); _tf = tf; }
  InlineTree*       ilt() const                 { return _ilt; }
  address           stub_function() const       { return _stub_function; }
  const char*       stub_name() const           { return _stub_name; }
  address           stub_entry_point() const    { return _stub_entry_point; }

  // Control of this compilation.
  int               sync() const                { assert(_sync >= 0, "");         return _sync; }
  void          set_sync(int n)                 { _sync = n; }
  int               major_progress() const      { return _major_progress; }
  void          set_major_progress()            { _major_progress++; }
  void        clear_major_progress()            { _major_progress = 0; }
  int               num_loop_opts() const       { return _num_loop_opts; }
  void          set_num_loop_opts(int n)        { _num_loop_opts = n; }
  int               max_inline_size() const     { return _max_inline_size; }
  void          set_freq_inline_size(int n)     { _freq_inline_size = n; }
  int               freq_inline_size() const    { return _freq_inline_size; }
  void          set_max_inline_size(int n)      { _max_inline_size = n; }
  bool              deopt_happens() const       { return _deopt_happens; }
  bool              has_loops() const           { return _has_loops; }
  void          set_has_loops(bool z)           { _has_loops = z; }
  bool              has_split_ifs() const       { return _has_split_ifs; }
  void          set_has_split_ifs(bool z)       { _has_split_ifs = z; }
  bool              has_unsafe_access() const   { return _has_unsafe_access; }
  void          set_has_unsafe_access(bool z)   { _has_unsafe_access = z; }
  void          set_trap_count(uint r, uint c)  { assert(r < trapHistLength, "oob");        _trap_hist[r] = c; }
  uint              trap_count(uint r) const    { assert(r < trapHistLength, "oob"); return _trap_hist[r]; }
  bool              trap_can_recompile() const  { return _trap_can_recompile; }
  void          set_trap_can_recompile(bool z)  { _trap_can_recompile = z; }
  uint              decompile_count() const     { return _decompile_count; }
  void          set_decompile_count(uint c)     { _decompile_count = c; }
  bool              allow_range_check_smearing() const;
  bool              do_inlining() const         { return _do_inlining; }
  void          set_do_inlining(bool z)         { _do_inlining = z; }
  bool              do_scheduling() const       { return _do_scheduling; }
  void          set_do_scheduling(bool z)       { _do_scheduling = z; }
  bool              do_count_invocations() const{ return _do_count_invocations; }
  void          set_do_count_invocations(bool z){ _do_count_invocations = z; }
  bool              do_method_data_update() const { return _do_method_data_update; }
  void          set_do_method_data_update(bool z) { _do_method_data_update = z; }
  int               AliasLevel() const          { return _AliasLevel; }
  bool              print_assembly() const       { return _print_assembly; }
  void          set_print_assembly(bool z)       { _print_assembly = z; }

  // Compilation environment.
  Arena*            comp_arena()                { return &_comp_arena; }
  ciEnv*            env() const                 { return _env; }
  CompileLog*       log() const                 { return _log; }
  bool              failing() const             { return _env->failing() || _failure_reason != NULL; }
  const char* failure_reason() { return _failure_reason; }
  ciMethod*         get_Method_invoke();
  ciSymbol*         get_invoke_name();
  ciInstanceKlass*  get_MethodAccessorImpl();

  void record_failure(const char* reason);
  void record_method_not_compilable(const char* reason, bool all_tiers = false) { 
    // All bailouts cover "all_tiers" when TieredCompilation is off.
    if (!TieredCompilation) all_tiers = true;
    env()->record_method_not_compilable(reason, all_tiers);
    // Record failure reason.
    record_failure(reason);
  }
  void record_method_not_compilable_all_tiers(const char* reason) { 
    record_method_not_compilable(reason, true);
  }
  bool check_node_count(uint margin, const char* reason) {
    if (unique() + margin > (uint)MaxNodeLimit) {
      record_method_not_compilable(reason);
      return true;
    } else {
      return false;
    }
  }

  bool need_jvmpi_compiled_method_event() const { return _need_jvmpi_compiled_method_event; }
  bool need_jvmpi_method_entry_event() const    { return _need_jvmpi_method_entry_event;    }
  bool need_jvmpi_method_entry2_event() const   { return _need_jvmpi_method_entry2_event;   }
  bool need_jvmpi_method_exit_event() const     { return _need_jvmpi_method_exit_event;     }
  bool need_jvmpi_method_event() const {
    return need_jvmpi_method_entry_event()
        || need_jvmpi_method_entry2_event()
        || need_jvmpi_method_exit_event();
  }

  // Node management
  uint              unique() const              { return _unique; }
  uint         next_unique()                    { return _unique++; }
  void          set_unique(uint i)              { _unique = i; }
  static int        debug_idx()                 { return debug_only(_debug_idx)+0; }
  static void   set_debug_idx(int i)            { debug_only(_debug_idx = i); }
  Arena*            node_arena()                { return &_node_arena; }
  Arena*            old_arena()                 { return &_old_arena; }
  RootNode*         root() const                { return _root; }
  void          set_root(RootNode* r)           { _root = r; }
  StartNode*        start() const;              // (Derived from root.)
  void         init_start(StartNode* s);

  // Handy undefined Node
  Node*             top() const                 { return _top; }

  // these are used by guys who need to know about creation and transformation of top:
  Node*             cached_top_node()           { return _top; }
  void          set_cached_top_node(Node* tn);

  // Type management
  Arena*            type_arena()                { return _type_arena; }
  Dict*             type_dict()                 { return _type_dict; }
  void*             type_hwm()                  { return _type_hwm; }
  size_t            type_last_size()            { return _type_last_size; }
  int               num_alias_types()           { return _num_alias_types; }

  void          init_type_arena()                       { _type_arena = &_Compile_types; }
  void          set_type_arena(Arena* a)                { _type_arena = a; }
  void          set_type_dict(Dict* d)                  { _type_dict = d; }
  void          set_type_hwm(void* p)                   { _type_hwm = p; }
  void          set_type_last_size(size_t sz)           { _type_last_size = sz; }

  const TypeFunc* last_tf(ciMethod* m) {
    return (m == _last_tf_m) ? _last_tf : NULL;
  }
  void set_last_tf(ciMethod* m, const TypeFunc* tf) {
    assert(m != NULL || tf == NULL, "");
    _last_tf_m = m;
    _last_tf = tf;
  }

  AliasType*        alias_type(int                idx)  { assert(idx < num_alias_types(), "oob"); return _alias_types[idx]; }
  AliasType*        alias_type(const TypePtr* adr_type) { return find_alias_type(adr_type, false); }
  bool         have_alias_type(const TypePtr* adr_type);
  AliasType*        alias_type(ciField*         field);

  int               get_alias_index(const TypePtr* at)  { return alias_type(at)->index(); }
  const TypePtr*    get_adr_type(uint aidx)             { return alias_type(aidx)->adr_type(); }

  // Building nodes
  void              rethrow_exceptions(JVMState* jvms);
  void              return_values(JVMState* jvms);
  JVMState*         build_start_state(StartNode* start, const TypeFunc* tf);

  // Decide how to build a call.
  // The profile factor is a discount to apply to this site's interp. profile.
  CallGenerator*    call_generator(ciMethod* call_method, bool call_is_virtual, JVMState* jvms, bool allow_inline, float profile_factor);

  // Parsing, optimization
  PhaseGVN*         initial_gvn()               { return _initial_gvn; }
  Unique_Node_List* for_igvn()                  { return _for_igvn; }
  inline void       record_for_igvn(Node* n);   // Body is after class Unique_Node_List.

  void          set_initial_gvn(PhaseGVN *gvn)           { _initial_gvn = gvn; }
  void          set_for_igvn(Unique_Node_List *for_igvn) { _for_igvn = for_igvn; }

  void              identify_useful_nodes(Unique_Node_List &useful);
  void              remove_useless_nodes  (Unique_Node_List &useful);

  WarmCallInfo*     warm_calls() const          { return _warm_calls; }
  void          set_warm_calls(WarmCallInfo* l) { _warm_calls = l; }
  WarmCallInfo* pop_warm_call();

  // Matching, CFG layout, allocation, code generation
  PhaseCFG*         cfg()                       { return _cfg; }
  bool              select_24_bit_instr() const { return _select_24_bit_instr; }
  bool              in_24_bit_fp_mode() const   { return _in_24_bit_fp_mode; }
  bool              has_java_calls() const      { return _has_java_calls; }
  Matcher*          matcher()                   { return _matcher; }
  PhaseRegAlloc*    regalloc()                  { return _regalloc; }
  int               frame_slots() const         { return _frame_slots; }
  int               frame_size_in_words() const; // frame_slots in units of the polymorphic 'words'
  RegMask&          FIRST_STACK_mask()          { return _FIRST_STACK_mask; }
  Arena*            indexSet_arena()            { return _indexSet_arena; }
  void*             indexSet_free_block_list()  { return _indexSet_free_block_list; }
  uint              node_bundling_limit()       { return _node_bundling_limit; }
  Bundle*           node_bundling_base()        { return _node_bundling_base; }
  void          set_node_bundling_limit(uint n) { _node_bundling_limit = n; }
  void          set_node_bundling_base(Bundle* b) { _node_bundling_base = b; }
  bool          starts_bundle(const Node *n) const;
  bool          need_stack_bang(int frame_size_in_bytes) const;
  bool          need_register_stack_bang() const;

  void          set_matcher(Matcher* m)                 { _matcher = m; }
//void          set_regalloc(PhaseRegAlloc* ra)           { _regalloc = ra; }
  void          set_indexSet_arena(Arena* a)            { _indexSet_arena = a; }
  void          set_indexSet_free_block_list(void* p)   { _indexSet_free_block_list = p; }

  // Remember if this compilation changes hardware mode to 24-bit precision
  void set_24_bit_selection_and_mode(bool selection, bool mode) {
    _select_24_bit_instr = selection;
    _in_24_bit_fp_mode   = mode;
  }

  void set_has_java_calls(bool z) { _has_java_calls = z; }

  // Instruction bits passed off to the VM
  int               code_size()                 { return _method_size; }
  CodeBuffer*       code_buffer()               { return _code_buffer; }
  address           code_base()                 { return _code_base; }
  int               first_block_size()          { return _first_block_size; }
  ExceptionHandlerTable*  handler_table()       { return &_handler_table; }
  ImplicitExceptionTable* inc_table()           { return &_inc_table; }
  OopMapSet*        oop_map_set()               { return _oop_map_set; }
  DebugInformationRecorder* recorder()          { return env()->recorder(); }
  void set_recorder(DebugInformationRecorder* r) { env()->set_recorder(r); }
  static int        CompiledZap_count()         { return _CompiledZap_count; }
  BufferBlob*       scratch_buffer_blob()       { return _scratch_buffer_blob; }
  void         init_scratch_buffer_blob();
  void          set_scratch_buffer_blob(BufferBlob* b) { _scratch_buffer_blob = b; }
  relocInfo*        scratch_locs_memory()       { return _scratch_locs_memory; }
  void          set_scratch_locs_memory(relocInfo* b)  { _scratch_locs_memory = b; }
  RelocateBuffer*   scratch_locs_stub_memory()  { return _scratch_locs_stub_memory; }
  void          set_scratch_locs_stub_memory(RelocateBuffer* b) { _scratch_locs_stub_memory = b; }
  void         init_scratch_locs_memory();

  enum ScratchBufferBlob {
    MAX_inst_size       = 1024,
    MAX_locs_size       = 64 * sizeof(relocInfo),
    MAX_const_size      = 128,
    MAX_stubs_size      = 128,
    MAX_locs_stub_size  = 8 * sizeof(RelocateBuffer)
  };

  // Major entry point.  Given a Scope, compile the associated method.
  // For normal compilations, entry_bci is InvocationEntryBci.  For on stack
  // replacement, entry_bci indicates the bytecode for which to compile a
  // continuation.
  Compile(ciEnv* ci_env, C2Compiler* compiler, ciMethod* target,
          int entry_bci, bool subsume_loads);

  // Second major entry point.  From the TypeFunc signature, generate code
  // to pass arguments from the Java calling convention to the C calling
  // convention.  
  Compile(ciEnv* ci_env, const TypeFunc *(*gen)(),
          address stub_function, const char *stub_name,
          int is_fancy_jump, bool pass_tls,
          bool save_arg_registers, bool return_pc);

  // Third major entry point.  Compile a native method.
  Compile(ciEnv* ci_env, C2Compiler* compiler, ciMethod* method);

  // Fourth major entry point.  Compile an adapter.
  Compile(ciEnv* ci_env, ciMethod* method, int /*AdapterType*/ type);

  // From the TypeFunc signature, generate code to pass arguments
  // from Compiled calling convention to Interpreter's calling convention
  void Generate_Compiled_To_Interpreter_Graph(const TypeFunc *tf, address interpreter_entry);

  // From the TypeFunc signature, generate code to pass arguments
  // from Interpreter's calling convention to Compiler's calling convention
  void Generate_Interpreter_To_Compiled_Graph(const TypeFunc *tf);

  // Are we compiling a method?
  bool has_method() { return method() != NULL; }

  // Maybe print some information about this compile.
  void print_compile_messages();

  // Final graph reshaping, a post-pass after the regular optimizer is done.
  bool final_graph_reshaping();

  // returns true if adr is completely contained in the given alias category
  bool must_alias(const TypePtr* adr, int alias_idx);

  // returns true if adr overlaps with the given alias category
  bool can_alias(const TypePtr* adr, int alias_idx);
  
  // Driver for converting compiler's IR into machine code bits
  void Output();

  // Accessors for node bundling info.
  Bundle* node_bundling(const Node *n);
  bool valid_bundle_info(const Node *n);

  // Schedule and Bundle the instructions
  void ScheduleAndBundle();

  // Build OopMaps for each GC point
  void BuildOopMaps();
  // Append debug info for the node to the array
  int FillLocArray( Node *local, GrowableArray<ScopeValue*> *array );

  // Process an OopMap Element while emitting nodes
  void Process_OopMap_Node(MachNode *mach, int code_offset);

  // Write out basic block data to code buffer
  void Fill_buffer();

  // Determine which variable sized branches can be shortened
  void Shorten_branches(Label *labels, int& code_size, int& reloc_size, int& stub_size, int& const_size);

  // Compute the information for the exception tables
  void FillExceptionTables(uint cnt, uint *call_returns, uint *inct_starts, Label *blk_labels);

  // Stack slots that may be unused by the calling convention but must
  // otherwise be preserved.  On Intel this includes the return address.
  // On PowerPC it includes the 4 words holding the old TOC & LR glue.
  uint in_preserve_stack_slots();

  // "Top of Stack" slots that may be unused by the calling convention but must
  // otherwise be preserved.  
  // On Intel these are not necessary and the value can be zero.
  // On Sparc this describes the words reserved for storing a register window
  // when an interrupt occurs.
  static uint out_preserve_stack_slots();

  // Number of outgoing stack slots killed above the out_preserve_stack_slots
  // for calls to C.  Supports the var-args backing area for register parms.
  uint varargs_C_out_slots_killed() const;

  // Number of Stack Slots consumed by a synchronization entry
  int sync_stack_slots() const;

  // Compute the name of old_SP.  See <arch>.ad for frame layout.
  OptoReg::Name compute_old_SP();
  OptoReg::Name compute_new_SP(int in_arg_size);
  OptoReg::Name compute_out_arg_base(OptoReg::Name new_SP);

  // Tell whether a C2I adapter can be generated for a given method.
  // Somebody must already have computed the number of compiled arguments.
  bool can_generate_C2I(ciMethod* method, int compiled_arg_size);

#ifdef ENABLE_ZAP_DEAD_LOCALS
  static bool is_node_getting_a_safepoint(Node*);
  void Insert_zap_nodes();
  Node* call_zap_node(Node* n, int block_no);
#endif

 private:
  // Phase control:
  void Init(int aliaslevel);                     // Prepare for a single compilation
  int  Inline_Warm();                            // Find more inlining work.
  void Finish_Warm();                            // Give up on further inlines.
  void Optimize();                               // Given a graph, optimize it
  void Code_Gen();                               // Generate code from a graph

  // Management of the AliasType table.
  void grow_alias_types();
  AliasCacheEntry* probe_alias_cache(const TypePtr* adr_type);
  const TypePtr *flatten_alias_type(const TypePtr* adr_type) const;
  AliasType* find_alias_type(const TypePtr* adr_type, bool no_create);

  void verify_top(Node*) const PRODUCT_RETURN;

  // Intrinsic setup.
  void           register_library_intrinsics();                            // initializer
  CallGenerator* make_vm_intrinsic(ciMethod* m, bool is_virtual);          // constructor
  int            intrinsic_insertion_index(ciMethod* m, bool is_virtual);  // helper
  CallGenerator* find_intrinsic(ciMethod* m, bool is_virtual);             // query fn
  void           register_intrinsic(CallGenerator* cg);                    // update fn

 public:
  // Graph verification code
  // Walk the node list, verifying that there is a one-to-one
  // correspondence between Use-Def edges and Def-Use edges
  // The option no_dead_code enables stronger checks that the
  // graph is strongly connected from root in both directions.
  void verify_graph_edges(bool no_dead_code = false) PRODUCT_RETURN;

  // Print bytecodes, including the scope inlining tree
  void print_codes();

  // Dump formatted assembly
  void dump_asm(int *pcs = NULL, uint pc_limit = 0) PRODUCT_RETURN;
  void dump_pc(int *pcs, int pc_limit, Node *n);

  // Verify ADLC assumptions during startup
  static void adlc_verification() PRODUCT_RETURN;

  // Definitions of pd methods
  static void pd_compiler2_init();
};
