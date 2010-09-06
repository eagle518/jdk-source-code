#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodeGenerator.cpp	1.145 04/03/31 18:13:11 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_CodeGenerator.cpp.incl"


#define __ emit()->lir()->

//----------------------------------------------------------------------
//                        Utilities
//----------------------------------------------------------------------

// The constructor locks the specified register and the destructor unlocks it;
// Use to prevent locking of a certain register
HideReg::HideReg(ValueGen* vg, RInfo reg, bool spill /*= false*/) {
  _vg = vg;
  _reg = reg;
  if (spill && !_vg->is_free_rinfo(reg)) {
    while (!_vg->ra()->is_free_reg(reg)) {
      _vg->spill_register(reg);
    }
  }
  assert(_vg->is_free_rinfo(reg), "cannot hide it, because it is already locked");
  _vg->ra()->lock_temp(NULL, reg);
  _vg->ra()->incr_spill_lock(reg);
}

// spills, locks
HideReg::HideReg(ValueGen* vg, ValueType* t) {
  _vg = vg;
  _reg = _vg->lock_free_rinfo(NULL, t); // <- may spill
  _vg->ra()->incr_spill_lock(_reg);
}


HideReg::HideReg(ValueGen* vg, c1_RegMask mask) {
  _vg = vg;
  _reg = _vg->lock_free_rinfo(NULL, mask); // <- may spill
  _vg->ra()->incr_spill_lock(_reg);
}


HideReg::~HideReg() {
  _vg->ra()->decr_spill_lock(_reg);
  _vg->ra()->free_reg(_reg);
  assert(_vg->is_free_rinfo(_reg), "must be free");
}


define_array(ValueTypeArray, ValueType*);
define_stack(ValueTypeList, ValueTypeArray);

// This class is used by ValueGen to cause spill slots to be released
// after the code generation for an instruction has completed.  This
// solves a problem where spilling necessary to free a register reused
// a spill slot used by an argument before the code that uses the
// spill slot has been generated.

class DelayedSpillMark: public StackObj {
 private:
  ValueGen*         _gen;
  intStack          _spill_index;
  ValueTypeList     _spill_type;
  DelayedSpillMark* _previous;
 public:
  DelayedSpillMark(ValueGen* gen): _gen(gen) {
    _previous = _gen->_spill_mark;
    _gen->_spill_mark = this;
  }
  ~DelayedSpillMark() {
    _gen->_spill_mark = _previous;
    for (int i = 0; i < _spill_index.length(); i++) {
      _gen->ra()->free_spill(_spill_index.at(i), _spill_type.at(i));
    }
  }

  void delayed_sfree(int spill_index, ValueType* spill_type) {
    _spill_index.append(spill_index);
    _spill_type.append(spill_type);
  }
};


//--------------------------------------------------------------


CodeGenerator::CodeGenerator (ValueGen* gen, ValueGenInvariant* vgi):  _gen(gen), _max_spills(0), _vgi(vgi) {
}


void CodeGenerator::block_do_prolog(BlockBegin* block) {
  if (PrintIRWithLIR) {
    block->print();
  }

  _gen->start_block(block);
  _gen->bind_block_entry(block);
  // add PcDesc to debug info so we can find entry point for exception handling
  // (note: we are finding the handler entry point by searching the PcDescs - if
  //        we use another method to find the exception handler entry we don't
  //        need to emit this information anymore).
  ValueStack* lock_stack = block->state()->copy(); 
  lock_stack->clear_stack(); // only locks of interest and correctly set
  if (block->is_set(BlockBegin::exception_entry_flag)) _gen->exception_handler_start(block->scope(), block->bci(), lock_stack);
}


void CodeGenerator::block_do_epilog(BlockBegin* block) {
  if (_vgi->emit() != NULL && _vgi->emit()->must_bailout()) return;
  assert(_vgi->reg_alloc()->are_all_free(), "not everything released at block end");
  _max_spills = MAX2(_max_spills, _vgi->reg_alloc()->max_spills());

  if (PrintIRWithLIR) {
    tty->cr();
  }
}


void CodeGenerator::block_do(BlockBegin* block) {
  if (_vgi->emit() != NULL && _vgi->emit()->must_bailout()) return;
  _gen->set_invariant(_vgi);
  block_do_prolog(block);
  _gen->block_prolog(block);
  _gen->set_block(block);

  for (Instruction* instr = block; instr != NULL; instr = instr->next()) {
    if (instr->is_root()) _gen->do_root(instr);
  }
  _gen->set_block(NULL);
  _gen->block_epilog(block);
  block_do_epilog(block);
}


static void clear_state_items(Value* x) {
  assert((*x)->as_Phi() != NULL, "state must only contain phis");
  (*x)->clear_item();
}


// This is same as clear_block_items in c1_Compiler1.cpp; factorize!
void CodeGenerator::clear_instruction_items(BlockBegin* b) {
  // clear phi's
  b->state_values_do(clear_state_items);
  // clear remaining instructions
  for (Instruction* n = b; n != NULL; n = n->next()) n->clear_item();
}


//-------------------------ValueGen-----------------------------

bool  ValueGen::_init_done = false;


void ValueGen::init_value_gen() {
  if (_init_done) return;
  _init_done = true;
}


bool ValueGen::is_backward_branch(Value instr, BlockBegin* dest) const {
  bool is_bb =  block()->is_after_block(dest);
  if (is_bb) {
    // the backward branch flag is set so that in the code emission pass we can 
    // align blocks that are backward branch targets
    dest->set(BlockBegin::backward_branch_target_flag);
  }
  return is_bb;
}


#ifndef PRODUCT
void ValueGen::print_at_do_root(Value instr, bool before_visit) {
  if (PrintC1RegAlloc) {
    if (before_visit) {
      tty->print("ROOT bci %d before: ", instr->bci()); ra()->print();
      emit()->print();
    } else {
      tty->print("ROOT bci %d after:  ", instr->bci()); ra()->print();
      emit()->print(); 
      tty->cr();
    }
  }
}
#endif


// This is where the tree-walk starts; instr must be root;
void ValueGen::do_root(Value instr) {
#ifdef ASSERT
  InstructionMark im(compilation(), instr);
#endif // ASSERT

  assert(instr->is_root(), "use only with roots");

  if (emit()->must_bailout()) return;

  debug_only(ra()->check_registers();)
  assert(ra()->are_all_spill_locks_free(), "no spill locks may be alive");
  // Phi's are visited separately during block prolog
  if (instr->as_Phi() != NULL) {
    assert(instr->item() != NULL, "phi item must be set");
    return;
  }

  print_at_do_root(instr, true);
  _hint   = HintItem::no_hint();
  _result = new RootItem(instr);
  assert (instr->item() == NULL, "somebody already visited it?");
  if (instr->as_BlockEnd() == NULL) {
    // Delay freeing of spill slots until after this instruction completes.
    DelayedSpillMark mark(this);
    instr->visit(this);
  } else {
    // Can't delay freeing of spill slots because BlockEnds expect all
    // resources to be free at the end.  It's also not necessary as
    // the prompt release of spill slots is really only a danger for
    // instruction which produce results which might cause spilling.
    instr->visit(this);
  }

  // process result item
  if (_result->has_result()) {
    if (instr->has_uses()) {
      // if _result->is_constant(), it will be generated when needed (see do_Constant)
      if (_result->is_stack() && !_result->is_spilled()) {
        // must load this stack value that did not result from a spill (i.e. it's a local)
        // for proper semantics:
        //     load local 0
        //     load local 0
        //     inc 1
        //     store local 0
        //          ....            <- on expression stack is the "old" local 0
        RInfo reg = rlock(instr);
        emit()->move(item2lir(_result), reg);
        _result->set_register(reg);
        ra()->set_reg(reg, instr->use_count(), instr);
        assert(!ra()->is_spill_locked(reg), "root value must be spillable");
      } else if (_result->is_register()) {
        RInfo reg = _result->get_register();
        ra()->set_reg(reg, instr->use_count(), instr);
        assert(!ra()->is_spill_locked(reg), "root value must be spillable");
      }
      assert(_result->is_root_item(), "");
      instr->set_item(_result);
    } else {
      // the result has no uses; release its register allocation resources
      release_item(_result);
      _result->set_no_result();
    }
  }

  print_at_do_root(instr, false);
  assert(!instr->has_uses() || instr->item()->is_valid(), "invalid item set");
  assert(ra()->are_all_spill_locks_free(), "no spill locks may be alive");
  debug_only(ra()->check_registers();)
}

void ValueGen::release_item(Item* item) {
  if (item->is_register()) {
    if (item->type()->is_double() || item->type()->is_float()) {
      emit()->remove_fpu_result(item->get_register());
    }
    raw_rfree(item);
  } else if (item->is_spilled()) {
    sfree(item);
  }
}


#ifndef PRODUCT
void ValueGen::print_at_walk(Value instr, bool before_visit) {
  if (PrintC1RegAlloc) {
    if (before_visit) {
      tty->print("     BCI %d before: ", instr->bci()); 
      emit()->print();
      ra()->print();  
    } else {
      tty->print("     BCI %d after:  ", instr->bci());  
      emit()->print();
      ra()->print(); 
      tty->cr(); 
    }
  }
}
#endif


// This is called for each node in tree; the walk stops if a root is reached
void ValueGen::walk(Value instr) {
#ifdef ASSERT
  InstructionMark im(compilation(), instr);
#endif // ASSERT
  //stop walk when encounter a root
  if (instr->is_root()) {
    assert(instr->item() != NULL, "this root has not yet been visited");
    // copy content of RootItem to stack Item.
    result()->set_from_item(instr->item());
    assert(result()->is_valid(), "invalid item");
    debug_only(check_item(instr->item());)
  } else {
    assert(instr->use_count() <= 1, "an instruction with use_count > 1 must be root ");
    print_at_walk(instr, true);
    {
      // Delay freeing of spill slots until after this instruction completes.
      DelayedSpillMark mark(this);
      instr->visit(this);
    }
    assert(instr->use_count() > 0, "leaf instruction must have a use");
    print_at_walk(instr, false);
  }
}


void ValueGen::item_free(Item* item) {
  if      (item->is_spilled() ) {
    if (_spill_mark != NULL) {
      _spill_mark->delayed_sfree(item->get_spilled_index(), item->type());
    } else {
      sfree(item);
}
  } else if (item->is_register()) rfree (item);
}


// releases spill index held by item
void ValueGen::sfree(Item* item) {
  ra()->free_spill(item->get_spilled_index(), item->type());
}


// locks specified RInfo 'reg'; spill RInfo 'reg' if not free;
void ValueGen::lock_spill_rinfo (Value instr, RInfo reg) {
  // do a loop because of register pairs, that requires two calls to spill_register
  while (!ra()->is_free_reg(reg)) {
    spill_register(reg);
  }
  ra()->lock_register(instr, reg);
}


// locks specified RInfo 'reg'; spill RInfo 'reg' if not free;
void ValueGen::lock_spill_temp (Value instr, RInfo reg) {
  // do a loop because of register pairs, that requires two calls to spill_register
  while (!ra()->is_free_reg(reg)) {
    spill_register(reg);
  }
  ra()->lock_temp(instr, reg);
}


// find an instruction that can be spilled; mark that instruction for spill
// and return its RInfo; spill priority is set by bci of root instructions;
// the smaller the bci, the higher the chance for spilling
void ValueGen::spill_one(ValueType* type) {
  // we need a loop as we may need to spill 2 registers (long type)
  while (!ra()->has_free_reg(type)) {
    Value root = ra()->get_smallest_value_to_spill(type);
    assert(root != NULL, "must search expression stack for stuff to spill (not implemented yet)");
    spill_value(root);
  }
}


// find an instruction that can be spilled; mark that instruction for spill
// and return its RInfo; spill priority is set by bci of root instructions;
// the smaller the bci, the higher the chance for spilling
void ValueGen::spill_one(c1_RegMask mask) {
  // we need a loop as we may need to spill 2 registers (long type)
  while (!ra()->has_free_reg(mask)) {
    Value root = ra()->get_smallest_value_to_spill(mask);
    assert(root != NULL, "must search expression stack for stuff to spill (not implemented yet)");
    spill_value(root);
  }
}


// search instruction that uses reg, find a spill index, and 
// spill that instruction
void ValueGen::spill_register(RInfo reg) {
  Value to_spill = ra()->get_value_for_rinfo(reg);
  assert(to_spill != NULL && to_spill->item() != NULL, "value not legal");
  spill_value(to_spill);
}



#ifndef PRODUCT
void ValueGen::print_spilling(int bci, int spill_ix) {
  if (PrintSpilling) {
    tty->print_cr("    spilling item for bci %d to spill-slot %d", bci, spill_ix);
  }
}
#endif


void ValueGen::round_spill_item(Item* item, bool round_op) {
  assert(item != NULL, "where is the item?");
  assert(item->is_root_item() || !item->value()->is_root(), "cannot spill a non-root item that points to a root instruction(inconsistency)");
  assert(item->is_register(), "why spill if item is not register?");
  int rc       = ra()->get_register_rc(item->get_register());
  int spill_ix = ra()->get_lock_spill(item->value(), rc);
  // 'spill_ix' is the locked spill slot
  for (int i = 0; i < rc; i++) {
    raw_rfree(item);
  }

  print_spilling(item->value()->bci(), spill_ix);
  if (round_op) {
    emit()->round(spill_ix, item2lir(item));
  } else {
    emit()->spill(spill_ix, item2lir(item));
  }

  item->set_spill_ix(spill_ix);
  assert(ra()->check_spilled(item), "wrong spilled state");
}


// similar to spill but emits different instruction (that can be optimized away)
void ValueGen::round_item(Item* item) {
  if (method()->is_strict()) {
    // we cannot optimize this away, so use spills
    round_spill_item(item, false);
  } else {
    round_spill_item(item, true);
  }
}


void ValueGen::spill_item(Item* item) {
  round_spill_item(item, false);
}



// Value->item must reside in register: will be moved into spill area
void ValueGen::spill_value(Value value) {
  assert(value->item()->is_register(), "must be a register");
  assert(!ra()->is_spill_locked(value->item()->get_register()), "cannot spill a spill locked value");
  assert(value->item()->value() == value, "inconsistency");
  spill_item(value->item());
}


// returns true if caller register was successfuly moved to a callee register
bool ValueGen::try_caller_to_callee(Value value, RInfo hide_reg) {
  assert(value->item() != NULL && value->item()->is_register(), "must be a register");
  assert(!ra()->is_spill_locked(value->item()->get_register()), "cannot spill a spill locked value");
  assert(value->item()->value() == value, "inconsistency");
  // any callee saved register free?
  if (value->type()->is_int() || value->type()->is_object()) {
    // cannot handle other types yet
    RootItem* item = value->item()->as_root();
    assert(item != NULL, "not a root item");
    c1_RegMask mask = FrameMap::callee_save_regs();
    if (!hide_reg.is_illegal()) {
      mask.remove_reg(hide_reg);
    }
    if (ra()->has_free_reg(mask)) {
      RInfo to_reg = ra()->reallocate(item->get_register(), mask);
      assert(!to_reg.is_same(hide_reg), "operation failed");
      emit()->move(item2lir(item), to_reg);
      item->set_register(to_reg);
      return true;
    }
  }
  return false;
}


// This is used to preserve registers across calls; only values in
// calller-save registers are spilled(saved)
void ValueGen::spill_values_on_stack(ValueStack* stack, RInfo hide_reg, bool caller_to_callee) {
  // Note: InlineMethodsWithExceptionHandlers does not work with this
  // backend since we would need to spill all expression stack items
  // with indices less than IRScope::lock_stack_size() to phi
  // locations on the stack. Currently we can only support this kind
  // of spilling at the end of basic blocks in move_to_phi since we
  // can not spill a Value to more than one stack slot.

  for (int i = 0; i < stack->stack_size();) {
    Value val = stack->stack_at_inc(i);
    if (!val->is_root() && val->item() != NULL && val->item()->is_register() && is_caller_save_register(val->item()->get_register())) {
      // it's possible that a value on the expression stack is in a register but for some reason
      // didn't become a root, in which case we bailout since we can't simply spill it because
      // that violates other invariants
      emit()->set_bailout("non root item on the expression stack has a register");
    } else if (val->has_uses() && val->item() != NULL && val->item()->is_valid()) {
      // roots on stack may have use_count 0, in which case we do not spill them
      // Sometimes the register of the root item may already be freed->do not spill then
      if (val->item()->is_register()) {
        if (is_caller_save_register(val->item()->get_register())) { 
          if (caller_to_callee) {
            bool done = try_caller_to_callee(val, hide_reg);
            if (!done ) spill_value(val);
          } else {
            spill_value(val);
          }
        }
      } else {
        // The root may not be on stack !! The reason is that we mark
        // Load locals as roots to force loading into registers as 
        // the stack location may be destroyed by a later store;
        // code example: a[i++]
        assert(val->item()->is_constant() || val->item()->is_spilled(),  "wrong item state");
      }
    }
  }
}


void ValueGen::release_roots(ValueStack* stack) {
  for (int i = 0; i < stack->stack_size();) {
    Value val = stack->stack_at_inc(i);
    if (val->is_root()) {
      release_item(val->item());
    }
  }
}



// Phi technique:
// This is about passing live values from one basic block to the other.
// In code generated with Java it is rather rare that more than one
// value is on the stack from one basic block to the other.
// We optimize our technique for efficient passing of one value
// (of type long, int, double..) but it can be extended.
// When entering or leaving a basic block, all registers and all spill
// slots are release and empty. We use the released registers
// and spill slots to pass the live values from one block
// to the other. The topmost value, i.e., the value on TOS of expression
// stack is passed in registers. All other values are stored in spilling
// area. Every Phi has an index which designates its spill slot
// At exit of a basic block, we fill the register(s) and spill slots.
// At entry of a basic block, the block_prolog sets up the content of phi nodes
// and locks necessary registers and spilling slots.


// Value v is the value that is spilled at spill_ix;
// least_spill_ix is the minimum index where the spill value at spill_ix
// has to be moved; this is used to move a spilled value out of area reserved for
// Phi-values

void ValueGen::move_spill_to(int spill_ix, int least_spill_ix, Value v) {
  for (int i = 0; i < v->type()->size(); ++i) {
    if (ra()->get_ref_count_at(spill_ix + i) > 0) {
      // release spill slot at spill_ix, while locking it after least_spill_ix
      Value to_move = ra()->value_spilled_at(spill_ix + i);
      int from_spill_ix = to_move->item()->get_spilled_index();
      int new_spill_ix = ra()->get_free_spill_after(least_spill_ix, to_move->type());
      assert(new_spill_ix >= least_spill_ix, "wrong index computation");
      HideReg rg(this, to_move->type());
      RInfo reg = rg.reg(); // get_free_rinfo(to_move->type());
      emit()->move_spill(new_spill_ix, from_spill_ix, to_move->type(), reg);
      ra()->move_spills (new_spill_ix, from_spill_ix, to_move->type());
    }
  }
}




// return phi tos value; spill slots for other values are returned in 'spill_ixs' and values in 'phi_values'
// 'stack' describes the spill values
Value ValueGen::compute_phi_arrays(ValueStack* stack, Values* phi_values, intStack* spill_ixs,
                                   bool force_tos_to_stack_for_athrow) {
  // Note: the force_tos_to_stack_for_athrow flag is a partial step
  // toward supporting InlineMethodsWithExceptionHandlers (by properly
  // flushing expression stack values to phi locations at Throw
  // instructions) but more work would be needed to fully support this
  // flag (in particular, causing a certain number of expression stack
  // values to go to phi locations at potentially exception-throwing
  // instructions, or creating adapter blocks to go to exception
  // handlers)

  if (stack->stack_size() == 0) return NULL;
  bool  is_tos  = !force_tos_to_stack_for_athrow;
  Value tos_val = NULL;
  // compute spill slots, spill values and tos-vallue
  for (int i = stack->stack_size(); i > 0;) {
    Value val = stack->stack_at_dec(i);
    assert(val != NULL, "wrong stack access");
    if (is_tos) {
      is_tos = false; tos_val = val;
    } else {
      int spill_ix = i;
      assert (0 <= spill_ix && spill_ix < stack->stack_size(), "wrong spill index");
      phi_values->append(val);
      spill_ixs->append(spill_ix);
    }
  }
  return tos_val;
}


// Moves all stack values into their PHI position and return true if
// spill moves were required.
bool ValueGen::move_to_phi(ValueStack* stack, bool force_tos_to_stack_for_athrow) {
  bool spill_moved = false;
  
  if (stack->stack_size() == 0) {
    if (!force_tos_to_stack_for_athrow) {
      assert(ra()->are_all_free(), "");
    }
    return false;
  }
  Values phi_values;
  intStack spill_ixs;
  Value tos_val = compute_phi_arrays(stack, &phi_values, &spill_ixs, force_tos_to_stack_for_athrow);

  assert(tos_val != NULL || force_tos_to_stack_for_athrow, "");
  // First: move phi values into spill-slots
  for (int i = 0; i < phi_values.length(); i++) {
    Value v  = phi_values.at(i);
    int   ix = spill_ixs.at(i);
    if (v->item() != NULL && v->item()->is_spilled() && v->item()->get_spilled_index() == ix) {
      // everything is fine, because the value is already in right place
      ra()->free_spill(ix, v->type());
    } else {
      spill_moved = true;
      if (!ra()->is_free_spill(ix, v->type())) {
        // the spill slot for v is currently blocked; move content of that spill slot elsewhere
        move_spill_to(ix, stack->stack_size(), v);
      }
      assert(ra()->is_free_spill(ix, v->type()), "spill slot still not empty");
      // Note: we will not lock the spill slot, but we are emitting code to move it into it.
      Item item(v);
      item.handle_float_kind(); // INTEL specific
      ValueGen vg(&item, HintItem::no_hint(), this);
      load_item(&item);
      item_free(&item);
      assert(ra()->is_free_spill(ix, v->type()), "spill slot not empty");
      emit()->spill(ix, item2lir(&item));
    }
  }
  // Second: now load TOS value into register
  if (tos_val != NULL) {
    RInfo reg = result_register_for(tos_val->type());
    HintItem hint(tos_val->type(), reg);
    Item item(tos_val);
    item.handle_float_kind();

    ValueGen vg(&item, &hint, this);
    // we cannot load_item_force here, because the spilling is not set up correctly (all is released)
    load_item_hint(&item, &hint);
    item_free(&item);
    if (!reg.is_same(item.get_register())) {
      emit()->move(item2lir(&item),  reg);
      spill_moved = true;
    }
    emit()->set_fpu_stack_empty();
  }
  if (!force_tos_to_stack_for_athrow) {
    assert(ra()->are_all_free(), "end of block: all registers and spilling slots must be released");
  }
  return spill_moved;
}


// Try to lock using register in hint
RInfo ValueGen::rlock(Value instr, const Item* hint /* = NULL */) {
  if (hint != NULL &&
      hint->has_hint() &&
      ra()->is_free_reg(hint->get_register())) {
    ra()->lock_register(instr, hint->get_register());   
    return hint->get_register();
  }

  // no hint or hint register is locked
  while (!ra()->has_free_reg(instr->type())) {
    spill_one(instr->type()); 
  }
  return ra()->get_lock_reg(instr, instr->type());
}


// does an rlock and sets result
RInfo ValueGen::rlock_result_with_hint(Value x, const Item* hint) {
  RInfo reg = rlock(x, hint);
  set_result(x, reg);
  return reg;
}


RInfo ValueGen::set_with_result_register(Value x) {
  RInfo reg = result_register_for(x->type());
  lock_spill_rinfo(x, reg);
  set_result(x, reg);
  return reg;
}


// If item is register, mark item as not spillable
void ValueGen::set_maynot_spill(Item* item) {
  if (item->is_register()) {
    // increase spill lock counter in register allocator
    ra()->incr_spill_lock(item->get_register());
  }
}


// release register described by item (decrease ref count by 1)
// this should be used together with load_item, as the spill lock counter is decreased too.
void ValueGen::rfree(Item* item) {
  ra()->decr_spill_lock(item->get_register());
  ra()->free_reg(item->get_register());
  // for roots, make sure that their items are not holding on a register anymore (otherwise we may have problems when spilling expression stack values)
  if (item->value() != NULL && item->value()->is_root()) {
    Item* root_item = item->value()->item();
    if (root_item->is_register() && ra()->is_free_reg(root_item->get_register())) {
      // no more use of this item -> invalidate it
      root_item->set_no_result();
    } else if (root_item->is_spilled() && 
               ra()->is_free_spill(root_item->get_spilled_index(), item->value()->type())) {
      // In many places (e.g., invokes) we need to preserve information about the spill slot location
      // even if the spill slot has been released; therefore do not kill that information.
      // root_item->set_no_result();
    }
  }
}


// release register described by hint (decrease ref count by 1)
// this is not used together with load item
void ValueGen::raw_rfree(Item* item) {
  ra()->free_reg(item->get_register());
}


bool ValueGen::is_free_rinfo(RInfo reg) {
  return ra()->is_free_reg(reg);
}


// locks a free register; sets refcount to 1
RInfo ValueGen::lock_free_rinfo(Value instr, ValueType* type) {
  if (!ra()->has_free_reg(type)) {
    spill_one(type);
  }
  return ra()->get_lock_temp(instr, type);
}


RInfo ValueGen::lock_free_rinfo(Value instr, c1_RegMask mask) {
  while (!ra()->has_free_reg(mask)) {
    Value val = ra()->get_smallest_value_to_spill(mask);
    assert(val != NULL, "could not spill");
    spill_value(val);
  }
  return ra()->get_lock_temp(instr, mask);
}


//----------------------------------------------------------------


void  ValueGen::set_no_result(Value x) { 
  assert(!x->has_uses(), "cannot have a use"); 
  result()->set_no_result();  
}


bool ValueGen::must_copy_register(Item* item) {
  return item->destroys_register() && (ra()->get_register_rc(item->get_register()) > 1);
}


// if the item is spilled or is register, check that the information
// entered in RegAlloc cooresponds
// We search for the value entered in the register allocator and spiller.
// Several Items may point to the same Value. A Value holds only one
// Item; Non-root items are referenced only by one Item; root Values
// can be referenced by several Items
void ValueGen::check_item(Item* item) {
  assert(item->is_valid(), "invalid item");
  if (item->is_spilled()) {
    int    ix = item->get_spilled_index();
    Value val = ra()->value_spilled_at(ix);
    assert(item->value() == val, "wrong item state");
  } else if (item->is_register()) {
    RInfo reg = item->get_register();
    Value val = ra()->get_value_for_rinfo(reg);
    assert(item->value() == val, "wrong item state");
  }
}


// Mark items that do not need to be loaded (we must call either load_item or dont_load_item)
void ValueGen::dont_load_item(Item* item) {
  dont_load_item_nocheck(item);
  debug_only(check_item(item);)
  if (item->is_register() && item->destroys_register()) {
    check_float_register(item);
#ifdef ASSERT
    if (!is_free_rinfo(item->get_register())) {
      assert(ra()->get_register_rc(item->get_register()) <= 1, "we should copy it instead of not loading it");
    }
#endif
  }
}


void ValueGen::dont_load_item_nocheck(Item* item) {
  item->update();
  set_maynot_spill(item);
}


// Note about item loading( load_xxxx..):
//    - at the start, the item being modified must be sycnhronized with the item that
//      is maintained by the instruction; 
//    - The item is then being modified and may not correspond to the item being pointed
//      to by instruction

// loads item in a register
void ValueGen::load_item(Item* item) {
  item->update();
  set_maynot_spill(item);
  debug_only(check_item(item);)
  if (item->is_register() && !must_copy_register(item)) {
    check_float_register(item);
  } else {
    // We come here when either item is not register or if we have to 
    // copy from a register to a different one
    if (!fpu_fanout_handled(item)) {
      RInfo reg = lock_free_rinfo(item->value(), item->type());
      emit()->move(item2lir(item), reg);
      item_free(item);
      item->set_register(reg);
    }
    set_maynot_spill(item);
  } 
}


// Used for static field access to patch the operation
//    movl reg, klass
void ValueGen::load_item_patching(IRScope* scope, int bci_of_patched_bytecode, Item* item, ValueStack* state, ExceptionScope* exception_scope) {
  item->update();
  assert(item->is_constant() &&  item->type()->is_object(), "must be an object constant");
  set_maynot_spill(item);
  RInfoCollection* oop_regs = NULL;
  // don't include the destination register as an oop in patching info
  oop_regs = ra()->oops_in_registers();

  RInfo reg = lock_free_rinfo(item->value(), item->type());

  // NOTE: we must currently keep track of the bci of the LoadField/
  // StoreField instruction which is causing the patch to take place.
  // This code used to use the bci of the item's value, which referred
  // to the class constant, but the bci was wrong there; accidentally
  // worked until full inlining was added, when the bci could suddenly
  // span scopes. Should fix by adding patching node in IR.
  CodeEmitInfo* info = new CodeEmitInfo(emit(), bci_of_patched_bytecode, ra()->oops_in_spill(), state, exception_scope, oop_regs);
  emit()->jobject2reg_with_patching(reg, item->get_jobject_constant(), info);

  item_free(item);
  item->set_register(reg);
  set_maynot_spill(item);
}


// loads item using hint if possible
void ValueGen::load_item_hint(Item* item, const Item* hint) {
  item->update();
  set_maynot_spill(item);
  debug_only(check_item(item);)
  if (item->is_register() && !must_copy_register(item)) {
    check_float_register(item);
    // Note: we may not have the value loaded in the correct
    //       register, but that is OK and simpler to leave it there instead
    //       of moving it into hint
  } else {
    if (!fpu_fanout_handled(item)) {
      RInfo reg;
      if (hint->has_hint() && is_free_rinfo(hint->get_register())) {
        reg = hint->get_register();
        ra()->lock_temp(item->value(), reg);
      } else {
        reg = lock_free_rinfo(item->value(), item->type());
      }
      emit()->move(item2lir(item), reg);
      item_free(item);
      item->set_register(reg);
    }
    set_maynot_spill(item);
  }
}


void ValueGen::load_item_with_reg_mask(Item* item, c1_RegMask mask) {
  assert(!item->type()->is_float() && !item->type()->is_double() && !item->type()->is_long(), "only cpu registers for now");
  set_maynot_spill(item);
  item->update();
  debug_only(check_item(item);)
  if (item->is_register() && mask.contains(item->get_register()) && !must_copy_register(item)) {
    return;
  }

  RInfo reg = lock_free_rinfo(item->value(), mask);
  emit()->move(item2lir(item), reg);

  item_free(item);
  item->set_register(reg);
  set_maynot_spill(item);
}


// Force loading of item into the specified register (used e.g. for return and intrinsics)
void ValueGen::load_item_force(Item* item, RInfo force_reg) {
  item->update();
  if (item->is_register()) {
    RInfo item_reg = item->get_register();
    assert(item_reg.is_same_type(force_reg), "register types must match");
    if (item_reg.overlaps(force_reg)) {
      // item_reg must be spilled if it is not the same as force_reg or
      // it has more than one use and is overwritten by the current operation.
      if (!item_reg.is_same(force_reg) || must_copy_register(item)) {
        Value item_value = item->value();
        spill_item(item_value->item());
      } else if (item_reg.is_same(force_reg)) {
        dont_load_item(item);
        return;
      }
    }
  }
  item->update();
  set_maynot_spill(item);
  debug_only(check_item(item));
  lock_spill_temp(item->value(), force_reg);
  emit()->move(item2lir(item), force_reg);
  item_free(item);
  item->set_register(force_reg);
  set_maynot_spill(item);
}


void ValueGen::block_epilog(BlockBegin* block) {
  // nothing to do for now.
}



void ValueGen::start_block(BlockBegin* block) {
  emit()->start_block(block);
  if (block->is_set(BlockBegin::backward_branch_target_flag)) {
    emit()->align_backward_branch(); 
  }
}



void ValueGen::block_prolog(BlockBegin* block) {
  // setup phi's
  assert(block->state() != NULL, "block state must exist");
  assert(ra()->are_all_free(), "");  

  // setup phi's
  if (!block->state()->stack_is_empty()) { 
    Values phi_values;
    intStack spill_ixs;
    Value tos_val = compute_phi_arrays(block->state(), &phi_values, &spill_ixs);
    for (int i = 0; i < phi_values.length(); i++) {
      Phi* phi = phi_values.at(i)->as_Phi();
      assert(phi != NULL, "value is not phi");
      int spill_ix = spill_ixs.at(i);
      if (phi->has_uses()) {
        assert(phi->item() == NULL, "item already set");
        RootItem* item = new RootItem(phi);
        phi->set_item(item);
        item->set_spill_ix(spill_ix);
        ra()->lock_spill(phi, spill_ix, phi->use_count());
        assert(phi->item()->is_valid(), "invalid item");
        assert(ra()->check_spilled(item), "wrong spilled state");
      } else {
        // must set and release in order to notify the register allocator of 
        // correct number of spills
        ra()->lock_spill(phi, spill_ix, 1);
        ra()->free_spill(spill_ix, phi->type());
        phi->set_item(new RootItem(phi));  // dummy
      }
    }
    // set up tos value
    assert(tos_val != NULL, "must generate tos_value");
    assert(tos_val->item() == NULL, "tos item already set");
    if (tos_val->has_uses()) {
      RInfo phi_reg = result_register_for(tos_val->type());
      RootItem* item = new RootItem(tos_val);
      item->set_register(phi_reg);
      tos_val->set_item(item);
      assert(tos_val->item()->is_valid(), "invalid_item");
      ra()->lock_register(tos_val, phi_reg);
      if (tos_val->type()->is_float() || tos_val->type()->is_double()) {
        emit()->set_fpu_result(tos_val->item()->get_register());
      }
    } else {
      tos_val->set_item(new RootItem(tos_val));
      if (tos_val->type()->is_float_kind()) {
        emit()->fpop(); // restore FPU stack
      }
    }
  }
  if (PrintC1RegAlloc) { 
    tty->print("Block prolog "); 
    emit()->print();
    ra()->print(); 
    tty->cr();  
  }
}


void ValueGen::bind_block_entry(BlockBegin* block) {
  emit()->bind_block_entry(block);
}


void ValueGen::exception_handler_start(IRScope* scope, int bci, ValueStack* lock_stack) {
  // the nop at start of exception handler prevents that we emit two pc/bci 
  // mappings with identical pc's but varying bci's
  emit()->handler_entry();
}


//-----------------------spilling--------------------------------------------------------

// spill all values in caller_save registers
void ValueGen::spill_caller_save() {
  NEEDS_CLEANUP
  // a more efficient way would be to go through the list of
  // locked registers and ask if they are caller save.
  const RInfoCollection* regs = FrameMap::caller_save_registers();
  NEEDS_CLEANUP
  // Note: must traverse from index 0 to index n, as the double registers
  // on SPARC lock two entries but only the first entry has the value set;
  // going toward zero instead from zero would hit first the locked register
  // without a Value set.
  int lng = regs->length();
  for (int i = 0; i < lng; i++) {
    const RInfo r = regs->at(i);
    if (!ra()->is_free_reg(r)) spill_register(r);
  }
}




//-----------------------register allocation---------------------------------------------

RInfo ValueGen::result_register_for(ValueType* type, bool callee) {
  ValueTag tag = type->tag();
  switch (tag) {
    case intTag:
    case objectTag:
    case addressTag: return callee ? callee_return1RInfo() : return1RInfo();

    case longTag:    return callee ? callee_return2RInfo() : return2RInfo();

    case floatTag:   return returnF0RInfo();
    case doubleTag:  return returnD0RInfo();

    default: ShouldNotReachHere(); return norinfo;
  }
}



//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//                        visitor functions
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------


// Phi that represent the TOS is in register; all others are in spill area
// TOS(1) element is at spill index 0.
void ValueGen::do_Phi(Phi* x) {
  assert(x->is_root(), "phi must be root");
  assert(x->item() != NULL, "phi's item must be set by block prolog");
}


// Code for a constant is generated lazily unless the constant is frequently used and can't be inlined.
void ValueGen::do_Constant(Constant* x) {
  _result->set_constant();
  ObjectConstant* oc = x->type()->as_ObjectConstant();
  if (oc != NULL && (!oc->value()->is_loaded() || (PatchALot && x->state() != NULL))) {
    // this is an ldc of an unloaded class constant.
    ValueStack* state = x->state();
    assert(state != NULL, "must be non NULL");
    
    spill_values_on_stack(x->state());

    // this must be the result of an ldc of a class
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(),
                                          x->state(), x->exception_scope(), ra()->oops_in_registers());
    RInfo reg = lock_free_rinfo(x, x->type());
    __ oop2reg_patch(NULL, reg, info);
    _result->set_register(reg);
  } else if (x->use_count() > 1 && !can_inline_as_constant(_result)) {
    assert(x->state() == NULL, "should be patching this");
    ValueType* type = x->type();
    ClassConstant* class_constant = type->as_ClassConstant();
    if (class_constant == NULL || (class_constant->value()->is_initialized() && !PatchALot)) {
      RInfo reg = lock_free_rinfo(x, type);
      emit()->move(item2lir(_result), reg);
      _result->set_register(reg);
    }
  } else {
    assert(oc == NULL || x->state() == NULL, "should be patching this");
  }
}


void ValueGen::do_Local(Local* x) {
  ShouldNotReachHere();
}


// Example: object.getClass ()
void ValueGen::do_getClass(Intrinsic* x) {
  assert(x->number_of_arguments() == 1, "wrong type");

  Item rcvr(x->argument_at(0));
  ValueGen rcvr_v(&rcvr, HintItem::no_hint(), this);
  load_item(&rcvr);
  item_free(&rcvr);
  RInfo result = rlock_result_with_hint(x, hint());

  // need to perform the null check on the rcvr
  CodeEmitInfo* info = NULL;
  if (x->needs_null_check()) {
    info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  }
  emit()->getClass(result, rcvr.get_register(), info);
}


//------------------------local access--------------------------------------

void ValueGen::do_StoreLocal(StoreLocal* x) {
  assert(x->has_local_name(), "no local name assigned to StoreLocal");
  // if this is store local, and we are caching it than we pass a hint
  set_no_result(x);
  Item value(x->value());
  value.handle_float_kind();
  
  ValueGen v  (&value, HintItem::no_hint(), this);
  if (can_inline_any_constant() && value.is_constant()) {
    // let it be a constant
    dont_load_item(&value);
  } else {
    load_item(&value);
    item_free(&value);
  }
  
  if (!x->is_eliminated()) {
    emit()->opr2local(x->local_name(), item2lir(&value));
  } else {
    // On x86 we have to consume any just-pushed FPU value
    if (x->type()->is_float_kind()) {
      if (value.is_register()) {
        emit()->pop_item(item2lir(&value));
      }
    }
  }
}


void ValueGen::do_LoadLocal(LoadLocal* x) {
  RInfo r;
  assert(x->use_count() == 0 || x->has_local_name(), "no local name assigned to LoadLocal");
  int local_name = x->use_count() == 0 ? 0 : x->local_name();
  _result->set_stack(local_name);
}


//------------------------field access--------------------------------------

// Comment copied form templateTable_i486.cpp
// ----------------------------------------------------------------------------
// Volatile variables demand their effects be made known to all CPU's in
// order.  Store buffers on most chips allow reads & writes to reorder; the
// JMM's ReadAfterWrite.java test fails in -Xint mode without some kind of
// memory barrier (i.e., it's not sufficient that the interpreter does not
// reorder volatile references, the hardware also must not reorder them).
// 
// According to the new Java Memory Model (JMM):
// (1) All volatiles are serialized wrt to each other.  
// ALSO reads & writes act as aquire & release, so:
// (2) A read cannot let unrelated NON-volatile memory refs that happen after
// the read float up to before the read.  It's OK for non-volatile memory refs
// that happen before the volatile read to float down below it.
// (3) Similar a volatile write cannot let unrelated NON-volatile memory refs
// that happen BEFORE the write float down to after the write.  It's OK for
// non-volatile memory refs that happen after the volatile write to float up
// before it.
//
// We only put in barriers around volatile refs (they are expensive), not
// _between_ memory refs (that would require us to track the flavor of the
// previous memory refs).  Requirements (2) and (3) require some barriers
// before volatile stores and after volatile loads.  These nearly cover
// requirement (1) but miss the volatile-store-volatile-load case.  This final
// case is placed after volatile-stores although it could just as well go
// before volatile-loads.


void ValueGen::do_StoreField(StoreField* x) {
  ExceptionScope* exception_scope = x->exception_scope();
  bool needs_patching = x->needs_patching();
  if (PrintNotLoaded && needs_patching) {
    tty->print_cr("   ###class not loaded at store_%s bci %d", x->is_static() ?  "static" : "field", x->bci());
  }
  intStack* oops_in_spill = NULL;
  if (needs_patching) {
    // for deoptimization (why must the stack be spilled?)
    spill_values_on_stack(x->state_before());
    // oops in spill area must be collected during patching of object
    oops_in_spill = ra()->oops_in_spill();
  }
  Item object(x->obj());
  Item value(x->value());
  value.handle_float_kind();
  ValueTag valueTag = x->type()->tag();
  if (valueTag == objectTag) {
    // SPARC Note: we could use a G register for preserving the object register instead
    //             of destroying it; this would free up a potentially caller saved register for
    //             caching
    object.set_destroys_register();
  }
  ValueGen o(&object, HintItem::no_hint(), this);
  ValueGen v(&value,  HintItem::no_hint(), this);

  if (needs_patching && x->is_static()) {
    load_item_patching(scope(), x->bci(), &object, x->state_before(), exception_scope);
  } else {
    load_item(&object);
  }

  RInfo tmp = scratch1_RInfo();
  if (x->field()->is_volatile() || !value.is_constant() || needs_patching || !can_inline_any_constant()) {
    // load item if field is volatile (less special cases for volatiles)
    // load item if field not initialized
    // load item if field not constant
    // load item if cannot inline constants
    // because of code patching we cannot inline constants
    bool isFloatKind = valueTag == floatTag || valueTag == doubleTag;
    if (prefer_alu_registers() && !needs_patching && value.is_stack() && isFloatKind) {
      // use integer registers for floats and doubles
      tmp = lock_free_rinfo(x->value(), valueTag == floatTag ? (ValueType*)intType : (ValueType*)longType);
      ra()->free_reg(tmp);
    } else {
      if (x->field_type() == T_BYTE || x->field_type() == T_BOOLEAN) { 
        load_byte_item(&value);
        assert(FrameMap::is_byte_rinfo(value.get_register()), "how come?");
      } else  {  
        load_item(&value);       
      }
    }
  } else {
    assert(value.is_constant(), "sanity check");
    // does not need patching and value is a constant
    dont_load_item(&value);
  }
  set_no_result(x);

  // create debug info for a safepoint during a patch or while throwing NullPointerException
  CodeEmitInfo* info = NULL;
  if (needs_patching) {
    assert(x->explicit_null_check() == NULL, "null check can't be folded into patched instruction");
    info = new CodeEmitInfo(emit(), x->bci(), oops_in_spill, x->state_before(), exception_scope, ra()->oops_in_registers());
  } else if (x->needs_null_check()) {
    oops_in_spill = ra()->oops_in_spill();
    NullCheck* nc = x->explicit_null_check();
    if (nc != NULL) {
      info = new CodeEmitInfo(emit(), nc->bci(), oops_in_spill, nc->lock_stack(), nc->exception_scope(), NULL);
    } else {
      info = new CodeEmitInfo(emit(), x->bci(), oops_in_spill, x->lock_stack() , exception_scope, NULL);
    }
  }

  if (x->field()->is_volatile() && os::is_MP()) emit()->membar_release();
  emit()->field_store(x->field(), item2lir(&object), x->offset(), item2lir(&value), needs_patching, x->is_loaded(), info, tmp);
  if (x->field()->is_volatile() && os::is_MP()) emit()->membar();

  item_free(&value);
  item_free(&object);
}


void ValueGen::do_LoadField(LoadField* x) {
  bool needs_patching = x->needs_patching();
  bool can_trap = needs_patching || x->needs_null_check();
  ValueStack* field_stack = x->lock_stack();
  ExceptionScope* exception_scope = x->exception_scope();
  if (needs_patching) {
    spill_values_on_stack(x->state_before()); // for deoptimization
    field_stack = x->state_before();
  }
  Item     object(x->obj());
  ValueGen o(&object, HintItem::no_hint(), this);

  CodeEmitInfo* info = NULL; 
  RInfoCollection* oop_regs = NULL;

  // must collect oops in spill beforehand, otherwise the spill element may be released 
  // and may not appear in the oop map list.
  if (needs_patching) {
    oop_regs = ra()->oops_in_registers();
    info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), field_stack, exception_scope, oop_regs);
  }
  if (needs_patching && x->is_static()) {
    load_item_patching(scope(), x->bci(), &object, field_stack, exception_scope);
  } else {
    load_item(&object);
  }
  if (info == NULL && can_trap) {
    NullCheck* nc = x->explicit_null_check();
    if (nc == NULL) {
      info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), field_stack, exception_scope, oop_regs);
    } else {
      info = new CodeEmitInfo(emit(), nc->bci(), ra()->oops_in_spill(), nc->lock_stack(), nc->exception_scope(), oop_regs);
    }
  } else if (needs_patching) {
    // make sure that the object is in the oop map too
    if (!oop_regs->contains(object.get_register())) oop_regs->append(object.get_register());
  }
  item_free(&object);
  // result should be hint
  RInfo reg = norinfo;
  if (x->field_type() == T_BYTE || x->field_type() == T_BOOLEAN) {
    reg = rlock_byte_result_with_hint(x, hint());
  } else {
    reg = rlock_result_with_hint(x, hint());
  }

  if (PrintNotLoaded && needs_patching) {
    tty->print_cr("   ###class not loaded at load_%s bci %d", x->is_static() ?  "static" : "field", x->bci());
  }
  emit()->field_load(reg, x->field(), item2lir(&object), x->offset(), needs_patching, x->is_loaded(), info);
  if (x->field()->is_volatile() && os::is_MP()) emit()->membar_acquire();
}


//------------------------java.nio.Buffer.checkIndex------------------------

// int java.nio.Buffer.checkIndex(int)
void ValueGen::do_NIOCheckIndex(Intrinsic* x) {
  // NOTE: by the time we are in checkIndex() we are guaranteed that
  // the buffer is non-null (because checkIndex is package-private and
  // only called from within other methods in the buffer).
  assert(x->number_of_arguments() == 2, "wrong type");
  Item buf  (x->argument_at(0));
  Item index(x->argument_at(1));
  ValueGen buf_v  (&buf,   HintItem::no_hint(), this);
  ValueGen index_v(&index, HintItem::no_hint(), this);
  load_item(&buf);
  load_item(&index);

  HintItem hint(x->type(), index.get_register());

  item_free(&buf);
  item_free(&index);

  //  RInfo result = rlock_result_with_hint(x, HintItem::no_hint());
  RInfo result = rlock_result_with_hint(x, &hint);
  if (GenerateRangeChecks) {
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->nio_range_check(item2lir(&buf), item2lir(&index), result, info);
  } else {
    // Just load the index into the result register
    emit()->lir()->move(item2lir(&index), LIR_OprFact::rinfo(result));
  }
}


//------------------------array access--------------------------------------


void ValueGen::do_ArrayLength(ArrayLength* x) {
  Item array(x->array());
  ValueGen a(&array, HintItem::no_hint(), this);
  load_item(&array);
  item_free(&array);
  RInfo reg = rlock_result_with_hint(x, hint());

  int array_length_offset = arrayOopDesc::length_offset_in_bytes(); 
  CodeEmitInfo* info = NULL;
  if (x->needs_null_check()) {
    NullCheck* nc = x->explicit_null_check();
    if (nc == NULL) {
      info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    } else {
      info = new CodeEmitInfo(emit(), nc->bci(), ra()->oops_in_spill(), nc->lock_stack(), nc->exception_scope());
    }
  }
  emit()->array_length(reg, item2lir(&array), info);
}


void ValueGen::do_LoadIndexed(LoadIndexed* x) {
  bool use_length = x->length() != NULL;
  Item array(x->array());
  Item index(x->index());
  Item length;
  if (use_length) {
    length.set_instruction(x->length());
  }
  ValueGen a(&array, HintItem::no_hint(), this);
  ValueGen i(&index, HintItem::no_hint(), this);
  ValueGen l(&length, HintItem::no_hint(), this, true);
  load_item(&array);
  if (use_length) {
    load_item(&length);
  }
  if (index.is_constant() && can_inline_as_constant(&index)) {
    // let it be a constant
    dont_load_item(&index);
  } else {
    load_item(&index);
    item_free(&index);
  }

  if (use_length) {
    item_free(&length);
  }

  // delayed register release for LONG types, otherwise array or index register may be overwritten
  // before both loads are completed
  if (!x->type()->is_long()) {
    item_free(&array);
  }

  RInfo reg = norinfo;
  if (x->elt_type() == T_BYTE || x->elt_type() == T_BOOLEAN) {
    reg = rlock_byte_result_with_hint(x, hint());
  } else {
    reg = rlock_result_with_hint(x, hint());
  }

  CodeEmitInfo* range_check_info = new CodeEmitInfo(emit(), x->bci(), NULL, x->lock_stack(), x->exception_scope());
  CodeEmitInfo* null_check_info = NULL;
  if (x->needs_null_check()) {
    NullCheck* nc = x->explicit_null_check();
    if (nc != NULL) {
      null_check_info = new CodeEmitInfo(emit(), nc->bci(), NULL, nc->lock_stack(), nc->exception_scope());
    } else {
      null_check_info = range_check_info;
    }
  }
  if (GenerateRangeChecks) {
    if (use_length) {
      emit()->length_range_check(item2lir(&length), item2lir(&index), range_check_info);
      assert(null_check_info == NULL, "should already have been done");
    } else {
      emit()->array_range_check(item2lir(&array), item2lir(&index), null_check_info, range_check_info);
      // The range check performs the null check, so clear it out for the load
      null_check_info = NULL;
    }
  }
  emit()->indexed_load(reg, x->elt_type(), item2lir(&array), item2lir(&index), null_check_info);
					
  if (x->type()->is_long()) {
    item_free(&array);
  }
}


void ValueGen::do_NullCheck(NullCheck* x) {
  Item value(x->obj());
  ValueGen v(&value, HintItem::no_hint(), this);
  if (x->can_trap()) {
    load_item(&value);
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->null_check(item2lir(&value), info);
  } else {
    // eliminate the check
    dont_load_item(&value);
  }
  item_free(&value);
}


void ValueGen::do_Throw(Throw* x) {
  if (!x->state()->stack_is_empty()) {
    // release all roots (computed values)
    release_roots(x->state());
  }

  if (x->check_flag(Instruction::KeepStateBeforeAliveFlag) && !x->state_before()->stack_is_empty()) {
    // release all roots (computed values)
    release_roots(x->state_before());
  }

  bool unwind = false;
  bool type_is_exact = true;
  // get some idea of the throw type
  ciType* throw_type = x->exception()->exact_type();
  if (throw_type == NULL) {
    type_is_exact = false;
    throw_type = x->exception()->declared_type();
  }
  if (throw_type != NULL && throw_type->is_instance_klass()) {
    ciInstanceKlass* throw_klass = (ciInstanceKlass*)throw_type;
    unwind = !x->exception_scope()->could_catch(throw_klass, type_is_exact);
  }

  Item exception(x->exception());
  HintItem exception_hint(x->type(), exceptionOopRInfo());
  ValueGen e(&exception, &exception_hint, this);

  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), NULL, x->state_before()->copy_locks(), x->exception_scope());

  load_item_force(&exception, exceptionOopRInfo());
  item_free(&exception);
  set_no_result(x);

  assert(exceptionOopRInfo().is_same(exception.get_register()), "should already be here");
  if (GenerateCompilerNullChecks && x->exception()->as_NewInstance() == NULL) {
    // if the exception object wasn't created using new then it might be null.
    __ null_check(exceptionOopRInfo(), info);
  }
  if (unwind) {
    __ unwind_exception(exceptionPcRInfo(), exceptionOopRInfo(), info);
  } else {
    __ throw_exception(exceptionPcRInfo(), exceptionOopRInfo(), info);
  }

  assert(ra()->are_all_free(), "Should release all registers and spilled locations");
}


void ValueGen::do_UnsafeGetRaw(UnsafeGetRaw* x) {
  Item obj(x->unsafe());
  Item src(x->base());
  Item index;

  if (x->has_index()) {
    index.set_instruction(x->index());
    assert(x->index()->type() == intType, "should not find non-int index");
  }

  ValueGen obj_v(&obj, HintItem::no_hint(), this);
  ValueGen src_v(&src, HintItem::no_hint(), this);
  ValueGen index_v(&index, HintItem::no_hint(), this, true);

  if (x->needs_null_check()) {
    load_item(&obj);
  } else {
    dont_load_item(&obj);
  }
  load_item(&src);
  
  if (x->has_index()) {
    load_item(&index);
  }
  // delay freeing the index register for double word loads, otherwise the
  // register allocator may put src and index in the same registers
  // used for the result, which makes it impossible to
  // correctly emit the assembly on intel
  if (x->has_index() && x->type()->size() != 2) {
    item_free(&index);
  }

  item_free(&src);
  item_free(&obj);
  RInfo reg = norinfo;
  if (x->basic_type() == T_BYTE || x->basic_type() == T_BOOLEAN) {
    reg = rlock_byte_result_with_hint(x, hint());
  } else {
    reg = rlock_result_with_hint(x, hint());
  }

  // free the index register for the double word case
  if (x->has_index() && x->type()->size() == 2) {
    item_free(&index);
  }

  if (x->needs_null_check()) {
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->null_check(item2lir(&obj), info);
  }
  assert(!x->has_index() || index.value() == x->index(), "should match");

  LIR_Opr index_opr = LIR_OprFact::illegalOpr;
  int   log2_scale = 0;
  if (x->has_index()) {
    index_opr = item2lir(&index);
    log2_scale = x->log2_scale();
  }
  emit()->get_raw_unsafe(reg, item2lir(&src), index_opr, log2_scale, x->basic_type());
}


void ValueGen::do_UnsafePutRaw(UnsafePutRaw* x) {
  Item obj (x->unsafe());
  Item dest(x->base());
  Item index;
  if (x->has_index()) {
    index.set_instruction(x->index());
  }
  Item data(x->value());
  BasicType type = x->basic_type();

  data.handle_float_kind();       // destroy the register on i486.

  if (x->has_index()) {
    assert(x->index()->type() == intType, "should not find non-int index");
  }

  ValueGen obj_v (&obj, HintItem::no_hint(), this);
  ValueGen dest_v(&dest, HintItem::no_hint(), this);
  ValueGen data_v(&data, HintItem::no_hint(), this);
  ValueGen index_v(&index, HintItem::no_hint(), this, true);
  
  if (x->needs_null_check()) {
    load_item(&obj);
  } else {
    dont_load_item(&obj);
  }
  load_item(&dest);

  if (x->has_index()) {
    load_item(&index);
  }
  if (type == T_BYTE || type == T_BOOLEAN) {
      load_byte_item(&data);
  } else {
      load_item(&data);
  }
  
  item_free(&data);
  item_free(&dest);
  if (x->has_index()) {
    item_free(&index);
  }
  item_free(&obj);

  set_no_result(x);

  if (x->needs_null_check()) {
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->null_check(item2lir(&obj), info);
  }
  assert(!x->has_index() || index.value() == x->index(), "should match");

  LIR_Opr index_opr = LIR_OprFact::illegalOpr;
  int   log2_scale = 0;
  if (x->has_index()) {
    index_opr = item2lir(&index);
    log2_scale = x->log2_scale();
  }
  emit()->put_raw_unsafe(item2lir(&dest), index_opr, log2_scale, item2lir(&data), x->basic_type());
}

void ValueGen::do_UnsafeGetObject(UnsafeGetObject* x) {
  Item obj (x->unsafe());
  Item src (x->object());
  Item off (x->offset());
  BasicType type = x->basic_type();
  
  ValueGen off_v (&off, HintItem::no_hint(), this); // 1st because usually frees a reg
  ValueGen obj_v (&obj, HintItem::no_hint(), this);
  ValueGen src_v (&src, HintItem::no_hint(), this);

  if (x->needs_null_check()) {
    load_item(&obj);
  } else {
    dont_load_item(&obj);
  }
  load_item(&off);
  load_item(&src);

  item_free(&src);
  // delay freeing the offset register for double word loads, otherwise the
  // register allocator may put src and off in the same registers
  // used for the result, which makes it impossible to
  // correctly emit the assembly on intel
  if (x->type()->size() != 2) {
    item_free(&off);
  }
  item_free(&obj);

  RInfo reg = norinfo;
  if (x->basic_type() == T_BYTE || x->basic_type() == T_BOOLEAN) {
    reg = rlock_byte_result_with_hint(x, hint());
  } else {
    reg = rlock_result_with_hint(x, hint());
  }

  // free the index register for the double word case
  if (x->type()->size() == 2) {
    item_free(&off);
  }

  if (x->needs_null_check()) {
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->null_check(item2lir(&obj), info);
  }

  emit()->get_Object_unsafe(reg, item2lir(&src), item2lir(&off), type, x->is_volatile());
  if (x->is_volatile() && os::is_MP()) emit()->membar_acquire();
}


void ValueGen::do_UnsafePutObject(UnsafePutObject* x) {
  Item obj (x->unsafe());
  Item src (x->object());
  Item off (x->offset());
  Item data(x->value());
  BasicType type = x->basic_type();

  data.handle_float_kind();   // destroy the register on i486.

  ValueGen src_v (&src, HintItem::no_hint(), this); // 1st because usually frees a reg
  ValueGen obj_v (&obj, HintItem::no_hint(), this);
  ValueGen off_v (&off, HintItem::no_hint(), this);
  ValueGen data_v(&data,HintItem::no_hint(), this);

  if (x->needs_null_check()) {
    load_item(&obj);
  } else {
    dont_load_item(&obj);
  }

  load_item(&src);
  if (type == T_BOOLEAN || type == T_BYTE) {
    load_byte_item(&data);
  } else {
    load_item(&data);
  }  
  load_item(&off);

  item_free(&off);
  item_free(&data);
  item_free(&src);
  item_free(&obj);

  set_no_result(x);

  if (x->needs_null_check()) {
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->null_check(item2lir(&obj), info);
  }

  if (x->is_volatile() && os::is_MP()) emit()->membar_release();
  emit()->put_Object_unsafe(item2lir(&src), item2lir(&off), item2lir(&data), type, x->is_volatile());
  if (x->is_volatile() && os::is_MP()) emit()->membar();
}


void ValueGen::do_Goto(Goto* x) {
  set_no_result(x);

  CodeEmitInfo* safepoint_info = NULL;
  if (x->is_safepoint()) {
    ValueStack * state_before = x->state_before() ? x->state_before() : x->state();
    safepoint_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), state_before,
                                      x->exception_scope(), ra()->oops_in_registers());
    safepoint_info->set_is_compiled_safepoint();
      
    if (SafepointPolling) {    
      RInfo tmp;
      if (safepoint_poll_needs_register()) {
        // Get a temporary register for the poll check
        tmp = lock_free_rinfo(NULL, objectType);
        ra()->free_reg(tmp);
      }
      __ safepoint(tmp, safepoint_info);
      safepoint_info = NULL;
    }
  }

  if (move_to_phi(x->state()) && safepoint_info != NULL) {
    // spill moves were needed to correct the phi state but we can't
    // easily decribe the state for the safepointing with compiled
    // safepoint since they occur after the move to phi, so bailout.
    emit()->set_bailout("spill moves needed at backward branch safepoint");
  }

  // check that this is endless goto (add nop)
  // Note: this is probably not needed as we have switched off EliminateJumpToJumps
  bool endless_loop = x->is_safepoint() && x->default_sux()->bci() == x->bci();
  if (endless_loop) {
    emit()->nop();
  }
  goto_default_successor(x, safepoint_info);
}




// do a jump to default successor, except if it is the immediate successor in _code (sequence of blocks)
void ValueGen::goto_default_successor(BlockEnd* block_end, CodeEmitInfo * info) {
  NEEDS_CLEANUP; // may need nops if this block_end is a safepoint instead of using the jump.
  bool must_be_safepoint = block_end->is_safepoint() && block_end->as_Goto() != NULL;
  if (!must_be_safepoint && block()->weight() + 1 == block_end->default_sux()->weight()) {
    // default successor is next block; no goto necessary
  } else {
    emit()->goto_op(block_end->default_sux(), info);
  }
}


static bool is_constant_zero(Instruction* inst) {
  IntConstant* c = inst->type()->as_IntConstant();
  if (c) {
    return (c->value() == 0);
  }
  return false;
}


static bool positive_constant(Instruction* inst) {
  IntConstant* c = inst->type()->as_IntConstant();
  if (c) {
    return (c->value() >= 0);
  }
  return false;
}


static ciArrayKlass* as_array_klass(ciType* type) {
  if (type != NULL && type->is_array_klass() && type->is_loaded()) {
    return (ciArrayKlass*)type;
  } else {
    return NULL;
  }
}

void ValueGen::arraycopy_helper(Intrinsic* x, int* flagsp, ciArrayKlass** expected_typep) {
  Instruction* src     = x->argument_at(0);
  Instruction* src_pos = x->argument_at(1);
  Instruction* dst     = x->argument_at(2);
  Instruction* dst_pos = x->argument_at(3);
  Instruction* length  = x->argument_at(4);

  // first try to identify the likely type of the arrays involved
  ciArrayKlass* expected_type = NULL;
  bool is_exact = false;
  {
    ciArrayKlass* src_exact_type    = as_array_klass(src->exact_type());
    ciArrayKlass* src_declared_type = as_array_klass(src->declared_type());
    ciArrayKlass* dst_exact_type    = as_array_klass(dst->exact_type());
    ciArrayKlass* dst_declared_type = as_array_klass(dst->declared_type());
    if (src_exact_type != NULL && src_exact_type == dst_exact_type) {
      // the types exactly match so the type is fully known
      is_exact = true;
      expected_type = src_exact_type;
    } else if (dst_exact_type != NULL && dst_exact_type->is_obj_array_klass()) {
      ciArrayKlass* dst_type = (ciArrayKlass*) dst_exact_type;
      ciArrayKlass* src_type = NULL;
      if (src_exact_type != NULL && src_exact_type->is_obj_array_klass()) {
        src_type = (ciArrayKlass*) src_exact_type;
      } else if (src_declared_type != NULL && src_declared_type->is_obj_array_klass()) {
        src_type = (ciArrayKlass*) src_declared_type;
      }
      if (src_type != NULL) {
        if (src_type->element_type()->is_subtype_of(dst_type->element_type())) {
          is_exact = true;
          expected_type = dst_type;
        }
      }
    }
    // at least pass along a good guess
    if (expected_type == NULL) expected_type = dst_exact_type;
    if (expected_type == NULL) expected_type = src_declared_type;
    if (expected_type == NULL) expected_type = dst_declared_type;
  }

  // if a probable array type has been identified, figure out if any
  // of the required checks for a fast case can be elided.
  int flags = LIR_OpArrayCopy::all_flags;
  if (expected_type != NULL) {
    // try to skip null checks
    if (src->as_NewArray() != NULL) 
      flags &= ~LIR_OpArrayCopy::src_null_check;
    if (dst->as_NewArray() != NULL)
      flags &= ~LIR_OpArrayCopy::dst_null_check;

    // check from incoming constant values
    if (positive_constant(src_pos))
      flags &= ~LIR_OpArrayCopy::src_pos_positive_check;
    if (positive_constant(dst_pos))
      flags &= ~LIR_OpArrayCopy::dst_pos_positive_check;
    if (positive_constant(length))
      flags &= ~LIR_OpArrayCopy::length_positive_check;

    // see if the range check can be elided, which might also imply
    // that src or dst is non-null.
    ArrayLength* al = length->as_ArrayLength();
    if (al != NULL) {
      if (al->array() == src) {
        // it's the length of the source array
        flags &= ~LIR_OpArrayCopy::length_positive_check;
        flags &= ~LIR_OpArrayCopy::src_null_check;
        if (is_constant_zero(src_pos))
          flags &= ~LIR_OpArrayCopy::src_range_check;
      }
      if (al->array() == dst) {
        // it's the length of the destination array
        flags &= ~LIR_OpArrayCopy::length_positive_check;
        flags &= ~LIR_OpArrayCopy::dst_null_check;
        if (is_constant_zero(dst_pos))
          flags &= ~LIR_OpArrayCopy::dst_range_check;
      }
    }
    if (is_exact) {
      flags &= ~LIR_OpArrayCopy::type_check;
    }
  }

  *flagsp = flags;
  *expected_typep = (ciArrayKlass*)expected_type;
}

void ValueGen::do_FPIntrinsics(Intrinsic* x) {
  assert(x->number_of_arguments() == 1, "wrong type");
  Item value       (x->argument_at(0));
  ValueGen value_v(&value, HintItem::no_hint(), this);  

  if (value.is_spilled()) {
    // if it's already stack leave it there
    dont_load_item(&value);
  } else {
    // produce the value
    load_item(&value);
  }
  item_free(&value);

  RInfo reg = rlock_result_with_hint(x, hint());
  emit()->move(item2lir(&value), LIR_OprFact::rinfo(reg));
}
