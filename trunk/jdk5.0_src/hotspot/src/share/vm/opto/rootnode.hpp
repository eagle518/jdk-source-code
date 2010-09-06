#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)rootnode.hpp	1.41 03/12/23 16:42:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//------------------------------RootNode---------------------------------------
// The one-and-only before-all-else and after-all-else RootNode.  The RootNode
// represents what happens if the user runs the whole program repeatedly.  The
// RootNode produces the initial values of I/O and memory for the program or
// procedure start.
class RootNode : public LoopNode {
public:
  RootNode( ) : LoopNode(0,0) { del_req(2); del_req(1); }
  virtual int   Opcode() const;
  virtual RootNode *is_Root() { return this; }
  virtual const Node *is_block_proj() const { return this; }
  virtual const Type *bottom_type() const { return Type::BOTTOM; }
  virtual Node *Identity( PhaseTransform *phase ) { return this; }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const { return Type::BOTTOM; }
};

//------------------------------HaltNode---------------------------------------
// Throw an exception & die
class HaltNode : public Node {
public:
  HaltNode( Node *ctrl, Node *frameptr );
  virtual int Opcode() const;
  virtual int pinned() const { return 1; };
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const;
  virtual bool  is_CFG() const { return true; }
  virtual uint hash() const { return NO_HASH; }  // CFG nodes do not hash
  virtual bool depends_only_on_test() const { return false; }
  virtual const Node *is_block_proj() const { return this; }
  virtual const RegMask &out_RegMask() const;
  virtual uint ideal_reg() const { return NotAMachineReg; }
  virtual uint match_edge(uint idx) const { return 0; }
};

