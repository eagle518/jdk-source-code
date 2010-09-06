#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)connode.cpp	1.200 04/06/07 16:02:30 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Optimization - Graph Style

#include "incls/_precompiled.incl"
#include "incls/_connode.cpp.incl"

//=============================================================================
//------------------------------hash-------------------------------------------
uint ConNode::hash() const {
  return (uintptr_t)in(TypeFunc::Control) + _type->hash();
}

//------------------------------make-------------------------------------------
ConNode *ConNode::make( const Type *t ) {
  switch( t->basic_type() ) {
  case T_INT:       return new (1) ConINode( t->is_int() );
  case T_ARRAY:     return new (1) ConPNode( t->is_aryptr() );
  case T_LONG:      return new (1) ConLNode( t->is_long() );
  case T_FLOAT:     return new (1) ConFNode( t->is_float_constant() );
  case T_DOUBLE:    return new (1) ConDNode( t->is_double_constant() );
  case T_VOID:      return new (1) ConNode ( Type::TOP );
  case T_OBJECT:    return new (1) ConPNode( t->is_oopptr() );
  case T_ADDRESS:   return new (1) ConPNode( t->is_ptr() );
    // Expected cases:  TypePtr::NULL_PTR, any is_rawptr()
    // Also seen: AnyPtr(TopPTR *+top); from command line:
    //   r -XX:+PrintOpto -XX:CIStart=285 -XX:+CompileTheWorld -XX:CompileTheWorldStartAt=660
    // %%%% Stop using TypePtr::NULL_PTR to represent nulls:  use either TypeRawPtr::NULL_PTR
    // or else TypeOopPtr::NULL_PTR.  Then set Type::_basic_type[AnyPtr] = T_ILLEGAL
  }
  ShouldNotReachHere();
  return NULL;
}

ConINode* ConINode::make( int con ) {
  return new (1) ConINode( TypeInt::make(con) );
}
ConPNode* ConPNode::make( address con ) {
  if (con == NULL)
    return new (1) ConPNode( TypePtr::NULL_PTR ) ;
  return new (1) ConPNode( TypeRawPtr::make(con) );
}
ConPNode* ConPNode::make( ciObject* con ) {
  return new (1) ConPNode( TypeOopPtr::make_from_constant(con) );
}
ConLNode* ConLNode::make( jlong con ) {
  return new (1) ConLNode( TypeLong::make(con) );
}
ConFNode* ConFNode::make( float con ) {
  return new (1) ConFNode( TypeF::make(con) );
}
ConDNode* ConDNode::make( double con ) {
  return new (1) ConDNode( TypeD::make(con) );
}


//=============================================================================
/*
The major change is for CMoveP and StrComp.  They have related but slightly
different problems.  They both take in TWO oops which are both null-checked
independently before the using Node.  After CCP removes the CastPP's they need
to pick up the guarding test edge - in this case TWO control edges.  I tried
various solutions, all have problems:

(1) Do nothing.  This leads to a bug where we hoist a Load from a CMoveP or a
StrComp above a guarding null check.  I've seen both cases in normal -Xcomp
testing.

(2) Plug the control edge from 1 of the 2 oops in.  Apparent problem here is
to figure out which test post-dominates.  The real problem is that it doesn't
matter which one you pick.  After you pick up, the dominating-test elider in
IGVN can remove the test and allow you to hoist up to the dominating test on
the choosen oop bypassing the test on the not-choosen oop.  Seen in testing.
Oops.

(3) Leave the CastPP's in.  This makes the graph more accurate in some sense;
we get to keep around the knowledge that an oop is not-null after some test.
Alas, the CastPP's interfere with GVN (some values are the regular oop, some
are the CastPP of the oop, all merge at Phi's which cannot collapse, etc).
This cost us 10% on SpecJVM, even when I removed some of the more trivial
cases in the optimizer.  Removing more useless Phi's started allowing Loads to 
illegally float above null checks.  I gave up on this approach.

(4) Add BOTH control edges to both tests.  Alas, too much code knows that
control edges are in slot-zero ONLY.  Many quick asserts fail; no way to do
this one.  Note that I really want to allow the CMoveP to float and add both
control edges to the dependent Load op - meaning I can select early but I
cannot Load until I pass both tests.

(5) Do not hoist CMoveP and StrComp.  To this end I added the v-call
depends_only_on_test().  No obvious performance loss on Spec, but we are
clearly conservative on CMoveP (also so on StrComp but that's unlikely to
matter ever).

*/


//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  
// Move constants to the right.
Node *CMoveNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if( in(0) && remove_dead_region(phase, can_reshape) ) return this;
  if( phase->eqv(in(Condition), this) || 
      phase->eqv(in(IfFalse), this) || 
      phase->eqv(in(IfTrue), this) ) {
    return phase->C->top(); // dead loop
  }
  if( phase->type(in(Condition)) == Type::TOP )
    return NULL; // return NULL when Condition is dead

  if( in(IfFalse)->is_Con() && !in(IfTrue)->is_Con() ) {
    BoolNode *b = in(Condition)->is_Bool();
    if( b ) {
      Node *b2 = phase->transform(new (2) BoolNode( b->in(1), b->_test.negate() ));
      return make( in(Control), b2, in(IfTrue), in(IfFalse), _type );
    }
  }
  return NULL;
}

//------------------------------is_cmove_id------------------------------------
// Helper function to check for CMOVE identity.  Shared with PhiNode::Identity
Node *CMoveNode::is_cmove_id( PhaseTransform *phase, Node *cmp, Node *t, Node *f, BoolNode *b ) {
  // Check for Cmp'ing and CMove'ing same values
  if( (phase->eqv(cmp->in(1),f) &&
       phase->eqv(cmp->in(2),t)) ||
      // Swapped Cmp is OK
      (phase->eqv(cmp->in(2),f) &&
       phase->eqv(cmp->in(1),t)) ) {
    // Check for "(t==f)?t:f;" and replace with "f"
    if( b->_test._test == BoolTest::eq ) 
      return f;
    // Allow the inverted case as well
    // Check for "(t!=f)?t:f;" and replace with "t"
    if( b->_test._test == BoolTest::ne )
      return t;
  }
  return NULL;
}

//------------------------------Identity---------------------------------------
// Conditional-move is an identity if both inputs are the same, or the test
// true or false.
Node *CMoveNode::Identity( PhaseTransform *phase ) {
  if( phase->eqv(in(IfFalse),in(IfTrue)) ) // C-moving identical inputs?
    return in(IfFalse);         // Then it doesn't matter
  if( phase->type(in(Condition)) == TypeInt::ZERO ) 
    return in(IfFalse);         // Always pick left(false) input
  if( phase->type(in(Condition)) == TypeInt::ONE ) 
    return in(IfTrue);          // Always pick right(true) input

  // Check for CMove'ing a constant after comparing against the constant.
  // Happens all the time now, since if we compare equality vs a constant in
  // the parser, we "know" the variable is constant on one path and we force
  // it.  Thus code like "if( x==0 ) {/*EMPTY*/}" ends up inserting a
  // conditional move: "x = (x==0)?0:x;".  Yucko.  This fix is slightly more
  // general in that we don't need constants.
  BoolNode *b = in(Condition)->is_Bool();
  if( b ) {
    Node *cmp = b->in(1);
    if( cmp->is_Cmp() ) {
      Node *id = is_cmove_id( phase, cmp, in(IfTrue), in(IfFalse), b );
      if( id ) return id;
    }
  }

  return this;
}

//------------------------------Value------------------------------------------
// Result is the meet of inputs
const Type *CMoveNode::Value( PhaseTransform *phase ) const {
  if( phase->type(in(Condition)) == Type::TOP )
    return Type::TOP;
  return phase->type(in(IfFalse))->meet(phase->type(in(IfTrue)));
}

//------------------------------make-------------------------------------------
// Make a correctly-flavored CMove.  Since _type is directly determined
// from the inputs we do not need to specify it here.
CMoveNode *CMoveNode::make( Node *c, Node *bol, Node *left, Node *right, const Type *t ) {
  switch( t->basic_type() ) {
  case T_INT:       return new CMoveINode( bol, left, right, t->is_int() );
  case T_FLOAT:     return new CMoveFNode( bol, left, right, t );
  case T_DOUBLE:    return new CMoveDNode( bol, left, right, t );
  case T_LONG:      return new CMoveLNode( bol, left, right, t->is_long() );
  case T_OBJECT:    return new CMovePNode( c, bol, left, right, t->is_oopptr() );
  case T_ADDRESS:   return new CMovePNode( c, bol, left, right, t->is_ptr() );
  default:
    ShouldNotReachHere();
    return NULL;
  }
}

//=============================================================================
//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  
// Check for conversions to boolean
Node *CMoveINode::Ideal(PhaseGVN *phase, bool can_reshape) {
  // Try generic ideal's first
  Node *x = CMoveNode::Ideal(phase, can_reshape);
  if( x ) return x;

  // If zero is on the left (false-case, no-move-case) it must mean another
  // constant is on the right (otherwise the shared CMove::Ideal code would
  // have moved the constant to the right).  This situation is bad for Intel
  // and a don't-care for Sparc.  It's bad for Intel because the zero has to
  // be manifested in a register with a XOR which kills flags, which are live
  // on input to the CMoveI, leading to a situation which causes excessive
  // spilling on Intel.  For Sparc, if the zero in on the left the Sparc will
  // zero a register via G0 and conditionally-move the other constant.  If the
  // zero is on the right, the Sparc will load the first constant with a
  // 13-bit set-lo and conditionally move G0.  See bug 4677505.
  if( phase->type(in(IfFalse)) == TypeInt::ZERO && !(phase->type(in(IfTrue)) == TypeInt::ZERO) ) {
    BoolNode *b = in(Condition)->is_Bool();
    if( b ) {
      Node *b2 = phase->transform(new (2) BoolNode( b->in(1), b->_test.negate() ));
      return make( in(Control), b2, in(IfTrue), in(IfFalse), _type );
    }
  }

  // Now check for booleans
  int flip = 0;

  // Check for picking from zero/one
  if( phase->type(in(IfFalse)) == TypeInt::ZERO && phase->type(in(IfTrue)) == TypeInt::ONE ) {
    flip = 1 - flip;
  } else if( phase->type(in(IfFalse)) == TypeInt::ONE && phase->type(in(IfTrue)) == TypeInt::ZERO ) {
  } else return NULL;

  // Check for eq/ne test
  BoolNode *bol = in(1)->is_Bool();
  if( !bol ) return NULL;
  if( bol->_test._test == BoolTest::eq ) {
  } else if( bol->_test._test == BoolTest::ne ) {
    flip = 1-flip;
  } else return NULL;

  // Check for vs 0 or 1
  const CmpNode *cmp = bol->in(1)->is_Cmp();
  if( !cmp ) return NULL;
  if( phase->type(cmp->in(2)) == TypeInt::ZERO ) {
  } else if( phase->type(cmp->in(2)) == TypeInt::ONE ) {
    // Allow cmp-vs-1 if the other input is bounded by 0-1
    if( phase->type(cmp->in(1)) != TypeInt::BOOL )
      return NULL;
    flip = 1 - flip;
  } else return NULL;
  
  // Convert to a bool (flipped)
  // Build int->bool conversion
#ifndef PRODUCT
  if( PrintOpto ) tty->print_cr("CMOV to I2B");
#endif
  Node *n = new (2) Conv2BNode( cmp->in(1) );
  if( flip ) 
    n = new (3) XorINode( phase->transform(n), phase->intcon(1) );

  return n;
}

//=============================================================================
//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  
// Check for absolute value
Node *CMoveFNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  // Try generic ideal's first
  Node *x = CMoveNode::Ideal(phase, can_reshape);
  if( x ) return x;

  // Abs or -Abs?  
  // Might be an issue with -0.0 vs +0.0
  int flip = 0;

  // Find the Bool
  BoolNode *bol = in(1)->is_Bool();
  if( !bol ) return NULL;
  // Check bool sense
  switch( bol->_test._test ) {
  case BoolTest::eq:
  case BoolTest::ne:
    return NULL;
  case BoolTest::gt:
  case BoolTest::ge:
    flip = 1-flip;
  case BoolTest::lt:
  case BoolTest::le:
    break;
  }

  // Find CmpF vs zero
  Node *cmpf = bol->in(1);
  if( cmpf->Opcode() != Op_CmpF ) return NULL;
  if( phase->type(cmpf->in(2)) != TypeF::ZERO ) return NULL;
  Node *X = cmpf->in(1);

  // Find CmpF vs zero
  Node *neg;
  if( X == in(IfFalse) ) {
    neg = in(IfTrue);
  } else if( X == in(IfTrue) ) {
    flip = 1-flip;
    neg = in(IfFalse);
  } else return NULL;

  if( neg->Opcode() != Op_NegF ) return NULL;
  if( neg->in(1) != X ) return NULL;

  Node *abs = new (2) AbsFNode( X );
  if( flip )
    abs = new (2) NegFNode( phase->transform(abs) );

  return abs;
}

//=============================================================================
//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  
// Check for absolute value
Node *CMoveDNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  // Try generic ideal's first
  Node *x = CMoveNode::Ideal(phase, can_reshape);
  if( x ) return x;

  // Abs or -Abs?
  // Might be an issue with -0.0 vs +0.0
  int flip = 0;

  // Find the Bool
  BoolNode *bol = in(1)->is_Bool();
  if( !bol ) return NULL;
  // Check bool sense
  switch( bol->_test._test ) {
  case BoolTest::eq:
  case BoolTest::ne:
    return NULL;
  case BoolTest::gt:
  case BoolTest::ge:
    flip = 1-flip;
  case BoolTest::lt:
  case BoolTest::le:
    break;
  }

  // Find CmpD vs zero
  Node *cmpd = bol->in(1);
  if( cmpd->Opcode() != Op_CmpD ) return NULL;
  if( phase->type(cmpd->in(2)) != TypeD::ZERO ) return NULL;
  Node *X = cmpd->in(1);

  // Find CmpD vs zero
  Node *neg;
  if( X == in(IfFalse) ) {
    neg = in(IfTrue);
  } else if( X == in(IfTrue) ) {
    flip = 1-flip;
    neg = in(IfFalse);
  } else return NULL;

  if( neg->Opcode() != Op_NegD ) return NULL;
  if( neg->in(1) != X ) return NULL;

  Node *abs = new (2) AbsDNode( X );
  if( flip )
    abs = new (2) NegDNode( phase->transform(abs) );

  return abs;
}

//=============================================================================
// If input is already higher or equal to cast type, then this is an identity.
Node *CastPPNode::Identity( PhaseTransform *phase ) {
  return phase->type(in(1))->higher_equal(_type) ? in(1) : this;
}

//------------------------------Value------------------------------------------
// Take 'join' of input and cast-up type
const Type *CastPPNode::Value( PhaseTransform *phase ) const {
  if( in(0) && phase->type(in(0)) == Type::TOP ) return Type::TOP;
  if( phase->type(in(1)) == TypePtr::NULL_PTR &&
      _type->isa_ptr() && _type->is_ptr()->_ptr == TypePtr::NotNull )
    return Type::TOP;
  return phase->type(in(1))->join(_type);
}

//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  Strip out 
// control copies
Node *CastPPNode::Ideal(PhaseGVN *phase, bool can_reshape){
  return (in(0) && remove_dead_region(phase, can_reshape)) ? this : NULL; 
}

//------------------------------Ideal_DU_postCCP-------------------------------
// If not converting int->oop, throw away cast after constant propagation
Node *CastPPNode::Ideal_DU_postCCP( PhaseCCP *ccp ) {
  const Type *t = ccp->type(in(1));
  if( t->isa_oop_ptr() ) {
    ccp->hash_delete(this);
    set_type(t);                   // Turn into ID function
    ccp->hash_insert(this);
    return this;
  }
  return NULL;                     // No progress
}



//=============================================================================
//------------------------------Identity---------------------------------------
// If input is already higher or equal to cast type, then this is an identity.
Node *CheckCastPPNode::Identity( PhaseTransform *phase ) {
  // Toned down to rescue meeting at a Phi 3 different oops all implementing
  // the same interface.  CompileTheWorld starting at 502, kd12rc1.zip.
  return (phase->type(in(1)) == phase->type(this)) ? in(1) : this;
}

//------------------------------Value------------------------------------------
// Take 'join' of input and cast-up type, unless working with an Interface
const Type *CheckCastPPNode::Value( PhaseTransform *phase ) const {
  if( in(0) && phase->type(in(0)) == Type::TOP ) return Type::TOP;

  const Type *inn = phase->type(in(1));
  if( inn == Type::TOP ) return Type::TOP;  // No information yet

  const TypePtr *in_type   = inn->isa_ptr();
  const TypePtr *my_type   = _type->isa_ptr();
  if( in_type != NULL && my_type != NULL ) {
    TypePtr::PTR   in_ptr    = in_type->ptr();
    if( in_ptr == TypePtr::Null ) {
      return in_type;
    } else if( in_ptr == TypePtr::Constant ) {
      // Casting a constant oop to an interface?
      // (i.e., a String to a Comparable?)
      // Then return the interface.
      const TypeOopPtr *jptr = my_type->isa_oopptr();
      assert( jptr, "" );
      return (jptr->klass()->is_interface() || !in_type->higher_equal(_type))
        ? my_type->cast_to_ptr_type( TypePtr::NotNull )
        : in_type;
    } else {
      return my_type->cast_to_ptr_type( my_type->join_ptr(in_ptr) );
    }
  }
  return _type;

  // JOIN NOT DONE HERE BECAUSE OF INTERFACE ISSUES.
  // FIX THIS (DO THE JOIN) WHEN UNION TYPES APPEAR!

  // 
  // Remove this code after overnight run indicates no performance
  // loss from not performing JOIN at CheckCastPPNode
  // 
  // const TypeInstPtr *in_oop = in->isa_instptr();
  // const TypeInstPtr *my_oop = _type->isa_instptr();
  // // If either input is an 'interface', return destination type
  // assert (in_oop == NULL || in_oop->klass() != NULL, "");
  // assert (my_oop == NULL || my_oop->klass() != NULL, "");
  // if( (in_oop && in_oop->klass()->klass_part()->is_interface())
  //   ||(my_oop && my_oop->klass()->klass_part()->is_interface()) ) {
  //   TypePtr::PTR  in_ptr = in->isa_ptr() ? in->is_ptr()->_ptr : TypePtr::BotPTR;
  //   // Preserve cast away nullness for interfaces
  //   if( in_ptr == TypePtr::NotNull && my_oop && my_oop->_ptr == TypePtr::BotPTR ) {
  //     return my_oop->cast_to_ptr_type(TypePtr::NotNull);
  //   }
  //   return _type;
  // }
  // 
  // // Neither the input nor the destination type is an interface, 
  // 
  // // history: JOIN used to cause weird corner case bugs
  // //          return (in == TypeOopPtr::NULL_PTR) ? in : _type;
  // // JOIN picks up NotNull in common instance-of/check-cast idioms, both oops.
  // // JOIN does not preserve NotNull in other cases, e.g. RawPtr vs InstPtr
  // const Type *join = in->join(_type);
  // // Check if join preserved NotNull'ness for pointers
  // if( join->isa_ptr() && _type->isa_ptr() ) {
  //   TypePtr::PTR join_ptr = join->is_ptr()->_ptr;
  //   TypePtr::PTR type_ptr = _type->is_ptr()->_ptr;
  //   // If there isn't any NotNull'ness to preserve
  //   // OR if join preserved NotNull'ness then return it
  //   if( type_ptr == TypePtr::BotPTR  || type_ptr == TypePtr::Null ||
  //       join_ptr == TypePtr::NotNull || join_ptr == TypePtr::Constant ) {
  //     return join;
  //   }
  //   // ELSE return same old type as before 
  //   return _type;
  // }
  // // Not joining two pointers
  // return join;
}

//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  Strip out 
// control copies
Node *CheckCastPPNode::Ideal(PhaseGVN *phase, bool can_reshape){
  return (in(0) && remove_dead_region(phase, can_reshape)) ? this : NULL; 
}

//=============================================================================
//------------------------------Identity---------------------------------------
Node *Conv2BNode::Identity( PhaseTransform *phase ) {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return in(1);
  if( t == TypeInt::ZERO ) return in(1);
  if( t == TypeInt::ONE ) return in(1);
  if( t == TypeInt::BOOL ) return in(1);
  return this;
}

//------------------------------Value------------------------------------------
const Type *Conv2BNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  if( t == TypeInt::ZERO ) return TypeInt::ZERO;
  if( t == TypePtr::NULL_PTR ) return TypeInt::ZERO;
  const TypePtr *tp = t->isa_ptr();
  if( tp != NULL ) {
    if( tp->ptr() == TypePtr::AnyNull ) return Type::TOP;
    if( tp->ptr() == TypePtr::Constant) return TypeInt::ONE;
    if (tp->ptr() == TypePtr::NotNull)  return TypeInt::ONE;
    return TypeInt::BOOL;
  }
  if (t->base() != Type::Int) return TypeInt::BOOL;
  const TypeInt *ti = t->is_int();
  if( ti->_hi < 0 || ti->_lo > 0 ) return TypeInt::ONE;
  return TypeInt::BOOL;
}


// The conversions operations are all Alpha sorted.  Please keep it that way!
//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvD2FNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  if( t == Type::DOUBLE ) return Type::FLOAT;
  const TypeD *td = t->is_double_constant();
  return TypeF::make( (float)td->getd() );
}

//------------------------------Identity---------------------------------------
// Float's can be converted to doubles with no loss of bits.  Hence
// converting a float to a double and back to a float is a NOP.
Node *ConvD2FNode::Identity(PhaseTransform *phase) {
  return (in(1)->Opcode() == Op_ConvF2D) ? in(1)->in(1) : this;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvD2INode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  if( t == Type::DOUBLE ) return TypeInt::INT;
  const TypeD *td = t->is_double_constant();
  return TypeInt::make( SharedRuntime::d2i( td->getd() ) );
}

//------------------------------Ideal------------------------------------------
// If converting to an int type, skip any rounding nodes
Node *ConvD2INode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if( in(1)->Opcode() == Op_RoundDouble )
    set_req(1,in(1)->in(1));
  return NULL;
}

//------------------------------Identity---------------------------------------
// Int's can be converted to doubles with no loss of bits.  Hence
// converting an integer to a double and back to an integer is a NOP.
Node *ConvD2INode::Identity(PhaseTransform *phase) {
  return (in(1)->Opcode() == Op_ConvI2D) ? in(1)->in(1) : this;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvD2LNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  if( t == Type::DOUBLE ) return TypeLong::LONG;
  const TypeD *td = t->is_double_constant();
  return TypeLong::make( SharedRuntime::d2l( td->getd() ) );
}

//------------------------------Identity---------------------------------------
Node *ConvD2LNode::Identity(PhaseTransform *phase) {
  // Remove ConvD2L->ConvL2D->ConvD2L sequences.
  if( in(1)       ->Opcode() == Op_ConvL2D &&
      in(1)->in(1)->Opcode() == Op_ConvD2L )
    return in(1)->in(1);
  return this;
}

//------------------------------Ideal------------------------------------------
// If converting to an int type, skip any rounding nodes
Node *ConvD2LNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if( in(1)->Opcode() == Op_RoundDouble )
    set_req(1,in(1)->in(1));
  return NULL;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvF2DNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  if( t == Type::FLOAT ) return Type::DOUBLE;
  const TypeF *tf = t->is_float_constant();
#ifndef IA64
  return TypeD::make( (double)tf->getf() );
#else
  float x = tf->getf();
  return TypeD::make( (x == 0.0f) ? (double)x : (double)x + ia64_double_zero );
#endif
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvF2INode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP )       return Type::TOP;
  if( t == Type::FLOAT ) return TypeInt::INT;
  const TypeF *tf = t->is_float_constant();
  return TypeInt::make( SharedRuntime::f2i( tf->getf() ) );
}

//------------------------------Identity---------------------------------------
Node *ConvF2INode::Identity(PhaseTransform *phase) {
  // Remove ConvF2I->ConvI2F->ConvF2I sequences.
  if( in(1)       ->Opcode() == Op_ConvI2F &&
      in(1)->in(1)->Opcode() == Op_ConvF2I )
    return in(1)->in(1);
  return this;
}

//------------------------------Ideal------------------------------------------
// If converting to an int type, skip any rounding nodes
Node *ConvF2INode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if( in(1)->Opcode() == Op_RoundFloat )
    set_req(1,in(1)->in(1));
  return NULL;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvF2LNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP )       return Type::TOP;
  if( t == Type::FLOAT ) return TypeLong::LONG;
  const TypeF *tf = t->is_float_constant();
  return TypeLong::make( SharedRuntime::f2l( tf->getf() ) );
}

//------------------------------Identity---------------------------------------
Node *ConvF2LNode::Identity(PhaseTransform *phase) {
  // Remove ConvF2L->ConvL2F->ConvF2L sequences.
  if( in(1)       ->Opcode() == Op_ConvL2F &&
      in(1)->in(1)->Opcode() == Op_ConvF2L )
    return in(1)->in(1);
  return this;
}

//------------------------------Ideal------------------------------------------
// If converting to an int type, skip any rounding nodes
Node *ConvF2LNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if( in(1)->Opcode() == Op_RoundFloat )
    set_req(1,in(1)->in(1));
  return NULL;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvI2DNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeInt *ti = t->is_int();
  if( ti->is_con() ) return TypeD::make( (double)ti->get_con() );
  return bottom_type();
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvI2FNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeInt *ti = t->is_int();
  if( ti->is_con() ) return TypeF::make( (float)ti->get_con() );
  return bottom_type();
}

//------------------------------Identity---------------------------------------
Node *ConvI2FNode::Identity(PhaseTransform *phase) {
  // Remove ConvI2F->ConvF2I->ConvI2F sequences.
  if( in(1)       ->Opcode() == Op_ConvF2I &&
      in(1)->in(1)->Opcode() == Op_ConvI2F )
    return in(1)->in(1);
  return this;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvI2LNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeInt *ti = t->is_int();
  const Type* tl = TypeLong::make(ti->_lo, ti->_hi);
  // Join my declared type against my incoming type.
  tl = tl->join(this->type());
  if (tl->empty())  return Type::TOP;
  return tl;
}

#ifdef _LP64
static inline bool long_ranges_overlap(jlong lo1, jlong hi1,
                                       jlong lo2, jlong hi2) {
  // Two ranges overlap iff one range's low point falls in the other range.
  return (lo2 <= lo1 && lo1 <= hi2) || (lo1 <= lo2 && lo2 <= hi1);
}
#endif

//------------------------------Ideal------------------------------------------
Node *ConvI2LNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  const TypeLong* this_type = this->type()->is_long();
  Node* this_changed = NULL;

  // If _major_progress, then more loop optimizations follow.  Do NOT
  // remove this node's type assertion until no more loop ops can happen.
  // The progress bit is set in the major loop optimizations THEN comes the
  // call to IterGVN and any chance of hitting this code.  Cf. Opaque1Node.
  if (!phase->C->major_progress()) {
    const TypeInt* in_type = phase->type(in(1))->isa_int();
    if (in_type != NULL && this_type != NULL &&
        (in_type->_lo != this_type->_lo ||
         in_type->_hi != this_type->_hi)) {
      // Although this WORSENS the type, it increases GVN opportunities,
      // because I2L nodes with the same input will common up, regardless
      // of slightly differing type assertions.  Such slight differences
      // arise routinely as a result of loop unrolling, so this is a
      // post-unrolling graph cleanup.  Choose a type which depends only
      // on my input.
      set_type(TypeLong::make(in_type->_lo, in_type->_hi));
      // Note: this_type still has old type value, for the logic below.
      this_changed = this;
    }
  }

#ifdef _LP64
  // Convert ConvI2L(AddI(x, y)) to AddL(ConvI2L(x), ConvI2L(y)) ,
  // but only if x and y have subranges that cannot cause 32-bit overflow,
  // under the assumption that x+y is in my own subrange this->type().

  // This assumption is based on a constraint (i.e., type assertion)
  // established in Parse::array_addressing or perhaps elsewhere.
  // This constraint has been adjoined to the "natural" type of
  // the incoming argument in(0).  We know (because of runtime
  // checks) - that the result value I2L(x+y) is in the joined range.
  // Hence we can restrict the incoming terms (x, y) to values such
  // that their sum also lands in that range.

  // This optimization is useful only on 64-bit systems, where we hope
  // the addition will end up subsumed in an addressing mode.
  // It is necessary to do this when optimizing an unrolled array
  // copy loop such as x[i++] = y[i++].

  // On 32-bit systems, it's better to perform as much 32-bit math as
  // possible before the I2L conversion, because 32-bit math is cheaper.
  // There's no common reason to "leak" a constant offset through the I2L.
  // Addressing arithmetic will not absorb it as part of a 64-bit AddL.

  Node* z = in(1);
  int op = z->Opcode();
  if (op == Op_AddI || op == Op_SubI) {
    Node* x = z->in(1);
    Node* y = z->in(2);
    if (x == z || y == z)             return this_changed;
    if (phase->type(x) == Type::TOP)  return this_changed;
    if (phase->type(y) == Type::TOP)  return this_changed;
    const TypeInt*  tx = phase->type(x)->is_int();
    const TypeInt*  ty = phase->type(y)->is_int();
    const TypeLong* tz = this_type;
    jlong xlo = tx->_lo;
    jlong xhi = tx->_hi;
    jlong ylo = ty->_lo;
    jlong yhi = ty->_hi;
    jlong zlo = tz->_lo;
    jlong zhi = tz->_hi;
    jlong vbit = CONST64(1) << BitsPerInt;
    if (op == Op_SubI) {
      jlong ylo0 = ylo;
      ylo = -yhi;
      yhi = -ylo0;
    }
    // See if x+y can cause positive overflow into z+2**32
    if (long_ranges_overlap(xlo+ylo, xhi+yhi, zlo+vbit, zhi+vbit)) {
      return this_changed;
    }
    // See if x+y can cause negative overflow into z-2**32
    if (long_ranges_overlap(xlo+ylo, xhi+yhi, zlo-vbit, zhi-vbit)) {
      return this_changed;
    }
    // Now it's always safe to assume x+y does not overflow.
    // This is true even if some pairs x,y might cause overflow, as long
    // as that overflow value cannot fall into [zlo,zhi].

    // Confident that the arithmetic is "as if infinite precision",
    // we can now use z's range to put constraints on those of x and y.
    // The "natural" range of x [xlo,xhi] can perhaps be narrowed to a
    // more "restricted" range by intersecting [xlo,xhi] with the
    // range obtained by subtracting y's range from the asserted range
    // of the I2L conversion.  Here's the interval arithmetic algebra:
    //    x == z-y == [zlo,zhi]-[ylo,yhi] == [zlo,zhi]+[-yhi,-ylo]
    //    => x in [zlo-yhi, zhi-ylo]
    //    => x in [zlo-yhi, zhi-ylo] INTERSECT [xlo,xhi]
    //    => x in [xlo MAX zlo-yhi, xhi MIN zhi-ylo]
    jlong rxlo = MAX2(xlo, zlo - yhi);
    jlong rxhi = MIN2(xhi, zhi - ylo);
    // And similarly, x changing place with y:
    jlong rylo = MAX2(ylo, zlo - xhi);
    jlong ryhi = MIN2(yhi, zhi - xlo);
    if (rxlo > rxhi || rylo > ryhi) {
      return this_changed;  // x or y is dying; don't mess w/ it
    }
    if (op == Op_SubI) {
      jlong rylo0 = rylo;
      rylo = -ryhi;
      ryhi = -rylo0;
    }

    Node* cx = phase->transform( new (2) ConvI2LNode(x, TypeLong::make(rxlo, rxhi)) );
    Node* cy = phase->transform( new (2) ConvI2LNode(y, TypeLong::make(rylo, ryhi)) );
    switch (op) {
    case Op_AddI:  return new (3) AddLNode(cx, cy);
    case Op_SubI:  return new (3) SubLNode(cx, cy);
    default:       ShouldNotReachHere();
    }
  }
#endif //_LP64

  return this_changed;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvL2DNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeLong *tl = t->is_long();
  if( tl->is_con() ) return TypeD::make( (double)tl->get_con() );
  return bottom_type();
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *ConvL2FNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeLong *tl = t->is_long();
  if( tl->is_con() ) return TypeF::make( (float)tl->get_con() );
  return bottom_type();
}

//=============================================================================
//----------------------------Identity-----------------------------------------
Node *ConvL2INode::Identity( PhaseTransform *phase ) {
  // Convert L2I(I2L(x)) => x
  if (in(1)->Opcode() == Op_ConvI2L)  return in(1)->in(1);
  return this;
}

//------------------------------Value------------------------------------------
const Type *ConvL2INode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeLong *tl = t->is_long();
  if( tl->is_con() ) return TypeInt::make( (jint)tl->get_con() );
  return bottom_type();
}

//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  
// Blow off prior masking to int
Node *ConvL2INode::Ideal(PhaseGVN *phase, bool can_reshape) {
  Node *andl = in(1);
  uint andl_op = andl->Opcode();
  if( andl_op == Op_AndL ) {
    // Blow off prior masking to int
    if( phase->type(andl->in(2)) == TypeLong::make( 0xFFFFFFFF ) ) {
      set_req(1,andl->in(1));
      return this;
    }
  }

  // Swap with a prior add: convL2I(addL(x,y)) ==> addI(convL2I(x),convL2I(y))
  // This replaces an 'AddL' with an 'AddI'.
  if( andl_op == Op_AddL ) {
    Node* x = andl->in(1);
    Node* y = andl->in(2);
    if( x == andl || y == andl ) { // Check for dead loop.
      return phase->C->top();
    }
    if (phase->type(x) == Type::TOP)  return NULL;
    if (phase->type(y) == Type::TOP)  return NULL;
    Node *add1 = phase->transform(new (2) ConvL2INode(x));
    Node *add2 = phase->transform(new (2) ConvL2INode(y));
    return new (3) AddINode(add1,add2);
  }

  // Fold up with a prior LoadL: LoadL->ConvL2I ==> LoadI
  // Requires we understand the 'endianess' of Longs.
  if( andl_op == Op_LoadL ) { 
    Node *adr = andl->in(MemNode::Address);
    // VM_LITTLE_ENDIAN is #defined appropriately in the Makefiles
#ifndef VM_LITTLE_ENDIAN
    // The transformation can cause problems on BIG_ENDIAN architectures
    // where the jint is not the same address as the jlong. Specifically, we
    // will fail to insert an anti-dependence in GCM between the LoadI and a
    // subsequent StoreL because different memory offsets provoke
    // flatten_alias_type() into indicating two different types.  See bug
    // 4755222.
    
    // Node *base = adr->is_AddP() ? adr->in(AddPNode::Base) : adr;
    // adr = phase->transform( new (4) AddPNode(base,adr,phase->MakeConX(sizeof(jint))));
    return NULL;
#else
    if (phase->C->alias_type(andl->adr_type())->is_volatile()) {
      // Picking up the low half by itself bypasses the atomic load and we could
      // end up with more than one non-atomic load.  See bugs 4432655 and 4526490.
      // We could go to the trouble of iterating over andl's output edges and
      // punting only if there's more than one real use, but we don't bother.
      return NULL;
    }
    return new (3) LoadINode(andl->in(MemNode::Control),andl->in(MemNode::Memory),adr,((LoadLNode*)andl)->raw_adr_type());
#endif
  }

  return NULL;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *CastL2PNode::Value( PhaseTransform *phase ) const {
  const Type* t = phase->type(in(1));
  if (t->base() == Type::Long && t->singleton()) {
    uintptr_t bits = (uintptr_t) t->is_long()->get_con();
    if (bits == 0)   return TypePtr::NULL_PTR;
    return TypeRawPtr::make((address) bits);
}
  return CastL2PNode::bottom_type();
}

//------------------------------Idealize---------------------------------------
static inline bool fits_in_int(const Type* t, bool but_not_min_int = false) {
  if (t == Type::TOP)  return false;
  const TypeLong *tl = t->is_long();
  jint lo = min_jint;
  jint hi = max_jint;
  if (but_not_min_int)  ++lo;  // caller wants to negate the value w/o overflow
  return (tl->_lo >= lo) && (tl->_hi <= hi);
}

static inline Node* addP_of_L2P(PhaseGVN *phase,
                                Node* base,
                                Node* disp,
                                bool negate = false) {
#ifdef _LP64
  Node* dispX = disp;
#else
  Node* dispX = new (2) ConvL2INode(disp);
#endif
  if (negate) {
    dispX = new (3) SubXNode(phase->intcon(0), phase->transform(dispX));
  }
  return new (4) AddPNode(phase->C->top(),
                          phase->transform(new (2) CastL2PNode(base)),
                          phase->transform(dispX));
}

Node *CastL2PNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  // convert CastL2P(AddL(x, y)) to AddP(CastL2P(x), y) if y fits in an int
  int op = in(1)->Opcode();
  Node* x;
  Node* y;
  switch (op) {
  case Op_SubL:
    x = in(1)->in(1);
    y = in(1)->in(2);
    if (fits_in_int(phase->type(y), true)) {
      return addP_of_L2P(phase, x, y, true);
    }
    break;
  case Op_AddL:
    x = in(1)->in(1);
    y = in(1)->in(2);
    if (fits_in_int(phase->type(y))) {
      return addP_of_L2P(phase, x, y);
    }
    if (fits_in_int(phase->type(x))) {
      return addP_of_L2P(phase, y, x);
    }
    break;
  }
  return NULL;
}

//------------------------------Identity---------------------------------------
Node *CastL2PNode::Identity( PhaseTransform *phase ) {
  if (in(1)->Opcode() == Op_CastP2L)  return in(1)->in(1);
  return this;
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *CastP2INode::Value( PhaseTransform *phase ) const {
  const Type* t = phase->type(in(1));
  if (t->base() == Type::RawPtr && t->singleton()) {
    uintptr_t bits = (uintptr_t) t->is_rawptr()->get_con();
    return TypeInt::make((juint)bits);
  }
  return CastP2INode::bottom_type();
}

Node *CastP2INode::Ideal(PhaseGVN *phase, bool can_reshape) {
  return (in(0) && remove_dead_region(phase, can_reshape)) ? this : NULL; 
}

//=============================================================================
//------------------------------Value------------------------------------------
const Type *CastP2LNode::Value( PhaseTransform *phase ) const {
  const Type* t = phase->type(in(1));
  if (t->base() == Type::RawPtr && t->singleton()) {
    uintptr_t bits = (uintptr_t) t->is_rawptr()->get_con();
    return TypeLong::make((julong)bits);
  }
  return CastP2LNode::bottom_type();
}

Node *CastP2LNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  return (in(0) && remove_dead_region(phase, can_reshape)) ? this : NULL; 
}

//------------------------------Identity---------------------------------------
Node *CastP2LNode::Identity( PhaseTransform *phase ) {
  if (in(1)->Opcode() == Op_CastL2P)  return in(1)->in(1);
  return this;
}


//=============================================================================
//------------------------------Identity---------------------------------------
// Remove redundant roundings
Node *RoundFloatNode::Identity( PhaseTransform *phase ) {
  assert(Matcher::strict_fp_requires_explicit_rounding, "should only generate for Intel");
  // Do not round constants
  if (phase->type(in(1))->base() == Type::FloatCon)  return in(1);
  int op = in(1)->Opcode();
  // Redundant rounding
  if( op == Op_RoundFloat ) return in(1);
  // Already rounded
  if( op == Op_Parm ) return in(1);
  if( op == Op_LoadF ) return in(1);
  return this;
}

//------------------------------Value------------------------------------------
const Type *RoundFloatNode::Value( PhaseTransform *phase ) const {
  return phase->type( in(1) );
}

//=============================================================================
//------------------------------Identity---------------------------------------
// Remove redundant roundings.  Incoming arguments are already rounded.
Node *RoundDoubleNode::Identity( PhaseTransform *phase ) {
  assert(Matcher::strict_fp_requires_explicit_rounding, "should only generate for Intel");
  // Do not round constants
  if (phase->type(in(1))->base() == Type::DoubleCon)  return in(1);
  int op = in(1)->Opcode();
  // Redundant rounding
  if( op == Op_RoundDouble ) return in(1);
  // Already rounded
  if( op == Op_Parm ) return in(1);
  if( op == Op_LoadD ) return in(1);
  if( op == Op_ConvF2D ) return in(1);
  if( op == Op_ConvI2D ) return in(1);
  return this;
}

//------------------------------Value------------------------------------------
const Type *RoundDoubleNode::Value( PhaseTransform *phase ) const {
  return phase->type( in(1) );
}


//=============================================================================
// Do not allow value-numbering
uint Opaque1Node::hash() const { return NO_HASH; }
uint Opaque1Node::cmp( const Node &n ) const { 
  return (&n == this);          // Always fail except on self
}

//------------------------------Identity---------------------------------------
// If _major_progress, then more loop optimizations follow.  Do NOT remove
// the opaque Node until no more loop ops can happen.  Note the timing of
// _major_progress; it's set in the major loop optimizations THEN comes the
// call to IterGVN and any chance of hitting this code.  Hence there's no
// phase-ordering problem with stripping Opaque1 in IGVN followed by some
// more loop optimizations that require it.
Node *Opaque1Node::Identity( PhaseTransform *phase ) {
  return phase->C->major_progress() ? this : in(1);
}

//=============================================================================
// A node to prevent unwanted optimizations.  Allows constant folding.  Stops
// value-numbering, most Ideal calls or Identity functions.  This Node is
// specifically designed to prevent the pre-increment value of a loop trip
// counter from being live out of the bottom of the loop (hence causing the
// pre- and post-increment values both being live and thus requiring an extra
// temp register and an extra move).  If we "accidentally" optimize through
// this kind of a Node, we'll get slightly pessimal, but correct, code.  Thus
// it's OK to be slightly sloppy on optimizations here.

// Do not allow value-numbering
uint Opaque2Node::hash() const { return NO_HASH; }
uint Opaque2Node::cmp( const Node &n ) const { 
  return (&n == this);          // Always fail except on self
}


//------------------------------Value------------------------------------------
const Type *MoveL2DNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeLong *tl = t->is_long();
  if( !tl->is_con() ) return bottom_type();
  JavaValue v;
  v.set_jlong(tl->get_con());
  return TypeD::make( v.get_jdouble() );
}

//------------------------------Value------------------------------------------
const Type *MoveI2FNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  const TypeInt *ti = t->is_int();
  if( !ti->is_con() )   return bottom_type();
  JavaValue v;
  v.set_jint(ti->get_con());
  return TypeF::make( v.get_jfloat() );
}

//------------------------------Value------------------------------------------
const Type *MoveF2INode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP )       return Type::TOP;
  if( t == Type::FLOAT ) return TypeInt::INT;
  const TypeF *tf = t->is_float_constant();
  JavaValue v;
  v.set_jfloat(tf->getf());
  return TypeInt::make( v.get_jint() );
}

//------------------------------Value------------------------------------------
const Type *MoveD2LNode::Value( PhaseTransform *phase ) const {
  const Type *t = phase->type( in(1) );
  if( t == Type::TOP ) return Type::TOP;
  if( t == Type::DOUBLE ) return TypeLong::LONG;
  const TypeD *td = t->is_double_constant();
  JavaValue v;
  v.set_jdouble(td->getd());
  return TypeLong::make( v.get_jlong() );
}
