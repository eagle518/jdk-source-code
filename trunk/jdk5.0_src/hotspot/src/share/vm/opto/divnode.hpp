#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)divnode.hpp	1.24 03/12/23 16:42:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style


//------------------------------DivINode---------------------------------------
// Integer division
// Note: this is division as defined by JVMS, i.e., MinInt/-1 == MinInt.
// On processors which don't naturally support this special case (e.g., x86), 
// the matcher or runtime system must take care of this.
class DivINode : public Node {
public:
  DivINode( Node *c, Node *dividend, Node *divisor ) : Node(c, dividend, divisor ) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
};

//------------------------------DivLNode---------------------------------------
// Long division
class DivLNode : public Node {
public:
  DivLNode( Node *c, Node *dividend, Node *divisor ) : Node(c, dividend, divisor ) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual uint ideal_reg() const { return Op_RegL; }
};

//------------------------------DivFNode---------------------------------------
// Float division
class DivFNode : public Node {
public:
  DivFNode( Node *c, Node *dividend, Node *divisor ) : Node(c, dividend, divisor) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
};

//------------------------------DivDNode---------------------------------------
// Double division
class DivDNode : public Node {
public:
  DivDNode( Node *c, Node *dividend, Node *divisor ) : Node(c,dividend, divisor) {}
  virtual int Opcode() const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};

//------------------------------ModINode---------------------------------------
// Integer modulus
class ModINode : public Node {
public:
  ModINode( Node *c, Node *in1, Node *in2 ) : Node(c,in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
};

//------------------------------ModLNode---------------------------------------
// Long modulus
class ModLNode : public Node {
public:
  ModLNode( Node *c, Node *in1, Node *in2 ) : Node(c,in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual uint ideal_reg() const { return Op_RegL; }
};

//------------------------------ModFNode---------------------------------------
// Float Modulus
class ModFNode : public Node {
public:
  ModFNode( Node *c, Node *in1, Node *in2 ) : Node(c,in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
};

//------------------------------ModDNode---------------------------------------
// Double Modulus
class ModDNode : public Node {
public:
  ModDNode( Node *c, Node *in1, Node *in2 ) : Node(c, in1, in2) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};


