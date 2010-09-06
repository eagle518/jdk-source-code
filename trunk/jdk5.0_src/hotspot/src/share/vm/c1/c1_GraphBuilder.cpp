#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_GraphBuilder.cpp	1.216 04/05/04 13:51:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_GraphBuilder.cpp.incl"


// Implementation of BlockListBuilder

BlockBegin* BlockListBuilder::new_block_at(int bci, BlockBegin::Flag f) {
  BlockBegin* b = _bci2block->at(bci);
  if (b == NULL) {
    b = new BlockBegin(bci);
    _bci2block->at_put(bci, b);
  }
  // make sure all flags are accumulated
  b->set(f);
  return b;
}


void BlockListBuilder::set_leaders() {
  ciBytecodeStream s(method());
  while (s.next() >= 0) {
    switch (s.code()) {
      case Bytecodes::_ifeq        : // fall through
      case Bytecodes::_ifne        : // fall through
      case Bytecodes::_iflt        : // fall through
      case Bytecodes::_ifge        : // fall through
      case Bytecodes::_ifgt        : // fall through
      case Bytecodes::_ifle        : // fall through
      case Bytecodes::_if_icmpeq   : // fall through
      case Bytecodes::_if_icmpne   : // fall through
      case Bytecodes::_if_icmplt   : // fall through
      case Bytecodes::_if_icmpge   : // fall through
      case Bytecodes::_if_icmpgt   : // fall through
      case Bytecodes::_if_icmple   : // fall through
      case Bytecodes::_if_acmpeq   : // fall through
      case Bytecodes::_if_acmpne   : // fall through
      case Bytecodes::_ifnull      : // fall through
      case Bytecodes::_ifnonnull   :
        new_block_at(s.dest());
        new_block_at(s.next_bci());
        break;

      case Bytecodes::_goto        :
        new_block_at(s.dest());
        break;

      case Bytecodes::_jsr         :
        new_block_at(s.dest(), BlockBegin::subroutine_entry_flag);
        break;

      case Bytecodes::_tableswitch :
        { Bytecode_tableswitch *switch_ = Bytecode_tableswitch_at(s.bcp());
          // set block for each case
          int l = switch_->length();
          for (int i = 0; i < l; i++) new_block_at(s.bci() + switch_->dest_offset_at(i));
          // set default block
          new_block_at(s.bci() + switch_->default_offset());
        }
        break;
      
      case Bytecodes::_lookupswitch:
        { Bytecode_lookupswitch *switch_ = Bytecode_lookupswitch_at(s.bcp());
          // set block for each case
          int l = switch_->number_of_pairs();        
          for (int i = 0; i < l; i++) new_block_at(s.bci() + switch_->pair_at(i)->offset());
          // set default block
          new_block_at(s.bci() + switch_->default_offset());
        }
        break;

      case Bytecodes::_goto_w      :
        new_block_at(s.dest_w());
        break;

      case Bytecodes::_jsr_w       :
        new_block_at(s.dest_w(), BlockBegin::subroutine_entry_flag);
        break;

      default                      :
        // nothing to do
        break;
    }
  }
}


void BlockListBuilder::set_xhandler_entries() {
  XHandlers* list = xhandlers();
  const int n = list->number_of_handlers();
  for (int i = 0; i < n; i++) {
    XHandler* h = list->handler_at(i);
    h->set_entry(new_block_at(h->handler_bci(), BlockBegin::exception_entry_flag));
  }
}


BlockListBuilder::BlockListBuilder(IRScope* scope, int osr_bci, bool generate_std_entry) {
  _scope = scope;
  _bci2block = new BlockList(method()->code_size(), NULL);
  set_leaders();
  set_xhandler_entries();
  if (generate_std_entry) {
    _std_entry = new_block_at(0, BlockBegin::std_entry_flag);
  }
  _osr_entry = (osr_bci >= 0 ? new_block_at(osr_bci, BlockBegin::osr_entry_flag) : NULL);
}


// Implementation of GraphBuilder's ScopeData

GraphBuilder::ScopeData::ScopeData(ScopeData* parent, bool using_cha)
  : _parent(parent)
  , _using_cha(using_cha)
  , _bci2block(NULL)
  , _scope(NULL)
  , _has_handler(false)
  , _stream(NULL)
  , _work_list(NULL)
  , _parsing_jsr(false)
  , _jsr_xhandlers(NULL)
  , _caller_stack_size(-1)
  , _continuation(NULL)
  , _num_returns(0)
  , _cleanup_block(NULL)
  , _cleanup_return_prev(NULL)
  , _cleanup_state(NULL)
{
  if (parent != NULL) {
    _max_inline_size = (intx) ((float) NestedInliningSizeRatio * (float) parent->max_inline_size() / 100.0f);
  } else {
    _max_inline_size = MaxInlineSize;
  }
  if (_max_inline_size < MaxTrivialSize) {
    _max_inline_size = MaxTrivialSize;
  }
}


BlockBegin* GraphBuilder::ScopeData::block_at(int bci) {
  if (parsing_jsr()) {
    // It is necessary to clone all blocks associated with a
    // subroutine, including those for exception handlers in the scope
    // of the method containing the jsr (because those exception
    // handlers may contain ret instructions in some cases).
    BlockBegin* block = bci2block()->at(bci);
    if (block != NULL && block == parent()->bci2block()->at(bci)) {
      BlockBegin* new_block = new BlockBegin(block->bci());
      // Preserve certain flags for assertion checking
      if (block->is_set(BlockBegin::subroutine_entry_flag)) new_block->set(BlockBegin::subroutine_entry_flag);
      if (block->is_set(BlockBegin::exception_entry_flag))  new_block->set(BlockBegin::exception_entry_flag);
      bci2block()->at_put(bci, new_block);
      block = new_block;
    }
    return block;
  } else {
    return bci2block()->at(bci);
  }
}


XHandlers* GraphBuilder::ScopeData::xhandlers() const {
  if (_jsr_xhandlers == NULL) {
    assert(!parsing_jsr(), "");
    return scope()->xhandlers();
  }
  assert(parsing_jsr(), "");
  return _jsr_xhandlers;
}


void GraphBuilder::ScopeData::set_scope(IRScope* scope) {
  _scope = scope;
  bool parent_has_handler = false;
  if (parent() != NULL) {
    parent_has_handler = parent()->has_handler();
  }
  _has_handler = parent_has_handler || scope->xhandlers()->has_handlers();
}


void GraphBuilder::ScopeData::set_inline_cleanup_info(BlockBegin* block,
                                                      Instruction* return_prev,
                                                      ValueStack* return_state) {
  _cleanup_block       = block;
  _cleanup_return_prev = return_prev;
  _cleanup_state       = return_state;
}


void GraphBuilder::ScopeData::add_to_work_list(BlockBegin* block) {
  if (_work_list == NULL) {
    _work_list = new BlockList();
  }


  if (!block->is_set(BlockBegin::is_on_work_list_flag)) {
    // Do not start parsing the continuation block while in a
    // sub-scope
    if (parsing_jsr()) {
      if (block == jsr_continuation()) {
        return;
      }
    } else {
      if (block == continuation()) {
        return;
      }
    }
    block->set(BlockBegin::is_on_work_list_flag);
    _work_list->push(block);
  }
}


int GraphBuilder::ScopeData::caller_stack_size() const {
  ValueStack* state = scope()->caller_state();
  if (state == NULL) {
    return 0;
  }
  return state->stack_size();
}


BlockBegin* GraphBuilder::ScopeData::remove_from_work_list() {
  if (is_work_list_empty()) {
    return NULL;
  }
  return _work_list->pop();
}


bool GraphBuilder::ScopeData::is_work_list_empty() const {
  return (_work_list == NULL || _work_list->length() == 0);
}


void GraphBuilder::ScopeData::setup_jsr_xhandlers() {
  assert(parsing_jsr(), "");
  XHandlers* handlers = new XHandlers(scope()->method());
  const int n = handlers->number_of_handlers();
  for (int i = 0; i < n; i++) {
    XHandler* h = handlers->handler_at(i);
    h->set_entry(block_at(h->handler_bci()));
  }
  _jsr_xhandlers = handlers;
}


int GraphBuilder::ScopeData::num_returns() {
  if (parsing_jsr()) {
    return parent()->num_returns();
  }
  return _num_returns;
}


void GraphBuilder::ScopeData::incr_num_returns() {
  if (parsing_jsr()) {
    parent()->incr_num_returns();
  } else {
    ++_num_returns;
  }
}


// Implementation of GraphBuilder

#define BAILOUT(msg)               { bailout(msg); return;              }
#define BAILOUT_(msg, res)         { bailout(msg); return res;          }
#define INLINE_BAILOUT(msg)        { inline_bailout(msg); return false; }


void GraphBuilder::load_constant() {
  ciConstant con = stream()->get_constant();
  if (con.basic_type() == T_ILLEGAL) {
    BAILOUT("could not resolve a constant");
  } else {
    ValueType* t = illegalType;
    ValueStack* patch_state = NULL;
    switch (con.basic_type()) {
      case T_BOOLEAN: t = new IntConstant     (con.as_boolean()); break;
      case T_BYTE   : t = new IntConstant     (con.as_byte   ()); break;
      case T_CHAR   : t = new IntConstant     (con.as_char   ()); break;
      case T_SHORT  : t = new IntConstant     (con.as_short  ()); break;
      case T_INT    : t = new IntConstant     (con.as_int    ()); break;
      case T_LONG   : t = new LongConstant    (con.as_long   ()); break;
      case T_FLOAT  : t = new FloatConstant   (con.as_float  ()); break;
      case T_DOUBLE : t = new DoubleConstant  (con.as_double ()); break;
      case T_ARRAY  : t = new ArrayConstant   (con.as_object ()->as_array   ()); break;
      case T_OBJECT : 
       {
        ciObject* obj = con.as_object();
        if (obj->is_klass()) {
          ciKlass* klass = obj->as_klass();
          if (!klass->is_loaded() || PatchALot) {
            patch_state = state()->copy();
            t = new ObjectConstant(obj); 
          } else {
            t = new InstanceConstant(klass->java_mirror()); 
          }
        } else {
          t = new InstanceConstant(obj->as_instance()); 
        }
        break;
       }
      default       : ShouldNotReachHere();
    }
    Value x;
    if (patch_state != NULL) {
      // since it's being patched, it needs to be pinned otherwise it
      // might not have an Item associated with it we won't be able to
      // produce debug info.  Normally this is handled in the various
      // ValueStack::pin routines but constants are never pinned
      // because previously they never needed patching.
      patch_state->pin_stack_all(Instruction::PinUninitialized);
      x = new Constant(t, patch_state);
      x->pin();
    } else {
      x = new Constant(t);
    }
    push(t, append(x));
  }
}


void GraphBuilder::load_local(ValueType* type, int index) {
  Value x = NULL;
  if (EliminateLoads) x = state()->load_local(index);
  if (x != NULL) {
    if (PrintLoadElimination) tty->print_cr("load local %d eliminated @ %d", index, bci());
  } else {
    x = append(new LoadLocal(scope()->local_at(type, index, true)));
    state()->store_local(index, x);
    if (EliminateStores) state()->clear_store(index);
  }
  push(type, x);
}


void GraphBuilder::store_local(ValueType* type, int index) {
  Value x = pop(type);
  store_local(state(), x, type, index, false);
}


void GraphBuilder::store_local(ValueStack* state, Value x, ValueType* type, int index, bool is_inline_argument) {
  if (parsing_jsr()) {
    // We need to do additional tracking of the location of the return
    // address for jsrs since we don't handle arbitrary jsr/ret
    // constructs. Here we are figuring out in which circumstances we
    // need to bail out.
    if (x->type()->is_address()) {
      scope_data()->set_jsr_return_address_local(index);

      // Also check parent jsrs (if any) at this time to see whether
      // they are using this local. We don't handle skipping over a
      // ret.
      for (ScopeData* cur_scope_data = scope_data()->parent();
           cur_scope_data != NULL && cur_scope_data->parsing_jsr() && cur_scope_data->scope() == scope();
           cur_scope_data = cur_scope_data->parent()) {
        if (cur_scope_data->jsr_return_address_local() == index) {
          BAILOUT("subroutine overwrites return address from previous subroutine");
        }
      }
    } else if (index == scope_data()->jsr_return_address_local()) {
      scope_data()->set_jsr_return_address_local(-1);
    }
  }

  // Supports storing across scopes for full inliner.

  // astore may also be used to store the return address of a jsr, so
  // make sure to get the type right, otherwise local will be mistyped
  if (type->is_object() && x->type()->is_address()) {
    type = x->type();
  }

  // because of Java FPU semantics, float and doubles must be rounded
  // when storing them => kill them instead of keeping the result if
  // the CPU does not provide correctly rounded results in general
  if (RoundFloatsWithStore && type->is_float_kind()) {
    state->kill_local(index);
  } else {
    state->store_local(index, x);
  }
  StoreLocal* store = new StoreLocal(state->scope()->local_at(type, index, true), x, is_inline_argument);
  append(store);
  if (EliminateStores) {
    state->store_local(store, bci());
  }
  // make sure any outstanding loads are executed before
  state->pin_stack_locals(index);
  if (type->is_double_word()) state->pin_stack_locals(index + 1);
}


void GraphBuilder::load_indexed(BasicType type) {
  Value index = ipop();
  Value array = apop();
  Value length = NULL;
  if (CSEArrayLength) {
    length = append(new ArrayLength(array, lock_stack()));
  }
  push(as_ValueType(type), append(new LoadIndexed(array, index, length, type, lock_stack())));
}


void GraphBuilder::store_indexed(BasicType type) {
  Value value = pop(as_ValueType(type));
  Value index = ipop();
  Value array = apop();
  Value length = NULL;
  if (CSEArrayLength) {
    length = append(new ArrayLength(array, lock_stack()));
  }
  StoreIndexed* result = new StoreIndexed(array, index, length, type, value, lock_stack());
  vmap()->kill_array(value->type()); // invalidate all CSEs that are memory accesses
  state()->pin_stack_indexed(value->type());
  append(result);
}


void GraphBuilder::stack_op(Bytecodes::Code code) {
  switch (code) {
    case Bytecodes::_pop:
      { state()->raw_pop();
      }
      break;
    case Bytecodes::_pop2:
      { state()->raw_pop();
        state()->raw_pop();
      }
      break;
    case Bytecodes::_dup:
      { Value w = state()->raw_pop();
        state()->raw_push(w);
        state()->raw_push(w);
      }
      break;
    case Bytecodes::_dup_x1:
      { Value w1 = state()->raw_pop();
        Value w2 = state()->raw_pop();
        state()->raw_push(w1);
        state()->raw_push(w2);
        state()->raw_push(w1);
      }
      break;
    case Bytecodes::_dup_x2:
      { Value w1 = state()->raw_pop();
        Value w2 = state()->raw_pop();
        Value w3 = state()->raw_pop();
        state()->raw_push(w1);
        state()->raw_push(w3);
        state()->raw_push(w2);
        state()->raw_push(w1);
      }
      break;
    case Bytecodes::_dup2:
      { Value w1 = state()->raw_pop();
        Value w2 = state()->raw_pop();
        state()->raw_push(w2);
        state()->raw_push(w1);
        state()->raw_push(w2);
        state()->raw_push(w1);
      }
      break;
    case Bytecodes::_dup2_x1:
      { Value w1 = state()->raw_pop();
        Value w2 = state()->raw_pop();
        Value w3 = state()->raw_pop();
        state()->raw_push(w2);
        state()->raw_push(w1);
        state()->raw_push(w3);
        state()->raw_push(w2);
        state()->raw_push(w1);
      }
      break;
    case Bytecodes::_dup2_x2:
      { Value w1 = state()->raw_pop();
        Value w2 = state()->raw_pop();
        Value w3 = state()->raw_pop();
        Value w4 = state()->raw_pop();
        state()->raw_push(w2);
        state()->raw_push(w1);
        state()->raw_push(w4);
        state()->raw_push(w3);
        state()->raw_push(w2);
        state()->raw_push(w1);
      }
      break;
    case Bytecodes::_swap:
      { Value w1 = state()->raw_pop();
        Value w2 = state()->raw_pop();
        state()->raw_push(w1);
        state()->raw_push(w2);
      }
      break;
    default:
      ShouldNotReachHere();
      break;
  }
}


void GraphBuilder::arithmetic_op(ValueType* type, Bytecodes::Code code, ValueStack* stack) {
  Value y = pop(type);
  Value x = pop(type);
  // NOTE: strictfp can be queried from current method since we don't
  // inline methods with differing strictfp bits
  push(type, append(new ArithmeticOp(code, x, y, method()->is_strict(), stack)));
}


void GraphBuilder::negate_op(ValueType* type) {
  push(type, append(new NegateOp(pop(type))));
}


void GraphBuilder::shift_op(ValueType* type, Bytecodes::Code code) {
  Value s = ipop();
  Value x = pop(type);
  // try to simplify
  // Note: This code should go into the canonicalizer as soon as it can
  //       can handle canonicalized forms that contain more than one node.
  if (CanonicalizeNodes && code == Bytecodes::_iushr) {
    // pattern: x >>> s
    IntConstant* s1 = s->type()->as_IntConstant();
    if (s1 != NULL) {
      // pattern: x >>> s1, with s1 constant
      ShiftOp* l = x->as_ShiftOp();
      if (l != NULL && l->op() == Bytecodes::_ishl) {
        // pattern: (a << b) >>> s1
        IntConstant* s0 = l->y()->type()->as_IntConstant();
        if (s0 != NULL) {
          // pattern: (a << s0) >>> s1
          const int s0c = s0->value() & 0x1F; // only the low 5 bits are significant for shifts
          const int s1c = s1->value() & 0x1F; // only the low 5 bits are significant for shifts
          if (s0c == s1c) {
            if (s0c == 0) {
              // pattern: (a << 0) >>> 0 => simplify to: a
              ipush(l->x());
            } else {
              // pattern: (a << s0c) >>> s0c => simplify to: a & m, with m constant
              assert(0 < s0c && s0c < BitsPerInt, "adjust code below to handle corner cases");
              const int m = (1 << (BitsPerInt - s0c)) - 1;
              Value s = append(new Constant(new IntConstant(m)));
              ipush(append(new LogicOp(Bytecodes::_iand, l->x(), s)));
            }
            return;
          }
        }
      }
    }
  }
  // could not simplify
  push(type, append(new ShiftOp(code, x, s)));
}


void GraphBuilder::logic_op(ValueType* type, Bytecodes::Code code) {
  Value y = pop(type);
  Value x = pop(type);
  push(type, append(new LogicOp(code, x, y)));
}


void GraphBuilder::compare_op(ValueType* type, Bytecodes::Code code) {
  ValueStack* state_before = state()->copy();
  Value y = pop(type);
  Value x = pop(type);
  ipush(append(new CompareOp(code, x, y, state_before)));
}


void GraphBuilder::convert(Bytecodes::Code op, BasicType from, BasicType to) {
  push(as_ValueType(to), append(new Convert(op, pop(as_ValueType(from)), as_ValueType(to))));
}


void GraphBuilder::increment() {
  int index = stream()->get_index();
  int delta = stream()->is_wide() ? (signed short)Bytes::get_Java_u2(stream()->bcp() + 4) : (signed char)(stream()->bcp()[2]);
  load_local(intType, index);
  ipush(append(new Constant(new IntConstant(delta))));
  arithmetic_op(intType, Bytecodes::_iadd);
  store_local(intType, index);
}


void GraphBuilder::if_node(Value x, If::Condition cond, Value y, ValueStack* state_before) {
  BlockBegin* tsux = block_at(stream()->dest());
  BlockBegin* fsux = block_at(stream()->next_bci());
  bool is_bb = tsux->bci() < stream()->bci() || fsux->bci() < stream()->bci();
  append(new If(x, cond, false, y, tsux, fsux, state_before, is_bb));
}


void GraphBuilder::if_zero(ValueType* type, If::Condition cond) {
  Value y = append(new Constant(intZero));
  ValueStack* state_before = state()->copy();
  Value x = ipop();
  if_node(x, cond, y, state_before);
}


void GraphBuilder::if_null(ValueType* type, If::Condition cond) {
  Value y = append(new Constant(objectNull));
  ValueStack* state_before = state()->copy();
  Value x = apop();
  if_node(x, cond, y, state_before);
}


void GraphBuilder::if_same(ValueType* type, If::Condition cond) {
  ValueStack* state_before = state()->copy();
  Value y = pop(type);
  Value x = pop(type);
  if_node(x, cond, y, state_before);
}


void GraphBuilder::jsr(int dest) {
  // We only handle well-formed jsrs (those which are "block-structured").
  // If the bytecodes are strange (jumping out of a jsr block) then we
  // might end up trying to re-parse a block containing a jsr which
  // has already been activated. Watch for this case and bail out.
  for (ScopeData* cur_scope_data = scope_data();
       cur_scope_data != NULL && cur_scope_data->parsing_jsr() && cur_scope_data->scope() == scope();
       cur_scope_data = cur_scope_data->parent()) {
    if (cur_scope_data->jsr_entry_bci() == dest) {
      BAILOUT("too-complicated jsr/ret structure");
    }
  }

  push(addressType, append(new Constant(new AddressConstant(next_bci()))));
  if (!try_inline_jsr(dest)) {
    return; // bailed out while parsing and inlining subroutine
  }
}


void GraphBuilder::ret(int local_index) {
  if (!parsing_jsr()) BAILOUT("ret encountered while not parsing subroutine");
  
  if (local_index != scope_data()->jsr_return_address_local()) {
    BAILOUT("can not handle complicated jsr/ret constructs");
  }

  // Rets simply become (NON-SAFEPOINT) gotos to the jsr continuation
  append(new Goto(scope_data()->jsr_continuation(), false));
}


void GraphBuilder::table_switch() {
  Bytecode_tableswitch* switch_ = Bytecode_tableswitch_at(method()->code() + bci());
  const int l = switch_->length();
  if (CanonicalizeNodes && l == 1) {
    // total of 2 successors => use If instead of switch
    // Note: This code should go into the canonicalizer as soon as it can
    //       can handle canonicalized forms that contain more than one node.
    Value key = append(new Constant(new IntConstant(switch_->low_key())));
    BlockBegin* tsux = block_at(bci() + switch_->dest_offset_at(0));
    BlockBegin* fsux = block_at(bci() + switch_->default_offset());
    ValueStack* state_before = state();
    bool is_bb = tsux->bci() < bci() || fsux->bci() < bci();
    append(new If(ipop(), If::eql, true, key, tsux, fsux, state_before, is_bb));
  } else {
    // collect successors
    BlockList* sux = new BlockList(l + 1, NULL);
    int i;
    bool has_bb = false;
    for (i = 0; i < l; i++) {
      sux->at_put(i, block_at(bci() + switch_->dest_offset_at(i)));
      if (switch_->dest_offset_at(i) < 0) has_bb = true;
    }
    // add default successor
    sux->at_put(i, block_at(bci() + switch_->default_offset()));
    ValueStack* state_before = state();
    append(new TableSwitch(ipop(), sux, switch_->low_key(), state_before, has_bb));
  }
}


void GraphBuilder::lookup_switch() {
  Bytecode_lookupswitch* switch_ = Bytecode_lookupswitch_at(method()->code() + bci());
  const int l = switch_->number_of_pairs();
  if (CanonicalizeNodes && l == 1) {
    // total of 2 successors => use If instead of switch
    // Note: This code should go into the canonicalizer as soon as it can
    //       can handle canonicalized forms that contain more than one node.
    // simplify to If
    LookupswitchPair* pair = switch_->pair_at(0);
    Value key = append(new Constant(new IntConstant(pair->match())));
    BlockBegin* tsux = block_at(bci() + pair->offset());
    BlockBegin* fsux = block_at(bci() + switch_->default_offset());
    ValueStack* state_before = state();
    bool is_bb = tsux->bci() < bci() || fsux->bci() < bci();
    append(new If(ipop(), If::eql, true, key, tsux, fsux, state_before, is_bb));
  } else {
    // collect successors & keys
    BlockList* sux = new BlockList(l + 1, NULL);
    intArray* keys = new intArray(l, 0);
    int i;
    bool has_bb = false;
    for (i = 0; i < l; i++) {
      LookupswitchPair* pair = switch_->pair_at(i);
      if (pair->offset() < 0) has_bb = true;
      sux->at_put(i, block_at(bci() + pair->offset()));
      keys->at_put(i, pair->match());
    }
    // add default successor
    sux->at_put(i, block_at(bci() + switch_->default_offset()));
    ValueStack* state_before = state();
    append(new LookupSwitch(ipop(), sux, keys, state_before, has_bb));
  }
}


void GraphBuilder::method_return(Value x) {
  // Check to see whether we are inlining. If so, Return
  // instructions become Gotos to the continuation point.
  if (continuation() != NULL) {
    assert(!method()->is_synchronized(), "can not inline synchronized methods yet");
    state()->truncate_stack(caller_stack_size());
    if (x != NULL) {
      // Must pin return value so any LoadLocals from the arguments
      // are consumed before we return and potentially overwrite a
      // local with an argument for a successive inlined invocation.
      x->pin(Instruction::PinInlineReturnValue);
      state()->push(x->type(), x);
    }
    Goto* goto_callee = new Goto(continuation(), false);

    // See whether this is the first return; if so, store off some
    // of the state for later examination
    if (num_returns() == 0) {
      set_inline_cleanup_info(_block, _last, state());
    }

    append(goto_callee);
    incr_num_returns();
    return;
  }

  int index = method()->is_synchronized() ? state()->unlock() : -1;
  append(new Return(x, index));
}


void GraphBuilder::access_field(Bytecodes::Code code) {
  ciField* field = stream()->get_field();
  ciInstanceKlass* holder = field->holder();
  BasicType field_type = field->type()->basic_type();
  ValueType* type = as_ValueType(field_type);
  // call to will_link may cause class loading: do not call it if holder is not loaded
  const bool is_loaded = holder->is_loaded() && field->will_link(method()->holder(), code);
  const bool is_initialized = is_loaded && holder->is_initialized() && !PatchALot;

  Value obj = NULL;
  if (code == Bytecodes::_getstatic || code == Bytecodes::_putstatic) {
    // commoning of class constants should only occur if the class is
    // fully initialized and resolved in this constant pool.  The will_link test
    // above essentially checks if this class is resolved in this constant pool
    // so, the is_initialized flag should be suffiect.
    obj = new Constant(new ClassConstant(holder), is_initialized);
  }

  ValueStack* state_copy = NULL;
  if (!is_initialized) {
    // make sure all values on stack get spilled in case deoptimization happens
    state()->pin_stack_all(Instruction::PinUninitialized);
    if (EliminateStores) {
      // make sure all pending stores get executed so that the oops maps are correct
      state()->clear_stores();
    }
    // save state before instruction for debug info when deoptimization happens during patching
    state_copy = state()->copy();
  }

  const int offset = is_loaded ? field->offset() : -1;
  AccessField* result = NULL;
  switch (code) {
    case Bytecodes::_getstatic: {
      // make sure klass is initialized, bailout otherwise
      if (!LateBailout && !is_initialized) BAILOUT("klass not initialized for getstatic");
      // check for compile-time constants, i.e., initialized static final fields
      Instruction* constant = NULL;
      if (field->is_constant() && !PatchALot) {
        ciConstant field_val = field->constant_value();
        BasicType field_type = field_val.basic_type();
        switch (field_type) {
        case T_ARRAY:
        case T_OBJECT:
          if (field_val.as_object()->has_encoding()) {
            constant =  new Constant(as_ValueType(field_val));
          }
          break;
          
        default:
          constant = new Constant(as_ValueType(field_val));
        }
      }
      if (constant != NULL) {
        push(type, append(constant));
        state_copy = NULL; // Not a potential deoptimization point (see set_state_before logic below)
      } else {
        push(type, append(result = new LoadField(append(obj), offset, field, true, lock_stack(), is_loaded, is_initialized)));
      }
      break;
    }
    case Bytecodes::_putstatic:
      // make sure klass is initialized, bailout otherwise
      if (!LateBailout && !is_initialized) BAILOUT("klass not initialized for putstatic");
      { Value val = pop(type);
        append(result = new StoreField(append(obj), offset, field, val, true, lock_stack(), is_loaded, is_initialized));
        vmap()->kill_field(field);   // invalidate all CSEs that are memory accesses
        state()->pin_stack_fields(field); // make sure any outstanding loads are executed before (conservative)
      }
      break;
    case Bytecodes::_getfield :
      push(type, append(result = new LoadField(apop(), offset, field, false, lock_stack(), is_loaded, true)));
      break;
    case Bytecodes::_putfield :
      { Value val = pop(type);
        append(result = new StoreField(apop(), offset, field, val, false, lock_stack(), is_loaded, true));
        vmap()->kill_field(field);   // invalidate all CSEs that are memory accesses
        state()->pin_stack_fields(field); // make sure any outstanding loads are executed before (conservative)
      }
      break;
    default                   :
      ShouldNotReachHere();
      break;
  }
  if (state_copy != NULL) { 
    assert(result != NULL, "result must exist for non-initialized access");
    result->set_state_before(state_copy);
  }
}


void GraphBuilder::add_dependent(ciInstanceKlass* klass, ciMethod* method) { 
  compilation()->set_needs_debug_information(true);
  scope()->add_dependent(klass, method); 
}


void GraphBuilder::invoke(Bytecodes::Code code) {
  ciMethod* target = stream()->get_method();
  // we have to make sure the argument size (incl. the receiver)
  // is correct for compilation (the call would fail later during
  // linkage anyway) - was bug (gri 7/28/99)
  if (target->is_loaded() && target->is_static() != (code == Bytecodes::_invokestatic)) BAILOUT("will cause link error");
  ciInstanceKlass* klass = target->holder();

  // check if CHA possible: if so, change the code to invoke_special
  ciInstanceKlass* calling_klass = method()->holder();
  ciKlass* holder = stream()->get_declared_method_holder();
  ciInstanceKlass* callee_holder = ciEnv::get_instance_klass_for_declared_method_holder(holder);
  ciInstanceKlass* actual_recv = callee_holder; // currently not possible to be more precise
  // NEEDS_CLEANUP
  // I've added the target-is_loaded() test below but I don't really understand
  // how klass->is_loaded() can be true and yet target->is_loaded() is false.
  // this happened while running the JCK invokevirtual tests under doit.  TKR
  ciMethod* cha_monomorphic_target = NULL;
  if (UseCHA && DeoptC1 && klass->is_loaded() && target->is_loaded()) {
    if (code == Bytecodes::_invokevirtual && klass->is_initialized()) {
      cha_monomorphic_target = target->find_monomorphic_target(calling_klass, callee_holder, actual_recv);
    } else if (code == Bytecodes::_invokeinterface && callee_holder->is_loaded()) {
      // if there is only one implementor of this interface then we
      // can bind this invoke directly to that method.
      ciInstanceKlass* singleton = callee_holder->implementor();
      if (singleton) {
        cha_monomorphic_target = target->find_monomorphic_target(calling_klass, callee_holder, singleton);
        if (cha_monomorphic_target != NULL) {
          // If CHA is able to bind this invoke then update the class
          // to match that class, otherwise klass will refer to the
          // interface.
          klass = cha_monomorphic_target->holder();

          // It's possible because of separate class file evolution
          // for an invokeinteface site to have been generated for a
          // class which no longer implements that interface.  The
          // verifier doesn't guarantee this case so to optimize
          // invoke interface using CHA we have to insert a dynamic
          // check.  Grab the receiver off the expression stack and
          // perform a type check on it.  There may be enough type
          // information to know that the check is unnecessary.
          // Otherwise add a CheckCast and set it up to throw
          // IncomptibleClassChangeError if the check fails.
          int index = state()->stack_size() - (target->arg_size_no_receiver() + 1);
          Value receiver = state()->stack_at_inc(index);
          ciType* type = receiver->exact_type();
          if (type == NULL) type = receiver->declared_type();
          if (type == NULL || !type->is_loaded() || !type->is_subtype_of(callee_holder)) {
            CheckCast* c = new CheckCast(klass, receiver);
            c->set_incompatible_class_change_check();
            c->set_direct_compare(direct_compare(klass));
            append_split(c);
          }
        }
      }
    }
  }

  if (cha_monomorphic_target != NULL) {
    if (cha_monomorphic_target->is_abstract()) {
      // Do not optimize for abstract methods
      cha_monomorphic_target = NULL;
    }
  }
  bool dependency_recorded = false;
  if (cha_monomorphic_target != NULL) {
    if (!(target->is_final_method())) {         
      // If we inlined because CHA revealed only a single target method,
      // then we are dependent on that target method not getting overridden
      // by dynamic class loading.  Be sure to test the "static" receiver
      // dest_method here, as opposed to the actual receiver, which may
      // falsely lead us to believe that the receiver is final or private.
      add_dependent(actual_recv, cha_monomorphic_target);
      code = Bytecodes::_invokespecial;
      dependency_recorded = true;
    }
  }
  // check if we could do inlining
  if (!PatchALot && Inline && klass->is_loaded() && klass->is_initialized() 
      && target->will_link(klass, callee_holder, code)) {
    // callee is known => check if we have static binding
    assert(target->is_loaded(), "callee must be known");
    if (code == Bytecodes::_invokestatic
     || code == Bytecodes::_invokespecial
     || code == Bytecodes::_invokevirtual && target->is_final_method()
    ) {
      // static binding => check if callee is ok
      ciMethod* inline_target = (cha_monomorphic_target != NULL)
                                  ? cha_monomorphic_target
                                  : target;
      // When inlining methods as a result of CHA, the bytecodes we're
      // inlining may not be the same as what actually gets executed
      // if a deopt occurs.  In this case a throw bytecode may appear
      // to terminate execution of the inlinee and shorten the
      // lifetime of values on the expression stack in the caller.
      // That would make it impossible to reproduce the values for
      // deopt and could result in incorrect execution in the
      // interpreter.  So for any throw bytecodes which are the result
      // of inlining from CHA we must arrange for the expression stack
      // to be kept alive.  try_inline is passed a flag which
      // indicates that the current inline tree is part of a CHA call
      // and throw_op checks this flag to cause the preservation of
      // the expression stack.
      bool res = try_inline(inline_target, dependency_recorded || scope_data()->using_cha());
#ifndef PRODUCT
      // printing
      if (PrintInlining && !res) {
        // if it was successfully inlined, then it was already printed.
        print_inline_result(inline_target, res);
      }
#endif
      clear_inline_bailout();
      if (res) {
        // Register dependence if JVMTI has either breakpoint
        // setting or hotswapping of methods capabilities since they may
        // cause deoptimization.
        if (JvmtiExport::can_hotswap_or_post_breakpoint() && !dependency_recorded) {
          add_dependent(actual_recv, inline_target);
        }
        return;
      }
    }
  }
  // If we attempted an inline which did not succeed because of a
  // bailout during construction of the callee graph, the entire
  // compilation has to be aborted. This is fairly rare and currently
  // seems to only occur for jasm-generated classes which contain
  // jsr/ret pairs which are not associated with finally clauses and
  // do not have exception handlers in the containing method, and are
  // therefore not caught early enough to abort the inlining without
  // corrupting the graph. (We currently bail out with a non-empty
  // stack at a ret in these situations.)
  if (bailed_out()) return;
  // inlining not successful => standard invoke
  bool is_static = code == Bytecodes::_invokestatic;
  ValueType* result_type = as_ValueType(target->return_type());
  Values* args = state()->pop_arguments(target->arg_size_no_receiver());
  Value recv = is_static ? NULL : apop();
  bool is_loaded = target->is_loaded();
  int vtable_index = -1;
  // The UseInlineCaches only controls dispatch to invokevirtuals for
  // loaded classes which we weren't able to statically bind.
  if (!UseInlineCaches && is_loaded && code == Bytecodes::_invokevirtual && !target->is_final_method()) {
    vtable_index = target->vtable_index();
    // vtable_index == -1 implies that a method should be statically
    // bindable but the our existing logic to use CHA requires that
    // that klass is initialized before attempting lookup.  Even
    // without CHA we should able to statically bind this method.  For
    // now we leave this logic alone.
    assert(vtable_index != -1 || !klass->is_initialized(), "bad vtable index");
  }
  Invoke* result = new Invoke(code, result_type, recv, args, vtable_index,
                              is_loaded && target->is_final_method(),
                              is_loaded,
                              is_loaded && target->is_strict());
  // push result
  append_split(result);
  if (result_type != voidType) push(result_type, result);
}


void GraphBuilder::new_instance(int klass_index) {
  ciKlass* klass = stream()->get_klass();
  assert(klass->is_instance_klass(), "must be an instance klass");
  apush(append_split(new NewInstance(klass->as_instance_klass())));
}


void GraphBuilder::new_type_array() {
  apush(append_split(new NewTypeArray(ipop(), (BasicType)stream()->get_index())));
}


void GraphBuilder::new_object_array() {
  ciKlass* klass = stream()->get_klass();
  ValueStack* state_before = !klass->is_loaded() || PatchALot ? state()->copy() : NULL;
  NewArray* n = new NewObjectArray(klass, ipop());
  n->set_state_before(state_before); // for patching
  apush(append_split(n));
}


bool GraphBuilder::direct_compare(ciKlass* k) {
  if (k->is_loaded() && k->is_instance_klass() && !UseSlowPath) {
    ciInstanceKlass* ik = k->as_instance_klass();
    if (ik->is_final()) {
      return true;
    } else {
      if (DeoptC1 && UseCHA && !(ik->has_subklass() || ik->flags().is_interface())) {
        // test class is leaf class
        add_dependent(ik, NULL);
        return true;
      }
    }
  }
  return false;
}


void GraphBuilder::check_cast(int klass_index) {
  ciKlass* klass = stream()->get_klass();
  ValueStack* state_before = !klass->is_loaded() || PatchALot ? state()->copy() : NULL;
  CheckCast* c = new CheckCast(klass, apop());
  c->set_state_before(state_before); // for patching
  apush(append_split(c));
  c->set_direct_compare(direct_compare(klass));
}


void GraphBuilder::instance_of(int klass_index) {
  ciKlass* klass = stream()->get_klass();
  ValueStack* state_before = !klass->is_loaded() || PatchALot ? state()->copy() : NULL;
  InstanceOf* i = new InstanceOf(klass, apop());
  i->set_state_before(state_before); // for patching
  ipush(append_split(i));
  i->set_direct_compare(direct_compare(klass));
}


void GraphBuilder::monitorenter(Value x) {
  // save state before locking in case of deoptimization after a NullPointerException
  ValueStack* lock_stack_before = lock_stack();
  append_split(new MonitorEnter(x, state()->lock(scope(), x), lock_stack_before));
}


void GraphBuilder::monitorexit(Value x) {
  // Note: the comment below is only relevant for the case where we do
  // not deoptimize due to asynchronous exceptions (!(DeoptC1 &&
  // DeoptOnAsyncException), which is not used anymore)

  // Note: Potentially, the monitor state in an exception handler
  //       can be wrong due to wrong 'initialization' of the handler
  //       via a wrong asynchronous exception path. This can happen,
  //       if the exception handler range for asynchronous exceptions
  //       is too long (see also java bug 4327029, and comment in
  //       GraphBuilder::handle_exception()). This may cause 'under-
  //       flow' of the monitor stack => bailout instead.
  if (state()->locks_size() < 1) BAILOUT("monitor stack underflow");
  append_split(new MonitorExit(x, state()->unlock()));
}


void GraphBuilder::new_multi_array(int dimensions) {
  ciKlass* klass = stream()->get_klass();
  ValueStack* state_before = !klass->is_loaded() || PatchALot ? state()->copy() : NULL;
  if (state_before != NULL) {
    // make sure all values on stack get spilled in case deoptimization happens
    state()->pin_stack_all(Instruction::PinUninitialized);
  }

  Values* dims = new Values(dimensions, NULL);
  // fill in all dimensions
  int i = dimensions;
  while (i-- > 0) dims->at_put(i, ipop());
  // create array
  NewArray* n = new NewMultiArray(klass, dims);
  n->set_state_before(state_before); // for patching
  apush(append_split(n));
}


void GraphBuilder::throw_op() {
  // We require that the debug info for a Throw be the "state before"
  // the Throw (i.e., exception oop is still on TOS)
  ValueStack* state_before = state()->copy();
  Throw* t = new Throw(apop());
  t->set_state_before(state_before);
  t->set_flag(Instruction::KeepStateBeforeAliveFlag, scope_data()->using_cha());
  append(t);
}


Instruction* GraphBuilder::append_base(Instruction* instr) {
  Canonicalizer canon(instr, bci());
  Instruction* i1 = canon.canonical();
  Instruction* i2 = vmap()->find(i1);
  // The canonicalizer may return instructions which are already linked into
  // the instruction list and the only way to identify an instruction which
  // hasn't been linked is to see if it has a bci.
  if (i1 == i2 && i2->bci() == -1) {
    // i1 was not eliminated => append it
    assert(i2->next() == NULL, "shouldn't already be linked");
    _last = _last->set_next(i2, canon.bci());
    if (++_instruction_count >= InstructionCountCutoff
        && !bailed_out()) {
      // set the bailout state but complete normal processing.  We
      // might do a little more work before noticing the bailout so we
      // want processing to continue normally until it's noticed.
      bailout("Method and/or inlining is too large");
    }

#ifndef PRODUCT
    if (PrintIRDuringConstruction) {
      InstructionPrinter ip;
      ip.print_line(i2);
    }
#endif
    assert(_last == i2, "adjust code below");
    if (i2->as_StateSplit() != NULL && i2->as_BlockEnd() == NULL) {
      StateSplit* s = i2->as_StateSplit();
      assert(s != NULL, "s must exist");
      // Continue load elimination and CSE across certain intrinsics
      Intrinsic* intrinsic = s->as_Intrinsic();
      if (intrinsic == NULL || !intrinsic->preserves_state()) {
        if (s->as_Invoke() == NULL || !EliminateLoadsAcrossCalls) {
          // if EliminateLoadsAcrossCalls is true and the StateSplit is an invoke,
          // do not clear all locals
          state()->clear_locals(); // for now, hopefully we need this only for calls eventually
        }
        vmap()->kill_all();      // for now, hopefully we need this only for calls eventually
      }
      state()->pin_stack_for_state_split();
      s->set_state(state()->copy());
    }
    // set up exception handlers for this instruction if necessary
    BlockEnd* be = i2->as_BlockEnd();
    if (i2->can_trap() ||
        (be != NULL && be->is_safepoint())) {
      i2->set_exception_scope(exception_scope()->copy());
    }
  }
  return i2;
}


Instruction* GraphBuilder::append(Instruction* instr) {
  assert(instr->as_StateSplit() == NULL || instr->as_BlockEnd() != NULL, "wrong append used");
  return append_base(instr);
}


Instruction* GraphBuilder::append_split(StateSplit* instr) {
  return append_base(instr);
}


void GraphBuilder::bailout(const char* msg) {
  assert(msg != NULL, "bailout msg must exist");
  _bailout_msg = msg;
  if (PrintBailouts) tty->print_cr(" Bailout reason: %s", msg);
}


void GraphBuilder::handle_exception(bool is_async) {
  bool cleared_stores = false;

  exception_scope()->clear();
  int cur_bci = bci();
  int num_state_pops_needed = 0;
  ScopeData* cur_scope_data = scope_data();
  ValueStack* s = state()->copy();
  bool is_top_scope = true;
  while (cur_scope_data != NULL) {
    // join with all potential exception handlers
    XHandlers* list = cur_scope_data->xhandlers();
    const int n = list->number_of_handlers();
    for (int i = 0; i < n; i++) {
      XHandler* h = list->handler_at(i);
      if (h->covers(cur_bci)) {
        compilation()->set_has_exception_handlers(true);

        if (EliminateStores) {
          if (!cleared_stores) {
            // Conservatively assume that the locals may be referenced by
            // GC, etc. and therefore all stores beforehand need to take place
            state()->clear_stores();
            cleared_stores = true;
          }
        }

        BlockBegin* entry = h->entry();
        if (cur_scope_data->parsing_jsr()) {
          // Exception handlers are inherited from parent scope; must
          // grab duplicated block from our scope
          entry = cur_scope_data->block_at(entry->bci());
        }

        assert(entry == cur_scope_data->block_at(h->handler_bci()), "blocks must correspond");
        // h is a potential exception handler => join it

        // Note: the comment below is only relevant for the case where
        // we do not deoptimize due to asynchronous exceptions
        // (!(DeoptC1 && DeoptOnAsyncException), which is not used
        // anymore)

        // Note: The code below is necessary since methods may have in-
        //       correct exception handler ranges for async. exceptions
        //       (this is the case for the current javac and some 3rd
        //       party javac's). In particular, the exception handler
        //       ranges may be too long, in which case an asynchronous
        //       exception will cause a control flow to the exception
        //       handler with the wrong monitor state (i.e., monitor
        //       already unlocked). If the graph builder tries to join
        //       the corresponding exception handler, it will fail with
        //       an assertion since the monitor stacks do not correspond,
        //       unless we do some extra checking beforehand.
        //       
        //       This problem showed up with java.lang.ref.Finalizer::add
        //       in J2SE 1.3 rc2 (see also bug 4324989: we forgot to add
        //       consider asynchronous exception handlers - after fixing
        //       that problem we encountered the javac problem 4327029).
        if (entry->state() != NULL) {
          if (state()->locks_size() != entry->state()->locks_size()) {
            // trying to join the exception handler would fail with
            // an assertion failure since the monitor stacks do not
            // correspond
            if (is_async) {
              // assume exception handler range is wrong for
              // asynchronous exception => simply ignore it
              return;
            } else {
              // the monitor states may not match because of
              // an asynchronous exception path that set up
              // the handler monitor state wrongly before =>
              // bailout since we don't know what is correct
              BAILOUT("illegal monitor state");
            }
          }
        }

        // Note: Since locals are all NULL here & the expression
        //       stack is empty besides the dummy exception object
        //       we don't have to do a full & time consuming join
        //       if the exception handler was visited before
        //       => do it always only in debug mode
#ifndef ASSERT
        if (!entry->is_set(BlockBegin::was_visited_flag)) {
#endif // ASSERT
          // empty expression stack after exception
          s->truncate_stack(cur_scope_data->caller_stack_size());
          s->apush(new Constant(objectNull)); // but the exception oop is on the stack

          // Note: Usually this join must work. However, very
          // complicated jsr-ret structures where we don't ret from
          // the subroutine can cause the objects on the monitor
          // stacks to not match because blocks can be parsed twice.
          // The only test case we've seen so far which exhibits this
          // problem is caught by the infinite recursion test in
          // GraphBuilder::jsr() if the join doesn't work.
          if (!entry->try_join(s->copy())) {
            BAILOUT("error while joining with exception handler, prob. due to complicated jsr/rets");
          }
          // fill in exception handler subgraph lazily
          // Note: exception handlers for scopes other than the top
          // scope have already been added to the work list (see
          // management of the exception scope below)
          cur_scope_data->add_to_work_list(entry);
#ifndef ASSERT
        }
#endif // ASSERT

        // add h to the list of exception handlers of this block
        _block->add_exception_handler(entry);

        if (is_top_scope) {
          // Add h to the list of exception handlers covering this bci.
          // Note: the management of exception scopes requires that we
          // call handle_exception before entering an inlined scope
          exception_scope()->add_handler(h);
        }

        // stop when reaching catchall
        if (h->catch_type() == 0) return;
      }
    }

    // Set up iteration for next time.
    // If parsing a jsr, do not grab exception handlers from the
    // parent scopes for this method (already got them, and they
    // needed to be cloned)

    is_top_scope = false;
    if (cur_scope_data->parsing_jsr()) {
      IRScope* tmp_scope = cur_scope_data->scope();
      while (cur_scope_data->parent() != NULL &&
             cur_scope_data->parent()->scope() == tmp_scope) {
        cur_scope_data = cur_scope_data->parent();
      }
    }
    if (cur_scope_data != NULL) {
      if (cur_scope_data->parent() != NULL) {
        s = s->pop_scope(false, cur_bci);
      }
      cur_bci = cur_scope_data->scope()->caller_bci();
      cur_scope_data = cur_scope_data->parent();
    }
  }
}


BlockEnd* GraphBuilder::connect_to_end(BlockBegin* beg) {
  // setup iteration
  vmap()->kill_all();
  _block = beg;
  _state = beg->state()->copy();
  _last  = beg;
  return iterate_bytecodes_for_block(beg->bci());
}


BlockEnd* GraphBuilder::iterate_bytecodes_for_block(int bci) {
#ifndef PRODUCT
  if (PrintIRDuringConstruction) {
    tty->cr();
    InstructionPrinter ip;
    ip.print_instr(_block); tty->cr();
    ip.print_stack(_block->state()); tty->cr();
    ip.print_inline_level(_block);
    ip.print_head();
  }
#endif
  ciBytecodeStream s(method());
  s.set_start(bci);
  int prev_bci = bci;
  scope_data()->set_stream(&s);
  // iterate
  Bytecodes::Code code = Bytecodes::_illegal;
  bool prev_is_monitorenter = false;
  while (!bailed_out() && last()->as_BlockEnd() == NULL && (code = stream()->next()) >= 0 && (block_at(s.bci()) == NULL || block_at(s.bci()) == block())) {
    // handle potential exceptions thrown by current bytecode
    // note 1: exceptions must be handled before the bytecode as
    //         the (locking) state before the bytecode is relevant
    //         in the exception handler (e.g. for monitorenter)
    // note 2: if the previous bytecode was a monitorenter bytecode,
    //         assume the current bytecode throws an asynchronous
    //         exception to get at least one control flow path to
    //         the handler for asynchronous exceptions (this will
    //         hopefully also be the first control flow path to
    //         that handler which will 'initialize' it correctly;
    //         see also javac problem mentioned in handle_exception())
    if (has_handler() && (prev_is_monitorenter || can_trap(code))) {
      handle_exception(prev_is_monitorenter || is_async(code));
      if (bailed_out()) return NULL;
    }
    prev_is_monitorenter = false;

    // Check for active jsr during OSR compilation
    if (compilation()->is_osr_compile()
        && scope()->is_top_scope()
        && parsing_jsr()
        && s.bci() == compilation()->osr_bci()) {
      bailout("OSR not supported while a jsr is active");
    }

    // handle bytecode
    switch (code) {
      case Bytecodes::_nop            : /* nothing to do */ break;
      case Bytecodes::_aconst_null    : apush(append(new Constant(objectNull            ))); break;
      case Bytecodes::_iconst_m1      : ipush(append(new Constant(new IntConstant   (-1)))); break;
      case Bytecodes::_iconst_0       : ipush(append(new Constant(intZero               ))); break;
      case Bytecodes::_iconst_1       : ipush(append(new Constant(intOne                ))); break;
      case Bytecodes::_iconst_2       : ipush(append(new Constant(new IntConstant   ( 2)))); break;
      case Bytecodes::_iconst_3       : ipush(append(new Constant(new IntConstant   ( 3)))); break;
      case Bytecodes::_iconst_4       : ipush(append(new Constant(new IntConstant   ( 4)))); break;
      case Bytecodes::_iconst_5       : ipush(append(new Constant(new IntConstant   ( 5)))); break;
      case Bytecodes::_lconst_0       : lpush(append(new Constant(new LongConstant  ( 0)))); break;
      case Bytecodes::_lconst_1       : lpush(append(new Constant(new LongConstant  ( 1)))); break;
      case Bytecodes::_fconst_0       : fpush(append(new Constant(new FloatConstant ( 0)))); break;
      case Bytecodes::_fconst_1       : fpush(append(new Constant(new FloatConstant ( 1)))); break;
      case Bytecodes::_fconst_2       : fpush(append(new Constant(new FloatConstant ( 2)))); break;
      case Bytecodes::_dconst_0       : dpush(append(new Constant(new DoubleConstant( 0)))); break;
      case Bytecodes::_dconst_1       : dpush(append(new Constant(new DoubleConstant( 1)))); break;
      case Bytecodes::_bipush         : ipush(append(new Constant(new IntConstant(((signed char*)s.bcp())[1])))); break;
      case Bytecodes::_sipush         : ipush(append(new Constant(new IntConstant((short)Bytes::get_Java_u2(s.bcp()+1))))); break;
      case Bytecodes::_ldc            : // fall through
      case Bytecodes::_ldc_w          : // fall through
      case Bytecodes::_ldc2_w         : load_constant(); break;
      case Bytecodes::_iload          : load_local(intType     , s.get_index()); break;
      case Bytecodes::_lload          : load_local(longType    , s.get_index()); break;
      case Bytecodes::_fload          : load_local(floatType   , s.get_index()); break;
      case Bytecodes::_dload          : load_local(doubleType  , s.get_index()); break;
      case Bytecodes::_aload          : load_local(instanceType, s.get_index()); break;
      case Bytecodes::_iload_0        : load_local(intType   , 0); break;
      case Bytecodes::_iload_1        : load_local(intType   , 1); break;
      case Bytecodes::_iload_2        : load_local(intType   , 2); break;
      case Bytecodes::_iload_3        : load_local(intType   , 3); break;
      case Bytecodes::_lload_0        : load_local(longType  , 0); break;
      case Bytecodes::_lload_1        : load_local(longType  , 1); break;
      case Bytecodes::_lload_2        : load_local(longType  , 2); break;
      case Bytecodes::_lload_3        : load_local(longType  , 3); break;
      case Bytecodes::_fload_0        : load_local(floatType , 0); break;
      case Bytecodes::_fload_1        : load_local(floatType , 1); break;
      case Bytecodes::_fload_2        : load_local(floatType , 2); break;
      case Bytecodes::_fload_3        : load_local(floatType , 3); break;
      case Bytecodes::_dload_0        : load_local(doubleType, 0); break;
      case Bytecodes::_dload_1        : load_local(doubleType, 1); break;
      case Bytecodes::_dload_2        : load_local(doubleType, 2); break;
      case Bytecodes::_dload_3        : load_local(doubleType, 3); break;
      case Bytecodes::_aload_0        : load_local(objectType, 0); break;
      case Bytecodes::_aload_1        : load_local(objectType, 1); break;
      case Bytecodes::_aload_2        : load_local(objectType, 2); break;
      case Bytecodes::_aload_3        : load_local(objectType, 3); break;
      case Bytecodes::_iaload         : load_indexed(T_INT   ); break;
      case Bytecodes::_laload         : load_indexed(T_LONG  ); break;
      case Bytecodes::_faload         : load_indexed(T_FLOAT ); break;
      case Bytecodes::_daload         : load_indexed(T_DOUBLE); break;
      case Bytecodes::_aaload         : load_indexed(T_OBJECT); break;
      case Bytecodes::_baload         : load_indexed(T_BYTE  ); break;
      case Bytecodes::_caload         : load_indexed(T_CHAR  ); break;
      case Bytecodes::_saload         : load_indexed(T_SHORT ); break;
      case Bytecodes::_istore         : store_local(intType   , s.get_index()); break;
      case Bytecodes::_lstore         : store_local(longType  , s.get_index()); break;
      case Bytecodes::_fstore         : store_local(floatType , s.get_index()); break;
      case Bytecodes::_dstore         : store_local(doubleType, s.get_index()); break;
      case Bytecodes::_astore         : store_local(objectType, s.get_index()); break;
      case Bytecodes::_istore_0       : store_local(intType   , 0); break;
      case Bytecodes::_istore_1       : store_local(intType   , 1); break;
      case Bytecodes::_istore_2       : store_local(intType   , 2); break;
      case Bytecodes::_istore_3       : store_local(intType   , 3); break;
      case Bytecodes::_lstore_0       : store_local(longType  , 0); break;
      case Bytecodes::_lstore_1       : store_local(longType  , 1); break;
      case Bytecodes::_lstore_2       : store_local(longType  , 2); break;
      case Bytecodes::_lstore_3       : store_local(longType  , 3); break;
      case Bytecodes::_fstore_0       : store_local(floatType , 0); break;
      case Bytecodes::_fstore_1       : store_local(floatType , 1); break;
      case Bytecodes::_fstore_2       : store_local(floatType , 2); break;
      case Bytecodes::_fstore_3       : store_local(floatType , 3); break;
      case Bytecodes::_dstore_0       : store_local(doubleType, 0); break;
      case Bytecodes::_dstore_1       : store_local(doubleType, 1); break;
      case Bytecodes::_dstore_2       : store_local(doubleType, 2); break;
      case Bytecodes::_dstore_3       : store_local(doubleType, 3); break;
      case Bytecodes::_astore_0       : store_local(objectType, 0); break;
      case Bytecodes::_astore_1       : store_local(objectType, 1); break;
      case Bytecodes::_astore_2       : store_local(objectType, 2); break;
      case Bytecodes::_astore_3       : store_local(objectType, 3); break;
      case Bytecodes::_iastore        : store_indexed(T_INT   ); break;
      case Bytecodes::_lastore        : store_indexed(T_LONG  ); break;
      case Bytecodes::_fastore        : store_indexed(T_FLOAT ); break;
      case Bytecodes::_dastore        : store_indexed(T_DOUBLE); break;
      case Bytecodes::_aastore        : store_indexed(T_OBJECT); break;
      case Bytecodes::_bastore        : store_indexed(T_BYTE  ); break;
      case Bytecodes::_castore        : store_indexed(T_CHAR  ); break;
      case Bytecodes::_sastore        : store_indexed(T_SHORT ); break;
      case Bytecodes::_pop            : // fall through
      case Bytecodes::_pop2           : // fall through
      case Bytecodes::_dup            : // fall through
      case Bytecodes::_dup_x1         : // fall through
      case Bytecodes::_dup_x2         : // fall through
      case Bytecodes::_dup2           : // fall through
      case Bytecodes::_dup2_x1        : // fall through
      case Bytecodes::_dup2_x2        : // fall through
      case Bytecodes::_swap           : stack_op(code); break;
      case Bytecodes::_iadd           : arithmetic_op(intType   , code); break;
      case Bytecodes::_ladd           : arithmetic_op(longType  , code); break;
      case Bytecodes::_fadd           : arithmetic_op(floatType , code); break;
      case Bytecodes::_dadd           : arithmetic_op(doubleType, code); break;
      case Bytecodes::_isub           : arithmetic_op(intType   , code); break;
      case Bytecodes::_lsub           : arithmetic_op(longType  , code); break;
      case Bytecodes::_fsub           : arithmetic_op(floatType , code); break;
      case Bytecodes::_dsub           : arithmetic_op(doubleType, code); break;
      case Bytecodes::_imul           : arithmetic_op(intType   , code); break;
      case Bytecodes::_lmul           : arithmetic_op(longType  , code); break;
      case Bytecodes::_fmul           : arithmetic_op(floatType , code); break;
      case Bytecodes::_dmul           : arithmetic_op(doubleType, code); break;
      case Bytecodes::_idiv           : arithmetic_op(intType   , code, lock_stack()); break;
      case Bytecodes::_ldiv           : arithmetic_op(longType  , code, lock_stack()); break;
      case Bytecodes::_fdiv           : arithmetic_op(floatType , code); break;
      case Bytecodes::_ddiv           : arithmetic_op(doubleType, code); break;
      case Bytecodes::_irem           : arithmetic_op(intType   , code, lock_stack()); break;
      case Bytecodes::_lrem           : arithmetic_op(longType  , code, lock_stack()); break;
      case Bytecodes::_frem           : arithmetic_op(floatType , code); break;
      case Bytecodes::_drem           : arithmetic_op(doubleType, code); break;
      case Bytecodes::_ineg           : negate_op(intType   ); break;
      case Bytecodes::_lneg           : negate_op(longType  ); break;
      case Bytecodes::_fneg           : negate_op(floatType ); break;
      case Bytecodes::_dneg           : negate_op(doubleType); break;
      case Bytecodes::_ishl           : shift_op(intType , code); break;
      case Bytecodes::_lshl           : shift_op(longType, code); break;
      case Bytecodes::_ishr           : shift_op(intType , code); break;
      case Bytecodes::_lshr           : shift_op(longType, code); break;
      case Bytecodes::_iushr          : shift_op(intType , code); break;
      case Bytecodes::_lushr          : shift_op(longType, code); break;
      case Bytecodes::_iand           : logic_op(intType , code); break;
      case Bytecodes::_land           : logic_op(longType, code); break;
      case Bytecodes::_ior            : logic_op(intType , code); break;
      case Bytecodes::_lor            : logic_op(longType, code); break;
      case Bytecodes::_ixor           : logic_op(intType , code); break;
      case Bytecodes::_lxor           : logic_op(longType, code); break;
      case Bytecodes::_iinc           : increment(); break;
      case Bytecodes::_i2l            : convert(code, T_INT   , T_LONG  ); break;
      case Bytecodes::_i2f            : convert(code, T_INT   , T_FLOAT ); break;
      case Bytecodes::_i2d            : convert(code, T_INT   , T_DOUBLE); break;
      case Bytecodes::_l2i            : convert(code, T_LONG  , T_INT   ); break;
      case Bytecodes::_l2f            : convert(code, T_LONG  , T_FLOAT ); break;
      case Bytecodes::_l2d            : convert(code, T_LONG  , T_DOUBLE); break;
      case Bytecodes::_f2i            : convert(code, T_FLOAT , T_INT   ); break;
      case Bytecodes::_f2l            : convert(code, T_FLOAT , T_LONG  ); break;
      case Bytecodes::_f2d            : convert(code, T_FLOAT , T_DOUBLE); break;
      case Bytecodes::_d2i            : convert(code, T_DOUBLE, T_INT   ); break;
      case Bytecodes::_d2l            : convert(code, T_DOUBLE, T_LONG  ); break;
      case Bytecodes::_d2f            : convert(code, T_DOUBLE, T_FLOAT ); break;
      case Bytecodes::_i2b            : convert(code, T_INT   , T_BYTE  ); break;
      case Bytecodes::_i2c            : convert(code, T_INT   , T_CHAR  ); break;
      case Bytecodes::_i2s            : convert(code, T_INT   , T_SHORT ); break;
      case Bytecodes::_lcmp           : compare_op(longType  , code); break;
      case Bytecodes::_fcmpl          : compare_op(floatType , code); break;
      case Bytecodes::_fcmpg          : compare_op(floatType , code); break;
      case Bytecodes::_dcmpl          : compare_op(doubleType, code); break;
      case Bytecodes::_dcmpg          : compare_op(doubleType, code); break;
      case Bytecodes::_ifeq           : if_zero(intType   , If::eql); break;
      case Bytecodes::_ifne           : if_zero(intType   , If::neq); break;
      case Bytecodes::_iflt           : if_zero(intType   , If::lss); break;
      case Bytecodes::_ifge           : if_zero(intType   , If::geq); break;
      case Bytecodes::_ifgt           : if_zero(intType   , If::gtr); break;
      case Bytecodes::_ifle           : if_zero(intType   , If::leq); break;
      case Bytecodes::_if_icmpeq      : if_same(intType   , If::eql); break;
      case Bytecodes::_if_icmpne      : if_same(intType   , If::neq); break;
      case Bytecodes::_if_icmplt      : if_same(intType   , If::lss); break;
      case Bytecodes::_if_icmpge      : if_same(intType   , If::geq); break;
      case Bytecodes::_if_icmpgt      : if_same(intType   , If::gtr); break;
      case Bytecodes::_if_icmple      : if_same(intType   , If::leq); break;
      case Bytecodes::_if_acmpeq      : if_same(objectType, If::eql); break;
      case Bytecodes::_if_acmpne      : if_same(objectType, If::neq); break;
      case Bytecodes::_goto           : append(new Goto(block_at(s.dest()), s.dest() <= s.bci())); break;
      case Bytecodes::_jsr            : jsr(s.dest()); break;
      case Bytecodes::_ret            : ret(s.get_index()); break;
      case Bytecodes::_tableswitch    : table_switch(); break;
      case Bytecodes::_lookupswitch   : lookup_switch(); break;
      case Bytecodes::_ireturn        : method_return(ipop()); break;
      case Bytecodes::_lreturn        : method_return(lpop()); break;
      case Bytecodes::_freturn        : method_return(fpop()); break;
      case Bytecodes::_dreturn        : method_return(dpop()); break;
      case Bytecodes::_areturn        : method_return(apop()); break;
      case Bytecodes::_return         : method_return(NULL  ); break;
      case Bytecodes::_getstatic      : // fall through
      case Bytecodes::_putstatic      : // fall through
      case Bytecodes::_getfield       : // fall through
      case Bytecodes::_putfield       : access_field(code); break;
      case Bytecodes::_invokevirtual  : // fall through
      case Bytecodes::_invokespecial  : // fall through
      case Bytecodes::_invokestatic   : // fall through
      case Bytecodes::_invokeinterface: invoke(code); break;
      case Bytecodes::_xxxunusedxxx   : ShouldNotReachHere(); break;
      case Bytecodes::_new            : new_instance(s.get_index_big()); break;
      case Bytecodes::_newarray       : new_type_array(); break;
      case Bytecodes::_anewarray      : new_object_array(); break;
      case Bytecodes::_arraylength    : ipush(append(new ArrayLength(apop(), lock_stack()))); break;
      case Bytecodes::_athrow         : throw_op(); break;
      case Bytecodes::_checkcast      : check_cast(s.get_index_big()); break;
      case Bytecodes::_instanceof     : instance_of(s.get_index_big()); break;
      // Note: we do not have special handling for the monitorenter bytecode if DeoptC1 && DeoptOnAsyncException
      case Bytecodes::_monitorenter   : monitorenter(apop()); prev_is_monitorenter = !(DeoptC1 && DeoptOnAsyncException); break;
      case Bytecodes::_monitorexit    : monitorexit (apop()); break;
      case Bytecodes::_wide           : ShouldNotReachHere(); break;
      case Bytecodes::_multianewarray : new_multi_array(s.bcp()[3]); break;
      case Bytecodes::_ifnull         : if_null(objectType, If::eql); break;
      case Bytecodes::_ifnonnull      : if_null(objectType, If::neq); break;
      case Bytecodes::_goto_w         : append(new Goto(block_at(s.dest_w()), s.dest_w() <= s.bci())); break;
      case Bytecodes::_jsr_w          : jsr(s.dest_w()); break;
      case Bytecodes::_breakpoint     : BAILOUT_("concurrent setting of breakpoint", NULL);
      default                         : ShouldNotReachHere(); break;
    }
    // save current bci to setup Goto at the end
    prev_bci = s.bci();
  }
  if (bailed_out()) return NULL;

  // if there are any, check if last instruction is a BlockEnd instruction
  BlockEnd* end = last()->as_BlockEnd();
  if (end == NULL) {
    // all blocks must end with a BlockEnd instruction => add a Goto
    end = new Goto(block_at(s.bci()), false);
    _last = _last->set_next(end, prev_bci);
  }
  assert(end == last()->as_BlockEnd(), "inconsistency");

  // if the method terminates, we don't need the stack anymore
  if (end->as_Return() != NULL) {
    state()->clear_stack();
    if (EliminateStores) {
      state()->eliminate_all_scope_stores(s.bci());
    }
  } else if (end->as_Throw() != NULL) {
    // May have exception handler in caller scopes
    state()->truncate_stack(scope()->lock_stack_size());
    if (EliminateStores) {
      state()->clear_stores();
    }
  }

  // here all expression stack values must be pinned
  // note: this only involves values that are inputs
  //       to phi nodes of other blocks - we must be
  //       sure we compute them before we compute the
  //       last expression of the block, usually a
  //       compare instruction

  // We can do better than to pin all of the values on the stack in
  // some situations -- specifically, if we will fall out of an
  // inlined scope and resume parsing in the caller. For these
  // situations we leave pinning of the stack up to the inliner.
  if (state() != inline_cleanup_state()) {
    state()->pin_stack_all(Instruction::PinEndOfBlock);
  }

  // connect to begin & set state
  // NOTE that inlining may have changed the block we are parsing
  block()->set_end(end);
  end->set_state(state());
  // propagate state
  for (int i = end->number_of_sux() - 1; i >= 0; i--) {
    // be careful, bailout if bytecodes are strange
    if (!end->sux_at(i)->try_join(state())) BAILOUT_("block join failed", NULL);
    scope_data()->add_to_work_list(end->sux_at(i));
  }

  // done
  return end;
}


void GraphBuilder::iterate_all_blocks(bool start_in_current_block_for_inlining) {
  do {
    if (start_in_current_block_for_inlining && !bailed_out()) {
      iterate_bytecodes_for_block(0);
      start_in_current_block_for_inlining = false;
    } else {
      BlockBegin* b;
      while ((b = scope_data()->remove_from_work_list()) != NULL) {
        if (!b->is_set(BlockBegin::was_visited_flag)) {
          b->set(BlockBegin::was_visited_flag);
          connect_to_end(b);
        }
      }
    }
  } while (!bailed_out() && !scope_data()->is_work_list_empty());
}


bool GraphBuilder::_is_initialized = false;
bool GraphBuilder::_can_trap      [Bytecodes::number_of_java_codes];
bool GraphBuilder::_is_async[Bytecodes::number_of_java_codes];

void GraphBuilder::initialize() {
  // make sure initialization happens only once (need a
  // lock here, if we allow the compiler to be re-entrant)
  if (is_initialized()) return;
  _is_initialized = true;

  // the following bytecodes are assumed to potentially
  // throw exceptions in compiled code - note that e.g.
  // monitorexit & the return bytecodes do not throw
  // exceptions since monitor pairing proved that they
  // succeed (if monitor pairing succeeded)
  Bytecodes::Code can_trap_list[] =
    { Bytecodes::_ldc
    , Bytecodes::_ldc_w
    , Bytecodes::_ldc2_w
    , Bytecodes::_iaload
    , Bytecodes::_laload
    , Bytecodes::_faload
    , Bytecodes::_daload
    , Bytecodes::_aaload
    , Bytecodes::_baload
    , Bytecodes::_caload
    , Bytecodes::_saload
    , Bytecodes::_iastore
    , Bytecodes::_lastore
    , Bytecodes::_fastore
    , Bytecodes::_dastore
    , Bytecodes::_aastore
    , Bytecodes::_bastore
    , Bytecodes::_castore
    , Bytecodes::_sastore
    , Bytecodes::_idiv
    , Bytecodes::_ldiv
    , Bytecodes::_irem
    , Bytecodes::_lrem
    , Bytecodes::_getstatic
    , Bytecodes::_putstatic
    , Bytecodes::_getfield
    , Bytecodes::_putfield
    , Bytecodes::_invokevirtual
    , Bytecodes::_invokespecial
    , Bytecodes::_invokestatic
    , Bytecodes::_invokeinterface
    , Bytecodes::_new
    , Bytecodes::_newarray
    , Bytecodes::_anewarray
    , Bytecodes::_arraylength
    , Bytecodes::_athrow
    , Bytecodes::_checkcast
    , Bytecodes::_instanceof
    , Bytecodes::_monitorenter
    , Bytecodes::_multianewarray
    };

  // the following bytecodes are assumed to potentially
  // throw asynchronous exceptions in compiled code due
  // to safepoints (note: these entries could be merged
  // with the can_trap_list - however, we need to know
  // which ones are asynchronous for now - see also the
  // comment in GraphBuilder::handle_exception)
  Bytecodes::Code is_async_list[] =
    { Bytecodes::_ifeq
    , Bytecodes::_ifne
    , Bytecodes::_iflt
    , Bytecodes::_ifge
    , Bytecodes::_ifgt
    , Bytecodes::_ifle
    , Bytecodes::_if_icmpeq
    , Bytecodes::_if_icmpne
    , Bytecodes::_if_icmplt
    , Bytecodes::_if_icmpge
    , Bytecodes::_if_icmpgt
    , Bytecodes::_if_icmple
    , Bytecodes::_if_acmpeq
    , Bytecodes::_if_acmpne
    , Bytecodes::_goto
    , Bytecodes::_jsr
    , Bytecodes::_ret
    , Bytecodes::_tableswitch
    , Bytecodes::_lookupswitch
    , Bytecodes::_ireturn
    , Bytecodes::_lreturn
    , Bytecodes::_freturn
    , Bytecodes::_dreturn
    , Bytecodes::_areturn
    , Bytecodes::_return
    , Bytecodes::_ifnull
    , Bytecodes::_ifnonnull
    , Bytecodes::_goto_w
    , Bytecodes::_jsr_w
    };

  // inititialize trap tables
  for (int i = 0; i < Bytecodes::number_of_java_codes; i++) {
    _can_trap[i] = false;
    _is_async[i] = false;
  }
  // set standard trap info
  for (int j = 0; j < sizeof(can_trap_list) / sizeof(can_trap_list[0]) ; j++) {
    _can_trap[can_trap_list[j]] = true;
  }

  // We now deoptimize if an asynchronous exception is thrown. This
  // considerably cleans up corner case issues related to javac's
  // incorrect exception handler ranges for async exceptions and
  // allows us to precisely analyze the types of exceptions from
  // certain bytecodes.
  if (!(DeoptC1 && DeoptOnAsyncException)) {
    // set asynchronous trap info
    for (int k = 0; k < sizeof(is_async_list) / sizeof(is_async_list[0]) ; k++) {
      assert(!_can_trap[is_async_list[k]], "can_trap_list and is_async_list should be disjoint");
      _can_trap[is_async_list[k]] = true;
      _is_async[is_async_list[k]] = true;
    }
  }
}


GraphBuilder::GraphBuilder(Compilation* compilation, IRScope* scope, BlockList* bci2block, BlockBegin* start)
  : _scope_data(NULL)
  , _exception_scope(NULL)
  , _instruction_count(0)
{
  assert(is_initialized(), "GraphBuilder must have been initialized");

  _compilation = compilation;
  push_root_scope(scope, bci2block, start);
  _vmap        = new ValueMap();
  _bailout_msg = NULL;
  scope_data()->add_to_work_list(start);
  iterate_all_blocks();
#ifndef PRODUCT
  if (PrintCompilation && Verbose) tty->print_cr("Created %d Instructions", _instruction_count);
#endif
}


void GraphBuilder::push_exception_scope() {
  if (_exception_scope == NULL) {
    _exception_scope = new ExceptionScope();
  } else {
    _exception_scope = _exception_scope->push_scope();    
  }
}


void GraphBuilder::pop_exception_scope() {
  _exception_scope = _exception_scope->pop_scope();
}


ValueStack* GraphBuilder::lock_stack() {
  // return a new ValueStack representing just the current lock stack
  // (for debug info at safepoints in exception throwing or handling)
  return state()->copy_locks();
}


int GraphBuilder::recursive_inline_level(ciMethod* cur_callee) const {
  int recur_level = 0;
  for (IRScope* s = scope(); s != NULL; s = s->caller()) {
    if (s->method() == cur_callee) {
      ++recur_level;
    }
  }
  return recur_level;
}


bool GraphBuilder::try_inline(ciMethod* callee, bool using_cha) {
  // Clear out any existing inline bailout condition
  clear_inline_bailout();

  if (compilation()->jvmpi_event_method_enabled()) {
    // do not inline at all
    INLINE_BAILOUT("jvmpi event method enabled")
  } else if (callee->should_exclude()) {
    // callee is excluded
    INLINE_BAILOUT("excluded by CompilerOracle")
  } else if (!callee->can_be_compiled()) {
    // callee is not compilable (prob. has breakpoints)
    INLINE_BAILOUT("not compilable")
  } else if (try_inline_intrinsics(callee)) {
    // intrinsics can be native or not
    return true;
  } else if (callee->is_native()) {
    // non-intrinsic natives cannot be inlined 
    INLINE_BAILOUT("non-intrinsic native")
  } else {
    return try_inline_full(callee, using_cha);
  }  
}


bool GraphBuilder::try_inline_intrinsics(ciMethod* callee) {
  bool preserves_state = false;
  if (!InlineNatives           ) INLINE_BAILOUT("intrinsic method inlining disabled");
  if (callee->is_synchronized()) INLINE_BAILOUT("intrinsic method is synchronized");
  // callee seems like a good candidate
  // determine id
  ciMethod::IntrinsicId id = callee->intrinsic_id();
  switch (id) {
    case ciMethod::_arraycopy     :
      if (!InlineArrayCopy) return false;
      break;

    case ciMethod::_currentTimeMillis:
      break;

    case ciMethod::_floatToRawIntBits   :
    case ciMethod::_intBitsToFloat      :
    case ciMethod::_doubleToRawLongBits :
    case ciMethod::_longBitsToDouble    :
      if (!InlineMathNatives) return false;
      preserves_state = true;
      break;

    case ciMethod::_getClass      :
      if (!InlineClassNatives) return false;
      preserves_state = true;
      break;

    case ciMethod::_currentThread :
      if (!InlineThreadNatives) return false;
      preserves_state = true;
      break;

    case ciMethod::_dsqrt         :
      if (!InlineMathNatives) return false;
      preserves_state = true;
      break;

    case ciMethod::_dsin          : // fall through
    case ciMethod::_dcos          : // fall through
      if (!InlineMathNatives) return false;
#ifndef SPARC
      preserves_state = true;
#endif
      break;

    // sun/misc/AtomicLong.attemptUpdate
    case ciMethod::_attemptUpdate :
      if (!VM_Version::supports_cx8()) return false;
      if (!InlineAtomicLong) return false;
      preserves_state = true;
      break;

    // %%% the following xxx_obj32 are temporary until the 1.4.0 sun.misc.Unsafe goes away
    case ciMethod::_getObject_obj32 : return append_unsafe_get_obj32(callee, T_OBJECT);  return true;
    case ciMethod::_getBoolean_obj32: return append_unsafe_get_obj32(callee, T_BOOLEAN); return true;
    case ciMethod::_getByte_obj32   : return append_unsafe_get_obj32(callee, T_BYTE);    return true;
    case ciMethod::_getShort_obj32  : return append_unsafe_get_obj32(callee, T_SHORT);   return true;
    case ciMethod::_getChar_obj32   : return append_unsafe_get_obj32(callee, T_CHAR);    return true;
    case ciMethod::_getInt_obj32    : return append_unsafe_get_obj32(callee, T_INT);     return true;
    case ciMethod::_getLong_obj32   : return append_unsafe_get_obj32(callee, T_LONG);    return true;
    case ciMethod::_getFloat_obj32  : return append_unsafe_get_obj32(callee, T_FLOAT);   return true;
    case ciMethod::_getDouble_obj32 : return append_unsafe_get_obj32(callee, T_DOUBLE);  return true;

    case ciMethod::_putObject_obj32 : return append_unsafe_put_obj32(callee, T_OBJECT);  return true;
    case ciMethod::_putBoolean_obj32: return append_unsafe_put_obj32(callee, T_BOOLEAN); return true;
    case ciMethod::_putByte_obj32   : return append_unsafe_put_obj32(callee, T_BYTE);    return true;
    case ciMethod::_putShort_obj32  : return append_unsafe_put_obj32(callee, T_SHORT);   return true;
    case ciMethod::_putChar_obj32   : return append_unsafe_put_obj32(callee, T_CHAR);    return true;
    case ciMethod::_putInt_obj32    : return append_unsafe_put_obj32(callee, T_INT);     return true;
    case ciMethod::_putLong_obj32   : return append_unsafe_put_obj32(callee, T_LONG);    return true;
    case ciMethod::_putFloat_obj32  : return append_unsafe_put_obj32(callee, T_FLOAT);   return true;
    case ciMethod::_putDouble_obj32 : return append_unsafe_put_obj32(callee, T_DOUBLE);  return true;

    // Use special nodes for Unsafe instructions so we can more easily
    // perform an address-mode optimization on the raw variants
    case ciMethod::_getObject_obj : return append_unsafe_get_obj(callee, T_OBJECT,  false);
    case ciMethod::_getBoolean_obj: return append_unsafe_get_obj(callee, T_BOOLEAN, false);
    case ciMethod::_getByte_obj   : return append_unsafe_get_obj(callee, T_BYTE,    false);
    case ciMethod::_getShort_obj  : return append_unsafe_get_obj(callee, T_SHORT,   false);
    case ciMethod::_getChar_obj   : return append_unsafe_get_obj(callee, T_CHAR,    false);
    case ciMethod::_getInt_obj    : return append_unsafe_get_obj(callee, T_INT,     false);
    case ciMethod::_getLong_obj   : return append_unsafe_get_obj(callee, T_LONG,    false);
    case ciMethod::_getFloat_obj  : return append_unsafe_get_obj(callee, T_FLOAT,   false);
    case ciMethod::_getDouble_obj : return append_unsafe_get_obj(callee, T_DOUBLE,  false);

    case ciMethod::_putObject_obj : return append_unsafe_put_obj(callee, T_OBJECT,  false);
    case ciMethod::_putBoolean_obj: return append_unsafe_put_obj(callee, T_BOOLEAN, false);
    case ciMethod::_putByte_obj   : return append_unsafe_put_obj(callee, T_BYTE,    false);
    case ciMethod::_putShort_obj  : return append_unsafe_put_obj(callee, T_SHORT,   false);
    case ciMethod::_putChar_obj   : return append_unsafe_put_obj(callee, T_CHAR,    false);
    case ciMethod::_putInt_obj    : return append_unsafe_put_obj(callee, T_INT,     false);
    case ciMethod::_putLong_obj   : return append_unsafe_put_obj(callee, T_LONG,    false);
    case ciMethod::_putFloat_obj  : return append_unsafe_put_obj(callee, T_FLOAT,   false);
    case ciMethod::_putDouble_obj : return append_unsafe_put_obj(callee, T_DOUBLE,  false); 

    case ciMethod::_getObjectVolatile_obj : return append_unsafe_get_obj(callee, T_OBJECT,  true); 
    case ciMethod::_getBooleanVolatile_obj: return append_unsafe_get_obj(callee, T_BOOLEAN, true);
    case ciMethod::_getByteVolatile_obj   : return append_unsafe_get_obj(callee, T_BYTE,    true);
    case ciMethod::_getShortVolatile_obj  : return append_unsafe_get_obj(callee, T_SHORT,   true);
    case ciMethod::_getCharVolatile_obj   : return append_unsafe_get_obj(callee, T_CHAR,    true);
    case ciMethod::_getIntVolatile_obj    : return append_unsafe_get_obj(callee, T_INT,     true);
    case ciMethod::_getLongVolatile_obj   : return append_unsafe_get_obj(callee, T_LONG,    true);
    case ciMethod::_getFloatVolatile_obj  : return append_unsafe_get_obj(callee, T_FLOAT,   true);
    case ciMethod::_getDoubleVolatile_obj : return append_unsafe_get_obj(callee, T_DOUBLE,  true);

    case ciMethod::_putObjectVolatile_obj : return append_unsafe_put_obj(callee, T_OBJECT,  true);
    case ciMethod::_putBooleanVolatile_obj: return append_unsafe_put_obj(callee, T_BOOLEAN, true);
    case ciMethod::_putByteVolatile_obj   : return append_unsafe_put_obj(callee, T_BYTE,    true);
    case ciMethod::_putShortVolatile_obj  : return append_unsafe_put_obj(callee, T_SHORT,   true);
    case ciMethod::_putCharVolatile_obj   : return append_unsafe_put_obj(callee, T_CHAR,    true);
    case ciMethod::_putIntVolatile_obj    : return append_unsafe_put_obj(callee, T_INT,     true);
    case ciMethod::_putLongVolatile_obj   : return append_unsafe_put_obj(callee, T_LONG,    true);
    case ciMethod::_putFloatVolatile_obj  : return append_unsafe_put_obj(callee, T_FLOAT,   true);
    case ciMethod::_putDoubleVolatile_obj : return append_unsafe_put_obj(callee, T_DOUBLE,  true);

    case ciMethod::_getByte_raw   : return append_unsafe_get_raw(callee, T_BYTE);
    case ciMethod::_getShort_raw  : return append_unsafe_get_raw(callee, T_SHORT);
    case ciMethod::_getChar_raw   : return append_unsafe_get_raw(callee, T_CHAR);
    case ciMethod::_getInt_raw    : return append_unsafe_get_raw(callee, T_INT);
    case ciMethod::_getLong_raw   : return append_unsafe_get_raw(callee, T_LONG);
    case ciMethod::_getFloat_raw  : return append_unsafe_get_raw(callee, T_FLOAT);
    case ciMethod::_getDouble_raw : return append_unsafe_get_raw(callee, T_DOUBLE);

    case ciMethod::_putByte_raw   : return append_unsafe_put_raw(callee, T_BYTE);
    case ciMethod::_putShort_raw  : return append_unsafe_put_raw(callee, T_SHORT);
    case ciMethod::_putChar_raw   : return append_unsafe_put_raw(callee, T_CHAR);
    case ciMethod::_putInt_raw    : return append_unsafe_put_raw(callee, T_INT);
    case ciMethod::_putLong_raw   : return append_unsafe_put_raw(callee, T_LONG);
    case ciMethod::_putFloat_raw  : return append_unsafe_put_raw(callee, T_FLOAT);
    case ciMethod::_putDouble_raw : return append_unsafe_put_raw(callee, T_DOUBLE);

    case ciMethod::_checkIndex    :
      if (!InlineNIOCheckIndex) return false;
      preserves_state = true;
      break;

    case ciMethod::_compareAndSwapLong_obj: 
      if (!VM_Version::supports_cx8()) return false;
      // fall through
    case ciMethod::_compareAndSwapInt_obj: 
    case ciMethod::_compareAndSwapObject_obj: 
      append_unsafe_CAS(callee);
      return true; 

    default                       : return false; // do not inline
  }
  // create intrinsic node
  const bool has_receiver = !callee->is_static();
  ValueType* result_type = as_ValueType(callee->return_type());
  Values* args = state()->pop_arguments(callee->arg_size());
  ValueStack* locks = lock_stack();
  Intrinsic* result = new Intrinsic(result_type, id, args, has_receiver, lock_stack(), preserves_state);
  // append instruction & push result
  Value v = append_split(result);
  if (result_type != voidType) push(result_type, v);
  
#ifndef PRODUCT
  // printing
  if (PrintInlining) {
    print_inline_result(callee, true);
  }
#endif

  // done
  return true;
}


bool GraphBuilder::try_inline_jsr(int jsr_dest_bci) {
  // Introduce a new callee continuation point - all Ret instructions
  // will be replaced with Gotos to this point.
  BlockBegin* cont = block_at(next_bci());
  bool continuation_existed = true;
  if (cont == NULL) {
    cont = new BlockBegin(next_bci());
    continuation_existed = false;
  }
  // Note: can not assign state to continuation yet, as we have to
  // pick up the state from the Ret instructions.

  // Push callee scope
  push_scope_for_jsr(cont, jsr_dest_bci);

  // Temporarily set up bytecode stream so we can append instructions
  // (only using the bci of this stream)
  scope_data()->set_stream(scope_data()->parent()->stream());

  BlockBegin* jsr_start_block = block_at(jsr_dest_bci);
  assert(jsr_start_block != NULL, "jsr start block must exist");
  assert(!jsr_start_block->is_set(BlockBegin::was_visited_flag), "should not have visited jsr yet");
  state()->pin_stack_all(Instruction::PinInlineEndOfBlock);
  Goto* goto_sub = new Goto(jsr_start_block, false);
  goto_sub->set_state(state());
  // Must copy state to avoid wrong sharing when parsing bytecodes
  // First make callee state into phis
  set_state(new ValueStack(state()));
  // Now copy it
  assert(jsr_start_block->state() == NULL, "should have fresh jsr starting block");
  jsr_start_block->set_state(state()->copy());
  append(goto_sub);
  _block->set_end(goto_sub);
  _last = _block = jsr_start_block;
  state()->clear_locals();
  vmap()->kill_all();

  // Clear out bytecode stream
  scope_data()->set_stream(NULL);

  scope_data()->add_to_work_list(jsr_start_block);

  // Ready to resume parsing in subroutine
  iterate_all_blocks();

  // If we bailed out during parsing, return immediately (this is bad news)
  if (bailed_out()) return false;
  
  assert(continuation_existed ||
         !jsr_continuation()->is_set(BlockBegin::was_visited_flag),
         "jsr continuation should not have been parsed yet if we created it");

  // Detect whether the continuation can actually be reached. If not,
  // it has not had state set by the join() operations in
  // iterate_bytecodes_for_block()/ret() and we should not touch the
  // iteration state. The calling activation of
  // iterate_bytecodes_for_block will then complete normally.
  if (jsr_continuation()->state() != NULL) {
    // Set up iteration state for resuming parsing in jsr continuation
    _last = _block = jsr_continuation();
    set_state(jsr_continuation()->state()->copy());
    vmap()->kill_all();
  }

  pop_scope_for_jsr();

  return true;
}


bool GraphBuilder::try_inline_full(ciMethod* callee, bool using_cha) {
  assert(!callee->is_native(), "callee must not be native");

  // first perform tests of things it's not possible to inline
  if (callee->has_exception_handlers() &&
      !InlineMethodsWithExceptionHandlers) INLINE_BAILOUT("callee has exception handlers");
  if (callee->is_synchronized()          ) INLINE_BAILOUT("callee is synchronized");
  if (!callee->holder()->is_initialized()) INLINE_BAILOUT("callee's klass not initialized yet");
  if (!callee->has_balanced_monitors())    INLINE_BAILOUT("callee's monitors do not match");
  
  // NOTE: for now, also check presence of monitors -- there is a bug
  // in the handling of nested scopes' monitor slots
  if (callee->has_monitor_bytecodes())     INLINE_BAILOUT("monitor bytecodes not supported by inliner yet");

  // Proper inlining of methods with jsrs requires us either to store
  // initialization values for initvars reported by the CI, or
  // (preferably) generation of oop maps directly from LIR
  if (callee->has_jsrs()                 ) INLINE_BAILOUT("jsrs not handled properly by inliner yet");

  // now perform tests that are based on flag settings
  if (inline_level() > MaxInlineLevel                         ) INLINE_BAILOUT("too-deep inlining");
  if (recursive_inline_level(callee) > MaxRecursiveInlineLevel) INLINE_BAILOUT("too-deep recursive inlining");
  if (callee->code_size() > max_inline_size()                 ) INLINE_BAILOUT("callee is too large");

  // don't inline throwable methods unless the inlining tree is rooted in a throwable class
  if (callee->name() == ciSymbol::object_initializer_name() &&
      callee->holder()->is_subclass_of(ciEnv::current()->Throwable_klass())) {
    // Throwable constructor call
    IRScope* top = scope();
    while (top->caller() != NULL) {
      top = top->caller();
    }
    if (!top->method()->holder()->is_subclass_of(ciEnv::current()->Throwable_klass())) {
      INLINE_BAILOUT("don't inline Throwable constructors");
    }
  }

  if (strict_fp_requires_explicit_rounding && method()->is_strict() != callee->is_strict()) INLINE_BAILOUT("caller and caller have different strict fp requirements");

#ifndef PRODUCT
      // printing
  if (PrintInlining) {
    print_inline_result(callee, true);
  }
#endif

  // NOTE: Bailouts from this point on, which occur at the
  // GraphBuilder level, do not cause bailout just of the inlining but
  // in fact of the entire compilation.

  BlockBegin* orig_block = block();

  const int args_base = state()->stack_size() - callee->arg_size();
  assert(args_base >= 0, "stack underflow during inlining");

  // Insert null check if necessary
  if (code() != Bytecodes::_invokestatic) {
    // note: null check must happen even if first instruction of callee does
    //       an implicit null check since the callee is in a different scope
    //       and we must make sure exception handling does the right thing
    assert(!callee->is_static(), "callee must not be static");
    assert(callee->arg_size() > 0, "must have at least a receiver");
    int i = args_base;
    append(new NullCheck(state()->stack_at_inc(i), lock_stack()));
  }

  // Introduce a new callee continuation point - if the callee has
  // more than one return instruction or the return does not allow
  // fall-through of control flow, all return instructions of the
  // callee will need to be replaced by Goto's pointing to this
  // continuation point.
  BlockBegin* cont = block_at(next_bci());
  bool continuation_existed = true;
  if (cont == NULL) {
    cont = new BlockBegin(next_bci());
    continuation_existed = false;
  }
  if (cont->state() == NULL) {
    // Assign state to continuation, pushing return value if any
    cont->set_state(new ValueStack(state()));
    cont->state()->truncate_stack(args_base);
    ValueType* return_type = NULL;
    if (callee->return_type()->basic_type() != T_VOID) {
      return_type = as_ValueType(callee->return_type());
      cont->state()->push(return_type, new Phi(return_type));
    }
  }
#ifdef ASSERT
  else {
    ValueStack* tmp_state = new ValueStack(state());
    tmp_state->truncate_stack(args_base);
    ValueType* return_type = NULL;
    if (callee->return_type()->basic_type() != T_VOID) {
      return_type = as_ValueType(callee->return_type());
      tmp_state->push(return_type, new Phi(return_type));
    }
    assert(cont->try_join(tmp_state), "error in management of continuation state");
  }
#endif

  // Push callee scope
  push_scope(callee, cont, using_cha);

  // Temporarily set up bytecode stream so we can append instructions
  // (only using the bci of this stream)
  scope_data()->set_stream(scope_data()->parent()->stream());

  // Pass parameters into callee state: add assignments
  // note: this will also ensure that all arguments are computed before being passed
  ValueStack* callee_state = state();
  ValueStack* caller_state = scope()->caller_state();
  { int i = args_base;
    while (i < caller_state->stack_size()) {
      const int par_no = i - args_base;
      Value  arg = caller_state->stack_at_inc(i);
      // NOTE: take base() of arg->type() to avoid problems storing
      // constants
      store_local(callee_state, arg, arg->type()->base(), par_no, true);
    }
  }
  
  // Remove args from stack.
  // Note that we preserve locals state in case we can use it later
  // (see use of pop_scope() below)
  caller_state->truncate_stack(args_base);
  callee_state->truncate_stack(args_base);

  // Compute lock stack size for callee scope now that args have been passed
  scope()->compute_lock_stack_size();
  
  BlockBegin* callee_start_block = block_at(0);
  if (callee_start_block != NULL) {
    // There is a backward branch to bci 0 somewhere in the callee,
    // meaning that parsing can not just resume in the callee; the
    // current block must be terminated, all values on the stack
    // pinned, the callee state turned into phis, and the ValueMap
    // killed since it is illegal to share values across basic blocks.
    caller_state->pin_stack_all(Instruction::PinInlineEndOfBlock);
    // Must copy callee state to avoid wrong sharing when parsing bytecodes
    // First make callee state into phis
    callee_state = new ValueStack(callee_state);
    // Now copy it
    callee_start_block->set_state(callee_state->copy());
    Goto* goto_callee = new Goto(callee_start_block, false);
    goto_callee->set_state(state());
    append(goto_callee);
    _block->set_end(goto_callee);
    _last = _block = callee_start_block;
    callee_state->clear_locals();
    vmap()->kill_all();
    scope_data()->add_to_work_list(callee_start_block);
  }

  // Clear out bytecode stream
  scope_data()->set_stream(NULL);

  // Ready to resume parsing in callee (either in the same block we
  // were in before or in the callee's start block)
  iterate_all_blocks(callee_start_block == NULL);

  // If we bailed out during parsing, return immediately (this is bad news)
  if (bailed_out()) return false;

  // iterate_all_blocks theoretically traverses in random order; in
  // practice, we have only traversed the continuation if we are
  // inlining into a subroutine
  assert(continuation_existed ||
         !continuation()->is_set(BlockBegin::was_visited_flag),
         "continuation should not have been parsed yet if we created it");

  // At this point we are almost ready to return and resume parsing of
  // the caller back in the GraphBuilder. The only thing we want to do
  // first is an optimization: during parsing of the callee we
  // generated at least one Goto to the continuation block. If we
  // generated exactly one, and if the inlined method spanned exactly
  // one block (and we didn't have to Goto its entry), then we snip
  // off the Goto to the continuation, allowing control to fall
  // through back into the caller block and effectively performing
  // block merging. This allows load elimination and CSE to take place
  // across multiple callee scopes if they are relatively simple, and
  // is currently essential to making inlining profitable.
  if (   num_returns() == 1
      && block() == orig_block
      && block() == inline_cleanup_block()) {
    _last = inline_cleanup_return_prev();
    _state = inline_cleanup_state()->pop_scope(true, bci());
  } else {
    // Can not perform this optimization.
    // Must go back and pin the state of the first goto to the
    // continuation point
    if (inline_cleanup_state() != NULL) {
      inline_cleanup_state()->pin_stack_all(Instruction::PinInlineEndOfBlock);
    }
    // Resume parsing in continuation block unless it was already parsed.
    // Note that if we don't change _last here, iteration in
    // iterate_bytecodes_for_block will stop when we return.
    if (!continuation()->is_set(BlockBegin::was_visited_flag)) {
      // Change the state that we will restore to the GraphBuilder to
      // instead be a list of Phis which includes the return value; this
      // is the continuation's state. (As usual, have to make a copy.)
      _last = _block = continuation();
      // Since we will be resuming parsing in the continuation block
      // when we return, mark it as having been traversed so we don't
      // visit it twice
      continuation()->set(BlockBegin::was_visited_flag);
      _state = continuation()->state()->copy();
      // Must not canonicalize across blocks
      vmap()->kill_all();
    }
  }
  
  pop_scope();

  compilation()->notice_inlined_method(callee);

  return true;
}


void GraphBuilder::inline_bailout(const char* msg) {
  assert(msg != NULL, "inline bailout msg must exist");
  _inline_bailout_msg = msg;
}


void GraphBuilder::clear_inline_bailout() {
  _inline_bailout_msg = NULL;
}


void GraphBuilder::push_root_scope(IRScope* scope, BlockList* bci2block, BlockBegin* start) {
  ScopeData* data = new ScopeData(NULL);
  data->set_scope(scope);
  data->set_bci2block(bci2block);
  _scope_data = data;
  _block = start;

  push_exception_scope();
}


void GraphBuilder::push_scope(ciMethod* callee, BlockBegin* continuation, bool using_cha) {
  IRScope* callee_scope = new IRScope(compilation(), scope(), bci(), callee, -1, false);
  scope()->add_callee(callee_scope);
  callee_scope->set_caller_state(state());
  set_state(state()->push_scope(callee_scope));
  ScopeData* data = new ScopeData(scope_data(), using_cha);
  data->set_scope(callee_scope);
  BlockListBuilder blb(callee_scope, -1, false);
  data->set_bci2block(blb.bci2block());
  data->set_continuation(continuation);
  _scope_data = data;

  push_exception_scope();
}


void GraphBuilder::push_scope_for_jsr(BlockBegin* jsr_continuation, int jsr_dest_bci) {
  ScopeData* data = new ScopeData(scope_data());
  data->set_parsing_jsr();
  data->set_jsr_entry_bci(jsr_dest_bci);
  data->set_jsr_return_address_local(-1);
  // Must clone bci2block list as we will be mutating it in order to
  // properly clone all blocks in jsr region as well as exception
  // handlers containing rets
  BlockList* new_bci2block = new BlockList(bci2block()->length());
  new_bci2block->push_all(bci2block());
  data->set_bci2block(new_bci2block);
  data->set_scope(scope());
  data->setup_jsr_xhandlers();
  data->set_continuation(continuation());
  data->set_jsr_continuation(jsr_continuation);
  _scope_data = data;
}


void GraphBuilder::pop_scope() {
  _scope_data = scope_data()->parent();
  
  pop_exception_scope();
}


void GraphBuilder::pop_scope_for_jsr() {
  _scope_data = scope_data()->parent();
}


bool GraphBuilder::append_unsafe_get_obj32(ciMethod* callee, BasicType t) {
  if (InlineUnsafeOps) {
    Values* args = state()->pop_arguments(callee->arg_size());
    Instruction* op = append(new UnsafeGetObject(t, args->at(0), args->at(1), args->at(2), lock_stack(), false));
    push(op->type(), op);
    compilation()->set_has_unsafe_access(true);
  }
  return InlineUnsafeOps;
}


bool GraphBuilder::append_unsafe_put_obj32(ciMethod* callee, BasicType t) {
  if (InlineUnsafeOps) {
    Values* args = state()->pop_arguments(callee->arg_size());
    Instruction* op = append(new UnsafePutObject(t, args->at(0), args->at(1), args->at(2), args->at(3), lock_stack(), false));
    compilation()->set_has_unsafe_access(true);
  }
  return InlineUnsafeOps;
}


bool GraphBuilder::append_unsafe_get_obj(ciMethod* callee, BasicType t, bool is_volatile) {
  if (InlineUnsafeOps) {
    Values* args = state()->pop_arguments(callee->arg_size());
    Instruction* offset = append(new Convert(Bytecodes::_l2i, args->at(2), as_ValueType(T_INT)));
    Instruction* op = append(new UnsafeGetObject(t, args->at(0), args->at(1), offset, lock_stack(), is_volatile));
    push(op->type(), op);
    compilation()->set_has_unsafe_access(true);
  }
  return InlineUnsafeOps;
}


bool GraphBuilder::append_unsafe_put_obj(ciMethod* callee, BasicType t, bool is_volatile) {
  if (InlineUnsafeOps) {
    Values* args = state()->pop_arguments(callee->arg_size());
    Instruction* offset = append(new Convert(Bytecodes::_l2i, args->at(2), as_ValueType(T_INT)));
    Instruction* op = append(new UnsafePutObject(t, args->at(0), args->at(1), offset, args->at(3), lock_stack(), is_volatile));
    compilation()->set_has_unsafe_access(true);
  }
  return InlineUnsafeOps;
}


bool GraphBuilder::append_unsafe_get_raw(ciMethod* callee, BasicType t) {
  if (InlineUnsafeOps) {
    Values* args = state()->pop_arguments(callee->arg_size());
    Instruction* op = append(new UnsafeGetRaw(t, args->at(0), args->at(1), lock_stack()));
    push(op->type(), op);
    compilation()->set_has_unsafe_access(true);
  }
  return InlineUnsafeOps;
}


bool GraphBuilder::append_unsafe_put_raw(ciMethod* callee, BasicType t) {
  if (InlineUnsafeOps) {
    Values* args = state()->pop_arguments(callee->arg_size());
    Instruction* op = append(new UnsafePutRaw(t, args->at(0), args->at(1), args->at(2), lock_stack()));
    compilation()->set_has_unsafe_access(true);
  }
  return InlineUnsafeOps;
}


void GraphBuilder::append_unsafe_CAS(ciMethod* callee) {
  ValueType* result_type = as_ValueType(callee->return_type());
  assert(result_type->is_int(), "int result");
  Values* args = state()->pop_arguments(callee->arg_size());

  // Pop off some args to speically handle, then push back
  Value newval = args->pop();
  Value cmpval = args->pop();
  Value long_offset = args->pop();
  Value src = args->pop();
  Value unsafe_obj = args->pop();

  // Separately handle the unsafe arg. It is not needed for code
  // generation, but must be null checked
  append(new NullCheck(unsafe_obj, lock_stack()));

  Instruction* offset = new Convert(Bytecodes::_l2i, long_offset, as_ValueType(T_INT));
  append(offset);

  args->push(src);
  args->push(offset);
  args->push(cmpval);
  args->push(newval);

  Intrinsic* result = new Intrinsic(result_type, callee->intrinsic_id(), args, false, lock_stack(), true);
  append_split(result);
  push(result_type, result);

  // An unsafe CAS can alias with other field accesses, but we
  // don't know which ones, so clear all
  state()->clear_stores();
  vmap()->kill_all();
  compilation()->set_has_unsafe_access(true);
}

#ifndef PRODUCT
void GraphBuilder::print_inline_result(ciMethod* callee, bool res) {
  tty->print("     ");
  for (int i = 0; i < scope()->level(); i++) tty->print("  ");
  if (res) {
    tty->print("  ");
  } else {
    tty->print("- ");
  }
  tty->print("@ %d  ", bci());
  callee->print_short_name();
  tty->print(" (%d bytes)", callee->code_size());
  if (_inline_bailout_msg) {
    tty->print("  %s", _inline_bailout_msg);
  }
  tty->cr();

  if (res && CIPrintMethodCodes) {
    callee->print_codes();
  }
}


void GraphBuilder::print_stats() {
  vmap()->print();
}
#endif // PRODUCT
