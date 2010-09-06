#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIROptimizer.cpp	1.42 04/03/31 18:13:06 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_LIROptimizer.cpp.incl"


LIR_PeepholeState::LIR_PeepholeState():
    _refcount(new intStack())
  , _defining_op(new intStack())
  , _register_values(new intStack())
  , _safe_to_delete(new intStack())
  , _stack_values(new intStack())
  , _value_map(new LIR_OprList())
  , _locals(NULL)
  , _disable_optimization(false) {
  NOT_PRODUCT(_register_names = new LIR_OprList());
  initialize(NULL);
}


void LIR_PeepholeState::initialize(LocalMapping* locals) {
  _locals = locals;
  clear_values();
  _defining_op->clear();
  _refcount->clear();
  _safe_to_delete->clear();
  assert(_forward_branches.length() == 0, "should have returned to original state");
  _forward_branches.clear();
  assert(!_disable_optimization, "should have returned to original state");
  _disable_optimization = false;
  _value_number = 0;
  
  NOT_PRODUCT(_state_number = _last_state_number = 0);
}


void LIR_PeepholeState::clear_values() {
  NOT_PRODUCT(_state_number++);
  _register_values->clear();
  _stack_values->clear();
  _value_map->clear();
}


bool LIR_PeepholeState::is_safe_to_delete(int opindex) const {
  return (_safe_to_delete->at_grow(opindex, 0) != 0);
}


void LIR_PeepholeState::mark_safe_to_delete(int opindex) {
  _safe_to_delete->at_put_grow(opindex, 1, 0);
}


void LIR_PeepholeState::increment_ref(int opindex) {
  if (opindex != -1) {
    _refcount->at_put(opindex, _refcount->at_grow(opindex, 0) + 1);
  }
}


int LIR_PeepholeState::stack2index(LIR_Opr opr) {
  if (opr->is_single_stack()) {
    return opr->single_stack_ix();
  }
  ShouldNotReachHere();
}


int LIR_PeepholeState::stack2indexLo(LIR_Opr opr) {
  if (opr->is_double_stack()) {
    return opr->double_stack_ix();
  }
  ShouldNotReachHere();
}


int LIR_PeepholeState::stack2indexHi(LIR_Opr opr) {
  if (opr->is_double_stack()) {
    return opr->double_stack_ix() + 1;
  }
  ShouldNotReachHere();
}


int LIR_PeepholeState::reg2index(LIR_Opr opr) {
  if (opr->is_single_cpu()) {
    assert(opr->cpu_regnr() < FrameMap::nof_cpu_regs, "overlap");
    return opr->cpu_regnr();
  } else if (opr->is_single_fpu()) {
    return opr->fpu_regnr() + FrameMap::nof_cpu_regs;
  }
  ShouldNotReachHere();
}


int LIR_PeepholeState::reg2indexLo(LIR_Opr opr) {
  if (opr->is_double_cpu()) {
    assert(opr->cpu_regnrLo() < FrameMap::nof_cpu_regs, "overlap");
    return opr->cpu_regnrLo();
  } else if (opr->is_double_fpu()) {
    return opr->fpu_regnrLo() + FrameMap::nof_cpu_regs;
  }
  ShouldNotReachHere();
}


int LIR_PeepholeState::reg2indexHi(LIR_Opr opr) {
  if (opr->is_double_cpu()) {
    assert(opr->cpu_regnrHi() < FrameMap::nof_cpu_regs, "overlap");
    return opr->cpu_regnrHi();
  } else if (opr->is_double_fpu()) {
    return opr->fpu_regnrHi() + FrameMap::nof_cpu_regs;
  }
  ShouldNotReachHere();
}


#ifndef PRODUCT
void LIR_PeepholeState::print(bool force) {
  if (_last_state_number != _state_number || force) {
    if (_register_values->length() == 0) {
      return;
    }

    int i;
    tty->print("[state ");
    for (i = 0; i < _register_values->length(); i++) {
      int vn = _register_values->at(i);
      if (vn != -1) {
        LIR_Opr value = _value_map->at(vn);
        LIR_Opr opr = _register_names->at(i);
        if (value->is_valid() && value != opr) {
          opr->rinfo().print(); tty->print("=");
          value->print(); tty->print(" "); if (Verbose) tty->print(" %d ", vn);
        }
      }
    }
    tty->print_cr("]");
    _last_state_number = _state_number;
  }
}
#endif /* PRODUCT */


int LIR_PeepholeState::record_opr_reference(LIR_Opr opr) {
  int def = -1;

  if (opr->is_pointer()) {
    LIR_Address* addr = opr->as_address_ptr();
    if (addr) {
      record_opr_reference(addr->base());
      record_opr_reference(addr->index());
    }
  } else if (opr->is_register()) {
#ifndef PRODUCT
    if (LIRTracePeephole && Verbose) {
      tty->print("  ref "); opr->print(); tty->cr();
    }
#endif // PRODUCT
    if (opr->is_single_word()) {
      def = defining_op(opr);
    } else if (opr->is_double_word()) {
      def = defining_op(opr);
      if (def == -1 && opr->is_double_cpu()) {
        // this opr might have been produced using two separate loads
        LIR_Opr lo = LIR_OprFact::rinfo(opr->rinfo().as_rinfo_lo());
        LIR_Opr hi = LIR_OprFact::rinfo(opr->rinfo().as_rinfo_hi());
        int deflo = defining_op(lo);
        int defhi = defining_op(hi);
        if (deflo != -1 && defhi != -1) {
          // return the defining op of the low word and increment
          // refcount of the hi word.
          def = deflo;
          increment_ref(defhi);
        } else {
#ifndef PRODUCT
          if (LIRTracePeephole && Verbose) {
            tty->print_cr("*** possible bogus definition of double word item");
          }
#endif // PRODUCT
        }
      }
    }
  }

  if (def != -1) {
    increment_ref(def);
  }
  return def;
}


LIR_Opr LIR_PeepholeState::equivalent_register(LIR_Opr opr) {
  LIR_Opr value = equivalent_opr(opr);
  if (value != opr && value->is_register()) {
    return value;
  }
  return opr;
}


LIR_Opr LIR_PeepholeState::equivalent_register_or_constant(LIR_Opr opr) {
  LIR_Opr value = equivalent_opr(opr);
  if (value != opr && (value->is_register() || value->is_constant())) {
    return value;
  }
  return opr;
}


LIR_Opr LIR_PeepholeState::equivalent_register_or_stack(LIR_Opr opr) {
  LIR_Opr value = equivalent_opr(opr);
  if (value != opr && (value->is_register() || value->is_stack())) {
    return value;
  }
  return opr;
}


LIR_Opr LIR_PeepholeState::equivalent_address(LIR_Opr opr) {
  if (opr->is_address()) {
    LIR_Address* addr = opr->as_address_ptr();
    LIR_Opr base = equivalent_register(addr->base());
    LIR_Opr index = equivalent_register(addr->index());
    assert(base->is_register(), "why isn't this a register");
    if ((base != addr->base() || index != addr->index())) {
      return LIR_OprFact::address(new LIR_Address(base, index, addr->scale(), addr->disp()));
    }
  }
  return opr;
}


LIR_Opr LIR_PeepholeState::equivalent_opr(LIR_Opr opr) {
  if (opr->is_register() && opr->is_single_word()) {
    int n = reg2index(opr);
    int vn = _register_values->at_grow(n, -1);
    if (vn != -1) {
      LIR_Opr value = _value_map->at(vn);
      if (value != opr && (value->is_register() || value->is_constant() || value->is_stack())) {
        if (!value->is_float_kind() || (LIROptimize && LIROptimizeFloats)) {
          return value;
        }
      }
    }
  } else if (opr->is_address()) {
    return equivalent_address(opr);
  }

  return opr;
}


void LIR_PeepholeState::kill_stack_slot(int n) {
  int vn = _stack_values->at_grow(n, -1);
  if (vn != -1) {
#ifndef PRODUCT
    if (LIRTracePeephole && Verbose) {
      LIR_Opr value = _value_map->at(vn);
      if (value->is_valid()) {
        tty->print("killing "); value->print();
      }
    }
#endif // PRODUCT
    _value_map->at_put_grow(vn, LIR_OprFact::illegalOpr, LIR_OprFact::illegalOpr);
    NOT_PRODUCT(_state_number++);
  }
}

// kill any registers which refer to this value
void LIR_PeepholeState::kill_register(int n) {
  int vn = _register_values->at_grow(n, -1);
  if (vn != -1) {
#ifndef PRODUCT
    if (LIRTracePeephole && Verbose) {
      LIR_Opr value = _value_map->at(vn);
      if (value->is_valid()) {
        tty->print("killing "); value->print();
      }
    }
#endif // PRODUCT
    // if the current state says esi=ebx and edi=ebx, killing esi should only
    // destroy the esi=ebx equivalence.  However killing ebx will kill both.
    // This is accomplished by destroying the value in value map only when killing

    LIR_Opr value = _value_map->at(vn);
    assert(!value->is_double_word(), "should be not double word values in value map");
    if (!value->is_register() || (value->is_single_word() && reg2index(value) == n)) {
      _value_map->at_put(vn, LIR_OprFact::illegalOpr);
    }
    _register_values->at_put(n, -1);
    NOT_PRODUCT(_state_number++);
  }
}


void LIR_PeepholeState::set_defining_op(int index, int op_index) {
  int def = _defining_op->at_grow(index, -1);
  if (def != op_index) {
    NOT_PRODUCT(_state_number++);
  }
  _defining_op->at_put(index, op_index);
}


// kill any registers which refer to this value
void LIR_PeepholeState::kill_operand(LIR_Opr opr, int op_index) {
#ifndef PRODUCT
  if (LIRTracePeephole && Verbose) {
    tty->print(" kill "); opr->print(); tty->cr();
  }
#endif // PRODUCT
  kill_equivalents(opr);
  if (opr->is_register()) {
    if (opr->is_single_word()) {
      record_defining_op(opr, op_index);
    } else if (opr->is_double_word()) {
      set_defining_op(reg2indexLo(opr), op_index);
      set_defining_op(reg2indexHi(opr), op_index);
    }
  }
}


// kill any registers which refer to this value
void LIR_PeepholeState::kill_equivalents(LIR_Opr opr) {
  if (opr->is_register()) {
    if (opr->is_single_word()) {
      kill_register(reg2index(opr));
    } else if (opr->is_double_word()) {
      kill_register(reg2indexLo(opr));
      kill_register(reg2indexHi(opr));
    }
  } else if (opr->is_stack()) {
    if (opr->is_single_stack()) {
      kill_stack_slot(stack2index(opr));
    } else {
      kill_stack_slot(stack2indexHi(opr));
      kill_stack_slot(stack2indexLo(opr));
    }
  }
}


void LIR_PeepholeState::do_move(LIR_Opr s, LIR_Opr d) {
  if (!(LIROptimize && LIROptimizeFloats) && (d->is_float_kind() || s->is_float_kind())) {
    // don't put any floats into state.
    return;
  }

  if ((d->is_register() && d->is_single_word()) && (d->is_float_kind() == s->is_float_kind()) &&
      ((s->is_register() && s->is_single_word()) || s->is_constant() || s->is_single_stack())) {
    NOT_PRODUCT(_state_number++);
    int vn = -1;
    if (s->is_register()) {
      vn = _register_values->at_grow(reg2index(s), -1);
    }
    if (vn == -1) {
      vn = _value_number++;
      if (s->is_register()) {
        _register_values->at_put_grow(reg2index(s), vn, -1);
      }
    }
    _value_map->at_put_grow(vn, s, LIR_OprFact::illegalOpr);
    if (s->is_single_stack()) {
      // record the location of the current value of this stack slot
      // so that later stores to that slot can kill it
      _stack_values->at_put_grow(stack2index(s), vn, -1);
    }
    _register_values->at_put_grow(reg2index(d), vn, -1);

#ifndef PRODUCT
    // record register names for printing
    if (d->is_register() && d->is_single_word()) {
      _register_names->at_put_grow(reg2index(d), d, LIR_OprFact::illegalOpr);
    }
    if (s->is_register() && s->is_single_word()) {
      _register_names->at_put_grow(reg2index(s), s, LIR_OprFact::illegalOpr);
    }
#endif
  }
}


void LIR_PeepholeState::do_call() {
  for (int i = 0; i < FrameMap::nof_caller_save_cpu_regs; i++) {
    LIR_Opr opr = FrameMap::caller_save_cpu_reg_at(i);
    kill_operand(opr, -1);
  }
}


void LIR_PeepholeState::record_defining_op(LIR_Opr opr, int index) {
  if (opr->is_register() && opr->is_single_word()) {
    if ((LIROptimize && LIROptimizeFloats) || !opr->is_float_kind()) {
#ifndef PRODUCT
      if (LIRTracePeephole && Verbose) {
        tty->print("  def "); opr->print(); tty->cr();
      }
#endif // PRODUCT
      int n = reg2index(opr);
      set_defining_op(n, index);
    }
  }
}


int LIR_PeepholeState::defining_op(LIR_Opr opr) {
  if (opr->is_register() && opr->is_single_word()) {
    int n = reg2index(opr);
    return _defining_op->at_grow(n, -1);
  }
  return -1;
}


void LIR_PeepholeState::start_forward_branch(Label* label) {
  if (_forward_branches.length() == 0) {
    assert(!should_disable_optimization(), "shouldn't be set yet");
  }

  set_disable_optimization(true);
  clear_values();
  assert(!_forward_branches.contains(label), "should only be in here once");
  _forward_branches.append(label);
}


void LIR_PeepholeState::finish_forward_branch(Label* label) {
  if (_forward_branches.contains(label)) {
    _forward_branches.remove(label);
  }
  if (_forward_branches.length() == 0) {
    set_disable_optimization(false);
  }
}


void LIR_PeepholeState::set_disable_optimization(bool flag) {
  if (!_disable_optimization && flag) {
    // turning off optimization implies that all the equivalences
    // should be cleared and the instructions that create them
    // should have their refcounts incremented so that they
    // don't get deleted.  This usually happens for strange regions
    // of code, like things with internal branches or code which
    // uses the math_epilog/math_prolog code.
    for (int i = 0; i < _defining_op->length(); i++) {
      // for ops which defined a value in the state,
      // increment it's ref count so it won't be deleted
      int def = _defining_op->at(i);
      if (def != -1) {
        increment_ref(def);
      }
    }

  }
  clear_values();
  _disable_optimization = flag;
}

// ---------------------------------------------------------------------------


#ifndef PRODUCT
void LIR_Optimizer::print_if_changed(LIR_Op* op) {
  if (_did_stack || _did_opto) {
    if (LIRTracePeephole) {
      if (_did_opto) {
        tty->print("*%-4d ", _opto_count);
      } else {
        tty->print("*     ", _opto_count);
      }
      op->print();
      tty->cr();
    }
  }
}


unsigned int LIR_Optimizer::_opto_count = 0;


bool LIR_Optimizer::allow_opto() {
  return (_opto_count >= (uint)LIROptoStart && _opto_count <= (uint)LIROptoStop && LIROptimize && !_state.should_disable_optimization());
}
#else // PRODUCT
bool LIR_Optimizer::allow_opto() {
  return LIROptimize && !_state.should_disable_optimization();
}
#endif // PRODUCT


LIR_Opr LIR_Optimizer::record_opto(LIR_Opr opr, LIR_Opr new_opr) {
  NOT_PRODUCT(_did_opto = true);
  assert(op()->as_Op1() == NULL || op()->as_Op1()->patch_code() == LIR_Op1::patch_none || (new_opr->is_address() && opr->is_address()), "shouldn't opto patching instructions");
  int old_ref = _state.defining_op(opr);
  if (old_ref == -1 && opr->is_address()) {
    old_ref = _state.defining_op(opr->as_address_ptr()->base());
  }
  if (old_ref != -1) {
    _state.mark_safe_to_delete(old_ref);
  } else {
#ifndef PRODUCT
    if (LIRTracePeephole && LIROptimizeDeleteOps && Verbose && !opr_live_on_entry(opr)) {
      tty->print("*** where did "); new_opr->print(); tty->print_cr(" come from?");
    }
#endif // PRODUCT
  }
  
  int def = _state.defining_op(new_opr);
  if (def == -1 && opr->is_address()) {
    def = _state.defining_op(new_opr->as_address_ptr()->base());
  }
  if (def != -1) {
    _state.increment_ref(def);
  } else {
#ifndef PRODUCT
    if (LIRTracePeephole && LIROptimizeDeleteOps && Verbose && !opr_live_on_entry(opr)) {
      tty->print("*** where did "); new_opr->print(); tty->print_cr(" come from?");
    }
#endif // PRODUCT
  }
  return new_opr;
}


void LIR_Optimizer::process_op() {
  assert(op() == op_at(_op_index), "out of sync");

  // instructions which patch should always be emitted
  LIR_Op1* op1 = op()->as_Op1();
  if (op1 && op1->patch_code() != LIR_Op1::patch_none) {
    _state.increment_ref(_op_index);
  }

  switch (op()->code()) {
  case lir_label: {
    // we need to clear the state at branch targets
    _state.clear_values();
    
    LIR_OpLabel* dest = op()->as_OpLabel();
    _state.finish_forward_branch(dest->label());
    break;
  }

  // see if we have a control flow split
  case lir_branch: {
    LIR_OpBranch* branch = op()->as_OpBranch();
    // branches to blocks or stubs shouldn't effect the peephole state
    // but branches within the block
    if (branch->block() == NULL && branch->stub() == NULL) {
      // this branch is internal to this basic block
      _state.start_forward_branch(branch->label());
    }
    break;
  }
  }

#ifndef PRODUCT
  _did_opto = _did_stack = false;
  if (LIRTracePeephole) {
    _state.print(Verbose ? true : false);
    tty->print("      ");
    op()->print();
  }
#endif // PRODUCT

  // now process the op itself
  op()->emit_code(this);
  for (LIR_OpVisitState::OprMode mode = LIR_OpVisitState::firstMode;
       mode < LIR_OpVisitState::numModes;
       mode = (LIR_OpVisitState::OprMode)(mode + 1)) {
    for (int i = 0; i < visit_state().opr_count(mode); i++) {
      LIR_Opr opr = visit_state().opr_at(mode, i);
      LIR_Opr new_opr = handle_opr(opr, mode);
      if (new_opr != opr) {
        visit_state().set_opr_at(mode, i, new_opr);
      }
    }
  }

  if (visit_state().has_call()) {
    _state.do_call();
  }

  for (int i = 0; i < visit_state().info_count(); i++) {
    handle_info(visit_state().info_at(i));
  }

  assert(op() == op_at(_op_index), "op changed but not updated");

#ifndef PRODUCT
  print_if_changed(op());
  if (_did_opto) {
    _opto_count++;
  }
#endif // PRODUCT

  _last_op = op();
  _op_index++;
}


bool LIR_Optimizer::is_cache_reg(LIR_Opr opr) {
  if (_locals == NULL || !opr->is_register()) {
    return false;
  }
  return _locals->is_cache_reg(opr);
}


bool LIR_Optimizer::is_cached(LIR_Opr opr) {
  if (opr->is_stack() && _locals) {
    LIR_Opr reg = _locals->get_cache_reg(opr);
    if (!reg->is_valid()) {
      assert(!opr->is_stack() || !_locals->get_cache_reg(opr->is_single_stack() ? opr->single_stack_ix() : opr->double_stack_ix()).is_valid(), "can't have cached locals with multiple types");
    }
    return reg->is_valid();
  }
  return false;
}
  

bool LIR_Optimizer::opr_live_on_entry(LIR_Opr opr) {
  if (!(LIROptimize && LIROptimizeFloats) && opr->is_register() && opr->is_float_kind()) {
    return true;
  }

  if (is_cache_reg(opr) || opr->is_stack() || opr->is_address() || opr->is_constant()) {
    return true;
  }

  // we don't want to kill any instructions in CachingChange blocks
  // since our usage information won't be correct
  if (block()->next()->as_CachingChange()) {
    return true;
  }

  // we may have values on the stack that need to remain alive across
  // blocks, so lets generate references to them so we dont kill them.
  // we don't actually have this information right now, so we won't
  // kill any instructions in blocks with non-empty stacks
  if (!block()->state()->stack_is_empty()) {
    if (opr->is_register()) {
      ValueStack* stack = block()->state();
      int i = stack->stack_size();
      Value tos_val = stack->stack_at_dec(i);
      RInfo tos_reg = ValueGen::result_register_for(tos_val->type());
      if (tos_reg.overlaps(opr->rinfo())) {
        return true;
      }
    }
  }

#ifdef SPARC
  if (opr->is_register() && opr->is_single_cpu()) {
    Register r = opr->as_register();
    if (r->is_global() || r->is_in()) {
      return true;
    }
  }
#endif

  return false;
}


bool LIR_Optimizer::opr_live_on_exit(LIR_Opr opr) {
  if (!(LIROptimize && LIROptimizeFloats) && opr->is_register() && opr->is_float_kind()) {
    return true;
  }

  if (is_cache_reg(opr) || opr->is_stack() || opr->is_address()) {
    return true;
  }

  // we don't want to kill any instructions in CachingChange blocks
  // since our usage information won't be correct
  if (block()->next()->as_CachingChange()) {
    return true;
  }

  // we may have values on the stack that need to remain alive across
  // blocks, so lets generate references to them so we dont kill them.
  // we don't actually have this information right now, so we won't
  // kill any instructions in blocks with non-empty stacks
  if (!block()->end()->state()->stack_is_empty()) {
    if (opr->is_register()) {
      ValueStack* stack = block()->end()->state();
      int i = stack->stack_size();
      Value tos_val = stack->stack_at_dec(i);
      RInfo tos_reg = ValueGen::result_register_for(tos_val->type());
      if (tos_reg.is_same(opr->rinfo()) || tos_reg.overlaps(opr->rinfo())) {
        return true;
      }
    }
  }

#ifdef SPARC
  if (opr->is_register()) {
    if (opr->is_single_cpu()) {
      Register r = opr->as_register();
      if (r->is_global() || r->is_out()) {
        return true;
      }
    } else if (opr->is_double_cpu()) {
      RInfo r = opr->rinfo();
      if (r.as_register_lo()->is_global() || r.as_register_lo()->is_out()) {
        return true;
      }
      if (r.as_register_hi()->is_global() || r.as_register_hi()->is_out()) {
        return true;
      }
    }
  }
#endif

  return false;
}


void LIR_Optimizer::block_prolog() {
  _op_index = 0;
  _last_op = NULL;
  _locals = block()->local_mapping();
  _state.initialize(_locals);
  if (LIRTracePeephole || TraceCachedLocals) {
    block()->print();
    if (_locals) _locals->print();
  }
}


void LIR_Optimizer::block_epilog() {
  if (!LIROptimize) {
    return;
  }

  LIR_OpList* inst = block()->lir()->instructions_list();

#ifdef ASSERT
  LIR_OpList* predead = new LIR_OpList();
  if (inst->length() > 0) {
    predead->at_put_grow(inst->length() - 1, NULL, NULL);
    for (int j = inst->length() - 1; j >= 0; j--) {
      if (_state.is_safe_to_delete(j) && _state.refcount(j) == 0) {
        predead->at_put_grow(j, inst->at(j), NULL);
      }
    }
  }
#endif // ASSERT

#ifndef PRODUCT
  if (LIRTracePeephole && Verbose) {
    tty->print_cr("instruction refcounts");
    for (int j = 0; j < inst->length(); j++) {
      tty->print("[%d]%c ", _state.refcount(j), _state.is_safe_to_delete(j) ? 'd' : ' ');
      inst->at(j)->print();
    }
  }
#endif // PRODUCT

  for (int j = inst->length() - 1; j >= 0; j--) {
    if (_state.is_safe_to_delete(j) && _state.refcount(j) == 0) {
      LIR_Op* dead = inst->at(j);
      assert(dead == predead->at(j), "should match");
      if (LIROptimize && LIROptimizeDeleteOps &&
          ((dead->code() == lir_move && !opr_live_on_exit(dead->result_opr())) || dead->code() == lir_nop)) {
        assert(dead->info() == NULL, "shouldn't be killing things with info");
        assert((dead->as_Op1() == NULL || dead->as_Op1()->patch_code() == LIR_Op1::patch_none), "shouldn't be killing patchable things");
        assert(!is_cache_reg(dead->result_opr()), "shouldn't be killing moves to locals");
        assert(!dead->result_opr()->is_pointer(), "shouldn't be killing moves to address");
        assert(!dead->result_opr()->is_stack(), "shouldn't be killing moves to stack");
#ifndef PRODUCT
        if (LIRTracePeephole) {
          tty->print("deleting "); dead->print();
        }
#endif // PRODUCT
        inst->remove_at(j);
      }
    } else {
      LIR_Op* op = inst->at(j);
#ifdef ASSERT
      if (LIRTracePeephole) {
        if (_state.refcount(j) == 0 && op->code() == lir_move && !opr_live_on_exit(op->result_opr())) {
          tty->print_cr("would like to delete:");
          op->print();
        }
      }
#endif // ASSERT
      assert(predead->at(j) == NULL, "safe_to_delete and dead don't match");
    }
  }
}


void LIR_Optimizer::record_opr_reference(LIR_Opr opr) {
  int def = _state.record_opr_reference(opr);
#ifndef PRODUCT
  if (LIROptimize && LIRTracePeephole && Verbose && def == -1 && opr->is_single_cpu() && !opr_live_on_entry(opr)) {
    tty->print("*** where did "); opr->print(); tty->print_cr(" come from?");
  }
  if (LIRTracePeephole && !(!LIROptimize || def == -1 || opr->is_double_cpu() ||
        op_at(def)->result_opr()->is_equivalent(opr) || op_at(def)->result_opr()->overlaps(opr) ||
        (op_at(def)->code() == lir_delay_slot && op_at(def)->as_OpDelay()->delay_op()->result_opr()->is_equivalent(opr)))) {
    if (!opr->is_float_kind()) {
      tty->print_cr("for: "); op_at(def)->print();
      op_at(def)->result_opr()->print(); tty->print(" not the same as "); opr->print(); tty->cr();
    }
  }
#endif // PRODUCT
}


LIR_Op* LIR_Optimizer::op_at(int index) {
  assert(block() != NULL, "have to have a block");
  LIR_OpList* lir = block()->lir()->instructions_list();
  if (index >= 0 && index < lir->length()) {
    return lir->at(index);
  } else {
    return NULL;
  }
}


void LIR_Optimizer::replace_op(int index, LIR_Op* new_op) {
  assert(index != _op_index || new_op == op() || _did_opto, "should have set opto if we changed this instruction");
  LIR_OpList* inst = block()->lir()->instructions_list();
  LIR_Op* old_op = op_at(index);
  inst->at_put(index, new_op);
  if (index == _op_index) {
    visit_state().set_op(new_op);
  }
  // make sure that the effects of the instruction being deleted are deleted as well.
  if (old_op->result_opr()->is_valid() && _state.defining_op(old_op->result_opr()) == index) {
    _state.kill_operand(old_op->result_opr(), -1);
  }
}

LIR_Op* LIR_Optimizer::defining_op(LIR_Opr opr) {
  return op_at(_state.defining_op(opr));
}


LIR_Op* LIR_Optimizer::next_op_with_code(LIR_Code code) {
  LIR_Op* next_op = op_at(_op_index + 1);
  if (next_op == NULL || next_op->code() != code) {
    return NULL;
  }
  return next_op;
}


LIR_Opr LIR_Optimizer::replace_stack_opr(LIR_Opr opr) {
  if (_locals) {
    LIR_Opr new_opr = _locals->get_cache_reg(opr);
    if (new_opr->is_valid()) {
      NOT_PRODUCT(_did_stack = true);
      opr = new_opr;
    } else {
      assert(!opr->is_stack() || !_locals->get_cache_reg(opr->is_single_stack() ? opr->single_stack_ix() : opr->double_stack_ix()).is_valid(), "can't have cached locals with multiple types");
    }
  }
  return opr;
}


void LIR_Optimizer::result_substitute() {
  LIR_Op* next = next_op_with_code(lir_move);
  if (next != NULL) {
    LIR_Op1* move = next->as_Op1();
    assert(move != NULL, "must have changed type");
    LIR_Opr result_reg = move->result_opr();
    bool was_cached = false;
    if (is_cached(result_reg)) {
      result_reg = replace_stack_opr(result_reg);
      was_cached = true;
    }
    
    if (move->in_opr()->is_equivalent(op()->result_opr())) {
      if (result_reg->is_register() && move->in_opr()->is_register() &&
          ((result_reg->is_float_kind() && move->in_opr()->is_float_kind()) ||
           (!result_reg->is_float_kind() && !move->in_opr()->is_float_kind())) &&
          ((LIROptimize && LIROptimizeFloats) || (!result_reg->is_float_kind() && !move->in_opr()->is_float_kind())) &&
          allow_opto()) {
        op()->_result = result_reg;
        NOT_PRODUCT(_did_opto = true);
        
        // mark this as deletable
        assert(op_at(_op_index + 1) == next, "marking wrong op");
        _state.mark_safe_to_delete(_op_index + 1);

        // reverse the move
        move->_result = move->_opr;
        move->_opr = result_reg;
      }
    }
  }
}


void LIR_Optimizer::process_move(LIR_Op1* op1) {
  LIR_Opr src = op1->in_opr();
  LIR_Opr dst = op1->result_opr();
    
  if (is_cached(dst)) {
    assert(!src->is_stack(), "can't both be stack");
    if (dst->is_single_stack()) {
      _state.mark_safe_to_delete(_op_index);
    }
    dst = replace_stack_opr(dst);
  } else if (is_cached(src)) {
    assert(!dst->is_stack(), "can't both be stack");
    if (src->is_single_stack()) {
      _state.mark_safe_to_delete(_op_index);
    }
    src = replace_stack_opr(op1->in_opr());
    dst = op1->result_opr();
  }
    
  bool substitute_result = false;
  if (LIROptimize) {
    substitute_result = optimize_move(op1, src, dst);
  }

  op1->_opr = src;
  op1->_result = dst;
    
  assert(src == op1->in_opr(), "should match");
  assert(dst == op1->result_opr(), "should match");
  assert(!is_cached(src) && !is_cached(dst), "should have been handled already");
    
  if (substitute_result) {
    result_substitute();
  }
  src = op1->in_opr();
  dst = op1->result_opr();

  if (src->is_equivalent(dst) && LIROptimizeDeleteOps && allow_opto()) {
    // let's get rid of this instruction completely
    assert(op()->info() == NULL, "let's hope not");
    NOT_PRODUCT(_did_opto = true);
    replace_op(_op_index, new LIR_Op0(lir_nop, NULL));
    //    _state.mark_safe_to_delete(_op_index);
    return;
  }

  if (_state.should_disable_optimization()) {
    // when processing internal branches, don't allow moves inside the
    // branch region to destroy instructions that produced values before
    // the branch. Do this by incrementing the reference count of instructions
    // with the same destination.
    int def = _state.defining_op(dst);
    if (def != -1) {
      _state.increment_ref(def);
    }
  }

  record_opr_reference(src);
  if (dst->is_address()) {
    record_opr_reference(dst);
  }

  if (op1->_info != NULL) {
    handle_info(op1->_info);
  }

  _state.kill_operand(dst, _op_index);
    
  // we don't want to track constant values which are
  // going to be patched later.
  if (op1->patch_code() == LIR_Op1::patch_none) {
    _state.do_move(src, dst);
  }

  if (!_saw_fpu2cpu_moves &&
      src->is_register() && dst->is_register() &&
      src->is_float_kind() != dst->is_float_kind()) {
    _saw_fpu2cpu_moves = true;
  }
}


LIR_Opr LIR_Optimizer::handle_opr(LIR_Opr opr, LIR_OpVisitState::OprMode mode) {
  if (is_cached(opr)) {
    opr = replace_stack_opr(opr);
    NOT_PRODUCT(_did_stack = true);
  }

  if (LIROptimize) {
    if (opr->is_address()) {
      opr = maybe_opto(opr, _state.equivalent_address(opr));
    }
    
    switch (mode) {
    case LIR_OpVisitState::outputMode:
      if (opr->is_address()) {
        record_opr_reference(opr);
      }
      _state.kill_operand(opr, _op_index);
      break;
      
    case LIR_OpVisitState::inputMode:
      record_opr_reference(opr);
      break;
      
    case LIR_OpVisitState::tempMode:
      record_opr_reference(opr);
      _state.kill_operand(opr, -1);
      break;
    }
  }
  return opr;
}


// for now just record the register references from the
// CodeEmitInfo.  It may be the case that the only live reference
// to register is from the CodeEmitInfo because the peephole
// optimization was able to remove the temporary.  It might be 
// a good idea to delete the reference in this case.
void LIR_Optimizer::record_register_oops(CodeEmitInfo* info) {
  const RInfoCollection* oops = info->register_oops();
  if (oops == NULL) {
    return;
  }

  for (int i = 0; i < oops->length(); i++) {
    handle_opr(LIR_OprFact::rinfo(oops->at(i), T_OBJECT), LIR_OpVisitState::tempMode);
  }
}


void LIR_Optimizer::handle_info(CodeEmitInfo* info) {
  if (info == NULL) {
    return;
  }

  _state.increment_ref(_op_index);

  // for the caching of locals we need to fixup the CodeEmitInfo
  // to reflect the actual location of the locals.
  if (_locals) {
    if (DeoptC1) {
      GrowableArray<LIR_Opr>* stack = info->lir_expression_stack();
      if (stack) {
        int l = stack->length();
        for (int i = 0; i < l; i++) {
          LIR_Opr opr = stack->at(i);
          LIR_Opr new_opr = LIR_OprFact::illegalOpr;

          // see if the stack element should be replaced by another LIR_Opr
          if (opr->is_stack()) {
            new_opr = _locals->get_cache_reg(opr);
          } else if (opr->is_register()) {
            // try to replace any expression stack registers with registers that contain locals.
            // In general, one register is as good as another but it should be a register which
            // is not caller saved at this point, and caching register are guaranteed to be safe.
            new_opr = _state.equivalent_register(opr);
            if (!_locals->is_cache_reg(new_opr)) {
              new_opr = LIR_OprFact::illegalOpr;
            }
          } else {
            assert(opr->is_constant(), "nothing else");
          }

          if (new_opr->is_valid()) {
            assert(new_opr->is_type_compatible(opr), "should be compatible");
            // make sure the type field of the new_opr is set correctly
            // and replace the value in stack
            new_opr = new_opr->with_type_of(opr);
            stack->at_put(i, new_opr);
            opr = new_opr;
          } 
          if (opr->is_register()) {
            // record a dependency with this register
            handle_opr(opr, LIR_OpVisitState::inputMode);
          }
        }
      }
    }

    // record the local register mapping for this block into
    // this CodeEmitInfo.  since CodeEmitInfos may be used
    // multiple times, we need to make sure that we are setting
    // them up consistently.
    if (info->local_mapping() == NULL) {
      info->set_local_mapping(_locals);
    } else {
      assert(info->local_mapping() == _locals, "why aren't these the same");
    }
  }

  const RInfoCollection *oops = info->register_oops();
  if (oops != NULL) {
    // the modeling of the register state may record an equivalence between registers
    // that is correct but is not reflected in the oop maps.  So any register equivalences
    // involving oops need to be cleared if both registers aren't recorded as oops.
    for (int i = 0; i < oops->length(); i++) {
      LIR_Opr oop_reg = LIR_OprFact::rinfo(oops->at(i));
      LIR_Opr eq = _state.equivalent_register(oop_reg);
      if (eq->is_valid() && !oops->contains(eq->rinfo())) {
        _state.kill_equivalents(oop_reg);
      }
    }
  }

  record_register_oops(info);

#ifndef PRODUCT
  // tag the info so we know we've at least seen it, then we
  // can make sure that if we generate debug info for it later
  // that it was processed appropriately.
  info->set_lir_adjusted();
#endif // PRODUCT
}


void LIR_Optimizer::optimize() {
  // first we expand the LIR so that each LIR instruction will
  // generate only a single machine instruction.  In some cases we will
  // probably still generate
  if ((LIRTracePeephole || TraceCachedLocals) && PrintLIR ) {
    print_LIR(ir()->code());
  }

  optimize(ir()->code());
}


void LIR_Optimizer::optimize(BlockList* blocks) {
  _visit_state.reset();
  for (int n = 0; n < blocks->length(); n++) {
    BlockBegin* bb = blocks->at(n);
    optimize(bb);
  }
}


void LIR_Optimizer::optimize(BlockBegin* bb) {
  if (bb->lir() != NULL) {
    LIR_OpList* inst = bb->lir()->instructions_list();
    set_block(bb);
    block_prolog();
    for (int i = 0; i < inst->length(); i++) {
      LIR_Op* op = inst->at(i);
      _visit_state.set_op(op);
      process_op();
      _visit_state.reset();
    }
    block_epilog();
    set_block(NULL);
  }
}


LIR_Optimizer::LIR_Optimizer(IR* ir):
    _ir(ir)
  , _last_op(NULL)
  , _locals(NULL)
  , _op_index(0)
  , _block(NULL)
  , _saw_fpu2cpu_moves(false)
  , _state() {
  NOT_PRODUCT(_did_opto = _did_stack = false);
  pd_init();
}
