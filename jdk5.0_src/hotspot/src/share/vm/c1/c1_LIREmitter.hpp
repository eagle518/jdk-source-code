#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIREmitter.hpp	1.89 04/04/20 15:56:12 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

class LocalMapping;
class IRScopeDebugInfo;

class CodeEmitInfo: public CompilationResourceObj {
 private:
  bool              _is_compiled_safepoint;
  RInfoCollection*  _register_oops;              // registers that contain oops
  IRScopeDebugInfo* _scope_debug_info;
  IRScope*          _scope;
  ExceptionScope*   _exception_scope;
  OopMap*           _oop_map;
  intStack*         _spilled_oops;               // all spill-indeces that are oops
  ValueStack*       _stack;                      // used by deoptimization (contains also monitors
  GrowableArray<LIR_OprDesc*>* _lir_expression_stack;
  int         _bci;
  const LocalMapping* _local_mapping;
#ifndef PRODUCT
  BitMap            _lir_oop_map;                // Oop map for locals only, from LIR
  bool              _lir_oop_map_set;
  bool              _lir_adjusted;
#endif

  // oop maps / address maps
  OopMap* create_oop_map();
  OopMap* create_oop_map_for_own_signature();
  OopMap* create_oop_map_inside_natives();
  bool    local_is_live_in_scope(int local_no);

  void add_registers_to_oop_map(OopMap* map);

  NOT_PRODUCT(bool oop_maps_equal(OopMap* om1, OopMap* om2);)

  Location location_for_monitor_lock_index  (int monitor_index);
  Location location_for_monitor_object_index(int monitor_index);
  Location location_for_name                (int name, Location::Type loc_type,
                                             bool is_two_word = false, bool for_hi_word = false);
  Location location_for_local_offset        (int local_offset, Location::Type loc_type);

  void scope_value_for_register(RInfo reg, ScopeValue** first, ScopeValue** second, Location::Type loc_type);
  ScopeValue* scope_value_for_local_offset(int local_offset, Location::Type loc_type, ScopeValue** second);
  IRScopeDebugInfo* compute_debug_info_for_scope(IRScope* scope, int bci,
                                                 GrowableArray<ScopeValue*>* stack, int stack_end, Values locks, int locks_end);
  Location::Type opr2location_type(LIR_Opr opr);
  void append_scope_value(LIR_Opr opr, GrowableArray<ScopeValue*>* expressions);
  GrowableArray<ScopeValue*>* lir_stack2value_stack(GrowableArray<LIR_Opr>* lir_stack);

  OopMap* oop_map();
  WordSizeList* local_name_to_offset_map() const { return scope()->compilation()->hir()->local_name_to_offset_map(); }
  FrameMap*     frame_map() const                { return scope()->compilation()->frame_map(); }
  Compilation*  compilation() const              { return scope()->compilation(); }

  void fill_expression_stack();

 public:

  // use scope from ValueStack
  CodeEmitInfo(LIR_Emitter* emit, int bci, intStack* soops, ValueStack* stack, ExceptionScope* exception_scope, RInfoCollection* oops_in_regs = NULL);

  // used by natives
  CodeEmitInfo(IRScope* scope, int bci, intStack* soops)
    : _scope(scope)
    , _bci(bci)
    , _spilled_oops(soops)
    , _oop_map(NULL)
    , _scope_debug_info(NULL)
    , _stack(NULL)
    , _exception_scope(NULL)
    , _lir_expression_stack(NULL)
    , _register_oops(NULL)
#ifndef PRODUCT
    , _lir_oop_map(NULL, 0)
    , _lir_oop_map_set(false)
    , _lir_adjusted(false)
#endif // PRODUCT
    , _local_mapping(NULL)
    , _is_compiled_safepoint(false) {
  }

  void set_is_compiled_safepoint()               { _is_compiled_safepoint = true; }
  bool is_compiled_safepoint() const             { return _is_compiled_safepoint; }
  void check_is_exception_info() const NOT_DEBUG({});

  // make a copy
  CodeEmitInfo(CodeEmitInfo* info, bool lock_stack_only = false);

  // accessors
  intStack* spilled_oops() const                 { return _spilled_oops; }
  const RInfoCollection* register_oops() const   { return _register_oops; }
  ciMethod* method() const                       { return _scope->method(); }
  IRScope* scope() const                         { return _scope; }
  ExceptionScope* exception_scope() const        { return _exception_scope; }
  ValueStack* stack() const                      { return _stack; }
  GrowableArray<LIR_OprDesc*>* lir_expression_stack() const { return _lir_expression_stack; }
  const LocalMapping* local_mapping() const      { return _local_mapping; }
  int bci() const                                { return _bci; }

  bool has_register_oops() const                 { return _register_oops != NULL && _register_oops->length() > 0; }
  void add_register_oop(RInfo reg);
  void set_local_mapping(LocalMapping* mapping);
#ifndef PRODUCT
  BitMap* lir_oop_map()                          { return &_lir_oop_map; }
  void set_lir_oop_map(BitMap* map);
  bool was_lir_oop_map_set()                     { return _lir_oop_map_set; }
#endif
  void compute_debug_info();
  void record_debug_info(DebugInformationRecorder* recorder, int pc_offset, bool at_call);

  void record_spilled_oops(FrameMap* frame_map, OopMap* oop_map) const;

  RInfo get_cache_reg(int pos, ValueTag tag) const;
  RInfo get_cache_reg(int pos) const;
  RInfo get_cache_reg_for_local_offset(int pos) const;
  bool  is_cache_reg(RInfo reg) const;

#ifndef PRODUCT
  void set_lir_adjusted()                        { _lir_adjusted = true; }
  bool is_lir_adjusted() const                   { return _lir_adjusted; }
#endif // PRODUCT
};


class CodeOffsets: public StackObj {
public:
  int _iep_offset;
  int _ep_offset;
  int _vep_offset;
  int _code_offset;
  int _osr_offset;
  CodeOffsets(): _iep_offset(0), _ep_offset(0), _vep_offset(0), _code_offset(0), _osr_offset(0) {}
};


class Compilation;
class LIR_Assembler;

// This class contains creation of LIR instructions and dispatches to LIR_Assembler
// for code emission

class LIR_Emitter: public CompilationResourceObj {
 private:
  bool      _bailout;
  FrameMap* _frame_map;

 private:
  LIR_List* _lir;

  Compilation*   _compilation;

  RInfo long2address(LIR_Opr opr);

  jint opr2int(LIR_Opr opr);
  jint opr2intLo(LIR_Opr opr);
  jint opr2intHi(LIR_Opr opr);
  jlong opr2long(LIR_Opr opr);
  jobject opr2jobject(LIR_Opr opr);

  // is_strictfp is only needed for mul and div (and only generates different code on i486)
  void arithmetic_op(Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, bool is_strictfp, RInfo tmp, CodeEmitInfo* info = NULL);
  // machine dependent.  returns true if it emitted code for the multiply
  bool strength_reduce_multiply(LIR_Opr left, int constant, LIR_Opr result, LIR_Opr tmp);

  LIR_OpBranch::LIR_Condition lir_cond(If::Condition cond);

  void init_local(IRScope* scope, int local_index);
  void monitorenter_at_entry(RInfo receiver, CodeEmitInfo* info);

  void field_store_byte(LIR_Opr object, int offset_in_bytes, LIR_Opr value, RInfo tmp, bool needs_patching, CodeEmitInfo* info);
  void field_store_long(LIR_Opr object, int offset_in_bytes, LIR_Opr value, bool needs_patching, CodeEmitInfo* info);
  LIR_Address* array_address(LIR_Opr array, LIR_Opr index, int offset, BasicType type);

  int hi_word_offset_in_bytes() const;
  int lo_word_offset_in_bytes() const;


//---------------------------------------------------------------------------------
 public:
  LIR_Emitter(Compilation* compilation);

  LIR_List* lir() const                          { return _lir; }

  void set_bailout(const char* msg);
  bool must_bailout() const;

  FrameMap* frame_map() const                    { return _frame_map;  }

  // two words on the stack must be adjacent (local index, index+1)
  void check_double_address(int local_index);

  C1_MacroAssembler* masm() const;
  Compilation* compilation() const               { return _compilation; }
  ciMethod* method() const;

  void start_block(BlockBegin* bb);
  int  frame_size(); // fp - sp
  int  initial_frame_size_in_bytes ();

  // call if an instruction implicitly sets an FP result (sets fpu_stack)
  void set_fpu_result      (RInfo reg);
  void remove_fpu_result   (RInfo reg);
  void set_fpu_stack_empty ();
  void fpop();

  void bind_block_entry(BlockBegin* block);
  void trace_block_entry(BlockBegin* block, address func); // machine dependent

  void print() PRODUCT_RETURN;

  // ======================== code emission ==================================

  void copy_fpu_item (RInfo toReg, LIR_Opr from);

  void push_item(LIR_Opr opr);

  void store_stack_parameter (LIR_Opr opr, int offset_from_sp_in_words);
  void pop_item(LIR_Opr opr);

  void emit_slow_case_stubs();
  void emit_call_stubs();

  int  emit_exception_handler();
  void emit_osr_entry(IRScope* scope, int number_of_locks, Label* continuation, int osr_bci);

  void opr2local  (int local_name, LIR_Opr opr);
  void move(LIR_Opr src, RInfo dst);
  void move(LIR_Opr src, LIR_Opr dst) { lir()->move(src, dst); }

  void jobject2reg_with_patching(RInfo r, ciObject* obj, CodeEmitInfo* info);

  void round(int spill_ix, LIR_Opr opr);
  void spill(int spill_ix, LIR_Opr opr);

  void move_spill    (int to_spill_ix, int from_spill_ix, ValueType* type, RInfo tmp);

  void null_check                 (LIR_Opr opr, CodeEmitInfo* info);
  void explicit_div_by_zero_check (LIR_Opr opr, CodeEmitInfo* info);
  // this loads the length and compares against the index
  void array_range_check          (LIR_Opr array, LIR_Opr index, CodeEmitInfo* null_check_info, CodeEmitInfo* range_check_info);
  // this expects the length to be loaded into a register and compares against the index
  void length_range_check         (LIR_Opr length, LIR_Opr index, CodeEmitInfo* range_check_info);

  void array_store_check(LIR_Opr array, LIR_Opr value, RInfo tmp1, RInfo tmp2, RInfo tmp3, CodeEmitInfo* info_for_exception);
  void array_length(RInfo dst, LIR_Opr array, CodeEmitInfo* info);

  // For java.nio.Buffer.checkIndex
  void nio_range_check            (LIR_Opr buffer, LIR_Opr index, RInfo result, CodeEmitInfo* info);

  void field_load    (RInfo dst, ciField* field, LIR_Opr object, int offset, bool needs_patching, bool is_loaded, CodeEmitInfo* info);
  void field_store   (ciField* field, LIR_Opr object, int offset, LIR_Opr value, bool needs_patching, bool is_loaded, CodeEmitInfo* info, RInfo temp_reg);
  void indexed_load  (RInfo dst, BasicType dst_type, LIR_Opr array, LIR_Opr index, CodeEmitInfo* info);
  void indexed_store (BasicType val_type, LIR_Opr array, LIR_Opr index, LIR_Opr value, RInfo temp_reg, CodeEmitInfo* info);

  void negate        (RInfo dst, LIR_Opr value);
  void math_intrinsic(ciMethod::IntrinsicId id,  RInfo dst, LIR_Opr value, RInfo thread_cache = norinfo);

  void arithmetic_op_int  (Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, RInfo tmp);
  void arithmetic_op_long (Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, CodeEmitInfo* info = NULL);
  void arithmetic_op_fpu  (Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, bool is_strictfp);
  void arithmetic_idiv    (Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, RInfo scratch, CodeEmitInfo* info);

  // Bitfield used to pass argument information to native runtime
  enum native_arg_type {
    native_arg_no_longs = 0,
    native_arg0_is_long = 1,
    native_arg1_is_long = 2,
    native_return_is_long = 4
  };

  void arithmetic_call_op (Bytecodes::Code code, RInfo temp_reg);

  void shift_op   (Bytecodes::Code code, RInfo dst_reg, LIR_Opr value, LIR_Opr count, RInfo tmp);

  void logic_op   (Bytecodes::Code code, RInfo dst_reg, LIR_Opr left, LIR_Opr right);
  void compare_op (Bytecodes::Code code, RInfo dst_reg, LIR_Opr left, LIR_Opr right);

  void convert_op (Bytecodes::Code code, LIR_Opr src, RInfo dst_reg, bool is_32bit = false);
  void call_convert_op(Bytecodes::Code code, RInfo tmp_reg);

  void call_op(Bytecodes::Code code, const BasicTypeArray* sig_types, CodeEmitInfo* info, int vtable_index, bool optimized, bool needs_null_check, RInfo receiver, LIR_Opr result);

  void throw_op (LIR_Opr exceptionItem, RInfo exceptionOop, RInfo exceptionPc, CodeEmitInfo* info);

  void monitor_enter (RInfo object, RInfo lock, RInfo hdr, RInfo scratch, int monitor_no, CodeEmitInfo* info_for_exception, CodeEmitInfo* info);
  void monitor_exit  (RInfo object, RInfo lock, RInfo hdr, int monitor_no);

  void goto_op (BlockBegin* dst, CodeEmitInfo* info);

  void if_op(int phase, If::Condition cond, LIR_Opr x, LIR_Opr y,
             BlockBegin* t_dest, BlockBegin* f_dest, BlockBegin* u_dest, CodeEmitInfo* safepoint = NULL);
  void tableswitch_op  (LIR_Opr tag, int match, BlockBegin* dest);
  void lookupswitch_op (LIR_Opr tag, int key,   BlockBegin* dest);
  void lookupswitch_range_op (LIR_Opr tag, int low_key, int high_key, BlockBegin* dest);

  void ifop_phase2     (RInfo dst, LIR_Opr tval, LIR_Opr fval, Instruction::Condition cond);
  void ifop_phase1     (Instruction::Condition cond, LIR_Opr x, LIR_Opr y);

  void new_instance    (RInfo dst, ciInstanceKlass* klass, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo klass_reg, CodeEmitInfo* info);
  void new_type_array  (RInfo dst, BasicType elem_type, LIR_Opr length, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo scratch4, RInfo klass_reg, CodeEmitInfo* info);
  void new_object_array(RInfo dst, ciKlass* elem_klass, LIR_Opr length, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo scratch4, RInfo klass_reg, CodeEmitInfo* info, CodeEmitInfo* patching_info);
  void new_multi_array (RInfo dst, ciKlass* klass, int rank, RInfo tmp, CodeEmitInfo* info, CodeEmitInfo* patching_info);

  void instanceof_op (LIR_Opr dst_reg, LIR_Opr obj, ciKlass* k, RInfo k_RInfo, RInfo klass_RInfo, bool fast_check, CodeEmitInfo* patching_info);
  void instance_of_test_op(RInfo dst_reg, LIR_Opr obj_item, ciKlass* k, RInfo klass_ri, BlockBegin* is_instance, BlockBegin* not_is_instance, bool is_inverted, CodeEmitInfo* info, CodeEmitInfo* patching_info);

  void nop();
  void safepoint_nop(CodeEmitInfo* info);
  void align_backward_branch();

  void getClass(RInfo dst, RInfo receiver, CodeEmitInfo* info);

  // Moved from private to public because needed by CAS ops -dl
  void write_barrier(LIR_Opr value, LIR_Opr tmp);

  // Unsafe ops are implemented with CPU-specific code to be able
  // to take advantage of scaled index addressing mode on Intel
  void put_raw_unsafe(LIR_Opr address, LIR_Opr index, int log2_scale, LIR_Opr value, BasicType type);
  void get_raw_unsafe(RInfo dst, LIR_Opr address, LIR_Opr index, int log2_scale, BasicType type);
  void put_Object_unsafe(LIR_Opr src, LIR_Opr offset, LIR_Opr data, BasicType type, bool is_volatile);
  void get_Object_unsafe(RInfo dest, LIR_Opr src, LIR_Opr offset, BasicType type, bool is_volatile);

  void return_op_prolog (int monitor_no);

  void return_op        (LIR_Opr opr);

  // libraries
  void set_24bit_fpu_precision();
  void restore_fpu_precision();

  void breakpoint();
  void handler_entry();

  void std_entry(IRScope* scope, intStack* init_locals, RInfo receiver, RInfo ic_klass);

  void membar();
  void membar_acquire();
  void membar_release();

  // machine dependent
  void cmp_mem_int(LIR_OpBranch::LIR_Condition condition, RInfo base, int disp, int c, CodeEmitInfo* info);
  void cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, RInfo base, int disp, BasicType type, CodeEmitInfo* info);
  void cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, RInfo base, RInfo disp, BasicType type, CodeEmitInfo* info);
  void call_slow_subtype_check(RInfo sub, RInfo super, RInfo result);
};

