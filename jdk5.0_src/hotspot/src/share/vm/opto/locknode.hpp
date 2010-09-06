#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)locknode.hpp	1.28 03/12/23 16:42:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//------------------------------BoxLockNode------------------------------------
class BoxLockNode : public Node {
public:
  const int _locknum;
  RegMask   _inmask;

  BoxLockNode( int lock );
  virtual int Opcode() const;
  virtual void emit(CodeBuffer &cbuf, PhaseRegAlloc *ra_) const;
  virtual uint size(PhaseRegAlloc *ra_) const;
  virtual bool rematerialize() const { return true; }
  virtual const RegMask &in_RegMask(uint) const;
  virtual const RegMask &out_RegMask() const;
  virtual uint size_of() const;
  virtual uint hash() const { return Node::hash() + _locknum; }
  virtual uint cmp( const Node &n ) const;
  virtual const class Type *bottom_type() const { return TypeRawPtr::BOTTOM; }
  virtual uint ideal_reg() const { return Op_RegP; }
  virtual const BoxLockNode *is_BoxLock() const { return this; }

  static OptoReg::Name stack_slot(Node* box_node);

#ifndef PRODUCT
  virtual void format( PhaseRegAlloc * ) const;
  virtual void dump_spec() const { tty->print("  Lock %d",_locknum); }
#endif
};

//------------------------------FastLockNode-----------------------------------
class FastLockNode: public CmpNode {
public:
  FastLockNode(Node *ctrl, Node *oop, Node *box) : CmpNode(oop,box) {
    set_req(0,ctrl);
  }
  virtual int Opcode() const;
  virtual const FastLockNode *is_FastLock() const { return this; }
  Node* obj_node() const { return in(1); }
  Node* box_node() const { return in(2); }
  virtual const Type *Value( PhaseTransform *phase ) const { return TypeInt::CC; }
  const Type *sub(const Type *t1, const Type *t2) const { return TypeInt::CC;}
};


//------------------------------FastUnlockNode---------------------------------
class FastUnlockNode: public CmpNode {
public:
  FastUnlockNode(Node *ctrl, Node *oop, Node *box) : CmpNode(oop,box) {
    set_req(0,ctrl);
  }
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const { return TypeInt::CC; }
  const Type *sub(const Type *t1, const Type *t2) const { return TypeInt::CC;}
};

