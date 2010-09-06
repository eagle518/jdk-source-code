#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)rootnode.cpp	1.70 03/12/23 16:42:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_rootnode.cpp.incl"

//------------------------------Ideal------------------------------------------
// Remove dead inputs
Node *RootNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  for( uint i = 1; i < req(); i++ ) { // For all inputs
    // Check for and remove dead inputs
    if( phase->type(in(i)) == Type::TOP ) {
      del_req(i--);             // Delete TOP inputs
    }
  }

  // I used to do tail-splitting in the Ideal graph here, but it does not
  // work.  The tail-splitting forces values live into the Return to be
  // ready at a point which dominates the split returns.  This forces Stores
  // to be hoisted high.  The "proper" fix would be to split Stores down
  // each path, but this makes the split unprofitable.  If we want to do this
  // optimization, it needs to be done after allocation so we can count all
  // the instructions needing to be cloned in the cost metric.

  // There used to be a spoof here for caffeine marks which completely
  // eliminated very simple self-recursion recursions, but it's not worth it.
  // Deep inlining of self-calls gets nearly all of the same benefits.
  // If we want to get the rest of the win later, we should pattern match
  // simple recursive call trees to closed-form solutions.

  return NULL;                  // No further opportunities exposed
}

//=============================================================================
HaltNode::HaltNode( Node *ctrl, Node *frameptr ) : Node(TypeFunc::Parms) { 
  Node* top = Compile::current()->top();
  set_req(TypeFunc::Control,  ctrl        ); 
  set_req(TypeFunc::I_O,      top);
  set_req(TypeFunc::Memory,   top);
  set_req(TypeFunc::FramePtr, frameptr    );
  set_req(TypeFunc::ReturnAdr,top);
}

const Type *HaltNode::bottom_type() const { return Type::BOTTOM; }

//------------------------------Ideal------------------------------------------
Node *HaltNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  return remove_dead_region(phase, can_reshape) ? this : NULL;
}

//------------------------------Value------------------------------------------
const Type *HaltNode::Value( PhaseTransform *phase ) const { 
  return ( phase->type(in(TypeFunc::Control)) == Type::TOP)
    ? Type::TOP
    : Type::BOTTOM;
}

const RegMask &HaltNode::out_RegMask() const { 
  return RegMask::Empty;
}


