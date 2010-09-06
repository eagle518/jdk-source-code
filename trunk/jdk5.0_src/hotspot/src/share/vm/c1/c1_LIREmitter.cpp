#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIREmitter.cpp	1.126 04/04/20 15:56:12 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_c1_LIREmitter.cpp.incl"

#define __ _lir->

//
// IRScopeDebugInfo records the debug information for a particular IRScope
// in a particular CodeEmitInfo.  This allows the information to be computed
// once early enough for the OopMap to be available to the LIR and also to be
// reemited for different pcs using the same CodeEmitInfo without recomputing
// everything.
//

class IRScopeDebugInfo: public CompilationResourceObj {
 private:
  IRScope*                      _scope;
  int                           _bci;
  GrowableArray<ScopeValue*>*   _locals;
  GrowableArray<ScopeValue*>*   _expressions;
  GrowableArray<MonitorValue*>* _monitors;
  IRScopeDebugInfo*             _caller;

 public:
  IRScopeDebugInfo(IRScope*                      scope,
                   int                           bci,
                   GrowableArray<ScopeValue*>*   locals,
                   GrowableArray<ScopeValue*>*   expressions,
                   GrowableArray<MonitorValue*>* monitors,
                   IRScopeDebugInfo*             caller):
      _scope(scope)
    , _locals(locals)
    , _bci(bci)
    , _expressions(expressions)
    , _monitors(monitors)
    , _caller(caller) {}


  IRScope*                      scope()       { return _scope;       }
  int                           bci()         { return _bci;         }
  GrowableArray<ScopeValue*>*   locals()      { return _locals;      }
  GrowableArray<ScopeValue*>*   expressions() { return _expressions; }
  GrowableArray<MonitorValue*>* monitors()    { return _monitors;    }
  IRScopeDebugInfo*             caller()      { return _caller;      }

  void record_debug_info(DebugInformationRecorder* recorder) {
    if (caller() != NULL) {
      caller()->record_debug_info(recorder);
    }
    DebugToken* locvals = recorder->create_scope_values(locals());
    DebugToken* expvals = recorder->create_scope_values(expressions());
    DebugToken* monvals = recorder->create_monitor_values(monitors());
    recorder->describe_scope(scope()->method(), bci(), locvals, expvals, monvals);
  }
};


// Stack must be NON-null
CodeEmitInfo::CodeEmitInfo(LIR_Emitter* emit, int bci, intStack* soops, ValueStack* stack, ExceptionScope* exception_scope, RInfoCollection* oops_in_regs)
  : _scope(stack->scope())
  , _bci(bci)
  , _spilled_oops(soops)
  , _scope_debug_info(NULL)
  , _oop_map(NULL)
  , _stack(stack)
  , _exception_scope(exception_scope)
  , _register_oops(oops_in_regs)
  , _local_mapping(NULL)
#ifndef PRODUCT
  , _lir_oop_map(NULL, 0)
  , _lir_oop_map_set(false)
  , _lir_adjusted(false)
#endif // PRODUCT
  , _lir_expression_stack(NULL)
  , _is_compiled_safepoint(false) {
  NEEDS_CLEANUP // fix comparision, as _stack is never
  if (DeoptC1 && _stack != NULL) {
    if (_stack->is_lock_stack()) {
      fill_expression_stack();
    } else {
      _lir_expression_stack = emit->compilation()->value_stack2lir_stack(stack);
    }
  }
  assert(_bci == SynchronizationEntryBCI || Bytecodes::is_defined(scope()->method()->java_code_at_bci(_bci)), "make sure bci points at a real bytecode");
}


CodeEmitInfo::CodeEmitInfo(CodeEmitInfo* info, bool lock_stack_only)
  : _scope(info->_scope)
  , _exception_scope(info->_exception_scope)
  , _bci(info->_bci)
  , _scope_debug_info(NULL)
  , _oop_map(NULL)
#ifndef PRODUCT
  , _lir_oop_map(NULL, 0) 
  , _lir_oop_map_set(false)
  , _lir_adjusted(false)
#endif // PRODUCT
  , _local_mapping(NULL)
  , _is_compiled_safepoint(false) {
  // make a copy of register_oops since it's mutable
  _register_oops = new RInfoCollection();
  if (info->_register_oops != NULL) {
    for (int i = 0; i < info->_register_oops->length(); i++) {
      _register_oops->append(info->_register_oops->at(i));
    }
  }
  if (lock_stack_only) {
    _stack = NULL;
    _lir_expression_stack = NULL;

    if (info->_stack != NULL) {
      _stack = info->_stack->copy_locks();
      fill_expression_stack();
    }

    _spilled_oops = NULL;
    _local_mapping = info->_local_mapping;
  } else {
    _stack = info->_stack;
    _lir_expression_stack = info->_lir_expression_stack;
    _spilled_oops = info->_spilled_oops;
    _local_mapping = info->_local_mapping;
  }
}


void CodeEmitInfo::fill_expression_stack() {
  assert(_stack->is_lock_stack(), "must be");

  // Create a dummy expression stack to make the debug info have the
  // right number of elements on the expression stack.
  ValueStack* caller_state = _stack->caller_state();
  if (caller_state != NULL) {
    int depth = caller_state->stack_size();
    if (depth > 0) {
      _lir_expression_stack = new GrowableArray<LIR_Opr>(depth);
      for (int i = 0; i < depth;) {
        Value v = caller_state->stack_at_inc(i);
        _lir_expression_stack->append(LIR_OprFact::dummy_value_type(v->type()));
      }
    }
  }
}



#ifdef ASSERT
void CodeEmitInfo::check_is_exception_info() const {
  // CodeEmitInfo that are used at exception sites are expected to
  // have no visible expression stack state but they need to have the
  // proper depth with dummy information, so check that.
  assert(stack() == NULL || stack()->is_lock_stack(), "must be");
  ValueStack* caller_stack = stack() != NULL ? stack()->caller_state() : NULL;
  if (caller_stack != NULL) {
    int depth = caller_stack->stack_size();
    GrowableArray<LIR_Opr>* stack = lir_expression_stack();
    if (stack == NULL) {
      assert(depth == 0, "stack depth doesn't match");
    } else for (int i = 0; i < stack->length(); i++) {
      // asserts check that this is a constant
      LIR_Const* value = stack->at(i)->as_constant_ptr();
      if (value->type() == T_DOUBLE || value->type() == T_LONG) {
        i++;
      }
    }
  }
}
#endif


void CodeEmitInfo::set_local_mapping(LocalMapping* mapping) {
  assert(_local_mapping == NULL, "can't set it twice");
  _local_mapping = mapping;
  if (_register_oops == NULL) {
    _register_oops = new RInfoCollection();
  }
}


#ifndef PRODUCT
void CodeEmitInfo::set_lir_oop_map(BitMap* map) {
  if (_lir_oop_map.size() != 0) {
    assert(_lir_oop_map.size() == map->size(), "Oop map sizes must be consistent");
  }
  _lir_oop_map.resize(map->size());
  _lir_oop_map.set_from(*map);
  NOT_PRODUCT(_lir_oop_map_set = true;)
}
#endif


void CodeEmitInfo::record_spilled_oops(FrameMap* frame_map, OopMap* oop_map) const {
  if (_spilled_oops != NULL) {
    int frame_size = frame_map->framesize();
    int arg_count = frame_map->oop_map_arg_count();
    for (int i = _spilled_oops->length(); i-- > 0;) {
      int spill_ix = _spilled_oops->at(i);
      OptoReg::Name rn = frame_map->single_word_regname(frame_map->spill_name(spill_ix));
      oop_map->set_oop(rn, frame_size, arg_count);
    }
  }
}


RInfo CodeEmitInfo::get_cache_reg(int pos, ValueTag tag) const {
  return _local_mapping != NULL ? _local_mapping->get_cache_reg(pos, tag) : norinfo;
}


RInfo CodeEmitInfo::get_cache_reg(int pos) const {
  return _local_mapping != NULL ? _local_mapping->get_cache_reg(pos) : norinfo;
}


RInfo CodeEmitInfo::get_cache_reg_for_local_offset(int pos) const {
  return _local_mapping != NULL ? _local_mapping->get_cache_reg_for_local_offset(pos) : norinfo;
}


bool CodeEmitInfo::is_cache_reg(RInfo reg) const {
  return _local_mapping != NULL ? _local_mapping->is_cache_reg(reg) : false;
}


OopMap* CodeEmitInfo::oop_map() {
  if (_oop_map == NULL) {
    compute_debug_info();
  }
  return _oop_map->deep_copy();
}


void CodeEmitInfo::compute_debug_info() {
  if (_oop_map == NULL) {
    NEEDS_CLEANUP; // we need to figure out what is the right thing to do for natives on x86
    // Note: i486 version is not using calling convention (passing argumentsn in registers)
    //       therefore we do not use create_oop_map_inside_natives for i486
#ifdef SPARC
    if (method()->is_native()) {
      if (bci() == SynchronizationEntryBCI) {
        _oop_map = create_oop_map_for_own_signature();
      } else {
        _oop_map = create_oop_map_inside_natives();
      }
    } else {
      _oop_map = create_oop_map();
    }
#else
    _oop_map = (bci() == SynchronizationEntryBCI || method()->is_native()) ?
      create_oop_map_for_own_signature() : create_oop_map();
#endif
    if (has_register_oops()) {
      assert(!method()->is_native(), "internal error");
      add_registers_to_oop_map(_oop_map);
    }

    // compute debug info for each enclosing scope, from outermost to innermost, with a recursive function
    GrowableArray<ScopeValue*>* stack = NULL;
    Values locks;
    int stack_end = 0;
    int locks_end = 0;
    if (compilation()->needs_debug_information()) {
      // expression stack; convert from LIR_Opr's to ScopeValue's
      stack = lir_stack2value_stack(lir_expression_stack());
      if (stack != NULL) stack_end = stack->length();
      // monitor stack
      locks = this->stack() != NULL ? this->stack()->locks() : Values();
      locks_end = locks.length();
    }

#ifdef ASSERT
    {
      if (bci() != SynchronizationEntryBCI && !scope()->method()->is_native()) {
        Bytecodes::Code code = scope()->method()->java_code_at_bci(bci());
        switch (code) {
        case Bytecodes::_ifnull    : // fall through
        case Bytecodes::_ifnonnull : // fall through
        case Bytecodes::_ifeq      : // fall through
        case Bytecodes::_ifne      : // fall through
        case Bytecodes::_iflt      : // fall through
        case Bytecodes::_ifge      : // fall through
        case Bytecodes::_ifgt      : // fall through
        case Bytecodes::_ifle      : // fall through
        case Bytecodes::_if_icmpeq : // fall through
        case Bytecodes::_if_icmpne : // fall through
        case Bytecodes::_if_icmplt : // fall through
        case Bytecodes::_if_icmpge : // fall through
        case Bytecodes::_if_icmpgt : // fall through
        case Bytecodes::_if_icmple : // fall through
        case Bytecodes::_if_acmpeq : // fall through
        case Bytecodes::_if_acmpne :
          assert(stack_end >= -Bytecodes::depth(code), "must have non-empty expression stack at if bytecode");
          break;
        }
      }
    }
#endif

    _scope_debug_info = compute_debug_info_for_scope(scope(), bci(), stack, stack_end, locks, locks_end);
  }
}


void CodeEmitInfo::record_debug_info(DebugInformationRecorder* recorder, int pc_offset, bool at_call) {
  // record the safepoint before recording the debug info for enclosing scopes
  recorder->add_safepoint(pc_offset, at_call, oop_map());
  _scope_debug_info->record_debug_info(recorder);
}



// This is the main method that creates an OopMap using the spilled oops,
// and pregenerated locals oop map information; it is called from the CodeEmit;
// The oop map generated here is returned to add_debug_info()
OopMap* CodeEmitInfo::create_oop_map() {
  IRScope* scope = this->scope();
  int bci = this->bci();
  int frame_size = frame_map()->framesize();
  int arg_count = frame_map()->oop_map_arg_count();
  OopMap* map = new OopMap(frame_size, arg_count);
  record_spilled_oops(frame_map(), map);

#ifndef PRODUCT
  if (LIROopMaps) {
    // Note that there is no block being assembled when slow case
    // stubs are emitted; verification of these LIR oop maps is
    // skipped since we don't know whether the instruction is
    // reachable
    assert(was_lir_oop_map_set() ||
           frame_map()->size_locals() == 0 ||
           scope->compilation()->cur_assembled_block() == NULL ||
           !scope->compilation()->cur_assembled_block()->is_set(BlockBegin::lir_oop_map_gen_reachable_flag),
           "Bug in LIR_OopMapGenerator's traversal; instruction not visited");

    // Note that we need an approximation to liveness,
    // "local_is_live_in_scope", because we don't have any real
    // liveness information.
    int sz = (int) _lir_oop_map.size();
    for (int i = 0; i < sz; i++) {
      if (_lir_oop_map.at(i) && local_is_live_in_scope(i)) {
        RInfo cache_reg = get_cache_reg(i, objectTag);
        if (!cache_reg.is_illegal()) {
          OptoReg::Name rn = FrameMap::cpu_regname(cache_reg);
          map->set_oop(rn, frame_size, arg_count);
        } else {
          OptoReg::Name rn = frame_map()->single_word_regname(i);
          map->set_oop(rn, frame_size, arg_count);
        }
      }
    }
  }
#endif // PRODUCT
  
  if (!LIROopMaps ||
      (LIROopMaps && VerifyLIROopMaps NOT_PRODUCT(&& was_lir_oop_map_set()))) {
    OopMap* map2;
    if (VerifyLIROopMaps) {
      map2 = new OopMap(frame_size, arg_count);
      record_spilled_oops(frame_map(), map2);
    } else {
      map2 = map;
    }

    for (; scope != NULL; bci = scope->caller_bci(), scope = scope->caller()) {
      scope->clear_local_oop_map();
      if (scope->oop_map() != NULL) {
        // FIXME
        /*
        tty->print_cr("Adding oop map entries for %s.%s%s@%d",
                      scope->method()->holder()->name()->as_utf8(),
                      scope->method()->name()->as_utf8(),
                      scope->method()->signature()->as_symbol()->as_utf8(),
                      bci);
        */

        // find oop map index for this bci
        ciLocalMapIterator oops(scope->oop_map(), bci);

        // This assert may fail for exception handlers that are not reachable
        // assert(oops != NULL, "No oop map for this bci");
        while (!oops.done()) {
          int local_index = oops.next_oop_offset();
          //          tty->print("(L%d?) ", local_index);
          scope->set_local_is_oop(local_index); // needed for debug info generation
          // convert to offset
          Local* aLocal = scope->local_at(objectType, local_index);
#ifdef ASSERT
          if (local_index == 0 && !scope->method()->is_static() && scope->method()->is_synchronized()) {
            assert(aLocal != NULL, "local at index 0 is used for synchronization");
          }
#endif
          // The local algorithm of the IR is more precise than the oop map generator
          if (aLocal != NULL && (scope->is_top_scope() || aLocal->use_count() > 0)) {
            int local_name = aLocal->local_name();
            //            tty->print("L%d/name %d ", aLocal->java_index(), local_name);
            RInfo cache_reg = get_cache_reg(local_name, aLocal->type()->tag());
            if (!cache_reg.is_illegal()) {
              assert (cache_reg.is_word(), "only words can be cached");
              add_register_oop(cache_reg);
              // these registers will be added to the map by add_registers_to_oop_map
            } else {
              OptoReg::Name rn = frame_map()->single_word_regname(local_name);
              map2->set_oop(rn, frame_size, arg_count);
            }
          }
          // FIXME
          /*
            else if (aLocal != NULL && (aLocal->use_count() == 0)) {
            tty->print_cr("Skipped inlined local");
            }
          */
        }

        //        tty->cr();
      }
    }

#ifndef PRODUCT
    if (VerifyLIROopMaps) {
      // Compare oop map to that generated by looking at the LIR and
      // assert that they are equal

      if (!oop_maps_equal(map, map2)) {
        tty->print_cr("Oop maps generated by CI and from LIR not equal.");
        IRScope* tmp_scope = this->scope();
        int tmp_bci = this->bci();
        tty->print_cr("Method: ");
        while (tmp_scope != NULL) {
          tty->print_cr("  %s.%s%s @ %d",
                        tmp_scope->method()->holder()->name()->as_utf8(),
                        tmp_scope->method()->name()->as_utf8(),
                        tmp_scope->method()->signature()->as_symbol()->as_utf8(),
                        tmp_bci);
          tmp_bci = tmp_scope->caller_bci();
          tmp_scope = tmp_scope->caller();
        }
        tty->print_cr("Locals marked as oops for LIR oop map construction:");
        tty->print_cr("  (Note, this is a superset of what is in the LIR oop map)");
        int i;
        for (i = 0; i < (int)_lir_oop_map.size(); i++) {
          if (_lir_oop_map.at(i)) tty->print(" Name %d", i);
        }
        tty->cr();
        tty->print_cr("Live locals according to LIR oop map:");
        for (i = 0; i < (int)_lir_oop_map.size(); i++) {
          if (_lir_oop_map.at(i) && local_is_live_in_scope(i)) tty->print(" Name %d", i);
        }
        tty->cr();
        tty->print_cr("Oops from CI (sorted by activation):");
        tmp_scope = this->scope();
        tmp_bci = this->bci();
        while (tmp_scope != NULL) {
          tty->print_cr("  %s.%s%s @ %d",
                        tmp_scope->method()->holder()->name()->as_utf8(),
                        tmp_scope->method()->name()->as_utf8(),
                        tmp_scope->method()->signature()->as_symbol()->as_utf8(),
                        tmp_bci);
          if (tmp_scope->oop_map() != NULL) {
            ciLocalMapIterator oops(tmp_scope->oop_map(), tmp_bci);
            while (!oops.done()) {
              tty->print("L%d ", oops.next_oop_offset());
            }
          }
          tty->cr();
          tmp_bci = tmp_scope->caller_bci();
          tmp_scope = tmp_scope->caller();
        }
        tty->print_cr("LIR oop map:");
        map->print();
        tty->cr();
        tty->print_cr("CI oop map:");
        map2->print();
        tty->cr();

        if (compilation()->method()->has_jsrs()) {
          assert(false, "Oop maps should match");
        } else {
          tty->print_cr("* NOTE: mismatch being treated as warning because of presence of *");
          tty->print_cr("* jsrs; LIR oop map should be correct                            *");
        }
      }
    }
#endif
  }

  if (stack() != NULL) {
    Values locks = stack()->locks();
    for (int i = 0; i < locks.length(); i++) {
      OptoReg::Name rn = frame_map()->monitor_object_regname(i);
      map->set_oop(rn, frame_size, arg_count);
    }
  }

  return map;
}


bool CodeEmitInfo::local_is_live_in_scope(int local_no) {
  IRScope* s = scope();
  while (s != NULL) {
    if (s->local_name_is_live_in_scope(local_no)) {
      return true;
    }
    s = s->caller();
  }
  return false;
}


#ifndef PRODUCT
static bool
oop_map_contains_all_entries(OopMap* om1, OopMap* om2) {
  OopMapStream oms1(om1);
  int num_verified = 0;
  for ( ; !oms1.is_done(); oms1.next() ) {
    OopMapValue val1 = oms1.current();
    assert(val1.is_oop(), "Should only find oop values in oop map");
    assert(!val1.is_callee_saved(), "Should not find callee saved registers in C1 methods' oop maps");
    bool found = false;
    OopMapStream oms2(om2);
    for ( ; !oms2.is_done(); oms2.next() ) {
      OopMapValue val2 = oms2.current();
      assert(val2.is_oop(), "Should only find oop values in oop map (2)");
      assert(!val2.is_callee_saved(), "Should not find callee saved registers in C1 methods' oop maps (2)");
      if (val1.is_register_loc() && val2.is_register_loc()) {
        if (val1.reg() == val2.reg()) {
          found = true;
          break;
        }
      } else if (val1.is_stack_loc() && val2.is_stack_loc()) {
        if (val1.stack_offset() == val2.stack_offset()) {
          found = true;
          break;
        }
      }
    }
    if (!found) {
      return false;
    }
    ++num_verified;
  }
  // FIXME
  //  tty->print_cr("Verified %d oop map entries", num_verified);
  return true;
}

bool CodeEmitInfo::oop_maps_equal(OopMap* om1, OopMap* om2) {
  return (oop_map_contains_all_entries(om1, om2) &&
          oop_map_contains_all_entries(om2, om1));
}
#endif


// Inside native stub all oops are on stack
OopMap* CodeEmitInfo::create_oop_map_inside_natives() {
#ifndef SPARC
  // c1/i486 doesn't use FrameMap::calling_convention yet
  ShouldNotReachHere();
#endif
  int frame_size = frame_map()->framesize();
  int arg_count = frame_map()->oop_map_arg_count();
  OopMap* map = new OopMap(frame_size, arg_count);
  record_spilled_oops(frame_map(), map);

  ciSignature* sig = method()->signature();
  CallingConvention* args = FrameMap::calling_convention(method());
  // Note that sig has one entry for double-word, but args have two entries
  // Note: sig does NOT contain the receiver, args contain the receiver
  // the receiver first
  int slot_ix = 0;
  // signature oops
  if (!method()->is_static()) {
    OptoReg::Name rn = frame_map()->single_word_regname(FrameMap::name_for_argument(0));
    map->set_oop(rn, frame_size, arg_count);
    slot_ix++;
  }
  for (int i = 0; i < sig->count(); i++) {
    if (sig->type_at(i)->basic_type() == T_OBJECT || sig->type_at(i)->basic_type() == T_ARRAY) {
      OptoReg::Name rn = frame_map()->single_word_regname(FrameMap::name_for_argument(slot_ix));
      map->set_oop(rn, frame_size, arg_count);
    }
    // Using this conditional expression hits a solaris-X86 compiler bug
    // slot_ix += sig->type_at(i)->is_two_word() ? 2 : 1;
    slot_ix += sig->type_at(i)->size();
    assert( sig->type_at(i)->size() == 1 || sig->type_at(i)->size() == 2, "Invalid type input to size(), must return 1 or 2");
  }
  if (method()->is_synchronized()) {
    OptoReg::Name rn = frame_map()->monitor_object_regname(0);
    map->set_oop(rn, frame_size, arg_count);
  }
  return map;
}


// Use calling convention to specify oop
OopMap* CodeEmitInfo::create_oop_map_for_own_signature() {
  IRScope* scope = this->scope();
  int frame_size = frame_map()->framesize();
  int arg_count = frame_map()->oop_map_arg_count();
  OopMap* map = new OopMap(frame_size, arg_count);
  record_spilled_oops(frame_map(), map);

  scope->clear_local_oop_map();
#ifdef SPARC
  ciSignature* sig = method()->signature();
  CallingConvention* args = FrameMap::calling_convention(method());
  // Note that sig has one entry for double-word, but args have two entries
  // Note: sig does NOT contain the receiver, args contain the receiver
  int slot_ix = 0;
  // the receiver first
  if (!method()->is_static()) {
    ArgumentLocation loc = args->arg_at(slot_ix);
    OptoReg::Name rn;
    if (loc.is_register_arg()) {
      rn = FrameMap::cpu_regname(loc.incoming_reg_location());
    } else {
      assert(loc.is_stack_arg(), "invalid mapping of args");
      rn = frame_map()->single_word_regname(FrameMap::name_for_argument(slot_ix));
    }
    map->set_oop(rn, frame_size, arg_count);
    scope->set_local_is_oop(slot_ix);
    slot_ix = 1;
  }
  for (int n = 0; n < sig->count(); n++) {
    if (!sig->type_at(n)->is_primitive_type()) {
      ArgumentLocation loc = args->arg_at(slot_ix);
      OptoReg::Name rn;
      if (loc.is_register_arg()) {
        rn = FrameMap::cpu_regname(loc.incoming_reg_location());
      } else {
        assert(loc.is_stack_arg(), "invalid mapping of args");
        rn = frame_map()->single_word_regname(FrameMap::name_for_argument(slot_ix));
      }
      map->set_oop(rn, frame_size, arg_count);
      scope->set_local_is_oop(slot_ix);
    }
    // Using this conditional expression hits a solaris-X86 compiler bug
    // slot_ix += sig->type_at(n)->is_two_word() ? 2 : 1;
    slot_ix += sig->type_at(n)->size();
    assert( sig->type_at(n)->size() == 1 || sig->type_at(n)->size() == 2, "Invalid type input to size(), must return 1 or 2");
  }
#else
  NEEDS_CLEANUP; // we need to extend c1/x86 to use FrameMap::calling_convention
  int slot_ix = 0;
  // signature oops
  if (!method()->is_static()) {
    OptoReg::Name rn = frame_map()->single_word_regname(FrameMap::name_for_argument(0));
    map->set_oop(rn, frame_size, arg_count);
    scope->set_local_is_oop(0);
    slot_ix++;
  }
  ciSignature* sig = method()->signature();
  for (int i = 0; i < sig->count(); i++) {
    if (sig->type_at(i)->basic_type() == T_OBJECT || sig->type_at(i)->basic_type() == T_ARRAY) {
      OptoReg::Name rn = frame_map()->single_word_regname(FrameMap::name_for_argument(slot_ix));
      map->set_oop(rn, frame_size, arg_count);
      scope->set_local_is_oop(slot_ix);
    }
    // Using this conditional expression hits a solaris-X86 compiler bug
    // slot_ix += sig->type_at(i)->is_two_word() ? 2 : 1;
    slot_ix += sig->type_at(i)->size();
    assert( sig->type_at(i)->size() == 1 || sig->type_at(i)->size() == 2, "Invalid type input to size(), must return 1 or 2");
  }
#endif
  if (method()->is_synchronized()) {
    OptoReg::Name rn = frame_map()->monitor_object_regname(0);
    map->set_oop(rn, frame_size, arg_count);
  }
  return map;
}


void CodeEmitInfo::add_register_oop(RInfo reg) {
  assert(reg.is_valid(), "must be");
  if (_register_oops == NULL) {
    _register_oops = new RInfoCollection();
  }
  if (!_register_oops->contains(reg)) {
    _register_oops->append(reg);
    if (_oop_map != NULL) {
      int frame_size = frame_map()->framesize();
      int arg_count = frame_map()->oop_map_arg_count();
      OptoReg::Name rn = FrameMap::cpu_regname(reg);
      _oop_map->set_oop(rn, frame_size, arg_count);
    }
  }
}


void CodeEmitInfo::add_registers_to_oop_map(OopMap* map) {
  const RInfoCollection* reg_oops = register_oops();
  assert(reg_oops != NULL, "null check");
  int frame_size = frame_map()->framesize();
  int arg_count = frame_map()->oop_map_arg_count();
  for (int i = 0; i < reg_oops->length(); i++) {
    RInfo rinfo = reg_oops->at(i);
    OptoReg::Name rn = FrameMap::cpu_regname(rinfo);
    map->set_oop(rn, frame_size, arg_count);
  }
}


// create a harmless constant
static ScopeValue* dummy_location(ValueTag tag) {
  switch (tag) {
    case objectTag: return new ConstantOopWriteValue(NULL);
    case addressTag:
    case intTag:    // fall through
    case floatTag:  // fall through
    case longTag:   // fall through
    case doubleTag: return new ConstantIntValue(0);
    default:        ShouldNotReachHere();
  }
  return NULL;
}


Location CodeEmitInfo::location_for_monitor_lock_index(int monitor_index) {
  Location loc;
  if (!frame_map()->location_for_monitor_lock_index(monitor_index, &loc)) {
    compilation()->bailout("too large frame");
  }
  return loc;
}


Location CodeEmitInfo::location_for_monitor_object_index(int monitor_index) {
  Location loc;
  if (!frame_map()->location_for_monitor_object_index(monitor_index, &loc)) {
    compilation()->bailout("too large frame");
  }
  return loc;
}


Location CodeEmitInfo::location_for_name(int name, Location::Type loc_type,
                                         bool is_two_word, bool for_hi_word) {
  Location loc;
  if (!frame_map()->location_for_name(name, loc_type, &loc, is_two_word, for_hi_word)) {
    compilation()->bailout("too large frame");
  }
  return loc;
}


Location CodeEmitInfo::location_for_local_offset(int local_offset, Location::Type loc_type) {
  Location loc;
  if (!frame_map()->location_for_local_offset(local_offset, loc_type, &loc)) {
    compilation()->bailout("too large frame");
  }
  return loc;
}


void CodeEmitInfo::scope_value_for_register(RInfo reg, ScopeValue** first, ScopeValue** second, Location::Type loc_type) {
  NEEDS_CLEANUP; // access to cpu/fpu register numbers should be symmetric
  // clear out the secondary result so that garbage isn't returned
  if (second != NULL) {
    *second = NULL;
  }
  if (reg.is_word()) {
    OptoReg::Name rname = FrameMap::cpu_regname(reg);
    *first = new LocationValue(Location::new_reg_loc(loc_type, rname));
  } else if (reg.is_float()) {
    OptoReg::Name rname = frame_map()->fpu_regname(reg.fpu_regnr());
    *first = new LocationValue(Location::new_reg_loc(loc_type, rname));
  } else if (reg.is_double()) {
    // On SPARC, fpu_regnrLo/fpu_regnrHi represents the two halves of
    // the double as float registers in the native ordering. On IA32,
    // fpu_regnrLo is a FPU stack slot whose OptoReg::Name represents
    // the low-order word of the double and fpu_regnrLo + 1 is the
    // name for the other half.  *first and *second must represent the
    // least and most significant words, respectively.
    OptoReg::Name rname_lo = frame_map()->fpu_regname(reg.fpu_regnrLo());
#ifdef IA32
    OptoReg::Name rname_hi = OptoReg::add(rname_lo, 1);
#endif
#ifdef SPARC
    OptoReg::Name rname_hi = frame_map()->fpu_regname(reg.fpu_regnrHi());
#endif
    if (first_java_word == native_hi_word) {
      // IA32 (little endian)
      *first = new LocationValue(Location::new_reg_loc(loc_type, rname_lo));
      *second = new LocationValue(Location::new_reg_loc(loc_type, rname_hi));
    } else {
      // SPARC (big endian)
      *first = new LocationValue(Location::new_reg_loc(loc_type, rname_hi));
      *second = new LocationValue(Location::new_reg_loc(loc_type, rname_lo));
    }
  } else if (reg.is_long()) {
    OptoReg::Name rname_lo = FrameMap::cpu_regname(reg.as_register_lo()->encoding());
    OptoReg::Name rname_hi = FrameMap::cpu_regname(reg.as_register_hi()->encoding());
    *first = new LocationValue(Location::new_reg_loc(loc_type, rname_lo));
    *second = new LocationValue(Location::new_reg_loc(loc_type, rname_hi));
  } else {
    ShouldNotReachHere();
  }
}

ScopeValue* CodeEmitInfo::scope_value_for_local_offset(int local_offset, Location::Type loc_type, ScopeValue** second) {
  assert(local_offset >= 0, "illegal offset");
  ScopeValue* scope_value;
  RInfo reg = get_cache_reg_for_local_offset(local_offset);
  if (local_offset >= frame_map()->size_locals()) {
    assert(!reg.is_valid(), "shouldn't have cached in reg");
    // frame map may have less locals than method specifies; this occurs if
    // the local at local_offset is not used, therefore the type is not important, so use intTag
    scope_value = dummy_location(intTag);
  } else if (reg.is_illegal()) {
    scope_value = new LocationValue(location_for_local_offset(local_offset, loc_type));
  } else {
    scope_value_for_register(reg, &scope_value, second, loc_type);
  }
  return scope_value;
}

IRScopeDebugInfo* CodeEmitInfo::compute_debug_info_for_scope(IRScope* scope, int bci,
                                                             GrowableArray<ScopeValue*>* stack, int stack_end, Values locks, int locks_end) {
  IRScopeDebugInfo* caller = NULL;
  int stack_begin, locks_begin;
  ValueStack* caller_state = scope->caller_state();
  if (caller_state != NULL) {
    // process recursively to compute outermost scope first
    stack_begin = caller_state->stack_size();
    locks_begin = caller_state->locks_size();
    caller = compute_debug_info_for_scope(scope->caller(), scope->caller_bci(), stack, stack_begin, locks, locks_begin);
  } else {
    stack_begin = 0;
    locks_begin = 0;
  }
  // initialize these to null.
  // If we don't need deopt info or there are no locals, expressions or monitors,
  // then these get recorded as no information and avoids the allocation of 0 length arrays.
  GrowableArray<ScopeValue*>*   locals      = NULL;
  GrowableArray<ScopeValue*>*   expressions = NULL;
  GrowableArray<MonitorValue*>* monitors    = NULL;

  if (DeoptC1 && compilation()->needs_debug_information()) {
    int i;
    // describe local variable values
    int nof_locals = scope->method()->max_locals();
    if (nof_locals > 0) {
      locals = new GrowableArray<ScopeValue*>(nof_locals);
      // If this is an exception site which isn't really a GC point, generate dummy debug info for all locals
      if (bci != SynchronizationEntryBCI &&
          !ciGenerateLocalMap::bytecode_is_gc_point(scope->method()->java_code_at_bci(bci),
                                                    scope->method()->has_exception_handlers(),
                                                    scope->method()->is_synchronized())) {
        for (int index = 0; index < nof_locals; index++) {
          locals->append(dummy_location(intTag));
        }
      } else {
        for (int index = 0; index < nof_locals; index++) {
          int offset = in_words(scope->offset_for_local_index(index));
          Location::Type loc_type = scope->local_is_oop(index) ? Location::oop : Location::normal;

          if (scope->local_is_oop(index)) {
            // if that local was not used, it will not have been marked as oop in the
            // oop maps and therefore may contain illegal value; put a NULL value
            // instead; this can cause problems for JVMDI, where the receiver becomes invisible.
            Local* aLocal = scope->local_at(objectType, index);
            if (aLocal != NULL && (scope == compilation()->hir()->top_scope() || aLocal->use_count() > 0)) {
              assert(offset != IRScope::BAD_LOCAL_OFFSET, "should have an offset assigned");
              locals->append(scope_value_for_local_offset(offset, loc_type, NULL));
            } else {
              // this local does not exist
              locals->append(dummy_location(objectTag));
            }
          } else {
            if (offset != IRScope::BAD_LOCAL_OFFSET) {
              ScopeValue* second = NULL;
              locals->append(scope_value_for_local_offset(offset, loc_type, &second));
              if (second != NULL) {
                // this local is a two word local that's stored in a register
                // so increment the index to the following local.
                index++;
                locals->append(second);
              }
            } else {
              // this local does not exist
              locals->append(dummy_location(intTag));
            }
          }
        }
      }
    }

    // When we inline methods containing exception handlers, the
    // "lock_stacks" are changed to preserve expression stack values
    // in caller scopes when exception handlers are present. This
    // can cause callee stacks to be smaller than caller stacks.
    if (stack != NULL &&
        stack_begin < stack_end &&
        stack_begin <= stack->length() &&
        stack_end   <= stack->length()) {
      // describe expression stack values for this scope (already computed by
      // lir_stack2value_stack(_stack) for the entire stack)
      expressions = new GrowableArray<ScopeValue*>(stack_end - stack_begin);
      for (i = stack_begin; i < stack_end; i++) {
        expressions->append(stack->at(i));
      }
    }

    // describe monitors
    assert(locks_begin <= locks_end, "error in scope iteration");
    int nof_locks = locks_end - locks_begin;
    if (nof_locks > 0) {
      monitors = new GrowableArray<MonitorValue*>(nof_locks);
      for (i = locks_begin; i < locks_end; i++) {
        ScopeValue* object_scope_value = new LocationValue(location_for_monitor_object_index(i));
        Location lock_location = location_for_monitor_lock_index(i);
        monitors->append(new MonitorValue(object_scope_value, lock_location));
      }
    }
  }
  assert(locals == NULL || locals->length() <= scope->method()->max_locals(), "described too many locals");
  return new IRScopeDebugInfo(scope, bci, locals, expressions, monitors, caller);
}


Location::Type CodeEmitInfo::opr2location_type(LIR_Opr opr) {
  if (opr->is_oop()) {
    return Location::oop;
  } else if (opr->is_illegal()) {
    return Location::invalid;
  } else if (opr->is_register() && opr->is_single_fpu() && float_saved_as_double) {
    return Location::float_in_dbl;
  } else {
    return Location::normal;
  }
}


void CodeEmitInfo::append_scope_value(LIR_Opr opr, GrowableArray<ScopeValue*>* expressions) {
  // Note: For patching or backward branch safepoints, caller-save registers will be spilled
  // in the patching or illegal instruction handler stub and their spill locations recorded
  // as callee-saved OopMapValues.  These locations are transfered to a RegisterMap for the
  // frame during StackFrameStream traversal by OopMapSet::update_register_map().
  Location::Type loc_type = opr2location_type(opr);
  if (opr->is_single_word() || (opr->is_constant() && type2size[opr->as_constant_ptr()->type()] == 1)) {
    ScopeValue* sv = NULL;
    if (opr->is_register()) {
      scope_value_for_register(opr->rinfo(), &sv, NULL, loc_type);
    } else if (opr->is_stack()) {
      sv = new LocationValue(location_for_name(opr->single_stack_ix(), loc_type));
    } else if (opr->is_constant()) {
      LIR_Const* c = opr->as_constant_ptr();
      BasicType t = c->type();
      switch (t) {
        case T_INT:    sv = new ConstantIntValue(c->as_jint()); break;
        case T_OBJECT: sv = new ConstantOopWriteValue(c->as_jobject()); break;
        case T_FLOAT:  sv = new ConstantIntValue(jint_cast(c->as_jfloat()));break;
        default: ShouldNotReachHere();
      }
    } else {
      ShouldNotReachHere();
    }
    assert(sv != NULL, "should be set");
    expressions->append(sv);
  } else {
    assert(opr->is_double_word() || (opr->is_constant() && type2size[opr->as_constant_ptr()->type()] == 2), "should be two words");
    ScopeValue* sv_hi = NULL;
    ScopeValue* sv_lo = NULL;
    if (opr->is_register()) {
      scope_value_for_register(opr->rinfo(), &sv_lo, &sv_hi, loc_type);
    } else if (opr->is_stack()) {
      // Lo word
      sv_lo = new LocationValue(location_for_name(opr->double_stack_ix(), loc_type, true, false));
      // Hi word
      sv_hi = new LocationValue(location_for_name(opr->double_stack_ix(), loc_type, true, true));
    } else if (opr->is_constant()) {
      LIR_Const* c = opr->as_constant_ptr();
      BasicType t = c->type();
      jlong tmp;
      switch (t) {
        case T_DOUBLE: tmp = jlong_cast(c->as_jdouble()); break;
        case T_LONG:   tmp = c->as_jlong(); break;
        default: ShouldNotReachHere();
      }
      sv_lo = new ConstantIntValue(low(tmp));
      sv_hi = new ConstantIntValue(high(tmp));
    } else {
      ShouldNotReachHere();
    }
    assert(sv_lo != NULL, "should be set");
    assert(sv_hi != NULL, "should be set");
    if (first_java_word == native_hi_word) {
      expressions->append(sv_hi);
      expressions->append(sv_lo);
    } else {
      expressions->append(sv_lo);
      expressions->append(sv_hi);
    }
  }
}


GrowableArray<ScopeValue*>* CodeEmitInfo::lir_stack2value_stack(GrowableArray<LIR_Opr>* lir_stack) {
  if (lir_stack == NULL) return NULL;
  int lng = lir_stack->length();
  GrowableArray<ScopeValue*>* expressions = new GrowableArray<ScopeValue*>();
  for (int i = 0; i < lng; i++) {
    append_scope_value(lir_stack->at(i), expressions);
  }
  return expressions;
}


//--------------------------------------------------------

void LIR_Emitter::set_bailout(const char* msg) {
  if (PrintBailouts && !_bailout) { // print only first time
    tty->print_cr("LIR_Emitter bailout reason: %s", msg);
  }
  _bailout = true;
}


bool LIR_Emitter::must_bailout() const {
  return _bailout;
}


// The local_index and local_index+1 addresses must be adjacent on stack
// (no link or return address may be in between)
void LIR_Emitter::check_double_address(int local_index) {
  if (!frame_map()->are_adjacent_indeces(local_index, local_index+1)) {
    set_bailout("double elements not adjacent" );
  }
}


ciMethod* LIR_Emitter::method() const {
  return compilation()->method();
}


//---------------------------------------------------------------


jint LIR_Emitter::opr2int(LIR_Opr item) {
  assert(item->is_constant(), "item is not an int constant");
  jint res;
  switch(item->type()) {
    case T_INT     : res = item->as_jint(); break;
    case T_FLOAT   :
      {
        // Note: do not change the following code "style"; without
        // explicit assignment, VC 5.0 generates wrong code in the release
        // version
        jfloat f = item->as_jfloat();
        res = jint_cast(f);
      }
      break;
    default: ShouldNotReachHere();
  }
  return res;
}


jlong LIR_Emitter::opr2long(LIR_Opr item) {
  assert(item->type() == T_LONG || item->type() == T_DOUBLE, "wrong item");
// [RGV] Do not change the order of this code.  It will
// fail on x86 Solaris SC5.0 compilers.  It failed if optimized
// when I had these two statements inside the else case below:
//     jdouble d = item->as_jdouble();
//     jlong   l = jlong_cast(d);
  jdouble d;
  jlong l;

  if (item->type() == T_DOUBLE) d = item->as_jdouble();

  if (item->type() == T_LONG) {
    l  = item->as_jlong();
    return (l);
  } else {
    l = jlong_cast(d);
    return (l);
  }
}


jint LIR_Emitter::opr2intLo(LIR_Opr item) {
  assert(item->is_constant(), "wrong item");
  jint res;
  if (item->type() == T_LONG) {
    res = low(item->as_jlong());
  } else if (item->type() == T_DOUBLE) {
    // Note: do not change the following code "style"; without
    // explicit assignment, VC 5.0 generates wrong code in the release
    // version
    jdouble d = item->as_jdouble();
    jlong   l = jlong_cast(d);
    res = low(l);
  } else {
    ShouldNotReachHere();
  }
  return res;
}


jint LIR_Emitter::opr2intHi(LIR_Opr item) {
  assert(item->is_constant(), "wrong item");
  jint res;
  if (item->type() == T_LONG) {
    // Note: do not change the following code "style"; without
    // explicit assignment, VC 5.0 generates wrong code in the release
    // version
    jlong l = item->as_jlong();
    res = high(l);
  } else if (item->type() == T_DOUBLE) {
    // Note: do not change the following code "style"; without
    // explicit assignment, VC 5.0 generates wrong code in the release
    // version
    jdouble d = item->as_jdouble();
    jlong   l = jlong_cast(d);
    res = high(l);
  } else {
    ShouldNotReachHere();
  }
  return res;
}


jobject LIR_Emitter::opr2jobject(LIR_Opr item) {
  assert(item->is_constant() && item->value_type()->is_object(), "item is not an object constant");
  return item->as_jobject();
}


void LIR_Emitter::jobject2reg_with_patching(RInfo r, ciObject* obj, CodeEmitInfo* info) {
  if (!obj->is_loaded() || PatchALot) {
    assert(info != NULL, "info must be set if class is not loaded");
    __ oop2reg_patch(NULL, r, info);
  } else {
    // no patching needed
    __ oop2reg(obj->encoding(), r);
  }
}


// Pack a long into a pointer
// in 64 bit mode, this means packing 2 registers into 1.
// In both 32 and 64 bit modes, we return the pointer in
// the low register
RInfo LIR_Emitter::long2address(LIR_Opr item) {
#ifdef _LP64
  RInfo reg_lo = item->as_rinfo_lo();
  RInfo reg_hi = item->as_rinfo_hi();
  __ shift_left_long(reg_hi, 32, reg_hi);
  __ shift_right(reg_lo, 0, reg_lo);
  __ logical_or(reg_hi, reg_lo, reg_lo);
  return (reg_lo);		// Return the 64 bit pointer in low register
#else
  return (item->as_rinfo_lo()); 	// Use only the low 32 bits
#endif
}


//----------------------------------------
LIR_Emitter::LIR_Emitter(Compilation* compilation)
  : _frame_map(compilation->frame_map())
  , _bailout(false)
  , _lir(NULL)
  , _compilation(compilation) {
}


void LIR_Emitter::start_block(BlockBegin* bb) {
  assert(bb->lir() == NULL, "LIR list already computed for this block");
  _lir = new LIR_List(compilation());
  bb->set_lir(_lir);
}

C1_MacroAssembler* LIR_Emitter::masm() const { return _compilation->masm(); }

// call if an instruction implicitly sets an FP result (sets fpu_stack)
void LIR_Emitter::set_fpu_result      (RInfo reg) {
  // mark stack simulator that register has been pushed
  __ push_fpu(reg);
}


void LIR_Emitter::fpop() {
  __ fpop_raw();
}


void LIR_Emitter::remove_fpu_result   (RInfo reg) {
  __ pop_fpu(reg);
}


// used only by asserts
void LIR_Emitter::set_fpu_stack_empty () { __ set_fpu_stack_empty(); }


void LIR_Emitter::bind_block_entry (BlockBegin* block) {
  Label* label = block->label();
  __ branch_destination(label);

  if (LIRTraceExecution &&
      compilation()->hir()->start()->block_id() != block->block_id()) {
    assert(_lir->instructions_list()->length() == 1, "should come right after br_dst");
    address entry = CAST_FROM_FN_PTR(address, Runtime1::trace_block_entry);
    trace_block_entry(block, entry);
  }
}

#ifndef PRODUCT
void LIR_Emitter::print() { tty->print_cr("LIR_Emitter..."); }
#endif


// ======================== code emission ==================================

void LIR_Emitter::init_local(IRScope* scope, int local_index) {
  Local* aLocal = scope->local_at(objectType, local_index);
  if (aLocal != NULL) {
    // IR builder is more precise: there may be no local for the given index
    int stackPos = aLocal->local_name();
    __ oop2stack((jobject)NULL, stackPos); // this is done only for possible oop locations
  }
}


void LIR_Emitter::std_entry(IRScope* scope, intStack* init_locals, RInfo receiver, RInfo ic_klass) {
  assert(!scope->method()->is_native(), "do not use LIR for natives");
  __ align_entry();
  __ unverified_entry_point(receiver, ic_klass);
  __ verified_entry_point();

  __ build_frame();
  if (init_locals != NULL) {
    int i = init_locals->length();
    while (i-- > 0) init_local(scope, init_locals->at(i));
  }

  CodeEmitInfo* info = method()->is_native() ? new CodeEmitInfo(scope, SynchronizationEntryBCI, NULL)
    : new CodeEmitInfo(this,  SynchronizationEntryBCI, NULL, scope->start()->state(), NULL);

  if (method()->is_synchronized() && GenerateSynchronizationCode) {
    monitorenter_at_entry(receiver, info);
  }

  if (compilation()->jvmpi_event_method_entry_enabled() || compilation()->jvmpi_event_method_entry2_enabled()) {
    __ jvmpi_method_enter(info);
  }

}


void LIR_Emitter::copy_fpu_item (RInfo toReg, LIR_Opr from) {
  RInfo fromReg = from->as_rinfo();
  assert(fromReg.is_float_kind(), "wrong item register");
  __ dup_fpu(fromReg, toReg);
}


void LIR_Emitter::push_item(LIR_Opr item) {
  __ push(item);
}


void LIR_Emitter::opr2local(int local_name, LIR_Opr item) {
  BasicType type = item->type();
  if (item->is_register()) {
    switch (type) {
      case T_ADDRESS: // fall through
      case T_ARRAY  : // fall through
      case T_OBJECT : // fall through
      case T_INT    : // fall through
      case T_FLOAT  :
        {
          int stackIx = local_name;
          __ reg2single_stack(item->as_rinfo(), stackIx, type);
        }
        break;
      case T_DOUBLE :
        { int stackIx = local_name;
          check_double_address(local_name);
          __ reg2double_stack(item->as_rinfo(), stackIx, T_DOUBLE);
        }
        break;
      case T_LONG :
        { int stackIx = local_name;
          check_double_address(local_name);
          __ reg2double_stack(item->as_rinfo(), stackIx, T_LONG);
        }
        break;

      default: ShouldNotReachHere();
    }
  } else if (item->is_constant()) {
    if (type == T_DOUBLE || type == T_LONG) {
      int stackIx = local_name;
      __ long2stack(opr2long(item), stackIx);
    } else if(type == T_OBJECT || type == T_ARRAY) {
      int stackIx = local_name;
      __ oop2stack(opr2jobject(item), stackIx);
    } else {
      int stackIx = local_name;
      __ int2stack(opr2int(item), stackIx);
    }
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Emitter::move(LIR_Opr item, RInfo reg) {
  BasicType type = item->type();

  bool is_item_fp = type == T_FLOAT || type == T_DOUBLE;
  bool is_reg_fp  = reg.is_float() || reg.is_double();

  if (is_item_fp != is_reg_fp) {
    assert(item->is_stack(), "can only do untyped load from the stack");
    Unimplemented();
    return;
  }


  bool is_double_word = type == T_DOUBLE || type == T_LONG;
  if (item->is_register()) {
    switch (type) {
      case T_ADDRESS: // fall through
      case T_INT    : // fall through
      case T_ARRAY  : // fall through
      case T_OBJECT : // fall through
      case T_FLOAT  : // fall through
      case T_DOUBLE : {
        // it's possible that this is already in the right location.
        RInfo from = item->as_rinfo();
        if (!from.is_same(reg)) {
          __ reg2reg(from, reg, type);
        }
        break;
      }

      case T_LONG   :
        {
          if (item->as_rinfo_hi().is_same(reg.as_rinfo_lo())) {
	    __ reg2reg(item->as_rinfo_hi(), reg.as_rinfo_hi(), T_INT);
	    __ reg2reg(item->as_rinfo_lo(), reg.as_rinfo_lo(), T_INT);
          } else {
	    __ reg2reg(item->as_rinfo_lo(), reg.as_rinfo_lo(), T_INT);
	    __ reg2reg(item->as_rinfo_hi(), reg.as_rinfo_hi(), T_INT);
          }
        }
        break;
      default: ShouldNotReachHere();
    }
  } else if (item->is_constant()) {
    if (type == T_LONG) {
      __ int2reg(opr2intLo(item), reg.as_rinfo_lo());
      __ int2reg(opr2intHi(item), reg.as_rinfo_hi());
    } else if(type == T_OBJECT || type == T_ARRAY) {
      __ oop2reg(opr2jobject(item), reg);
    } else if (type == T_DOUBLE) {
      __ double2reg(item->as_jdouble(), reg);
    } else if(type == T_FLOAT) {
      __ float2reg(item->as_jfloat(), reg);
    } else {
      __ int2reg(opr2int(item), reg);
    }
  } else if (item->is_stack()) {
    __ move(item, LIR_OprFact::rinfo(reg, type));
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Emitter::round(int spill_ix, LIR_Opr item) {
  assert(item->is_register() , "only a registercan be spilled");
  assert(item->value_type()->is_float(), "rounding only for floats available");
  int stackIx = frame_map()->spill_name(spill_ix);
  __ round32bit(item->as_rinfo(), stackIx);
}


void LIR_Emitter::spill(int spill_ix, LIR_Opr item) {
  assert(item->is_register() , "only a registercan be spilled");

  switch (item->value_type()->tag()) {
    case addressTag: // fall through
    case intTag    : // fall through
    case objectTag : // fall through
    case floatTag  :
      {
  	BasicType t = item->type();
        int stackIx = frame_map()->spill_name(spill_ix);
        __ reg2single_stack(item->as_rinfo(), stackIx, t);
      }
      break;

    case doubleTag :
      {
        int stackIx = frame_map()->spill_name(spill_ix);
        __ reg2double_stack(item->as_rinfo(), stackIx, T_DOUBLE);
      }
      break;

    case longTag :
      {
        int stackIx = frame_map()->spill_name(spill_ix);
        __ reg2double_stack(item->as_rinfo(), stackIx, T_LONG);
      }
      break;

    default: ShouldNotReachHere();
  }
}


void LIR_Emitter::move_spill    (int to_spill_ix, int from_spill_ix, ValueType* type, RInfo tmp) {
  switch (type->tag()) {
    case addressTag: // fall through
    case intTag    : // fall through
    case objectTag : // fall through
    case floatTag  :
      {
	BasicType t = as_BasicType(type);
        int fromStackIx = frame_map()->spill_name(from_spill_ix);
        int toStackIx   = frame_map()->spill_name(to_spill_ix);
        __ single_stack2reg(fromStackIx, tmp, t);
        __ reg2single_stack(tmp, toStackIx, t);
      }
      break;
    case doubleTag :
      { int fromStackIx = frame_map()->spill_name(from_spill_ix);
        int toStackIx   = frame_map()->spill_name(to_spill_ix);
        __ double_stack2reg(fromStackIx, tmp, T_DOUBLE);
        __ reg2double_stack(tmp, toStackIx, T_DOUBLE);
      }
      break;
    case longTag :
      { int fromStackIx = frame_map()->spill_name(from_spill_ix);
        int toStackIx   = frame_map()->spill_name(to_spill_ix);
        __ double_stack2reg(fromStackIx, tmp, T_LONG);
        __ reg2double_stack(tmp, toStackIx, T_LONG);
      }
      break;

    default: ShouldNotReachHere();
  }
}


void LIR_Emitter::null_check (LIR_Opr item, CodeEmitInfo* info) {
  __ null_check(item->as_rinfo(), info);
}


void LIR_Emitter::explicit_div_by_zero_check (LIR_Opr item, CodeEmitInfo* info) {
  if (item->is_register()) {
    if (item->type() == T_LONG) {
      __ logical_orcc(item->as_rinfo_lo(), item->as_rinfo_hi(), item->as_rinfo_lo());
    } else {
      Unimplemented();
    }
    DivByZeroStub* stub = new DivByZeroStub(info);
    __ branch(LIR_OpBranch::equal, stub);
  } else if (item->is_constant()) {
    if (item->type() == T_LONG) {
      if (opr2intLo(item) == 0 && opr2intHi(item) == 0) {
        DivByZeroStub* stub = new DivByZeroStub(info);
        __ branch(LIR_OpBranch::always, stub);
      }
    } else {
      Unimplemented();
    }
  } else {
    Unimplemented();
  }
}


void LIR_Emitter::array_range_check(LIR_Opr array, LIR_Opr index,
                                    CodeEmitInfo* null_check_info, CodeEmitInfo* range_check_info) { 
  if (index->is_constant()) {
    int index_value = opr2int(index);
    CodeStub* stub = new RangeCheckStub(range_check_info, norinfo, index_value);
    cmp_mem_int(LIR_OpBranch::belowEqual, array->as_rinfo(),
                arrayOopDesc::length_offset_in_bytes(), index_value, null_check_info);
    __ branch(LIR_OpBranch::belowEqual, stub); // forward branch
  } else {
    RInfo index_rinfo = index->as_rinfo();
    CodeStub* stub = new RangeCheckStub(range_check_info, index_rinfo, 0);
    cmp_reg_mem(LIR_OpBranch::aboveEqual, index_rinfo, array->as_rinfo(),
                arrayOopDesc::length_offset_in_bytes(), T_INT, null_check_info);
    __ branch(LIR_OpBranch::aboveEqual, stub); // forward branch
  }
}


void LIR_Emitter::length_range_check(LIR_Opr length, LIR_Opr index, CodeEmitInfo* range_check_info) { 
  CodeStub* stub = NULL;
  if (index->is_constant()) {
    stub = new RangeCheckStub(range_check_info, norinfo, index->as_jint());
  } else {
    stub = new RangeCheckStub(range_check_info, index->as_rinfo(), 0);
  }
  __ cmp(LIR_OpBranch::belowEqual, length, index);
  __ branch(LIR_OpBranch::belowEqual, stub);
}


void LIR_Emitter::array_store_check(LIR_Opr array, LIR_Opr value, RInfo k_RInfo, RInfo klass_RInfo, RInfo Rtmp1, CodeEmitInfo* info_for_exception) {
  __ store_check(value, array, LIR_OprFact::rinfo(k_RInfo), LIR_OprFact::rinfo(klass_RInfo), LIR_OprFact::rinfo(Rtmp1), info_for_exception);
}


void LIR_Emitter::nio_range_check(LIR_Opr buffer, LIR_Opr index, RInfo result, CodeEmitInfo* info) {
  if (index->is_constant()) {
    int index_value = opr2int(index);
    CodeStub* stub = new RangeCheckStub(info, norinfo, index_value, true);
    cmp_mem_int(LIR_OpBranch::belowEqual, buffer->as_rinfo(), java_nio_Buffer::limit_offset(), index_value, info);
    __ branch(LIR_OpBranch::belowEqual, stub); // forward branch
    __ int2reg(index_value, result);
  } else {
    RInfo index_rinfo = index->as_rinfo();
    CodeStub* stub = new RangeCheckStub(info, index_rinfo, 0, true);
    cmp_reg_mem(LIR_OpBranch::aboveEqual, index_rinfo, buffer->as_rinfo(), java_nio_Buffer::limit_offset(), T_INT, info);
    __ branch(LIR_OpBranch::aboveEqual, stub); // forward branch
    __ reg2reg(index_rinfo, result, index->type());
  }
}


void LIR_Emitter::array_length(RInfo dst, LIR_Opr array, CodeEmitInfo* info) {
  __ load_mem_reg(array->as_rinfo(), arrayOopDesc::length_offset_in_bytes(), dst, T_INT, info, LIR_Op1::patch_none);
}


void LIR_Emitter::field_load(RInfo dst, ciField* field, LIR_Opr object, int offset_in_bytes, bool needs_patching, bool is_loaded, CodeEmitInfo* info) {
  assert(object->value_type()->is_object(), "not an object");
  offset_in_bytes = needs_patching ? max_jint : offset_in_bytes;
  assert(offset_in_bytes >= 0 || needs_patching, "wrong offset");
  BasicType dst_type = field->type()->basic_type();

  if (dst_type == T_LONG) {
    RInfo objR = object->as_rinfo();
    if (field->is_volatile()) {
      LIR_Op1::LIR_PatchCode patch_code = needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none;
      __ volatile_load_mem_reg(object->as_rinfo(), offset_in_bytes, dst, dst_type, info, patch_code);
    } else {
      RInfo rlo = dst.as_rinfo_lo();
      RInfo rhi = dst.as_rinfo_hi();
      assert(!rlo.is_same(rhi), "error in regalloc");
      LIR_Op1::LIR_PatchCode codeLo = needs_patching ? LIR_Op1::patch_low : LIR_Op1::patch_none;
      LIR_Op1::LIR_PatchCode codeHi = needs_patching ? LIR_Op1::patch_high : LIR_Op1::patch_none;
      int offsetLo = needs_patching ? max_jint : offset_in_bytes + lo_word_offset_in_bytes();
      int offsetHi = needs_patching ? max_jint : offset_in_bytes + hi_word_offset_in_bytes();
      if (objR.is_same(rlo)) {
        // destroy hi register first
        __ load_mem_reg(object->as_rinfo(), offsetHi, dst.as_rinfo_hi(), T_INT, info, codeHi);
        __ load_mem_reg(object->as_rinfo(), offsetLo, dst.as_rinfo_lo(), T_INT, info, codeLo);
      } else {
        __ load_mem_reg(object->as_rinfo(), offsetLo, dst.as_rinfo_lo(), T_INT, info, codeLo);
        __ load_mem_reg(object->as_rinfo(), offsetHi, dst.as_rinfo_hi(), T_INT, info, codeHi);
      }
    }
  } else {
    LIR_Op1::LIR_PatchCode patch_code = needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none;
    __ load_mem_reg(object->as_rinfo(), offset_in_bytes, dst, dst_type, info, patch_code);
  }
}


void LIR_Emitter::field_store_long(LIR_Opr object, int offset_in_bytes, LIR_Opr value, bool needs_patching, CodeEmitInfo* info) {
  LIR_Op1::LIR_PatchCode codeLo = needs_patching ? LIR_Op1::patch_low : LIR_Op1::patch_none;
  LIR_Op1::LIR_PatchCode codeHi = needs_patching ? LIR_Op1::patch_high : LIR_Op1::patch_none;
  int offsetLo = needs_patching ? max_jint : offset_in_bytes + lo_word_offset_in_bytes();
  int offsetHi = needs_patching ? max_jint : offset_in_bytes + hi_word_offset_in_bytes() ;
  if (value->is_constant()) {
    __ store_mem_int(opr2intLo(value), object->as_rinfo(), offsetLo, T_INT, info, codeLo);
    __ store_mem_int(opr2intHi(value), object->as_rinfo(), offsetHi, T_INT, info, codeHi);
  } else if (value->is_register()) {
    __ store_mem_reg(value->as_rinfo_lo(), object->as_rinfo(), offsetLo, T_INT, info, codeLo);
    __ store_mem_reg(value->as_rinfo_hi(), object->as_rinfo(), offsetHi, T_INT, info, codeHi);
  } else {
    Unimplemented();
  }
}


NEEDS_CLEANUP;
// Check float implementation : uses cpu register for transporting fpu values if possible,
// because i486 is slow on floating point registers
// Volatiles note:
void LIR_Emitter::field_store(ciField* field, LIR_Opr object, int offset_in_bytes, LIR_Opr value, bool needs_patching, bool is_loaded, CodeEmitInfo* info, RInfo tmp) {
  assert(object->value_type()->is_object() && object->is_register(), "wrong object type");
  BasicType value_type = field->type()->basic_type();
  bool is_oop = value_type == T_OBJECT || value_type == T_ARRAY;
  offset_in_bytes = needs_patching ? max_jint : offset_in_bytes;
  assert(offset_in_bytes >= 0 || needs_patching, "wrong offset");

  if (value->is_constant()) {
    if (value_type == T_DOUBLE || value_type == T_LONG) {
      field_store_long(object, offset_in_bytes, value, needs_patching, info);
    } else if (value_type == T_OBJECT || value_type == T_ARRAY) {
      LIR_Op1::LIR_PatchCode code = needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none;
      offset_in_bytes = needs_patching ? max_jint : offset_in_bytes;
      __ store_mem_oop(opr2jobject(value), object->as_rinfo(), offset_in_bytes, value_type, info, code);
    } else {
      LIR_Op1::LIR_PatchCode code = needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none;
      offset_in_bytes = needs_patching ? max_jint : offset_in_bytes;
      __ store_mem_int(opr2int(value), object->as_rinfo(), offset_in_bytes, value_type, info, code);
    }
  } else if (value->is_stack()) {
    // local->mem
    assert(!needs_patching, "cannot handle code patching with value on stack");
    if (value_type == T_FLOAT) {
      // use int register to move
      __ move(value, LIR_OprFact::rinfo(tmp));
      __ store_mem_reg(tmp, object->as_rinfo(), offset_in_bytes, T_INT, info, LIR_Op1::patch_none);
    } else if (value_type == T_DOUBLE) {
      // use long register(s) to move
      __ move(value, LIR_OprFact::rinfo(tmp));
      if (field->is_volatile()) {
        __ volatile_store_mem_reg(tmp, object->as_rinfo(), offset_in_bytes, T_LONG, info);
      } else {
        __ store_mem_reg(tmp.as_rinfo_lo(), object->as_rinfo(), offset_in_bytes + lo_word_offset_in_bytes(), T_INT, info, LIR_Op1::patch_none);
        __ store_mem_reg(tmp.as_rinfo_hi(), object->as_rinfo(), offset_in_bytes + hi_word_offset_in_bytes(), T_INT, info, LIR_Op1::patch_none);
      }
    } else {
      Unimplemented();
    }
  } else if (value->is_register()) {
    if (value_type == T_LONG) {
      if (field->is_volatile()) {
        __ volatile_store_mem_reg(value->as_rinfo(), object->as_rinfo(), offset_in_bytes, value_type, info, needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none);
      } else {
        field_store_long(object, offset_in_bytes, value, needs_patching, info);
      }
    } else if (value_type == T_BOOLEAN || value_type == T_BYTE) {
      // machine dependent
      field_store_byte(object, offset_in_bytes, value, tmp, needs_patching, info);
    } else {
      __ store_mem_reg(value->as_rinfo(), object->as_rinfo(), offset_in_bytes, value_type, info, needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none);
    }
    if (value_type == T_ARRAY || value_type == T_OBJECT) {
      write_barrier(object, LIR_OprFact::rinfo(tmp, value_type));
    }
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Emitter::negate (RInfo dst, LIR_Opr value) {
  // XXX TKR this assertion is wrong on sparc.  should we get rid of it?
  // XXX TKR assert(value->destroys_register(), "check");
  __ negate(value->as_rinfo(), dst);
}


void LIR_Emitter::arithmetic_op(Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, bool is_strictfp, RInfo tmp_rinfo, CodeEmitInfo* info) {
  LIR_Opr result_op = result;
  LIR_Opr left_op   = left;
  LIR_Opr right_op  = right;

  switch(code) {
    case Bytecodes::_dadd:
    case Bytecodes::_fadd:
    case Bytecodes::_ladd:
    case Bytecodes::_iadd:  __ add(left_op, right_op, result_op); break;
    case Bytecodes::_fmul:
    case Bytecodes::_lmul:  __ mul(left_op, right_op, result_op); break;

    case Bytecodes::_dmul:
      {
        if (is_strictfp) {
          __ mul_strictfp(left_op, right_op, result_op); break;
        } else {
          __ mul(left_op, right_op, result_op); break;
        }
      }
      break;

    case Bytecodes::_imul:
      {
        LIR_Opr tmp_op       = LIR_OprFact::rinfo(tmp_rinfo, T_INT);
        bool    did_strength_reduce = false;

        if (right->is_constant()) {
          int c = opr2int(right);
          if (is_power_of_2(c)) {
            // do not need tmp here
            __ shift_left(left_op, exact_log2(c), result_op);
            did_strength_reduce = true;
          } else {
            did_strength_reduce = strength_reduce_multiply(left_op, c, result_op, tmp_op);
          }
        }
        // we couldn't strength reduce so just emit the multiply
        if (!did_strength_reduce) {
          __ mul(left_op, right_op, result_op);
        }
      }
      break;

    case Bytecodes::_dsub:
    case Bytecodes::_fsub:
    case Bytecodes::_lsub:
    case Bytecodes::_isub: __ sub(left_op, right_op, result_op); break;

    case Bytecodes::_fdiv: __ div (left_op, right_op, result_op); break;
    case Bytecodes::_ldiv: __ div (left_op, right_op, result_op, info); break;

    case Bytecodes::_ddiv:
      {
        if (is_strictfp) {
          __ div_strictfp (left_op, right_op, result_op); break;
        } else {
          __ div (left_op, right_op, result_op); break;
        }
      }
      break;

    case Bytecodes::_lrem: __ rem (left_op, right_op, result_op, info); break;
    case Bytecodes::_drem:
    case Bytecodes::_frem: __ rem (left_op, right_op, result_op); break;

    default: ShouldNotReachHere();
  }
}


void LIR_Emitter::arithmetic_op_int(Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, RInfo tmp) {
  arithmetic_op(code, result, left, right, false, tmp);
}


void LIR_Emitter::arithmetic_op_long(Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, CodeEmitInfo* info) {
  arithmetic_op(code, result, left, right, false, norinfo, info);
}


void LIR_Emitter::arithmetic_op_fpu(Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, bool is_strictfp) {
  arithmetic_op(code, result, left, right, is_strictfp, norinfo);
}


void LIR_Emitter::arithmetic_idiv (Bytecodes::Code code, LIR_Opr result, LIR_Opr left, LIR_Opr right, RInfo scratch, CodeEmitInfo* info) {
  assert(left->is_register() && (right->is_register() || right->is_constant()) && result->is_register(), "wrong item type");
  NEEDS_CLEANUP;
  // assert(left->destroys_register() && right->destroys_register(), "check");
  if (code == Bytecodes::_irem) {
    if (right->is_constant()) {
      __ irem(left->as_rinfo(), opr2int(right), result->as_rinfo(), scratch, info);
    } else {
      __ irem(left->as_rinfo(), right->as_rinfo(), result->as_rinfo(), scratch, info);
    }
  } else if (code == Bytecodes::_idiv) {
    if (right->is_constant()) {
      __ idiv(left->as_rinfo(), opr2int(right), result->as_rinfo(), scratch, info);
    } else {
      __ idiv(left->as_rinfo(), right->as_rinfo(), result->as_rinfo(), scratch, info);
    }
  } else {
    ShouldNotReachHere();
  }
}


void LIR_Emitter::arithmetic_call_op (Bytecodes::Code code, RInfo temp_reg) {
  address entry;
  int size = -1;
  int args = native_arg_no_longs;
  switch (code) {
    case Bytecodes::_lrem:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::lrem);
      size = 4;
      args = native_arg0_is_long | native_arg1_is_long | native_return_is_long;
      break; // check if dividend is 0 is done elsewhere
    case Bytecodes::_ldiv:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::ldiv);
      size = 4;
      args = native_arg0_is_long | native_arg1_is_long | native_return_is_long;
      break; // check if dividend is 0 is done elsewhere
    case Bytecodes::_lmul:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::lmul);
      size = 4;
      args = native_arg0_is_long | native_arg1_is_long | native_return_is_long;
      break;
    case Bytecodes::_frem:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::frem);
      size = 2;
      break;
    case Bytecodes::_drem:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::drem);
      size = 4;
      break;
    default:
      ShouldNotReachHere();
  }
  __ call_runtime_leaf(entry, temp_reg, size, args);
}


void LIR_Emitter::shift_op(Bytecodes::Code code, RInfo dst_reg, LIR_Opr value, LIR_Opr count, RInfo tmp) {
  RInfo valueR = value->as_rinfo();
  if (count->is_constant()) {
    int countV = opr2int(count);
    switch(code) {
      case Bytecodes::_ishl:
      case Bytecodes::_lshl: __ shift_left(valueR, countV, dst_reg); break;
      case Bytecodes::_ishr:
      case Bytecodes::_lshr: __ shift_right(valueR, countV, dst_reg); break;
      case Bytecodes::_iushr:
      case Bytecodes::_lushr: __ unsigned_shift_right(valueR, countV, dst_reg); break;
      default: ShouldNotReachHere();
    };
  } else {
    LIR_Opr dst_opr = LIR_OprFact::rinfo(dst_reg);
    LIR_Opr tmp_opr = LIR_OprFact::rinfo(tmp);
    switch(code) {
      case Bytecodes::_ishl:
      case Bytecodes::_lshl: __ shift_left(value, count, dst_opr, tmp_opr); break;
      case Bytecodes::_ishr:
      case Bytecodes::_lshr: __ shift_right(value, count, dst_opr, tmp_opr); break;
      case Bytecodes::_iushr:
      case Bytecodes::_lushr: __ unsigned_shift_right(value, count, dst_opr, tmp_opr); break;
      default: ShouldNotReachHere();
    };
  }
}


void LIR_Emitter::logic_op (Bytecodes::Code code, RInfo dst, LIR_Opr left, LIR_Opr right) {
  RInfo l = left->as_rinfo();
  LIR_Opr r = right;
  switch(code) {
    case Bytecodes::_iand:
    case Bytecodes::_land:  __ logical_and(l, r, dst); break;

    case Bytecodes::_ior:
    case Bytecodes::_lor:   __ logical_or(l, r, dst);  break;

    case Bytecodes::_ixor:
    case Bytecodes::_lxor:  __ logical_xor(l, r, dst); break;

    default: ShouldNotReachHere();
  }
}


// _lcmp, _fcmpl, _fcmpg, _dcmpl, _dcmpg
void LIR_Emitter::compare_op (Bytecodes::Code code, RInfo dst, LIR_Opr left, LIR_Opr right) {
  if (left->value_type()->is_float_kind()) {
    __ fcmp2int(left->as_rinfo(), right->as_rinfo(), dst, (code == Bytecodes::_fcmpl || code == Bytecodes::_dcmpl));
  } else if (left->value_type()->is_long()) {
    __ lcmp2int(left->as_rinfo(), right->as_rinfo(), dst);
  } else {
    Unimplemented();
  }
}


void LIR_Emitter::convert_op (Bytecodes::Code code, LIR_Opr src, RInfo dst_reg, bool is_32bit) {
  __ convert(code, src, LIR_OprFact::rinfo(dst_reg), is_32bit);
}


void LIR_Emitter::call_op(Bytecodes::Code code, const BasicTypeArray* sig_types, CodeEmitInfo* info, int vtable_index, bool optimized, bool needs_null_check, RInfo receiver, LIR_Opr result) {
  LIR_Opr resultOpr = result;
  switch (code) {
    case Bytecodes::_invokestatic:
      { StaticCallStub* call_stub = new StaticCallStub();
        // the stub is added as part of call_static
        __ call_static(resultOpr, Runtime1::entry_for(Runtime1::resolve_invokestatic_id), info, call_stub);
      }
      break;
    case Bytecodes::_invokespecial:
    case Bytecodes::_invokevirtual:
    case Bytecodes::_invokeinterface:
      // for final target we still produce an inline cache, in order
      // to be able to call mixed mode
      if (needs_null_check && GenerateCompilerNullChecks) {
        assert(info != NULL, "must have debug information");
        __ null_check(receiver, info);
      }
      if (code == Bytecodes::_invokespecial || optimized) {
        StaticCallStub* call_stub = new StaticCallStub();
        __ call_opt_virtual(receiver, resultOpr, Runtime1::entry_for(Runtime1::resolve_invoke_opt_virtual_id), info, call_stub);
      } else if (vtable_index == -1) {
        __ call_icvirtual(receiver, resultOpr, Runtime1::entry_for(Runtime1::resolve_invokevirtual_id), info);
      } else {
        int entry_offset = instanceKlass::vtable_start_offset() + vtable_index*vtableEntry::size();
        int vtable_offset = entry_offset*wordSize + vtableEntry::method_offset_in_bytes();
        __ call_virtual(receiver, resultOpr, vtable_offset, info);
      }
      break;
    default:
      ShouldNotReachHere();
      break;
  }
}


void LIR_Emitter::throw_op (LIR_Opr exceptionItem, RInfo exceptionOop, RInfo exceptionPc, CodeEmitInfo* info) {
  if (!exceptionOop.is_same(exceptionItem->as_rinfo())) {
    __ reg2reg(exceptionItem->as_rinfo(), exceptionOop, T_OBJECT);
  }
  if (GenerateCompilerNullChecks) {
    __ null_check(exceptionOop, info);
  }
  __ throw_exception(exceptionPc, exceptionOop, info);
}


void LIR_Emitter::monitor_enter(RInfo object, RInfo lock, RInfo hdr, RInfo scratch, int monitor_no, CodeEmitInfo* info_for_exception, CodeEmitInfo* info) {
  if (!GenerateSynchronizationCode) return;
  // for slow path, use debug info for state after successful locking
  CodeStub* slow_path = new MonitorEnterStub(object, lock, info);
  __ load_stack_address_monitor(monitor_no, lock);
  // for handling NullPointerException, use debug info representing just the lock stack before this monitorenter
  __ lock_object(hdr, object, lock, scratch, slow_path, info_for_exception);
}


void LIR_Emitter::monitor_exit(RInfo object, RInfo lock, RInfo new_hdr, int monitor_no) {
  if (!GenerateSynchronizationCode) return;
  // setup registers
  RInfo hdr = lock;
  lock = new_hdr;
  CodeStub* slow_path = new MonitorExitStub(lock, UseFastLocking, monitor_no);
  __ load_stack_address_monitor(monitor_no, lock);
  __ unlock_object(hdr, object, lock, slow_path);
}


void LIR_Emitter::goto_op (BlockBegin* dst, CodeEmitInfo* info) {
  // check if it is backward branch
  __ jump(dst, info);
}


//    equal, notEqual, less, lessEqual, greaterEqual, greater, belowEqual, aboveEqual, always, intrinsicFailed // more to be added


LIR_OpBranch::LIR_Condition LIR_Emitter::lir_cond(If::Condition cond) {
  LIR_OpBranch::LIR_Condition l;
  switch (cond) {
    case If::eql: l = LIR_OpBranch::equal;        break;
    case If::neq: l = LIR_OpBranch::notEqual;     break;
    case If::lss: l = LIR_OpBranch::less;         break;
    case If::leq: l = LIR_OpBranch::lessEqual;    break;
    case If::geq: l = LIR_OpBranch::greaterEqual; break;
    case If::gtr: l = LIR_OpBranch::greater;      break;
    default: ShouldNotReachHere();
  };
  return l;
}

void LIR_Emitter::if_op(int phase, If::Condition cond, LIR_Opr x, LIR_Opr y,
                        BlockBegin* t_dest, BlockBegin* f_dest, BlockBegin* u_dest, CodeEmitInfo* safepoint) {
  ValueTag cmpTag = y->value_type()->tag();
  if (phase == 1) {
    if (y->is_constant()) {
      if (cmpTag == objectTag || cmpTag == intTag  || cmpTag == longTag) {
        __ cmp(lir_cond(cond), x, y, safepoint);
      } else {
        Unimplemented();
      }
    } else if (y->is_stack()) {
      __ cmp(lir_cond(cond), x, y, safepoint);
    } else if (y->is_register()) {
      __ cmp(lir_cond(cond), x, y, safepoint);
    } else {
      ShouldNotReachHere();
    }
  } else if (phase == 2) {
    if (cmpTag == floatTag || cmpTag == doubleTag) {
      assert(u_dest != NULL, "must have unordered successor");
      __ branch_float(lir_cond(cond), t_dest->label(), u_dest->label());
    } else {
      __ branch(lir_cond(cond), t_dest);
    }
  } else {
    ShouldNotReachHere(); // there are only phases 1 and 2
  }
}


void LIR_Emitter::tableswitch_op  (LIR_Opr tag, int match, BlockBegin* dest) {
  __ cmp(LIR_OpBranch::equal, tag, match);
  __ branch(LIR_OpBranch::equal, dest);
}


void LIR_Emitter::lookupswitch_op(LIR_Opr tag, int key,   BlockBegin* dest) {
  __ cmp(LIR_OpBranch::equal, tag, key);
  __ branch(LIR_OpBranch::equal, dest);
}


void LIR_Emitter::lookupswitch_range_op(LIR_Opr tag, int low_key, int high_key, BlockBegin* dest) {
  assert(low_key <= high_key, "wrong range");
  if (low_key == high_key) {
    __ cmp(LIR_OpBranch::equal, tag, low_key);
    __ branch(LIR_OpBranch::equal, dest);
  } else if (high_key - low_key == 1) {
    __ cmp(LIR_OpBranch::equal, tag, low_key);
    __ branch(LIR_OpBranch::equal, dest);
    __ cmp(LIR_OpBranch::equal, tag, high_key);
    __ branch(LIR_OpBranch::equal, dest);
  } else {
    LabelObj* L = new LabelObj();
    __ cmp(LIR_OpBranch::less, tag, low_key);
    __ branch(LIR_OpBranch::less, L->label());
    __ cmp(LIR_OpBranch::lessEqual, tag, high_key);
    __ branch(LIR_OpBranch::lessEqual, dest);
    __ branch_destination(L->label());
  }
}


void LIR_Emitter::ifop_phase2(RInfo dst, LIR_Opr tval, LIR_Opr fval, Instruction::Condition cond) {
  move(tval, dst);
  LabelObj* L = new LabelObj();
  __ branch(lir_cond(cond), L->label());
  move(fval, dst);
  __ branch_destination(L->label());
}


void LIR_Emitter::ifop_phase1(Instruction::Condition cond, LIR_Opr x, LIR_Opr y) {
  __ cmp(lir_cond(cond), x, y);
}


void LIR_Emitter::new_instance(RInfo dst, ciInstanceKlass* klass, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo klass_reg, CodeEmitInfo* info) {
  jobject2reg_with_patching(klass_reg, klass, info);
  // If klass is not loaded we do not know if the klass has finalizers:
  if (UseFastNewInstance && klass->is_loaded() && (klass->access_flags() & JVM_ACC_CAN_BE_FASTPATH_ALLOCATED)) {

    Runtime1::StubID stub_id = klass->is_initialized() ? Runtime1::fast_new_instance_id : Runtime1::fast_new_instance_init_check_id;

    CodeStub* slow_path = new NewInstanceStub(klass_reg, LIR_OprFact::rinfo(dst, T_OBJECT), klass, info, stub_id);

    // make sure klass is fully initialized
    if (!klass->is_initialized()) {
      cmp_mem_int(LIR_OpBranch::notEqual, klass_reg, instanceKlass::init_state_offset_in_bytes() + sizeof(oopDesc), instanceKlass::fully_initialized, info);
      __ branch(LIR_OpBranch::notEqual, slow_path);
    }
    assert(klass->is_loaded(), "must be loaded");
    // allocate space for instance
    assert(klass->size_helper() >= 0, "illegal instance size");
    const int instance_size = align_object_size(klass->size_helper());
    __ allocate_object(dst, scratch1, scratch2, scratch3,
                       oopDesc::header_size(), instance_size, klass_reg, slow_path);
  } else {
    CodeStub* slow_path = new NewInstanceStub(klass_reg, LIR_OprFact::rinfo(dst, T_OBJECT), klass, info, Runtime1::new_instance_id);
    __ branch(LIR_OpBranch::always, slow_path);
    __ branch_destination(slow_path->continuation());
  }
}

void LIR_Emitter::new_multi_array (RInfo dst, ciKlass* klass, int rank, RInfo tmp, CodeEmitInfo* info, CodeEmitInfo* patching_info) {
  jobject2reg_with_patching(dst, klass, patching_info);
  __ allocate_multi_array(dst, rank, info);
}


void LIR_Emitter::instanceof_op(LIR_Opr dst, LIR_Opr objItem, ciKlass* k, RInfo k_RInfo, RInfo klass_RInfo, bool fast_check, CodeEmitInfo* patching_info) {
  assert(!UseSlowPath || !fast_check, "fast checks are disabled");
  __ instanceof(dst, objItem, k, LIR_OprFact::rinfo(k_RInfo), LIR_OprFact::rinfo(klass_RInfo), fast_check, patching_info);
}


void LIR_Emitter::instance_of_test_op(RInfo dst_reg, LIR_Opr obj_item, ciKlass* k, RInfo klass_ri, BlockBegin* is_instance, BlockBegin* not_is_instance, bool is_inverted, CodeEmitInfo* info, CodeEmitInfo* patching_info) {
  // this optimization has not been implemented yet in the frontend
  Unimplemented();
}


void LIR_Emitter::nop() {
  __ nop(NULL);
}


void LIR_Emitter::safepoint_nop(CodeEmitInfo* info) {
  __ nop(info);
}


void LIR_Emitter::align_backward_branch() {
  __ backward_branch_target();
}


void LIR_Emitter::return_op(LIR_Opr opr) {
  assert(opr->is_illegal() || opr->is_register(), "must be register");
  __ return_op(opr);
}


void LIR_Emitter::set_24bit_fpu_precision() {
  __ set_24bit_fpu();
}


void LIR_Emitter::restore_fpu_precision() {
  __ restore_fpu();
}


void LIR_Emitter::breakpoint() {
  __ breakpoint();
}


void LIR_Emitter::handler_entry() {
  __ nop(NULL);
}


void LIR_Emitter::getClass(RInfo dst, RInfo rcvr, CodeEmitInfo* info) {
  __ load_mem_reg(rcvr, oopDesc::klass_offset_in_bytes(), dst, T_OBJECT, info, LIR_Op1::patch_none);
  __ load_mem_reg(dst, Klass::java_mirror_offset_in_bytes() + klassOopDesc::klass_part_offset_in_bytes(), dst, T_OBJECT, NULL, LIR_Op1::patch_none);
}


void LIR_Emitter::membar() {
  __ membar();
}

void LIR_Emitter::membar_acquire() {
  __ membar_acquire();
}

void LIR_Emitter::membar_release() {
  __ membar_release();
}

#undef __


