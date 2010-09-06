#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_CodeGenerator.hpp	1.161 04/03/31 18:13:10 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// The classes responsible for code emission and register allocation

class DelayedSpillMark;

class LookupRange: public CompilationResourceObj {
 public:
  int _low_key;
  int _high_key;
  BlockBegin* _sux;
 public:
  LookupRange(int start_key, BlockBegin* sux): _low_key(start_key), _high_key(start_key), _sux(sux) {}
  void set_high_key(int key) { _high_key = key; }

  int high_key() const { return _high_key; }
  int low_key() const { return _low_key; }
  BlockBegin* sux() const { return _sux; }
};


class ValueGenInvariant: public StackObj {
 private:
  RegAlloc*     _ra;        // register allocator
  LIR_Emitter*  _emit;      // code/LIR emitter
  ciMethod*     _method;    // method that we are compiling
  BlockBegin*   _block;
  IRScope*      _scope;
 public:
  ValueGenInvariant(ciMethod* m, RegAlloc* ra, LIR_Emitter* ce)
    : _method(m)
    , _emit(ce)
    , _ra(ra)
    , _block(NULL) {}

  RegAlloc*     reg_alloc() const                { return _ra;     }
  LIR_Emitter*  emit() const                     { return _emit;   }
  ciMethod*     method() const                   { return _method; }
  BlockBegin*   block() const                    { return _block;  }
  IRScope*      scope() const                    { return block()->scope(); }
  void set_block(BlockBegin* block)              { _block = block; }
};


// only the classes below belong in the same file
class ValueGen: public InstructionVisitor {
 private:
  // ---------------------------STATICS-------------------------------
  static bool  _init_done;

 private:
  Compilation*       _compilation;
  DelayedSpillMark*  _spill_mark;

  // data
  const Item*        _hint;      // hints what result should look like (e.g, preferred register)
  Item*              _result;    // describes the result of do_xxx function
  ValueGenInvariant* _data;      // data that is invariant 

  Compilation*  compilation() const              { return _compilation; }
  const Item*   hint() const                     { return _hint; }
  Item*         result() const                   { return _result; }
  RegAlloc*     ra() const                       { return _data->reg_alloc(); } 
  LIR_Emitter*  emit() const                     { return _data->emit(); }
  ciMethod*     method() const                   { return _data->method(); }
  BlockBegin*   block() const                    { return _data->block(); }
  IRScope*      scope() const                    { return _data->scope(); }

  bool is_strict_fp() const                      { return method()->is_strict(); }
  bool is_single_precision() const               { return block()->is_set(BlockBegin::single_precision_flag); }

  // register allocation
  RInfo rlock                  (Value instr, const Item* hint = NULL); // lock a free register, spill if necessary, use hint if possible
  RInfo rlock_result_with_hint (Value instr, const Item* hint);
  RInfo rlock_byte_result_with_hint (Value instr, const Item* hint);
  RInfo set_with_result_register(Value instr);
  RInfo lock_free_rinfo        (Value instr, ValueType* type);         // lock a free register of a specific type, spill if necessary
  RInfo lock_free_rinfo        (Value instr, c1_RegMask mask);         // lock a free register from the register set, spill if necessary
  void  lock_spill_rinfo       (Value instr, RInfo reg);               // lock a specific register, if it's not free spill
  void  lock_spill_temp        (Value instr, RInfo reg);               // lock a specific register, if it's not free spill

  void  set_maynot_spill(Item* item);

  void  rfree(Item* item);
  void  raw_rfree(Item* item);

  bool  is_free_rinfo    (RInfo reg);

  void  set_result(Value x, RInfo reg)           {  result()->set_register(reg); }
  void  set_no_result(Value x);

  friend class HideReg;
  friend class SpillLockReg;
  friend class DelayedSpillMark;

  void item_free(Item* item);
  void release_item(Item* item);

  void round_spill_item(Item* item, bool round_op);
  void round_item(Item* item);

  // spilling
  void  sfree(Item* item);
  void  delayed_sfree(Item* item);

  void  spill_one(ValueType* type);
  void  spill_one(c1_RegMask mask);
  void  spill_register(RInfo reg);
  void  spill_item(Item* item); 
  void  spill_value(Value value);
  void  spill_values_on_stack (ValueStack* stack, RInfo hide_reg = RInfo(), bool caller_to_callee = false);
  void  spill_caller_save();
  bool  try_caller_to_callee(Value value, RInfo hide_reg);

  // returns true if a spill move was required.
  bool  move_to_phi(ValueStack* stack, bool force_tos_to_stack_for_athrow = false);
  void  setup_phis_for_switch(Item* tag, ValueStack* stack);

  // code emission
  void  check_float_register (Item* item );
  bool  must_copy_register   (Item* item );

  void  dont_load_item         (Item* item);
  void  dont_load_item_nocheck (Item* item);

  void  load_item       (Item* item);
  void  load_item_patching(IRScope* scope, int bci_of_patched_bytecode, Item* item, ValueStack* state, ExceptionScope* exception_scope);
  void  load_byte_item  (Item* item);

  bool  fpu_fanout_handled (Item* item);

  void  load_item_hint  (Item* item, const Item* hint);
  void  load_item_force (Item* item, RInfo force_reg);
  void  move_spill_to   (int spill_ix, int least_spill_ix, Value v);

  void  load_item_with_reg_mask(Item* item, c1_RegMask mask);

  void  check_item      (Item* item);

  void  release_roots(ValueStack* stack);

  void do_ArithmeticOp_Long   (ArithmeticOp*    x);
  void do_ArithmeticOp_Int    (ArithmeticOp*    x);
  void do_ArithmeticOp_FPU    (ArithmeticOp*    x);

  void do_getClass(Intrinsic* x);
  void do_currentThread(Intrinsic* x);
  void do_MathIntrinsic(Intrinsic* x);
  void do_ArrayCopy(Intrinsic* x);
  void arraycopy_helper(Intrinsic* x, int* flags, ciArrayKlass** expected_type);

  void do_NIOCheckIndex(Intrinsic* x);

  void do_FPIntrinsics(Intrinsic* x);

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

  static bool is_caller_save_register(RInfo reg);

  void invoke_do_arguments (Invoke* x);
  void invoke_do_spill     (Invoke* x, RInfo receiver_reg = RInfo());
  void invoke_do_result    (Invoke* x, bool needs_null_check, Item* receiver = NULL);

  ItemArray* invoke_visit_arguments(Invoke* x, CallingConvention* arg_loc);
  void invoke_load_arguments(Invoke* x, ItemArray* args, CallingConvention* arg_loc);

  void goto_default_successor(BlockEnd* b, CodeEmitInfo* info);


  //---------
  Value compute_phi_arrays(ValueStack* stack, Values* phi_values, intStack* spill_ixs,
                           bool force_tos_to_stack_for_athrow = false);

  bool is_backward_branch(Value instr, BlockBegin* dest) const;
  bool must_round(Value x, const Item* hint);
  bool is_32bit_mode();

  void do_LookupSwitch_key(Item* tag, LookupSwitch* x, LookupRange* range);
  void do_LookupSwitch_default(LookupSwitch* x);

  void print_at_do_root(Value instr, bool before_visit) PRODUCT_RETURN;
  void print_at_walk(Value instr, bool before_visit) PRODUCT_RETURN;
  void print_spilling(int bci, int spill_ix) PRODUCT_RETURN;

  bool is_simm13(Item* item); // for SPARC

  // machine preferences and characteristics
  bool can_inline_any_constant() const; 
  bool prefer_alu_registers() const; 
  bool can_inline_as_constant(Item* i);
  static bool safepoint_poll_needs_register();

  RInfo scratch1_RInfo() const;

  LIR_Opr item2lir(const Item* item) { return compilation()->item2lir(item); }

 public:
  static RInfo result_register_for(ValueType* type, bool callee = false);

  static RInfo receiverRInfo();
  static void init_value_gen();

  // This constructor is called as the main root object that passes
  // LIR_Emitter and register allocation structure to other ValueGen objects
  ValueGen(Compilation* compilation, ValueGenInvariant* vgi) :
    _compilation(compilation), _result(NULL), _hint(NULL), _data(vgi), _spill_mark(NULL)
  {  assert(_init_done, "ValueGen is not initialized");  }

  // This is called for each instruction and is used to visit each node;
  // It returns the state in _result.  If may_be_null is true, then it's
  // possible that there isn't an instruction associated with the Item,
  // in which case it shouldn't be walked.  This is useful for generating code for
  // instructions which have optional arguments.
  ValueGen(Item* result, const Item* hint, ValueGen* parent, bool may_be_null = false):
    _compilation(parent->compilation()), _hint(hint), _result(result), _data(parent->_data), _spill_mark(NULL)
  {
    assert(_init_done, "ValueGen is not initialized");
    if (may_be_null && result->value() == NULL) {
      return;
    }
    walk(result->value());
  }

  void set_invariant(ValueGenInvariant* vgi)     { _data = vgi; }
  void set_block(BlockBegin* block)              { _data->set_block(block); }

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
  virtual void do_CompareAndSwap (Intrinsic*       x, ValueType* type);
};


class CodeGenerator: public BlockClosure {
 private:
  ValueGen* _gen;
  int       _max_spills;
  ValueGenInvariant* _vgi;

  void block_do_prolog(BlockBegin* block);
  void block_do_epilog(BlockBegin* block);

 public:
  CodeGenerator(ValueGen* gen, ValueGenInvariant* _vgi);

  void block_do(BlockBegin* block);

  bool has_spills() const { return _max_spills > 0; }
  int  max_spills() const { return _max_spills;     }

  static void clear_instruction_items(BlockBegin* b);
};


class HideReg: public StackObj {
 private:
  ValueGen* _vg;
  RInfo     _reg;
 public:
  HideReg(ValueGen* vg, RInfo reg, bool spill = false);
  // spills, locks
  HideReg(ValueGen* vg, ValueType* t);
  HideReg(ValueGen* vg, c1_RegMask mask);
  ~HideReg();

  RInfo reg() const { return _reg; }
};







