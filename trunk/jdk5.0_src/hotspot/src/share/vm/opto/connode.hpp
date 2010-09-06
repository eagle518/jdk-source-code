#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)connode.hpp	1.144 04/03/31 18:13:02 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class PhaseTransform;
class MachNode;
class OptoRegPair;

//------------------------------ConNode----------------------------------------
// Simple constants
class ConNode : public TypeNode {
public:
  ConNode( const Type *t ) : TypeNode(t,1) { set_req(0,(Node*)Compile::current()->root());}
  virtual int  Opcode() const;
  virtual uint hash() const;
  virtual uint is_Con() const { return 1; }
  virtual const RegMask &out_RegMask() const { return RegMask::Empty; }
  virtual const RegMask &in_RegMask(uint) const { return RegMask::Empty; }

  // Polymorphic factory method:
  static ConNode* make( const Type *t );
};

//------------------------------ConINode---------------------------------------
// Simple integer constants
class ConINode : public ConNode {
public:
  ConINode( const TypeInt *t ) : ConNode(t) {}
  virtual int Opcode() const;
  virtual int get_int( jint *con ) const { *con = _type->is_int()->get_con(); return true; }

  // Factory method:
  static ConINode* make( int con );
};

//------------------------------ConPNode---------------------------------------
// Simple pointer constants
class ConPNode : public ConNode {
public:
  ConPNode( const TypePtr *t ) : ConNode(t) {}
  virtual int Opcode() const;

  // Factory methods:
  static ConPNode* make( address con );
  static ConPNode* make( ciObject* con );
};


//------------------------------ConLNode---------------------------------------
// Simple long constants
class ConLNode : public ConNode {
public:
  ConLNode( const TypeLong *t ) : ConNode(t) {}
  virtual int Opcode() const;

  // Factory method:
  static ConLNode* make( jlong con );
};

//------------------------------ConFNode---------------------------------------
// Simple float constants
class ConFNode : public ConNode {
public:
  ConFNode( const TypeF *t ) : ConNode(t) {}
  virtual int Opcode() const;

  // Factory method:
  static ConFNode* make( float con );
};

//------------------------------ConDNode---------------------------------------
// Simple double constants
class ConDNode : public ConNode {
public:
  ConDNode( const TypeD *t ) : ConNode(t) {}
  virtual int Opcode() const;

  // Factory method:
  static ConDNode* make( double con );
};

//------------------------------BinaryNode-------------------------------------
// Place holder for the 2 conditional inputs to a CMove.  CMove needs 4
// inputs: the Bool (for the lt/gt/eq/ne bits), the flags (result of some
// compare), and the 2 values to select between.  The Matcher requires a
// binary tree so we break it down like this: 
//     (CMove (Binary bol cmp) (Binary src1 src2))
class BinaryNode : public Node {
public:
  BinaryNode( Node *n1, Node *n2 ) : Node(0,n1,n2) { }
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return 0; }
};

//------------------------------CMoveNode--------------------------------------
// Conditional move
class CMoveNode : public TypeNode {
public:
  enum { Control,               // When is it safe to do this cmove?
         Condition,             // Condition controlling the cmove
         IfFalse,               // Value if condition is false
         IfTrue };              // Value if condition is true
  CMoveNode( Node *bol, Node *left, Node *right, const Type *t ) : TypeNode(t,4) { set_req(Control,NULL); set_req(Condition,bol); set_req(IfFalse,left); set_req(IfTrue,right); }
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  static CMoveNode *make( Node *c, Node *bol, Node *left, Node *right, const Type *t );
  virtual CMoveNode *is_CMove() { return this; }
  // Helper function to spot cmove graph shapes
  static Node *is_cmove_id( PhaseTransform *phase, Node *cmp, Node *t, Node *f, BoolNode *b );
};

//------------------------------CMoveDNode-------------------------------------
class CMoveDNode : public CMoveNode {
public:
  CMoveDNode( Node *bol, Node *left, Node *right, const Type* t) : CMoveNode(bol,left,right,t){}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------CMoveFNode-------------------------------------
class CMoveFNode : public CMoveNode {
public:
  CMoveFNode( Node *bol, Node *left, Node *right, const Type* t ) : CMoveNode(bol,left,right,t) {}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------CMoveINode-------------------------------------
class CMoveINode : public CMoveNode {
public:
  CMoveINode( Node *bol, Node *left, Node *right, const TypeInt *ti ) : CMoveNode(bol,left,right,ti){}
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
};

//------------------------------CMoveLNode-------------------------------------
class CMoveLNode : public CMoveNode {
public:
  CMoveLNode(Node *bol, Node *left, Node *right, const TypeLong *tl ) : CMoveNode(bol,left,right,tl){}
  virtual int Opcode() const;
};

//------------------------------CMovePNode-------------------------------------
class CMovePNode : public CMoveNode {
public:
  CMovePNode( Node *c, Node *bol, Node *left, Node *right, const TypePtr* t ) : CMoveNode(bol,left,right,t) { set_req(Control,c); }
  virtual int Opcode() const;
};

//------------------------------CastPPNode-------------------------------------
// cast pointer to pointer (different type)
class CastPPNode: public TypeNode {
public:
  CastPPNode (Node *n, const Type *t ): TypeNode(t,2) { set_req(0, NULL); set_req(1, n); }
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegP; }
  virtual Node *Ideal_DU_postCCP( PhaseCCP * );
};

//------------------------------CheckCastPPNode--------------------------------
// for _checkcast, cast pointer to pointer (different type), without JOIN,
class CheckCastPPNode: public TypeNode {
public:
  CheckCastPPNode( Node *c, Node *n, const Type *t )
    : TypeNode(t,2)
  { set_req(0, c); set_req(1, n); }
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual int   Opcode() const;
  virtual uint  ideal_reg() const { return Op_RegP; }
  // No longer remove CheckCast after CCP as it gives me a place to hang
  // the proper address type - which is required to compute anti-deps.
  //virtual Node *Ideal_DU_postCCP( PhaseCCP * );
};

//------------------------------Conv2BNode-------------------------------------
// Convert int/pointer to a Boolean.  Map zero to zero, all else to 1.
class Conv2BNode : public Node {
public:
  Conv2BNode( Node *i ) : Node(0,i) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::BOOL; }
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint  ideal_reg() const { return Op_RegI; }
};

// The conversions operations are all Alpha sorted.  Please keep it that way!
//------------------------------ConvD2FNode------------------------------------
// Convert double to float
class ConvD2FNode : public Node {
public:
  ConvD2FNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual uint  ideal_reg() const { return Op_RegF; }
};

//------------------------------ConvD2INode------------------------------------
// Convert Double to Integer
class ConvD2INode : public Node {
public:
  ConvD2INode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint  ideal_reg() const { return Op_RegI; }
};

//------------------------------ConvD2LNode------------------------------------
// Convert Double to Long
class ConvD2LNode : public Node {
public:
  ConvD2LNode( Node *dbl ) : Node(0,dbl) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint ideal_reg() const { return Op_RegL; }
};

//------------------------------ConvF2DNode------------------------------------
// Convert Float to a Double.
class ConvF2DNode : public Node {
public:
  ConvF2DNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint  ideal_reg() const { return Op_RegD; }
};

//------------------------------ConvF2INode------------------------------------
// Convert float to integer
class ConvF2INode : public Node {
public:
  ConvF2INode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint  ideal_reg() const { return Op_RegI; }
};

//------------------------------ConvF2LNode------------------------------------
// Convert float to long
class ConvF2LNode : public Node {
public:
  ConvF2LNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint  ideal_reg() const { return Op_RegL; }
};

//------------------------------ConvI2DNode------------------------------------
// Convert Integer to Double
class ConvI2DNode : public Node {
public:
  ConvI2DNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint  ideal_reg() const { return Op_RegD; }
};

//------------------------------ConvI2FNode------------------------------------
// Convert Integer to Float
class ConvI2FNode : public Node {
public:
  ConvI2FNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Identity( PhaseTransform *phase );
  virtual uint  ideal_reg() const { return Op_RegF; }
};

//------------------------------ConvI2LNode------------------------------------
// Convert integer to long
class ConvI2LNode : public TypeNode {
public:
  ConvI2LNode(Node *in1, const TypeLong* t = TypeLong::INT)
    : TypeNode(t, 2)
  { set_req(1, in1); }
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint  ideal_reg() const { return Op_RegL; }
};

//------------------------------ConvL2DNode------------------------------------
// Convert Long to Double
class ConvL2DNode : public Node {
public:
  ConvL2DNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint ideal_reg() const { return Op_RegD; }
};

//------------------------------ConvL2FNode------------------------------------
// Convert Long to Float
class ConvL2FNode : public Node {
public:
  ConvL2FNode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual uint  ideal_reg() const { return Op_RegF; }
};

//------------------------------ConvL2INode------------------------------------
// Convert long to integer
class ConvL2INode : public Node {
public:
  ConvL2INode( Node *in1 ) : Node(0,in1) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint  ideal_reg() const { return Op_RegI; }
};

//------------------------------CastL2PNode-------------------------------------
class CastL2PNode : public Node {
public:
  CastL2PNode( Node *n ) : Node(NULL, n) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual Node *Identity( PhaseTransform *phase );
  virtual uint ideal_reg() const { return Op_RegP; }
  virtual const Type *bottom_type() const { return TypeRawPtr::BOTTOM; }
};

//------------------------------CastP2INode-------------------------------------
// Only usable in 32-bit land.  Allows Unsafe to do pointer math.
// Allows card-marks.
class CastP2INode : public Node {
public:
  CastP2INode( Node *ctrl, Node *n ) : Node(ctrl, n) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  // Return false to keep node from moving away from an associated card mark.
  virtual bool depends_only_on_test() const { return false; }
};

//------------------------------CastP2LNode-------------------------------------
// Used in both 32-bit and 64-bit land.  In 32-bit-land it's only used by 
// the unsafe ops to allow Unsafe code to do pointer math.  
class CastP2LNode : public Node {
public:
  CastP2LNode( Node *ctrl, Node *n ) : Node(ctrl, n) {}
  virtual int Opcode() const;
  virtual const Type *Value( PhaseTransform *phase ) const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual Node *Identity( PhaseTransform *phase );
  virtual uint ideal_reg() const { return Op_RegL; }
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  // Return false to keep node from moving away from an associated card mark.
  virtual bool depends_only_on_test() const { return false; }
};

//------------------------------MemMoveNode------------------------------------
// Memory to memory move.  Inserted very late, after allocation.
class MemMoveNode : public Node {
public:
  MemMoveNode( Node *dst, Node *src ) : Node(0,dst,src) {}
  virtual int Opcode() const;
};

//------------------------------ThreadLocalNode--------------------------------
// Ideal Node which returns the base of ThreadLocalStorage.
class ThreadLocalNode : public Node {
public:
  ThreadLocalNode( ) : Node((Node*)Compile::current()->root()) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeRawPtr::BOTTOM;}
  virtual uint ideal_reg() const { return Op_RegP; }
};

//------------------------------BoxNode----------------------------------------
// Take the address of a value.  Since values have no location, the "box" 
// requires its input to be stack-based.  It returns the stack address.  Spill
// code will "load" the box if needed.  Since stack locations are treated as 
// unaliased values, the user of this Node needs to make sure the boxed value
// remains alive past the last use of the box.
class BoxNode : public Node {
public:
  BoxNode( Node *value ) : Node( 0, value ) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeRawPtr::BOTTOM; }
  virtual uint ideal_reg() const { return Op_RegP; }
};

//------------------------------LoadPCNode-------------------------------------
class LoadPCNode: public Node {
public:
  LoadPCNode(Node *c) : Node(c) { }
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegP; }
};


//------------------------------LoadReturnPCNode-------------------------------
class LoadReturnPCNode: public Node {
public:
  LoadReturnPCNode(Node *c) : Node(c) { }
  virtual int Opcode() const;
  virtual uint ideal_reg() const { return Op_RegP; }
};


//-----------------------------RoundFloatNode----------------------------------
class RoundFloatNode: public Node {
public:
  RoundFloatNode(Node* c, Node *in1): Node(c, in1) {}
  virtual int   Opcode() const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint  ideal_reg() const { return Op_RegF; }
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
};


//-----------------------------RoundDoubleNode---------------------------------
class RoundDoubleNode: public Node {
public:
  RoundDoubleNode(Node* c, Node *in1): Node(c, in1) {}
  virtual int   Opcode() const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint  ideal_reg() const { return Op_RegD; }
  virtual Node *Identity( PhaseTransform *phase );
  virtual const Type *Value( PhaseTransform *phase ) const;
};

//------------------------------Opaque1Node------------------------------------
// A node to prevent unwanted optimizations.  Allows constant folding.
// Stops value-numbering, Ideal calls or Identity functions.
class Opaque1Node : public Node {
  virtual uint hash() const ;                  // { return NO_HASH; }
  virtual uint cmp( const Node &n ) const;
public:
  Opaque1Node( Node *n ) : Node(0,n) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual Node *Identity( PhaseTransform *phase );
};

//------------------------------Opaque2Node------------------------------------
// A node to prevent unwanted optimizations.  Allows constant folding.  Stops
// value-numbering, most Ideal calls or Identity functions.  This Node is
// specifically designed to prevent the pre-increment value of a loop trip
// counter from being live out of the bottom of the loop (hence causing the
// pre- and post-increment values both being live and thus requiring an extra
// temp register and an extra move).  If we "accidentally" optimize through
// this kind of a Node, we'll get slightly pessimal, but correct, code.  Thus
// it's OK to be slightly sloppy on optimizations here.
class Opaque2Node : public Node {
  virtual uint hash() const ;                  // { return NO_HASH; }
  virtual uint cmp( const Node &n ) const;
public:
  Opaque2Node( Node *n ) : Node(0,n) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
};

//----------------------PartialSubtypeCheckNode--------------------------------
// The 2nd slow-half of a subtype check.  Scan the subklass's 2ndary superklass
// array for an instance of the superklass.  Set a hidden internal cache on a
// hit (cache is checked with exposed code in gen_subtype_check()).  Return
// not zero for a miss or zero for a hit.  
class PartialSubtypeCheckNode : public Node {
public:
  PartialSubtypeCheckNode( Node *sub, Node *super ) : Node(0,sub,super) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
};

// 
class MoveI2FNode : public Node {
 public:
  MoveI2FNode( Node *value ) : Node(0,value) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::FLOAT; }
  virtual uint ideal_reg() const { return Op_RegF; }
  virtual const Type* Value( PhaseTransform *phase ) const;
};

class MoveL2DNode : public Node {
 public:
  MoveL2DNode( Node *value ) : Node(0,value) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return Type::DOUBLE; }
  virtual uint ideal_reg() const { return Op_RegD; }
  virtual const Type* Value( PhaseTransform *phase ) const;
};

class MoveF2INode : public Node {
 public:
  MoveF2INode( Node *value ) : Node(0,value) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeInt::INT; }
  virtual uint ideal_reg() const { return Op_RegI; }
  virtual const Type* Value( PhaseTransform *phase ) const;
};

class MoveD2LNode : public Node {
 public:
  MoveD2LNode( Node *value ) : Node(0,value) {}
  virtual int Opcode() const;
  virtual const Type *bottom_type() const { return TypeLong::LONG; }
  virtual uint ideal_reg() const { return Op_RegL; }
  virtual const Type* Value( PhaseTransform *phase ) const;
};
