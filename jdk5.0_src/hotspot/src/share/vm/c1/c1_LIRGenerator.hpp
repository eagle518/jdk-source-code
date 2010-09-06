#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIRGenerator.hpp	1.6 03/12/23 16:39:13 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// The classes responsible for code emission and register allocation

#ifndef PRODUCT

class LIRGenerator;
class LIREmitter;
class CallingConvention;
class Invoke;
class SwitchLookupRange;
class LIRItem;

define_array(LIRItemArray, LIRItem*)
define_stack(LIRItemList, LIRItemArray)

class SwitchLookupRange: public CompilationResourceObj {
 public:
  int _low_key;
  int _high_key;
  BlockBegin* _sux;
 public:
  SwitchLookupRange(int start_key, BlockBegin* sux): _low_key(start_key), _high_key(start_key), _sux(sux) {}
  void set_high_key(int key) { _high_key = key; }

  int high_key() const { return _high_key; }
  int low_key() const { return _low_key; }
  BlockBegin* sux() const { return _sux; }
};

define_array(SwitchLookupRangeArray, SwitchLookupRange*)
define_stack(SwitchLookupRangeList, SwitchLookupRangeArray)


// only the classes below belong in the same file
class LIRGenerator: public InstructionVisitor, public BlockClosure {
 private:
  Compilation*  _compilation;
  LIR_Emitter*  _emit;      // code/LIR emitter
  ciMethod*     _method;    // method that we are compiling
  BlockBegin*   _block;
  LIRGenerator* _gen;
  int           _virtual_register_number;
  c1_RegMask    _byte_reg_mask;

  // data
  Value                  _value;

  void block_do_prolog(BlockBegin* block);
  void block_do_epilog(BlockBegin* block);

  void free_reg(LIR_Opr reg) {}

  bool is_strict_fp() const                      { return method()->is_strict(); }
  bool is_single_precision() const               { return block()->is_set(BlockBegin::single_precision_flag); }

  // register allocation
  LIR_Opr rlock(Value instr);                      // lock a free register
  LIR_Opr rlock(Value instr, c1_RegMask mask);
  LIR_Opr rlock_result(Value instr);
  LIR_Opr rlock_result(Value instr, BasicType type);
  LIR_Opr set_with_result_register(Value instr);

  void  set_result(Value x, LIR_Opr opr)           { x->set_operand(opr); }
  void  set_result(Value x, RInfo reg)             { set_result(x, LIR_OprFact::rinfo(reg)); }
  void  set_no_result(Value x)                     { assert(!x->has_uses(), "can't have use"); x->clear_operand(); }

  friend class LIRItem;

  void round_spill_item(LIR_Opr opr, bool round_op);
  void round_item(LIR_Opr opr);

  // spilling
  void  spill_values_on_stack (ValueStack* stack, RInfo hide_reg = RInfo(), bool caller_to_callee = false);
  void  spill_caller_save();
  bool  try_caller_to_callee(Value value, RInfo hide_reg);

  void  move_to_phi(ValueStack* stack);
  void  setup_phis_for_switch(LIR_Opr tag, ValueStack* stack);

  // code emission
  void  finish_root          (Value instr);
  void  check_float_register (LIR_Opr opr );

  void  load_item_patching(IRScope* scope, int bci_of_patched_bytecode, ValueStack* state);

  bool  fpu_fanout_handled();

  void  move_spill_to   (int spill_ix, int least_spill_ix, Value v);

  void  check_result_usage (Value instr); // release reg/spill if instr->use_count is 0
  void  release_roots(ValueStack* stack);

  void do_ArithmeticOp_Long   (ArithmeticOp*    x);
  void do_ArithmeticOp_Int    (ArithmeticOp*    x);
  void do_ArithmeticOp_FPU    (ArithmeticOp*    x);

  void do_getClass(Intrinsic* x);
  void do_currentThread(Intrinsic* x);
  void do_MathIntrinsic(Intrinsic* x);
  void do_ArrayCopy(Intrinsic* x);

  void do_NIOCheckIndex(Intrinsic* x);

  // statics
  static RInfo exceptionOopRInfo();
  static RInfo exceptionPcRInfo();
  static RInfo return1RInfo();
  static RInfo callee_return1RInfo();
  static RInfo divInRInfo();
  static RInfo divOutRInfo();
  static RInfo remOutRInfo();
  static RInfo shiftCountRInfo();
  static RInfo return2RInfo();         // for long result
  static RInfo callee_return2RInfo();
  static RInfo returnF0RInfo();
  static RInfo returnD0RInfo();
  static RInfo callee_receiverRInfo();
  static RInfo syncTempRInfo();
  static RInfo nonReturnRInfo();       // an rinfo that is not a return register
  static RInfo icKlassRInfo();         // an rinfo that is not a return register
  static RInfo scratch1_RInfo();

  static bool is_caller_save_register(RInfo reg);
  ciObject* LIRGenerator::get_jobject_constant(Value value);

  void invoke_do_arguments (Invoke* x);
  void invoke_do_spill     (Invoke* x, RInfo receiver_reg = RInfo());
  void invoke_do_result    (Invoke* x, bool needs_null_check, LIR_Opr receiver = NULL);

  LIRItemList* invoke_visit_arguments(Invoke* x, CallingConvention* arg_loc);
  void invoke_load_arguments(Invoke* x, LIRItemList* args, CallingConvention* arg_loc);

  void goto_default_successor(BlockEnd* b);


  //---------
  Value compute_phi_arrays(ValueStack* stack, Values* phi_values, intStack* spill_ixs);

  bool is_backward_branch(Value instr, BlockBegin* dest) const;
  bool must_round(Value x, LIR_Opr hint);
  bool is_32bit_mode() { Unimplemented(); return false; }

  void print_at_do_root(Value instr, bool before_visit) PRODUCT_RETURN;
  void print_at_walk(Value instr, bool before_visit) PRODUCT_RETURN;
  void print_spilling(int bci, int spill_ix) PRODUCT_RETURN;

  bool is_simm13(LIR_Opr opr) const; // for SPARC

  // machine preferences and characteristics
  bool can_inline_any_constant() const; 
  bool prefer_alu_registers() const; 
  bool can_inline_as_constant(Value i) const;

  void check_for_spill(LIR_Opr opr, RInfo reg);

  CodeEmitInfo* state_for(Instruction* x, ValueStack* state);
  CodeEmitInfo* state_for(Instruction* x);

 public:
  Compilation*  compilation() const              { return _compilation; }
  LIR_Opr       hint() const                     { return LIR_OprFact::illegalOpr; }
  LIR_Opr       result() const                   { return _value->operand(); }
  Value         value() const                    { return _value; }
  ValueType*    type() const                     { return _value->type(); }
  LIR_Emitter*  emit() const                     { return _emit; }
  ciMethod*     method() const                   { return _method; }
  BlockBegin*   block() const                    { return _block; }
  IRScope*      scope() const                    { return block()->scope(); }

  static LIR_Opr result_register_for(ValueType* type, bool callee = false);

  LIR_Opr new_register(BasicType type);
  LIR_Opr new_register(Value value)              { return new_register(as_BasicType(value->type())); }
  LIR_Opr new_register(ValueType* type)          { return new_register(as_BasicType(type)); }

  void block_do(BlockBegin* block);

  static RInfo receiverRInfo();

  // This constructor is called as the main root object that passes
  // LIR_Emitter and register allocation structure to other LIRGenerator objects
  LIRGenerator(Compilation* compilation, LIR_Emitter* emit, ciMethod* method)
    : _compilation(compilation)
    , _value(NULL) 
    , _method(method)
    , _emit(emit)
    , _virtual_register_number(0) {}

  void set_block(BlockBegin* block)              { _block = block; }

  void block_prolog(BlockBegin* block);
  void block_epilog(BlockBegin* block);

  void do_root (Instruction* instr);
  void walk    (Instruction* instr);

  void bind_block_entry(BlockBegin* block);
  void start_block(BlockBegin* block);

  void exception_handler_start(IRScope* scope, int bci, ValueStack* lock_stack);

  // visitor functionality
  virtual void do_Phi            (Phi*             x);
  virtual void do_Local          (Local*           x);
  virtual void do_Constant       (Constant*        x);
  virtual void do_LoadLocal      (LoadLocal*       x);
  virtual void do_StoreLocal     (StoreLocal*      x);
  virtual void do_LoadField      (LoadField*       x);
  virtual void do_StoreField     (StoreField*      x);
  virtual void do_ArrayLength    (ArrayLength*     x);
  virtual void do_LoadIndexed    (LoadIndexed*     x);
  virtual void do_StoreIndexed   (StoreIndexed*    x);
  virtual void do_CachingChange  (CachingChange*   x);
  virtual void do_NegateOp       (NegateOp*        x);
  virtual void do_ArithmeticOp   (ArithmeticOp*    x);
  virtual void do_ShiftOp        (ShiftOp*         x);
  virtual void do_LogicOp        (LogicOp*         x);
  virtual void do_CompareOp      (CompareOp*       x);
  virtual void do_IfOp           (IfOp*            x);
  virtual void do_Convert        (Convert*         x);
  virtual void do_NullCheck      (NullCheck*       x);
  virtual void do_Invoke         (Invoke*          x);
  virtual void do_NewInstance    (NewInstance*     x);
  virtual void do_NewTypeArray   (NewTypeArray*    x);
  virtual void do_NewObjectArray (NewObjectArray*  x);
  virtual void do_NewMultiArray  (NewMultiArray*   x);
  virtual void do_CheckCast      (CheckCast*       x);
  virtual void do_InstanceOf     (InstanceOf*      x);
  virtual void do_MonitorEnter   (MonitorEnter*    x);
  virtual void do_MonitorExit    (MonitorExit*     x);
  virtual void do_Intrinsic      (Intrinsic*       x);
  virtual void do_BlockBegin     (BlockBegin*      x);
  virtual void do_Goto           (Goto*            x);
  virtual void do_If             (If*              x);
  virtual void do_IfInstanceOf   (IfInstanceOf*    x);
  virtual void do_TableSwitch    (TableSwitch*     x);
  virtual void do_LookupSwitch   (LookupSwitch*    x);
  virtual void do_Return         (Return*          x);
  virtual void do_Throw          (Throw*           x);
  virtual void do_Base           (Base*            x);
  virtual void do_UnsafeGetRaw   (UnsafeGetRaw*    x);
  virtual void do_UnsafePutRaw   (UnsafePutRaw*    x);
  virtual void do_UnsafeGetObject(UnsafeGetObject* x);
  virtual void do_UnsafePutObject(UnsafePutObject* x);
};


class LIRItem: public CompilationResourceObj {
  Value         _value;
  LIRGenerator* _gen;
  LIR_Opr       _result;
  bool          _destroys_register;
 public:
  LIRItem(Value value, LIRGenerator* gen) {
    _destroys_register = false;
    _value = value;
    _gen = gen;
    _gen->walk(_value);
    _result = _value->operand();
  }

  Value value() const          { return _value;          }
  ValueType* type() const      { return value()->type(); }
  LIR_Opr result() const       { if (_destroys_register && _result->is_register()) return _result->make_destroyed(); else return _result;         }

  RInfo get_register() const   { return _result->rinfo(); }

  void set_result(LIR_Opr opr) {  if (opr->is_illegal()) value()->clear_operand(); else value()->set_operand(opr); _result = opr; }

  void load_item();
  void load_byte_item();
  void load_nonconstant();
  void load_item_with_reg_mask(c1_RegMask mask);
  void load_item_force(RInfo reg);

  void dont_load_item() {
    // do nothing
  }

  void set_destroys_register() {
    _destroys_register = true;
  }

  void handle_float_kind() {
    // XXX do nothing?
  }
  bool is_constant() const { return value()->as_Constant() != NULL; }
  bool is_stack() const    { return result()->is_stack(); }
  bool is_register() const { return result()->is_register(); }

  ciObject* get_jobject_constant() const;
  jint      get_jint_constant() const;
  jlong     get_jlong_constant() const;
  jfloat    get_jfloat_constant() const;
  jdouble   get_jdouble_constant() const;
  jint      get_address_constant() const;
};

class LIRHideReg: public StackObj {
 private:
  RInfo _reg;
 public:
  LIRHideReg(LIRGenerator* lirgen, ValueType* type) {
    _reg = lirgen->new_register(type)->rinfo();
  }

  LIRHideReg(LIRGenerator* lirgen, c1_RegMask mask) {
    _reg = lirgen->new_register(T_INT)->rinfo();
    lirgen->emit()->set_bailout("LIRHideReg with mask unimplemented");
  }

  RInfo reg() const { return _reg; }
};


#endif // PRODUCT
