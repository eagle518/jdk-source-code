#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)subnode.hpp	1.69 03/12/23 16:43:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

//------------------------------SUBNode----------------------------------------
// Class SUBTRACTION functionality.  This covers all the usual 'subtract'
// behaviors.  Subtract-integer, -float, -double, binary xor, compare-integer,
// -float, and -double are all inherited from this class.  The compare 
// functions behave like subtract functions, except that all negative answers
// are compressed into -1, and all positive answers compressed to 1.
class SubNode : public Node {
public:
  SubNode( Node *in1, Node *in2 ) : Node(0,in1,in2) {}

  // Handle algebraic identities here.  If we have an identity, return the Node
  // we are equivalent to.  We look for "add of zero" as an identity.  
  virtual Node *Identity( PhaseTransform *phase );

  // Compute a new Type for this node.  Basically we just do the pre-check,
  // then call the virtual add() to set the type.
  virtual const Type *Value( PhaseTransform *phase ) const;

  // Supplied function returns the subtractend of the inputs.
  // This also type-checks the inputs for sanity.  Guaranteed never to
  // be passed a TOP or BOTTOM type, these are filtered out by a pre-check.
  virtual const Type *sub( const Type *, const Type * ) const = 0;

  // Supplied function to return the additive identity type.
  // This is returned whenever the subtracts inputs are the same.
  virtual const Type *add_id() const = 0;

  virtual SubNode *is_Sub() { return this; }
};


// NOTE: SubINode should be taken away and replaced by add and negate
//------------------------------SubINode---------------------------------------
// Subtract 2 integers
class SubINode : public SubNode {
public:
  SubINode( Node *in1, Node *in2 ) : SubNode(in1,in2) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *sub( const Type *, const Type * ) const;
  const Type *add_id() const { return TypeInt::ZERO; }
  const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
};

//------------------------------SubLNode---------------------------------------
// Subtract 2 integers
class SubLNode : public SubNode {
public:
  SubLNode( Node *in1, Node *in2 ) : SubNode(in1,in2) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *sub( const Type *, const Type * ) const;
  const Type *add_id() const { return TypeLong::ZERO; }
  const Type *bottom_type() const { return TypeLong::LONG; }
  virtual uint ideal_reg() const { return Op_RegL; }
};

// NOTE: SubFPNode should be taken away and replaced by add and negate
//------------------------------SubFPNode--------------------------------------
// Subtract 2 floats or doubles
class SubFPNode : public SubNode {
protected:
  SubFPNode( Node *in1, Node *in2 ) : SubNode(in1,in2) {}
public:
  const Type *Value( PhaseTransform *phase ) const;
};

// NOTE: SubFNode should be taken away and replaced by add and negate
//------------------------------SubFNode---------------------------------------
// Subtract 2 doubles
class SubFNode : public SubFPNode {
public:
  SubFNode( Node *in1, Node *in2 ) : SubFPNode(in1,in2) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *sub( const Type *, const Type * ) const;
  const Type   *add_id() const { return TypeF::ZERO; }
  const Type   *bottom_type() const { return Type::FLOAT; }
  virtual uint  ideal_reg() const { return Op_RegF; }
};

// NOTE: SubDNode should be taken away and replaced by add and negate
//------------------------------SubDNode---------------------------------------
// Subtract 2 doubles
class SubDNode : public SubFPNode {
public:
  SubDNode( Node *in1, Node *in2 ) : SubFPNode(in1,in2) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *sub( const Type *, const Type * ) const;
  const Type   *add_id() const { return TypeD::ZERO; }
  const Type   *bottom_type() const { return Type::DOUBLE; }
  virtual uint  ideal_reg() const { return Op_RegD; }
};

//------------------------------CmpNode---------------------------------------
// Compare 2 values, returning condition codes (-1, 0 or 1).
class CmpNode : public SubNode {
public:
  CmpNode( Node *in1, Node *in2 ) : SubNode(in1,in2) {}
  virtual Node *Identity( PhaseTransform *phase );
  const Type *add_id() const { return TypeInt::ZERO; }
  const Type *bottom_type() const { return TypeInt::CC; }
  virtual const CmpNode *is_Cmp() const { return this; }
  virtual uint ideal_reg() const { return Op_RegFlags; }
};

//------------------------------CmpINode---------------------------------------
// Compare 2 signed values, returning condition codes (-1, 0 or 1).
class CmpINode : public CmpNode {
public:
  CmpINode( Node *in1, Node *in2 ) : CmpNode(in1,in2) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *sub( const Type *, const Type * ) const;
};

//------------------------------CmpUNode---------------------------------------
// Compare 2 unsigned values (integer or pointer), returning condition codes (-1, 0 or 1).
class CmpUNode : public CmpNode {
public:
  CmpUNode( Node *in1, Node *in2 ) : CmpNode(in1,in2) {}
  virtual int Opcode() const;
  virtual const Type *sub( const Type *, const Type * ) const;
};

//------------------------------CmpPNode---------------------------------------
// Compare 2 pointer values, returning condition codes (-1, 0 or 1).
class CmpPNode : public CmpNode {
public:
  CmpPNode( Node *in1, Node *in2 ) : CmpNode(in1,in2) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *sub( const Type *, const Type * ) const;
};

//------------------------------CmpLNode---------------------------------------
// Compare 2 long values, returning condition codes (-1, 0 or 1).
class CmpLNode : public CmpNode {
public:
  CmpLNode( Node *in1, Node *in2 ) : CmpNode(in1,in2) {}
  virtual int    Opcode() const;
  virtual const Type *sub( const Type *, const Type * ) const;
};

//------------------------------CmpL3Node--------------------------------------
// Compare 2 long values, returning integer value (-1, 0 or 1).
class CmpL3Node : public CmpLNode {
public:
  CmpL3Node( Node *in1, Node *in2 ) : CmpLNode(in1,in2) {}
  virtual int    Opcode() const;
  // Since it is not consumed by Bools, it is not really a Cmp.
  virtual const CmpNode *is_Cmp() const { return 0; }
  virtual uint ideal_reg() const { return Op_RegI; }
};

//------------------------------CmpFNode---------------------------------------
// Compare 2 float values, returning condition codes (-1, 0 or 1).
// This implements the Java bytecode fcmpl, so unordered returns -1.
// Operands may not commute.
class CmpFNode : public CmpNode {
public:
  CmpFNode( Node *in1, Node *in2 ) : CmpNode(in1,in2) {}
  virtual int Opcode() const;
  virtual const Type *sub( const Type *, const Type * ) const { ShouldNotReachHere(); return NULL; }
  const Type *Value( PhaseTransform *phase ) const;
};

//------------------------------CmpF3Node--------------------------------------
// Compare 2 float values, returning integer value (-1, 0 or 1).
// This implements the Java bytecode fcmpl, so unordered returns -1.
// Operands may not commute.
class CmpF3Node : public CmpFNode {
public:
  CmpF3Node( Node *in1, Node *in2 ) : CmpFNode(in1,in2) {}
  virtual int Opcode() const;
  // Since it is not consumed by Bools, it is not really a Cmp.
  virtual const CmpNode *is_Cmp() const { return 0; }
  virtual uint ideal_reg() const { return Op_RegI; }
};


//------------------------------CmpDNode---------------------------------------
// Compare 2 double values, returning condition codes (-1, 0 or 1).
// This implements the Java bytecode dcmpl, so unordered returns -1.
// Operands may not commute.
class CmpDNode : public CmpNode {
public:
  CmpDNode( Node *in1, Node *in2 ) : CmpNode(in1,in2) {}
  virtual int Opcode() const;
  virtual const Type *sub( const Type *, const Type * ) const { ShouldNotReachHere(); return NULL; }
  const Type *Value( PhaseTransform *phase ) const;
  virtual Node  *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------CmpD3Node--------------------------------------
// Compare 2 double values, returning integer value (-1, 0 or 1).
// This implements the Java bytecode dcmpl, so unordered returns -1.
// Operands may not commute.
class CmpD3Node : public CmpDNode {
public:
  CmpD3Node( Node *in1, Node *in2 ) : CmpDNode(in1,in2) {}
  virtual int Opcode() const;
  // Since it is not consumed by Bools, it is not really a Cmp.
  virtual const CmpNode *is_Cmp() const { return 0; }
  virtual uint ideal_reg() const { return Op_RegI; }
};


//------------------------------BoolTest---------------------------------------
// Convert condition codes to a boolean test value (0 or -1).
// We pick the values as 3 bits; the low order 2 bits we compare against the
// condition codes, the high bit flips the sense of the result.
struct BoolTest VALUE_OBJ_CLASS_SPEC {
  enum mask { eq = 0, ne = 4, le = 5, ge = 7, lt = 3, gt = 1};
  mask _test;
  BoolTest( mask btm ) : _test(btm) {}
  const Type *cc2logical( const Type *CC ) const;
  // Commute the test.  I use a small table lookup.  The table is created as
  // a simple char array where each element is the ASCII version of a 'mask'
  // enum from above.
  mask commute( ) const { return mask("03X147X5"[_test]-'0'); }
  mask negate( ) const { return mask(_test^4); }
  bool is_canonical( ) const { return (_test == BoolTest::ne || _test == BoolTest::lt || _test == BoolTest::le); }
#ifndef PRODUCT
  void dump() const;
#endif
};

//------------------------------BoolNode---------------------------------------
// A Node to convert a Condition Codes to a Logical result.
class BoolNode : public Node {
  virtual uint hash() const;
  virtual uint cmp( const Node &n ) const;
  virtual uint size_of() const;
public:
  const BoolTest _test;
  BoolNode( Node *cc, BoolTest::mask t): _test(t), Node(0,cc) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual const Type *bottom_type() const { return TypeInt::BOOL; }
  uint match_edge(uint idx) const { return 0; }
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual BoolNode *is_Bool() { return this; }

  bool is_counted_loop_exit_test();
#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};

//------------------------------AbsNode----------------------------------------
// Abstract class for absolute value.  Mostly used to get a handy wrapper
// for finding this pattern in the graph.
class AbsNode : public Node {
public:
  AbsNode( Node *value ) : Node(0,value) {}
  // Check for the absolute-value graph
  static Node *is_absolute( PhaseGVN *phase, Node *phi_root );
};

//------------------------------AbsINode---------------------------------------
// Absolute value an integer.  Since a naive graph involves control flow, we
// "match" it in the ideal world (so the control flow can be removed).
class AbsINode : public AbsNode {
public:
  AbsINode( Node *in1 ) : AbsNode(in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
};

//------------------------------AbsFNode---------------------------------------
// Absolute value a float, a common float-point idiom with a cheap hardware
// implemention on most chips.  Since a naive graph involves control flow, we
// "match" it in the ideal world (so the control flow can be removed).
class AbsFNode : public AbsNode {
public:
  AbsFNode( Node *in1 ) : AbsNode(in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
};

//------------------------------AbsDNode---------------------------------------
// Absolute value a double, a common float-point idiom with a cheap hardware
// implemention on most chips.  Since a naive graph involves control flow, we
// "match" it in the ideal world (so the control flow can be removed).
class AbsDNode : public AbsNode {
public:
  AbsDNode( Node *in1 ) : AbsNode(in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};


//------------------------------CmpLTMaskNode----------------------------------
// If p < q, return -1 else return 0.  Nice for flow-free idioms.
class CmpLTMaskNode : public Node {
public:
  CmpLTMaskNode( Node *p, Node *q ) : Node(0, p, q) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
};


//------------------------------NegNode----------------------------------------
class NegNode : public Node {
public:
  NegNode( Node *in1 ) : Node(0,in1) {}
};

//------------------------------NegFNode---------------------------------------
// Negate value a float.  Negating 0.0 returns -0.0, but subtracting from
// zero returns +0.0 (per JVM spec on 'fneg' bytecode).  As subtraction
// cannot be used to replace negation we have to implement negation as ideal
// node; note that negation and addition can replace subtraction.
class NegFNode : public NegNode {
public:
  NegFNode( Node *in1 ) : NegNode(in1) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
};

//------------------------------NegDNode---------------------------------------
// Negate value a double.  Negating 0.0 returns -0.0, but subtracting from
// zero returns +0.0 (per JVM spec on 'dneg' bytecode).  As subtraction
// cannot be used to replace negation we have to implement negation as ideal
// node; note that negation and addition can replace subtraction.
class NegDNode : public NegNode {
public:
  NegDNode( Node *in1 ) : NegNode(in1) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};

//------------------------------CosDNode---------------------------------------
// Cosinus of a double
class CosDNode : public Node {
public:
  CosDNode( Node *in1  ) : Node(0, in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};

//------------------------------CosDNode---------------------------------------
// Sinus of a double
class SinDNode : public Node {
public:
  SinDNode( Node *in1  ) : Node(0, in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};


//------------------------------TanDNode---------------------------------------
// tangens of a double
class TanDNode : public Node {
public:
  TanDNode(Node *c, Node *in1  ) : Node(c, in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};


//------------------------------AtanDNode--------------------------------------
// arcus tangens of a double
class AtanDNode : public Node {
public:
  AtanDNode(Node *c, Node *in1, Node *in2  ) : Node(c, in1, in2) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
};


//------------------------------SqrtDNode--------------------------------------
// square root a double
class SqrtDNode : public Node {
public:
  SqrtDNode(Node *c, Node *in1  ) : Node(c, in1) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
  virtual const Type *Value( PhaseTransform *phase ) const;
};

//------------------------------PowDNode---------------------------------------
// Raise a double to a double power
class PowDNode : public Node {
public:
  PowDNode(Node *c, Node *in1, Node *in2  ) : Node(c, in1, in2) {}
  virtual int Opcode() const;
  const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
  virtual const Type *Value( PhaseTransform *phase ) const;
};

