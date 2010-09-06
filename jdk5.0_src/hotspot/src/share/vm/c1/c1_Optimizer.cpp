#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Optimizer.cpp	1.58 03/12/23 16:39:16 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_Optimizer.cpp.incl"

define_array(ValueSetArray, ValueSet*);
define_stack(ValueSetList, ValueSetArray);


Optimizer::Optimizer(IR* ir) {
  assert(ir->is_valid(), "IR must be valid");
  _ir = ir;
}


class CE_Eliminator: public BlockClosure {
 private:
  int _cee_count;                                // the number of CEs successfully eliminated

  Value simple_value_copy(Value x) {
    ValueType* type = x->type();
    if (type->is_float_kind()) return NULL;      // don't do floats for now since backend doesn't support it
    if (x->as_Constant() != NULL) return new Constant(type);
    if (x->as_LoadLocal() != NULL) return new LoadLocal(x->as_LoadLocal()->local());
    return NULL;
  }

 public:
  CE_Eliminator() : _cee_count(0)                {}
  int cee_count() const                          { return _cee_count; }

  virtual void block_do(BlockBegin* block) {
    // 1) find conditional expression
    // check if block ends with an If
    If* if_ = block->end()->as_If();
    if (if_ == NULL) return;
    // check if If works on int or object types
    // (we cannot handle If's working on long, float or doubles yet,
    // since IfOp doesn't support them - these If's show up if cmp
    // operations followed by If's are eliminated)
    { ValueType* type = if_->x()->type();
      if (!type->is_int() && !type->is_object()) return;
    }
    // check if branches push a simple value as first instruction
    // (i.e., a constant or a local) and copy the instruction if so
    Instruction* t1 = if_->tsux()->next();
    Instruction* f1 = if_->fsux()->next();
    Value tval = simple_value_copy(t1);
    if (tval == NULL) return;
    Value fval = simple_value_copy(f1);
    if (fval == NULL) return;
    // check if both branches end with a goto
    // note: next() must exist since previous instruction
    //       was simple (and thus not a BlockEnd instruction)
    assert(t1->next() != NULL && f1->next() != NULL, "next must exist");
    Goto* t2 = t1->next()->as_Goto();
    if (t2 == NULL) return;
    Goto* f2 = f1->next()->as_Goto();
    if (f2 == NULL) return;
    // check if both gotos merge into the same block
    BlockBegin* sux = t2->default_sux();
    if (sux != f2->default_sux()) return;
    // check if both branches actually "produce" a value
    // Note: both the left and the right branch could load a value and
    //       then pop it, but the load would still be there as instruction
    //       (this could happen after inlining) => we must check that the
    //       stack size has actually changed by - was bug (gri 8/20/99).
    assert(tval->type()->size() == fval->type()->size(), "sizes must match");
    if (if_->state()->stack_size() + tval->type()->size() != sux->state()->stack_size()) return;

    // 2) substitute conditional expression
    //    with an IfOp followed by a Goto
    // cut if_ away and get node before
    Instruction* if_prev = if_->prev(block);
    if_prev->set_next(NULL, 0);
    // create new nodes and connect them
    IfOp* if_op = new IfOp(if_->x(), if_->cond(), if_->y(), tval, fval);
    Goto* goto_ = new Goto(sux, if_->is_safepoint());
    int bci = if_->bci();
    if_prev
      ->set_next(tval , bci)
      ->set_next(fval , bci)
      ->set_next(if_op, bci)
      ->set_next(goto_, bci);
    // set state and join sux
    ValueStack* state = if_->state()->copy();
    state->push(if_op->type(), if_op);
    goto_->set_state(state);
    sux->join(state);
    // update block end
    block->set_end(goto_);
    state->pin_stack_all(Instruction::PinStackCEE);

    // 3) successfully eliminated a conditional expression
    _cee_count++;
    if (PrintCEE) {
      tty->print_cr("%d. CEE in B%d", cee_count(), block->id());
    }
  }
};


void Optimizer::eliminate_conditional_expressions() {
  // find conditional expressions & replace them with IfOps
  CE_Eliminator ce;
  ir()->iterate_preorder(&ce);
}


class PredecessorCounter: public BlockClosure {
 private:
  GrowableArray<intx> _count;
  void add_pred(BlockBegin* block)               { (*_count.adr_at(block->block_id()))++; }

 public:
  PredecessorCounter(int n) : _count(n, n, 0)    {}

  int pred_count(BlockBegin* block) const        { return _count.at(block->block_id()); }

  virtual void block_do(BlockBegin* block) {
    // Note: Make sure to count subroutine and exception handler paths as well in case
    //       one of them is shared with regular control flow (was bug - gri 4/10/2000).
    for (int i = block->end()->number_of_sux        (); i-- > 0;) add_pred(block->end()->sux_at       (i));
    for (int k = block->number_of_exception_handlers(); k-- > 0;) add_pred(block->exception_handler_at(k));
  }
};


class BlockMerger: public BlockClosure {
 private:
  PredecessorCounter* _pc;
  int                 _merge_count;              // the number of block pairs successfully merged

 public:
  BlockMerger(PredecessorCounter* pc)
  : _pc(pc)
  , _merge_count(0)
  {}

  int pred_count(BlockBegin* block) const        { return _pc->pred_count(block); }
  int merge_count() const                        { return _merge_count; }

  bool try_merge(BlockBegin* block) {
    BlockEnd* end = block->end();
    bool done = false;
    int i = 0;

    if (end->as_Goto() != NULL) {
      assert(end->number_of_sux() == 1, "end must have exactly one successor");
      // Note: It would be sufficient to check for the number of successors (= 1)
      //       in order to decide if this block can be merged potentially. That
      //       would then also include switch statements w/ only a default case.
      //       However, in that case we would need to make sure the switch tag
      //       expression is executed if it can produce observable side effects.
      //       We should probably have the canonicalizer simplifying such switch
      //       statements and then we are sure we don't miss these merge oportunities
      //       here (was bug - gri 7/7/99).
      BlockBegin* sux = end->default_sux();
      if (pred_count(sux) == 1 && !sux->is_entry_block()) {
        // merge the two blocks
        assert(end->state()->stack_size() == sux->state()->stack_size(), "stack sizes must correspond");
        // find instruction before end & append first instruction of sux block
        Instruction* prev = end->prev(block);
        Instruction* next = sux->next();
        assert(prev->as_BlockEnd() == NULL, "must not be a BlockEnd");
        prev->set_next(next, next->bci());
        block->set_end(sux->end());
        // add exception handlers of deleted block, if any
        for (int k = 0; k < sux->number_of_exception_handlers(); k++) {
          block->add_exception_handler(sux->exception_handler_at(k));
        }
        // substitute phi's
        int stack_size = sux->state()->stack_size();
        if (stack_size > 0) {
          // set substitutions
          for (int i = 0; i < stack_size;) {
            int i0 = i; // stack_at_inc(...) modifies i
#ifdef ASSERT
            int i1 = i;
            assert(sux->state()->stack_at_inc(i1)->as_Phi() != NULL, "must be a phi node");
#endif // ASSERT
            sux->state()->stack_at_inc(i0)->set_subst(end->state()->stack_at_inc(i));
          }
          // resolve substitutions
          sux->resolve_substitution();
        }
        // debugging output
        _merge_count++;
        if (PrintBlockElimination) {
          tty->print_cr("%d. merged B%d & B%d (stack size = %d)", merge_count(), block->block_id(), sux->block_id(), stack_size);
        }
        return true;
      }
    }
    return false;
  }

  virtual void block_do(BlockBegin* block) {
    while (try_merge(block)); // repeat since the same block may merge again
  }
};


void Optimizer::eliminate_blocks() {
  // determine predecessor count for each block
  PredecessorCounter pc(BlockBegin::number_of_blocks());
  ir()->iterate_preorder(&pc);
  // merge blocks if possible
  BlockMerger bm(&pc);
  ir()->iterate_preorder(&bm);
}


class NullCheckEliminator;
class NullCheckVisitor: public InstructionVisitor {
private:
  NullCheckEliminator* _nce;
  NullCheckEliminator* nce() { return _nce; }

public:
  NullCheckVisitor() {}

  void set_eliminator(NullCheckEliminator* nce) { _nce = nce; }

  void do_Phi            (Phi*             x);
  void do_Local          (Local*           x);
  void do_Constant       (Constant*        x);
  void do_LoadLocal      (LoadLocal*       x);
  void do_StoreLocal     (StoreLocal*      x);
  void do_LoadField      (LoadField*       x);
  void do_StoreField     (StoreField*      x);
  void do_ArrayLength    (ArrayLength*     x);
  void do_LoadIndexed    (LoadIndexed*     x);
  void do_StoreIndexed   (StoreIndexed*    x);
  void do_CachingChange  (CachingChange*   x);
  void do_NegateOp       (NegateOp*        x);
  void do_ArithmeticOp   (ArithmeticOp*    x);
  void do_ShiftOp        (ShiftOp*         x);
  void do_LogicOp        (LogicOp*         x);
  void do_CompareOp      (CompareOp*       x);
  void do_IfOp           (IfOp*            x);
  void do_Convert        (Convert*         x);
  void do_NullCheck      (NullCheck*       x);
  void do_Invoke         (Invoke*          x);
  void do_NewInstance    (NewInstance*     x);
  void do_NewTypeArray   (NewTypeArray*    x);
  void do_NewObjectArray (NewObjectArray*  x);
  void do_NewMultiArray  (NewMultiArray*   x);
  void do_CheckCast      (CheckCast*       x);
  void do_InstanceOf     (InstanceOf*      x);
  void do_MonitorEnter   (MonitorEnter*    x);
  void do_MonitorExit    (MonitorExit*     x);
  void do_Intrinsic      (Intrinsic*       x);
  void do_BlockBegin     (BlockBegin*      x);
  void do_Goto           (Goto*            x);
  void do_If             (If*              x);
  void do_IfInstanceOf   (IfInstanceOf*    x);
  void do_TableSwitch    (TableSwitch*     x);
  void do_LookupSwitch   (LookupSwitch*    x);
  void do_Return         (Return*          x);
  void do_Throw          (Throw*           x);
  void do_Base           (Base*            x);
  void do_UnsafeGetRaw   (UnsafeGetRaw*    x);
  void do_UnsafePutRaw   (UnsafePutRaw*    x);
  void do_UnsafeGetObject(UnsafeGetObject* x);
  void do_UnsafePutObject(UnsafePutObject* x);
};


// Because of a static contained within (for the purpose of iteration
// over instructions), it is only valid to have one of these active at
// a time
class NullCheckEliminator {
 private:
  static NullCheckEliminator* _static_nce;
  static void                 do_value(Value* vp);

  Optimizer*        _opt;

  ValueSet*         _visited_instructions;        // Visit each instruction only once per basic block
  BlockList*        _work_list;                   // Basic blocks to visit

  bool visited(Value x)                           { assert(_visited_instructions != NULL, "check");
                                                    return _visited_instructions->contains(x); }
  void mark_visited(Value x)                      { assert(_visited_instructions != NULL, "check");
                                                    _visited_instructions->put(x); }
  void clear_visited_state()                      { assert(_visited_instructions != NULL, "check");
                                                    _visited_instructions->clear(); }

  ValueSet*         _set;                         // current state, propagated to subsequent BlockBegins
  ValueSetList      _block_states;                // BlockBegin null-check states for all processed blocks
  NullCheckVisitor  _visitor;
  NullCheck*        _last_explicit_null_check;

  bool set_contains(Value x)                      { assert(_set != NULL, "check"); return _set->contains(x); }
  void set_put     (Value x)                      { assert(_set != NULL, "check"); _set->put(x); }
  void set_remove  (Value x)                      { assert(_set != NULL, "check"); _set->remove(x); }

  BlockList* work_list()                          { return _work_list; }

  void iterate_all();
  void iterate_one(BlockBegin* block);

  ValueSet* state()                               { return _set; }
  void      set_state_from (ValueSet* state)      { _set->set_from(state); }
  ValueSet* state_for      (BlockBegin* block)    { return _block_states[block->block_id()]; }
  void      set_state_for  (BlockBegin* block, ValueSet* stack) { _block_states[block->block_id()] = stack; }
  // Returns true if caused a change in the block's state.
  bool      merge_state_for(BlockBegin* block,
                            ValueStack* incoming_stack,
                            ValueSet*   incoming_state);

 public:
  // constructor
  NullCheckEliminator(Optimizer* opt)
    : _opt(opt)
    , _set(new ValueSet())
    , _last_explicit_null_check(NULL)
    , _block_states(BlockBegin::number_of_blocks(), NULL)
    , _work_list(new BlockList()) {
    _visited_instructions = new ValueSet();
    _visitor.set_eliminator(this);
  }
  
  Optimizer*  opt()                               { return _opt; }
  IR*         ir ()                               { return opt()->ir(); }

  // Process a graph
  void iterate(BlockBegin* root);

  // In some situations (like NullCheck(x); getfield(x)) the debug
  // information from the explicit NullCheck can be used to populate
  // the getfield, even if the two instructions are in different
  // scopes; this allows implicit null checks to be used but the
  // correct exception information to be generated. We must clear the
  // last-traversed NullCheck when we reach a potentially-exception-
  // throwing instruction, as well as in some other cases.
  void        set_last_explicit_null_check(NullCheck* check) { _last_explicit_null_check = check; }
  NullCheck*  last_explicit_null_check()                     { return _last_explicit_null_check; }
  Value       last_explicit_null_check_obj()                 { return (_last_explicit_null_check
                                                                         ? _last_explicit_null_check->obj()
                                                                         : NULL); }
  NullCheck*  consume_last_explicit_null_check()             { _last_explicit_null_check->unpin(Instruction::PinExplicitNullCheck);
                                                               return _last_explicit_null_check; }
  void        clear_last_explicit_null_check()               { _last_explicit_null_check = NULL; }

  // Handlers for relevant instructions
  // (separated out from NullCheckVisitor for clarity)

  // The basic contract is that these must leave the instruction in
  // the desired state; must not assume anything about the state of
  // the instruction. We make multiple passes over some basic blocks
  // and the last pass is the only one whose result is valid.
  void handle_LoadLocal       (LoadLocal* x);
  void handle_StoreLocal      (StoreLocal* x);
  void handle_AccessField     (AccessField* x);
  void handle_ArrayLength     (ArrayLength* x);
  void handle_LoadIndexed     (LoadIndexed* x);
  void handle_StoreIndexed    (StoreIndexed* x);
  void handle_NullCheck       (NullCheck* x);
  void handle_Invoke          (Invoke* x);
  void handle_NewInstance     (NewInstance* x);
  void handle_NewArray        (NewArray* x);
  void handle_AccessMonitor   (AccessMonitor* x);
  void handle_Intrinsic       (Intrinsic* x);
  void handle_UnsafeOp        (UnsafeOp* x);
};


// NEEDS_CLEANUP
// There may be other instructions which need to clear the last
// explicit null check. Anything across which we can not hoist the
// debug information for a NullCheck instruction must clear it. It
// might be safer to pattern match "NullCheck ; {AccessField,
// ArrayLength, LoadIndexed}" but it is more easily structured this way.
// Should test to see performance hit of clearing it for all handlers
// with empty bodies below. If it is negligible then we should leave
// that in for safety, otherwise should think more about it.
void NullCheckVisitor::do_Phi            (Phi*             x) {}
void NullCheckVisitor::do_Local          (Local*           x) {}
void NullCheckVisitor::do_Constant       (Constant*        x) { /* FIXME: handle object constants */ }
void NullCheckVisitor::do_LoadLocal      (LoadLocal*       x) { nce()->handle_LoadLocal(x);   }
void NullCheckVisitor::do_StoreLocal     (StoreLocal*      x) { nce()->handle_StoreLocal(x);  }
void NullCheckVisitor::do_LoadField      (LoadField*       x) { nce()->handle_AccessField(x); }
void NullCheckVisitor::do_StoreField     (StoreField*      x) { nce()->handle_AccessField(x); }
void NullCheckVisitor::do_ArrayLength    (ArrayLength*     x) { nce()->handle_ArrayLength(x); }
void NullCheckVisitor::do_LoadIndexed    (LoadIndexed*     x) { nce()->handle_LoadIndexed(x); }
void NullCheckVisitor::do_StoreIndexed   (StoreIndexed*    x) { nce()->handle_StoreIndexed(x); }
void NullCheckVisitor::do_CachingChange  (CachingChange*   x) {}
void NullCheckVisitor::do_NegateOp       (NegateOp*        x) {}
void NullCheckVisitor::do_ArithmeticOp   (ArithmeticOp*    x) { if (x->can_trap()) nce()->clear_last_explicit_null_check(); }
void NullCheckVisitor::do_ShiftOp        (ShiftOp*         x) {}
void NullCheckVisitor::do_LogicOp        (LogicOp*         x) {}
void NullCheckVisitor::do_CompareOp      (CompareOp*       x) {}
void NullCheckVisitor::do_IfOp           (IfOp*            x) {}
void NullCheckVisitor::do_Convert        (Convert*         x) {}
void NullCheckVisitor::do_NullCheck      (NullCheck*       x) { nce()->handle_NullCheck(x); }
void NullCheckVisitor::do_Invoke         (Invoke*          x) { nce()->handle_Invoke(x); }
void NullCheckVisitor::do_NewInstance    (NewInstance*     x) { nce()->handle_NewInstance(x); }
void NullCheckVisitor::do_NewTypeArray   (NewTypeArray*    x) { nce()->handle_NewArray(x); }
void NullCheckVisitor::do_NewObjectArray (NewObjectArray*  x) { nce()->handle_NewArray(x); }
void NullCheckVisitor::do_NewMultiArray  (NewMultiArray*   x) { nce()->handle_NewArray(x); }
void NullCheckVisitor::do_CheckCast      (CheckCast*       x) {}
void NullCheckVisitor::do_InstanceOf     (InstanceOf*      x) {}
void NullCheckVisitor::do_MonitorEnter   (MonitorEnter*    x) { nce()->handle_AccessMonitor(x); }
void NullCheckVisitor::do_MonitorExit    (MonitorExit*     x) { nce()->handle_AccessMonitor(x); }
void NullCheckVisitor::do_Intrinsic      (Intrinsic*       x) { nce()->clear_last_explicit_null_check(); }
void NullCheckVisitor::do_BlockBegin     (BlockBegin*      x) {}
void NullCheckVisitor::do_Goto           (Goto*            x) {}
void NullCheckVisitor::do_If             (If*              x) {}
void NullCheckVisitor::do_IfInstanceOf   (IfInstanceOf*    x) {}
void NullCheckVisitor::do_TableSwitch    (TableSwitch*     x) {}
void NullCheckVisitor::do_LookupSwitch   (LookupSwitch*    x) {}
void NullCheckVisitor::do_Return         (Return*          x) {}
void NullCheckVisitor::do_Throw          (Throw*           x) { nce()->clear_last_explicit_null_check(); }
void NullCheckVisitor::do_Base           (Base*            x) {}
void NullCheckVisitor::do_UnsafeGetRaw   (UnsafeGetRaw*    x) { nce()->handle_UnsafeOp(x); }
void NullCheckVisitor::do_UnsafePutRaw   (UnsafePutRaw*    x) { nce()->handle_UnsafeOp(x); }
void NullCheckVisitor::do_UnsafeGetObject(UnsafeGetObject* x) { nce()->handle_UnsafeOp(x); }
void NullCheckVisitor::do_UnsafePutObject(UnsafePutObject* x) { nce()->handle_UnsafeOp(x); }


NullCheckEliminator* NullCheckEliminator::_static_nce = NULL;


void NullCheckEliminator::do_value(Value* p) {
  assert(*p != NULL, "should not find NULL instructions");
  if (!_static_nce->visited(*p)) {
    _static_nce->mark_visited(*p);
    (*p)->input_values_do(&NullCheckEliminator::do_value);
    (*p)->visit(&_static_nce->_visitor);
  }
}


bool NullCheckEliminator::merge_state_for(BlockBegin* block, ValueStack* stack, ValueSet* incoming_state) {
  ValueSet* state = state_for(block);
  ValueStack* phi_stack = block->state();
  if (state == NULL) {
    state = incoming_state->copy();
    set_state_for(block, state);
    // Root node does not have incoming stack
    if (stack != NULL) {
      // Pass stack arguments in phis
      for (int i = 0, j = 0; i < stack->stack_size(); ) {
        Value val = stack->stack_at_inc(i);
        Value phi = phi_stack->stack_at_inc(j);
        assert(i == j, "stack mismatch");
        if (incoming_state->contains(val)) {
          if (PrintNullCheckElimination) {
            tty->print_cr("Phi %d for block %d proven non-null", phi->id(), block->block_id());
          }
          state->put(phi);
        }
      }
    }
    return true;
  } else {
    // Merge in the state and see whether a change occurred
    bool changed = state->set_intersect(incoming_state);
    // Disable any phis which may be null
    for (int i = 0, j = 0; i < stack->stack_size(); ) {
      Value val = stack->stack_at_inc(i);
      Value phi = phi_stack->stack_at_inc(j);
      assert(i == j, "stack mismatch");
      if (!incoming_state->contains(val)) {
        changed = changed || (state->contains(phi));
        if (PrintNullCheckElimination && state->contains(phi)) {
          tty->print_cr("Phi %d for block %d revised to may-be-null", phi->id(), block->block_id());
        }
        state->remove(phi);
      }
    }
    if (PrintNullCheckElimination && changed) {
      tty->print_cr("Block %d's null check state changed", block->block_id());
    }
    return changed;
  }
}


void NullCheckEliminator::iterate_all() {
  while (work_list()->length() > 0) {
    iterate_one(work_list()->pop());
  }
}


void NullCheckEliminator::iterate_one(BlockBegin* block) {
  _static_nce = this;
  clear_visited_state();

  if (PrintNullCheckElimination) {
    tty->print_cr(" ...iterating block %d in null check elimination for %s::%s%s",
                  block->block_id(),
                  ir()->method()->holder()->name()->as_utf8(),
                  ir()->method()->name()->as_utf8(),
                  ir()->method()->signature()->as_symbol()->as_utf8());
  }

  // Create new state if none present (only happens at root)
  if (state_for(block) == NULL) {
    ValueSet* tmp_state = new ValueSet();
    set_state_for(block, tmp_state);
    // Initial state is that local 0 (receiver) is non-null for
    // non-static methods
    ValueStack* stack  = block->state();
    IRScope*    scope  = stack->scope();
    ciMethod*   method = scope->method();
    if (!method->is_static()) {
      Local* local0 = scope->local_at(objectType, 0);
      if (local0 != NULL) {
        // Local 0 is used in this scope
        tmp_state->put(local0);
        if (PrintNullCheckElimination) {
          tty->print_cr("Local 0 proven non-null upon entry");
        }
      }
    }
  }

  // Must copy block's state to avoid mutating it during iteration
  // through the block -- otherwise "not-null" states can accidentally
  // propagate "up" through the block during processing of backward
  // branches and algorithm is incorrect (and does not converge)
  set_state_from(state_for(block));

  // Iterate through block, updating state
  for (Instruction* instr = block; instr != NULL; instr = instr->next()) {
    if (instr->is_root() || (instr->as_NullCheck() != NULL)) {
      if (!visited(instr)) {
        mark_visited(instr);
        instr->input_values_do(&NullCheckEliminator::do_value);
        instr->visit(&_visitor);
      }
    }
  }

  // Propagate state to successors if necessary
  BlockEnd* e = block->end();
  assert(e != NULL, "incomplete graph");
  for (int i = 0; i < e->number_of_sux(); i++) {
    BlockBegin* next = e->sux_at(i);
    if (merge_state_for(next, e->state(), state())) {
      if (!work_list()->contains(next)) {
        work_list()->push(next);
      }
    }
  }
}


void NullCheckEliminator::iterate(BlockBegin* block) {
  work_list()->push(block);
  iterate_all();
}

void NullCheckEliminator::handle_LoadLocal(LoadLocal* x) {
  Local* local = x->local();
  if (set_contains(local)) {
    if (PrintNullCheckElimination) {
      tty->print_cr("LoadLocal %d proven non-null", x->id());
    }
    set_put(x);
  }
}


void NullCheckEliminator::handle_StoreLocal(StoreLocal* x) {
  Local* local = x->local();
  Value  value = x->value();
  if (set_contains(value)) {
    if (PrintNullCheckElimination) {
      tty->print_cr("StoreLocal %d of value %d proven non-null", x->id(), value->id());
    }
    set_put(local);
  } else {
    if (PrintNullCheckElimination && set_contains(local)) {
      tty->print_cr("StoreLocal %d of value %d revised to may-be-null", x->id(), value->id());
    }
    set_remove(local);
  }
}


void NullCheckEliminator::handle_AccessField(AccessField* x) {
  if (x->is_static()) {
    if (x->as_LoadField() != NULL) {
      // If the field is a non-null static final object field (as is
      // often the case for sun.misc.Unsafe), put this LoadField into
      // the non-null map
      ciField* field = x->field();
      if (field->is_constant()) {
        ciConstant field_val = field->constant_value();
        BasicType field_type = field_val.basic_type();
        if (field_type == T_OBJECT || field_type == T_ARRAY) {
          ciObject* obj_val = field_val.as_object();
          if (!obj_val->is_null_object()) {
            if (PrintNullCheckElimination) {
              tty->print_cr("AccessField %d proven non-null by static final non-null oop check",
                            x->id());
            }
            set_put(x);
          }
        }
      }
    }
    // Be conservative
    clear_last_explicit_null_check();
    return;
  }

  Value obj = x->obj();
  if (set_contains(obj)) {
    // Value is non-null => update AccessField
    if (last_explicit_null_check_obj() == obj && !x->needs_patching()) {
      x->set_explicit_null_check(consume_last_explicit_null_check());
      x->set_needs_null_check(true);
      if (PrintNullCheckElimination) {
        tty->print_cr("Folded NullCheck %d into AccessField %d's null check for value %d",
                      x->explicit_null_check()->id(), x->id(), obj->id());
      }
    } else {
      x->set_explicit_null_check(NULL);
      x->set_needs_null_check(false);
      if (PrintNullCheckElimination) {
        tty->print_cr("Eliminated AccessField %d's null check for value %d", x->id(), obj->id());
      }
    }
  } else {
    set_put(obj);
    if (PrintNullCheckElimination) {
      tty->print_cr("AccessField %d of value %d proves value to be non-null", x->id(), obj->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
    x->set_explicit_null_check(NULL);
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_ArrayLength(ArrayLength* x) {
  Value array = x->array();
  if (set_contains(array)) {
    // Value is non-null => update AccessArray
    if (last_explicit_null_check_obj() == array) {
      x->set_explicit_null_check(consume_last_explicit_null_check());
      x->set_needs_null_check(true);
      if (PrintNullCheckElimination) {
        tty->print_cr("Folded NullCheck %d into ArrayLength %d's null check for value %d",
                      x->explicit_null_check()->id(), x->id(), array->id());
      }
    } else {
      x->set_explicit_null_check(NULL);
      x->set_needs_null_check(false);
      if (PrintNullCheckElimination) {
        tty->print_cr("Eliminated ArrayLength %d's null check for value %d", x->id(), array->id());
      }
    }
  } else {
    set_put(array);
    if (PrintNullCheckElimination) {
      tty->print_cr("ArrayLength %d of value %d proves value to be non-null", x->id(), array->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
    x->set_explicit_null_check(NULL);
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_LoadIndexed(LoadIndexed* x) {
  Value array = x->array();
  if (set_contains(array)) {
    // Value is non-null => update AccessArray
    if (last_explicit_null_check_obj() == array) {
      x->set_explicit_null_check(consume_last_explicit_null_check());
      x->set_needs_null_check(true);
      if (PrintNullCheckElimination) {
        tty->print_cr("Folded NullCheck %d into LoadIndexed %d's null check for value %d",
                      x->explicit_null_check()->id(), x->id(), array->id());
      }
    } else {
      x->set_explicit_null_check(NULL);
      x->set_needs_null_check(false);
      if (PrintNullCheckElimination) {
        tty->print_cr("Eliminated LoadIndexed %d's null check for value %d", x->id(), array->id());
      }
    }
  } else {
    set_put(array);
    if (PrintNullCheckElimination) {
      tty->print_cr("LoadIndexed %d of value %d proves value to be non-null", x->id(), array->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
    x->set_explicit_null_check(NULL);
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_StoreIndexed(StoreIndexed* x) {
  Value array = x->array();
  if (set_contains(array)) {
    // Value is non-null => update AccessArray
    if (PrintNullCheckElimination) {
      tty->print_cr("Eliminated StoreIndexed %d's null check for value %d", x->id(), array->id());
    }
    x->set_needs_null_check(false);
  } else {
    set_put(array);
    if (PrintNullCheckElimination) {
      tty->print_cr("StoreIndexed %d of value %d proves value to be non-null", x->id(), array->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_NullCheck(NullCheck* x) {
  Value obj = x->obj();
  if (set_contains(obj)) {
    // Already proven to be non-null => this NullCheck is useless
    if (PrintNullCheckElimination) {
      tty->print_cr("Eliminated NullCheck %d for value %d", x->id(), obj->id());
    }
    // Don't unpin since that may shrink obj's live range and make it unavailable for debug info.
    // The code generator won't emit LIR for a NullCheck that cannot trap.
    x->set_can_trap(false);
  } else {
    // May be null => add to map and set last explicit NullCheck
    x->set_can_trap(true);
    set_put(obj);
    set_last_explicit_null_check(x);
    if (PrintNullCheckElimination) {
      tty->print_cr("NullCheck %d of value %d proves value to be non-null", x->id(), obj->id());
    }
  }
}


void NullCheckEliminator::handle_Invoke(Invoke* x) {
  if (!x->has_receiver()) {
    // Be conservative
    clear_last_explicit_null_check();
    return;
  }

  Value recv = x->receiver();
  if (set_contains(recv)) {
    // Value is non-null => update Invoke
    if (PrintNullCheckElimination) {
      tty->print_cr("Eliminated Invoke %d's null check for value %d", x->id(), recv->id());
    }
    x->set_needs_null_check(false);
  } else {
    set_put(recv);
    if (PrintNullCheckElimination) {
      tty->print_cr("Invoke %d of value %d proves value to be non-null", x->id(), recv->id());
    }
    // Ensure previous passes do not cause wrong state
    if (x->code() == Bytecodes::_invokespecial) {
      // it's only relevant for invokespecial bytecodes.
      x->set_needs_null_check(true);
    }
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_NewInstance(NewInstance* x) {
  set_put(x);
  if (PrintNullCheckElimination) {
    tty->print_cr("NewInstance %d is non-null", x->id());
  }
}


void NullCheckEliminator::handle_NewArray(NewArray* x) {
  set_put(x);
  if (PrintNullCheckElimination) {
    tty->print_cr("NewArray %d is non-null", x->id());
  }
}


void NullCheckEliminator::handle_AccessMonitor(AccessMonitor* x) {
  Value obj = x->obj();
  if (set_contains(obj)) {
    // Value is non-null => update AccessMonitor
    if (PrintNullCheckElimination) {
      tty->print_cr("Eliminated AccessMonitor %d's null check for value %d", x->id(), obj->id());
    }
    x->set_needs_null_check(false);
  } else {
    set_put(obj);
    if (PrintNullCheckElimination) {
      tty->print_cr("AccessMonitor %d of value %d proves value to be non-null", x->id(), obj->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_Intrinsic(Intrinsic* x) {
  if (!x->has_receiver()) {
    // Be conservative
    clear_last_explicit_null_check();
    return;
  }

  Value recv = x->receiver();
  if (set_contains(recv)) {
    // Value is non-null => update Intrinsic
    if (PrintNullCheckElimination) {
      tty->print_cr("Eliminated Intrinsic %d's null check for value %d", x->id(), recv->id());
    }
    x->set_needs_null_check(false);
  } else {
    set_put(recv);
    if (PrintNullCheckElimination) {
      tty->print_cr("Intrinsic %d of value %d proves value to be non-null", x->id(), recv->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
  }
  clear_last_explicit_null_check();
}


void NullCheckEliminator::handle_UnsafeOp(UnsafeOp* x) {
  Value unsafe = x->unsafe();
  if (set_contains(unsafe)) {
    // Value is non-null => update UnsafeOp
    if (PrintNullCheckElimination) {
      tty->print_cr("Eliminated UnsafeOp %d's null check for unsafe %d", x->id(), unsafe->id());
    }
    x->set_needs_null_check(false);
  } else {
    set_put(unsafe);
    if (PrintNullCheckElimination) {
      tty->print_cr("UnsafeOp %d of unsafe %d proves value to be non-null", x->id(), unsafe->id());
    }
    // Ensure previous passes do not cause wrong state
    x->set_needs_null_check(true);
  }
  clear_last_explicit_null_check();
}


void Optimizer::eliminate_null_checks() {
  NullCheckEliminator nce(this);

  if (PrintNullCheckElimination) {
    tty->print_cr("Starting null check elimination for method %s::%s%s",
                  ir()->method()->holder()->name()->as_utf8(),
                  ir()->method()->name()->as_utf8(),
                  ir()->method()->signature()->as_symbol()->as_utf8());
  }
  
  // Apply to graph
  nce.iterate(ir()->start());

  // walk over the graph looking for exception
  // handlers and iterate over them as well
  int nblocks = BlockBegin::number_of_blocks();
  BlockList blocks(nblocks);
  boolArray visited_block(nblocks, false);

  blocks.push(ir()->start());
  visited_block[ir()->start()->block_id()] = true;
  for (int i = 0; i < blocks.length(); i++) {
    BlockBegin* b = blocks[i];
    // exception handlers need to be treated as additional roots
    for (int e = b->number_of_exception_handlers(); e-- > 0; ) {
      BlockBegin* excp = b->exception_handler_at(e);
      int id = excp->block_id();
      if (!visited_block[id]) {
        blocks.push(excp);
        visited_block[id] = true;
        nce.iterate(excp);
      }
    }
    // traverse successors
    BlockEnd *end = b->end();
    for (int s = end->number_of_sux(); s-- > 0; ) {
      BlockBegin* next = end->sux_at(s);
      int id = next->block_id();
      if (!visited_block[id]) {
        blocks.push(next);
        visited_block[id] = true;
      }
    }
  }

 
  if (PrintNullCheckElimination) {
    tty->print_cr("Done with null check elimination for method %s::%s%s",
                  ir()->method()->holder()->name()->as_utf8(),
                  ir()->method()->name()->as_utf8(),
                  ir()->method()->signature()->as_symbol()->as_utf8());
  }
}
