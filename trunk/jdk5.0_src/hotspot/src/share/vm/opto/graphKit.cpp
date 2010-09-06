#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)graphKit.cpp	1.91 04/03/31 18:13:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_graphKit.cpp.incl"

//----------------------------GraphKit-----------------------------------------
// Main utility constructor.
GraphKit::GraphKit(JVMState* jvms)
  : Phase(Phase::Parser),
    _env(C->env()),
    _gvn(*C->initial_gvn())
{
  _exceptions = jvms->map()->next_exception();
  if (_exceptions != NULL)  jvms->map()->set_next_exception(NULL);
  set_jvms(jvms);
}

// Private constructor for parser.
GraphKit::GraphKit()
  : Phase(Phase::Parser),
    _env(C->env()),
    _gvn(*C->initial_gvn())
{
  _exceptions = NULL;
  set_map(NULL);
  debug_only(_sp = -99);
  debug_only(set_bci(-99));
}



//---------------------------clean_stack---------------------------------------
// Clear away rubbish from the stack area of the JVM state.
// This destroys any arguments that may be waiting on the stack.
void GraphKit::clean_stack(int from_sp) {
  SafePointNode* map      = this->map();
  JVMState*      jvms     = this->jvms();
  int            stk_size = jvms->stk_size();
  int            stkoff   = jvms->stkoff();
  Node*          top      = this->top();
  for (int i = from_sp; i < stk_size; i++) {
    if (map->in(stkoff + i) != top) {
      map->set_req(stkoff + i, top);
    }
  }
}


//--------------------------------sync_jvms-----------------------------------
// Make sure our current jvms agrees with our parse state.
JVMState* GraphKit::sync_jvms() const {
  JVMState* jvms = this->jvms();
  jvms->set_bci(bci());       // Record the new bci in the JVMState
  jvms->set_sp(sp());         // Record the new sp in the JVMState
  assert(jvms_in_sync(), "jvms is now in sync");
  return jvms;
}

#ifdef ASSERT
bool GraphKit::jvms_in_sync() const {
  Parse* parse = is_Parse();
  if (parse == NULL) {
    if (bci() !=      jvms()->bci())          return false;
    if (sp()  != (int)jvms()->sp())           return false;
    return true;
  }
  if (jvms()->method() != parse->method())    return false;
  if (jvms()->bci()    != parse->bci())       return false;
  int jvms_sp = jvms()->sp();
  if (jvms_sp          != parse->sp())        return false;
  int jvms_depth = jvms()->depth();
  if (jvms_depth       != parse->depth())     return false;
  return true;
}

// Local helper checks for special internal merge points
// used to accumulate and merge exception states.
// They are marked by the region's in(0) edge being the map itself.
// Such merge points must never "escape" into the parser at large,
// until they have been handed to gvn.transform.
static bool is_hidden_merge(Node* reg) {
  if (reg == NULL)  return false;
  if (reg->is_Phi()) {
    reg = reg->in(0);
    if (reg == NULL)  return false;
  }
  return reg->is_Region() && reg->in(0) != NULL && reg->in(0)->is_Root();
}

void GraphKit::verify_map() const {
  if (map() == NULL)  return;  // null map is OK
  assert(map()->req() <= jvms()->endoff(), "no extra garbage on map");
  assert(!map()->has_exceptions(),    "call add_exception_states_from 1st");
  assert(!is_hidden_merge(control()), "call use_exception_state, not set_map");
}

void GraphKit::verify_exception_state(SafePointNode* ex_map) {
  assert(ex_map->next_exception() == NULL, "not already part of a chain");
  assert(has_saved_ex_oop(ex_map), "every exception state has an ex_oop");
}
#endif

//---------------------------------stop----------------------------------------
// Set _map to NULL, signalling a stop to further bytecode execution.
void GraphKit::stop() {
  _map = NULL;
}


//--------------------------------stopped--------------------------------------
// Tell if _map is NULL, or control is top.
bool GraphKit::stopped() {
  if (map() == NULL)           return true;
  else if (control() == top()) return true;
  else                         return false;
}


//-----------------------------has_ex_handler----------------------------------
// Tell if this method or any caller method has exception handlers.
bool GraphKit::has_ex_handler() {
  for (JVMState* jvmsp = jvms(); jvmsp != NULL; jvmsp = jvmsp->caller()) {
    if (jvmsp->has_method() && jvmsp->method()->has_exception_handlers()) {
      return true;
    }
  }
  return false;
}

//------------------------------save_ex_oop------------------------------------
// Save an exception without blowing stack contents or other JVM state.
void GraphKit::set_saved_ex_oop(SafePointNode* ex_map, Node* ex_oop) {
  assert(!has_saved_ex_oop(ex_map), "clear ex-oop before setting again");
  ex_map->add_req(ex_oop);
  debug_only(verify_exception_state(ex_map));
}

inline static Node* common_saved_ex_oop(SafePointNode* ex_map, bool clear_it) {
  assert(GraphKit::has_saved_ex_oop(ex_map), "ex_oop must be there");
  Node* ex_oop = ex_map->in(ex_map->req()-1);
  if (clear_it)  ex_map->del_req(ex_map->req()-1);
  return ex_oop;
}

//-----------------------------saved_ex_oop------------------------------------
// Recover a saved exception from its map.
Node* GraphKit::saved_ex_oop(SafePointNode* ex_map) {
  return common_saved_ex_oop(ex_map, false);
}

//--------------------------clear_saved_ex_oop---------------------------------
// Erase a previously saved exception from its map.
Node* GraphKit::clear_saved_ex_oop(SafePointNode* ex_map) {
  return common_saved_ex_oop(ex_map, true);
}

#ifdef ASSERT
//---------------------------has_saved_ex_oop----------------------------------
// Erase a previously saved exception from its map.
bool GraphKit::has_saved_ex_oop(SafePointNode* ex_map) {
  return ex_map->req() == ex_map->jvms()->endoff()+1;
}
#endif

//-------------------------make_exception_state--------------------------------
// Turn the current JVM state into an exception state, appending the ex_oop.
SafePointNode* GraphKit::make_exception_state(Node* ex_oop) {
  sync_jvms();
  SafePointNode* ex_map = map();
  stop();  // do not manipulate this map any more
  set_saved_ex_oop(ex_map, ex_oop);
  return ex_map;
}


//--------------------------add_exception_state--------------------------------
// Add an exception to my list of exceptions.
void GraphKit::add_exception_state(SafePointNode* ex_map) {
  if (ex_map == NULL || ex_map->control() == top()) {
    return;
  }
#ifdef ASSERT
  verify_exception_state(ex_map);
  if (has_exceptions()) {
    assert(ex_map->jvms()->same_calls_as(_exceptions->jvms()), "all collected exceptions must come from the same place");
  }
#endif

  // If there is already an exception of exactly this type, merge with it.
  // In particular, null-checks and other low-level exceptions common up here.
  Node*       ex_oop  = saved_ex_oop(ex_map);
  const Type* ex_type = _gvn.type(ex_oop);
  if (ex_oop == top()) {
    // No action needed.
    return;
  }
  assert(ex_type->isa_instptr(), "exception must be an instance");
  for (SafePointNode* e2 = _exceptions; e2 != NULL; e2 = e2->next_exception()) {
    const Type* ex_type2 = _gvn.type(saved_ex_oop(e2));
    // We check sp also because call bytecodes can generate exceptions
    // both before and after arguments are popped!
    if (ex_type2 == ex_type
        && e2->_jvms->sp() == ex_map->_jvms->sp()) {
      combine_exception_states(ex_map, e2);
      return;
    }
  }

  // No pre-existing exception of the same type.  Chain it on the list.
  push_exception_state(ex_map);
}

//-----------------------add_exception_states_from-----------------------------
void GraphKit::add_exception_states_from(JVMState* jvms) {
  SafePointNode* ex_map = jvms->map()->next_exception();
  if (ex_map != NULL) {
    jvms->map()->set_next_exception(NULL);
    for (SafePointNode* next_map; ex_map != NULL; ex_map = next_map) {
      next_map = ex_map->next_exception();
      ex_map->set_next_exception(NULL);
      add_exception_state(ex_map);
    }
  }
}

//-----------------------transfer_exceptions_into_jvms-------------------------
JVMState* GraphKit::transfer_exceptions_into_jvms() {
  if (map() == NULL) {
    // We need a JVMS to carry the exceptions, but the map has gone away.
    // Create a scratch JVMS, cloned from any of the exception states...
    if (has_exceptions()) {
      _map = _exceptions;
      _map = clone_map();
      _map->set_next_exception(NULL);
      clear_saved_ex_oop(_map);
      debug_only(verify_map());
    } else {
      // ...or created from scratch
      JVMState* jvms = new JVMState(_method, NULL);
      jvms->set_bci(_bci);
      jvms->set_sp(_sp);
      jvms->set_map(new SafePointNode(TypeFunc::Parms, jvms));
      set_jvms(jvms);
      for (uint i = 0; i < map()->req(); i++)  map()->set_req(i, top());
      set_all_memory(top());
      while (map()->req() < jvms->endoff())  map()->add_req(top());
    }
    // (This is a kludge, in case you didn't notice.)
    set_control(top());
  }
  JVMState* jvms = sync_jvms();
  assert(!jvms->map()->has_exceptions(), "no exceptions on this map yet");
  jvms->map()->set_next_exception(_exceptions);
  _exceptions = NULL;   // done with this set of exceptions
  return jvms;
}

static inline void add_n_reqs(Node* dstphi, Node* srcphi) {
  assert(is_hidden_merge(dstphi), "must be a special merge node");
  assert(is_hidden_merge(srcphi), "must be a special merge node");
  uint limit = srcphi->req();
  for (uint i = PhiNode::Input; i < limit; i++) {
    dstphi->add_req(srcphi->in(i));
  }
}
static inline void add_one_req(Node* dstphi, Node* src) {
  assert(is_hidden_merge(dstphi), "must be a special merge node");
  assert(!is_hidden_merge(src), "must not be a special merge node");
  dstphi->add_req(src);
}

//-----------------------combine_exception_states------------------------------
// This helper function combines exception states by building phis on a
// specially marked state-merging region.  These regions and phis are
// untransformed, and can build up gradually.  The region is marked by
// having a control input of its exception map, rather than NULL.  Such
// regions do not appear except in this function, and in use_exception_state.
void GraphKit::combine_exception_states(SafePointNode* ex_map, SafePointNode* phi_map) {
  if (failing())  return;  // dying anyway...
  JVMState* ex_jvms = ex_map->_jvms;
  assert(ex_jvms->same_calls_as(phi_map->_jvms), "consistent call chains");
  assert(ex_jvms->stkoff() == phi_map->_jvms->stkoff(), "matching locals");
  assert(ex_jvms->sp() == phi_map->_jvms->sp(), "matching stack sizes");
  assert(ex_jvms->monoff() == phi_map->_jvms->monoff(), "matching JVMS");
  assert(ex_map->req() == phi_map->req(), "matching maps");
  uint tos = ex_jvms->stkoff() + ex_jvms->sp();
  Node*         hidden_merge_mark = root();
  Node*         region  = phi_map->control();
  MergeMemNode* phi_mem = phi_map->merged_memory();
  MergeMemNode* ex_mem  = ex_map->merged_memory();
  if (region->in(0) != hidden_merge_mark) {
    // The control input is not (yet) a specially-marked region in phi_map.
    // Make it so, and build some phis.
    region = new RegionNode(1);
    _gvn.set_type(region, Type::CONTROL);
    region->set_req(0, hidden_merge_mark);  // marks an internal ex-state
    region->add_req(phi_map->control());
    phi_map->set_control(region);
    Node* io_phi = PhiNode::make(region, phi_map->i_o(), Type::ABIO);
    record_for_igvn(io_phi);
    _gvn.set_type(io_phi, Type::ABIO);
    phi_map->set_i_o(io_phi);
    for (MergeMemStream mms(phi_mem); mms.next_non_empty(); ) {
      Node* m = mms.memory();
      Node* m_phi = PhiNode::make(region, m, Type::MEMORY, mms.adr_type());
      record_for_igvn(m_phi);
      _gvn.set_type(m_phi, Type::MEMORY);
      mms.set_memory(m_phi);
    }
  }

  // Either or both of phi_map and ex_map might already be converted into phis.
  Node* ex_control = ex_map->control();
  // if there is special marking on ex_map also, we add multiple edges from src
  bool add_multiple = (ex_control->in(0) == hidden_merge_mark);
  // how wide was the destination phi_map, originally?
  uint orig_width = region->req();

  if (add_multiple) {
    add_n_reqs(region, ex_control);
    add_n_reqs(phi_map->i_o(), ex_map->i_o());
  } else {
    // ex_map has no merges, so we just add single edges everywhere
    add_one_req(region, ex_control);
    add_one_req(phi_map->i_o(), ex_map->i_o());
  }
  for (MergeMemStream mms(phi_mem, ex_mem); mms.next_non_empty2(); ) {
    if (mms.is_empty()) {
      // get a copy of the base memory, and patch some inputs into it
      const TypePtr* adr_type = mms.adr_type();
      Node* phi = mms.force_memory()->is_Phi()->slice_memory(adr_type);
      assert(phi->is_Phi()->region() == mms.base_memory()->in(0), "");
      mms.set_memory(phi);
      // Prepare to append interesting stuff onto the newly sliced phi:
      while (phi->req() > orig_width)  phi->del_req(phi->req()-1);
    }
    // Append stuff from ex_map:
    if (add_multiple) {
      add_n_reqs(mms.memory(), mms.memory2());
    } else {
      add_one_req(mms.memory(), mms.memory2());
    }
  }
  uint limit = ex_map->req();
  for (uint i = TypeFunc::Parms; i < limit; i++) {
    // Skip everything in the JVMS after tos.  (The ex_oop follows.)
    if (i == tos)  i = ex_jvms->monoff();
    Node* src = ex_map->in(i);
    Node* dst = phi_map->in(i);
    if (src != dst) {
      PhiNode* phi;
      if (dst->in(0) != region) {
        dst = phi = PhiNode::make(region, dst, _gvn.type(dst));
        record_for_igvn(phi);
        _gvn.set_type(phi, phi->type());
        phi_map->set_req(i, dst);
        // Prepare to append interesting stuff onto the new phi:
        while (dst->req() > orig_width)  dst->del_req(dst->req()-1);
      } else {
        assert(dst->is_Phi(), "nobody else uses a hidden region");
        phi = (PhiNode*)dst;
      }
      if (add_multiple && src->in(0) == ex_control) {
        // Both are phis.
        add_n_reqs(dst, src);
      } else {
        while (dst->req() < region->req())  add_one_req(dst, src);
      }
      const Type* srctype = _gvn.type(src);
      if (phi->type() != srctype) {
        const Type* dsttype = phi->type()->meet(srctype);
        if (phi->type() != dsttype) {
          phi->set_type(dsttype);
          _gvn.set_type(phi, dsttype);
        }
      }
    }
  }
}

//--------------------------use_exception_state--------------------------------
Node* GraphKit::use_exception_state(SafePointNode* phi_map) {
  if (failing()) { stop(); return top(); }
  Node* region = phi_map->control();
  Node* hidden_merge_mark = root();
  assert(phi_map->jvms()->map() == phi_map, "sanity: 1-1 relation");
  Node* ex_oop = clear_saved_ex_oop(phi_map);
  if (region->in(0) == hidden_merge_mark) {
    // Special marking for internal ex-states.  Process the phis now.
    region->set_req(0, region);  // now it's an ordinary region
    set_jvms(phi_map->jvms());   // ...so now we can use it as a map
    // Note: Setting the jvms also sets the bci and sp.
    set_control(_gvn.transform(region));
    uint tos = jvms()->stkoff() + sp();
    for (uint i = 1; i < tos; i++) {
      Node* x = phi_map->in(i);
      if (x->in(0) == region) {
        assert(x->is_Phi(), "expected a special phi");
        phi_map->set_req(i, _gvn.transform(x));
      }
    }
    for (MergeMemStream mms(merged_memory()); mms.next_non_empty(); ) {
      Node* x = mms.memory();
      if (x->in(0) == region) {
        assert(x->is_Phi(), "nobody else uses a hidden region");
        mms.set_memory(_gvn.transform(x));
      }
    }
    if (ex_oop->in(0) == region) {
      assert(ex_oop->is_Phi(), "expected a special phi");
      ex_oop = _gvn.transform(ex_oop);
    }
  } else {
    set_jvms(phi_map->jvms());
  }

  assert(!is_hidden_merge(phi_map->control()), "hidden ex. states cleared");
  assert(!is_hidden_merge(phi_map->i_o()), "hidden ex. states cleared");
  return ex_oop;
}

//---------------------------------java_bc-------------------------------------
Bytecodes::Code GraphKit::java_bc() const {
  ciMethod* method = this->method();
  int       bci    = this->bci();
  if (method != NULL && bci != InvocationEntryBci)
    return method->java_code_at_bci(bci);
  else
    return Bytecodes::_illegal;
}

//------------------------------builtin_throw----------------------------------
void GraphKit::builtin_throw(Deoptimization::DeoptReason reason, Node* arg) {
  if (JvmtiExport::can_post_exceptions()) {
    // Do not try anything fancy if we're notifying the VM on every throw.
    // Cf. case Bytecodes::_athrow in parse2.cpp.
    uncommon_trap(reason, Deoptimization::Action_none);
    return;
  }

  // If this particular condition has not yet happened at this
  // bytecode, then use the uncommon trap mechanism, and allow for
  // a future recompilation if several traps occur here.
  // If the throw is hot, try to use a more complicated inline mechanism
  // which keeps execution inside the compiled code.
  bool treat_throw_as_hot = false;
  ciMethodData* md = method()->method_data();

  if (ProfileTraps) {
    if (too_many_traps(reason)) {
      treat_throw_as_hot = true;
    }
    // (If there is no MDO at all, assume it is early in
    // execution, and that any deopts are part of the
    // startup transient, and don't need to be remembered.)

    // Also, if there is a local exception handler, treat all throws
    // as hot if there has been at least one in this method.
    if (C->trap_count(reason) != 0
        && method()->method_data()->trap_count(reason) != 0
        && has_ex_handler()) {
        treat_throw_as_hot = true;
    }
  }

  // If this throw happens frequently, an uncommon trap might cause
  // a performance pothole.  If there is a local exception handler,
  // and if this particular bytecode appears to be deoptimizing often,
  // let us handle the throw inline, with a preconstructed instance.
  // Note:   If the deopt count has blown up, the uncommon trap
  // runtime is going to flush this nmethod, not matter what.
  if (treat_throw_as_hot
      && (!StackTraceInThrowable || OmitStackTraceInFastThrow)) {
    // If the throw is local, we use a pre-existing instance and
    // punt on the backtrace.  This would lead to a missing backtrace
    // (a repeat of 4292742) if the backtrace object is ever asked
    // for its backtrace.
    // Fixing this remaining case of 4292742 requires some flavor of
    // escape analysis.  Leave that for the future.
    ciInstance* ex_obj = NULL;
    switch (reason) {
    case Deoptimization::Reason_null_check:
      ex_obj = env()->NullPointerException_instance();
      break;
    case Deoptimization::Reason_div0_check:
      ex_obj = env()->ArithmeticException_instance();
      break;
    case Deoptimization::Reason_range_check:
      ex_obj = env()->ArrayIndexOutOfBoundsException_instance();
      break;
    case Deoptimization::Reason_class_check:
      if (java_bc() == Bytecodes::_aastore) {
        ex_obj = env()->ArrayStoreException_instance();
      } else {
        ex_obj = env()->ClassCastException_instance();
      }
      break;
    }
    if (failing()) { stop(); return; }  // exception allocation might fail
    if (ex_obj != NULL) {
      // Cheat with a preallocated exception object.
      if (C->log() != NULL)
        C->log()->elem("hot_throw preallocated='1' reason='%s'",
                       Deoptimization::trap_reason_name(reason));
      const TypeInstPtr* ex_con  = TypeInstPtr::make(ex_obj);
      Node*              ex_node = _gvn.transform(new(1) ConPNode(ex_con));
      add_exception_state(make_exception_state(ex_node));
      return;
    }
  }

  // %%% Maybe add entry to OptoRuntime which directly throws the exc.?
  // It won't be much cheaper than bailing to the interp., since we'll
  // have to pass up all the debug-info, and the runtime will have to
  // create the stack trace.

  // Usual case:  Bail to interpreter.
  // Reserve the right to recompile if we haven't seen anything yet.

  Deoptimization::DeoptAction action = Deoptimization::Action_maybe_recompile;
  if (treat_throw_as_hot
      && (method()->method_data()->trap_recompiled_at(bci())
          || C->trap_count(reason) >= (uint)PerMethodTrapLimit)) {
    // We cannot afford to take more traps here.  Suffer in the interpreter.
    if (C->log() != NULL)
      C->log()->elem("hot_throw preallocated='0' reason='%s' mcount='%d'",
                     Deoptimization::trap_reason_name(reason),
                     C->trap_count(reason));
    action = Deoptimization::Action_none;
  }

  bool must_throw = true;
  // "must_throw" prunes the JVM state to include only the stack, if there
  // are no local exception handlers.  This should cut down on register
  // allocation time and code size, by drastically reducing the number
  // of in-edges on the call to the uncommon trap.

  uncommon_trap(reason, action, (ciKlass*)NULL, (char*)NULL, must_throw);
}


//----------------------------PreserveJVMState---------------------------------
PreserveJVMState::PreserveJVMState(GraphKit* kit, bool clone_map) {
  debug_only(kit->verify_map());
  _kit    = kit;
  _map    = kit->map();   // preserve the map
  _sp     = kit->sp();
  kit->set_map(clone_map ? kit->clone_map() : NULL);
#ifdef ASSERT
  _bci    = kit->bci();
  Parse* parser = kit->is_Parse();
  int block = (parser == NULL || parser->block() == NULL) ? -1 : parser->block()->pre_order();
  _block  = block;
#endif
}
PreserveJVMState::~PreserveJVMState() {
  GraphKit* kit = _kit;
#ifdef ASSERT
  assert(kit->bci() == _bci, "bci must not shift");
  Parse* parser = kit->is_Parse();
  int block = (parser == NULL || parser->block() == NULL) ? -1 : parser->block()->pre_order();
  assert(block == _block,    "block must not shift");
#endif
  kit->set_map(_map);
  kit->set_sp(_sp);
}


//-----------------------------BuildCutout-------------------------------------
BuildCutout::BuildCutout(GraphKit* kit, Node* p, float prob, float cnt)
  : PreserveJVMState(kit)
{
  assert(p->is_Con() || p->is_Bool(), "test must be a bool");
  SafePointNode* outer_map = _map;   // preserved map is caller's
  SafePointNode* inner_map = kit->map();
  IfNode* iff = kit->create_and_map_if(outer_map->control(), p, prob, cnt);
  outer_map->set_control(kit->gvn().transform( new (1) IfTrueNode(iff) ));
  inner_map->set_control(kit->gvn().transform( new (1) IfFalseNode(iff) ));
}
BuildCutout::~BuildCutout() {
  GraphKit* kit = _kit;
  assert(kit->stopped(), "cutout code must stop, throw, return, etc.");
}


//------------------------------clone_map--------------------------------------
// Implementation of PreserveJVMState
//
// Only clone_map(...) here. If this function is only used in the
// PreserveJVMState class we may want to get rid of this extra
// function eventually and do it all there.

SafePointNode* GraphKit::clone_map() {
  if (map() == NULL)  return NULL;
  
  // Clone the memory edge first
  Node *mem = MergeMemNode::clone_all_memory(map()->memory());
  gvn().set_type_bottom(mem);

  SafePointNode *clonemap = (SafePointNode*)map()->clone();
  JVMState* jvms = this->jvms();
  JVMState* clonejvms = jvms->clone_shallow();
  clonemap->set_memory(mem);
  clonemap->set_jvms(clonejvms);
  clonejvms->set_map(clonemap);
  record_for_igvn(clonemap);
  gvn().set_type_bottom(clonemap);
  return clonemap;
}


//-----------------------------set_map_clone-----------------------------------
void GraphKit::set_map_clone(SafePointNode* m) {
  _map = m;
  _map = clone_map();
  _map->set_next_exception(NULL);
  debug_only(verify_map());
}


//----------------------------kill_dead_locals---------------------------------
// Detect any locals which are known to be dead, and force them to top.
void GraphKit::kill_dead_locals() {
  // Consult the liveness information for the locals.  If any
  // of them are unused, then they can be replaced by top().  This
  // should help register allocation time and cut down on the size
  // of the deoptimization information.

  // This call is made from many of the bytecode handling
  // subroutines called from the Big Switch in do_one_bytecode.
  // Every bytecode which might include a slow path is responsible
  // for killing its dead locals.  The more consistent we
  // are about killing deads, the fewer useless phis will be
  // constructed for them at various merge points.

  // bci can be -1 (InvocationEntryBci).  We return the entry
  // liveness for the method.

  if (method() == NULL || method()->code_size() == 0) {
    // We are building a graph for a call to a native method.
    // All locals are live.
    return;
  }

  ResourceMark rm;

  // Consult the liveness information for the locals.  If any
  // of them are unused, then they can be replaced by top().  This
  // should help register allocation time and cut down on the size
  // of the deoptimization information.
  BitMap live_locals = method()->liveness_at_bci(bci());
  
  int len = (int)live_locals.size();
  assert(len <= jvms()->loc_size(), "too many live locals");
  for (int local = 0; local < len; local++) {
    if (!live_locals.at(local)) {
      set_local(local, top());
    }
  }
}

#ifdef ASSERT
//-------------------------dead_locals_are_killed------------------------------
// Return true if all dead locals are set to top in the map.
// Used to assert "clean" debug info at various points.
bool GraphKit::dead_locals_are_killed() {
  if (method() == NULL || method()->code_size() == 0) {
    // No locals need to be dead, so all is as it should be.
    return true;
  }

  // Make sure somebody called kill_dead_locals upstream.
  ResourceMark rm;
  for (JVMState* jvms = this->jvms(); jvms != NULL; jvms = jvms->caller()) {
    if (jvms->loc_size() == 0)  continue;  // no locals to consult
    SafePointNode* map = jvms->map();
    ciMethod* method = jvms->method();
    int       bci    = jvms->bci();
    if (jvms == this->jvms()) {
      bci = this->bci();  // it might not yet be synched
    }
    BitMap live_locals = method->liveness_at_bci(bci);
    int len = (int)live_locals.size();
    if (len == 0)
      // This method is trivial, or is poisoned by a breakpoint.
      return true;
    assert(len == jvms->loc_size(), "live map consistent with locals map");
    for (int local = 0; local < len; local++) {
      if (!live_locals.at(local) && map->local(jvms, local) != top()) {
        if (PrintMiscellaneous && (Verbose || WizardMode)) {
          tty->print_cr("Zombie local %d: ", local);
          jvms->dump();
        }
        return false;
      }
    }
  }
  return true;
}

#endif //ASSERT

// Helper function for adding debug information to node
void GraphKit::add_safepoint_edges(SafePointNode* call, bool must_throw) {
  // Add the safepoint edges to the call (or other safepoint).

  // Make sure dead locals are set to top.  This
  // should help register allocation time and cut down on the size
  // of the deoptimization information.
  assert(dead_locals_are_killed(), "garbage in debug info before safepoint");

  // Walk the inline list to fill in the correct set of JVMState's
  // Also fill in the associated edges for each JVMState.

  JVMState* youngest_jvms = sync_jvms();

  // Do we need debug info here?  If it is a SafePoint and this method
  // cannot de-opt, then we do NOT need any debug info.
  bool full_info = (C->deopt_happens() || call->Opcode() != Op_SafePoint);

  // If we are guaranteed to throw, we can prune everything but the
  // input to the current bytecode.
  bool can_prune_locals = false;
  uint stack_slots_not_pruned = 0;
  if (must_throw) {
    assert(method() == youngest_jvms->method(), "sanity");
    Bytecodes::Code code = java_bc();
    if (code != Bytecodes::_illegal) {
      BasicType rtype = Bytecodes::result_type(code); // checkcast=P, athrow=V
      uint      depth = Bytecodes::depth(code);       // checkcast=0, athrow=-1
      uint      inputs = - ((int)depth);
      if (rtype != T_ILLEGAL) {
        inputs += type2size[rtype];  // output reduces apparent negative depth
      } else {
        switch (code) {
        case Bytecodes::_putfield:
        case Bytecodes::_getfield:
        case Bytecodes::_putstatic:
        case Bytecodes::_getstatic:
          // conservative estimate; don't bother to refine
          inputs += type2size[T_LONG];
          break;
        default:
          // Must be an invoke.  Keep the whole stack.
          inputs = max_uint;
          break;
        }
      }
      // inputs:  checkcast=1, athrow=1
      can_prune_locals = true;
      stack_slots_not_pruned = inputs;
    }
  }

  if (JvmtiExport::can_examine_or_deopt_anywhere()) {
    // At any safepoint, this method can get breakpointed, which would
    // then require an immediate deoptimization.
    full_info = true;
    can_prune_locals = false;  // do not prune locals
    stack_slots_not_pruned = 0;
  }

  // do not scribble on the input jvms
  JVMState* out_jvms = youngest_jvms->clone_deep();
  call->set_jvms(out_jvms); // Start jvms list for call node

  // Presize the call:
  debug_only(uint non_debug_edges = call->req());
  call->add_req_batch(top(), youngest_jvms->debug_depth());
  assert(call->req() == non_debug_edges + youngest_jvms->debug_depth(), "");

  // Set up edges so that the call looks like this:
  //  Call [state:] ctl io mem fptr retadr
  //       [parms:] parm0 ... parmN
  //       [root:]  loc0 ... locN stk0 ... stkSP mon0 obj0 ... monN objN
  //    [...mid:]   loc0 ... locN stk0 ... stkSP mon0 obj0 ... monN objN [...]
  //       [young:] loc0 ... locN stk0 ... stkSP mon0 obj0 ... monN objN
  // Note that caller debug info precedes callee debug info.

  // Fill pointer walks backwards from "young:" to "root:" in the diagram above:
  uint debug_ptr = call->req();

  // Loop over the map input edges associated with jvms, add them
  // to the call node, & reset all offsets to match call node array.
  for (JVMState* in_jvms = youngest_jvms; in_jvms != NULL; ) {
    uint debug_end   = debug_ptr;
    uint debug_start = debug_ptr - in_jvms->debug_size();
    debug_ptr = debug_start;  // back up the ptr

    uint p = debug_start;  // walks forward in [debug_start, debug_end)
    uint j, k, l;
    SafePointNode* in_map = in_jvms->map();
    out_jvms->set_map(call);

    if (can_prune_locals) {
      assert(in_jvms->method() == out_jvms->method(), "sanity");
      // If the current throw can reach an exception handler in this JVMS,
      // then we must keep everything live that can reach that handler.
      // As a quick and dirty approximation, we look for any handlers at all.
      if (in_jvms->method()->has_exception_handlers()) {
        can_prune_locals = false;
      }
    }

    // Add the Locals
    k = in_jvms->locoff();
    l = in_jvms->loc_size();
    out_jvms->set_locoff(p);
    if (full_info && !can_prune_locals) {
      for (j = 0; j < l; j++)
        call->set_req(p++, in_map->in(k+j));
    } else {
      p += l;  // already set to top above by add_req_batch
    }

    // Add the Expression Stack
    k = in_jvms->stkoff();
    l = in_jvms->sp();
    out_jvms->set_stkoff(p);
    if (full_info && !can_prune_locals) {
      for (j = 0; j < l; j++)
        call->set_req(p++, in_map->in(k+j));
    } else if (can_prune_locals && stack_slots_not_pruned != 0) {
      // Divide stack into {S0,...,S1}, where S0 is set to top.
      uint s1 = stack_slots_not_pruned;
      stack_slots_not_pruned = 0;  // for next iteration
      if (s1 > l)  s1 = l;
      uint s0 = l - s1;
      p += s0;  // skip the tops preinstalled by add_req_batch
      for (j = s0; j < l; j++)
        call->set_req(p++, in_map->in(k+j));
    } else {
      p += l;  // already set to top above by add_req_batch
    }

    // Add the Monitors
    k = in_jvms->monoff();
    l = in_jvms->mon_size();
    out_jvms->set_monoff(p);
    for (j = 0; j < l; j++)
      call->set_req(p++, in_map->in(k+j));

    // Finish the new jvms.
    out_jvms->set_endoff(p);

    assert(out_jvms->endoff()     == debug_end,             "fill ptr must match");
    assert(out_jvms->depth()      == in_jvms->depth(),      "depth must match");
    assert(out_jvms->loc_size()   == in_jvms->loc_size(),   "size must match");
    assert(out_jvms->mon_size()   == in_jvms->mon_size(),   "size must match");
    assert(out_jvms->debug_size() == in_jvms->debug_size(), "size must match");

    // Update the two tail pointers in parallel.
    out_jvms = out_jvms->caller();
    in_jvms  = in_jvms->caller();
  }

  assert(debug_ptr == non_debug_edges, "debug info must fit exactly");

  // Test the correctness of JVMState::debug_xxx accessors:
  assert(call->jvms()->debug_start() == non_debug_edges, "");
  assert(call->jvms()->debug_end()   == call->req(), "");
  assert(call->jvms()->debug_depth() == call->req() - non_debug_edges, "");
}


//------------------------------basic_plus_adr---------------------------------
Node* GraphKit::basic_plus_adr(Node* base, Node* ptr, intptr_t offset) {
  return _gvn.transform( new (4) AddPNode(base,ptr,_gvn.MakeConX(offset)) );
}


//-----------------------------array_length------------------------------------
Node* GraphKit::array_length(Node* array) {
  Node *r_adr = basic_plus_adr(array, array, arrayOopDesc::length_offset_in_bytes());
  return _gvn.transform( new (3) LoadRangeNode(0, memory(TypeAryPtr::RANGE), r_adr, TypeInt::POS));
}

//------------------------------do_null_check----------------------------------
// Helper function to do a NULL pointer check.  Returned value is 
// the incoming address with NULL casted away.  You are allowed to use the
// not-null value only if you are control dependent on the test.
extern int explicit_null_checks_inserted, 
           explicit_null_checks_elided;
Node* GraphKit::null_check_common(Node* value, BasicType type, bool assert_null) {
  if (!GenerateCompilerNullChecks && !assert_null) {
    // For some performance testing, we may wish to suppress null checking.
    value = cast_not_null(value);   // Make it appear to be non-null (4962416).
    return value;
  }
  explicit_null_checks_inserted++;

  // Construct NULL check
  Node *chk = NULL;
  switch(type) {
    case T_LONG   : chk = new (3) CmpLNode(value, _gvn.zerocon(T_LONG)); break;
    case T_INT    : chk = new (3) CmpINode( value, _gvn.intcon(0)); break;
    case T_OBJECT : // fall through
    case T_ARRAY  : {
      const Type *t = _gvn.type( value );

      const TypeInstPtr* tp = t->isa_instptr();
      if (tp != NULL && !tp->klass()->is_loaded() && !assert_null) {
        // Usually, any field access or invocation on an unloaded oop type
        // will simply fail to link, since the statically linked class is
        // likely also to be unloaded.  However, in -Xcomp mode, sometimes
        // the static class is loaded but the sharper oop type is not.
        // Rather than checking for this obscure case in lots of places,
        // we simply observe that a null check on an unloaded class
        // will always be followed by a nonsense operation, so we
        // can just issue the uncommon trap here.
        // Our access to the unloaded class will only be correct
        // after it has been loaded and initialized, which requires
        // a trip through the interpreter.
        if (WizardMode) { tty->print("Null check of unloaded "); tp->klass()->print(); tty->cr(); }
        uncommon_trap(Deoptimization::Reason_unloaded,
                      Deoptimization::Action_reinterpret,
                      tp->klass(), "!loaded");
        return top();
      }

      if (assert_null) {
        // See if the type is contained in NULL_PTR.
        // If so, then the value is already null.
        if (t->higher_equal(TypePtr::NULL_PTR)) {
          explicit_null_checks_elided++;  
          return value;           // Elided null assert quickly!
        }
      } else {
        // See if mixing in the NULL pointer changes type.
        // If so, then the NULL pointer was not allowed in the original
        // type.  In other words, "value" was not-null.
        if (t->meet(TypePtr::NULL_PTR) != t) {
          // same as: if (!TypePtr::NULL_PTR->higher_equal(t)) ...
          explicit_null_checks_elided++;  
          return value;           // Elided null check quickly!
        }
      }
      chk = new (3) CmpPNode( value, null() );
      break;    
    }

    default      : ShouldNotReachHere();
  }
  assert(chk != NULL, "sanity check"); 
  chk = _gvn.transform(chk);

  BoolTest::mask btest = assert_null ? BoolTest::eq : BoolTest::ne;
  BoolNode *btst = new (2) BoolNode( chk, btest);
  Node   *tst = _gvn.transform( btst );

  //-----------
  // if peephole optimizations occured, a prior test existed.
  // If a prior test existed, maybe it dominates as we can avoid this test.
  if( tst != btst && (type == T_ARRAY || type == T_OBJECT) ) {
    // At this point we want to scan up the CFG to see if we can
    // find an identical test (and so avoid this test altogether).
    Node *cfg = control();
    int depth = 0;
    while( depth < 16 ) {       // Limit search depth for speed
      if( !cfg->in(0) ) {
        cfg = cfg->nonnull_req();
        continue;
      }
      if( cfg->Opcode() == Op_IfTrue &&
          cfg->in(0)->in(1) == tst ) {
        // Found prior test.  Use "cast_not_null" to construct an identical
        // CastPP (and hence hash to) as already exists for the prior test.
        // Return that casted value.
        if (assert_null) {
          replace_in_map(value, null());
          return null();  // do not issue the redundant test
        }
        Node *oldcontrol = control();
        set_control(cfg);
        Node *res = cast_not_null(value);
        set_control(oldcontrol);
        explicit_null_checks_elided++;  
        return res;
      }
      Node *next = cfg->in(0);  // Follow up the CFG chain
      if( next == cfg ) break;  // Quit at region nodes
      cfg = next;
      depth++;
    }
  }

  //-----------
  // Branch to failure if null
  float notNullProb;

  // To cause an implicit null check, we set the not-null probability
  // to the maximum (PROB_MAX).  For an explicit check the probablity
  // is set to a smaller value.
  if (ImplicitNullCheckThreshold > 0 && method() &&
      (method()->method_data()->trap_count(Deoptimization::Reason_null_check)
       >= (uint)ImplicitNullCheckThreshold)) {
    notNullProb =  PROB_LIKELY_MAG(3);
  } else {
    notNullProb =  PROB_MAX;
  }

  { BuildCutout unless(this, tst, notNullProb);
    // Check for optimizer eliding test at parse time
    if (stopped()) {
      // Failure not possible; do not bother making uncommon trap.
      explicit_null_checks_elided++;
    } else if (assert_null) {
      uncommon_trap(Deoptimization::Reason_unhandled,
                    Deoptimization::Action_make_not_entrant,
                    NULL, "assert_null");
    } else if (type == T_ARRAY || type == T_OBJECT) {
      builtin_throw(Deoptimization::Reason_null_check);
    } else {
      builtin_throw(Deoptimization::Reason_div0_check);
    }
  }

  // Must throw exception, fall-thru not possible?
  if (stopped()) {
    return top();               // No result
  }

  if (assert_null) {
    // Cast obj to null on this path.
    replace_in_map(value, zerocon(type));
    return zerocon(type);
  }

  // Cast obj to not-null on this path
  return (type == T_ARRAY || type == T_OBJECT) ? cast_not_null(value) : value;
}


//------------------------------cast_not_null----------------------------------
// Cast obj to not-null on this path
Node* GraphKit::cast_not_null(Node* obj) {
  const Type *t = _gvn.type(obj);
  const Type *t_not_null = t->join(TypePtr::NOTNULL);
  // Object is already not-null?
  if( t == t_not_null ) return obj;

  Node *cast = new (2) CastPPNode(obj,t_not_null);
  cast->set_req(0,control());
  cast = _gvn.transform( cast );

  // Scan for instances of 'obj' in the current JVM mapping.
  // These instances are known to be not-null after the test.
  replace_in_map(obj, cast);

  return cast;                  // Return casted value
}


//--------------------------replace_in_map-------------------------------------
void GraphKit::replace_in_map(Node* old, Node* neww) {
  SafePointNode* map = this->map();
  for (uint i = 0; i < map->req(); i++) {
    if (map->in(i) == old)  map->set_req(i, neww);
  }
  // We can consider replacing in caller maps.
  // The idea would be that an inlined function's null checks
  // can be shared with the entire inlining tree.
  // The expense of doing this is that the PreserveJVMState class
  // would have to preserve caller states too, with a deep copy.
}



//=============================================================================
//--------------------------------memory---------------------------------------
Node* GraphKit::memory(uint alias_idx) {
  MergeMemNode* mem = merged_memory();
  Node* p = mem->memory_at(alias_idx);
  _gvn.set_type(p, Type::MEMORY);  // must be mapped
  return p;
}

//-----------------------------reset_memory------------------------------------
Node* GraphKit::reset_memory() {
  Node* mem = map()->memory();
  // do not use this node for any more parsing!
  debug_only( map()->set_memory((Node*)NULL) );
  return _gvn.transform( mem ); 
}

//------------------------------set_all_memory---------------------------------
void GraphKit::set_all_rewritable_memory(Node* newmem) {
#if 1 // %%% Fix: "#else" code doesn't work with incremental inlining.
  set_all_memory(newmem);
#else
  // This code routes write-once memory states directly around a call.
  // However, this will lead to a write getting dropped if the call is
  // later inlined and includes a new write to the write-once memory.
  // I suppose we can resurrect this trick later when we get bi-di edges.
  assert(newmem->Opcode() != Op_MergeMem, "new memory is a projection of a call");
  if (stopped()) { set_all_memory(newmem); return; }
  MergeMemNode* oldm = map()->memory()->is_MergeMem();
  set_all_memory(newmem);

  if (oldm == NULL)  return;  // cannot extract any slices
  MergeMemNode* newm = merged_memory();

  // now, copy selected slices from oldm into newm
  for (MergeMemStream mms(newm, oldm); mms.next_non_empty2(); ) {
    if( ! C->_map_adr_mem_is_rewritable[mms.alias_idx()] )
      mms.set_memory(mms.memory2());
  }
#endif
}

//------------------------------set_all_memory---------------------------------
void GraphKit::set_all_memory(Node* newmem) {
  Node *mergemem = new MergeMemNode(newmem);
  map()->set_memory(mergemem);
  _gvn.set_type(mergemem, Type::MEMORY);
}

//------------------------------set_all_memory_call----------------------------
void GraphKit::set_all_memory_call(Node* call) {
  Node* newmem = _gvn.transform( new (1) ProjNode(call, TypeFunc::Memory) );
  if( call->is_SafePoint() || call->is_MemBar() ) {
    // The call or pause cannot possibly modify any immutable memory state we can observe.
    set_all_rewritable_memory( newmem );
  } else {
    assert( call->Opcode() != Op_MergeMem, "" );
    assert(!call->is_Call() || !call->is_Call()->is_CallJava() ||
           !call->is_Call()->is_CallJava()->method(),
           "this runtime call must not redefine all of memory");
    set_all_memory( newmem );
  }
}

//=============================================================================
//
// parser factory methods for MemNodes
//
// These are layered on top of the factory methods in LoadNode and StoreNode,
// and integrate with the parser's memory state and _gvn engine.
//

// factory methods in "int adr_idx"
Node* GraphKit::make_load(Node* ctl, Node* adr, const Type* t, BasicType bt, int adr_idx) {
  assert(adr_idx != Compile::AliasIdxTop, "use other make_load factory" );
  const TypePtr* adr_type = NULL; // debug-mode-only argument
  debug_only(adr_type = C->get_adr_type(adr_idx));
  Node* mem = memory(adr_idx);
  return _gvn.transform( LoadNode::make( ctl, mem, adr, adr_type, t, bt ) );
}

Node* GraphKit::store_to_memory(Node* ctl, Node* adr, Node *val, BasicType bt, int adr_idx) {
  assert(adr_idx != Compile::AliasIdxTop, "use other store_to_memory factory" );
  const TypePtr* adr_type = NULL;
  debug_only(adr_type = C->get_adr_type(adr_idx));
  Node *mem = memory(adr_idx);
  Node* st = _gvn.transform( StoreNode::make( ctl, mem, adr, adr_type, val, bt ) );
  set_memory(st, adr_idx);
  // Back-to-back stores can only remove intermediate store with DU info
  // so push on worklist for optimizer.
  if (mem->req() > MemNode::Address && adr == mem->in(MemNode::Address))  
    record_for_igvn(st);

  return st;
}


//-------------------------set_arguments_for_java_call-------------------------
// Arguments (pre-popped from the stack) are taken from the JVMS.
void GraphKit::set_arguments_for_java_call(CallJavaNode* call) {
  // Add the call arguments:
  uint nargs = call->method()->arg_size();
  for (uint i = 0; i < nargs; i++) {
    Node* arg = argument(i);
    call->set_req(i + TypeFunc::Parms, arg);
  }
}

//---------------------------set_edges_for_java_call---------------------------
// Connect a newly created call into the current JVMS.
// A return value node (if any) is returned from set_edges_for_java_call.
void GraphKit::set_edges_for_java_call(CallJavaNode* call, bool must_throw) {

  // Add the predefined inputs:
  call->set_req( TypeFunc::Control, control() );
  call->set_req( TypeFunc::I_O    , i_o() );
  call->set_req( TypeFunc::Memory , reset_memory() );
  call->set_req( TypeFunc::ReturnAdr, top() );
  call->set_req( TypeFunc::FramePtr, frameptr() );

  add_safepoint_edges(call, must_throw);

  Node* xcall = _gvn.transform(call);

  if (xcall == top()) {
    set_control(top());
    return;
  }
  assert(xcall == call, "call identity is stable");

  // Re-use the current map to produce the result.

  set_control(_gvn.transform(new(1) ProjNode(call, TypeFunc::Control)));
  set_i_o(    _gvn.transform(new(1) ProjNode(call, TypeFunc::I_O    )));
  set_all_memory_call(xcall);

  //return xcall;   // no need, caller already has it
}

Node* GraphKit::set_results_for_java_call(CallJavaNode* call) {
  if (stopped())  return top();  // maybe the call folded up?

  // Capture the return value, if any.
  Node* ret;
  if (call->method() == NULL ||
      call->method()->return_type()->basic_type() == T_VOID)
        ret = top();
  else  ret = _gvn.transform(new(1) ProjNode(call, TypeFunc::Parms));

  // Note:  Since any out-of-line call can produce an exception,
  // we always insert an I_O projection from the call into the result.

  make_slow_call_ex(call, env()->Throwable_klass());

  return ret;
}

//--------------------set_predefined_input_for_runtime_call--------------------
// Reading and setting the memory state is way conservative here.
// The real problem is that I am not doing real Type analysis on memory,
// so I cannot distinguish card mark stores from other stores.  Across a GC
// point the Store Barrier and the card mark memory has to agree.  I cannot
// have a card mark store and its barrier split across the GC point from
// either above or below.  Here I get that to happen by reading ALL of memory.
// A better answer would be to seperate out card marks from other memory.
void GraphKit::set_predefined_input_for_runtime_call(SafePointNode* call) {
  // Set fixed predefined input arguments
  call->set_req( TypeFunc::Control, control() );
  call->set_req( TypeFunc::I_O    , top() )     ;   // does no i/o
  call->set_req( TypeFunc::Memory , reset_memory() ); // may gc ptrs
  call->set_req( TypeFunc::ReturnAdr, top() );
  call->set_req( TypeFunc::FramePtr, frameptr() );
}

//-------------------set_predefined_output_for_runtime_call--------------------
// Set control and memory (not i_o) from the call.
// If keep_mem is not NULL, use it for the output state,
// except for the RawPtr output of the call.
void GraphKit::set_predefined_output_for_runtime_call(Node* call, MergeMemNode* keep_mem) {
  // no i/o
  set_control(_gvn.transform( new (1) ProjNode(call,TypeFunc::Control) ));
  if (keep_mem) {
    // First clone the existing memory state
    set_all_memory(keep_mem);
    // Make memory for the call
    Node* mem = _gvn.transform( new (1) ProjNode(call, TypeFunc::Memory) );
    // Set the RawPtr memory state only.  This covers all the heap top/GC stuff
    set_memory(mem, C->AliasLevel() == 0 ? Compile::AliasIdxBot : Compile::AliasIdxRaw);
  } else {
    // This is not a "slow path" call; all memory comes from the call.
    set_all_memory_call(call);
  }
} 

//------------------------------increment_counter------------------------------
// for statistics: increment a VM counter by 1

void GraphKit::increment_counter(address counter_addr) {
  Node* adr1 = makecon(TypeRawPtr::make(counter_addr));
  increment_counter(adr1);
}

void GraphKit::increment_counter(Node* counter_addr) {
  const TypePtr* adr_type = TypeRawPtr::BOTTOM;
  Node* cnt  = make_load(NULL, counter_addr, TypeInt::INT, T_INT, adr_type);
  Node* incr = _gvn.transform(new (3) AddINode(cnt, _gvn.intcon(1)));
  store_to_memory( NULL, counter_addr, incr, T_INT, adr_type );
}


//------------------------------uncommon_trap----------------------------------
// Bail out to the interpreter in mid-method.  Implemented by calling the
// uncommon_trap blob.  This helper function inserts a runtime call with the
// right debug info.  
void GraphKit::uncommon_trap(int trap_request,
                             ciKlass* klass, const char* comment,
                             bool must_throw) {
  if (failing())  stop();
  if (stopped())  return; // trap reachable?

  // Note:  If ProfileTraps is true, and if a deopt. actually
  // occurs here, the runtime will make sure an MDO exists.  There is
  // no need to call method()->build_method_data() at this point.

  Deoptimization::DeoptReason reason = Deoptimization::trap_request_reason(trap_request);
  Deoptimization::DeoptAction action = Deoptimization::trap_request_action(trap_request);

  switch (action) {
  case Deoptimization::Action_maybe_recompile:
  case Deoptimization::Action_reinterpret:
    if (Deoptimization::trap_request_index(trap_request) < 0
        && too_many_recompiles(reason)) {
      // This BCI is causing too many recompilations.
      action = Deoptimization::Action_none;
      trap_request = Deoptimization::make_trap_request(reason, action);
    } else {
      C->set_trap_can_recompile(true);
    }
    break;
  case Deoptimization::Action_make_not_entrant:
    C->set_trap_can_recompile(true);
    break;
#ifdef ASSERT
  case Deoptimization::Action_none:
  case Deoptimization::Action_make_not_compilable:
    break;
  default:
    assert(false, "bad action");
#endif
  }

  if (TraceOptoParse) {
    char buf[100];
    tty->print_cr("Uncommon trap %s at bci:%d",
                  Deoptimization::format_trap_request(buf, sizeof(buf),
                                                      trap_request), bci());
  }

  CompileLog* log = C->log();
  if (log != NULL) {
    int kid = (klass == NULL)? -1: log->identify(klass);
    log->begin_elem("uncommon_trap bci='%d'", bci());
    char buf[100];
    log->print(" %s", Deoptimization::format_trap_request(buf, sizeof(buf),
                                                          trap_request));
    if (kid >= 0)         log->print(" klass='%d'", kid);
    if (comment != NULL)  log->print(" comment='%s'", comment);
    log->end_elem();
  }

  // Make sure any guarding test views this path as very unlikely
  if (control()->in(0)) {
    IfNode *iff = control()->in(0)->is_If();
    if (iff != NULL) {        // Found a guarding if test?
      float f = iff->_prob;   // Get prob
      if (control()->Opcode() == Op_IfTrue) {
        if (f > PROB_LIKELY_MAG(4))
          iff->_prob = PROB_MIN;
      } else {
        if (f < PROB_UNLIKELY_MAG(4))
          iff->_prob = PROB_MAX;
      }
    }
  }

  // Clear out dead values from the debug info.
  kill_dead_locals();

  // Now insert the uncommon trap subroutine call
  CallNode *call =
    new CallStaticJavaNode(
      OptoRuntime::uncommon_trap_Type(),
      OptoRuntime::uncommon_trap_blob()->instructions_begin(),
      "uncommon_trap", 
      bci()
    );

  set_predefined_input_for_runtime_call(call);
  call->set_req(TypeFunc::I_O, i_o());
  call->set_req(TypeFunc::ReturnAdr, returnadr());

  // Pass the index of the class to be loaded
  call->set_req(TypeFunc::Parms, _gvn.intcon(trap_request));

  // The debug info is the only real input to this call.
  add_safepoint_edges(call, must_throw);

  Node *c = _gvn.transform(call);
  Node *ctrl = _gvn.transform(new (1) ProjNode(c,TypeFunc::Control));

  // Halt-and-catch fire here.  The above call should never return!
  HaltNode *halt = new HaltNode(ctrl,frameptr());
  _gvn.set_type_bottom(halt);
  root()->add_req(halt);

  stop();
}


//-----------------------------too_many_traps----------------------------------
// Report if there are too many traps at the current method and bci.
// Return true if there was a trap, and/or PerMethodTrapLimit is exceeded.
bool GraphKit::too_many_traps(Deoptimization::DeoptReason reason) {
  ciMethodData* md = method()->method_data();
  if (md->is_empty()) {
    // Assume the trap has not occurred, or that it occurred only
    // because of a transient condition during start-up in the interpreter.
    return false;
  }
  if (md->has_trap_at(bci(), reason) != 0) {
    // Assume PerBytecodeTrapLimit==0, for a more conservative heuristic.
    // Also, if there are multiple reasons, or if there is no per-BCI record,
    // assume the worst.
    if (C->log())
      C->log()->elem("observe trap='%s' count='%d'",
                     Deoptimization::trap_reason_name(reason),
                     md->trap_count(reason));
    return true;
  } else if (C->trap_count(reason) >= (uint)PerMethodTrapLimit) {
    // Too many traps globally.
    // Note that we use cumulative C->trap_count, not just md->trap_count.
    if (C->log())
      C->log()->elem("observe trap='%s' count='0' mcount='%d' ccount='%d'",
                     Deoptimization::trap_reason_name(reason),
                     md->trap_count(reason), C->trap_count(reason));
    return true;
  } else {
    // The coast is clear.
    return false;
  }
}

//--------------------------too_many_recompiles--------------------------------
// Report if there are too many recompiles at the current method and bci.
// Consults PerBytecodeRecompilationCutoff and PerMethodRecompilationCutoff.
// Is not eager to return true, since this will cause the compiler to use
// Action_none for a trap point, to avoid too many recompilations.
bool GraphKit::too_many_recompiles(Deoptimization::DeoptReason reason) {
  ciMethodData* md = method()->method_data();
  if (md->is_empty()) {
    // Assume the trap has not occurred, or that it occurred only
    // because of a transient condition during start-up in the interpreter.
    return false;
  }
  // Pick a cutoff point well within PerBytecodeRecompilationCutoff.
  uint bc_cutoff = (uint) PerBytecodeRecompilationCutoff / 8;
  uint m_cutoff  = (uint) PerMethodRecompilationCutoff / 2 + 1;  // not zero
  if ((!Deoptimization::reason_is_recorded_per_bytecode(reason)
       || md->has_trap_at(bci(), reason) != 0)
      // The trap frequency measure we care about is the recompile count:
      && md->trap_recompiled_at(bci())
      && md->overflow_recompile_count() >= bc_cutoff) {
    // Do not emit a trap here if it has already caused recompilations.
    // Also, if there are multiple reasons, or if there is no per-BCI record,
    // assume the worst.
    if (C->log())
      C->log()->elem("observe trap='%s recompiled' count='%d' recompiles2='%d'",
                     Deoptimization::trap_reason_name(reason),
                     md->trap_count(reason),
                     md->overflow_recompile_count());
    return true;
  } else if (C->trap_count(reason) != 0
             && C->decompile_count() >= m_cutoff) {
    // Too many recompiles globally, and we have seen this sort of trap.
    // Use cumulative C->decompile_count, not just md->decompile_count.
    if (C->log())
      C->log()->elem("observe trap='%s' count='%d' mcount='%d' decompiles='%d' mdecompiles='%d'",
                     Deoptimization::trap_reason_name(reason),
                     md->trap_count(reason), C->trap_count(reason),
                     md->decompile_count(), C->decompile_count());
    return true;
  } else {
    // The coast is clear.
    return false;
  }
}


//------------------------------store_barrier----------------------------------
// Insert a write-barrier store.  This is to let generational GC work; we have 
// to flag all oop-stores before the next GC point.
void GraphKit::store_barrier(Node *oop_store, Node* adr, Node* val) {
  // No store check needed if we're storing a NULL or an old object
  // (latter case is probably a string constant). The concurrent
  // mark sweep garbage collector, however, needs to have all
  // oop updates flagged via card-marks.
  if (val->is_Con() && !UseConcMarkSweepGC) {
    // must be either an oop or NULL
    const Type* t = val->bottom_type();
    assert( t == Type::TOP || t == TypePtr::NULL_PTR || t->is_oopptr()->const_oop() != NULL, 
            "must be either a constant oop or NULL");
    // no store barrier needed, because no old-to-new ref created
    return;
  }
  // Get the alias_index for raw card-mark memory
  const TypePtr* adr_type = TypeRawPtr::BOTTOM;
  // Convert the pointer to an int prior to doing math on it
  Node *cast = _gvn.transform(new (2) CastP2XNode(control(), adr));
  // Divide by card size
  assert(Universe::heap()->barrier_set()->kind() == BarrierSet::CardTableModRef,
         "Only one we handle so far.");
  CardTableModRefBS* ct =
    (CardTableModRefBS*)(Universe::heap()->barrier_set());
  Node *a = _gvn.transform(new (3) URShiftXNode( cast, _gvn.intcon(CardTableModRefBS::card_shift) ));
  // We store into a byte array, so do not bother to left-shift by zero
  Node *b = a;
  // Get base of card map
  assert(sizeof(*ct->byte_map_base) == sizeof(jbyte),
         "adjust this code");
  Node *c = makecon(TypeRawPtr::make((address)ct->byte_map_base));
  // Combine
  Node *sb_ctl = control();
  Node *sb_adr = _gvn.transform(new (4) AddPNode( top()/*no base ptr*/, c, b ));
  Node *sb_val = _gvn.intcon(0);
  BasicType bt = T_BYTE;
  // Smash zero into card
  if( !UseConcMarkSweepGC ) {
    store_to_memory(sb_ctl, sb_adr, sb_val, bt, adr_type);
  } else {
    // Specialized path for CM store barrier
    cms_card_mark( sb_ctl, sb_adr, sb_val, bt, adr_type, oop_store);
  }
}

// Specialized path for CMS store barrier
void GraphKit::cms_card_mark(Node* ctl, Node* adr, Node* val, BasicType bt, const TypePtr* adr_type, Node *oop_store) {
  int adr_idx = C->get_alias_index(adr_type);
  Node *mem = memory(adr_idx);

  // The type input is NULL in PRODUCT builds
  const TypePtr* type = NULL;
  debug_only(type = C->get_adr_type(adr_idx));

  // Add required edge to oop_store, optimizer does not support precedence edges.
  // Convert required edge to precedence edge before allocation.
  Node *store = _gvn.transform( new (5) StoreCMNode(ctl, mem, adr, type, val, oop_store) );
  set_memory(store, adr_idx);

  // For CMS, back-to-back card-marks can only remove the first one 
  // and this requires DU info.  Push on worklist for optimizer.
  if (mem->req() > MemNode::Address && adr == mem->in(MemNode::Address))  
    record_for_igvn(store);
}


void GraphKit::round_double_arguments(ciMethod* dest_method) {
  // (Note:  TypeFunc::make has a cache that makes this fast.)
  const TypeFunc* tf    = TypeFunc::make(dest_method);
  int             nargs = tf->_domain->_cnt - TypeFunc::Parms;
  for (int j = 0; j < nargs; j++) {
    const Type *targ = tf->_domain->field_at(j + TypeFunc::Parms);
    if( targ->basic_type() == T_DOUBLE ) {
      // If any parameters are doubles, they must be rounded before
      // the call, dstore_rounding does gvn.transform
      Node *arg = argument(j);
      arg = dstore_rounding(arg);
      set_argument(j, arg);
    }
  }
}

void GraphKit::round_double_result(ciMethod* dest_method) {
  // A non-strict method may return a double value which has an extended 
  // exponent, but this must not be visible in a caller which is 'strict'
  // If a strict caller invokes a non-strict callee, round a double result

  BasicType result_type = dest_method->return_type()->basic_type();
  assert( method() != NULL, "must have caller context");
  if( result_type == T_DOUBLE && method()->is_strict() && !dest_method->is_strict() ) {
    // Destination method's return value is on top of stack
    // dstore_rounding() does gvn.transform
    Node *result = pop_pair();
    result = dstore_rounding(result);
    push_pair(result);
  }
}

// rounding for strict float precision conformance
Node* GraphKit::precision_rounding(Node* n) {
  return UseStrictFP && _method->flags().is_strict() 
    && UseSSE == 0 && Matcher::strict_fp_requires_explicit_rounding
    ? _gvn.transform( new (2) RoundFloatNode(0, n) )
    : n;
}

// rounding for strict double precision conformance
Node* GraphKit::dprecision_rounding(Node *n) {
  return UseStrictFP && _method->flags().is_strict() 
    && UseSSE <= 1 && Matcher::strict_fp_requires_explicit_rounding
    ? _gvn.transform( new (2) RoundDoubleNode(0, n) )
    : n;
}

// rounding for non-strict double stores
Node* GraphKit::dstore_rounding(Node* n) {
  return Matcher::strict_fp_requires_explicit_rounding
    && UseSSE <= 1
    ? _gvn.transform( new (2) RoundDoubleNode(0, n) )
    : n;
}

//=============================================================================
// Generate a fast path/slow path idiom.  Graph looks like:
// [foo] indicates that 'foo' is a parameter
//
//              [in]     NULL
//                 \    /
//                  CmpP
//                  Bool ne
//                   If
//                  /  \
//              True    False-<2>
//              / |        
//             /  cast_not_null
//           Load  |    |   ^
//        [fast_test]   |   |
// gvn to   opt_test    |   |
//          /    \      |  <1>
//      True     False  |
//        |         \\  |
//   [slow_call]     \[fast_result]
//    Ctl   Val       \      \
//     |               \      \ 
//    Catch       <1>   \      \
//   /    \        ^     \      \
//  Ex    No_Ex    |      \      \
//  |       \   \  |       \ <2>  \
//  ...      \  [slow_res] |  |    \   [null_result]
//            \         \--+--+---  |  |
//             \           | /    \ | /
//              --------Region     Phi
//
//=============================================================================
// Code is structured as a series of driver functions all called 'do_XXX' that 
// call a set of helper functions.  Helper functions first, then drivers.

//------------------------------null_check_oop---------------------------------
// Null check oop.  Set null-path control into Region in slot 3.
// Make a cast-not-nullness use the other not-null control.  Return cast.
Node* GraphKit::null_check_oop(RegionNode* region, Node* oop,
                               bool never_see_null) {

  // Initial NULL check
  Node*   cmp = _gvn.transform( new (3) CmpPNode(oop, null()) );
  Node*   tst = _gvn.transform( new (2) BoolNode(cmp, BoolTest::ne) );
  IfNode* iff = create_and_xform_if( control(), tst, never_see_null ? PROB_LIKELY_MAG(5) : PROB_LIKELY_MAG(3), COUNT_UNKNOWN );

  // Initial NULL check taken path
  Node *null_true = _gvn.transform( new (1) IfFalseNode(iff) );
  if (never_see_null) {
    // If we see an unexpected null at a check-cast we record it and force a
    // recompile; the offending check-cast will be compiled to handle NULLs.
    // If we see more than one offending BCI, then all checkcasts in the
    // method will be compiled to handle NULLs.
    PreserveJVMState pjvms(this);
    set_control(null_true);
    uncommon_trap(Deoptimization::Reason_null_check,
                  Deoptimization::Action_make_not_entrant);
    region->set_req(3,top());    // NULL path is dead
  } else {
    region->set_req(3,null_true); 
  }

  // NULL check not-taken path
  Node *null_false = _gvn.transform( new (1) IfTrueNode(iff) );

  // Cast away null-ness on the result
  const Type *t = _gvn.type(oop);
  const Type *t_not_null = t->join(TypePtr::NOTNULL);
  Node *cast = new (2) CastPPNode(oop,t_not_null);
  cast->set_req(0,null_false);
  return cast;
}

//------------------------------opt_iff----------------------------------------
// Optimize the fast-check IfNode.  Set the fast-path region slot 2.
// Return slow-path control.
Node* GraphKit::opt_iff(Node* region, Node* iff) {
  Node *opt_test = _gvn.transform(iff);
  assert( opt_test->is_If(), "Expect an IfNode");
  IfNode *opt_iff = (IfNode*)opt_test;

  // Fast path taken; set region slot 2
  Node *fast_taken = _gvn.transform( new (1) IfFalseNode(opt_iff) );
  region->set_req(2,fast_taken); // Capture fast-control 

  // Fast path not-taken, i.e. slow path
  Node *slow_taken = _gvn.transform( new (1) IfTrueNode(opt_iff) );
  return slow_taken;
}

//------------------------------make_slow_call---------------------------------
Node* GraphKit::make_slow_call(const TypeFunc* slow_call_type, address slow_call, const char* leaf_name, Node* slow_path, Node* parm0, Node* parm1) {

  MergeMemNode* fast_mem = merged_memory(); // Capture and return old memory

  // Slow-path call
  CallNode *call = leaf_name 
    ? (CallNode*)new CallLeafNode      ( slow_call_type, slow_call, leaf_name )
    : (CallNode*)new CallStaticJavaNode( slow_call_type, slow_call, OptoRuntime::stub_name(slow_call), bci() );

  // The following is similar to set_edges_for_java_call,
  // except that the memory effects of the call are restricted to AliasIdxRaw.

  // Slow path call has no side-effects, uses few values
  set_control(slow_path);
  set_predefined_input_for_runtime_call(call);
  if (parm0 != NULL)  call->set_req(TypeFunc::Parms+0, parm0);
  if (parm1 != NULL)  call->set_req(TypeFunc::Parms+1, parm1);
  if( !leaf_name )              // Leaf calls do not block, are not safepoints
    add_safepoint_edges(call);

  Node *c = _gvn.transform(call);

  // Slow path call has no side-effects, sets few values
  set_predefined_output_for_runtime_call(call, fast_mem);

  // Return old memory to call
  return fast_mem;
}

//------------------------------make_merge_mem---------------------------------
// Merge fast path memory with slow path memory
void GraphKit::make_merge_mem(Node* region, Node* call, Node* fast_mem) {
  // ignore call
  merge_fast_memory(fast_mem, region, /*slow_path=*/ 1);
}

//----------------------------merge_fast_memory--------------------------------
// Merge one slow path into the rest of memory.
void GraphKit::merge_fast_memory(Node* fast_mem, Node* region, int slow_path) {

  // Slow path memory comes from the current map (which is from a slow call)
  // Fast path/null path memory comes from the call's input

  // Merge the other fast-memory inputs with the new slow-default memory.
  for (MergeMemStream mms(merged_memory(), fast_mem->is_MergeMem()); mms.next_non_empty2(); ) {
    Node* slow_slice = mms.force_memory();
    Node* fast_slice = mms.memory2();
    if (slow_slice != fast_slice) {
      PhiNode* phi = fast_slice->is_Phi();
      if (phi && phi->region() == region) {
        #ifdef ASSERT
        PhiNode* slow_phi = slow_slice->is_Phi();
        if (slow_phi && slow_phi->region() == region)
          slow_slice = slow_phi->in(slow_path);
        // Caller is responsible for ensuring that any pre-existing
        // phis are already aware of fast_memory.
        assert(phi->in(slow_path) == slow_slice, "pre-existing phis OK");
        #endif
        mms.set_memory(phi);
      } else {
        phi = PhiNode::make(region, fast_slice, Type::MEMORY, mms.adr_type());
        _gvn.set_type(phi, Type::MEMORY);
        phi->set_req(slow_path, slow_slice);
        mms.set_memory(_gvn.transform(phi));
      }
    }
  }
}

//------------------------------make_slow_call_ex------------------------------
// Make the exception handler hookups for the slow call
void GraphKit::make_slow_call_ex(Node* call, ciInstanceKlass* ex_klass) {
  // Make a catch node with just two handlers:  fall-through and catch-all
  Node* i_o  = _gvn.transform( new (1) ProjNode(call, TypeFunc::I_O) );
  Node* catc = _gvn.transform( new CatchNode(control(), i_o, 2) );
  Node* norm = _gvn.transform( new(1) CatchProjNode(catc, CatchProjNode::fall_through_index, CatchProjNode::no_handler_bci) );
  Node* excp = _gvn.transform( new(1) CatchProjNode(catc, CatchProjNode::catch_all_index,    CatchProjNode::no_handler_bci) );

  { PreserveJVMState pjvms(this);
    set_control(excp);
    set_i_o(i_o);

    if (excp != top()) {
      // Create an exception state also.
      // Use an exact type if the caller has specified a specific exception.
      const Type* ex_type = TypeOopPtr::make_from_klass_unique(ex_klass)->cast_to_ptr_type(TypePtr::NotNull);
      Node*       ex_oop  = new(2) CreateExNode(ex_type, control(), i_o);
      add_exception_state(make_exception_state(_gvn.transform(ex_oop)));
    }
  }

  // Get the no-exception control from the CatchNode. 
  set_control(norm);
}  


//-------------------------------gen_subtype_check-----------------------------
// Generate a subtyping check.  Takes as input the subtype and supertype.
// Returns 2 values: sets the default control() to the true path and returns
// the false path.  Only reads invariant memory; sets no (visible) memory.
// The PartialSubtypeCheckNode sets the hidden 1-word cache in the encoding
// but that's not exposed to the optimizer.  This call also doesn't take in an
// Object; if you wish to check an Object you need to load the Object's class
// prior to coming here.
Node *GraphKit::gen_subtype_check( Node *subklass, Node *superklass ) {

  // Shortcut important common case: superklass has NO subtypes and we
  // can check with a simple compare.
  if( _gvn.type(superklass)->singleton() ) {
    bool easy_test = true;
    ciType *superk = _gvn.type(superklass)->is_klassptr()->klass();

    // Object arrays must have their base element have no subtypes
    while( superk->is_obj_array_klass() )
      superk = superk->as_obj_array_klass()->base_element_type();
    // If casting to an instance klass, it must have no subtypes
    if( superk->is_instance_klass() ) {
      ciInstanceKlass* ik = superk->as_instance_klass();
      if( ik->has_subklass() || ik->flags().is_interface() ) 
        easy_test = false;
      // Add a dependency if there is a chance that a subclass will be added later.
      else if (!ik->flags().is_final()) {
        CompileLog* log = C->log();
        if (log != NULL) {
          log->elem("cast_up reason='!has_subklass' from='%d' to='(exact)'",
                    log->identify(ik));
        }
        C->recorder()->add_dependent(ik, NULL);
      }
    }
    if( easy_test ) {           // If we can still do the easy test
      // Just do a direct pointer compare and be done.
      Node *cmp = _gvn.transform( new (3) CmpPNode( subklass, superklass ) );
      Node *bol = _gvn.transform( new (2) BoolNode( cmp, BoolTest::eq ) );
      IfNode *iff = create_and_xform_if( control(), bol, PROB_STATIC_FREQUENT, COUNT_UNKNOWN );
      set_control( _gvn.transform( new (1) IfTrueNode ( iff ) ) );
      return       _gvn.transform( new (1) IfFalseNode( iff ) );
    }
  }

  // First load the super-klass's check-offset
  Node *p1 = basic_plus_adr( superklass, superklass, sizeof(oopDesc) + Klass::super_check_offset_offset_in_bytes() );
  Node *chk_off = _gvn.transform( new (3) LoadINode( NULL, memory(p1), p1, _gvn.type(p1)->is_ptr() ) );

  // Load from the sub-klass's super-class display list, or a 1-word cache of
  // the secondary superclass list, or a failing value with a sentinel offset
  // if the super-klass is an interface or exceptionally deep in the Java
  // hierarchy and we have to scan the secondary superclass list the hard way.
  // Worst-case type is a little odd: NULL is allowed as a result (usually
  // klass loads can never produce a NULL).
  const TypeKlassPtr *tkl = TypeKlassPtr::OBJECT->meet(TypePtr::NULL_PTR)->is_klassptr();
#ifdef _LP64
  Node *chk_off_X = _gvn.transform( new (2) ConvI2LNode(chk_off) );
#else
  Node *chk_off_X = chk_off;
#endif
  Node *p2 = _gvn.transform( new (4) AddPNode(subklass,subklass,chk_off_X) );
  Node *nkls = _gvn.transform( new (3) LoadKlassNode( NULL, memory(p2), p2, _gvn.type(p2)->is_ptr(), tkl ) );

  // Compile speed common case: ARE a subtype and we canNOT fail
  if( superklass == nkls )
    return top();             // false path is dead; no test needed.

  // Gather the various success & failures here
  RegionNode *r_ok_subtype = new RegionNode(1);
  RegionNode *r_not_subtype = new RegionNode(1);
  record_for_igvn(r_ok_subtype);
  record_for_igvn(r_not_subtype);

  // See if we get an immediate positive hit.  Happens roughly 83% of the
  // time.  Test to see if the value loaded just previously from the subklass
  // is exactly the superklass.
  Node *cmp1 = _gvn.transform( new (3) CmpPNode( superklass, nkls ) );
  Node *bol1 = _gvn.transform( new (2) BoolNode( cmp1, BoolTest::eq ) );
  IfNode *iff1 = create_and_xform_if( control(), bol1, PROB_LIKELY(0.83f), COUNT_UNKNOWN );
  r_ok_subtype->add_req( _gvn.transform( new (1) IfTrueNode ( iff1 ) ) );
  set_control(           _gvn.transform( new (1) IfFalseNode( iff1 ) ) );

  // Compile speed common case: Check for being deterministic right now.  If
  // chk_off is a constant and not equal to cacheoff then we are NOT a
  // subklass.  In this case we need exactly the 1 test above and we can
  // return those results immediately.
  int cacheoff_con = sizeof(oopDesc) + Klass::secondary_super_cache_offset_in_bytes();
  int chk_off_con;              // set by reference in get_int
  if( chk_off->get_int(&chk_off_con) &&
      chk_off_con != cacheoff_con ) {
    Node *not_subtype_ctrl = control();
    set_control(r_ok_subtype->in(1)); // We need exactly the 1 test above 
    return not_subtype_ctrl;
  }

  // Check for immediate negative hit.  Happens roughly 11% of the time (which
  // is roughly 63% of the remaining cases).  Test to see if the loaded
  // check-offset points into the subklass display list or the 1-element
  // cache.  If it points to the display (and NOT the cache) and the display
  // missed then it's not a subtype.
  Node *cacheoff = _gvn.intcon(cacheoff_con);
  Node *cmp2 = _gvn.transform( new (3) CmpINode( chk_off, cacheoff ) );
  Node *bol2 = _gvn.transform( new (2) BoolNode( cmp2, BoolTest::ne ) );
  IfNode *iff2 = create_and_xform_if( control(), bol2, PROB_LIKELY(0.63f), COUNT_UNKNOWN );
  r_not_subtype->add_req( _gvn.transform( new (1) IfTrueNode ( iff2 ) ) );
  set_control(            _gvn.transform( new (1) IfFalseNode( iff2 ) ) );
  
  // Check for self.  Very rare to get here, but its taken 1/3 the time.
  // No performance impact (too rare) but allows sharing of secondary arrays
  // which has some footprint reduction.
  Node *cmp3 = _gvn.transform( new (3) CmpPNode( subklass, superklass ) );
  Node *bol3 = _gvn.transform( new (2) BoolNode( cmp3, BoolTest::eq ) );
  IfNode *iff3 = create_and_xform_if( control(), bol3, PROB_LIKELY(0.36f), COUNT_UNKNOWN );
  r_ok_subtype->add_req( _gvn.transform( new (1) IfTrueNode ( iff3 ) ) );
  set_control(           _gvn.transform( new (1) IfFalseNode( iff3 ) ) );

  // Now do a linear scan of the secondary super-klass array.  Again, no real
  // performance impact (too rare) but it's gotta be done.
#ifdef IA64
  Node *psc = new (3) PartialSubtypeCheckNode( subklass, superklass );
  psc->set_req(0, control());
  Node *idx = _gvn.transform( psc );
#else
  Node *idx = _gvn.transform( new (3) PartialSubtypeCheckNode( subklass, superklass ) );
#endif

  Node *cmp4 = _gvn.transform( new (3) CmpINode( idx, _gvn.intcon(0) ) );
  Node *bol4 = _gvn.transform( new (2) BoolNode( cmp4, BoolTest::ne ) );
  IfNode *iff4 = create_and_xform_if( control(), bol4, PROB_FAIR, COUNT_UNKNOWN );
  r_not_subtype->add_req( _gvn.transform( new (1) IfTrueNode ( iff4 ) ) );
  r_ok_subtype ->add_req( _gvn.transform( new (1) IfFalseNode( iff4 ) ) );

  // Return false path; set default control to true path.
  set_control( _gvn.transform(r_ok_subtype) );
  return _gvn.transform(r_not_subtype);
}

//-------------------------------gen_instanceof--------------------------------
// Generate an instance-of idiom.  Used by both the instance-of bytecode
// and the reflective instance-of call.
Node* GraphKit::gen_instanceof( Node *subobj, Node* superklass ) {
  C->set_has_split_ifs(true); // Has chance for split-if optimization
  assert( !stopped(), "dead parse path should be checked in callers" );
  assert(!TypePtr::NULL_PTR->higher_equal(_gvn.type(superklass)->is_klassptr()), 
         "must check for not-null not-dead klass in callers");

  // Make the merge point
  RegionNode *region = new RegionNode(4);
  Node *phi = new PhiNode(region,TypeInt::BOOL);

  // Null check; get casted pointer; set region slot 3
  Node *not_null_obj = null_check_oop(region, subobj);
  phi->set_req(3,_gvn.intcon(0)); // Set null path value

  // If not_null_obj is dead, only null-path is taken
  set_control( not_null_obj->in(0) );
  if( stopped() ) {             // Doing instance-of on a NULL?
    set_control(region->in(3)); // Use is-null arm of test.
    return phi->in(3);          // Return 0
  }
  not_null_obj = _gvn.transform(not_null_obj);

  // Load the object's klass
  Node* p = basic_plus_adr( not_null_obj, not_null_obj, oopDesc::klass_offset_in_bytes() );
  Node* obj_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(TypeInstPtr::KLASS), p, TypeInstPtr::KLASS));

  // Generate the subtype check
  Node *not_subtype_ctrl = gen_subtype_check( obj_klass, superklass );

  // Plug in the success path to the general merge in slot 1.
  region->set_req(1,control());
  phi->set_req(1,_gvn.intcon(1));

  // Plug in the failing path to the general merge in slot 2.
  region->set_req(2,not_subtype_ctrl);
  phi->set_req(2,_gvn.intcon(0));

  // Return final merged results
  set_control( _gvn.transform(region) );
  record_for_igvn(region);
  return _gvn.transform(phi);
}

//-------------------------------gen_checkcast---------------------------------
// Generate a checkcast idiom.  Used by both the checkcast bytecode and the
// array store bytecode.  Stack must be as-if BEFORE doing the bytecode so the
// uncommon-trap paths work.  Adjust stack after this call.
// If failure_control is supplied and not null, it is filled in with
// the control edge for the cast failure.  Otherwise, an appropriate
// uncommon trap or exception is thrown.
Node* GraphKit::gen_checkcast(Node *obj, Node* superklass,
                              Node* *failure_control) {
  kill_dead_locals();           // Benefit all the uncommon traps
  const TypeKlassPtr *tk = _gvn.type(superklass)->is_klassptr();
  const Type *toop = TypeOopPtr::make_from_klass(tk->klass());

  // Some timing tests show this to be a 0.25% lossage on SpecJVM98 Intel and
  // 2-warehouse SpecJBB on a U80.  Differences are so low as to be difficult
  // to measure, but appears to be a lossage.
  // Fast cutout:  Check the case that the cast is vacuously true
  /*
  if( tk->singleton() ) {
    const TypeOopPtr *objtp = _gvn.type(obj)->isa_oopptr();
    if( objtp && objtp->klass() == tk->klass() )
      return obj;
  }
  */

  // Make the merge point
  RegionNode *region = new RegionNode(4);
  Node *phi = new PhiNode(region,toop);
  C->set_has_split_ifs(true); // Has chance for split-if optimization

  // Use null-cast information if it is available
  bool never_see_null = false;
  // If we see an unexpected null at a check-cast we record it and force a
  // recompile; the offending check-cast will be compiled to handle NULLs.
  // If we see several offending BCIs, then all checkcasts in the
  // method will be compiled to handle NULLs.
  if (UncommonNullCast            // Cutout for this technique
      && failure_control == NULL  // regular case
      && obj != null()            // And not the -Xcomp stupid case?
      && !too_many_traps(Deoptimization::Reason_null_check)) {
    // Finally, check the "null_seen" bit from the interpreter.
    ciProfileData* data = method()->method_data()->bci_to_data(bci());
    if (!(data != NULL && data->as_BitData()->was_null_seen())) {
      never_see_null = true;
    }
  }

  // Null check; get casted pointer; set region slot 3
  Node *not_null_obj = null_check_oop(region, obj, never_see_null);
  phi->set_req(3,null());       // Set null path value

  // If not_null_obj is dead, only null-path is taken
  set_control( not_null_obj->in(0) );
  if( stopped() ) {             // Doing instance-of on a NULL?
    set_control(region->in(3)); // Use is-null arm of test.
    return phi->in(3);          // Return NULL
  }
  not_null_obj = _gvn.transform(not_null_obj);

  // Load the object's klass
  Node* p = basic_plus_adr( not_null_obj, not_null_obj, oopDesc::klass_offset_in_bytes() );
  Node* obj_klass = _gvn.transform(new (3) LoadKlassNode(0, memory(TypeInstPtr::KLASS), p, TypeInstPtr::KLASS));

  // Generate the subtype check
  Node *not_subtype_ctrl = gen_subtype_check( obj_klass, superklass );

  // Plug in success path into the merge
  region->set_req(2,control());
  Node *cast_obj = _gvn.transform(new (2) CheckCastPPNode(control(),not_null_obj,toop));
  phi->set_req(2,cast_obj);

  // Failure path ends in uncommon trap (or may be dead - failure impossible)
  if (failure_control == NULL) {
    set_control(not_subtype_ctrl);
    if (!stopped()) {           // If failure is possible
      PreserveJVMState pjvms(this);
      builtin_throw(Deoptimization::Reason_class_check, obj_klass);
    }
  } else {
    (*failure_control) = not_subtype_ctrl;
  }
  region->set_req(1,top());     // Failure path was uncommon-trapped
  phi->set_req(1,top());        // so not possible here


  // A merge of NULL or Casted-NotNull obj
  Node *res = _gvn.transform(phi);

  // Note I do NOT always 'replace_in_map(obj,result)' here.
  //  if( tk->klass()->can_be_primary_super()  ) 
    // This means that if I successfully store an Object into an array-of-String
    // I 'forget' that the Object is really now known to be a String.  I have to
    // do this because we don't have true union types for interfaces - if I store
    // a Baz into an array-of-Interface and then tell the optimizer it's an
    // Interface, I forget that it's also a Baz and cannot do Baz-like field
    // references to it.  FIX THIS WHEN UNION TYPES APPEAR!
  //  replace_in_map( obj, res );

  // Return final merged results
  set_control( _gvn.transform(region) );
  record_for_igvn(region);
  return res;
}

//------------------------------next_monitor-----------------------------------
// What number should be given to the next monitor?
int GraphKit::next_monitor() {
  int next = jvms()->monitor_depth();
  // Keep the toplevel high water mark current:
  if (C->sync() < next+1)  C->set_sync(next+1);
  return next;
}

//------------------------------insert_mem_bar---------------------------------
// Memory barrier to avoid floating things around
void GraphKit::insert_mem_bar(MemBarNode* mb) {
  mb->set_req(TypeFunc::Control,control());
  mb->set_req(TypeFunc::Memory ,reset_memory());
  Node *membar = _gvn.transform(mb);
  set_control(_gvn.transform(new (1) ProjNode(membar,TypeFunc::Control) ));
  set_all_memory_call(membar);
}
  
//-------------------------insert_mem_bar_volatile----------------------------
// Memory barrier to avoid floating things around
void GraphKit::insert_mem_bar_volatile(MemBarNode* mb, int alias_idx) {
  mb->set_req(TypeFunc::Control,control());
  if (alias_idx == Compile::AliasIdxBot) {
    mb->set_req(TypeFunc::Memory, merged_memory()->base_memory());
  } else {
    mb->set_req(TypeFunc::Memory, memory(alias_idx));
  }
  Node *membar = _gvn.transform(mb);
  set_control(_gvn.transform(new (1) ProjNode(membar, TypeFunc::Control)));
  if (alias_idx == Compile::AliasIdxBot) {
    merged_memory()->set_base_memory(_gvn.transform(new (1) ProjNode(membar, TypeFunc::Memory)));
  } else {
    set_memory(_gvn.transform(new (1) ProjNode(membar, TypeFunc::Memory)),alias_idx);
  }
}
  
//------------------------------shared_lock------------------------------------
// Emit locking code.
FastLockNode* GraphKit::shared_lock(Node* obj) {
  // bci is either a monitorenter bc or InvocationEntryBci
  // %%% SynchronizationEntryBCI is redundant; use InvocationEntryBci in interfaces
  assert(SynchronizationEntryBCI == InvocationEntryBci, "");

  if( !GenerateSynchronizationCode )
    return NULL;                // Not locking things?
  if (stopped())                // Dead monitor?
    return NULL;

  assert(dead_locals_are_killed(), "should kill locals before sync. point");

  // Box the stack location
  Node *box = _gvn.transform(new (1) BoxLockNode(next_monitor()));

  // Make the merge point
  Node *region = new RegionNode(3);
  // If converting bytecodes, push onto worklist for later transform
  record_for_igvn(region);
  
  FastLockNode* flock = new (3) FastLockNode( 0, obj, box );

  _gvn.transform( flock );
  Node *bol = _gvn.transform(new (2) BoolNode(flock,BoolTest::ne));
  Node *iff = new (2) IfNode( control(), bol, PROB_MIN, COUNT_UNKNOWN );
  // Optimize test; set region slot 2
  Node *slow_path = opt_iff(region,iff);
  
  // Add monitor to debug info for the slow path.  If we block inside the
  // slow path and de-opt, we need the monitor hanging around
  map()->push_monitor( flock );

  // Make slow path call
  Node *f_mem = make_slow_call( OptoRuntime::complete_monitor_enter_Type(), OptoRuntime::complete_monitor_locking_Java(), NULL, slow_path, obj, box );
  Node *call = control()->in(0); 

  // Slow path can only throw asynchronous exceptions, which are always
  // de-opted.  So the compiler thinks the slow-call can never throw an
  // exception.  If it DOES throw an exception we would need the debug
  // info removed first (since if it throws there is no monitor).

  // Capture slow path
  region->set_req(1,control());
  // Optimize control
  region = _gvn.transform(region);
  set_control(region);
  // Hack memory state
  make_merge_mem( region, call, f_mem );

  // Prevent things from floating up before the lock
  insert_mem_bar( new MemBarAcquireNode() );

  return flock;
}


//------------------------------shared_unlock----------------------------------
// Emit unlocking code.
void GraphKit::shared_unlock(Node* box, Node* obj) {
  // bci is either a monitorenter bc or InvocationEntryBci
  // %%% SynchronizationEntryBCI is redundant; use InvocationEntryBci in interfaces
  assert(SynchronizationEntryBCI == InvocationEntryBci, "");

  if( !GenerateSynchronizationCode )
    return;
  if (stopped()) {               // Dead monitor?
    map()->pop_monitor();        // Kill monitor from debug info
    return;
  }

  // No need for a null check on unlock

  // Memory barrier to avoid floating things down past the locked region
  insert_mem_bar( new MemBarReleaseNode() );

  // Make the merge point
  RegionNode *region = new RegionNode(3);
  // If converting bytecodes, push onto worklist for later transform
  record_for_igvn(region);

  FastUnlockNode *funlock = new (3) FastUnlockNode( control(), obj, box );
 
  Node *tmp_funlock = _gvn.transform( funlock );

  // Verify that node will be a fast unlock
  assert( tmp_funlock->Opcode() == Op_FastUnlock, "must do a fast unlock");
  funlock = (FastUnlockNode*)tmp_funlock;
  Node *bol = _gvn.transform(new (2) BoolNode(funlock,BoolTest::ne));
  Node *iff = new (2) IfNode( control(), bol, PROB_MIN, COUNT_UNKNOWN );
  // Optimize test; set region slot 2
  Node *slow_path = opt_iff(region,iff);

  Node *f_mem = make_slow_call( OptoRuntime::complete_monitor_exit_Type(), CAST_FROM_FN_PTR(address, OptoRuntime::complete_monitor_unlocking_C), "complete_monitor_unlocking_C", slow_path, obj, box );
  
  Node *call = control()->in(0); 

  // No exceptions for unlocking

  // Capture slow path
  region->set_req(1,control());
  // Optimize final control
  set_control( _gvn.transform(region) );
  // Hack memory state
  make_merge_mem( region, call, f_mem );

  // Kill monitor from debug info
  map()->pop_monitor( );
}


//=============================================================================
// 
//                              A L L O C A T I O N 
//
// Allocation attempts to be fast in the case of frequent small objects.
// It breaks down like this: 
//
// 1) Size in doublewords is computed.  This is a constant for objects and
// variable for most arrays.  Doubleword units are used to avoid size
// overflow of huge doubleword arrays.  We need doublewords in the end for
// rounding.
//
// 2) Size is checked for being 'too large'.  Too-large allocations will go
// the slow path into the VM.  The slow path can throw any required
// exceptions, and does all the special checks for very large arrays.  The
// size test can constant-fold away for objects.  For objects with
// finalizers it constant-folds the otherway: you always go slow with
// finalizers.
//
// 3) If NOT using TLABs, this is the contended loop-back point.
// Load-Locked the heap top.  If using TLABs normal-load the heap top.
//
// 4) Check that heap top + size*8 < max.  If we fail go the slow ` route.
// NOTE: "top+size*8" cannot wrap the 4Gig line!  Here's why: for largish
// "size*8" we always enter the VM, where "largish" is a constant picked small
// enough that there's always space between the eden max and 4Gig (old space is
// there so it's quite large) and large enough that the cost of entering the VM
// is dwarfed by the cost to initialize the space.
//
// 5) If NOT using TLABs, Store-Conditional the adjusted heap top back
// down.  If contended, repeat at step 3.  If using TLABs normal-store
// adjusted heap top back down; there is no contention.
// 
// 6) If !ZeroTLAB then Bulk-clear the object/array.  Fill in klass & mark
// fields.
//
// 7) Merge with the slow-path; cast the raw memory pointer to the correct
// oop flavor.
//
//=============================================================================
// FastAllocateSizeLimit value is in DOUBLEWORDS.
// Allocations bigger than this always go the slow route.
// This value must be small enough that allocation attempts that need to
// trigger exceptions go the slow route.  Also, it must be small enough so
// that heap_top + size_in_bytes does not wrap around the 4Gig limit.
//=============================================================================

Node* GraphKit::allocate_heap( Node *size, // Computed size in doublewords
                               Node *initial_slow_test, // Initial go-slow-path test
                               // Note: klass is a constant for most normal
                               // instances and arrays.  It's only variable
                               // for the inlined NewInstance0 native.
                               Node *klass_node, // Klass to jam into object header and slow-path parm0
                               Node *length, // Array length or NULL
                               const TypeFunc *slow_call_type, // Type of slow call
                               address slow_call_address, // Address of slow call
                               Node *parm0, // Parm0 for slow call
                               Node *parm1, // Parm1 for slow call
                               bool slowcall_writes_memory,
                               const TypeOopPtr *oop_type // Resultant oop type
                               ) {
  CollectedHeap* ch = Universe::heap();
  assert(ch->supports_inline_contig_alloc(), "Assuming inline allocation");
  uint raw_idx = C->get_alias_index(TypeRawPtr::BOTTOM);

  // Compute address for Eden::top.  It will be loaded with a
  // LoadPLocked in the contended loop case, or a normal load in
  // the uncontended TLAB case.
  Node *eden_top_adr;
  Node *eden_end_adr;
  if( UseTLAB ) {               // Private allocation: load from TLS
    Node *thread = _gvn.transform(new (1) ThreadLocalNode());
    eden_top_adr = _gvn.transform( new (4) AddPNode(top()/*not oop*/, thread, _gvn.MakeConX( in_bytes(JavaThread::tlab_top_offset()))));
    eden_end_adr = _gvn.transform( new (4) AddPNode(top()/*not oop*/, thread, _gvn.MakeConX( in_bytes(JavaThread::tlab_end_offset()))));
  } else {                      // Shared allocation: load from globals
    address top_adr = (address)ch->top_addr();
    address end_adr = (address)ch->end_addr();
    eden_top_adr = makecon(TypeRawPtr::make(top_adr));
    eden_end_adr = basic_plus_adr( eden_top_adr, eden_top_adr, end_adr - top_adr );
  }
  // Load Eden::end.  Loop invariant and hoisted.
  //
  // Note: We set the control input on "eden_end" and "old_eden_top" when using
  //       a TLAB to work around a bug where these values were being moved across
  //       a safepoint.  These are not oops, so they cannot be include in the oop
  //       map, but the can be changed by a GC.   The proper way to fix this would
  //       be to set the raw memory state when generating a  SafepointNode.  However
  //       this will require extensive changes to the loop optimization in order to
  //       prevent a degradation of the optimization.
  //       See comment in memnode.hpp, around line 227 in class LoadPNode.
  Node *eden_end = make_load(control(), eden_end_adr, TypeRawPtr::BOTTOM, T_ADDRESS, TypeRawPtr::BOTTOM);

  // We need a Region to merge the slow-path and fast-path results.
  // This Region won't ever constant fold away.
  enum { slow_result_path = 1, fast_result_path = 2 };
  Node *result_region = new RegionNode(3);
  Node *result_phi_rawmem = new (3) PhiNode( result_region, Type::MEMORY, TypeRawPtr::BOTTOM );
  Node *result_phi_rawoop = new (3) PhiNode( result_region, TypeRawPtr::BOTTOM );
  Node *result_phi_i_o    = new (3) PhiNode( result_region, Type::ABIO ); // I/O is used for Prefetch

  // We need a Region to merge the two slow-path cases: the array-too-big or
  // oop-has-finalizers test and the heap-full-must-GC test.  For small
  // constant allocations with no finalizers (the common case) the first test
  // will go dead in the optimizer.  We'll constant fold it in the parser but
  // it will simplify things to have all the logic around for the general
  // case.
  enum { too_big_or_final_path = 1, need_gc_path = 2 };
  Node *slow_region;
  Node *slow_phi_rawmem;
  Node *toobig_false;
  // Do we have an initial slow case?  NULL if it's a small allocation
  if( initial_slow_test ) {
    slow_region = new RegionNode(3);
    slow_phi_rawmem = new (3) PhiNode( (RegionNode*)slow_region, Type::MEMORY, TypeRawPtr::BOTTOM );

    // Now make the initial failure test.  Usually a too-big test but
    // might be a TRUE for finalizers or a fancy class check for
    // newInstance0.
    IfNode *toobig_iff = create_and_xform_if( control(), initial_slow_test, PROB_MIN, COUNT_UNKNOWN );
    // Plug the failing-too-big test into the slow-path region
    Node *toobig_true = _gvn.transform( new (1) IfTrueNode( toobig_iff ) );
    slow_region    ->set_req( too_big_or_final_path, toobig_true );
    slow_phi_rawmem->set_req( too_big_or_final_path, memory(raw_idx) );
    toobig_false = _gvn.transform( new (1) IfFalseNode( toobig_iff ) );
  } else {                      // No initial test, just fall into next case
    toobig_false = control();
    debug_only(slow_region = slow_phi_rawmem = NodeSentinel);
  }

  // We need a Region for the loop-back contended case.  The loop-back edge
  // is dead and the Region unused with TLABs.  We'll make it anyways and
  // rely on the optimizer to fold it away.  Then we'll do the folding in the
  // parser but it will simplify things to have all the logic around for the
  // general case.
  enum { fall_in_path = 1, contended_loopback_path = 2 };
  Node *contended_region;
  Node *contended_phi_rawmem;
  if( UseTLAB ) {
    contended_region = toobig_false;
    contended_phi_rawmem = memory(raw_idx);
  } else {
    contended_region = new RegionNode(3);
    contended_phi_rawmem = new (3) PhiNode( contended_region, Type::MEMORY, TypeRawPtr::BOTTOM);
    _gvn.set_type(contended_region    , Type::CONTROL); // Must set_type because of loop ref
    _gvn.set_type(contended_phi_rawmem, Type::MEMORY ); // Must set_type because of loop ref
    record_for_igvn(contended_region);
    record_for_igvn(contended_phi_rawmem);
    // Now handle the passing-too-big test.  We fall into the contended
    // loop-back merge point.
    contended_region    ->set_req( fall_in_path, toobig_false );
    contended_phi_rawmem->set_req( fall_in_path, memory(raw_idx) );
  }

  // Load(-locked) the heap top.  
  // See note above concerning the control input when using a TLAB
  Node *old_eden_top = UseTLAB 
    ? new (3) LoadPNode     ( control(),        contended_phi_rawmem, eden_top_adr, TypeRawPtr::BOTTOM, TypeRawPtr::BOTTOM )
    : new (3) LoadPLockedNode( contended_region, contended_phi_rawmem, eden_top_adr );
  old_eden_top = _gvn.transform( old_eden_top );

  // Scale size in doublewords to get size in bytes
#ifdef _LP64
  // Since size is limited for the fast-path already, scaled size must
  // still fit in an int.  However, pointer math is 64-bits so convert.
  // Convert first, then shift, because some architectures can subsume
  // the shift into an address mode if it comes second.
  Node *sizex = _gvn.transform(new (2) ConvI2LNode(size));
#else
  Node *sizex = size;
#endif
  Node *size_in_bytes = _gvn.transform(new (3) LShiftXNode(sizex, _gvn.intcon(LogBytesPerLong)));
  // Add to heap top to get a new heap top
  Node *new_eden_top = _gvn.transform( new (4) AddPNode( top(), old_eden_top, size_in_bytes ) );
  // Check for needing a GC; compare against heap end
  Node *needgc_cmp = _gvn.transform( new (3) CmpPNode( new_eden_top, eden_end ) );
  Node *needgc_bol = _gvn.transform( new (2) BoolNode( needgc_cmp, BoolTest::ge ) );
  IfNode *needgc_iff = create_and_xform_if( contended_region, needgc_bol, PROB_UNLIKELY_MAG(4), COUNT_UNKNOWN );
  
  // Plug the failing-heap-space-need-gc test into the slow-path region
  Node *needgc_true = _gvn.transform( new (1) IfTrueNode( needgc_iff ) );
  if( initial_slow_test ) {
    slow_region    ->set_req( need_gc_path, needgc_true );
    slow_phi_rawmem->set_req( need_gc_path, memory(raw_idx) );
    // This completes all paths into the slow merge point
    slow_region     = _gvn.transform( slow_region );
    slow_phi_rawmem = _gvn.transform( slow_phi_rawmem );
  } else {                      // No initial slow path needed!
    // Just fall from the need-GC path straight into the VM call.
    slow_region    = needgc_true;
    slow_phi_rawmem= memory(raw_idx);
  }

  // No need for a GC.  Setup for the Store-Conditional
  Node *needgc_false = _gvn.transform( new (1) IfFalseNode( needgc_iff ) );
  // Store (-conditional) the modified eden top back down.
  // StorePConditional produces flags for a test PLUS a modified raw
  // memory state.
  Node *store_eden_top;
  Node *fast_oop_ctrl;
  if( UseTLAB ) {
    store_eden_top = new (4) StorePNode( needgc_false, contended_phi_rawmem, eden_top_adr, TypeRawPtr::BOTTOM, new_eden_top );
    store_eden_top = _gvn.transform( store_eden_top );
    fast_oop_ctrl = needgc_false; // No contention, so this is the fast path

  } else {
    store_eden_top = new (4) StorePConditionalNode( needgc_false, contended_phi_rawmem, eden_top_adr, new_eden_top, old_eden_top );
    store_eden_top = _gvn.transform( store_eden_top );
    Node *contention_check = _gvn.transform( new (2) BoolNode( store_eden_top, BoolTest::ne ) );
    store_eden_top = _gvn.transform( new (1) SCMemProjNode(store_eden_top));

    // If not using TLABs, check to see if there was contention.
    IfNode *contention_iff = create_and_xform_if( needgc_false, contention_check, PROB_MIN, COUNT_UNKNOWN );
    Node *contention_true = _gvn.transform( new (1) IfTrueNode( contention_iff ) );
    // If contention, loopback and try again.
    contended_region    ->set_req( contended_loopback_path, contention_true );
    contended_phi_rawmem->set_req( contended_loopback_path, store_eden_top );

    // This completes all paths into the contended merge point.
    _gvn.transform( contended_region );
    _gvn.transform( contended_phi_rawmem );

    // Fast-path succeeded with no contention!
    Node *contention_false = _gvn.transform( new (1) IfFalseNode( contention_iff ) );
    fast_oop_ctrl = contention_false;
  }

  // Rename successful fast-path variables to make meaning more obvious
  Node *fast_oop = old_eden_top;
  Node *fast_oop_rawmem = store_eden_top; 

  // Setup parser memory for 'store_to_memory' factory
  set_memory( fast_oop_rawmem, raw_idx );

  // Store the klass & mark bits
  markOop mark = markOopDesc::prototype();
  Node *adr = basic_plus_adr(fast_oop, fast_oop, oopDesc::mark_offset_in_bytes());
  Node *mark_node = makecon(TypeRawPtr::make((address)mark));
  store_to_memory(fast_oop_ctrl, adr, mark_node, T_ADDRESS, TypeRawPtr::BOTTOM);
  adr = basic_plus_adr(fast_oop, fast_oop, oopDesc::klass_offset_in_bytes()); 
  store_to_memory(fast_oop_ctrl, adr, klass_node, T_OBJECT, TypeRawPtr::BOTTOM);
  // Array length
  if( length ) {                // Arrays need length field
    adr = basic_plus_adr(fast_oop, fast_oop, arrayOopDesc::length_offset_in_bytes());
    store_to_memory( fast_oop_ctrl, adr, length, T_INT, TypeRawPtr::BOTTOM );
  }

  // Now bulk-clear the object body.  There may be a padding word after the
  // length, but it doesn't need to be initialized.  Optimizer will expand
  // this to a series of Stores if it's short and fixed size.
  if(!ZeroTLAB) {
    // Cache raw memory across bulk-zero code
    fast_oop_rawmem = memory(raw_idx);

    int offset_in_bytes = oopDesc::klass_offset_in_bytes() + oopSize;
    assert( (offset_in_bytes&3) == 0, "only doing word alignment" );
    if( length )                // Do not re-zero array length
      offset_in_bytes = arrayOopDesc::length_offset_in_bytes() + BytesPerInt;

    // Zero up to double-word alignment
    if( (offset_in_bytes & BytesPerInt) == BytesPerInt ) {
      adr = basic_plus_adr(fast_oop, fast_oop, offset_in_bytes);
      fast_oop_rawmem = _gvn.transform(new (4) StoreINode(fast_oop_ctrl, fast_oop_rawmem, adr, TypeRawPtr::BOTTOM, intcon(0)));
      offset_in_bytes += BytesPerInt;
    }
    // Init'd some stuff already, don't re-zero it
    Node *size_remaining = _gvn.transform( new (3) AddINode( size, _gvn.intcon(-offset_in_bytes>>LogBytesPerLong) ));
    // Bulk clear double-words
    adr = basic_plus_adr(fast_oop, fast_oop, offset_in_bytes);
    fast_oop_rawmem = _gvn.transform(new (4) ClearArrayNode(fast_oop_ctrl, fast_oop_rawmem, size_remaining, adr));
    // Store memory cache back to Parser state for more store factories
    set_memory(fast_oop_rawmem, raw_idx);
  }

  // Grab regular I/O before optional prefetch may change it.
  // Slow-path does no I/O so just set it to the original I/O.
  result_phi_i_o->set_req( slow_result_path, i_o() );
  // Insert a prefetch only on the fast-path
  if( AllocatePrefetchStyle ) {
    Node *prefetch_adr = basic_plus_adr(fast_oop, new_eden_top, AllocatePrefetchDistance);
    Node *prefetch = new (3) PrefetchNode(i_o(), prefetch_adr);
    // Do not let it float too high, since if eden_top == eden_end, both might be null.
    prefetch->set_req(0, fast_oop_ctrl);
    set_i_o(_gvn.transform(prefetch));
  }

  // Capture the fast-path memory
  MergeMemNode *fast_memory = NULL;
  if( slowcall_writes_memory ) 
    fast_memory = (MergeMemNode*)merged_memory()->clone();

  // Plug in the successful fast-path into the result merge point
  result_region    ->set_req( fast_result_path, fast_oop_ctrl );
  result_phi_rawoop->set_req( fast_result_path, fast_oop );
  result_phi_i_o   ->set_req( fast_result_path, i_o() );
  if( !slowcall_writes_memory ) 
    result_phi_rawmem->set_req( fast_result_path, memory(raw_idx) );

  // Generate slow-path call
  set_memory( slow_phi_rawmem, raw_idx );// Setup to use 'make_slow_call'
  make_slow_call(slow_call_type, slow_call_address, NULL, slow_region, parm0, parm1);
  Node *call = control()->in(0); 
  make_slow_call_ex( call, slowcall_writes_memory // Throw everything?
                     ? env()->Throwable_klass()
                     : env()->OutOfMemoryError_klass()); // Else just throw OOM

  // Plug slow-path into result merge point
  result_region    ->set_req( slow_result_path, control() );
  result_phi_rawoop->set_req( slow_result_path, _gvn.transform( new (1) ProjNode(call,TypeFunc::Parms)));

  // This completes all paths into the result merge point
  result_region     = _gvn.transform( result_region );
  result_phi_rawoop = _gvn.transform( result_phi_rawoop );
  result_phi_i_o    = _gvn.transform( result_phi_i_o    );

  // Slow-path memory: RAW or ALL
  if( slowcall_writes_memory ) {
    merge_fast_memory( fast_memory, result_region, slow_result_path );
  } else {
    result_phi_rawmem->set_req( slow_result_path, memory(raw_idx) );
    result_phi_rawmem = _gvn.transform( result_phi_rawmem );
    set_memory ( result_phi_rawmem, raw_idx );
  }

  // Pass results back to parser state
  set_control( result_region     );
  set_i_o    ( result_phi_i_o    );

  // Cast raw oop to the real thing
  Node *newoop = _gvn.transform( new (2) CheckCastPPNode( result_region, result_phi_rawoop, oop_type ));

  return newoop;
}


//---------------------------new_instance--------------------------------------
Node* GraphKit::new_instance(ciInstanceKlass* klass) {

  // Compute size in doublewords
  int instance_size_in_words = klass->size_helper() << (LogHeapWordSize -LogBytesPerInt);
  assert( (instance_size_in_words & 1) == 0, "odd words allocated?  how did old code ever work?" );
  int instance_size_in_doublewords = instance_size_in_words >> 1;
  Node *size = _gvn.intcon(instance_size_in_doublewords);

  // Need klass node constant for either fast or slow path
  Node *klass_node = makecon(TypeKlassPtr::make(klass));

  // Generate the initial go-slow test.  It's either ALWAYS (return a
  // Node for 1) or NEVER (return a NULL).  Since the ALWAYS case is
  // statically rare, we'll generate all paths and allow the optimizer
  // to fold things away.  For the common NEVER case, we'll pass in a
  // NULL as a clue to the parser to skip the initial failure test.
  Node *initial_slow_test =  (klass->has_finalizer() || instance_size_in_doublewords >= FastAllocateSizeLimit) ? _gvn.intcon(1) : NULL;

  // Characterize slow-path call 
  const TypeFunc *slow_call_type = OptoRuntime::new_Type();
  address slow_call_address      = OptoRuntime::new_Java();

  // This is a precise notnull oop of the klass.
  // It's what we cast the result to.
  const TypeInstPtr *oop_type = TypeInstPtr::make_exact(TypePtr::NotNull, klass);

  // Now generate allocation code
  return allocate_heap( size,       // Size in doublewords
                        initial_slow_test, // Too-big or finalizer check
                        klass_node, // Klass to jam into header
                        NULL,       // No array length for instances
                        slow_call_type,
                        slow_call_address,
                        klass_node, // slow-path parm0
                        NULL,       // No 2nd parm needed
                        false,      // Slow-call does not write memory
                        oop_type ); // Compiler type
}

//-------------------------------new_array-------------------------------------
// helper for both newarray and anewarray
Node* GraphKit::new_array(Node* count_val,      // number of elements
                          BasicType elem_type,  // BasicType of elements
                          const Type* etype,    // Compiler type of elements
                          const TypeKlassPtr* array_klass // array klass
                          ) {

  // Generate the initial go-slow test.  Make sure we do not overflow
  // if count_val is huge (near 2Gig) or negative!  We do not need
  // exact double-words here, just a close approximation of needed
  // double-words.  We can't add any offset or rounding bits, lest we
  // take a size -1 of bytes and make it positive.  Use an unsigned
  // compare, so negative sizes look hugely positive.
  int fast_size_limit = FastAllocateSizeLimit << ( LogBytesPerLong - exact_log2(type2aelembytes[elem_type]));
  Node *initial_slow_cmp  = _gvn.transform( new (3) CmpUNode( count_val, _gvn.intcon( fast_size_limit ) ) );
  Node *initial_slow_test = _gvn.transform( new (2) BoolNode( initial_slow_cmp, BoolTest::gt ) );

  // --- Vast Annoying Size Computation ---
  // compute array_words = size in words of count_val elements,
  // (elem_byte_size * len + 3) >> 2.
  Node *array_words;
  if( type2aelembytes[elem_type] == BytesPerInt ) { 
    array_words = count_val;    // Skip rounding for popular size 4 elements
  } else {
    Node* elem_size = _gvn.intcon(type2aelembytes[elem_type]);
    Node* mul = _gvn.transform(new (3) MulINode( elem_size, count_val));
    Node* add = _gvn.transform(new (3) AddINode( mul, _gvn.intcon(BytesPerInt-1)));
    array_words = _gvn.transform(new (3) URShiftINode(add, _gvn.intcon(LogBytesPerInt)));
  }

  // base_words = size in words without array elements
  int base_words = typeArrayOopDesc::header_size(elem_type) << (LogHeapWordSize -LogBytesPerInt);
  // Add array header size; add 1 more for rounding
  Node *alloc_words = _gvn.transform(new (3) AddINode( array_words, _gvn.intcon(base_words+1)));
  // Round to doublewords
  Node *size = _gvn.transform(new (3) URShiftINode( alloc_words, _gvn.intcon(LogBytesPerLong-LogBytesPerInt) ));

  // Class type of result
  Node* klass_node = makecon(array_klass);

  // Characterize slow-path call.  Normal primitive arrays are easy.
  // Extra argument is the primitive BasicType.
  const TypeFunc *slow_call_type = OptoRuntime::new_typeArray_Type();
  address slow_call_address      = OptoRuntime::new_typeArray_Java();
  Node *parm0 = _gvn.intcon(elem_type);
  // Arrays of objects are more difficult.  VM knows its an object
  // array by the different entry point.  Instead of T_OBJECT, the
  // extra argument is the element class.
  if( elem_type == T_OBJECT ) {
    assert( etype->isa_instptr() || etype->isa_aryptr(), "" );
    const TypeOopPtr *jetype = etype->isa_oopptr();
    ciKlass* jeklass = jetype->klass();
    Node *elem_klass_node = makecon( TypeKlassPtr::make(jeklass) );
    slow_call_type    = OptoRuntime::new_objArray_Type();
    slow_call_address = OptoRuntime::new_objArray_Java();
    parm0 = elem_klass_node;
  }

  // Try to get a better type than POS for the size
  const Type* count_type = _gvn.type(count_val)->join(TypeInt::POS);
  assert(count_type->is_int(), "must be integer");
  if( count_type->empty() )     // Negative length arrays will produce weird
    count_type = TypeInt::ZERO; // intermediate dead fast-path goo

  // Cast to correct type
  const TypeAry* arr0 = TypeAry::make(etype, (TypeInt*)count_type);
  const TypeAryPtr* tarr = TypeAryPtr::make(TypePtr::NotNull, arr0, array_klass->klass(), true, 0);

  // Now generate allocation code
  return allocate_heap( size,       // Size in doublewords
                        initial_slow_test, // Too-big size check
                        klass_node, // Klass to jam in header
                        count_val,  // Array length
                        slow_call_type,
                        slow_call_address,
                        parm0,      // slow-path parm0
                        count_val,  // 2nd argument is array length
                        false,      // Slow-call does not write memory
                        tarr );
}
