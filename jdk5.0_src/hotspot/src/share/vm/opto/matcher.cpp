#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)matcher.cpp	1.351 04/05/04 13:26:16 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_matcher.cpp.incl"



const int Matcher::base2reg[Type::lastype] = {
  Node::NotAMachineReg,0,0, Op_RegI, Op_RegL, 0, 
  Node::NotAMachineReg, Node::NotAMachineReg, /* tuple, array */
  Op_RegP, Op_RegP, Op_RegP, Op_RegP, Op_RegP, Op_RegP, /* the pointers */
  0, 0/*abio*/,
  Op_RegP /* Return address */, 0, /* the memories */
  Op_RegF, Op_RegF, Op_RegF, Op_RegD, Op_RegD, Op_RegD, 
  0  /*bottom*/
};

const RegMask *Matcher::idealreg2regmask[_last_machine_leaf];
RegMask Matcher::mreg2regmask[_last_Mach_Reg];
RegMask Matcher::STACK_ONLY_mask;
RegMask Matcher::c_frame_ptr_mask;
const uint Matcher::_begin_rematerialize = _BEGIN_REMATERIALIZE;
const uint Matcher::_end_rematerialize   = _END_REMATERIALIZE;

//---------------------------Matcher-------------------------------------------
Matcher::Matcher( Node_List &proj_list ) :
  PhaseTransform( Phase::Ins_Select ),
#ifdef ASSERT
  _old2new_map(C->comp_arena()),
#endif
  _reduceOp(reduceOp), _leftOp(leftOp), _rightOp(rightOp), 
  _swallowed(swallowed), 
  _begin_inst_chain_rule(_BEGIN_INST_CHAIN_RULE), 
  _end_inst_chain_rule(_END_INST_CHAIN_RULE), 
  _must_clone(must_clone), _proj_list(proj_list),
  _register_save_policy(register_save_policy),
  _c_reg_save_policy(c_reg_save_policy),
  _register_save_type(register_save_type),
  _ruleName(ruleName), 
  _allocation_started(false),
  _states(Chunk::medium_size),
  _visited(&_states),
  _shared(&_states),
  _dontcare(&_states) {
  C->set_matcher(this);

  idealreg2spillmask[Op_RegI] = NULL;
  idealreg2spillmask[Op_RegL] = NULL;
  idealreg2spillmask[Op_RegF] = NULL;
  idealreg2spillmask[Op_RegD] = NULL;
  idealreg2spillmask[Op_RegP] = NULL;

  idealreg2debugmask[Op_RegI] = NULL;
  idealreg2debugmask[Op_RegL] = NULL;
  idealreg2debugmask[Op_RegF] = NULL;
  idealreg2debugmask[Op_RegD] = NULL;
  idealreg2debugmask[Op_RegP] = NULL;
}

//------------------------------adjust_incoming_stk_arg------------------------
OptoReg::Name Matcher::adjust_incoming_stk_arg( OptoReg::Name reg ) {
  if( reg >= SharedInfo::stack0 ) {  // Stack slot argument?
    reg = OptoReg::add(_old_SP, SharedInfo::reg2stack(reg) );
    reg = OptoReg::add(reg, C->out_preserve_stack_slots());
    if( reg >= _in_arg_limit )
      _in_arg_limit = OptoReg::add(reg,1); // Bump max stack slot seen
    if (!RegMask::can_represent(reg)) {
      // the compiler cannot represent this method's calling sequence
      C->record_method_not_compilable_all_tiers("unsupported incoming calling sequence");
      return OptoReg::Bad;
    }
  }
  return reg;
}

//---------------------------compute_old_SP------------------------------------
OptoReg::Name Compile::compute_old_SP() {
  int locks    = sync() * sync_stack_slots();
  int preserve = in_preserve_stack_slots();
  return SharedInfo::stack2reg(round_to(locks + preserve, 1 << (Matcher::stack_alignment() - LogBitsPerInt)));
}


//---------------------------compute_new_SP------------------------------------
OptoReg::Name Compile::compute_new_SP(int in_arg_size) {
  OptoReg::Name old_SP = compute_old_SP();
  int preserve = out_preserve_stack_slots();  // the OLD out_preserve!
  assert(is_even(old_SP), "SP must always be even");
  int old_words = round_to(preserve + in_arg_size, 1 << (Matcher::stack_alignment() - LogBitsPerInt));
  return OptoReg::add(old_SP, old_words);
}
//---------------------------compute_out_arg_base------------------------------
OptoReg::Name Compile::compute_out_arg_base(OptoReg::Name new_SP) {
  int preserve = out_preserve_stack_slots();  // the NEW out_preserve!
  return OptoReg::add(new_SP, preserve);
}

bool Compile::can_generate_C2I(ciMethod* method, int compiled_arg_size) {
  int slots = in_preserve_stack_slots();
  slots = round_to(slots, 1 << (Matcher::stack_alignment() - LogBitsPerInt));
  // slots == reg2stack(_old_SP of adapter)
  slots += out_preserve_stack_slots();
  slots += round_to(compiled_arg_size, RegMask::SlotsPerLong);
  // slots == reg2stack(_new_SP of adapter)
  slots += out_preserve_stack_slots();
  slots += method->arg_size() * (wordSize/jintSize);
  // All of the preceding stack slots must be bind-able in a regmask.
  return RegMask::can_represent(SharedInfo::stack2reg(slots));
}

//---------------------------match---------------------------------------------
void Matcher::match( ) { 
  // One-time initialization of some register masks.
  init_spill_mask( C->root()->in(1) );
  _return_addr_mask = return_addr();
#ifdef _LP64
  // Pointers take 2 slots in 64-bit land
  _return_addr_mask.Insert(OptoReg::add(return_addr(),1));
#endif

  // Map a Java-signature return type into return register-value
  // machine registers for 0, 1 and 2 returned values.
  const TypeTuple *range = C->tf()->range();
  if( range->cnt() > TypeFunc::Parms ) { // If not a void function
    // Get ideal-register return type
    int ireg = base2reg[range->field_at(TypeFunc::Parms)->base()];
    // Get machine return register
    uint sop = C->start()->Opcode();
    OptoRegPair regs = (sop == Op_StartI2C || sop == Op_StartOSR)
      ? c_return_value(ireg,false)
      :   return_value(ireg,false);

    // And mask for same
    _return_value_mask = RegMask(regs.lo());
    if( regs.hi() != OptoReg::Bad )
      _return_value_mask.Insert(regs.hi());
  }

  // ---------------
  // Frame Layout

  // Need the method signature to determine the incoming argument types,
  // because the types determine which registers the incoming arguments are
  // in, and this affects the matched code.
  const TypeTuple *domain = C->tf()->domain();
  uint             argcnt = domain->cnt() - TypeFunc::Parms;
  _parm_regs = NEW_RESOURCE_ARRAY( OptoRegPair, argcnt );
  _calling_convention_mask = NEW_RESOURCE_ARRAY( RegMask, argcnt );
  uint i;
  for( i = 0; i<argcnt; i++ )
    _parm_regs[i].set_ideal_reg(base2reg[domain->field_at(i+TypeFunc::Parms)->base()]);

  // Pass array of ideal registers and length to USER code (from the AD file)
  // that will convert this to an array of register numbers.
  const StartNode *start = C->start();
  start->calling_convention( _parm_regs, argcnt );
#ifdef ASSERT
  // Sanity check users' calling convention.  Real handy while trying to 
  // get the initial port correct.
  { for (uint i = 0; i<argcnt; i++) {
      if( _parm_regs[i].lo() == OptoReg::Bad && _parm_regs[i].hi() == OptoReg::Bad ) {
        assert(domain->field_at(i+TypeFunc::Parms)==Type::HALF, "only allowed on halve" );
        continue;
      }
      int parm_reg = _parm_regs[i].lo();
      assert(parm_reg >= SharedInfo::stack0 ||
             can_be_arg(parm_reg) ||
              C->stub_function() == CAST_FROM_FN_PTR(address, OptoRuntime::rethrow_C) ||
             parm_reg == inline_cache_reg() ||
             parm_reg == interpreter_arg_ptr_reg(),
             "parameters in register must be preserved by runtime stubs");
      for (uint j = 0; j < i; j++) {
        assert(parm_reg != (int)(_parm_regs[j].lo()), 
               "calling conv. must produce distinct regs");
      }
    }
  }
#endif

  // Do some initial frame layout.

  // Compute the old incoming SP (may be called FP) as
  //   SharedInfo::stack0 + locks + in_preserve_stack_slots + pad2.
  _old_SP = C->compute_old_SP();
  assert( is_even(_old_SP), "must be even" );

  // Compute highest incoming stack argument as
  //   _old_SP + out_preserve_stack_slots + incoming argument size.
  _in_arg_limit = OptoReg::add(_old_SP, C->out_preserve_stack_slots());
  assert( is_even(_in_arg_limit), "out_preserve must be even" );
  for( i = 0; i < argcnt; i++ ) {
    // Permit args to have no register
    _calling_convention_mask[i].Clear();
    if( _parm_regs[i].lo() == OptoReg::Bad && _parm_regs[i].hi() == OptoReg::Bad )
      continue;
    // calling_convention returns stack arguments as a count of
    // slots beyond SharedInfo::stack0.  We need to convert this to
    // the allocators point of view, taking into account all the
    // preserve area, locks & pad2.
    OptoReg::Name reg1 = _parm_regs[i].lo();
    OptoReg::Name reg2 = _parm_regs[i].hi();

    reg1 = adjust_incoming_stk_arg(reg1);
    if( reg1 != OptoReg::Bad )
      _calling_convention_mask[i].Insert(reg1);

    reg2 = adjust_incoming_stk_arg(reg2);
    if( reg2 != OptoReg::Bad )
      _calling_convention_mask[i].Insert(reg2);

    _parm_regs[i].set_pair(reg2,reg1); // Saved biased stack-slot register number
  }

  // Finally, make sure the incoming arguments take up an even number of
  // words, in case the arguments or locals need to contain doubleword stack 
  // slots.  The rest of the system assumes that stack slot pairs (in 
  // particular, in the spill area) which look aligned will in fact be 
  // aligned relative to the stack pointer in the target machine.  Double 
  // stack slots will always be allocated aligned.
  _new_SP = OptoReg::Name(round_to(_in_arg_limit, RegMask::SlotsPerLong));

  // Compute highest outgoing stack argument as
  //   _new_SP + out_preserve_stack_slots + max(outgoing argument size).
  _out_arg_limit = OptoReg::add(_new_SP, C->out_preserve_stack_slots());
  assert( is_even(_out_arg_limit), "out_preserve must be even" );

  if (C->failing())  return;  // bailed out on incoming arg failure

  // ---------------
  // Collect roots of matcher trees.  Every node for which
  // _shared[_idx] is cleared is guaranteed to not be shared, and thus
  // can be a valid interior of some tree.
  find_shared( C->root() );
  find_shared( C->top() );

  // Swap out to old-space; emptying new-space 
  Arena *old = C->node_arena()->move_contents(C->old_arena());

  // Pre-size the new_node table to avoid the need for range checks.
  grow_new_node_array(C->unique());

  // Reset node counter so MachNodes start with _idx at 0
  int nodes = C->unique(); // save value
  C->set_unique(0);

  // Recursively match trees from old space into new space.
  // Correct leaves of new-space Nodes; they point to old-space.
  _visited.Clear();             // Clear visit bits for xform call
  C->set_cached_top_node(xform( C->top(), nodes ));
  if (!C->failing()) {
    Node* xroot =        xform( C->root(), 1 );
    if (xroot == NULL) {
      Matcher::soft_match_failure();  // recursive matching process failed
      C->record_method_not_compilable("instruction match failed");
    } else {
      C->set_root(xroot->is_Root());
    }
  }
  if (C->top() == NULL || C->root() == NULL) {
    C->record_method_not_compilable("graph lost"); // %%% cannot happen?
  }
  if (C->failing()) {
    // delete old;
    old->destruct_contents();
    return;
  }
  assert( C->top(), "" );
  assert( C->root(), "" );
  validate_null_checks();
  // Now smoke old-space
  // delete old;
  old->destruct_contents();

  // ------------------------
  // Set up save-on-entry registers
  Fixup_Save_On_Entry( );
}


//------------------------------Fixup_Save_On_Entry----------------------------
// The stated purpose of this routine is to take care of save-on-entry 
// registers.  However, the overall goal of the Match phase is to convert into
// machine-specific instructions which have RegMasks to guide allocation.
// So what this procedure really does is put a valid RegMask on each input
// to the machine-specific variations of all Return, TailCall and Halt 
// instructions.  It also adds edgs to define the save-on-entry values (and of
// course gives them a mask).

static RegMask *init_input_masks( uint size, RegMask &ret_adr, RegMask &fp ) {
  RegMask *rms = NEW_RESOURCE_ARRAY( RegMask, size );
  // Do all the pre-defined register masks
  rms[TypeFunc::Control  ] = RegMask::Empty;
  rms[TypeFunc::I_O      ] = RegMask::Empty;
  rms[TypeFunc::Memory   ] = RegMask::Empty;
  rms[TypeFunc::ReturnAdr] = ret_adr;
  rms[TypeFunc::FramePtr ] = fp;
  return rms;
}

//---------------------------init_first_stack_mask-----------------------------
// Create the initial stack mask used by values spilling to the stack.
// Disallow any debug info in outgoing argument areas by setting the
// initial mask accordingly.
void Matcher::init_first_stack_mask() {

  // Allocate storage for spill masks as masks for the appropriate load type.
  RegMask *rms = (RegMask*)C->comp_arena()->Amalloc_4(sizeof(RegMask)*10);
  idealreg2spillmask[Op_RegI] = &rms[0];
  idealreg2spillmask[Op_RegL] = &rms[1];
  idealreg2spillmask[Op_RegF] = &rms[2];
  idealreg2spillmask[Op_RegD] = &rms[3];
  idealreg2spillmask[Op_RegP] = &rms[4];
  idealreg2debugmask[Op_RegI] = &rms[5];
  idealreg2debugmask[Op_RegL] = &rms[6];
  idealreg2debugmask[Op_RegF] = &rms[7];
  idealreg2debugmask[Op_RegD] = &rms[8];
  idealreg2debugmask[Op_RegP] = &rms[9];

  OptoReg::Name i;

  // At first, start with the empty mask
  C->FIRST_STACK_mask().Clear();

  // Add in the incoming argument area
  OptoReg::Name init = OptoReg::add(_old_SP, C->out_preserve_stack_slots());
  for (i = init; i < _in_arg_limit; i = OptoReg::add(i,1))
    C->FIRST_STACK_mask().Insert(i);

  // Add in all bits past the outgoing argument area
  guarantee(RegMask::can_represent(OptoReg::add(_out_arg_limit,-1)),
            "must be able to represent all call arguments in reg mask");
  init = _out_arg_limit;
  for (i = init; RegMask::can_represent(i); i = OptoReg::add(i,1))
    C->FIRST_STACK_mask().Insert(i);

  // Finally, set the "infinite stack" bit.
  C->FIRST_STACK_mask().set_AllStack();

  // Make spill masks.  Registers for their class, plus FIRST_STACK_mask.
  *idealreg2spillmask[Op_RegI] = *idealreg2regmask[Op_RegI]; 
   idealreg2spillmask[Op_RegI]->OR(C->FIRST_STACK_mask());
  *idealreg2spillmask[Op_RegL] = *idealreg2regmask[Op_RegL]; 
   idealreg2spillmask[Op_RegL]->OR(C->FIRST_STACK_mask());
  *idealreg2spillmask[Op_RegF] = *idealreg2regmask[Op_RegF]; 
   idealreg2spillmask[Op_RegF]->OR(C->FIRST_STACK_mask());
  *idealreg2spillmask[Op_RegD] = *idealreg2regmask[Op_RegD]; 
   idealreg2spillmask[Op_RegD]->OR(C->FIRST_STACK_mask());
  *idealreg2spillmask[Op_RegP] = *idealreg2regmask[Op_RegP]; 
   idealreg2spillmask[Op_RegP]->OR(C->FIRST_STACK_mask());

  // Make up debug masks.  Any spill slot plus callee-save registers.
  // Caller-save registers are assumed to be trashable by the various
  // inline-cache fixup routines.
  *idealreg2debugmask[Op_RegI]= *idealreg2spillmask[Op_RegI];
  *idealreg2debugmask[Op_RegL]= *idealreg2spillmask[Op_RegL];
  *idealreg2debugmask[Op_RegF]= *idealreg2spillmask[Op_RegF];
  *idealreg2debugmask[Op_RegD]= *idealreg2spillmask[Op_RegD];
  *idealreg2debugmask[Op_RegP]= *idealreg2spillmask[Op_RegP];

  for( i=OptoReg::Name(0); i<OptoReg::Name(_last_Mach_Reg); i = OptoReg::add(i,1) ) {
    // registers the caller has to save do not work 
    if( _register_save_policy[i] == 'C' ||  
        _register_save_policy[i] == 'A' ) { 
      idealreg2debugmask[Op_RegI]->Remove(i); // Exclude save-on-call 
      idealreg2debugmask[Op_RegL]->Remove(i); // registers from debug
      idealreg2debugmask[Op_RegF]->Remove(i); // masks
      idealreg2debugmask[Op_RegD]->Remove(i);
      idealreg2debugmask[Op_RegP]->Remove(i); 
    }
  }
}

//---------------------------is_save_on_entry----------------------------------
bool Matcher::is_save_on_entry( int reg ) {
  return 
    _register_save_policy[reg] == 'E' ||
    _register_save_policy[reg] == 'A' || // Save-on-entry register?
    // Also save argument registers in the trampolining stubs
    (C->save_argument_registers() && is_spillable_arg(reg));
}

//---------------------------Fixup_Save_On_Entry-------------------------------
void Matcher::Fixup_Save_On_Entry( ) {
  init_first_stack_mask();

  Node *root = C->root();       // Short name for root  
  // Count number of save-on-entry registers.  
  uint soe_cnt = number_of_saved_registers();
  uint i;

  // Find the procedure Start Node
  StartNode *start = C->start();
  assert( start, "Expect a start node" );
  
  // Save argument registers in the trampolining stubs
  if( C->save_argument_registers() ) 
    for( i = 0; i < _last_Mach_Reg; i++ ) 
      if( is_spillable_arg(i) ) 
        soe_cnt++;

  // Input RegMask array shared by all Returns.
  // The type for doubles and longs has a count of 2, but
  // there is only 1 returned value
  uint ret_edge_cnt = TypeFunc::Parms + ((C->tf()->range()->cnt() == TypeFunc::Parms) ? 0 : 1);
  RegMask *ret_rms  = init_input_masks( ret_edge_cnt + soe_cnt, _return_addr_mask, c_frame_ptr_mask );
  // Returns have 0 or 1 returned values depending on call signature. 
  // Return register is specified by return_value in the AD file.  
  if (ret_edge_cnt > TypeFunc::Parms)
    ret_rms[TypeFunc::Parms+0] = _return_value_mask;
  
  // Input RegMask array shared by all Rethrows.
  uint reth_edge_cnt = TypeFunc::Parms+1;
  RegMask *reth_rms  = init_input_masks( reth_edge_cnt + soe_cnt, _return_addr_mask, c_frame_ptr_mask );
  // Rethrow takes exception oop only, but in the argument 0 slot.
  reth_rms[TypeFunc::Parms] = mreg2regmask[find_receiver(false)];
#ifdef _LP64
  // Need two slots for ptrs in 64-bit land
  reth_rms[TypeFunc::Parms].Insert(OptoReg::add(OptoReg::Name(find_receiver(false)),1));
#endif
  
  // Input RegMask array shared by all TailCalls
  uint tail_call_edge_cnt = TypeFunc::Parms+2;
  RegMask *tail_call_rms = init_input_masks( tail_call_edge_cnt + soe_cnt, _return_addr_mask, c_frame_ptr_mask );

  // Input RegMask array shared by all TailJumps
  uint tail_jump_edge_cnt = TypeFunc::Parms+2;
  RegMask *tail_jump_rms = init_input_masks( tail_jump_edge_cnt + soe_cnt, _return_addr_mask, c_frame_ptr_mask );

  // TailCalls have 2 returned values (target & moop), whose masks come
  // from the usual MachNode/MachOper mechanism.  Find a sample
  // TailCall to extract these masks and put the correct masks into
  // the tail_call_rms array.
  for( i=1; i < root->req(); i++ ) {
    MachReturnNode *m = root->in(i)->is_Mach()->is_MachReturn();
    if( m->ideal_Opcode() == Op_TailCall ) {
      tail_call_rms[TypeFunc::Parms+0] = m->MachNode::in_RegMask(TypeFunc::Parms+0);
      tail_call_rms[TypeFunc::Parms+1] = m->MachNode::in_RegMask(TypeFunc::Parms+1);
      break;
    }
  }

  // TailJumps have 2 returned values (target & ex_oop), whose masks come
  // from the usual MachNode/MachOper mechanism.  Find a sample
  // TailJump to extract these masks and put the correct masks into
  // the tail_jump_rms array.
  for( i=1; i < root->req(); i++ ) {
    MachReturnNode *m = root->in(i)->is_Mach()->is_MachReturn();
    if( m->ideal_Opcode() == Op_TailJump ) {
      tail_jump_rms[TypeFunc::Parms+0] = m->MachNode::in_RegMask(TypeFunc::Parms+0);
      tail_jump_rms[TypeFunc::Parms+1] = m->MachNode::in_RegMask(TypeFunc::Parms+1);
      break;
    }
  }

  // Input RegMask array shared by all Halts
  uint halt_edge_cnt = TypeFunc::Parms;
  RegMask *halt_rms = init_input_masks( halt_edge_cnt + soe_cnt, _return_addr_mask, c_frame_ptr_mask );

  // Capture the return input masks into each exit flavor
  for( i=1; i < root->req(); i++ ) {
    MachReturnNode *exit = root->in(i)->is_Mach()->is_MachReturn();
    switch( exit->ideal_Opcode() ) {
      case Op_Return   : exit->_in_rms = ret_rms;  break;
      case Op_Rethrow  : exit->_in_rms = reth_rms; break;
      case Op_TailCall : exit->_in_rms = tail_call_rms; break;
      case Op_TailJump : exit->_in_rms = tail_jump_rms; break;
      case Op_Halt     : exit->_in_rms = halt_rms; break;
      default          : ShouldNotReachHere();
    }
  }

  // Next unused projection number from Start.
  int proj_cnt = C->tf()->domain()->cnt();  
  
  // Do all the save-on-entry registers.  Make projections from Start for 
  // them, and give them a use at the exit points.  To the allocator, they 
  // look like incoming register arguments.
  for( i = 0; i < _last_Mach_Reg; i++ ) {
    if( is_save_on_entry(i) ) {

      // Add the save-on-entry to the mask array
      ret_rms      [      ret_edge_cnt] = mreg2regmask[i];
      reth_rms     [     reth_edge_cnt] = mreg2regmask[i];
      tail_call_rms[tail_call_edge_cnt] = mreg2regmask[i];
      tail_jump_rms[tail_jump_edge_cnt] = mreg2regmask[i];
      // Halts need the SOE registers, but only in the stack as debug info.
      // A just-prior uncommon-trap or deoptimization will use the SOE regs.
      halt_rms     [     halt_edge_cnt] = *idealreg2spillmask[_register_save_type[i]];

      Node *mproj;

      // Is this a RegF low half of a RegD?  Double up 2 adjacent RegF's
      // into a single RegD.
      if( (i&1) == 0 &&
          _register_save_type[i  ] == Op_RegF &&
          _register_save_type[i+1] == Op_RegF &&
          is_save_on_entry(i+1) ) {
        // Add other bit for double
        ret_rms      [      ret_edge_cnt].Insert(OptoReg::Name(i+1));
        reth_rms     [     reth_edge_cnt].Insert(OptoReg::Name(i+1));
        tail_call_rms[tail_call_edge_cnt].Insert(OptoReg::Name(i+1));
        tail_jump_rms[tail_jump_edge_cnt].Insert(OptoReg::Name(i+1));
        halt_rms     [     halt_edge_cnt].Insert(OptoReg::Name(i+1));
        mproj = new (1) MachProjNode( start, proj_cnt, ret_rms[ret_edge_cnt], Op_RegD );
        proj_cnt += 2;          // Skip 2 for doubles
      } 
      else if( (i&1) == 1 &&    // Else check for high half of double
               _register_save_type[i-1] == Op_RegF &&
               _register_save_type[i  ] == Op_RegF &&
               is_save_on_entry(i-1) ) {
        ret_rms      [      ret_edge_cnt] = RegMask::Empty;
        reth_rms     [     reth_edge_cnt] = RegMask::Empty;
        tail_call_rms[tail_call_edge_cnt] = RegMask::Empty;
        tail_jump_rms[tail_jump_edge_cnt] = RegMask::Empty;
        halt_rms     [     halt_edge_cnt] = RegMask::Empty;
        mproj = C->top();
      } 
      // Is this a RegI low half of a RegL?  Double up 2 adjacent RegI's
      // into a single RegL.
      else if( (i&1) == 0 &&
          _register_save_type[i  ] == Op_RegI &&
          _register_save_type[i+1] == Op_RegI &&
        is_save_on_entry(i+1) ) {
        // Add other bit for long
        ret_rms      [      ret_edge_cnt].Insert(OptoReg::Name(i+1));
        reth_rms     [     reth_edge_cnt].Insert(OptoReg::Name(i+1));
        tail_call_rms[tail_call_edge_cnt].Insert(OptoReg::Name(i+1));
        tail_jump_rms[tail_jump_edge_cnt].Insert(OptoReg::Name(i+1));
        halt_rms     [     halt_edge_cnt].Insert(OptoReg::Name(i+1));
        mproj = new (1) MachProjNode( start, proj_cnt, ret_rms[ret_edge_cnt], Op_RegL );
        proj_cnt += 2;          // Skip 2 for longs
      } 
      else if( (i&1) == 1 &&    // Else check for high half of long
               _register_save_type[i-1] == Op_RegI &&
               _register_save_type[i  ] == Op_RegI &&
               is_save_on_entry(i-1) ) {
        ret_rms      [      ret_edge_cnt] = RegMask::Empty;
        reth_rms     [     reth_edge_cnt] = RegMask::Empty;
        tail_call_rms[tail_call_edge_cnt] = RegMask::Empty;
        tail_jump_rms[tail_jump_edge_cnt] = RegMask::Empty;
        halt_rms     [     halt_edge_cnt] = RegMask::Empty;
        mproj = C->top();
      } else {
        // Make a projection for it off the Start
        mproj = new (1) MachProjNode( start, proj_cnt++, ret_rms[ret_edge_cnt], _register_save_type[i] );
      }

      ret_edge_cnt ++;
      reth_edge_cnt ++;
      tail_call_edge_cnt ++;
      tail_jump_edge_cnt ++;
      halt_edge_cnt ++;

      // Add a use of the SOE register to all exit paths
      for( uint j=1; j < root->req(); j++ ) 
        root->in(j)->add_req(mproj); 
    } // End of if a save-on-entry register
  } // End of for all machine registers
}

//------------------------------init_spill_mask--------------------------------
void Matcher::init_spill_mask( Node *ret ) {
  if( idealreg2regmask[Op_RegI] ) return; // One time only init

  // Stack registers start at next register past last machine register
  SharedInfo::stack0 = OptoReg::Name(round_to(_last_Mach_Reg, RegMask::SlotsPerLong));

  SharedInfo::c_frame_pointer = c_frame_pointer();
  c_frame_ptr_mask = c_frame_pointer();
#ifdef _LP64
  // pointers are twice as big
  c_frame_ptr_mask.Insert(OptoReg::add(c_frame_pointer(),1));
#endif

  // Start at SharedInfo::stack0
  STACK_ONLY_mask.Clear();
  OptoReg::Name init = OptoReg::Name(SharedInfo::stack0);
  // STACK_ONLY_mask is all stack bits
  OptoReg::Name i;
  for (i = init; RegMask::can_represent(i); i = OptoReg::add(i,1))
    STACK_ONLY_mask.Insert(i);
  // Also set the "infinite stack" bit.
  STACK_ONLY_mask.set_AllStack();

  // Copy the register names over into the shared world
  for( i=OptoReg::Name(0); i<OptoReg::Name(_last_Mach_Reg); i = OptoReg::add(i,1) ) {
    SharedInfo::regName[i] = regName[i];
    // Handy RegMasks per machine register
    mreg2regmask[i].Insert(i);
  }

  // Grab the Frame Pointer
  Node *fp  = ret->in(TypeFunc::FramePtr);
  Node *mem = ret->in(TypeFunc::Memory);
  const TypePtr* atp = TypePtr::BOTTOM;
  // Share frame pointer while making spill ops
  set_shared(fp);

  // Compute generic short-offset Loads
  MachNode *spillI  = match_tree(new (3) LoadINode(NULL,mem,fp,atp));
  MachNode *spillL  = match_tree(new (3) LoadLNode(NULL,mem,fp,atp));
  MachNode *spillF  = match_tree(new (3) LoadFNode(NULL,mem,fp,atp));
  MachNode *spillD  = match_tree(new (3) LoadDNode(NULL,mem,fp,atp));
  MachNode *spillP  = match_tree(new (3) LoadPNode(NULL,mem,fp,atp,TypeInstPtr::BOTTOM));

  // Get the ADLC notion of the right regmask, for each basic type.
  idealreg2regmask[Op_RegI] = &spillI->out_RegMask();
  idealreg2regmask[Op_RegL] = &spillL->out_RegMask();
  idealreg2regmask[Op_RegF] = &spillF->out_RegMask();
  idealreg2regmask[Op_RegD] = &spillD->out_RegMask();
  idealreg2regmask[Op_RegP] = &spillP->out_RegMask();
}

#ifdef ASSERT
static void match_alias_type(Compile* C, Node* n, Node* m) {
  if (!VerifyAliases)  return;  // do not go looking for trouble by default
  const TypePtr* nat = n->adr_type();
  const TypePtr* mat = m->adr_type();
  int nidx = C->get_alias_index(nat);
  int midx = C->get_alias_index(mat);
  // Detune the assert for cases like (AndI 0xFF (LoadB p)).
  if (nidx == Compile::AliasIdxTop && midx >= Compile::AliasIdxRaw) {
    for (uint i = 1; i < n->req(); i++) {
      Node* n1 = n->in(i);
      const TypePtr* n1at = n1->adr_type();
      if (n1at != NULL) {
        nat = n1at;
        nidx = C->get_alias_index(n1at);
      }
    }
  }
  // %%% Kludgery.  Instead, fix ideal adr_type methods for all these cases:
  if (nidx == Compile::AliasIdxTop && midx == Compile::AliasIdxRaw) {
    switch (n->Opcode()) {
    case Op_Prefetch:
      nidx = Compile::AliasIdxRaw;
      nat = TypeRawPtr::BOTTOM;
      break;
    }
  }
  if (nidx == Compile::AliasIdxRaw && midx == Compile::AliasIdxTop) {
    switch (n->Opcode()) {
    case Op_ClearArray:
      midx = Compile::AliasIdxRaw;
      mat = TypeRawPtr::BOTTOM;
      break;
    }
  }
  if (nidx == Compile::AliasIdxTop && midx == Compile::AliasIdxBot) {
    switch (n->Opcode()) {
    case Op_Return:
    case Op_Rethrow:
    case Op_Halt:
    case Op_TailCall:
    case Op_TailJump:
      nidx = Compile::AliasIdxBot;
      nat = TypePtr::BOTTOM;
      break;
    }
  }
  if (nidx == Compile::AliasIdxBot && midx == Compile::AliasIdxTop) {
    switch (n->Opcode()) {
    case Op_StrComp:
    case Op_MemBarVolatile:
    case Op_MemBarCPUOrder: // %%% these ideals should have narrower adr_type?
      nidx = Compile::AliasIdxTop;
      nat = NULL;
      break;
    }
  }
  if (nidx != midx) {
    if (PrintOpto || (PrintMiscellaneous && (WizardMode || Verbose))) {
      tty->print_cr("==== Matcher alias shift %d => %d", nidx, midx);
      n->dump();
      m->dump();
    }
    assert(C->subsume_loads() && C->must_alias(nat, midx),
           "must not lose alias info when matching");
  }
}
#endif


//------------------------------MStack-----------------------------------------
// State and MStack class used in xform() and find_shared() iterative methods.
enum Node_State { Pre_Visit,  // node has to be pre-visited
                      Visit,  // visit node
                 Post_Visit,  // post-visit node
             Alt_Post_Visit   // alternative post-visit path
                };

class MStack: public Node_Stack {
  public:
    MStack(int size) : Node_Stack(size) { }

    void push(Node *n, Node_State ns) {
      Node_Stack::push(n, (uint)ns);
    }
    void push(Node *n, Node_State ns, Node *parent, int indx) {
      ++_inode_top;
      if ((_inode_top + 1) >= _inode_max) grow();
      _inode_top->node = parent;
      _inode_top->indx = (uint)indx;
      ++_inode_top;
      _inode_top->node = n;
      _inode_top->indx = (uint)ns;
    }
    Node *parent() {
      pop();
      return node();
    }
    Node_State state() const {
      return (Node_State)index();
    }
    void set_state(Node_State ns) {
      set_index((uint)ns);
    }
};


//------------------------------xform------------------------------------------
// Given a Node in old-space, Match him (Label/Reduce) to produce a machine
// Node in new-space.  Given a new-space Node, recursively walk his children.
Node *Matcher::transform( Node *n ) { ShouldNotCallThis(); return n; }
Node *Matcher::xform( Node *n, int max_stack ) {
  // Use one stack to keep both: child's node/state and parent's node/index
  MStack mstack(max_stack * 2 * 2); // C->unique() * 2 * 2
  mstack.push(n, Visit, NULL, -1);  // set NULL as parent to indicate root

  while (mstack.is_nonempty()) {
    n = mstack.node();          // Leave node on stack
    Node_State nstate = mstack.state();
    if (nstate == Visit) {
      mstack.set_state(Post_Visit);
      Node *oldn = n;
      // Old-space or new-space check
      if (!C->node_arena()->contains(n)) {
        // Old space!
        Node* m;
        if (has_new_node(n)) {  // Not yet Label/Reduced
          m = new_node(n);
        } else {
          if (!is_dontcare(n)) { // Matcher can match this guy
            // Calls match special.  They match alone with no children.
            // Their children, the incoming arguments, match normally.
            SafePointNode *sfpt = n->is_SafePoint();
            m = sfpt ? match_sfpt(sfpt) : match_tree(n);
            if (m == NULL) { Matcher::soft_match_failure(); return NULL; }
          } else {                  // Nothing the matcher cares about
            ProjNode *proj = n->is_Proj();
            if( proj && proj->in(0)->is_Multi()) {       // Projections?
              // Convert to machine-dependent projection
              m = proj->in(0)->is_Multi()->match( proj, this );
              if (m->in(0) != NULL) // m might be top
                collect_null_checks(m);
            } else {                // Else just a regular 'ol guy
              m = n->clone();       // So just clone into new-space
              // Def-Use edges will be added incrementally as Uses
              // of this node are matched.
              assert(m->outcnt() == 0, "no Uses of this clone yet");
            }
          }

          set_new_node(n, m);       // Map old to new
          debug_only(match_alias_type(C, n, m));
        }
        n = m;    // n is now a new-space node
        mstack.set_node(n);
      }

      // New space!
      if (_visited.test_set(n->_idx)) continue; // while(mstack.is_nonempty())

      int i;
      // Put precedence edges on stack first (match them last).
      for (i = oldn->req(); (uint)i < oldn->len(); i++) {
        Node *m = oldn->in(i);
        if (m == NULL) break;
        // set -1 to call add_prec() instead of set_req() during Step1
        mstack.push(m, Visit, n, -1);
      }

      // For constant debug info, I'd rather have unmatched constants.
      int cnt = n->req();
      JVMState* jvms = n->jvms();
      int debug_cnt = jvms ? jvms->debug_start() : cnt;

      // Now do only debug info.  Clone constants rather than matching.
      // Constants are represented directly in the debug info without
      // the need for executable machine instructions.
      // Monitor boxes are also represented directly.
      for (i = cnt - 1; i >= debug_cnt; --i) { // For all debug inputs do
        Node *m = n->in(i);          // Get input
        int op = m->Opcode();
        assert((op == Op_BoxLock) == jvms->is_monitor_use(i), "boxes only at monitor sites");
        if( op == Op_ConI || op == Op_ConP ||
            op == Op_ConF || op == Op_ConD || op == Op_ConL
            // || op == Op_BoxLock  // %%%% enable this and remove (+++) in chaitin.cpp
            ) {
          m = m->clone();
          mstack.push(m, Post_Visit, n, i); // Don't neet to visit
          mstack.push(m->in(0), Visit, m, 0);
        } else {
          mstack.push(m, Visit, n, i);
        }
      }

      // And now walk his children, and convert his inputs to new-space.
      for( ; i >= 0; --i ) { // For all normal inputs do
        Node *m = n->in(i);  // Get input
        if(m != NULL) 
          mstack.push(m, Visit, n, i);
      }

    }
    else if (nstate == Post_Visit) {
      // Set xformed input
      Node *p = mstack.parent();
      if (p != NULL) { // root doesn't have parent
        int i = (int)mstack.index();
        if (i >= 0)
          p->set_req(i, n); // required input
        else if (i == -1)
          p->add_prec(n);   // precedence input
        else
          ShouldNotReachHere();
      }
      mstack.pop(); // remove processed node from stack
    }
    else {
      ShouldNotReachHere();
    }
  } // while (mstack.is_nonempty())
  return n; // Return new-space Node
}

//------------------------------adjust_outgoing_stk_arg------------------------
OptoReg::Name Matcher::adjust_outgoing_stk_arg( int reg, OptoReg::Name begin_out_arg_area, OptoReg::Name &out_arg_limit_per_call ) {
  OptoReg::Name out_reg = OptoReg::Name(reg);
  if (out_reg == OptoReg::Bad)
    return out_reg;
  // Convert outgoing argument location to a pre-biased stack offset
  int stk_slot_num = reg - SharedInfo::stack0;
  if( stk_slot_num >= 0 ) { // Check for being in stack or register
    // Adjust the stack slot offset to be the register number used
    // by the allocator.
    out_reg = OptoReg::add(begin_out_arg_area,stk_slot_num);
    // Keep track of the largest numbered stack slot used for an arg.
    // Largest used slot per call-site indicates the amount of stack
    // that is killed by the call.
    if( out_reg >= out_arg_limit_per_call ) 
      out_arg_limit_per_call = OptoReg::add(out_reg,1);
    if (!RegMask::can_represent(out_reg)) {
      C->record_method_not_compilable_all_tiers("unsupported calling sequence");
      return OptoReg::Bad;
    }
  }
  assert( out_reg >= 0, "broken ad file register decls" );
  return out_reg;
}


//------------------------------match_sfpt-------------------------------------
// Helper function to match call instructions.  Calls match special.
// They match alone with no children.  Their children, the incoming
// arguments, match normally.
MachNode *Matcher::match_sfpt( SafePointNode *sfpt ) {
  MachSafePointNode *msfpt = NULL;
  MachCallNode      *mcall = NULL;
  uint               cnt;
  // Split out case for SafePoint vs Call
  CallNode *call = sfpt->is_Call();
  const TypeTuple *domain = (call != NULL) ? call->tf()->domain() : NULL;
  ciMethod*        method = NULL;
  if( call != NULL ) {
    cnt = domain->cnt();

    // Match just the call, nothing else
    MachNode *m = match_tree(call);
    if( m == NULL ) { Matcher::soft_match_failure(); return NULL; }

    // Copy data from the Ideal SafePoint to the machine version
    mcall = m->is_MachCall();

    mcall->set_tf(         call->tf());
    mcall->set_entry_point(call->entry_point());
  
    MachCallRuntimeNode     *mcall_runtime = mcall->is_MachCallRuntime();
    MachCallJavaNode        *mcall_java    = mcall->is_MachCallJava();
    if( mcall_java ) {
      const CallJavaNode *call_java = call->is_CallJava();
      method = call_java->method();
      mcall_java->_method = method;
      mcall_java->_bci = call_java->_bci;
      mcall_java->_optimized_virtual = call_java->is_optimized_virtual(); 
      MachCallStaticJavaNode *mcalls_java = mcall_java->is_MachCallStaticJava();
      if( mcalls_java ) 
        mcalls_java->_name = call_java->is_CallStaticJava()->_name;
    } 
    else if( mcall_runtime ) {
      const CallRuntimeNode *call_runtime = call->is_CallRuntime();
      mcall_runtime->_name = call_runtime->_name;
    }
    msfpt = mcall;
  }
  // This is a non-call safepoint
  else {
    MachNode *mn = match_tree(sfpt);
    msfpt = mn->is_MachSafePoint();
    cnt = TypeFunc::Parms;
  }

  // Allocate a private array of RegMasks.  These RegMasks are not shared.
  msfpt->_in_rms = NEW_RESOURCE_ARRAY( RegMask, cnt );
  // Empty them all.
  memset( msfpt->_in_rms, 0, sizeof(RegMask)*cnt );

  // Do all the pre-defined non-Empty register masks
  msfpt->_in_rms[TypeFunc::ReturnAdr] = _return_addr_mask;
  msfpt->_in_rms[TypeFunc::FramePtr ] = c_frame_ptr_mask;

  // Place first outgoing argument can possibly be put.
  OptoReg::Name begin_out_arg_area = OptoReg::add(_new_SP, C->out_preserve_stack_slots());
  assert( is_even(begin_out_arg_area), "" );
  // Compute max outgoing register number per call site.  
  OptoReg::Name out_arg_limit_per_call = begin_out_arg_area;
  // Calls to C may hammer extra stack slots above and beyond any arguments.
  // These are usually backing store for register arguments for varargs.
  if( call && call->is_CallRuntime() && !call->is_CallInterpreter() )
    out_arg_limit_per_call = OptoReg::add(out_arg_limit_per_call,C->varargs_C_out_slots_killed());


  // Do the normal argument list (parameters) register masks
  int argcnt = cnt - TypeFunc::Parms;
  if( argcnt > 0 ) {          // Skip it all if we have no args
    OptoRegPair *parm_regs = NEW_RESOURCE_ARRAY( OptoRegPair, argcnt );
    int i;
    for( i = 0; i < argcnt; i++ ) {
      parm_regs[i].set_ideal_reg(base2reg[domain->field_at(i+TypeFunc::Parms)->base()]);
    }
    // V-call to pick proper calling convention
    call->calling_convention( parm_regs, argcnt );

#ifdef ASSERT
    // Sanity check users' calling convention.  Really handy during 
    // the initial porting effort.  Fairly expensive otherwise.
    { for (int i = 0; i<argcnt; i++) {
      if( parm_regs[i].lo() == OptoReg::Bad &&
          parm_regs[i].hi() == OptoReg::Bad ) continue;
      int reg1 = parm_regs[i].lo();
      int reg2 = parm_regs[i].hi();
      for (int j = 0; j < i; j++) {
        if( parm_regs[j].lo() == OptoReg::Bad &&
            parm_regs[j].hi() == OptoReg::Bad ) continue;
        int reg3 = parm_regs[j].lo();
        int reg4 = parm_regs[j].hi();
        if( reg1 == OptoReg::Bad ) {
          assert( reg2 == OptoReg::Bad, "valid halvsies" );
        } else if( reg3 == OptoReg::Bad ) {
          assert( reg4 == OptoReg::Bad, "valid halvsies" );
        } else {
          assert( reg1 != reg2, "calling conv. must produce distinct regs");
          assert( reg1 != reg3, "calling conv. must produce distinct regs");
          assert( reg1 != reg4, "calling conv. must produce distinct regs");
          assert( reg2 != reg3, "calling conv. must produce distinct regs");
          assert( reg2 != reg4 || reg2 == OptoReg::Bad, "calling conv. must produce distinct regs");
          assert( reg3 != reg4, "calling conv. must produce distinct regs");
        }
      }
    }
    }
#endif

    // Visit each argument.  Compute its outgoing register mask.
    // Return results now can have 2 bits returned.
    // Compute max over all outgoing arguments both per call-site
    // and over the entire method.
    for( i = 0; i < argcnt; i++ ) {
      // Address of incoming argument mask to fill in
      RegMask *rm = &mcall->_in_rms[i+TypeFunc::Parms];
      if( parm_regs[i].lo() == OptoReg::Bad &&
          parm_regs[i].hi() == OptoReg::Bad )
        continue;               // Avoid Halves
      // Grab first register, adjust stack slots and insert in mask.
      OptoReg::Name reg1 = adjust_outgoing_stk_arg(parm_regs[i].lo(), begin_out_arg_area, out_arg_limit_per_call );
      if (reg1 != OptoReg::Bad)
        rm->Insert( reg1 );
      // Grab second register (if any), adjust stack slots and insert in mask.
      OptoReg::Name reg2 = adjust_outgoing_stk_arg(parm_regs[i].hi(), begin_out_arg_area, out_arg_limit_per_call );
      if (reg2 != OptoReg::Bad)
        rm->Insert( reg2 );
      parm_regs[i].set_pair(reg2,reg1);
    } // End of for all arguments

#if defined(_LP64) && !defined(CC_INTERP)
    // Miserable hack: if the first argument in a C2I adapter is a LONG, the
    // first stack 64-bit word goes unused (the long is stuffed in the other
    // 64-bit word) in the adapter.  BUT the interpreter expects to own that
    // space - it becomes his java-local-zero.  If we deopt and unpack an
    // interpreter frame, it will naturally scribble down the unpack info into
    // this topmost stack slot.  If the C2I adapter has not reserved it, we'll
    // end up smashing the 1st 64-bit word of the next frame, which tends to
    // smash the return address.  :-(
    // Note: IA64 C++ interpreter should not have this problem and doesn't
    // need this hack.
    if( call->is_CallInterpreter() && 
        type2size[domain->field_at(TypeFunc::Parms)->basic_type()]==2 )
      out_arg_limit_per_call = OptoReg::add(out_arg_limit_per_call,2);
#endif

    // Compute number of stack slots needed to restore stack in case of
    // Pascal-style argument popping.
    mcall->_argsize = out_arg_limit_per_call - begin_out_arg_area;

    // Make sure a C2I adapter can be created for this call.
    // Consult the ascii-gram in <arch>.ad for the layout of the frame.
    if (method != NULL && !C->can_generate_C2I(method, mcall->_argsize)) {
      C->record_method_not_compilable_all_tiers("unsupported C2I calling sequence");
    }

  }

  // Compute the max stack slot killed by any call.  These will not be 
  // available for debug info, and will be used to adjust FIRST_STACK_mask 
  // after all call sites have been visited.
  if( _out_arg_limit < out_arg_limit_per_call) 
    _out_arg_limit = out_arg_limit_per_call;

  if (mcall) {
    // Kill the outgoing argument area, including any non-argument holes and
    // any legacy C-killed slots.  Use Fat-Projections to do the killing.
    // Since the max-per-method covers the max-per-call-site and debug info
    // is excluded on the max-per-method basis, debug info cannot land in
    // this killed area.
    uint r_cnt = mcall->tf()->range()->cnt();
    MachProjNode *proj = new (1) MachProjNode( mcall, r_cnt+10000, RegMask::Empty, MachProjNode::fat_proj );
    if (!RegMask::can_represent(OptoReg::Name(out_arg_limit_per_call-1))) {
      C->record_method_not_compilable_all_tiers("unsupported outgoing calling sequence");
    } else {
      for (int i = begin_out_arg_area; i < out_arg_limit_per_call; i++)
        proj->_rout.Insert(OptoReg::Name(i));
    }
    if( proj->_rout.is_NotEmpty() )
      _proj_list.push(proj);
  }
  // Transfer the safepoint information from the call to the mcall
  // Move the JVMState list
  msfpt->set_jvms(sfpt->jvms());
  for (JVMState* jvms = msfpt->jvms(); jvms; jvms = jvms->caller()) {
    jvms->set_map(sfpt);
  }

  // Debug inputs begin just after the last incoming parameter
  assert( (mcall == NULL) || (mcall->jvms() == NULL) ||
          (mcall->jvms()->debug_start() + mcall->_jvmadj == mcall->tf()->domain()->cnt()), "" );

  // Move the OopMap
  msfpt->_oop_map = sfpt->_oop_map;

  // Registers killed by the call are set in the local scheduling pass
  // of Global Code Motion.
  return msfpt;
}

//---------------------------match_tree----------------------------------------
// Match a Ideal Node DAG - turn it into a tree; Label & Reduce.  Used as part
// of the whole-sale conversion from Ideal to Mach Nodes.  Also used for
// making GotoNodes while building the CFG and in init_spill_mask() to identify
// a Load's result RegMask for memoization in idealreg2regmask[]
MachNode *Matcher::match_tree( const Node *n ) {
  assert( n->Opcode() != Op_Phi, "cannot match" );
  assert( !n->is_block_start(), "cannot match" );
  // Set the mark for all locally allocated State objects.
  // When this call returns, the _states arena will be reset
  // freeing all State objects.
  ResourceMark rm( &_states );

  // StoreNodes require their Memory input to match any LoadNodes
  Node *mem = n->is_Store() ? n->in(MemNode::Memory) : (Node*)1 ;

  // State object for root node of match tree
  State s;
  s._kids[0] = s._kids[1] = NULL;
  s._leaf = (Node*)n;
  // Label the input tree, allocating labels from top-level arena
  Label_Root( n, &s, n->in(0), mem );
  
  // The minimum cost match for the whole tree is found at the root State
  uint mincost = max_juint;
  uint cost = max_juint;
  uint i;
  for( i = 0; i < NUM_OPERANDS; i++ ) {
    if( s.valid(i) &&                // valid entry and
        s._cost[i] < cost &&         // low cost and 
        s._rule[i] >= NUM_OPERANDS ) // not an operand
      cost = s._cost[mincost=i];
  }
  if (mincost == max_juint) {
#ifndef PRODUCT
    tty->print("No matching rule for:");
    s.dump();
#endif
    Matcher::soft_match_failure();
    return NULL;
  }
  // Reduce input tree based upon the state labels to machine Nodes
  MachNode *m = ReduceInst( &s, s._rule[mincost], mem );
#ifdef ASSERT
  _old2new_map.map(n->_idx, m);
#endif
  
  // Add any Matcher-ignored edges
  uint cnt = n->req();
  uint start = 1;
  if( mem != (Node*)1 ) start = MemNode::Memory+1;
  if( n->Opcode() == Op_AddP ) {
    assert( mem == (Node*)1, "" );
    start = AddPNode::Base+1;
  }
  for( i = start; i < cnt; i++ ) {
    if( !n->match_edge(i) ) {
      if( i < m->req() )
        m->ins_req( i, n->in(i) );
      else
        m->add_req( n->in(i) ); 
    }
  }

  return m;
}

//------------------------------match_into_reg---------------------------------
// Choose to either match this Node in a register or part of the current
// match tree.  Return true for requiring a register and false for matching
// as part of the current match tree.
static bool match_into_reg( Node *m, int used_by_store, int i, bool shared, Node *control ) {
  
  const Type *t = m->bottom_type();

  // 'm' is a constant?  Probably match it as part of this tree.  Most
  // small constants are swallowed into most machine instructions, e.g.
  // add+1.
  if( t->singleton() ) {
    
    // Nearly always clone integer constants: e.g., add+1 or shift-by-2.
    // Cloning the constant means we can match it directly into the machine
    // instruction instead of loading the constant into a register and use a
    // reg/reg operation.

    // Clone small long constants.  This is kinda Sparc-specific but only
    // kinda.  Intel wants the long constants to be +/-128 to use it's short
    // form but we currently lack encodings for this.  All interesting long
    // constants are short: div/mod 10 and +/-48 for doing long-to-string
    // conversions.  If I shrink this +/-4K to +/-128 I'll lose an
    // uninteresting number of cases on Sparc and gain nothing on Intel.
    const TypeLong *tl = t->isa_long();
    if( tl && 
        (tl->get_con() >= Matcher::short_spill_offset_limit || 
         tl->get_con() < -Matcher::short_spill_offset_limit) ) {
      // Big long constant: put in register
    } else {                    // Not a big long constant

      // Pointer constants in addressing modes, e.g., referencing
      // global statics, should be shared because it's expensive to
      // make an oop.  Share on Sparc, where registers are plentiful
      // and suck it into the addressing mode on Intel.
      if( !Matcher::clone_shift_expressions && 
          t->isa_ptr() && t != TypePtr::NULL_PTR ) return shared;

      // Another exception is storing a constant.  Sparc can't do it in one
      // instruction.  Intel can, but repeated stores get big.
     if( !used_by_store || i != MemNode::ValueIn )
        return false;           // Not storing a constant, so clone
        
     // Let little constants fold into the store.
     // Helps Intel cardmark idioms.
     if (used_by_store == Op_StoreB || used_by_store == Op_StoreCM)
       return false;           // Storing a card mark, probably.
     
      // Storing a constant.  For Sparc, storing a zero is cheap (G0), not so on
      // Intel (repeated stores of a zero are cheaper to share the zero in a
      // register than use repeated long encodings).  For Sparc, storing a
      // non-zero needs an extra temp reg, not so on Intel.
      if( t == TypeInt::ZERO || // Fancy zero check
          t == TypeLong::ZERO ||
          t == TypePtr::NULL_PTR ||
          t == TypeF::ZERO ||
          t == TypeD::ZERO ) {
        if( !Matcher::clone_shift_expressions )
          return false;         // Zero store on sparc so clone
      } else {                  // Non-zero
        if( Matcher::clone_shift_expressions )
          return false;         // Non-zero store on Intel, so clone
      }
    }      

  } else {                      // Not a constant
    // Stop recursion if they have different Controls.
    // Slot 0 of constants is not really a Control.
    if( control && m->in(0) && control != m->in(0) ) {

      // Actually, we can live with the most conservative control we
      // find, if it post-dominates the others.  This allows us to 
      // pick up load/op/store trees where the load can float a little
      // above the store.
      Node *x = control;
      const int max_scan = 6;   // Arbitrary scan cutoff
      uint j;
      for( j=0; j<max_scan; j++ ) {
        if( x->is_Region() )    // Bail out at merge points
          return true;
        x = x->in(0);
        if( x == m->in(0) )     // Does 'control' post-dominate
          break;                // m->in(0)?  If so, we can use it
      }
      if( j == max_scan )       // No post-domination before scan end?
        return true;            // Then break the match tree up
    }
  }

  // Not forceably cloning.  If shared, put it into a register.
  return shared;
}


//------------------------------Instruction Selection--------------------------
// Label method walks a "tree" of nodes, using the ADLC generated DFA to match
// ideal nodes to machine instructions.  Trees are delimited by shared Nodes,
// things the Matcher does not match (e.g., Memory), and things with different
// Controls (hence forced into different blocks).  We pass in the Control 
// selected for this entire State tree.

// The Matcher works on Trees, but an Intel add-to-memory requires a DAG: the
// Store and the Load must have identical Memories (as well as identical 
// pointers).  Since the Matcher does not have anything for Memory (and 
// does not handle DAGs), I have to match the Memory input myself.  If the
// Tree root is a Store, I require all Loads to have the identical memory.
Node *Matcher::Label_Root( const Node *n, State *svec,Node *control,Node *mem){
  uint care = 0;                // Edges matcher cares about
  uint cnt = n->req();
  uint i = 0;

  // Examine children for memory state
  // Can only subsume a child into your match-tree if that child's memory state
  // is not modified along the path to another input.  
  // It is unsafe even if the other inputs are separate roots.
  Node *input_mem = NULL;
  for( i = 1; i < cnt; i++ ) { 
    if( !n->match_edge(i) ) continue;
    Node *m = n->in(i);         // Get ith input
    if( m->is_Load() ) {
      Node *mem = m->in(MemNode::Memory);
      if( input_mem == NULL ) {
        input_mem = mem;
      } else if( input_mem != mem ) { 
        input_mem = NodeSentinel;
      }
    }
  }

  // Big stores of big constants get expensive.  Share the big
  // constant, but let little constants fold into the store.
  // Helps Intel cardmark idioms.
  int used_by_store = n->is_Store() ? n->Opcode() : 0;

  for( i = 1; i < cnt; i++ ){// For my children
    if( !n->match_edge(i) ) continue;
    Node *m = n->in(i);         // Get ith input
    assert( m, "expect non-null children" );
    // Allocate states out of a private arena
    State *s = svec->_kids[care++] = new (&_states) State;
    assert( care <= 2, "binary only for now" );

    // Recursively label the State tree.
    s->_kids[0] = s->_kids[1] = NULL;
    s->_leaf = (Node*)m;

    // Check for leaves of the State Tree; things that cannot be a part of 
    // the current tree.  If it finds any, that value is matched as a 
    // register operand.  If not, then the normal matching is used.
    if( match_into_reg(m, used_by_store, i, is_shared(m), control) ||
        // 
        // Stop recursion if this is LoadNode and the root of this tree is a
        // StoreNode and the load & store have different memories.
        ((mem!=(Node*)1) && m->is_Load() && m->in(MemNode::Memory) != mem) ||
        // Can NOT include the match of a subtree when its memory state
        // is used by any of the other subtrees
        (input_mem == NodeSentinel) ) {
      // Print when we exclude matching due to different memory states at input-loads
      if( PrintOpto && (Verbose && WizardMode) && (input_mem == NodeSentinel) 
        && !((mem!=(Node*)1) && m->is_Load() && m->in(MemNode::Memory) != mem) ) { 
        tty->print_cr("invalid input_mem"); 
      }
      // Switch to a register-only opcode; this value must be in a register
      // and cannot be subsumed as part of a larger instruction.
      s->DFA( m->ideal_reg(), m );
    
    } else {
      // If match tree has no control and we do, adopt it for entire tree
      if( !control && m->in(0) && m->req() > 1 )
        control = m->in(0);         // Pick up control
      // Else match as a normal part of the match tree.
      control = Label_Root(m,s,control,mem);
    }
  }

  
  // Call DFA to match this node, and return
  bool success = svec->DFA( n->Opcode(), n );

#ifdef ASSERT
  uint x;
  for( x = 0; x < _LAST_MACH_OPER; x++ )
    if( svec->valid(x) )
      break;

  if (x >= _LAST_MACH_OPER) {
    n->dump();
    svec->dump();
    assert( false, "bad AD file" );
  }
#endif
  return control;
}


//------------------------------ReduceInst-------------------------------------
// Reduce a State tree (with given Control) into a tree of MachNodes.
// This routine (and it's cohort ReduceOper) convert Ideal Nodes into 
// complicated machine Nodes.  Each MachNode covers some tree of Ideal Nodes.
// Each MachNode has a number of complicated MachOper operands; each 
// MachOper also covers a further tree of Ideal Nodes.  

// The root of the Ideal match tree is always an instruction, so we enter
// the recursion here.  After building the MachNode, we need to recurse
// the tree checking for these cases:
// (1) Child is an instruction - 
//     Build the instruction (recursively), add it as an edge.
//     Build a simple operand (register) to hold the result of the instruction.
// (2) Child is an interior part of an instruction -
//     Skip over it (do nothing)
// (3) Child is the start of a operand -
//     Build the operand, place it inside the instruction
//     Call ReduceOper.
MachNode *Matcher::ReduceInst( State *s, int rule, Node *&mem ) {
  assert( rule >= NUM_OPERANDS, "called with operand rule" );

  // Build the object to represent this state & prepare for recursive calls
  MachNode *mach = s->MachNodeGenerator( rule, C );
  Node *leaf = s->_leaf;
  // Check for instruction or instruction chain rule
  if( rule >= _END_INST_CHAIN_RULE || rule < _BEGIN_INST_CHAIN_RULE ) {
    // Instruction
    mach->add_req( s->_leaf->in(0) );// Set initial control
    mach->_opnds[0] = s->MachOperGenerator( _reduceOp[rule], mach, C );
    assert( mach->_opnds[0], "Missing result operand" );
    // Reduce interior of complex instruction
    ReduceInst_Interior( s, rule, mach, 1, mem );
  } else {
    // Instruction chain rules are data-dependent on their inputs
    mach->add_req(0);                   // Set initial control to none
    mach->_opnds[0] = s->MachOperGenerator( _reduceOp[rule], mach, C );
    assert( mach->_opnds[0], "Missing result operand" );
    ReduceInst_Chain_Rule( s, rule, mach, mem );
  }

  // If a Memory was used, insert a Memory edge
  if( mem != (Node*)1 )
    mach->ins_req(MemNode::Memory,mem);

  // If the _leaf is an AddP, insert the base edge
  if( leaf->Opcode() == Op_AddP )
    mach->ins_req(AddPNode::Base,leaf->in(AddPNode::Base));

  int num_proj = _proj_list.size();

  // Perform any 1-to-many expansions required
  MachNode *ex = mach->Expand(s,_proj_list);
  if( ex != mach ) {
    assert(ex->ideal_reg() == mach->ideal_reg(), "ideal types should match");
    if( ex->in(1)->is_Con() )
      ex->in(1)->set_req(0, C->root());
    // Remove old node from the graph
    for( uint i=0; i<mach->req(); i++ ) {
      mach->set_req(i,NULL);
    }
  }

  // PhaseChaitin::fixup_spills will sometimes generate spill code
  // via the matcher.  By the time, nodes have been wired into the CFG,
  // and any further nodes generated by expand rules will be left hanging
  // in space, and will not get emitted as output code.  Catch this.
  // Also, catch any new register allocation constraints ("projections")
  // generated belatedly during spill code generation.
  if (_allocation_started) {
    guarantee(ex == mach, "no expand rules during spill generation");
    int new_proj = _proj_list.size() - num_proj;
    guarantee(new_proj == 0, "no allocation during spill generation");
  }

  return ex;
}

void Matcher::ReduceInst_Chain_Rule( State *s, int rule, MachNode *mach, Node *&mem ) {
  if( s->_leaf->is_Load() ) {
    // !!!!! !!!!!
    // tty->print("\nChain Rule %s on result of leaf: ",ruleName[rule]);
    // s->_leaf->dump();
    // tty->print_cr("");
  }

  // 'op' is what I am expecting to receive
  int op = _leftOp[rule];
  // Operand type to catch childs result
  // This is what my child will give me.
  int opnd_class_instance = s->_rule[op];
  // Choose between operand class or not.
  // This is what I will recieve.
  int catch_op = (FIRST_OPERAND_CLASS <= op && op < NUM_OPERANDS) ? opnd_class_instance : op;
  // New rule for child.  Chase operand classes to get the actual rule.
  int newrule = s->_rule[catch_op];

  if( newrule < NUM_OPERANDS ) {
    // Chain from operand or operand class, may be output of shared node
    assert( 0 <= opnd_class_instance && opnd_class_instance < NUM_OPERANDS,
            "Bad AD file: Instruction chain rule must chain from operand");
    MachOper *oper = s->MachOperGenerator( opnd_class_instance, mach, C );
    // Insert operand into array of operands for this instruction
    mach->_opnds[1] = oper;

    ReduceOper( s, newrule, mach, mem );
    // delete s;  // Done by ReduceOper
  } else {
    // Chain from the result of an instruction
    assert( newrule >= _LAST_MACH_OPER, "Do NOT chain from internal operand");
    MachOper *oper = s->MachOperGenerator( _reduceOp[catch_op], mach, C );
    mach->_opnds[1] = oper;
    Node *mem = (Node*)1;
    mach->add_req( ReduceInst(s, newrule, mem) );
    // delete s;  // Done by ReduceInst_Interior
  }
  return;
}


uint Matcher::ReduceInst_Interior( State *s, int rule, MachNode *mach, uint num_opnds, Node *&mem ) {
  if( s->_leaf->is_Load() ) {
    Node *mem2 = s->_leaf->in(MemNode::Memory);
    assert( mem == (Node*)1 || mem == mem2, "multiple Memories being matched at once?" );
    mem = mem2;
  }
  if( s->_leaf->in(0) && s->_leaf->req() > 1) {
    if( !mach->in(0) ) 
      mach->set_req(0,s->_leaf->in(0));
    else {
      // Allow mach->in(0) to post-dominate s->_leaf->in(0)
      //      assert( s->_leaf->in(0) == mach->in(0), "same instruction, differing controls?" );
    }
  }

  // Now recursively walk the state tree & add operand list.
  for( uint i=0; i<2; i++ ) {   // binary tree
    State *newstate = s->_kids[i];
    if( !newstate ) break;      // Might only have 1 child
    // 'op' is what I am expecting to receive
    int op = (i?_rightOp:_leftOp)[rule];
    // Operand type to catch childs result
    // This is what my child will give me.
    int opnd_class_instance = newstate->_rule[op];
    // Choose between operand class or not.
    // This is what I will receive.
    int catch_op = (op >= FIRST_OPERAND_CLASS && op < NUM_OPERANDS) ? opnd_class_instance : op;
    // New rule for child.  Chase operand classes to get the actual rule.
    int newrule = newstate->_rule[catch_op];

    if( newrule < NUM_OPERANDS ) { // Operand/operandClass or internalOp/instruction?
      // Operand/operandClass
      MachOper *oper = newstate->MachOperGenerator( opnd_class_instance, mach, C );
      // Insert operand into array of operands for this instruction
      mach->_opnds[num_opnds++] = oper;
      ReduceOper( newstate, newrule, mach, mem );

    } else {                    // Child is internal operand or new instruction
      if( newrule < _LAST_MACH_OPER ) { // internal operand or instruction?
        // internal operand --> call ReduceInst_Interior
        // Interior of complex instruction.  Do nothing but recurse.
        num_opnds = ReduceInst_Interior( newstate, newrule, mach, num_opnds, mem );
      } else {
        // instruction --> call build operand(  ) to catch result
        //             --> ReduceInst( newrule )
        mach->_opnds[num_opnds++] = s->MachOperGenerator( _reduceOp[catch_op], mach, C );
        Node *mem = (Node*)1;
        mach->add_req( ReduceInst( newstate, newrule, mem ) );
      }
    }
    assert( mach->_opnds[num_opnds-1], "" );
  }
  delete s;
  return num_opnds;
}

// This routine walks the interior of possible complex operands.
// At each point we check our children in the match tree:
// (1) No children -
//     We are a leaf; add _leaf field as an input to the MachNode
// (2) Child is an internal operand -
//     Skip over it ( do nothing )
// (3) Child is an instruction -
//     Call ReduceInst recursively and
//     and instruction as an input to the MachNode
void Matcher::ReduceOper( State *s, int rule, MachNode *mach, Node *&mem ) {
  assert( rule < _LAST_MACH_OPER, "called with operand rule" );
  assert( !s->_kids[0] || !s->_leaf->in(0), "internal operands have no control" );

  // Leaf?  And not subsumed?
  if( !s->_kids[0] && !_swallowed[rule] ) {
    mach->add_req( s->_leaf );  // Add leaf pointer
    delete s;
    return;                     // Bail out
  }

  if( s->_leaf->is_Load() ) {
    assert( mem == (Node*)1, "multiple Memories being matched at once?" );
    mem = s->_leaf->in(MemNode::Memory);
  }
  if( s->_leaf->in(0) && s->_leaf->req() > 1) {
    if( !mach->in(0) ) 
      mach->set_req(0,s->_leaf->in(0));
    else {
      assert( s->_leaf->in(0) == mach->in(0), "same instruction, differing controls?" );
    }
  }

  for( uint i=0; i<2; i++ ) {   // binary tree
    if( !s->_kids[i] ) break;   // Might be only 1 child
    int op = (i?_rightOp:_leftOp)[rule];
    int newrule = s->_kids[i]->_rule[op];
    if( newrule < _LAST_MACH_OPER ) { // Operand or instruction?
      // Internal operand; recurse but do nothing else
      ReduceOper( s->_kids[i], newrule, mach, mem );

    } else {                    // Child is a new instruction
      // Reduce the instruction, and add a direct pointer from this
      // machine instruction to the newly reduced one.
      Node *mem = (Node*)1;
      mach->add_req( ReduceInst( s->_kids[i], newrule, mem ) );
    }
  }
  delete s;
}


// -------------------------------------------------------------------------
// Java-Java calling convention
// (what you use when Java calls Java)

//------------------------------find_receiver----------------------------------
// For a given signature, return the OptoReg for parameter 0.
VMReg::Name Matcher::find_receiver( bool is_outgoing ) {
  OptoRegPair regs(0,0);
  regs.set_ideal_reg(Op_RegP);
  calling_convention(&regs,1,is_outgoing);
  // Return argument 0 register.  In the LP64 build pointers
  // take 2 registers, but the VM wants only the 'main' name.
  return VMReg::Name(regs.lo());
}

OptoRegPair *Matcher::find_callee_arguments(symbolOop sig, bool is_static, int* arg_size) {  
  // This method is returning a data structure allocating as a
  // ResourceObject, so do not put any ResourceMarks in here.
  char *s = sig->as_C_string();
  int len = (int)strlen(s);
  *s++; len--;                  // Skip opening paren
  char *t = s+len;
  while( *(--t) != ')' ) ;      // Find close paren

  OptoRegPair *regs = NEW_RESOURCE_ARRAY( OptoRegPair, 256 );
  int cnt = 0;
  if (!is_static) {    
    regs[cnt++].set_ideal_reg(Op_RegP); // Receiver is argument 0; not in signature
  }

  while( s < t ) {
    switch( *s++ ) {            // Switch on signature character
    case 'B': regs[cnt++].set_ideal_reg(Op_RegI); break;
    case 'C': regs[cnt++].set_ideal_reg(Op_RegI); break;
    case 'D': regs[cnt++].set_ideal_reg(Op_RegD); regs[cnt++].set_ideal_reg(0); break;
    case 'F': regs[cnt++].set_ideal_reg(Op_RegF); break;
    case 'I': regs[cnt++].set_ideal_reg(Op_RegI); break;
    case 'J': regs[cnt++].set_ideal_reg(Op_RegL); regs[cnt++].set_ideal_reg(0); break;
    case 'S': regs[cnt++].set_ideal_reg(Op_RegI); break;
    case 'Z': regs[cnt++].set_ideal_reg(Op_RegI); break;
    case 'V': regs[cnt++].set_ideal_reg(0);       break;
    case 'L':                   // Oop
      while( *s++ != ';'  ) ;   // Skip signature
      regs[cnt++].set_ideal_reg(Op_RegP); 
      break;
    case '[': {                 // Array
      do {                      // Skip optional size
        while( *s >= '0' && *s <= '9' ) s++;
      } while( *s++ == '[' );   // Nested arrays?
      // Skip element type
      if( s[-1] == 'L' )
        while( *s++ != ';'  ) ; // Skip signature
      regs[cnt++].set_ideal_reg(Op_RegP); 
      break;
    }
    default : ShouldNotReachHere();
    }
  }
  assert( cnt < 256, "grow table size" );

  // Pass array of ideal registers and length to USER code (from the
  // AD file) that will convert this to an array of register numbers.
  // The "true" means report arguments from the caller's point of
  // view.  The name "callee arguments" means arguments to the current
  // frame's call, but named according the caller's point of view.
  calling_convention(regs,cnt,true);

  for (int i = 0; i < cnt; i++) {
    OptoReg::Name reg1 = regs[i].lo();
    if( reg1 >= SharedInfo::stack0 )  // Stack slot argument?
      reg1 = OptoReg::add(reg1, Compile::out_preserve_stack_slots());
    OptoReg::Name reg2 = regs[i].hi();
    if( reg2 != OptoReg::Bad && reg2 >= SharedInfo::stack0 )  // Stack slot argument?
      reg2 = OptoReg::add(reg2, Compile::out_preserve_stack_slots());
    regs[i].set_pair(reg2,reg1);
  }

  // results
  *arg_size = cnt;
  return regs;
}

// A method-klass-holder may be passed in the inline_cache_reg
// and then expanded into the inline_cache_reg and a method_oop register
//   defined in ad_<arch>.cpp


//------------------------------find_shared------------------------------------
// Set bits if Node is shared or otherwise a root
void Matcher::find_shared( Node *n ) {
  // Allocate stack of size C->unique() * 2 to avoid frequent realloc
  MStack mstack(C->unique() * 2); 
  mstack.push(n, Visit);     // Don't need to pre-visit root node
  while (mstack.is_nonempty()) {
    n = mstack.node();       // Leave node on stack
    Node_State nstate = mstack.state();
    if (nstate == Pre_Visit) {
      if (is_visited(n)) {   // Visited already?
        // Node is shared and has no reason to clone.  Flag it as shared.  
        // This causes it to match into a register for the sharing.
        set_shared(n);       // Flag as shared and
        mstack.pop();        // remove node from stack
        continue;
      }
      nstate = Visit; // Not already visited; so visit now
    }
    if (nstate == Visit) {
      mstack.set_state(Post_Visit);
      set_visited(n);   // Flag as visited now
      bool mem_op = false;

      switch( n->Opcode() ) {  // Handle some opcodes special
      case Op_Phi:             // Treat Phis as shared roots
      case Op_Parm:
      case Op_Proj:            // All handled specially during matching
        set_shared(n);
        set_dontcare(n);
        break;
      case Op_If:
      case Op_CountedLoopEnd:
        mstack.set_state(Alt_Post_Visit); // Alternative way
        // Convert (If (Bool (CmpX A B))) into (If (Bool) (CmpX A B)).  Helps
        // with matching cmp/branch in 1 instruction.  The Matcher needs the
        // Bool and CmpX side-by-side, because it can only get at constants
        // that are at the leaves of Match trees, and the Bool's condition acts
        // as a constant here.  
        mstack.push(n->in(1), Visit);         // Clone the Bool
        mstack.push(n->in(0), Pre_Visit);     // Visit control input
        continue; // while (mstack.is_nonempty())
      case Op_ConvI2D:         // These forms efficiently match with a prior
      case Op_ConvI2F:         //   Load but not a following Store
        if( n->in(1)->is_Load() &&        // Prior load
            n->outcnt() == 1 &&           // Not already shared
            n->unique_out()->is_Store() ) // Following store
          set_shared(n);       // Force it to be a root
        break;
      case Op_BoxLock:         // Cant match until we get stack-regs in ADLC
      case Op_IfFalse:
      case Op_IfTrue:
      case Op_MachProj:
      case Op_MergeMem:
      case Op_Catch:
      case Op_CatchProj:
      case Op_CProj:
      case Op_JumpProj:
      case Op_JProj:
      case Op_NeverBranch:
        set_dontcare(n);
        break;
      case Op_Jump:
        mstack.push(n->in(1), Visit);         // Switch Value
        mstack.push(n->in(0), Pre_Visit);     // Visit Control input
        continue;                             // while (mstack.is_nonempty())                   
      case Op_StrComp:
        set_shared(n); // Force result into register (it will be anyways)
        break;
      case Op_ConP: {  // Convert pointers above the centerline to NUL
        TypeNode *tn = n->is_Type(); // Constants derive from type nodes
        const TypePtr* tp = tn->type()->is_ptr();
        if (tp->_ptr == TypePtr::AnyNull) {
          tn->set_type(TypePtr::NULL_PTR);
        }
        break;
      }
      case Op_Binary:         // Match these, despite no ideal reg
        break;
      case Op_StoreB:         // Do match these, despite no ideal reg
      case Op_StoreC:
      case Op_StoreCM:
      case Op_StoreD:
      case Op_StoreF:
      case Op_StoreI:
      case Op_StoreL:
      case Op_StoreP:
      case Op_ClearArray:
      case Op_SafePoint:
        mem_op = true;
        break;
      case Op_LoadB:
      case Op_LoadC:
      case Op_LoadD:
      case Op_LoadF:
      case Op_LoadI:
      case Op_LoadKlass:
      case Op_LoadL:
      case Op_LoadS:
      case Op_LoadP:
      case Op_LoadRange:
      case Op_LoadD_unaligned:
      case Op_LoadL_unaligned:
        mem_op = true;
        // Must be root of match tree due to prior load conflict
        if( C->subsume_loads() == false ) {
          set_shared(n);
        }
        // Fall into default case
      default:
        if( !n->ideal_reg() )
          set_dontcare(n);  // Unmatchable Nodes
      } // end_switch

      for(int i = n->req() - 1; i >= 0; --i) { // For my children
        Node *m = n->in(i); // Get ith input
        if (m == NULL) continue;  // Ignore NULLs
        uint mop = m->Opcode();

        // Must clone all producers of flags, or we will not match correctly.
        // Suppose a compare setting int-flags is shared (e.g., a switch-tree)
        // then it will match into an ideal Op_RegFlags.  Alas, the fp-flags
        // are also there, so we may match a float-branch to int-flags and
        // expect the allocator to haul the flags from the int-side to the
        // fp-side.  No can do.
        if( _must_clone[mop] ) {
          mstack.push(m, Visit);
          continue; // for(int i = ...)
        } 

        // Clone addressing expressions as they are "free" in most instructions
        if( mem_op && i == MemNode::Address && mop == Op_AddP ) {
          Node *off = m->in(AddPNode::Offset);
          if( off->is_Con() ) {
            set_visited(m);  // Flag as visited now
            Node *adr = m->in(AddPNode::Address);

            // Intel, ARM and friends can handle 2 adds in addressing mode
            if( clone_shift_expressions && adr->Opcode() == Op_AddP &&
                // AtomicAdd is not an addressing expression.
                // Cheap to find it by looking for screwy base.
                !adr->in(AddPNode::Base)->is_top() ) {
              set_visited(adr);  // Flag as visited now
              Node *shift = adr->in(AddPNode::Offset);
              // Check for shift by small constant as well
              if( shift->Opcode() == Op_LShiftX && shift->in(2)->is_Con() && 
                  shift->in(2)->get_int() <= 3 ) { 
                set_visited(shift);  // Flag as visited now
                mstack.push(shift->in(2), Visit);
                mstack.push(shift->in(1), Pre_Visit);
              } else {
                mstack.push(shift, Pre_Visit);
              }
              mstack.push(adr->in(AddPNode::Address), Pre_Visit);
              mstack.push(adr->in(AddPNode::Base), Pre_Visit);
            } else {  // Sparc, Alpha, PPC and friends
              mstack.push(adr, Pre_Visit);
            }

            // Clone X+offset as it also folds into most addressing expressions
            mstack.push(off, Visit);
            mstack.push(m->in(AddPNode::Base), Pre_Visit);
            continue; // for(int i = ...)
          } // if( off->is_Con() )
        }   // if( mem_op &&
        mstack.push(m, Pre_Visit);
      }     // for(int i = ...)
    }
    else if (nstate == Alt_Post_Visit) {
      mstack.pop(); // Remove node from stack
      // We cannot remove the Cmp input from the Bool here, as the Bool may be
      // shared and all users of the Bool need to move the Cmp in parallel.
      // This leaves both the Bool and the If pointing at the Cmp.  To 
      // prevent the Matcher from trying to Match the Cmp along both paths
      // BoolNode::match_edge always returns a zero.

      // We reorder the Op_If in a pre-order manner, so we can visit without
      // accidently sharing the Cmp (the Bool and the If make 2 users).
      n->add_req( n->in(1)->in(1) ); // Add the Cmp next to the Bool
    }
    else if (nstate == Post_Visit) {
      mstack.pop(); // Remove node from stack

      // Now hack a few special opcodes
      switch( n->Opcode() ) {       // Handle some opcodes special
      case Op_StorePConditional:
      case Op_StoreLConditional: {  // Convert trinary to binary-tree
        Node *newval = n->in(MemNode::ValueIn );
        Node *oldval  = n->in(StorePConditionalNode::LL_input);
        Node *pair = new (3) BinaryNode( oldval, newval );
        n->set_req(MemNode::ValueIn,pair);
        n->del_req(StorePConditionalNode::LL_input);
        break;
      }
      case Op_CompareAndSwapI:
      case Op_CompareAndSwapL:
      case Op_CompareAndSwapP: {	// Convert trinary to binary-tree
        Node *newval = n->in(MemNode::ValueIn );
        Node *oldval  = n->in(CompareAndSwapNode::ExpectedIn);
        Node *pair = new (3) BinaryNode( oldval, newval );
        n->set_req(MemNode::ValueIn,pair);
        n->del_req(CompareAndSwapNode::ExpectedIn);
        break;
      }
      case Op_CMoveD:              // Convert trinary to binary-tree
      case Op_CMoveF:
      case Op_CMoveI:
      case Op_CMoveL:
      case Op_CMoveP: {
        // Restructure into a binary tree for Matching.  It's possible that
        // we could move this code up next to the graph reshaping for IfNodes
        // or vice-versa, but I do not want to debug this for Ladybird.
        // 10/2/2000 CNC.
        Node *pair1 = new (3) BinaryNode(n->in(1),n->in(1)->in(1));
        n->set_req(1,pair1);
        Node *pair2 = new (3) BinaryNode(n->in(2),n->in(3));
        n->set_req(2,pair2);
        n->del_req(3);
        break;
      }
      default:
        break;
      }
    }
    else {
      ShouldNotReachHere();
    }
  } // end of while (mstack.is_nonempty())
}

#ifdef ASSERT
// machine-independent root to machine-dependent root
void Matcher::dump_old2new_map() {
  _old2new_map.dump();
}
#endif

//---------------------------collect_null_checks-------------------------------
// Find null checks in the ideal graph; write a machine-specific node for
// it.  Used by later implicit-null-check handling.  Actually collects
// either an IfTrue or IfFalse for the common NOT-null path, AND the ideal
// value being tested.
void Matcher::collect_null_checks( Node *proj ) {
  Node *iff = proj->in(0);
  if( iff->Opcode() == Op_If ) {
    // During matching If's have Bool & Cmp side-by-side
    BoolNode *b = iff->in(1)->is_Bool();
    Node *cmp = iff->in(2);
    if( cmp->Opcode() == Op_CmpP ) {
      if( cmp->in(2)->bottom_type() == TypePtr::NULL_PTR ) {

        if( proj->Opcode() == Op_IfTrue ) {
          extern int all_null_checks_found;
          all_null_checks_found++;
          if( b->_test._test == BoolTest::ne ) {
            _null_check_tests.push(proj);
            _null_check_tests.push(cmp->in(1));
          }
        } else {
          assert( proj->Opcode() == Op_IfFalse, "" );
          if( b->_test._test == BoolTest::eq ) {
            _null_check_tests.push(proj);
            _null_check_tests.push(cmp->in(1));
          }
        }
      }
    }
  }
}

//---------------------------validate_null_checks------------------------------
// Its possible that the value being NULL checked is not the root of a match
// tree.  If so, I cannot use the value in an implicit null check.
void Matcher::validate_null_checks( ) {
  uint cnt = _null_check_tests.size();
  for( uint i=0; i < cnt; i+=2 ) {
    Node *test = _null_check_tests[i];
    Node *val = _null_check_tests[i+1];
    if (has_new_node(val)) {
      // Is a match-tree root, so replace with the matched value 
      _null_check_tests.map(i+1, new_node(val));
    } else {
      // Yank from candidate list
      _null_check_tests.map(i+1,_null_check_tests[--cnt]);
      _null_check_tests.map(i,_null_check_tests[--cnt]);
      _null_check_tests.pop();
      _null_check_tests.pop();
      i-=2;
    }
  }  
}


// Used by the DFA in dfa_sparc.cpp.  Check for a prior FastLock
// acting as an Acquire and thus we don't need an Acquire here.  We
// retain the Node to act as a compiler ordering barrier.
bool Matcher::prior_fast_lock( const Node *acq ) {
  Node *r = acq->in(0);
  if( !r->is_Region() || r->req() <= 1 ) return false;
  Node *proj = r->in(1);
  if( !proj->is_Proj() ) return false;
  CallNode *call = proj->in(0)->is_Call();
  if( !call || call->entry_point() != OptoRuntime::complete_monitor_locking_Java() )
    return false;

  return true;
}

// Used by the DFA in dfa_sparc.cpp.  Check for a following FastUnLock
// acting as a Release and thus we don't need a Release here.  We
// retain the Node to act as a compiler ordering barrier.
bool Matcher::post_fast_unlock( const Node *rel ) {
  Compile *C = Compile::current();
  assert( rel->Opcode() == Op_MemBarRelease, "" );
  const MemBarReleaseNode *mem = (const MemBarReleaseNode*)rel;
  DUIterator_Fast imax, i = mem->fast_outs(imax); 
  Node *ctrl = NULL;
  while( true ) {
    ctrl = mem->fast_out(i);            // Throw out-of-bounds if proj not found
    assert( ctrl->is_Proj(), "only projections here" );
    ProjNode *proj = (ProjNode*)ctrl;
    if( proj->_con == TypeFunc::Control &&
        !C->node_arena()->contains(ctrl) ) // Unmatched old-space only
      break;
    i++;
  }
  Node *iff = NULL;
  for( DUIterator_Fast jmax, j = ctrl->fast_outs(jmax); j < jmax; j++ ) {
    Node *x = ctrl->fast_out(j); 
    if( x->is_If() && x->req() > 1 && 
        !C->node_arena()->contains(x) ) { // Unmatched old-space only
      iff = x;
      break;
    }
  }
  if( !iff ) return false;
  Node *bol = iff->in(1);
  // The iff might be some random subclass of If or bol might be Con-Top
  if (!bol->is_Bool())  return false;
  assert( bol->req() > 1, "" );
  return (bol->in(1)->Opcode() == Op_FastUnlock);
}

// Used by the DFA in dfa_xxx.cpp.  Check for a following barrier or
// atomic instruction acting as a store_load barrier without any
// intervening volatile load, and thus we don't need a barrier here.
// We retain the Node to act as a compiler ordering barrier.
bool Matcher::post_store_load_barrier(const Node *vmb) {
  Compile *C = Compile::current();
  assert( vmb->is_MemBar(), "" );
  assert( vmb->Opcode() != Op_MemBarAcquire, "" );
  const MemBarNode *mem = (const MemBarNode*)vmb;

  // Get the Proj node, ctrl, that can be used to iterate forward
  Node *ctrl = NULL;
  DUIterator_Fast imax, i = mem->fast_outs(imax); 
  while( true ) {
    ctrl = mem->fast_out(i);		// Throw out-of-bounds if proj not found
    assert( ctrl->is_Proj(), "only projections here" );
    ProjNode *proj = (ProjNode*)ctrl;
    if( proj->_con == TypeFunc::Control &&
	!C->node_arena()->contains(ctrl) ) // Unmatched old-space only
      break;
    i++;
  }

  for( DUIterator_Fast jmax, j = ctrl->fast_outs(jmax); j < jmax; j++ ) {
    Node *x = ctrl->fast_out(j); 
    int xop = x->Opcode();

    // We don't need current barrier if we see another or a lock
    // before seeing volatile load. 
    if (xop == Op_MemBarVolatile || 
        xop == Op_FastLock ||
        xop == Op_FastUnlock ||
        xop == Op_CompareAndSwapL ||
        xop == Op_CompareAndSwapP ||
        xop == Op_CompareAndSwapI)
      return true;

    if (x->is_MemBar()) {
      // We must retain this membar if there is an upcoming volatile
      // load, which will be preceded by acquire membar.
      if (xop == Op_MemBarAcquire) 
        return false;
      // For other kinds of barriers, check by pretending we
      // are them, and seeing if we can be removed.
      else 
        return post_store_load_barrier((const MemBarNode*)x);
    }

    // Delicate code to detect case of an upcoming fastlock block
    if( x->is_If() && x->req() > 1 && 
	!C->node_arena()->contains(x) ) { // Unmatched old-space only
      Node *iff = x;
      Node *bol = iff->in(1);
      // The iff might be some random subclass of If or bol might be Con-Top
      if (!bol->is_Bool())  return false;
      assert( bol->req() > 1, "" );
      return (bol->in(1)->Opcode() == Op_FastUnlock);
    }
    // probably not necessary to check for these
    if (x->is_Call() || x->is_SafePoint() || x->is_block_proj())
      return false;
  }
  return false;
}

//=============================================================================
//---------------------------State---------------------------------------------
State::State(void) {
#ifdef ASSERT
  _id = 0;
  _kids[0] = _kids[1] = (State*)CONST64(0xcafebabecafebabe);
  _leaf = (Node*) CONST64(0xbaadf00dbaadf00d);
  //memset(_cost, -1, sizeof(_cost));
  //memset(_rule, -1, sizeof(_rule));
#endif
  memset(_valid, 0, sizeof(_valid));
}

State::~State() {
#ifdef ASSERT
  _id = 99;
  _kids[0] = _kids[1] = (State*)CONST64(0xcafebabecafebabe);
  _leaf = (Node*) CONST64(0xbaadf00dbaadf00d);
  memset(_cost, -3, sizeof(_cost));
  memset(_rule, -3, sizeof(_rule));
#endif
}

//---------------------------dump----------------------------------------------
void State::dump() { 
  tty->print("\n");
  dump(0); 
}

void State::dump(int depth) { 
  for( int j = 0; j < depth; j++ )
    tty->print("   ");
  tty->print("--N: ");
#ifndef PRODUCT
  _leaf->dump();
#endif
  uint i;
  for( i = 0; i < _LAST_MACH_OPER; i++ )
    // Check for valid entry
    if( valid(i) ) {
      for( int j = 0; j < depth; j++ )
        tty->print("   ");
        assert(_cost[i] != max_juint, "cost must be a valid value");
        assert(_rule[i] < _last_Mach_Node, "rule[i] must be valid rule");
        tty->print_cr("%s  %d  %s", 
                      ruleName[i], _cost[i], ruleName[_rule[i]] );
      }
  tty->print_cr("");

  for( i=0; i<2; i++ )
    if( _kids[i] )
      _kids[i]->dump(depth+1);
}

