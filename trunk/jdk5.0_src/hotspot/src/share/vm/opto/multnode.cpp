#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)multnode.cpp	1.49 03/12/23 16:42:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_multnode.cpp.incl"

//=============================================================================
//------------------------------MultiNode--------------------------------------
MultiNode *MultiNode::is_Multi() { return this; }

const RegMask &MultiNode::out_RegMask() const { 
  return RegMask::Empty;
}

Node *MultiNode::match( const ProjNode *proj, const Matcher *m ) { return proj->clone(); }

//------------------------------proj_out---------------------------------------
// Get a named projection
ProjNode* MultiNode::proj_out(uint which_proj) const {
  assert(Opcode() != Op_If || which_proj == true || which_proj == false, "must be 1 or 0");
  assert(Opcode() != Op_If || outcnt() == 2, "bad if #1");
  for( DUIterator_Fast imax, i = fast_outs(imax); true; i++ ) {
    Node *p = fast_out(i);      // Throw out-of-bounds if proj not found
    assert( p->is_Proj(), "only projections here" );
    ProjNode *proj = (ProjNode*)p;
    if( proj->_con == which_proj ) {
      assert(Opcode() != Op_If || proj->Opcode() == (which_proj?Op_IfTrue:Op_IfFalse), "bad if #2");
      return proj;
    }
  }
}

//=============================================================================
//------------------------------ProjNode---------------------------------------
uint ProjNode::hash() const { 
  // only one input
  return (uintptr_t)in(TypeFunc::Control) + _con; 
}
uint ProjNode::cmp( const Node &n ) const { return _con == ((ProjNode&)n)._con; }
uint ProjNode::size_of() const { return sizeof(ProjNode); }

ProjNode *ProjNode::is_Proj() { return this; }

// Test if we propagate interesting control along this projection
bool ProjNode::is_CFG() const {
  Node *def = in(0);
  return (_con == TypeFunc::Control && def->is_CFG());
}

const Type *ProjNode::bottom_type() const { 
  const Type *tb = in(0)->bottom_type();
  if( tb == Type::TOP ) return Type::TOP;
  if( tb == Type::BOTTOM ) return Type::BOTTOM;
  const TypeTuple *t = tb->is_tuple();
  return t->field_at(_con); 
}

const TypePtr *ProjNode::adr_type() const {
  if (bottom_type() == Type::MEMORY) {
    // in(0) might be a narrow MemBar; otherwise we will report TypePtr::BOTTOM
    const TypePtr* adr_type = in(0)->adr_type();
    #ifdef ASSERT
    if (!is_error_reported() && !Node::in_dump())
      assert(adr_type != NULL, "source must have adr_type");
    #endif
    return adr_type;
  }
  assert(bottom_type()->base() != Type::Memory, "no other memories?");
  return NULL;
}

int ProjNode::pinned() const { return in(0)->pinned(); }
#ifndef PRODUCT
void ProjNode::dump_spec() const { tty->print("#%d",_con); }
#endif

//----------------------------check_con----------------------------------------
void ProjNode::check_con() const {
  Node* n = in(0);
  if (n == NULL)       return;  // should be assert, but NodeHash makes bogons
  if (n->is_Mach())    return;  // mach. projs. are not type-safe
  if (n->is_Start())   return;  // alas, starts can have mach. projs. also
  if (_con == SCMemProjNode::SCMEMPROJCON ) return;
  const Type* t = n->bottom_type();
  if (t == Type::TOP)  return;  // multi is dead
  assert(_con < t->is_tuple()->cnt(), "ProjNode::_con must be in range");
}

//------------------------------Value------------------------------------------
const Type *ProjNode::Value( PhaseTransform *phase ) const {
  if( !in(0) ) return Type::TOP;
  const Type *t = phase->type(in(0));
  if( t == Type::TOP ) return t;
  if( t == Type::BOTTOM ) return t;
  return t->is_tuple()->field_at(_con);
}

//------------------------------out_RegMask------------------------------------
// Pass the buck uphill
const RegMask &ProjNode::out_RegMask() const {
  return RegMask::Empty;
}

//------------------------------ideal_reg--------------------------------------
uint ProjNode::ideal_reg() const { 
  return Matcher::base2reg[bottom_type()->base()];
}


