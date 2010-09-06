#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)locknode.cpp	1.36 03/12/23 16:42:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_locknode.cpp.incl"

//=============================================================================
const RegMask &BoxLockNode::in_RegMask(uint i) const {
  return _inmask;
}

const RegMask &BoxLockNode::out_RegMask() const { 
  return *Matcher::idealreg2regmask[Op_RegP];
}

uint BoxLockNode::size_of() const { return sizeof(*this); }

BoxLockNode::BoxLockNode( int lock ) : Node( Compile::current()->root() ), _locknum(lock) {
  OptoReg::Name reg = SharedInfo::stack2reg(_locknum * Compile::current()->sync_stack_slots());
  _inmask.Insert(reg);
}

//------------------------------cmp--------------------------------------------
uint BoxLockNode::cmp( const Node &n ) const {
  const BoxLockNode &bn = (const BoxLockNode &)n;
  return bn._locknum == _locknum;
}

OptoReg::Name BoxLockNode::stack_slot(Node* box_node) {
  // Chase down the BoxNode 
  while (!box_node->is_BoxLock()) {
    //    if (box_node->is_SpillCopy()) {
    //      MachNode *m = box_node->in(1)->is_Mach();
    //      if (m && m->ideal_Opcode() == Op_StoreP) {
    //        box_node = m->in(m->operand_index(2));
    //        continue;
    //      }
    //    }
    assert(box_node->is_SpillCopy() || box_node->is_Phi(), "Bad spill of Lock.");
    box_node = box_node->in(1);
  }
  return box_node->in_RegMask(0).find_first_elem();
}

//=============================================================================
//------------------------------do_monitor_enter-------------------------------
void Parse::do_monitor_enter() {
  kill_dead_locals();
 
  // Null check; get casted pointer.
  Node *obj = do_null_check(peek(), T_OBJECT);
  // Check for locking null object
  if (stopped()) return;

  // the monitor object is not part of debug info expression stack
  pop(); 

  // Insert a FastLockNode which takes as arguments the current thread pointer,
  // the obj pointer & the address of the stack slot pair used for the lock.
  shared_lock(obj);
}

//------------------------------do_monitor_exit--------------------------------
void Parse::do_monitor_exit() {
  kill_dead_locals();

  pop();                        // Pop oop to unlock
  // Because monitors are guarenteed paired (else we bail out), we know
  // the matching Lock for this Unlock.  Hence we know there is no need
  // for a null check on Unlock.
  shared_unlock(map()->peek_monitor_box(), map()->peek_monitor_obj());
} 


