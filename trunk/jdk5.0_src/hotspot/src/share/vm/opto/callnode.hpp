#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)callnode.hpp	1.170 04/06/02 17:59:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style

class Chaitin;
class MultiNode;
class  SafePointNode;
class   CallNode;
class     CallJavaNode;
class       CallStaticJavaNode;
class       CallDynamicJavaNode;
class     CallCompiledJavaNode;
class     CallRuntimeNode;
class       CallInterpreterNode;
class       CallLeafNode;
class         CallLeafNoFPNode;
class       CallNativeNode;
class JVMState;
class OopMap;
class State;
class StartNode;
class   StartI2CNode;
class   StartC2INode;
class MachCallNode;
class FastLockNode;
class OptoRegPair;

//------------------------------StartNode--------------------------------------
// The method start node
class StartNode : public MultiNode {
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const; // Size is bigger
public:
  const TypeTuple *_domain;
  StartNode( Node *root, const TypeTuple *domain ) : MultiNode(2), _domain(domain) { set_req(0,this); set_req(1,root); }
  virtual int Opcode() const;
  virtual int pinned() const { return 1; };
  virtual bool  is_CFG        () const { return true; }
  virtual bool depends_only_on_test() const { return false; }
  virtual int   is_block_start() const { return 1; }
  virtual const Type *bottom_type() const;
  virtual const TypePtr *adr_type() const { return TypePtr::BOTTOM; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual StartNode *is_Start() { return this; }
  virtual void  calling_convention( OptoRegPair *parm_reg, uint length ) const;
  virtual const RegMask &in_RegMask(uint) const;
  virtual Node *match( const ProjNode *proj, const Matcher *m );
  virtual uint ideal_reg() const { return 0; }
#ifndef PRODUCT
  virtual void  dump_spec() const;
#endif
};

//------------------------------StartC2INode-----------------------------------
// The method start node for a Compiled to Interpreter frame converter
class StartC2INode : public StartNode {
public:
  StartC2INode( Node *root, const TypeTuple *domain ) : StartNode(root, domain) {}
  virtual int   Opcode() const;
  static  const TypeTuple *c2i_domain( const TypeTuple *domain );
  virtual void  calling_convention( OptoRegPair *parm_reg, uint length ) const;
};


//------------------------------StartI2CNode-----------------------------------
// The method start node for an Interpreter to Compiled frame converter
class StartI2CNode : public StartNode {
public:
  StartI2CNode( Node *root, const TypeTuple *domain ) : StartNode(root, domain) {}
  virtual int   Opcode() const;
  virtual void  calling_convention( OptoRegPair *parm_regs, uint argcnt ) const;
};


//------------------------------StartOSRNode-----------------------------------
// The method start node for on stack replacement code
class StartOSRNode : public StartNode {
public:
  StartOSRNode( Node *root, const TypeTuple *domain ) : StartNode(root, domain) {}
  virtual int   Opcode() const;
  static  const TypeTuple *osr_domain();
  virtual void  calling_convention( OptoRegPair *parm_regs, uint argcnt ) const;
  // Use the StartNode implementation of match
};


//------------------------------ParmNode---------------------------------------
// Incoming parameters
class ParmNode : public ProjNode {
  static const char * const names[TypeFunc::Parms+1];
public:
  ParmNode( StartNode *src, uint con ) : ProjNode(src,con) {}
  virtual int Opcode() const;
  virtual bool  is_CFG() const { return (_con == TypeFunc::Control); }
  virtual bool depends_only_on_test() const { return false; }
  virtual uint ideal_reg() const;
#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};


//------------------------------ReturnNode-------------------------------------
// Return from subroutine node
class ReturnNode : public Node {
public:
  ReturnNode( Node *cntrl, Node *i_o, Node *memory, Node *retadr, Node *frameptr );
  virtual int Opcode() const;
  virtual bool  is_CFG() const { return true; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual bool depends_only_on_test() const { return false; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual uint match_edge(uint idx) const;
#ifndef PRODUCT
  virtual void dump_req() const;
#endif
};


//------------------------------RethrowNode------------------------------------
// Rethrow of exception at call site.  Ends a procedure before rethrowing;
// ends the current basic block like a ReturnNode.  Restores registers and
// unwinds stack.  Rethrow happens in the caller's method.
class RethrowNode : public Node {
 public:
  RethrowNode( Node *cntrl, Node *i_o, Node *memory, Node *frameptr, Node *ret_adr, Node *exception );
  virtual int Opcode() const;
  virtual bool  is_CFG() const { return true; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual bool depends_only_on_test() const { return false; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint match_edge(uint idx) const;
  virtual uint ideal_reg() const { return NotAMachineReg; }
#ifndef PRODUCT
  virtual void dump_req() const;
#endif
};


//------------------------------TailCallNode-----------------------------------
// Pop stack frame and jump indirect
class TailCallNode : public ReturnNode {
public:
  TailCallNode( Node *cntrl, Node *i_o, Node *memory, Node *retadr, Node *frameptr, Node *target, Node *moop );

  virtual int Opcode() const;
  virtual uint match_edge(uint idx) const;
};

//------------------------------TailJumpNode-----------------------------------
// Pop stack frame and jump indirect
class TailJumpNode : public ReturnNode {
public:
  TailJumpNode( Node *cntrl, Node *i_o, Node *memory, Node *frameptr, Node *target, Node *ex_oop);

  virtual int Opcode() const;
  virtual uint match_edge(uint idx) const;
};

//-------------------------------JVMState-------------------------------------
// A linked list of JVMState nodes captures the whole interpreter state,
// plus GC roots, for all active calls at some call site in this compilation
// unit.  (If there is no inlining, then the list has exactly one link.)
// This provides a way to map the optimized program back into the interpreter,
// or to let the GC mark the stack.
class JVMState : public ResourceObj {
private:
  JVMState*         _caller;    // List pointer for forming scope chains
  uint              _depth;     // One mroe than caller depth, or one.
  uint              _locoff;    // Offset to locals in input edge mapping
  uint              _stkoff;    // Offset to stack in input edge mapping
  uint              _monoff;    // Offset to monitors in input edge mapping
  uint              _endoff;    // Offset to end of input edge mapping
  uint              _sp;        // Jave Expression Stack Pointer for this state
  int               _bci;       // Byte Code Index of this JVM point
  ciMethod*         _method;    // Method Pointer
  SafePointNode*    _map;       // Map node associated with this scope
public:
  friend class Compile;

  // Because JVMState objects live over the entire lifetime of the
  // Compile object, they are allocated into the comp_arena, which
  // does not get resource marked or reset during the compile process
  void *operator new( size_t x ) { return Compile::current()->comp_arena()->Amalloc(x); }
  void operator delete( void * ) { } // fast deallocation

  // Create a new JVMState, ready for abstract interpretation.
  JVMState(ciMethod* method, JVMState* caller);
  JVMState(int stack_size);  // root state; has a null method

  // Access functions for the JVM
  uint              locoff() const { return _locoff; }
  uint              stkoff() const { return _stkoff; }
  uint              argoff() const { return _stkoff + _sp; }
  uint              monoff() const { return _monoff; }
  uint              endoff() const { return _endoff; }
  uint              oopoff() const { return debug_end(); }

  int            loc_size() const { return _stkoff - _locoff; }
  int            stk_size() const { return _monoff - _stkoff; }
  int            mon_size() const { return _endoff - _monoff; }

  bool        is_loc(uint i) const { return i >= _locoff && i < _stkoff; }
  bool        is_stk(uint i) const { return i >= _stkoff && i < _monoff; }
  bool        is_mon(uint i) const { return i >= _monoff && i < _endoff; }

  uint              sp()     const { return _sp; }
  int               bci()    const { return _bci; }
  bool          has_method() const { return _method != NULL; }
  ciMethod*         method() const { assert(has_method(), ""); return _method; }
  JVMState*         caller() const { return _caller; }
  SafePointNode*    map()    const { return _map; }
  uint              depth()  const { return _depth; }
  uint        debug_start()  const; // returns locoff of root caller
  uint        debug_end()    const; // returns endoff of self
  uint        debug_size()   const { return loc_size() + sp() + mon_size(); }
  uint        debug_depth()  const; // returns sum of debug_size values at all depths

  // Returns the JVM state at the desired depth (1 == root).
  JVMState* of_depth(int d) const;

  // Tells if two JVM states have the same call chain (depth, methods, & bcis).
  bool same_calls_as(const JVMState* that) const;

  // Monitors (monitors are stored as (boxNode, objNode) pairs
  enum { logMonitorEdges = 1 };
  int  nof_monitors()              const { return mon_size() >> logMonitorEdges; }
  int  monitor_depth()             const { return nof_monitors() + (caller() ? caller()->monitor_depth() : 0); }
  int  monitor_box_offset(int idx) const { return monoff() + (idx << logMonitorEdges) + 0; }
  int  monitor_obj_offset(int idx) const { return monoff() + (idx << logMonitorEdges) + 1; }
  bool is_monitor_box(uint off)    const {
    assert(is_mon(off), "should be called only for monitor edge");
    return (0 == bitfield(off - monoff(), 0, logMonitorEdges));
  }
  bool is_monitor_use(uint off)    const { return (is_mon(off)
                                                   && is_monitor_box(off))
                                             || (caller() && caller()->is_monitor_use(off)); }

  // Initialization functions for the JVM
  void              set_locoff(uint off) { _locoff = off; }
  void              set_stkoff(uint off) { _stkoff = off; }
  void              set_monoff(uint off) { _monoff = off; }
  void              set_endoff(uint off) { _endoff = off; }
  void              set_offsets(uint off) { _locoff = _stkoff = _monoff = _endoff = off; }
  void              set_map(SafePointNode *map) { _map = map; }
  void              set_sp(uint sp) { _sp = sp; }
  void              set_bci(int bci) { _bci = bci; }

  // Miscellaneous utility functions
  JVMState* clone_deep() const;      // recursively clones caller chain
  JVMState* clone_shallow() const;   // retains uncloned caller

#ifndef PRODUCT
  void      format(PhaseRegAlloc *regalloc, const Node *n) const;
  void      dump_spec() const;
  void      dump() const;
#endif
};

//------------------------------SafePointNode----------------------------------
// A SafePointNode is a subclass of a MultiNode for convenience (and
// potential code sharing) only - conceptually it is independent of
// the Node semantics.
class SafePointNode : public MultiNode {
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint           cmp( const Node &n ) const;
  virtual uint           size_of() const;       // Size is bigger

public:
  SafePointNode(uint edges, JVMState* jvms);
  
  OopMap*         _oop_map;   // Array of OopMap info (8-bit char) for GC
  JVMState* const _jvms;      // Pointer to list of JVM State objects

  virtual JVMState* jvms() const { return _jvms; }
  void set_jvms(JVMState* s) {
    *(JVMState**)&_jvms = s;  // override const attribute in the accessor
  }
  OopMap *oop_map() const { return _oop_map; }
  void set_oop_map(OopMap *om) { _oop_map = om; }

  // Functionality from old debug nodes which has changed
  Node *local(JVMState* jvms, uint idx) const {
    assert(verify_jvms(jvms), "jvms must match");
    return in(jvms->locoff() + idx);
  }
  Node *stack(JVMState* jvms, uint idx) const {
    assert(verify_jvms(jvms), "jvms must match");
    return in(jvms->stkoff() + idx);
  }
  Node *argument(JVMState* jvms, uint idx) const {
    assert(verify_jvms(jvms), "jvms must match");
    return in(jvms->argoff() + idx);
  }
  Node *monitor_box(JVMState* jvms, uint idx) const {
    assert(verify_jvms(jvms), "jvms must match");
    return in(jvms->monitor_box_offset(idx));
  }
  Node *monitor_obj(JVMState* jvms, uint idx) const {
    assert(verify_jvms(jvms), "jvms must match");
    return in(jvms->monitor_obj_offset(idx));
  }
  void  set_local(JVMState* jvms, uint idx, Node *c) {
    assert(verify_jvms(jvms), "jvms must match");
    set_req(jvms->locoff() + idx, c);
  }
  void  set_stack(JVMState* jvms, uint idx, Node *c) {
    assert(verify_jvms(jvms), "jvms must match");
    set_req(jvms->stkoff() + idx, c);
  }
  void  set_argument(JVMState* jvms, uint idx, Node *c) {
    assert(verify_jvms(jvms), "jvms must match");
    set_req(jvms->argoff() + idx, c);
  }
  void ensure_stack(JVMState* jvms, uint stk_size) {
    assert(verify_jvms(jvms), "jvms must match");
    int grow_by = (int)stk_size - (int)jvms->stk_size();
    if (grow_by > 0)  grow_stack(jvms, grow_by);
  }
  void grow_stack(JVMState* jvms, uint grow_by);
  // Handle monitor stack
  void push_monitor( const FastLockNode *lock );
  void pop_monitor ();
  Node *SafePointNode::peek_monitor_box() const;
  Node *SafePointNode::peek_monitor_obj() const;

  // Access functions for the JVM
  Node *control  () const { return in(TypeFunc::Control  ); }
  Node *i_o      () const { return in(TypeFunc::I_O      ); }
  Node *memory   () const { return in(TypeFunc::Memory   ); }
  Node *returnadr() const { return in(TypeFunc::ReturnAdr); }
  Node *frameptr () const { return in(TypeFunc::FramePtr ); }

  void set_control  ( Node *c ) { set_req(TypeFunc::Control,c); }
  void set_i_o      ( Node *c ) { set_req(TypeFunc::I_O    ,c); }
  void set_memory   ( Node *c ) { set_req(TypeFunc::Memory ,c); }

  MergeMemNode* merged_memory() const {
    Node* mem = in(TypeFunc::Memory);
    assert(mem->Opcode() == Op_MergeMem, "memory must be pre-split");
    return (MergeMemNode*)mem;
  }

  // Exception states bubbling out of subgraphs such as inlined calls
  // are recorded here.  (There might be more than one, hence the "next".)
  // This feature is used only for safepoints which serve as "maps"
  // for JVM states during parsing, intrinsic expansion, etc.
  SafePointNode*         next_exception() const;
  void               set_next_exception(SafePointNode* n);
  bool                   has_exceptions() const { return next_exception() != NULL; }

  // Standard Node stuff
  virtual int            Opcode() const;
  virtual int            pinned() const { return 1; }
  virtual bool           is_CFG() const { return true; }
  virtual const Type    *Value( PhaseTransform *phase ) const;
  virtual const Type    *bottom_type() const { return Type::CONTROL; }
  virtual const TypePtr *adr_type() const { return TypePtr::BOTTOM; }
  virtual Node          *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual Node          *Identity( PhaseTransform *phase );
  virtual uint           ideal_reg() const { return 0; }
  virtual const RegMask &in_RegMask(uint) const;
  virtual const RegMask &out_RegMask() const;
  virtual SafePointNode *is_SafePoint() { return this; }
  virtual uint           match_edge(uint idx) const;

  static  bool           needs_polling_address_input();

#ifndef PRODUCT
  virtual void              dump_spec() const;
#endif
};

//------------------------------CallNode---------------------------------------
// Call nodes now subsume the function of debug nodes at callsites, so they
// contain the functionality of a full scope chain of debug nodes.
class CallNode : public SafePointNode {
public:
  const TypeFunc *_tf;        // Function type
  address      _entry_point;  // Address of method being called

  CallNode(const TypeFunc *tf , address addr)
    : SafePointNode(tf->domain()->cnt(), NULL), _tf(tf), _entry_point(addr) {}

  const TypeFunc* tf()        const { return _tf; }
  const address entry_point() const { return _entry_point; }

  void set_tf(const TypeFunc* tf) { _tf = tf; }
  void set_entry_point(address p) { _entry_point = p; }

  virtual const Type *bottom_type() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase ) { return this; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint        cmp( const Node &n ) const;
  virtual uint        size_of() const = 0;
  virtual void        calling_convention( OptoRegPair *parm_regs, uint argcnt ) const;
  virtual Node       *match( const ProjNode *proj, const Matcher *m );
  virtual uint        ideal_reg() const { return NotAMachineReg; }

  // Node casting functions
  virtual       CallNode             *is_Call() { return this; }
  virtual const CallJavaNode         *is_CallJava() const { return 0; }
  virtual const CallStaticJavaNode   *is_CallStaticJava() const { return 0; }
  virtual const CallDynamicJavaNode  *is_CallDynamicJava() const { return 0; }
  virtual const CallCompiledJavaNode *is_CallCompiledJava() const { return 0; }
  virtual const CallInterpreterNode  *is_CallInterpreter() const { return 0; }
  virtual const CallRuntimeNode      *is_CallRuntime() const { return 0; }
  virtual const CallLeafNode         *is_CallLeaf() const { return 0; }  

  virtual uint match_edge(uint idx) const;

#ifndef PRODUCT
  virtual void        dump_req()  const;
  virtual void        dump_spec() const;
#endif
};

//------------------------------CallJavaNode-----------------------------------
// Make a static or dynamic subroutine call node using Java calling
// convention.  (The "Java" calling convention is the compiler's calling
// convention, as opposed to the interpreter's or that of native C.)
class CallJavaNode : public CallNode {
protected:
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const; // Size is bigger

  bool    _optimized_virtual;
  ciMethod* _method;            // Method being direct called
public:
  const int       _bci;         // Byte Code Index of call byte code
  CallJavaNode( const TypeFunc *tf , address addr, ciMethod* method, int bci ) : CallNode(tf,addr), _method(method), _bci(bci), _optimized_virtual(false) { }

  virtual int   Opcode() const;
  virtual const CallJavaNode *is_CallJava() const { return this; }
  ciMethod* method() const                { return _method; }
  void  set_method(ciMethod *m)           { _method = m; }
  void  set_optimized_virtual(bool f)     { _optimized_virtual = f; }
  bool  is_optimized_virtual() const      { return _optimized_virtual; }

#ifndef PRODUCT
  virtual void  dump_spec() const;
#endif
};

//------------------------------CallStaticJavaNode-----------------------------
// Make a direct subroutine call using Java calling convention (for static
// calls and optimized virtual calls, plus calls to wrappers for run-time
// routines); generates static stub.
class CallStaticJavaNode : public CallJavaNode {
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const; // Size is bigger
public:
  CallStaticJavaNode( const TypeFunc *tf, address addr, ciMethod* method, int bci ) : CallJavaNode(tf,addr,method,bci), _name(0) {}
  CallStaticJavaNode( const TypeFunc *tf, address addr, const char *name, int bci ) : CallJavaNode(tf,addr,0,bci), _name(name) {}  
  const char *_name;            // Runtime wrapper name

  virtual int         Opcode() const;
  virtual const       CallStaticJavaNode *is_CallStaticJava() const { return this; }
#ifndef PRODUCT
  virtual void        dump_spec() const;
#endif
};

//------------------------------CallDynamicJavaNode----------------------------
// Make a dispatched call using Java calling convention.
class CallDynamicJavaNode : public CallJavaNode {
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const; // Size is bigger
public:
  CallDynamicJavaNode( const TypeFunc *tf , address addr, ciMethod* method, int bci ) : CallJavaNode(tf,addr,method,bci) {}

  virtual int   Opcode() const;
  virtual const CallDynamicJavaNode *is_CallDynamicJava() const { return this; }
#ifndef PRODUCT
  virtual void  dump_spec() const;
#endif
};

//------------------------------CallCompiledJavaNode---------------------------
// Make a direct subroutine call node from interpreter to compiled java
class CallCompiledJavaNode : public CallNode {
  virtual uint size_of() const;
public:
  CallCompiledJavaNode( const TypeFunc *tf , address addr ) : CallNode(tf,addr) {}

  virtual int   Opcode() const;
  virtual const CallCompiledJavaNode *is_CallCompiledJava() const { return this; }
  virtual void  calling_convention( OptoRegPair *parm_regs, uint argcnt ) const;
#ifndef PRODUCT
  virtual void  dump_spec() const;
#endif
};

//------------------------------CallRuntimeNode--------------------------------
// Make a direct subroutine call node into compiled C++ code.
class CallRuntimeNode : public CallNode {
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const; // Size is bigger
public:
  CallRuntimeNode( const TypeFunc *tf, address addr, const char *name ) : CallNode(tf,addr), _name(name) {}

  const char *_name;            // Printable name, if _method is NULL
  virtual int   Opcode() const;
  virtual const CallRuntimeNode *is_CallRuntime() const { return this; }
  virtual void  calling_convention( OptoRegPair *parm_regs, uint argcnt ) const;

#ifndef PRODUCT
  virtual void  dump_spec() const;
#endif
};

//------------------------------CallInterpreterNode----------------------------
// Make a direct subroutine call node into the interpreter
class CallInterpreterNode : public CallRuntimeNode {
public:
  CallInterpreterNode( const TypeFunc *tf , address addr ) : CallRuntimeNode(tf,addr,"Interpreter") {}
  virtual int   Opcode() const;
  virtual const CallInterpreterNode *is_CallInterpreter() const { return this; }
  // Handle additional parameters, inline_cached_klass and method_oop
  virtual void  calling_convention( OptoRegPair *parm_regs, uint argcnt ) const;
};

//------------------------------CallLeafNode-----------------------------------
// Make a direct subroutine call node into compiled C++ code, without
// safepoints
class CallLeafNode : public CallRuntimeNode {
public:
  CallLeafNode( const TypeFunc *tf, address addr, const char *name ) : 
      CallRuntimeNode(tf,addr,name){}
  virtual int   Opcode() const;
  virtual const CallLeafNode *is_CallLeaf() const { return this; }
#ifndef PRODUCT
  virtual void  dump_spec() const;
#endif
};

//------------------------------CallLeafNoFPNode-------------------------------
// CallLeafNode, not using floating point or using it in the same manner as
// the generated code
class CallLeafNoFPNode : public CallLeafNode {
public:
  CallLeafNoFPNode( const TypeFunc *tf, address addr, const char *name ) : 
      CallLeafNode(tf,addr,name){}
  virtual int   Opcode() const;
};

//------------------------------CallNativeNode---------------------------------
// Make a direct subroutine call node into Native code.
class CallNativeNode : public CallRuntimeNode {
public:
  CallNativeNode( const TypeFunc *tf, address addr, const char *name ) : CallRuntimeNode(tf,addr,name) {}
  virtual int Opcode() const;
};




