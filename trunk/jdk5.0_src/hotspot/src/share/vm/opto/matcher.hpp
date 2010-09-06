#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)matcher.hpp	1.163 04/05/04 13:26:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Compile;
class Node;
class MachNode;
class MachTypeNode;
class MachOper;

//---------------------------Matcher-------------------------------------------
// Pairs of registers for arguments.  First set ideal_reg in all entries,
// then call calling_convention and the structs are written over with the
// calling convention's registers.  OptoReg::Bad is returned for any unused
// register.  This happens for the unused high half of Int arguments, or
// for 32-bit pointers, or for Half's.  Longs & Doubles always return a high
// and a low register, as do 64-bit pointers.
class OptoRegPair {
private:
  short _hi;
  short _lo;
public:
  void set_ideal_reg( int ideal_reg ) { _lo = ideal_reg; debug_only(_hi=OptoReg::Bad;) }
  int ideal_reg() const { return _lo; }
  void set_bad (                   ) { _hi=OptoReg::Bad; _lo=OptoReg::Bad; }
  void set1    (         int lo    ) { _hi=OptoReg::Bad; _lo=lo; }
  void set2    (         int lo    ) { _hi=lo+1;         _lo=lo; }
  void set_pair( int hi, int lo    ) { _hi=hi;           _lo=lo; }
  void set_ptr ( OptoReg::Name ptr ) {
#ifdef _LP64
    _hi = ptr+1;
#else
    _hi = OptoReg::Bad;
#endif
    _lo = ptr;
  }
  OptoReg::Name hi() const { return OptoReg::Name(_hi); }
  OptoReg::Name lo() const { return OptoReg::Name(_lo); }
  OptoRegPair(int h, int l) : _hi(h),_lo(l) {}
};

//---------------------------Matcher-------------------------------------------
class Matcher : public PhaseTransform {
  friend class VMStructs;
  // Private arena of State objects
  ResourceArea _states;

  VectorSet   _visited;         // Visit bits

  // Used to control the Label pass
  VectorSet   _shared;          // Shared Ideal Node
  VectorSet   _dontcare;        // Nothing the matcher cares about

  // Private methods which perform the actual matching and reduction
  // Walks the label tree, generating machine nodes
  MachNode *ReduceInst( State *s, int rule, Node *&mem);
  void ReduceInst_Chain_Rule( State *s, int rule, MachNode *mach, Node *&mem);
  uint ReduceInst_Interior(State *s, int rule, MachNode *mach, uint num_opnds, Node *&mem);
  void ReduceOper( State *s, int newrule, MachNode *mach, Node *&mem );

  // Convert a dense opcode number to an expanded rule number
  const int *_reduceOp;
  const int *_leftOp;
  const int *_rightOp;

  // Map dense opcode number to info on when rule is swallowed constant.
  const bool *_swallowed;
  
  // Map dense rule number to determine if this is an instruction chain rule
  const uint _begin_inst_chain_rule;
  const uint _end_inst_chain_rule;
  
  // We want to clone constants and possible CmpI-variants.
  // If we do not clone CmpI, then we can have many instances of
  // condition codes alive at once.  This is OK on some chips and
  // bad on others.  Hence the machine-dependent table lookup.
  const char *_must_clone;

  // Find shared Nodes, or Nodes that otherwise are Matcher roots
  void find_shared( Node *n );

  // Node labeling iterator for instruction selection
  Node *Label_Root( const Node *n, State *svec, Node *control, Node *mem );
  
  Node *transform( Node *dummy );

  Node_List &_proj_list;        // For Machine nodes killing many values

  debug_only(Node_Array _old2new_map;)   // Map roots of ideal-trees to machine-roots

  debug_only(uint *_matchstat;) // Track node types for ins_req

  // Accessors for the inherited field PhaseTransform::_nodes:
  void   grow_new_node_array(uint idx_limit) {
    _nodes.map(idx_limit-1, NULL);
  }
  bool    has_new_node(const Node* n) const {
    return _nodes.at(n->_idx) != NULL;
  }
  Node*       new_node(const Node* n) const {
    assert(has_new_node(n), "set before get");
    return _nodes.at(n->_idx);
  }
  void    set_new_node(const Node* n, Node *nn) {
    assert(!has_new_node(n), "set only once");
    _nodes.map(n->_idx, nn);
  }

public:
  static const int base2reg[];        // Map Types to machine register types
  // Convert ideal machine register to a register mask for spill-loads
  static const RegMask *idealreg2regmask[];
  RegMask *idealreg2spillmask[_last_machine_leaf];
  RegMask *idealreg2debugmask[_last_machine_leaf];
  void init_spill_mask( Node *ret );
  // Convert machine register number to register mask
  static uint mreg2regmask_max;
  static RegMask mreg2regmask[];
  static RegMask STACK_ONLY_mask;

  bool    is_shared( Node *n ) { return _shared.test(n->_idx); }
  void   set_shared( Node *n ) {  _shared.set(n->_idx); }
  bool   is_visited( Node *n ) { return _visited.test(n->_idx); }
  void  set_visited( Node *n ) { _visited.set(n->_idx); }
  bool  is_dontcare( Node *n ) { return _dontcare.test(n->_idx); }
  void set_dontcare( Node *n ) {  _dontcare.set(n->_idx); }

  // Mode bit to tell DFA and expand rules whether we are running after
  // (or during) register selection.  Usually, the matcher runs before,
  // but it will also get called to generate post-allocation spill code.
  // In this situation, it is a deadly error to attempt to allocate more
  // temporary registers.
  bool _allocation_started;

  // Machine register names
  static const char *regName[];
  // Machine register encodings
  static const unsigned char _regEncode[];
  // Machine Node names
  const char **_ruleName;
  // Rules that are cheaper to rematerialize than to spill
  static const uint _begin_rematerialize;
  static const uint _end_rematerialize;

  // An array of chars, from 0 to _last_Mach_Reg.
  // No Save       = 'N' (for register windows)
  // Save on Entry = 'E'
  // Save on Call  = 'C'
  // Always Save   = 'A' (same as SOE + SOC)
  const char *_register_save_policy;
  const char *_c_reg_save_policy;
  // Convert a machine register to a machine register type, so-as to
  // properly match spill code.
  const int *_register_save_type;
  // Maps from machine register to boolean; true if machine register can
  // be holding a call argument in some signature.
  static bool can_be_arg( int reg );
  // Maps from machine register to boolean; true if machine register holds
  // a spillable argument.
  static bool is_spillable_arg( int reg );

  // List of IfFalse or IfTrue Nodes that indicate a taken null test.
  // List is valid in the post-matching space.
  Node_List _null_check_tests;
  void collect_null_checks( Node *proj );
  void validate_null_checks( );

  Matcher( Node_List &proj_list );

  // Select instructions for entire method
  void  match( );
  // Helper for match
  OptoReg::Name adjust_incoming_stk_arg( OptoReg::Name reg );

  // Transform, then walk.  Does implicit DCE while walking.
  // Name changed from "transform" to avoid it being virtual.
  Node *xform( Node *old_space_node, int Nodes );

  // Match a single Ideal Node - turn it into a 1-Node tree; Label & Reduce.
  MachNode *match_tree( const Node *n );
  MachNode *match_sfpt( SafePointNode *sfpt );
  // Helper for match_sfpt
  OptoReg::Name adjust_outgoing_stk_arg( int reg, OptoReg::Name begin_out_arg_area, OptoReg::Name &out_arg_limit_per_call );

  // Initialize first stack mask and related masks.
  void init_first_stack_mask();

  // If we should save-on-entry this register
  bool is_save_on_entry( int reg );

  // Fixup the save-on-entry registers
  void Fixup_Save_On_Entry( );

  // --- Frame handling ---

  // Register number of the stack slot corresponding to the incoming SP.
  // Per the Big Picture in the AD file, it is:
  //   SharedInfo::stack0 + locks + in_preserve_stack_slots + pad2.
  OptoReg::Name _old_SP;

  // Register number of the stack slot corresponding to the highest incoming
  // argument on the stack.  Per the Big Picture in the AD file, it is:
  //   _old_SP + out_preserve_stack_slots + incoming argument size.
  OptoReg::Name _in_arg_limit;

  // Register number of the stack slot corresponding to the new SP.
  // Per the Big Picture in the AD file, it is:
  //   _in_arg_limit + pad0
  OptoReg::Name _new_SP;

  // Register number of the stack slot corresponding to the highest outgoing
  // argument on the stack.  Per the Big Picture in the AD file, it is:
  //   _new_SP + max outgoing arguments of all calls
  OptoReg::Name _out_arg_limit;

  OptoRegPair *_parm_regs;        // Array of machine registers per argument
  RegMask *_calling_convention_mask; // Array of RegMasks per argument

  // Does hardware support 'fast' versions of these?
  static const bool modFSupported(void);
  static const bool modDSupported(void);
  static const bool sinDSupported(void);
  static const bool cosDSupported(void);
  static const bool tanDSupported(void);
  static const bool atanDSupported(void);
  static const bool sqrtDSupported(void);
  static const bool powDSupported(void);

  // Used to determine if we have fast l2f conversion
  // USII has it, USIII doesn't
  static const bool convL2FSupported(void);

  // Used to determine if we have JumpTable support
  static const bool jumpTableSupported(void);

  // These calls are all generated by the ADLC

  // TRUE - grows up, FALSE - grows down (Intel)
  virtual bool stack_direction() const;

  // Java-Java calling convention
  // (what you use when Java calls Java)

  // Alignment of stack in bits, standard Intel word alignment is 5.
  // Sparc probably wants at least double-word (6).
  static uint stack_alignment();
  // Array mapping arguments to registers.  Argument 0 is usually the 'this' 
  // pointer.  Registers can include stack-slots and regular registers.
  static void calling_convention( OptoRegPair *, uint len, bool is_outgoing );

  // Convert a sig into a calling convention register layout
  // and find interesting things about it.
  static VMReg::Name     find_receiver( bool is_outgoing );
  static OptoRegPair* find_callee_arguments(symbolOop sig, bool is_static, int *arg_size);
  // Return address register.  On Intel it is a stack-slot.  On PowerPC
  // it is the Link register.  On Sparc it is r31?
  virtual OptoReg::Name return_addr() const;
  RegMask              _return_addr_mask;
  // Return value register.  On Intel it is EAX.  On Sparc i0/o0.
  static OptoRegPair   return_value(int ideal_reg, bool is_outgoing);
  static OptoRegPair c_return_value(int ideal_reg, bool is_outgoing);
  RegMask                     _return_value_mask;
  // Inline Cache Register
  static OptoReg::Name  inline_cache_reg();
  static const RegMask &inline_cache_reg_mask();
  static int            inline_cache_reg_encode();
  

  // Java-Interpreter calling convention
  // (what you use when calling between compiled-Java and Interpreted-Java

  // Number of callee-save + always-save registers 
  // Ignores frame pointer and "special" registers
  static int  number_of_saved_registers();

  // The Method-klass-holder may be passed in the inline_cache_reg
  // and then expanded into the inline_cache_reg and a method_oop register
  static OptoReg::Name  interpreter_arg_ptr_reg();
  static const RegMask &interpreter_arg_ptr_reg_mask();
  static int            interpreter_arg_ptr_reg_encode();

  static OptoReg::Name  interpreter_method_oop_reg();
  static const RegMask &interpreter_method_oop_reg_mask();
  static int            interpreter_method_oop_reg_encode();

  static OptoReg::Name  compiler_method_oop_reg();
  static const RegMask &compiler_method_oop_reg_mask();
  static int            compiler_method_oop_reg_encode();

  // Interpreter's Frame Pointer Register
  static OptoReg::Name  interpreter_frame_pointer_reg();
  static const RegMask &interpreter_frame_pointer_reg_mask();

  // Java-Native calling convention 
  // (what you use when intercalling between Java and C++ code)

  // Array mapping arguments to registers.  Argument 0 is usually the 'this' 
  // pointer.  Registers can include stack-slots and regular registers.
  static void c_calling_convention( OptoRegPair *, uint );
  // Frame pointer. The frame pointer is kept at the base of the stack
  // and so is probably the stack pointer for most machines.  On Intel
  // it is ESP.  On the PowerPC it is R1.  On Sparc it is SP.
  OptoReg::Name  c_frame_pointer() const;
  static RegMask c_frame_ptr_mask;

  // !!!!! Special stuff for building ScopeDescs
  virtual int      regnum_to_fpu_offset(int regnum);

  // Is this branch offset small enough to be addressed by a short branch?
  bool is_short_branch_offset(int offset);

  // Should the Matcher clone shifts on addressing modes, expecting them to
  // be subsumed into complex addressing expressions or compute them into
  // registers?  True for Intel but false for most RISCs
  static const bool clone_shift_expressions;
  
  // Is it better to copy float constants, or load them directly from memory?
  // Intel can load a float constant from a direct address, requiring no
  // extra registers.  Most RISCs will have to materialize an address into a
  // register first, so they may as well materialize the constant immediately.
  static const bool rematerialize_float_constants;

  // If CPU can load and store mis-aligned doubles directly then no fixup is
  // needed.  Else we split the double into 2 integer pieces and move it
  // piece-by-piece.  Only happens when passing doubles into C code or when
  // calling i2c adapters as the Java calling convention forces doubles to be
  // aligned.
  static const bool misaligned_doubles_ok;

  // Perform a platform dependent implicit null fixup.  This is needed
  // on windows95 to take care of some unusual register constraints.
  void pd_implicit_null_fixup(MachNode *load, uint idx);

  // Advertise here if the CPU requires explicit rounding operations
  // to implement the UseStrictFP mode.
  static const bool strict_fp_requires_explicit_rounding;

  // Do floats take an entire double register or just half?
  static const bool float_in_double;
  // Do ints take an entire long register or just half?
  static const bool int_in_long;

  // What is the range of offsets for allocator spill instructions?  Offsets
  // larger than this will encode to a 'large' instruction and offsets same
  // size or smaller will encode to a 'small' instruction.  On Sparc the
  // 'small' offset is from 0 to 4096; offsets larger than this will not have
  // any sane encoding (there's no spare register to build up a large offset).
  // However, 4096 should be plenty large enough.  On Intel the 'small' offset
  // is from 0 to 127; 'large' offsets are +128 on up.  The allocator will
  // match both large and small versions of load/store [SP+offset]
  // instructions, and will clone such instructions in fixup_spills and patch
  // in the correct offset.
  static const int short_spill_offset_limit;

  // This routine is run whenever a graph fails to match.
  // If it returns, the compiler should bailout to interpreter without error.
  // In non-product mode, SoftMatchFailure is false to detect non-canonical
  // graphs.  Print a message and exit.
  static void soft_match_failure() {
    if( SoftMatchFailure ) return;
    else { fatal("SoftMatchFailure is not allowed except in product"); }
  }

  // Used by the DFA in dfa_sparc.cpp.  Check for a prior FastLock
  // acting as an Acquire and thus we don't need an Acquire here.  We
  // retain the Node to act as a compiler ordering barrier.
  static bool prior_fast_lock( const Node *acq );

  // Used by the DFA in dfa_sparc.cpp.  Check for a following
  // FastUnLock acting as a Release and thus we don't need a Release
  // here.  We retain the Node to act as a compiler ordering barrier.
  static bool post_fast_unlock( const Node *rel );

  // Check for a following volatile memory barrier without an
  // intervening load and thus we don't need a barrier here.  We
  // retain the Node to act as a compiler ordering barrier.
  static bool post_store_load_barrier(const Node* mb);


#ifdef ASSERT
  void dump_old2new_map();      // machine-independent to machine-dependent
#endif
};
