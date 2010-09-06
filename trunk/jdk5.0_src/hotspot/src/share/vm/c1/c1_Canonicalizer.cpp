#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Canonicalizer.cpp	1.46 04/03/31 18:13:11 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_Canonicalizer.cpp.incl"


void Canonicalizer::set_canonical(Value x) {
  assert(x != NULL, "value must exist");
  // Note: we can not currently substitute root nodes which show up in
  // the instruction stream (because the instruction list is embedded
  // in the instructions). Phis always show up as roots. It is fatal
  // to embed a phi into the instruction stream as block merging will
  // not be able to substitute the old phi.
  if (x->as_Phi() != NULL) {
    if (PrintCanonicalization) {
      tty->print_cr("canonicalization produced a phi node; rejected");
    }
    return;
  }
  if (canonical() != x) {
    if (PrintCanonicalization) {
      canonical()->print(); tty->cr();
      tty->print_cr("canonicalized to:");
      x->print(); tty->cr();
      tty->cr();
    }
    _canonical = x;
  }
}


void Canonicalizer::move_const_to_right(Op2* x) {
  if (x->x()->type()->is_constant() && x->is_commutative()) x->swap_operands();
}


void Canonicalizer::do_Op2(Op2* x) {
  if (x->x()->type()->is_constant() && x->y()->type()->is_constant()) {
    // do constant folding for selected operations
    switch (x->type()->tag()) {
      case intTag:
        { jint a = x->x()->type()->as_IntConstant()->value();
          jint b = x->y()->type()->as_IntConstant()->value();
          switch (x->op()) {
            case Bytecodes::_iadd: set_constant(a + b); return;
            case Bytecodes::_isub: set_constant(a - b); return;
            case Bytecodes::_imul: set_constant(a * b); return;
            // Note: for div and rem, make sure that C semantics
            //       corresponds to Java semantics!
            case Bytecodes::_iand: set_constant(a & b); return;
            case Bytecodes::_ior : set_constant(a | b); return;
            case Bytecodes::_ixor: set_constant(a ^ b); return;
          }
        }
        break;
      case longTag:
        { jlong a = x->x()->type()->as_LongConstant()->value();
          jlong b = x->y()->type()->as_LongConstant()->value();
          switch (x->op()) {
            case Bytecodes::_ladd: set_constant(a + b); return;
            case Bytecodes::_lsub: set_constant(a - b); return;
            case Bytecodes::_lmul: set_constant(a * b); return;
            // Note: for div and rem, make sure that C semantics
            //       corresponds to Java semantics!
            case Bytecodes::_land: set_constant(a & b); return;
            case Bytecodes::_lor : set_constant(a | b); return;
            case Bytecodes::_lxor: set_constant(a ^ b); return;
          }
        }
        break;
      // other cases not implemented (must be extremely careful with floats & doubles!)
    }
  }
  // make sure constant is on the right side, if any
  move_const_to_right(x);

  if (x->y()->type()->is_constant()) {
    // do constant folding for selected operations
    switch (x->type()->tag()) {
      case intTag:
        if (x->y()->type()->as_IntConstant()->value() == 0) {
          switch (x->op()) {
            case Bytecodes::_iadd: set_canonical(x->x()); return;
            case Bytecodes::_isub: set_canonical(x->x()); return;
            case Bytecodes::_imul: set_constant(0); return;
              // Note: for div and rem, make sure that C semantics
              //       corresponds to Java semantics!
            case Bytecodes::_iand: set_constant(0); return;
            case Bytecodes::_ior : set_canonical(x->x()); return;
          }
        }
        break;
      case longTag:
        if (x->y()->type()->as_LongConstant()->value() == (jlong)0) {
          switch (x->op()) {
            case Bytecodes::_ladd: set_canonical(x->x()); return;
            case Bytecodes::_lsub: set_canonical(x->x()); return;
            case Bytecodes::_lmul: set_constant((jlong)0); return;
              // Note: for div and rem, make sure that C semantics
              //       corresponds to Java semantics!
            case Bytecodes::_land: set_constant((jlong)0); return;
            case Bytecodes::_lor : set_canonical(x->x()); return;
          }
        }
        break;
    }
  }
}


void Canonicalizer::do_Phi            (Phi*             x) {}
void Canonicalizer::do_Constant       (Constant*        x) {}
void Canonicalizer::do_Local          (Local*           x) {}
void Canonicalizer::do_LoadLocal      (LoadLocal*       x) {}
void Canonicalizer::do_StoreLocal     (StoreLocal*      x) {}
void Canonicalizer::do_LoadField      (LoadField*       x) {}
void Canonicalizer::do_StoreField     (StoreField*      x) {}

void Canonicalizer::do_ArrayLength    (ArrayLength*     x) {
  NewArray* array = x->array()->as_NewArray();
  if (array != NULL && array->length() != NULL) {
    Constant* length = array->length()->as_Constant();
    if (length != NULL) {
      set_canonical(length);
    }
  }
}

void Canonicalizer::do_LoadIndexed    (LoadIndexed*     x) {}
void Canonicalizer::do_StoreIndexed   (StoreIndexed*    x) {}
void Canonicalizer::do_CachingChange  (CachingChange*   x) {}


void Canonicalizer::do_NegateOp(NegateOp* x) {
  ValueType* t = x->x()->type();
  if (t->is_constant()) {
    switch (t->tag()) {
      case intTag   : set_constant(-t->as_IntConstant   ()->value()); return;
      case longTag  : set_constant(-t->as_LongConstant  ()->value()); return;
      case floatTag : set_constant(-t->as_FloatConstant ()->value()); return;
      case doubleTag: set_constant(-t->as_DoubleConstant()->value()); return;
      default       : ShouldNotReachHere();
    }
  }
}


void Canonicalizer::do_ArithmeticOp   (ArithmeticOp*    x) { do_Op2(x); }


void Canonicalizer::do_ShiftOp        (ShiftOp*         x) {
  ValueType* t = x->x()->type();
  if (t->is_constant()) {
    switch (t->tag()) {
      case intTag   : if (t->as_IntConstant()->value() == 0)  set_constant(0); return;
      case longTag  : if (t->as_LongConstant()->value() == (jlong)0) set_constant(jlong_cast(0)); return;
      default       : ShouldNotReachHere();
    }
  }
  ValueType* t2 = x->y()->type();
  if (t2->is_constant()) {
    switch (t2->tag()) {
      case intTag   : if (t2->as_IntConstant()->value() == 0)  set_canonical(x->x()); return;
      default       : ShouldNotReachHere();
    }
  }
}


void Canonicalizer::do_LogicOp        (LogicOp*         x) { do_Op2(x); }
void Canonicalizer::do_CompareOp      (CompareOp*       x) {}


void Canonicalizer::do_IfInstanceOf(IfInstanceOf*    x) {}

void Canonicalizer::do_IfOp(IfOp* x) {
  // Caution: do not use do_Op2(x) here for now since
  //          we map the condition to the op for now!
  move_const_to_right(x);
}


void Canonicalizer::do_Intrinsic      (Intrinsic*       x) {
  switch (x->id()) {
  case ciMethod::_floatToRawIntBits   : {
    FloatConstant* c = x->argument_at(0)->type()->as_FloatConstant();
    if (c != NULL) {
      JavaValue v;
      v.set_jfloat(c->value());
      set_constant(v.get_jint());
    }
    break;
  }
  case ciMethod::_intBitsToFloat      : {
    IntConstant* c = x->argument_at(0)->type()->as_IntConstant();
    if (c != NULL) {
      JavaValue v;
      v.set_jint(c->value());
      set_constant(v.get_jfloat());
    }
    break;
  }
  case ciMethod::_doubleToRawLongBits : {
    DoubleConstant* c = x->argument_at(0)->type()->as_DoubleConstant();
    if (c != NULL) {
      JavaValue v;
      v.set_jdouble(c->value());
      set_constant(v.get_jlong());
    }
    break;
  }
  case ciMethod::_longBitsToDouble    : {
    LongConstant* c = x->argument_at(0)->type()->as_LongConstant();
    if (c != NULL) {
      JavaValue v;
      v.set_jlong(c->value());
      set_constant(v.get_jdouble());
    }
    break;
  }
  }
}

void Canonicalizer::do_Convert        (Convert*         x) {}
void Canonicalizer::do_NullCheck      (NullCheck*       x) {}
void Canonicalizer::do_Invoke         (Invoke*          x) {}
void Canonicalizer::do_NewInstance    (NewInstance*     x) {}
void Canonicalizer::do_NewTypeArray   (NewTypeArray*    x) {}
void Canonicalizer::do_NewObjectArray (NewObjectArray*  x) {}
void Canonicalizer::do_NewMultiArray  (NewMultiArray*   x) {}
void Canonicalizer::do_CheckCast      (CheckCast*       x) {}
void Canonicalizer::do_InstanceOf     (InstanceOf*      x) {}
void Canonicalizer::do_MonitorEnter   (MonitorEnter*    x) {}
void Canonicalizer::do_MonitorExit    (MonitorExit*     x) {}
void Canonicalizer::do_BlockBegin     (BlockBegin*      x) {}
void Canonicalizer::do_Goto           (Goto*            x) {}


static bool is_true(jlong x, If::Condition cond, jlong y) {
  switch (cond) {
    case If::eql: return x == y;
    case If::neq: return x != y;
    case If::lss: return x <  y;
    case If::leq: return x <= y;
    case If::gtr: return x >  y;
    case If::geq: return x >= y;
  }
  ShouldNotReachHere();
  return false;
}


void Canonicalizer::do_If(If* x) {
  // move const to right
  if (x->x()->type()->is_constant()) x->swap_operands();
  // simplify
  const Value l = x->x(); ValueType* lt = l->type();
  const Value r = x->y(); ValueType* rt = r->type();
  if (lt->is_constant() && rt->is_constant()) {
    // pattern: If (lc cond rc) => simplify to: Goto
    Goto* g = NULL;
    switch (lt->tag()) {
      case intTag:
        g = new Goto(x->sux_for(is_true(lt->as_IntConstant ()->value(), x->cond(), rt->as_IntConstant ()->value())), x->is_safepoint());
        break;
      case longTag:
        g = new Goto(x->sux_for(is_true(lt->as_LongConstant()->value(), x->cond(), rt->as_LongConstant()->value())), x->is_safepoint());
        break;
      // other cases not implemented (must be extremely careful with floats & doubles!)
    }
    if (g != NULL) {
      // If this If is a safepoint then the debug information should come from the state_before of the If.
      g->set_state_before(x->state_before());
      set_canonical(g);
    }
  } else if (rt->as_IntConstant() != NULL) {
    // pattern: If (l cond rc) => investigate further
    const jint rc = rt->as_IntConstant()->value();
    if (l->as_CompareOp() != NULL) {
      // pattern: If ((a cmp b) cond rc) => simplify to: If (x cond y) or: Goto
      CompareOp* cmp = l->as_CompareOp();
      bool unordered_is_less = cmp->op() == Bytecodes::_fcmpl || cmp->op() == Bytecodes::_dcmpl;
      BlockBegin* lss_sux = x->sux_for(is_true(-1, x->cond(), rc)); // successor for a < b
      BlockBegin* eql_sux = x->sux_for(is_true( 0, x->cond(), rc)); // successor for a = b
      BlockBegin* gtr_sux = x->sux_for(is_true(+1, x->cond(), rc)); // successor for a > b
      BlockBegin* nan_sux = unordered_is_less ? lss_sux : gtr_sux ; // successor for unordered
      // Note: At this point all successors (lss_sux, eql_sux, gtr_sux, nan_sux) are
      //       equal to x->tsux() or x->fsux(). Furthermore, nan_sux equals either
      //       lss_sux or gtr_sux.
      if (lss_sux == eql_sux && eql_sux == gtr_sux) {
        // all successors identical => simplify to: Goto
        set_canonical(new Goto(lss_sux, x->is_safepoint()));
      } else {
        // two successors differ and two successors are the same => simplify to: If (x cmp y)
        // determine new condition & successors
        If::Condition cond;
        BlockBegin* tsux = NULL;
        BlockBegin* fsux = NULL;
             if (lss_sux == eql_sux) { cond = If::leq; tsux = lss_sux; fsux = gtr_sux; }
        else if (lss_sux == gtr_sux) { cond = If::neq; tsux = lss_sux; fsux = eql_sux; }
        else if (eql_sux == gtr_sux) { cond = If::geq; tsux = eql_sux; fsux = lss_sux; }
        else                         { ShouldNotReachHere();                           }
        set_canonical(new If(cmp->x(), cond, nan_sux == tsux, cmp->y(), tsux, fsux, cmp->state_before(), x->is_safepoint()));
        set_bci(cmp->bci());
      }
    } else if (l->as_InstanceOf() != NULL) {
      // NOTE: Code permanently disabled for now since it leaves the old InstanceOf
      //       instruction in the graph (it is pinned). Need to fix this at some point.
      return;
      // pattern: If ((obj instanceof klass) cond rc) => simplify to: IfInstanceOf or: Goto
      InstanceOf* inst = l->as_InstanceOf();
      BlockBegin* is_inst_sux = x->sux_for(is_true(1, x->cond(), rc)); // successor for instanceof == 1
      BlockBegin* no_inst_sux = x->sux_for(is_true(0, x->cond(), rc)); // successor for instanceof == 0
      if (is_inst_sux == no_inst_sux && inst->is_loaded()) {
        // both successors identical and klass is loaded => simplify to: Goto
        set_canonical(new Goto(is_inst_sux, x->is_safepoint()));       
      } else {
        // successors differ => simplify to: IfInstanceOf
        set_canonical(new IfInstanceOf(inst->klass(), inst->obj(), true, inst->bci(), is_inst_sux, no_inst_sux));
      }
    }
  }
}


void Canonicalizer::do_TableSwitch(TableSwitch* x) {
  if (x->number_of_sux() == 1) {
    // NOTE: Code permanently disabled for now since the switch statement's
    //       tag expression may produce side-effects in which case it must
    //       be executed.
    return;
    // simplify to Goto
    set_canonical(new Goto(x->default_sux(), x->is_safepoint()));
  } else if (x->number_of_sux() == 2) {
    // NOTE: Code permanently disabled for now since it produces two new nodes
    //       (Constant & If) and the Canonicalizer cannot return them correctly
    //       yet. For now we copied the corresponding code directly into the
    //       GraphBuilder (i.e., we should never reach here).
    return;
    // simplify to If
    assert(x->lo_key() == x->hi_key(), "keys must be the same");
    Constant* key = new Constant(new IntConstant(x->lo_key()));
    set_canonical(new If(x->tag(), If::eql, true, key, x->sux_at(0), x->default_sux(), x->state_before(), x->is_safepoint()));
  }
}


void Canonicalizer::do_LookupSwitch(LookupSwitch* x) {
  if (x->number_of_sux() == 1) {
    // NOTE: Code permanently disabled for now since the switch statement's
    //       tag expression may produce side-effects in which case it must
    //       be executed.
    return;
    // simplify to Goto
    set_canonical(new Goto(x->default_sux(), x->is_safepoint()));
  } else if (x->number_of_sux() == 2) {
    // NOTE: Code permanently disabled for now since it produces two new nodes
    //       (Constant & If) and the Canonicalizer cannot return them correctly
    //       yet. For now we copied the corresponding code directly into the
    //       GraphBuilder (i.e., we should never reach here).
    return;
    // simplify to If
    assert(x->length() == 1, "length must be the same");
    Constant* key = new Constant(new IntConstant(x->key_at(0)));
    set_canonical(new If(x->tag(), If::eql, true, key, x->sux_at(0), x->default_sux(), x->state_before(), x->is_safepoint()));
  }
}


void Canonicalizer::do_Return         (Return*          x) {}
void Canonicalizer::do_Throw          (Throw*           x) {}
void Canonicalizer::do_Base           (Base*            x) {}

static bool match_index_and_scale(Instruction*  instr,
                                  Instruction** index,
                                  int*          log2_scale,
                                  Instruction** instr_to_unpin) {
  *instr_to_unpin = NULL;

  // Skip conversion ops
  Convert* convert = instr->as_Convert();
  if (convert != NULL) {
    instr = convert->value();
  }

  ShiftOp* shift = instr->as_ShiftOp();
  if (shift != NULL) {
    if (shift->is_pinned()) {
      *instr_to_unpin = shift;
    }
    // Constant shift value?
    Constant* con = shift->y()->as_Constant();
    if (con == NULL) return false;
    // Well-known type and value?
    IntConstant* val = con->type()->as_IntConstant();
    if (val == NULL) return false;
    if (shift->x()->type() != intType) return false;
    *index = shift->x();
    int tmp_scale = val->value();
    if (tmp_scale >= 0 && tmp_scale < 4) {
      *log2_scale = tmp_scale;
      return true;
    } else {
      return false;
    }
  }

  ArithmeticOp* arith = instr->as_ArithmeticOp();
  if (arith != NULL) {
    if (arith->is_pinned()) {
      *instr_to_unpin = arith;
    }
    // Check for integer multiply
    if (arith->op() == Bytecodes::_imul) {
      // See if either arg is a known constant
      Constant* con = arith->x()->as_Constant();
      if (con != NULL) {
        *index = arith->y();
      } else {
        con = arith->y()->as_Constant();
        if (con == NULL) return false;
        *index = arith->x();
      }
      if ((*index)->type() != intType) return false;
      // Well-known type and value?
      IntConstant* val = con->type()->as_IntConstant();
      if (val == NULL) return false;
      switch (val->value()) {
      case 1: *log2_scale = 0; return true;
      case 2: *log2_scale = 1; return true;
      case 4: *log2_scale = 2; return true;
      case 8: *log2_scale = 3; return true;
      default:            return false;
      }
    }
  }

  // Unknown instruction sequence; don't touch it
  return false;
}


static bool match(UnsafeRawOp* x,
                  Instruction** base,
                  Instruction** index,
                  int*          log2_scale) {
  Instruction* instr_to_unpin = NULL;
  ArithmeticOp* root = x->base()->as_ArithmeticOp();
  if (root == NULL) return false;
  // Limit ourselves to addition for now
  if (root->op() != Bytecodes::_ladd) return false;
  // Try to find shift or scale op
  if (match_index_and_scale(root->y(), index, log2_scale, &instr_to_unpin)) {
    *base = root->x();
  } else if (match_index_and_scale(root->x(), index, log2_scale, &instr_to_unpin)) {
    *base = root->y();
  } else if (root->y()->as_Convert() != NULL) {
    Convert* convert = root->y()->as_Convert();
    if (convert->op() == Bytecodes::_i2l && convert->value()->type() == intType) {
      // pick base and index, setting scale at 1
      *base  = root->x();
      *index = convert->value();
      *log2_scale = 0;
    } else {
      return false;
    }
  } else {
    // doesn't match any expected sequences
    return false;
  }
  // Typically the addition is pinned, as it is the result of an
  // inlined routine. We want to unpin it so as to avoid the long
  // addition, but in order to do so we must still ensure that the
  // operands are pinned, as they may be computed arbitrarily before
  // the Unsafe op completes (even if the Unsafe op is pinned). At
  // this point we do not really need to pin Unsafe raw or object
  // gets.
  if (root->is_pinned()) {
    if (root->pin_state() == Instruction::PinInlineReturnValue) {
      assert(x->is_pinned(), "All unsafe raw ops should be pinned");
      root->unpin(Instruction::PinInlineReturnValue);
      (*base)->pin();
      (*index)->pin();
    } else {
      // can't safely unpin this instruction
      return false;
    }
  }

  if (PrintUnsafeOptimization && instr_to_unpin != NULL) {
    tty->print_cr("pin_state = 0x%x", instr_to_unpin->pin_state());
    instr_to_unpin->print();
  }
  return true;
}


void Canonicalizer::do_UnsafeRawOp(UnsafeRawOp* x) {
  Instruction* base = NULL;
  Instruction* index = NULL;
  int          log2_scale;

  if (match(x, &base, &index, &log2_scale)) {
    x->set_base(base);
    x->set_index(index);
    x->set_log2_scale(log2_scale);
    if (PrintUnsafeOptimization) {
      tty->print_cr("Canonicalizer: UnsafeRawOp id %d: base = id %d, index = id %d, log2_scale = %d",
                    x->id(), x->base()->id(), x->index()->id(), x->log2_scale());
    }
  }
}

void Canonicalizer::do_UnsafeGetRaw(UnsafeGetRaw* x) { if (OptimizeUnsafes) do_UnsafeRawOp(x); }
void Canonicalizer::do_UnsafePutRaw(UnsafePutRaw* x) { if (OptimizeUnsafes) do_UnsafeRawOp(x); }
void Canonicalizer::do_UnsafeGetObject(UnsafeGetObject* x) {}
void Canonicalizer::do_UnsafePutObject(UnsafePutObject* x) {}
