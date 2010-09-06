#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIROptimizer.hpp	1.23 04/03/31 18:13:06 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class IR;
class BlockBegin; 

define_array(LabelList, Label*);
define_stack(LabelStack, LabelList);

class LIR_PeepholeState {
 private:
  intStack*         _register_values;
  intStack*         _stack_values;
  LIR_OprList*      _value_map;
  int               _value_number;
  intStack*         _defining_op;
  intStack*         _refcount;
  intStack*         _safe_to_delete;
  LabelStack        _forward_branches;
  LocalMapping*     _locals;
  bool              _disable_optimization;

#ifndef PRODUCT
  int               _state_number;
  int               _last_state_number;
  LIR_OprList*      _register_names;
#endif

  int stack2index(LIR_Opr opr);
  int stack2indexLo(LIR_Opr opr);
  int stack2indexHi(LIR_Opr opr);
  int reg2index(LIR_Opr opr);
  int reg2indexLo(LIR_Opr opr);
  int reg2indexHi(LIR_Opr opr);

  void kill_stack_slot(int n);
  void kill_register(int n);
  void set_defining_op(int index, int op_index);

 public:
  LIR_PeepholeState();

  // initialize all state.
  void initialize(LocalMapping* locals);

  // clear the modeled register and stack state
  void clear_values();
  void clear_stack_values();

  int record_opr_reference(LIR_Opr opr);
  void record_defining_op(LIR_Opr opr, int index);
  int defining_op(LIR_Opr opr);

  void start_forward_branch(Label* label);
  void finish_forward_branch(Label* label);

  void set_disable_optimization(bool flag);
  bool should_disable_optimization() const          { return _disable_optimization; }

  void increment_ref(int op_index);
  int refcount(int index) const                     { return (index >= 0 && index < _refcount->length()) ? _refcount->at(index) : 0; }

  void mark_safe_to_delete(int op_index);
  bool is_safe_to_delete(int op_index) const;

  // model the effect of a move instruction
  void do_move(LIR_Opr s, LIR_Opr d);

  // model the effect of a move instruction
  void do_call();

  // kill any registers which refer to this value and update the defining op table
  void kill_operand(LIR_Opr opr, int op_index);

  // kill any registers which refer to this value but don't update defining op table
  void kill_equivalents(LIR_Opr opr);

  // produce operands with equivalent values, or return the original value
  // if there an no better values.
  LIR_Opr equivalent_register_or_constant(LIR_Opr opr);
  LIR_Opr equivalent_register_or_stack(LIR_Opr opr);
  LIR_Opr equivalent_register(LIR_Opr opr);
  LIR_Opr equivalent_address(LIR_Opr opr);
  LIR_Opr equivalent_opr(LIR_Opr opr);

  void print(bool force = true) PRODUCT_RETURN;
};





class LIR_Optimizer : public LIR_AbstractAssembler {
 private:
  IR* _ir;
  LIR_PeepholeState   _state;
  LocalMapping*       _locals;
  LIR_OpVisitState    _visit_state;

  int                 _op_index;
  LIR_Op*             _last_op;
  BlockBegin*         _block;

  bool                _saw_fpu2cpu_moves;

#ifndef PRODUCT
  bool              _did_stack;
  bool              _did_opto;
  static unsigned int _opto_count;
#endif

  void set_block(BlockBegin* block)              { _block = block; }
  BlockBegin* block() const                      { return _block; }
  LIR_Op* op() const                             { return _visit_state.op(); }
  LIR_OpVisitState& visit_state()                { return _visit_state; }

  // returns true if result_substitute should be called
  bool optimize_move(LIR_Op1* move, LIR_Opr& src, LIR_Opr& dst);

  void process_move(LIR_Op1* op1);
  void result_substitute();

  void visit() {
    _visit_state.visit(op());
  }

 public:
  LIR_Optimizer(IR * ir);
  IR* ir() const                                 { return _ir; }
  void optimize();

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
  void emit_code_stub(CodeStub* op);

  bool allow_opto();
  void print_if_changed(LIR_Op* op) PRODUCT_RETURN;
  LIR_Opr record_opto(LIR_Opr opr, LIR_Opr new_opr);
  LIR_Opr maybe_opto(LIR_Opr opr, LIR_Opr new_opr) {
    if (opr->is_register()) {
      if (opr->is_destroyed()) {
        // don't optimize registers which are going to be destroyed
        return opr;
      }
      // don't replace a caching register with some other random register
      // since it's better to just use the caching register, and may not be
      // correct relative to the oopmaps
      if (new_opr->is_register()) {
        if (is_cache_reg(opr)) {
          return opr;
        }
      }
    }

    if (new_opr->is_valid() && opr != new_opr && allow_opto()) {
      record_opto(opr, new_opr);
      return new_opr;
    } else {
      return opr;
    }
  }

  void block_prolog();
  void block_epilog();

  void process_op();

  void record_opr_reference(LIR_Opr opr);

  LIR_Op* op_at(int index);
  void replace_op(int index, LIR_Op* new_op);
  LIR_Op* defining_op(LIR_Opr opr);
  LIR_Op* next_op_with_code(LIR_Code code);


  LIR_Opr handle_opr(LIR_Opr opr, LIR_OpVisitState::OprMode mode);
  void handle_info(CodeEmitInfo* info);

  bool is_cache_reg(LIR_Opr opr);
  bool is_cached(LIR_Opr opr);
  bool opr_live_on_entry(LIR_Opr opr);
  bool opr_live_on_exit(LIR_Opr opr);

  LIR_Opr replace_stack_opr(LIR_Opr opr);
  void record_register_oops(CodeEmitInfo* info);

  void optimize(BlockList* blocks);
  void optimize(BlockBegin* block);

  bool saw_fpu2cpu_moves() const { return _saw_fpu2cpu_moves; }

  #include "incls/_c1_LIROptimizer_pd.hpp.incl"
};
