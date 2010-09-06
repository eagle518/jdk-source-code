#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)callnode.cpp	1.215 03/12/23 16:42:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style

#include "incls/_precompiled.incl"
#include "incls/_callnode.cpp.incl"

//=============================================================================
uint StartNode::size_of() const { return sizeof(*this); }
//uint StartNode::hash() const { 
//  // We do not create equivalent starts, just add to Node::in(control)
//  return (uintptr_t)in(TypeFunc::Control) + (uintptr_t)_domain; 
//}
uint StartNode::cmp( const Node &n ) const
{ return _domain == ((StartNode&)n)._domain; }
const Type *StartNode::bottom_type() const { return _domain; }
const Type *StartNode::Value(PhaseTransform *phase) const { return _domain; }
#ifndef PRODUCT
void StartNode::dump_spec() const { tty->print(" #"); _domain->dump();}
#endif

//------------------------------Ideal------------------------------------------
Node *StartNode::Ideal(PhaseGVN *phase, bool can_reshape){
  return remove_dead_region(phase, can_reshape) ? this : NULL;
}

//------------------------------calling_convention-----------------------------
void StartNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
  Matcher::calling_convention( parm_regs, argcnt, false );
}

//------------------------------Registers--------------------------------------
const RegMask &StartNode::in_RegMask(uint) const { 
  return RegMask::Empty;
}

//------------------------------match------------------------------------------
// Construct projections for incoming parameters, and their RegMask info
Node *StartNode::match( const ProjNode *proj, const Matcher *match ) {
  switch (proj->_con) {
  case TypeFunc::Control: 
  case TypeFunc::I_O:
  case TypeFunc::Memory:
    return new (1) MachProjNode(this,proj->_con,RegMask::Empty,MachProjNode::unmatched_proj);
  case TypeFunc::FramePtr:
    return new (1) MachProjNode(this,proj->_con,Matcher::c_frame_ptr_mask, Op_RegP);
  case TypeFunc::ReturnAdr:
    return new (1) MachProjNode(this,proj->_con,match->_return_addr_mask,Op_RegP);
  case TypeFunc::Parms:
  default: {
      uint parm_num = proj->_con - TypeFunc::Parms;
      const Type *t = _domain->field_at(proj->_con);
      if (t->base() == Type::Half)  // 2nd half of Longs and Doubles
        return new (1) ConNode(Type::TOP);
      uint ideal_reg = Matcher::base2reg[t->base()];
      RegMask &rm = match->_calling_convention_mask[parm_num];
      return new (1) MachProjNode(this,proj->_con,rm,ideal_reg);
    }
  } 
  return NULL;
}

//------------------------------StartC2INode----------------------------------
// The method start node for a Compiled to Interpreter frame converter

//------------------------------c2i_domain-----------------------------
// Construct the new incoming signature, 
// Since the MethodKlassHolder parameter is split outside of ideal world
// Expand signature to include inline_cached_klass and method_oop
const TypeTuple *StartC2INode::c2i_domain( const TypeTuple *domain ) {
  uint j_parm_cnt     = domain->_cnt;
  uint start_parm_cnt = j_parm_cnt + 1;
  const Type     **fields = TypeTuple::fields(start_parm_cnt);
  for( uint i = 0; i < j_parm_cnt; i++ )
    fields[i] = domain->field_at(i);
  // either method oop or MethodKlassHolder converted to method_oop by prefix
  fields[j_parm_cnt + 0] = TypeInstPtr::BOTTOM;  // method_oop
  return TypeTuple::make( start_parm_cnt, fields );
}

//------------------------------calling_convention----------------------------
// Frame converters from compiled code to interpreter 
// pass the last parameter in the inline_cache_reg, 
// not through the standard calling convention
void StartC2INode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
  // Use the standard compiled-java calling convention with one additional
  // arguments: the inline_cache_reg() holds the method oop 
  assert( argcnt >= 1, "Need method_oop parameters");
  uint java_arg_count = argcnt - 1;
  Matcher::calling_convention( parm_regs, java_arg_count, false );

  // Set up register assignments for extra parameter
  parm_regs[ java_arg_count + 0 ].set_ptr(Matcher::inline_cache_reg());
}

//------------------------------StartI2CNode----------------------------------
// The method start node for an Interpreter to Compiled frame converter

//------------------------------calling_convention-----------------------------
// Frame converters from interpreter to compiled code are passed the
// methodOop in the inline_cache_reg and the argument pointer in the
// interpreter_arg_ptr_reg, not through the standard calling convention.
void StartI2CNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {

  assert( argcnt == 2, "Need the methodOop in parm0 and arg ptr in parm1");

  // Set up register assignments for extra parameter, methodOop
  parm_regs[ 0 ].set_ptr(Matcher::inline_cache_reg());
  parm_regs[ 1 ].set_ptr(Matcher::interpreter_arg_ptr_reg());
}

//------------------------------StartOSRNode----------------------------------
// The method start node for an on stack replacement adapter

//------------------------------osr_domain-----------------------------
const TypeTuple *StartOSRNode::osr_domain() {
  const Type **fields = TypeTuple::fields(2);
  fields[TypeFunc::Parms+0] = TypeRawPtr::BOTTOM;  // address of locals
  fields[TypeFunc::Parms+1] = TypeRawPtr::BOTTOM;  // address of monitors

  return TypeTuple::make(TypeFunc::Parms+2, fields);
}

//------------------------------calling_convention-----------------------------
// On stack replacement calls use the interpreter calling convention.
void StartOSRNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
  // On stack replacement code has 2 arguments, the address of the
  // interpreter's locks and the address of the interpreter's locals.
  assert( argcnt == 2, "Need the exactly the locals address and monitors address");
  parm_regs[0].set_ptr(Matcher::inline_cache_reg());
  parm_regs[1].set_ptr(Matcher::interpreter_arg_ptr_reg());
}

//=============================================================================
const char * const ParmNode::names[TypeFunc::Parms+1] = {
  "Control", "I_O", "Memory", "FramePtr", "ReturnAdr", "Parms"
};

#ifndef PRODUCT
void ParmNode::dump_spec() const {
  if( _con < TypeFunc::Parms ) {
    tty->print(names[_con]);
  } else {
    tty->print("Parm%d: ",_con-TypeFunc::Parms);
    // Verbose and WizardMode dump bottom_type for all nodes
    if( !Verbose && !WizardMode )   bottom_type()->dump();
  }
}
#endif

uint ParmNode::ideal_reg() const {
  switch( _con ) {
  case TypeFunc::Control  : // fall through
  case TypeFunc::I_O      : // fall through
  case TypeFunc::Memory   : return 0;
  case TypeFunc::FramePtr : // fall through
  case TypeFunc::ReturnAdr: return Op_RegP;      
  default                 : assert( _con > TypeFunc::Parms, "" ); 
    // fall through
  case TypeFunc::Parms    : {
    // Type of argument being passed
    const Type *t = in(0)->is_Start()->_domain->field_at(_con);
    return Matcher::base2reg[t->base()];
  }
  }
  ShouldNotReachHere();
  return 0;
}

//=============================================================================
ReturnNode::ReturnNode(Node *cntrl, Node *i_o, Node *memory, Node *frameptr, Node *retadr ) : Node(TypeFunc::Parms) { 
  set_req(TypeFunc::Control,cntrl); 
  set_req(TypeFunc::I_O,i_o); 
  set_req(TypeFunc::Memory,memory); 
  set_req(TypeFunc::FramePtr,frameptr);
  set_req(TypeFunc::ReturnAdr,retadr); 
}

Node *ReturnNode::Ideal(PhaseGVN *phase, bool can_reshape){
  return remove_dead_region(phase, can_reshape) ? this : NULL; 
}

const Type *ReturnNode::Value( PhaseTransform *phase ) const { 
  return ( phase->type(in(TypeFunc::Control)) == Type::TOP)
    ? Type::TOP
    : Type::BOTTOM;
}

// Do we Match on this edge index or not?  No edges on return nodes
uint ReturnNode::match_edge(uint idx) const {
  return 0;
}


#ifndef PRODUCT
void ReturnNode::dump_req() const {
  // Dump the required inputs, enclosed in '(' and ')'
  uint i;                       // Exit value of loop
  for( i=0; i<req(); i++ ) {    // For all required inputs
    if( i == TypeFunc::Parms ) tty->print("returns");
    if( in(i) ) tty->print("%c%d ", Compile::current()->node_arena()->contains(in(i)) ? ' ' : 'o', in(i)->_idx);
    else tty->print("_ ");
  }
}
#endif

//=============================================================================
RethrowNode::RethrowNode(
  Node* cntrl,
  Node* i_o,
  Node* memory,
  Node* frameptr,
  Node* ret_adr,
  Node* exception
) : Node(TypeFunc::Parms + 1) { 
  set_req(TypeFunc::Control  , cntrl    ); 
  set_req(TypeFunc::I_O      , i_o      ); 
  set_req(TypeFunc::Memory   , memory   ); 
  set_req(TypeFunc::FramePtr , frameptr );
  set_req(TypeFunc::ReturnAdr, ret_adr);
  set_req(TypeFunc::Parms    , exception);
}

Node *RethrowNode::Ideal(PhaseGVN *phase, bool can_reshape){
  return remove_dead_region(phase, can_reshape) ? this : NULL; 
}

const Type *RethrowNode::Value( PhaseTransform *phase ) const { 
  return (phase->type(in(TypeFunc::Control)) == Type::TOP)
    ? Type::TOP
    : Type::BOTTOM;
}

uint RethrowNode::match_edge(uint idx) const {
  return 0;
}

#ifndef PRODUCT
void RethrowNode::dump_req() const {
  // Dump the required inputs, enclosed in '(' and ')'
  uint i;                       // Exit value of loop
  for( i=0; i<req(); i++ ) {    // For all required inputs
    if( i == TypeFunc::Parms ) tty->print("exception");
    if( in(i) ) tty->print("%c%d ", Compile::current()->node_arena()->contains(in(i)) ? ' ' : 'o', in(i)->_idx);
    else tty->print("_ ");
  }
}
#endif

//=============================================================================
TailCallNode::TailCallNode(Node *cntrl, Node *i_o, Node *memory, Node *frameptr, Node *retadr, Node *target, Node *moop) : ReturnNode( cntrl, i_o, memory, frameptr, retadr ) {
  add_req(target);
  add_req(moop);
}

// Do we Match on this edge index or not?  Match only target address & method
uint TailCallNode::match_edge(uint idx) const {
  return TypeFunc::Parms <= idx  &&  idx <= TypeFunc::Parms+1;
}

//=============================================================================
TailJumpNode::TailJumpNode(Node *cntrl, Node *i_o, Node *memory, Node *frameptr, Node *target, Node *ex_oop) : ReturnNode(cntrl, i_o, memory, frameptr, Compile::current()->top()) {
  add_req(target);
  add_req(ex_oop);
}

// Do we Match on this edge index or not?  Match only target address & oop
uint TailJumpNode::match_edge(uint idx) const {
  return TypeFunc::Parms <= idx  &&  idx <= TypeFunc::Parms+1;
}

//=============================================================================
JVMState::JVMState(ciMethod* method, JVMState* caller) {
  assert(method != NULL, "must be valid call site");
  _method = method;
  debug_only(_bci = -99);  // random garbage value
  debug_only(_map = (SafePointNode*)-1);
  _caller = caller;
  _depth  = 1 + (caller == NULL ? 0 : caller->depth());
  _locoff = TypeFunc::Parms;
  _stkoff = _locoff + _method->max_locals();
  _monoff = _stkoff + _method->max_stack();
  _endoff = _monoff;
  _sp = 0;
}
JVMState::JVMState(int stack_size) {
  _method = NULL;
  _bci = InvocationEntryBci;
  debug_only(_map = (SafePointNode*)-1);
  _caller = NULL;
  _depth  = 1;
  _locoff = TypeFunc::Parms;
  _stkoff = _locoff;
  _monoff = _stkoff + stack_size;
  _endoff = _monoff;
  _sp = 0;
}

//--------------------------------of_depth-------------------------------------
JVMState* JVMState::of_depth(int d) const {
  const JVMState* jvmp = this;
  assert(0 < d && (uint)d <= depth(), "oob");
  for (int skip = depth() - d; skip > 0; skip--) {
    jvmp = jvmp->caller();
  }
  assert(jvmp->depth() == (uint)d, "found the right one");
  return (JVMState*)jvmp;
}

//-----------------------------same_calls_as-----------------------------------
bool JVMState::same_calls_as(const JVMState* that) const {
  if (this == that)                    return true;
  if (this->depth() != that->depth())  return false;
  const JVMState* p = this;
  const JVMState* q = that;
  for (;;) {
    if (p->_method != q->_method)    return false;
    if (p->_method == NULL)          return true;   // bci is irrelevant
    if (p->_bci    != q->_bci)       return false;
    p = p->caller();
    q = q->caller();
    if (p == q)                      return true;
    assert(p != NULL && q != NULL, "depth check ensures we don't run off end");
  }
}

//------------------------------debug_start------------------------------------
uint JVMState::debug_start()  const {
  debug_only(JVMState* jvmroot = of_depth(1));
  assert(jvmroot->locoff() <= this->locoff(), "youngest JVMState must be last");
  return of_depth(1)->locoff();
}

//-------------------------------debug_end-------------------------------------
uint JVMState::debug_end() const {
  debug_only(JVMState* jvmroot = of_depth(1));
  assert(jvmroot->endoff() <= this->endoff(), "youngest JVMState must be last");
  return endoff();
}

//------------------------------debug_depth------------------------------------
uint JVMState::debug_depth() const {
  uint total = 0;
  for (const JVMState* jvmp = this; jvmp != NULL; jvmp = jvmp->caller()) {
    total += jvmp->debug_size();
  }
  return total;
}

//------------------------------format_helper----------------------------------
// Given an allocation (a Chaitin object) and a Node decide if the Node carries
// any defined value or not.  If it does, print out the register or constant.
#ifndef PRODUCT
static void format_helper( PhaseRegAlloc *regalloc, Node *n, const char *msg, uint i ) {
  if (n == NULL) { tty->print(" NULL"); return; }
  if( regalloc->get_reg_lo(n) != OptoReg::Bad ) { // Check for undefined
    char buf[50];
    regalloc->dump_register(n,buf);
    tty->print(" %s%d=%s",msg,i,buf);
  } else {                      // No register, but might be constant  
    const Type *t = n->bottom_type();
    switch (t->base()) {
    case Type::Int:  
      tty->print(" %s%d=#"INT32_FORMAT,msg,i,t->is_int()->get_con()); 
      break;
    case Type::AnyPtr: 
      assert( t == TypePtr::NULL_PTR, "" );
      tty->print(" %s%d=#NULL",msg,i);
      break;
    case Type::AryPtr: 
    case Type::KlassPtr:
    case Type::InstPtr: 
      tty->print(" %s%d=#Ptr" INTPTR_FORMAT,msg,i,t->isa_oopptr()->const_oop());
      break;
    case Type::RawPtr: 
      tty->print(" %s%d=#Raw" INTPTR_FORMAT,msg,i,t->is_rawptr());
      break;
    case Type::DoubleCon:
      tty->print(" %s%d=#%fD",msg,i,t->is_double_constant()->_d);
      break;
    case Type::FloatCon:
      tty->print(" %s%d=#%fF",msg,i,t->is_float_constant()->_f);
      break;
    case Type::Long:
      tty->print(" %s%d=#"INT64_FORMAT,msg,i,t->is_long()->get_con());
      break;
    case Type::Half:
    case Type::Top:  
      tty->print(" %s%d=_",msg,i);
      break;
    default: ShouldNotReachHere();
    }
  }
}
#endif

//------------------------------format-----------------------------------------
#ifndef PRODUCT
void JVMState::format(PhaseRegAlloc *regalloc, const Node *n) const {
  tty->print("        #");
  if( _method ) {
    _method->print_short_name();
    tty->print(" @ bci:%d ",_bci);
  } else {
    tty->print_cr(" runtime stub ");
    return;
  }
  MachNode *mach = ((Node *)n)->is_Mach();
  MachSafePointNode *mcall = mach->is_MachSafePoint();
  if (mcall) {
    uint i;
    // Print locals
    for( i = 0; i < (uint)loc_size(); i++ ) 
      format_helper( regalloc, mcall->local(this, i), "L", i );
    // Print stack
    for (i = 0; i < (uint)stk_size(); i++) {
      if ((uint)(_stkoff + i) >= mcall->len()) 
        tty->print(" oob ");
      else
       format_helper( regalloc, mcall->stack(this, i), "STK", i );
    }
    for (i = 0; (int)i < nof_monitors(); i++) {
      Node *box = mcall->monitor_box(this, i);
      Node *obj = mcall->monitor_obj(this, i);
      if ( regalloc->get_reg_lo(box) != OptoReg::Bad ) {
        while( !box->is_BoxLock() )  box = box->in(1);
        format_helper( regalloc, box, "MON-BOX", i );
      } else {
        OptoReg::Name box_reg = BoxLockNode::stack_slot(box);
        tty->print(" MON-BOX%d=%s+%d",i,SharedInfo::regName[SharedInfo::c_frame_pointer],regalloc->reg2offset(box_reg));
      }
      format_helper( regalloc, obj, "MON-OBJ", i );      
    }
  }
  tty->print_cr("");
  if (caller() != NULL)  caller()->format(regalloc, n);
}
#endif

#ifndef PRODUCT
void JVMState::dump_spec() const { 
  if( _method ) {
    _method->print_short_name();
    tty->print(" @ bci:%d ",_bci);
  } else {
    tty->print(" runtime stub ");
  }
  if (caller() != NULL)  caller()->dump_spec();
}
#endif

#ifndef PRODUCT
void JVMState::dump() const {
  if (_map && !((uintptr_t)_map & 1)) {
    if (_map->len() > _map->req()) {  // _map->has_exceptions()
      Node* ex = _map->in(_map->req());  // _map->next_exception()
      // skip the first one; it's already being printed
      while (ex != NULL && ex->len() > ex->req()) {
        ex = ex->in(ex->req());  // ex->next_exception()
        ex->dump(1);
      }
    }
    _map->dump(2);
  }
  tty->print("JVMS depth=%d loc=%d stk=%d mon=%d end=%d mondepth=%d sp=%d bci=%d method=",
             depth(), locoff(), stkoff(), monoff(), endoff(), monitor_depth(), sp(), bci());
  if (_method == NULL) {
    tty->print_cr("(none)");
  } else {
    _method->print_name();
    tty->cr();
    if (bci() >= 0 && bci() < _method->code_size()) {
      tty->print("    bc: ");
      _method->print_codes(bci(), bci()+1);
    }
  }
  if (caller() != NULL) {
    caller()->dump();
  }
}

// Extra way to dump a jvms from the debugger,
// to avoid a bug with C++ member function calls.
void dump_jvms(JVMState* jvms) {
  jvms->dump();
}
#endif

//--------------------------clone_shallow--------------------------------------
JVMState* JVMState::clone_shallow() const {
  JVMState* n = has_method() ? new JVMState(_method, _caller) : new JVMState(0);
  n->set_bci(_bci);
  n->set_locoff(_locoff);
  n->set_stkoff(_stkoff);
  n->set_monoff(_monoff);
  n->set_endoff(_endoff);
  n->set_sp(_sp);
  n->set_map(_map);
  return n;
}

//---------------------------clone_deep----------------------------------------
JVMState* JVMState::clone_deep() const {
  JVMState* n = clone_shallow();
  for (JVMState* p = n; p->_caller != NULL; p = p->_caller) {
    p->_caller = p->_caller->clone_shallow();
  }
  assert(n->depth() == depth(), "sanity");
  assert(n->debug_depth() == debug_depth(), "sanity");
  return n;
}

//=============================================================================
//uint CallNode::hash() const { return Node::hash() + (intptr_t)_tf + (intptr_t)_jvms; }
uint CallNode::cmp( const Node &n ) const
{ return _tf == ((CallNode&)n)._tf && _jvms == ((CallNode&)n)._jvms; }
#ifndef PRODUCT
void CallNode::dump_req() const { 
  // Dump the required inputs, enclosed in '(' and ')'
  uint i;                       // Exit value of loop
  for( i=0; i<req(); i++ ) {    // For all required inputs
    if( i == TypeFunc::Parms ) tty->print("(");
    if( in(i) ) tty->print("%c%d ", Compile::current()->node_arena()->contains(in(i)) ? ' ' : 'o', in(i)->_idx);
    else tty->print("_ ");
  }
  tty->print(")");
}

void CallNode::dump_spec() const { 
  tty->print(" "); 
  tf()->dump();
  if (jvms() != NULL)  jvms()->dump_spec();
}
#endif

const Type *CallNode::bottom_type() const { return tf()->range(); }
const Type *CallNode::Value(PhaseTransform *phase) const { 
  if (phase->type(in(0)) == Type::TOP)  return Type::TOP;
  return tf()->range(); 
}

//------------------------------calling_convention-----------------------------
void CallNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
  // Use the standard compiler calling convention 
  Matcher::calling_convention( parm_regs, argcnt, true );
}


//------------------------------match------------------------------------------
// Construct projections for control, I/O, memory-fields, ..., and
// return result(s) along with their RegMask info
Node *CallNode::match( const ProjNode *proj, const Matcher *match ) {
  switch (proj->_con) {
  case TypeFunc::Control: 
  case TypeFunc::I_O:
  case TypeFunc::Memory:
    return new (1) MachProjNode(this,proj->_con,RegMask::Empty,MachProjNode::unmatched_proj);

  case TypeFunc::Parms+1:       // For LONG & DOUBLE returns
    assert(tf()->_range->field_at(TypeFunc::Parms+1) == Type::HALF, "");
    // 2nd half of doubles and longs
    return new (1) MachProjNode(this,proj->_con, RegMask::Empty, (uint)OptoReg::Bad);

  case TypeFunc::Parms: {       // Normal returns
    uint ideal_reg = Matcher::base2reg[tf()->range()->field_at(TypeFunc::Parms)->base()];
    OptoRegPair regs = is_CallRuntime() 
      ? match->c_return_value(ideal_reg,true)  // Calls into C runtime
      : match->  return_value(ideal_reg,true); // Calls into compiled Java code
    RegMask rm = RegMask(regs.lo());
    if( regs.hi() != OptoReg::Bad )
      rm.Insert( regs.hi() );
    return new (1) MachProjNode(this,proj->_con,rm,ideal_reg);
  }

  case TypeFunc::ReturnAdr:
  case TypeFunc::FramePtr:
  default:
    ShouldNotReachHere();
  }
  return NULL;
}

// Do we Match on this edge index or not?  Match no edges
uint CallNode::match_edge(uint idx) const {
  return 0;
}

//=============================================================================
uint CallJavaNode::size_of() const { return sizeof(*this); }
//uint CallJavaNode::hash() const { return CallNode::hash() + (intptr_t)_method; }
uint CallJavaNode::cmp( const Node &n ) const { 
  CallJavaNode &call = (CallJavaNode&)n;
  return CallNode::cmp(call) && _method == call._method; 
}
#ifndef PRODUCT
void CallJavaNode::dump_spec() const { 
  if( _method ) _method->print_short_name();
  CallNode::dump_spec();
}
#endif

//=============================================================================
uint CallStaticJavaNode::size_of() const { return sizeof(*this); }
//uint CallStaticJavaNode::hash() const {
//  // Since calls produce control, distinguishing control is sufficient
//  // Node::in(control) + CallNode::_tf + CallJavaNode_method + this::_name
//  return (uintptr_t)in(TypeFunc::Control) + (uintptr_t)_tf + (uintptr_t)_method + (uintptr_t)_name;
//}
uint CallStaticJavaNode::cmp( const Node &n ) const { 
  CallStaticJavaNode &call = (CallStaticJavaNode&)n;
  return CallJavaNode::cmp(call); 
}
#ifndef PRODUCT
void CallStaticJavaNode::dump_spec() const { 
  tty->print("# Static ");
  if( _name ) {
    tty->print(_name);
    tty->print(" ");
  }
  CallJavaNode::dump_spec();
}
#endif

//=============================================================================
uint CallDynamicJavaNode::size_of() const { return sizeof(*this); }
//uint CallDynamicJavaNode::hash() const { 
//  // Since calls produce control, distinguishing control is sufficient
//  // Node::in(control) + CallNode::_tf + CallJavaNode_method
//  return (uintptr_t)in(TypeFunc::Control) + (uintptr_t)_tf + (uintptr_t)_method; 
//}
uint CallDynamicJavaNode::cmp( const Node &n ) const { 
  CallDynamicJavaNode &call = (CallDynamicJavaNode&)n;
  return CallJavaNode::cmp(call); 
}
#ifndef PRODUCT
void CallDynamicJavaNode::dump_spec() const { 
  tty->print("# Dynamic ");
  CallJavaNode::dump_spec();
}
#endif

//=============================================================================
uint CallCompiledJavaNode::size_of() const { return sizeof(*this); }
#ifndef PRODUCT
void CallCompiledJavaNode::dump_spec() const { 
  tty->print("# CompiledJava ");
  CallNode::dump_spec();
}
#endif

//------------------------------calling_convention-----------------------------
void CallCompiledJavaNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
  // Use the standard compiler calling convention 
  // for all arguments except the last, method_oop
  assert( argcnt > 0, "Need the method_oop in inline_cache_reg");
  uint java_arg_count = argcnt - 1;
  Matcher::calling_convention( parm_regs, java_arg_count, true );

  // Set up register assignments for extra parameter
  parm_regs[ java_arg_count + 0 ].set_ptr(Matcher::inline_cache_reg());
}


//=============================================================================
uint CallRuntimeNode::size_of() const { return sizeof(*this); }
//uint CallRuntimeNode::hash() const { return CallNode::hash() + _name[0]; }
uint CallRuntimeNode::cmp( const Node &n ) const { 
  CallRuntimeNode &call = (CallRuntimeNode&)n;
  return CallNode::cmp(call) && !strcmp(_name,call._name);
}
#ifndef PRODUCT
void CallRuntimeNode::dump_spec() const { 
  tty->print("# "); 
  tty->print(_name);
  CallNode::dump_spec();
}
#endif

//------------------------------calling_convention-----------------------------
void CallRuntimeNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
  Matcher::c_calling_convention( parm_regs, argcnt );
}

//=============================================================================
//------------------------------calling_convention-----------------------------
void CallInterpreterNode::calling_convention( OptoRegPair *parm_regs, uint argcnt ) const {
#ifdef _LP64
  // In the _LP64 build, interpreter stack slots are twice as big
#define STK_SCALE(x) ((x)<<1)
#else
#define STK_SCALE(x)  (x)
#endif
  assert( argcnt > 0, "Must have method_oop");
  // One extra argument was passed, the methodOop, so the number of Java args
  // is one less.
  uint java_argcnt = argcnt-1;
  // Java args are laid out on the stack in reverse order.
  for( uint i=0; i < java_argcnt; i++ ) {
    int reg;
    switch( parm_regs[i].ideal_reg() ) {
    case Op_RegL:
    case Op_RegD:
      assert( parm_regs[i+1].ideal_reg() == 0, "expected Half" );
      // While the argument list is reversed as a whole, longs and doubles
      // retain their natural layout - even if it's misaligned.
      reg = SharedInfo::stack2reg(STK_SCALE(java_argcnt-1 - (i+1)));
      parm_regs[i+0].set2(reg);
      parm_regs[i+1].set_bad(); // Skip the 'Half'
      i++;                      // Skip the 'Half'
      break;
    case Op_RegI:
    case Op_RegF:
      reg = SharedInfo::stack2reg(STK_SCALE(java_argcnt-1 - i));
      parm_regs[i].set1(reg);
      break;
    case Op_RegP:
      reg = SharedInfo::stack2reg(STK_SCALE(java_argcnt-1 - i));
      parm_regs[i].set_ptr(OptoReg::Name(reg));
      break;
    }
  }

  // Set up register assignment for the extra parameter
  parm_regs[ java_argcnt ].set_ptr(Matcher::interpreter_method_oop_reg());
}


//=============================================================================
#ifndef PRODUCT
void CallLeafNode::dump_spec() const { 
  tty->print("# "); 
  tty->print(_name);
  CallNode::dump_spec();
}
#endif

//=============================================================================
SafePointNode::SafePointNode(uint edges, JVMState* jvms) 
: MultiNode( edges ), _jvms(jvms), _oop_map(NULL) {
}

uint SafePointNode::size_of() const { return sizeof(*this); }
uint SafePointNode::cmp( const Node &n ) const { 
  return (&n == this);          // Always fail except on self
}

//-------------------------set_next_exception----------------------------------
void SafePointNode::set_next_exception(SafePointNode* n) {
  assert(!n || n->Opcode() == Op_SafePoint, "correct value for next_exception");
  if (len() == req()) {
    if (n != NULL)  add_prec(n);
  } else {
    set_prec(req(), n);
  }
}


//----------------------------next_exception-----------------------------------
SafePointNode* SafePointNode::next_exception() const {
  if (len() == req()) {
    return NULL;
  } else {
    Node* n = in(req());
    assert(!n || n->Opcode() == Op_SafePoint, "no other uses of prec edges");
    return (SafePointNode*) n;
  }
}


//------------------------------Ideal------------------------------------------
// Skip over any collapsed Regions
Node *SafePointNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if (remove_dead_region(phase, can_reshape))  return this;

  return NULL; 
}

//------------------------------Identity---------------------------------------
// Remove obviously duplicate safepoints
Node *SafePointNode::Identity( PhaseTransform *phase ) {

  // If you have back to back safepoints, remove one
  if( in(TypeFunc::Control)->is_SafePoint() )
    return in(TypeFunc::Control);

  if( in(0)->is_Proj() ) {
    Node *n0 = in(0)->in(0);
    // Check if he is a call projection
    if( n0->is_Call() || n0->is_Catch() ) 
      // Useless Safepoint, so remove it
      return in(TypeFunc::Control);
  }

  return this;
}

//------------------------------Value------------------------------------------
const Type *SafePointNode::Value( PhaseTransform *phase ) const {
  if( phase->type(in(0)) == Type::TOP ) return Type::TOP;
  if( phase->eqv( in(0), this ) ) return Type::TOP; // Dead infinite loop
  return Type::CONTROL;
}

#ifndef PRODUCT
void SafePointNode::dump_spec() const { 
  tty->print(" SafePoint "); 
}
#endif

const RegMask &SafePointNode::in_RegMask(uint idx) const { 
  if( idx < TypeFunc::Parms ) return RegMask::Empty;
  // Values outside the domain represent debug info
  return *(Compile::current()->matcher()->idealreg2debugmask[in(idx)->ideal_reg()]);
}
const RegMask &SafePointNode::out_RegMask() const {
  return RegMask::Empty;
}


void SafePointNode::grow_stack(JVMState* jvms, uint grow_by) {
  assert((int)grow_by > 0, "sanity");
  int monoff = jvms->monoff();
  int endoff = jvms->endoff();
  assert(endoff == (int)req(), "no other states or debug info after me");
  Node* top = Compile::current()->top();
  for (uint i = 0; i < grow_by; i++) {
    ins_req(monoff, top);
  }
  jvms->set_monoff(monoff + grow_by);
  jvms->set_endoff(endoff + grow_by);
}

void SafePointNode::push_monitor(const FastLockNode *lock) {
  // Add a FastLockNode, which points to both the original BoxLockNode (the
  // stack space for the monitor) and the Object being locked.
  const int MonitorEdges = 2;
  assert(JVMState::logMonitorEdges == exact_log2(MonitorEdges), "correct MonitorEdges");
  assert(req() == jvms()->endoff(), "correct sizing");
  if (GenerateSynchronizationCode) {
    add_req(lock->box_node());
    add_req(lock->obj_node());
  } else {
    add_req(NULL);
    add_req(NULL);
  }
  jvms()->set_endoff(req());
}

void SafePointNode::pop_monitor() {
  // Delete last monitor from debug info
  debug_only(int num_before_pop = jvms()->nof_monitors());
  const int MonitorEdges = (1<<JVMState::logMonitorEdges);
  int endoff = jvms()->endoff();
  int new_endoff = endoff - MonitorEdges;
  jvms()->set_endoff(new_endoff);
  while (endoff > new_endoff)  del_req(--endoff);
  assert(jvms()->nof_monitors() == num_before_pop-1, "");
}

Node *SafePointNode::peek_monitor_box() const {
  int mon = jvms()->nof_monitors() - 1;
  assert(mon >= 0, "most have a monitor");
  return monitor_box(jvms(), mon);
}

Node *SafePointNode::peek_monitor_obj() const {
  int mon = jvms()->nof_monitors() - 1;
  assert(mon >= 0, "most have a monitor");
  return monitor_obj(jvms(), mon);
}

// Do we Match on this edge index or not?  Match no edges
uint SafePointNode::match_edge(uint idx) const {
  if( !SafepointPolling || !needs_polling_address_input() )
    return 0;

  return (TypeFunc::Parms == idx);
}
