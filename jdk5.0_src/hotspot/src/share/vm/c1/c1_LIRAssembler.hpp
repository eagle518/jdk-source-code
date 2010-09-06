#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIRAssembler.hpp	1.99 04/04/20 15:56:13 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class FPUConstEntry: public CompilationResourceObj {
 private:
  address    _pc;
  const JavaValue _fp_value;
 public:
  FPUConstEntry(jfloat f):  _fp_value (JavaValue(f))  { _pc = NULL; }
  FPUConstEntry(jdouble d): _fp_value (JavaValue(d))  { _pc = NULL; }

  void set_pc(address pc) { _pc = pc; }

  bool is_float()   const { return _fp_value.get_type() == T_FLOAT; }
  bool is_double()  const { return _fp_value.get_type() == T_DOUBLE; }

  jfloat  float_value()  const { assert (is_float() , "wrong type"); return _fp_value.get_jfloat(); }
  jdouble double_value() const { assert (is_double(), "wrong type"); return _fp_value.get_jdouble(); }

  address pc()           const { return _pc; }
};


define_array(FPUConstEntryArray, FPUConstEntry*)
define_stack(FPUConstEntryList, FPUConstEntryArray)


class ConstantTable: public StackObj {
 private:
  FPUConstEntryList* _entries;
 public:
  ConstantTable ();
  void append_float  (jfloat f);
  void append_double (jdouble d);

  address address_of_float_constant  (jfloat f);
  address address_of_double_constant (jdouble d);

  void emit_entries(MacroAssembler* masm, bool littleEndian);

#ifndef PRODUCT
  void print();
#endif
};


class Compilation;
class ScopeValue;

class LIR_AbstractAssembler: public CompilationResourceObj {
public:
  virtual void emit_op0(LIR_Op0* op) = 0;
  virtual void emit_op1(LIR_Op1* op) = 0;
  virtual void emit_op2(LIR_Op2* op) = 0;
  virtual void emit_op3(LIR_Op3* op) = 0;
  virtual void emit_opBranch(LIR_OpBranch* op) = 0;
  virtual void emit_opLabel(LIR_OpLabel* op) = 0;
  virtual void emit_arraycopy(LIR_OpArrayCopy* op) = 0;
  virtual void emit_opConvert(LIR_OpConvert* op) = 0;
  virtual void emit_alloc_obj(LIR_OpAllocObj* op) = 0;
  virtual void emit_alloc_array(LIR_OpAllocArray* op) = 0;
  virtual void emit_lock(LIR_OpLock* op) = 0;
  virtual void emit_call(LIR_OpJavaCall* op) = 0;
  virtual void emit_rtcall(LIR_OpRTCall* op) = 0;
  virtual void emit_delay(LIR_OpDelay* op) { Unimplemented(); }
  virtual void emit_code_stub(CodeStub* op) = 0;
  virtual void emit_opTypeCheck(LIR_OpTypeCheck* op) = 0;
  virtual void emit_compare_and_swap(LIR_OpCompareAndSwap* op) = 0;
};

class LIR_Assembler: public LIR_AbstractAssembler {
 private:
  friend class LIR_CodeGen;

  CodeOffsets*       _offsets;
  C1_MacroAssembler* _masm;
  CodeStubList*      _slow_case_stubs;
  CodeStubList*      _call_stubs;

  Compilation*       _compilation;
  FrameMap*          _frame_map;
  Label              _throw_entry_label;         // entry point for throw_op for throw
  Label              _unwind_entry_label;        // entry point for throw_op for unwind
  Label              _vep_label;
  ConstantTable      _const_table;
  BlockBegin*        _current_block;
  bool               _bailout;
  int                _last_debug_info_pc_offset;

  void set_bailout(const char* s);

  FrameMap* frame_map() const { return _frame_map; }

  void set_current_block(BlockBegin* b) { _current_block = b; }
  BlockBegin* current_block() const { return _current_block; }

  // code emission patterns and accessors
  void check_codespace();
  bool needs_icache(ciMethod* method) const;
  // returns offset of icache check
  int check_icache();
  int check_icache(Register receiver, Register ic_klass);
  Address receiver_address_in_prolog(ciMethod* method);
  void jvmpi_method_enter(CodeEmitInfo* info); 
  void jvmpi_method_exit(ciMethod* method, bool result_is_oop);

  void ciObject2reg(ciObject* con, Register reg);
  void jobject2reg(jobject o, Register reg);
  void jobject2reg_with_patching(Register reg, CodeEmitInfo* info);

  void emit_stubs(CodeStubList* stub_list);

  // addresses
  Address as_ArrayAddress(LIR_Address* addr, BasicType type);
  Address as_Address(LIR_Address* addr);

  // debug information
  void add_call_info(int pc_offset, CodeEmitInfo* cinfo);
  void add_debug_info_for_branch(CodeEmitInfo* info);
  void add_debug_info_for_div0(int pc_offset, CodeEmitInfo* cinfo);
  void add_debug_info_for_div0_here(CodeEmitInfo* info);
  void add_debug_info_for_null_check(int pc_offset, CodeEmitInfo* cinfo);
  void add_debug_info_for_null_check_here(CodeEmitInfo* info);

  void trace_method_entry(jint con);

  void fpu_pop(RInfo reg);
  void fpu_push(RInfo reg);
  void fpu_on_tos(RInfo reg);
  void fpu_two_on_tos(RInfo tos0, RInfo tos1, bool must_be_ordered);
  bool must_swap_two_on_tos(RInfo tos0, RInfo tos1);
  void set_24bit_FPU();
  void reset_FPU();
  void remove_fpu_result(RInfo reg);
  void set_fpu_stack_empty();
  void fpop();

  void breakpoint();
  void push(LIR_Opr opr);
  void pop(LIR_Opr opr);

  void setup_locals_at_entry();

  // patching
  void append_patching_stub(PatchingStub* stub);
  void patching_epilog(PatchingStub* patch, LIR_Op1::LIR_PatchCode patch_code, Register obj, CodeEmitInfo* info);

  // conversions
  RInfo base_of(LIR_Address* addr);
  int offset_of(LIR_Address* addr);

  void comp_op(LIR_OpBranch::LIR_Condition condition, LIR_Opr src, LIR_Opr result, BasicType type);
  void monitorenter(RInfo obj_, RInfo lock_, Register hdr, Register scratch, int monitor_no, CodeEmitInfo* info);

 public:
  LIR_Assembler(Compilation* c, CodeOffsets* offsets);
  ~LIR_Assembler();
  C1_MacroAssembler* masm() const                { return _masm; }
  Compilation* compilation() const               { return _compilation; }
  ciMethod* method() const                       { return compilation()->method(); }

  CodeOffsets* offsets() const                   { return _offsets; }
  int code_offset() const;
  address pc() const;

  int  initial_frame_size_in_bytes();

  // test for constants which can be encoded directly in instructions
  static bool is_small_constant(LIR_Opr opr);
  static bool is_single_instruction(LIR_Op* op, FrameMap* frame_map);

  // stubs
  void emit_slow_case_stubs();
  void emit_call_stubs();
  void emit_code_stub(CodeStub* op);
  void load_receiver_reg(Register reg);
  void add_call_info_here(CodeEmitInfo* info)                              { add_call_info(code_offset(), info); }

  // code patterns
  int  emit_exception_handler();
  void emit_osr_entry(IRScope* scope, int number_of_locks, Label* continuation, int osr_bci);
  void emit_constants();
  void emit_code(BlockList* hir);

  void emit_method_entry      (LIR_Emitter* emit, IRScope* scope);
  void emit_native_call       (address native_entry, CodeEmitInfo* info);
  void emit_native_method_exit(CodeEmitInfo* info);
  void emit_string_compare(IRScope* scope);
  void emit_triglib(ciMethod::IntrinsicId trig_id);


  void return_op(RInfo result, bool result_is_oop);
  void safepoint_poll(RInfo result, CodeEmitInfo* info);

  void const2reg(LIR_Const* c, RInfo to_reg, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info);
  void const2stack(LIR_Const* c, int stack_index);
  void const2mem(LIR_Const* c, LIR_Address* addr, BasicType type, CodeEmitInfo* info);
  void const2array(LIR_Const* from_const, LIR_Address* addr, BasicType type, CodeEmitInfo* info);

  void stack2reg(LIR_Opr src, LIR_Opr dest, BasicType type);
  void array2reg(LIR_Address* addr, RInfo to_reg, BasicType type, CodeEmitInfo* info);
  void mem2reg(LIR_Address* from_addr, RInfo to_reg, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info);

  void reg2stack(RInfo from_reg, int to_stack_index, BasicType type);
  void reg2reg(RInfo from_reg, RInfo to_reg);
  void reg2mem(RInfo from_reg, LIR_Address* addr, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info);
  void reg2array(RInfo from_rinfo, LIR_Address* addr, BasicType type, CodeEmitInfo* info);

  void shift_op(LIR_Code code, RInfo left, RInfo count, RInfo dest, RInfo tmp);
  void shift_op(LIR_Code code, RInfo left, jint  count, RInfo dest);

  void move_regs(Register from_reg, Register to_reg);
  void swap_reg(Register a, Register b);
  void push_parameter(Register r, int offset_from_esp_in_words);
  void push_parameter(jint c,     int offset_from_esp_in_words);

  void emit_op0(LIR_Op0* op);
  void emit_op1(LIR_Op1* op);
  void emit_op2(LIR_Op2* op);
  void emit_op3(LIR_Op3* op);
  void emit_opBranch(LIR_OpBranch* op);
  void emit_opLabel(LIR_OpLabel* op);
  void emit_arraycopy(LIR_OpArrayCopy* op);
  void emit_opConvert(LIR_OpConvert* op);
  void emit_alloc_obj(LIR_OpAllocObj* op);
  void emit_alloc_array(LIR_OpAllocArray* op);
  void emit_opTypeCheck(LIR_OpTypeCheck* op);
  void emit_compare_and_swap(LIR_OpCompareAndSwap* op);
  void emit_lock(LIR_OpLock* op);
  void emit_call(LIR_OpJavaCall* op);
  void emit_rtcall(LIR_OpRTCall* op);

  void arith_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dest, CodeEmitInfo* info = NULL);
  void arithmetic_idiv(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr temp, LIR_Opr result, CodeEmitInfo* info);
  void intrinsic_op(LIR_Code code, LIR_Opr value, LIR_Opr unused, LIR_Opr dest);

  void logic_op(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr dest);

  void round32_op(LIR_Opr src, LIR_Opr dest);
  void array_move_op(LIR_Opr src, LIR_Opr dest, BasicType type, CodeEmitInfo* info);
  void move_op(LIR_Opr src, LIR_Opr result, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info);
  void volatile_move_op(LIR_Opr src, LIR_Opr result, BasicType type, LIR_Op1::LIR_PatchCode patch_code, CodeEmitInfo* info);
  void comp_op(LIR_Opr src, LIR_Opr result, CodeEmitInfo* info); // info set for backward branch
  void comp_mem_op(LIR_Opr src, LIR_Opr result, BasicType type, CodeEmitInfo* info);  // info set for null exceptions
  void comp_fl2i(LIR_Code code, LIR_Opr left, LIR_Opr right, LIR_Opr result);

  void ic_call(address destination, CodeEmitInfo* info);
  void vtable_call(int vtable_offset, CodeEmitInfo* info);
  void call(address entry, relocInfo::relocType rtype, CodeEmitInfo* info);

  void build_frame();

  void throw_op(RInfo exceptionPC, RInfo exceptionOop, CodeEmitInfo* info, bool unwind);
  void monitor_address(int monitor_ix, RInfo dst);

  void align_backward_branch_target();
  void align_call(LIR_Code code);

  void negate(LIR_Opr left, LIR_Opr dest);
  void leal(LIR_Opr left, LIR_Opr dest);

  void dup_fpu(RInfo from_reg, RInfo to_reg);
  void rt_call(address dest, RInfo tmp, int size, int args);
  void new_multi_array(int rank, RInfo dest, CodeEmitInfo* info);

  void membar();
  void membar_acquire();
  void membar_release();
  void get_thread(LIR_Opr result);

  bool must_bailout() const                      { return _bailout; }

  void safepoint_nop();  // machine dependent

  #include "incls/_c1_LIRAssembler_pd.hpp.incl"
};



