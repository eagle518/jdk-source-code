#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cfgnode.cpp	1.237 04/05/04 13:26:18 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Portions of code courtesy of Clifford Click

// Optimization - Graph Style

#include "incls/_precompiled.incl"
#include "incls/_cfgnode.cpp.incl"

//=============================================================================
//------------------------------Value------------------------------------------
// Compute the type of the RegionNode.
const Type *RegionNode::Value( PhaseTransform *phase ) const {
  for( uint i=1; i<req(); ++i ) {       // For all paths in
    Node *n = in(i);            // Get Control source
    if( !n ) continue;          // Missing inputs are TOP
    if( phase->type(n) == Type::CONTROL )
      return Type::CONTROL;
  }
  return Type::TOP;             // All paths dead?  Then so are we
}

//------------------------------Identity---------------------------------------
// Check for Region being Identity.
Node *RegionNode::Identity( PhaseTransform *phase ) {
  // Cannot have Region be an identity, even if it has only 1 input.
  // Phi users cannot have their Region input folded away for them,
  // since they need to select the proper data input
  return this;
}

//------------------------------merge_region-----------------------------------
// If a Region flows into a Region, merge into one big happy merge.  This is
// hard to do if there is stuff that has to happen
static Node *merge_region(RegionNode *region, PhaseGVN *phase) {
  if( region->Opcode() != Op_Region ) // Do not do to LoopNodes
    return NULL;
  Node *progress = NULL;        // Progress flag
  PhaseIterGVN *igvn = phase->is_IterGVN();

  uint rreq = region->req();
  for( uint i = 1; i < rreq; i++ ) {
    Node *r = region->in(i);
    if( r && r->Opcode() == Op_Region && // Found a region?
        r->in(0) == r &&        // Not already collapsed?
        r != region &&          // Avoid stupid situations
        r->outcnt() == 2 ) {    // Self user and 'region' user only?
      assert(!r->is_Region()->has_phi(), "no phi users");
      if( !progress ) {         // No progress
        if (region->has_phi()) {
          return NULL;        // Only flatten if no Phi users
          // igvn->hash_delete( phi );
        }
        igvn->hash_delete( region );
        progress = region;      // Making progress
      }
      igvn->hash_delete( r );

      // Append inputs to 'r' onto 'region'
      for( uint j = 1; j < r->req(); j++ ) {
        // Move an input from 'r' to 'region'
        region->add_req(NULL);
        region->set_req(rreq, r->in(j));
        r->set_req(j, phase->C->top());
        // Update phis of 'region'
        //for( uint k = 0; k < max; k++ ) {
        //  Node *phi = region->out(k);
        //  if( phi->is_Phi() ) {
        //    phi->add_req(NULL);
        //    phi->set_req(rreq, phi->in(i));
        //  }
        //}

        rreq++;                 // One more input to Region
      } // Found a region to merge into Region
      // Clobber pointer to the now dead 'r'
      region->set_req(i, phase->C->top());
    }
  }

  return progress;
}



//--------------------------------has_phi--------------------------------------
// Helper function: Return any PhiNode that uses this region or NULL
PhiNode* RegionNode::has_phi() const {
  for (DUIterator_Fast imax, i = fast_outs(imax); i < imax; i++) {
    PhiNode* phi = fast_out(i)->is_Phi();
    if (phi) {   // Check for Phi users
      assert(phi->in(0) == (Node*)this, "phi uses region only via in(0)");
      return phi;  // this one is good enough
    }
  }

  return NULL;
}


//-----------------------------has_unique_phi----------------------------------
// Helper function: Return the only PhiNode that uses this region or NULL
PhiNode* RegionNode::has_unique_phi() const {
  // Check that only one use is a Phi
  PhiNode* only_phi = NULL;
  for (DUIterator_Fast imax, i = fast_outs(imax); i < imax; i++) {
    PhiNode* phi = fast_out(i)->is_Phi();
    if (phi) {   // Check for Phi users
      assert(phi->in(0) == (Node*)this, "phi uses region only via in(0)");
      if (only_phi == NULL) {
        only_phi = phi;
      } else {
        return NULL;  // multiple phis
      }
    }
  }

  return only_phi;
}


//------------------------------check_phi_clipping-----------------------------
// Helper function for RegionNode's identification of FP clipping
// Check inputs to the Phi
static bool check_phi_clipping( PhiNode *phi, ConNode * &min, uint &min_idx, ConNode * &max, uint &max_idx, Node * &val, uint &val_idx ) {
  min     = NULL;
  max     = NULL;
  val     = NULL;
  min_idx = 0;
  max_idx = 0;
  val_idx = 0;
  uint  phi_max = phi->req();
  if( phi_max == 4 ) {
    for( uint j = 1; j < phi_max; ++j ) {
      Node *n = phi->in(j);
      int opcode = n->Opcode();
      switch( opcode ) {
      case Op_ConI: 
        {
          if( min == NULL ) {
            min     = n->Opcode() == Op_ConI ? (ConNode*)n : NULL;
            min_idx = j;
          } else {
            max     = n->Opcode() == Op_ConI ? (ConNode*)n : NULL;
            max_idx = j;
            if( min->get_int() > max->get_int() ) {
              // Swap min and max
              ConNode *temp;
              uint     temp_idx;
              temp     = min;     min     = max;     max     = temp;
              temp_idx = min_idx; min_idx = max_idx; max_idx = temp_idx;
            }
          }
        }
        break;
      default:
        {
          val = n;
          val_idx = j;
        }
        break;
      }
    }
  }
  return ( min && max && val && (min->get_int() <= 0) && (max->get_int() >=0) );
}
  

//------------------------------check_if_clipping------------------------------
// Helper function for RegionNode's identification of FP clipping
// Check that inputs to Region come from two IfNodes, 
// 
//            If
//      False    True
//       If        |
//  False  True    |
//    |      |     |
//  RegionNode_inputs
// 
static bool check_if_clipping( const RegionNode *region, IfNode * &bot_if, IfNode * &top_if ) {
  top_if = NULL;
  bot_if = NULL;

  // Check control structure above RegionNode for (if  ( if  ) )
  ProjNode *in1 = region->in(1)->is_Proj();
  ProjNode *in2 = region->in(2)->is_Proj();
  ProjNode *in3 = region->in(3)->is_Proj();
  // Check that all inputs are projections
  if( in1 && in2 && in3 ) {
    IfNode *in10 = in1->in(0) ? in1->in(0)->is_If() : NULL;
    IfNode *in20 = in2->in(0) ? in2->in(0)->is_If() : NULL;
    IfNode *in30 = in3->in(0) ? in3->in(0)->is_If() : NULL;
    // Check that #1 and #2 are ifTrue and ifFalse from same If
    if( in10 && in20 && in30 && in10 == in20 && (in1->Opcode() != in2->Opcode()) ) {
      ProjNode *in100  = in10->in(0)  ? in10->in(0)->is_Proj() : NULL;
      IfNode   *in1000 = in100 && in100->in(0) ? in100->in(0)->is_If() : NULL;
      // Check that control for in10 comes from other branch of IF from in3
      if( in30 == in1000 && (in3->Opcode() != in100->Opcode()) ) {
        // Control pattern checks
        top_if = in1000;
        bot_if = in10;
      }
    }
  }

  return (top_if != NULL);
}


//------------------------------check_convf2i_clipping-------------------------
// Helper function for RegionNode's identification of FP clipping
// Verify that the value input to the phi comes from "ConvF2I; LShift; RShift"
static bool check_convf2i_clipping( PhiNode *phi, uint idx, ConvF2INode * &convf2i, Node *min, Node *max) {
  convf2i = NULL;

  // Check for the RShiftNode
  Node *rshift = phi->in(idx);
  assert( rshift, "Previous checks ensure phi input is present");
  if( rshift->Opcode() != Op_RShiftI )  { return false; }

  // Check for the LShiftNode
  Node *lshift = rshift->in(1);
  assert( lshift, "Previous checks ensure phi input is present");
  if( lshift->Opcode() != Op_LShiftI )  { return false; }

  // Check for the ConvF2INode
  Node *conv = lshift->in(1);
  if( conv->Opcode() != Op_ConvF2I ) { return false; }

  // Check that shift amounts are only to get sign bits set after F2I
  jint max_cutoff     = max->get_int();
  jint min_cutoff     = min->get_int();
  jint left_shift     = lshift->in(2)->get_int();
  jint right_shift    = rshift->in(2)->get_int();
  jint max_post_shift = nth_bit(BitsPerJavaInteger - left_shift - 1);
  if( left_shift != right_shift || 
      0 > left_shift || left_shift >= BitsPerJavaInteger ||
      max_post_shift < max_cutoff ||
      max_post_shift < -min_cutoff ) {
    // Shifts are necessary but current transformation eliminates them
    return false;
  }

  // OK to return the result of ConvF2I without shifting
  convf2i = (ConvF2INode*)conv;
  return true;
}


//------------------------------check_compare_clipping-------------------------
// Helper function for RegionNode's identification of FP clipping
static bool check_compare_clipping( bool less_than, IfNode *iff, ConNode *limit, Node * & input ) {
  BoolNode *bool1 = iff->in(1)->is_Bool();
  if ( bool1 == NULL ) { return false; }
  if(       less_than && bool1->_test._test != BoolTest::le ) { return false; }
  else if( !less_than && bool1->_test._test != BoolTest::lt ) { return false; }
  const Node *cmpF = bool1->in(1);
  if( cmpF->Opcode() != Op_CmpF )      { return false; }
  // Test that the float value being compared against
  // is equivalent to the int value used as a limit
  Node *nodef = cmpF->in(2);
  if( nodef->Opcode() != Op_ConF ) { return false; }
  jfloat conf = nodef->getf();
  jint   coni = limit->get_int();
  if( ((int)conf) != coni )        { return false; }
  input = cmpF->in(1);
  return true;
}

//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  Must preserve
// the CFG, but we can still strip out dead paths.
Node *RegionNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  if( !in(0) ) return NULL;     // Already degraded to a Copy
  assert(!in(0)->is_Root(), "not a specially hidden merge");

  // Check for RegionNode with no Phi users and both inputs come from either
  // arm of the same IF.  If found, then the control-flow split is useless.
  if (can_reshape) {            // Need DU info to check for Phi users
    if (!has_phi()) {           // No Phi users?  Nothing merging?
      for (uint i = 1; i < req()-1; i++) {
        Node *if1 = in(i);
        if( !if1 ) continue;
        Node *iff = if1->in(0);
        if( !iff || !iff->is_If() ) continue;
        for( uint j=i+1; j<req(); j++ ) {
          if( in(j) && in(j)->in(0) == iff &&
              if1->Opcode() != in(j)->Opcode() ) {
            // Add the IF Projections to the worklist. They (and the IF itself)
            // will be eliminated if dead.
            phase->is_IterGVN()->add_users_to_worklist(iff);
            set_req(i, iff->in(0));// Skip around the useless IF diamond
            set_req(j, NULL);
            return this;      // Record progress
          }
        }
      }
    }
  }

  // Remove TOP or NULL input paths. If only 1 input path remains, this Region 
  // degrades to a copy.
  int cnt = 0;                  // Count of values merging
  // For all inputs...
  for( uint i=1; i<req(); ++i ){// For all paths in
    Node *n = in(i);            // Get the input
    if( n ) {
      const RegionNode *region = n->is_Region();
      // Remove useless control copy inputs
      if( region && region->is_copy() ) {
        set_req(i, n->nonnull_req());
        i--;
        continue;
      }
      if( n->is_Proj() ) {      // Remove useless rethrows
        CallNode *call = n->in(0)->is_Call();
        if (call && call->entry_point() == OptoRuntime::rethrow_stub()) {
          set_req(i, call->in(0));
          i--;
          continue;
        }
      }
      if( phase->type(n) == Type::TOP ) {
        set_req(i, NULL);       // Ignore TOP inputs
        i--;
        continue;
      }
      cnt++;                    // One more value merging

    } else if (can_reshape) {   // Else found dead path with DU info
      PhaseIterGVN *igvn = phase->is_IterGVN();
      del_req(i);               // Yank path from self
      uint max = outcnt();
      DUIterator j;
      bool progress = true;
      while(progress) {         // Need to establish property over all users
        progress = false;
        for (j = outs(); has_out(j); j++) {
          Node *n = out(j);
          if( n->req() != req() && n->is_Phi() ) {
            assert( n->in(0) == this, "" );
            igvn->hash_delete(n); // Yank from hash before hacking edges
            n->set_req_X(i,NULL,igvn);// Correct DU info
            n->del_req(i);        // Yank path from Phis
            if( max != outcnt() ) {
              progress = true;
              j = refresh_out_pos(j);
              max = outcnt();
            }
          }
        }
      }
      igvn->add_users_to_worklist(this); // Revisit collapsed Phis
      i--;
    }
  }
  
  if( cnt <= 1 ) {              // Only 1 path in?  Become a copy!
    set_req(0, NULL);           // Null control input for region copy
    if( cnt == 0 ) {
      if( req() > 1 ) // No inputs at all?
        set_req(1, phase->C->top());  // Then copy from TOP control
    } else if (can_reshape) {
      PhaseIterGVN *igvn = phase->is_IterGVN();
      Node *parent_ctrl = in(1);
      assert(parent_ctrl != NULL, "Region is a copy of some non-null control");
      if( parent_ctrl == this ) // Closed, dead, loop?
        return phase->C->top();
      igvn->add_users_to_worklist(this); // Check for further allowed opts
      for (DUIterator_Last imin, i = last_outs(imin); i >= imin; --i) {
        Node* n = last_out(i);
        igvn->hash_delete(n); // Remove from worklist before modifying edges
        if( n->is_Phi() ) {   // Collapse all Phis to copies
          n->set_req(0, NULL);
        } 
        else if( n->is_Region() ) { // Update all incoming edges
          assert( n != this, "Must be removed from DefUse edges");
          for( uint k=1; k < n->req(); k++ ) 
            if( n->in(k) == this ) 
              n->set_req(k, parent_ctrl);
        }
        else {
          assert( n->in(0) == this, "Expect RegionNode to be control parent");
          n->set_req(0, parent_ctrl);
        }
#ifdef ASSERT
        for( uint k=0; k < n->req(); k++ ) {
          assert( n->in(k) != this, "All uses of RegionNode should be gone");
        }
#endif
      }
      // Remove the RegionNode itself from DefUse info
      igvn->remove_dead_node(this);
    }
    return this;                // Record progress
  }


  // If a Region flows into a Region, merge into one big happy merge.
  if (can_reshape) {
    Node *m = merge_region(this, phase);
    if (m != NULL)  return m;
  }

  // Check if this region is the root of a clipping idiom on floats
  if( ConvertFloat2IntClipping && can_reshape && req() == 4 ) {
    // Check that only one use is a Phi and that it simplifies to two constants +
    PhiNode* phi = has_unique_phi();
    if (phi != NULL) {          // One Phi user
      // Check inputs to the Phi
      ConNode *min;
      ConNode *max;
      Node    *val;
      uint     min_idx;
      uint     max_idx;
      uint     val_idx;
      if( check_phi_clipping( phi, min, min_idx, max, max_idx, val, val_idx )  ) {
        IfNode *top_if;
        IfNode *bot_if;
        if( check_if_clipping( this, bot_if, top_if ) ) {
          // Control pattern checks, now verify compares
          Node   *top_in = NULL;   // value being compared against
          Node   *bot_in = NULL;
          if( check_compare_clipping( true,  bot_if, min, bot_in ) && 
              check_compare_clipping( false, top_if, max, top_in ) ) {
            if( bot_in == top_in ) {
              PhaseIterGVN *gvn = phase->is_IterGVN();
              assert( gvn != NULL, "Only had DefUse info in IterGVN");
              // Only remaining check is that bot_in == top_in == (Phi's val + mods)

              // Check for the ConvF2INode
              ConvF2INode *convf2i;
              if( check_convf2i_clipping( phi, val_idx, convf2i, min, max ) && 
                convf2i->in(1) == bot_in ) {
                // Matched pattern, including LShiftI; RShiftI, replace with integer compares
                // max test
                Node *cmp   = gvn->register_new_node_with_optimizer(new (3) CmpINode( convf2i, min ));
                Node *boo   = gvn->register_new_node_with_optimizer(new (2) BoolNode( cmp, BoolTest::lt ));
                IfNode *iff = (IfNode*)gvn->register_new_node_with_optimizer(new (2) IfNode( top_if->in(0), boo, PROB_UNLIKELY_MAG(5), top_if->_fcnt ));
                Node *if_min= gvn->register_new_node_with_optimizer(new (1) IfTrueNode (iff));
                Node *ifF   = gvn->register_new_node_with_optimizer(new (1) IfFalseNode(iff));
                // min test
                cmp         = gvn->register_new_node_with_optimizer(new (3) CmpINode( convf2i, max ));
                boo         = gvn->register_new_node_with_optimizer(new (2) BoolNode( cmp, BoolTest::gt ));
                iff         = (IfNode*)gvn->register_new_node_with_optimizer(new (2) IfNode( ifF, boo, PROB_UNLIKELY_MAG(5), bot_if->_fcnt ));
                Node *if_max= gvn->register_new_node_with_optimizer(new (1) IfTrueNode (iff));
                ifF         = gvn->register_new_node_with_optimizer(new (1) IfFalseNode(iff));
                // update input edges to region node
                set_req_X( min_idx, if_min, gvn );
                set_req_X( max_idx, if_max, gvn );
                set_req_X( val_idx, ifF,    gvn );
                // remove unnecessary 'LShiftI; RShiftI' idiom
                gvn->hash_delete(phi);
                phi->set_req_X( val_idx, convf2i, gvn );
                gvn->hash_find_insert(phi);
                // Return transformed region node
                return this;
              }
            }
          }
        }
      }
    }
  }

  return NULL;
}



const RegMask &RegionNode::out_RegMask() const { 
  return RegMask::Empty;
}

// Find the one non-null required input.  RegionNode only
Node *Node::nonnull_req() const {
  assert( is_Region(), "" );
  for( uint i = 1; i < _cnt; i++ )
    if( in(i) )
      return in(i);
  ShouldNotReachHere();
  return NULL;
}


//=============================================================================
// note that these functions assume that the _adr_type field is flattened
uint PhiNode::hash() const {
  const Type* at = _adr_type;
  return TypeNode::hash() + (at ? at->hash() : 0);
}
uint PhiNode::cmp( const Node &n ) const {
  return TypeNode::cmp(n) && _adr_type == ((PhiNode&)n)._adr_type;
}
static inline
const TypePtr* flatten_phi_adr_type(const TypePtr* at) {
  if (at == NULL || at == TypePtr::BOTTOM)  return at;
  return Compile::current()->alias_type(at)->adr_type();
}

//----------------------------make---------------------------------------------
// create a new phi with edges matching r and set (initially) to x
PhiNode* PhiNode::make(Node* r, Node* x, const Type *t, const TypePtr* at) {
  uint preds = r->req();   // Number of predecessor paths
  assert(t != Type::MEMORY || at == flatten_phi_adr_type(at), "flatten at");
  PhiNode* p = new (preds) PhiNode(r, t, at);
  for (uint j = 1; j < preds; j++) {
    // Fill in all inputs, except those which the region does not yet have
    Node* ctl = r->in(j);
    p->set_req(j, ctl ? x : NULL);
  }
  return p;
}
PhiNode* PhiNode::make(Node* r, Node* x) {
  const Type*    t  = x->bottom_type();
  const TypePtr* at = NULL;
  if (t == Type::MEMORY)  at = flatten_phi_adr_type(x->adr_type());
  return make(r, x, t, at);
}
PhiNode* PhiNode::make_blank(Node* r, Node* x) {
  const Type*    t  = x->bottom_type();
  const TypePtr* at = NULL;
  if (t == Type::MEMORY)  at = flatten_phi_adr_type(x->adr_type());
  return new (r->req()) PhiNode(r, t, at);
}


//------------------------slice_memory-----------------------------------------
// create a new phi with narrowed memory type
PhiNode* PhiNode::slice_memory(const TypePtr* adr_type) const {
  PhiNode* mem = (PhiNode*) clone();
  *(const TypePtr**)&mem->_adr_type = adr_type;
  // convert self-loops, or else we get a bad graph
  for (uint i = 1; i < req(); i++) {
    if ((const Node*)in(i) == this)  mem->set_req(i, mem);
  }
  mem->verify_adr_type();
  return mem;
}


//------------------------degrade_to_copy--------------------------------------
void PhiNode::degrade_to_copy(Node* n) {
  set_req(Region, NULL);
  set_req(Input,  n);
  for (uint i = Input+1; i < req(); i++)  set_req(i, NULL);
  set_type(n->bottom_type());
}


//------------------------verify_adr_type--------------------------------------
#ifdef ASSERT
void PhiNode::verify_adr_type(VectorSet& visited, const TypePtr* at) const {
  if (visited.test_set(_idx))  return;  //already visited

  // recheck constructor invariants:
  verify_adr_type(false);

  // recheck local phi/phi consistency:
  assert(_adr_type == at || _adr_type == TypePtr::BOTTOM,
         "adr_type must be consistent across phi nest");

  // walk around
  for (uint i = 1; i < req(); i++) {
    Node* n = in(i);
    if (n == NULL)  continue;
    const PhiNode* np = in(i)->is_Phi();
    if (np != NULL) {
      np->verify_adr_type(visited, at);
    } else if (n->bottom_type() == Type::TOP
               || (n->is_Mem() && n->in(MemNode::Address)->bottom_type() == Type::TOP)) {
      // ignore top inputs
    } else {
      const TypePtr* nat = flatten_phi_adr_type(n->adr_type());
      // recheck phi/non-phi consistency at leaves:
      assert((nat != NULL) == (at != NULL), "");
      assert(nat == at || nat == TypePtr::BOTTOM,
             "adr_type must be consistent at leaves of phi nest");
    }
  }
}

// Verify a whole nest of phis rooted at this one.
void PhiNode::verify_adr_type(bool recursive) const {
  if (is_error_reported())  return;  // muzzle asserts when debugging an error
  if (Node::in_dump())      return;  // muzzle asserts when printing

  assert((_type == Type::MEMORY) == (_adr_type != NULL), "adr_type for memory phis only");

  if (!VerifyAliases)       return;  // verify thoroughly only if requested

  assert(_adr_type == flatten_phi_adr_type(_adr_type),
         "Phi::adr_type must be pre-normalized");

  if (recursive) {
    VectorSet visited(Thread::current()->resource_area());
    verify_adr_type(visited, _adr_type);
  }
}
#endif


//------------------------------Value------------------------------------------
// Compute the type of the PhiNode
const Type *PhiNode::Value( PhaseTransform *phase ) const {
  Node *r = in(0);              // RegionNode
  if( !r )                      // Copy or dead
    return in(1) ? phase->type(in(1)) : Type::TOP;

  // Note: During parsing, phis are often transformed before their regions.
  // This means we have to use type_or_null to defend against untyped regions.
  if( phase->type_or_null(r) == Type::TOP )  // Dead code?
    return Type::TOP;

  // Check for trip-counted loop.  If so, be smarter.
  CountedLoopNode *l = r->is_CountedLoop();
  if( l && l->can_be_counted_loop(phase) &&
      ((const Node*)l->phi() == this) ) { // Trip counted loop!
    // protect against init_trip() or limit() returning NULL
    const Node *init   = l->init_trip();
    const Node *limit  = l->limit();
    if( init != NULL && limit != NULL && l->stride_is_con() ) {
      const TypeInt *lo = init ->bottom_type()->isa_int();
      const TypeInt *hi = limit->bottom_type()->isa_int();
      if( lo && hi ) {            // Dying loops might have TOP here
        int stride = l->stride_con();
        if( stride < 0 ) {          // Down-counter loop
          const TypeInt *tmp = lo; lo = hi; hi = tmp;
          stride = -stride;
        }
        if( lo->_hi < hi->_lo )     // Reversed endpoints are well defined :-(
          return TypeInt::make(lo->_lo,hi->_hi,3);
      }
    }
  }

  // Until we have harmony between classes and interfaces in the type
  // lattice, we must tread carefully around phis which implicitly
  // convert the one to the other.
  const TypeInstPtr* ttip = _type->isa_instptr();
  bool is_intf = false;
  if (ttip != NULL) {
    ciKlass* k = ttip->klass();
    if (k->is_loaded() && k->is_interface())
      is_intf = true;
  }

  // Default case: merge all inputs
  const Type *t = Type::TOP;        // Merged type starting value
  for (uint i = 1; i < req(); ++i) {// For all paths in
    // Reachable control path?
    if (r->in(i) && phase->type(r->in(i)) == Type::CONTROL) {
      const Type* ti = phase->type(in(i));
      // We assume that each input of an interface-valued Phi is a true
      // subtype of that interface.  This might not be true of the meet
      // of all the input types.  The lattice is not distributive in
      // such cases.  Ward off asserts in type.cpp by refusing to do
      // meets between interfaces and proper classes.
      const TypeInstPtr* tiip = ti->isa_instptr();
      if (tiip) {
        bool ti_is_intf = false;
        ciKlass* k = tiip->klass();
        if (k->is_loaded() && k->is_interface())
          ti_is_intf = true;
        if (is_intf != ti_is_intf)
          { t = _type; break; }
      }
      t = t->meet(ti);
    }
  }
  // If we grew the result range, we also need to widen
  // In phases other than CCP, it's OK for the second argument (old_type)
  // to be null, because non-CCP phases ignore that argument.
  // In CCP, types are always non-null.
  t = phase->widen(t, phase->type_or_null(this)); // Widen once per Phi
  // The worst-case type (from ciTypeFlow) should be consistent with "t".
  // That is, we expect that "t->higher_equal(_type)" holds true.
  // There are various exceptions:
  // - Inputs which are phis might in fact be widened unnecessarily.
  //   For example, an input might be a widened int while the phi is a short.
  // - Inputs might be BotPtrs but this phi is dependent on a null check,
  //   and postCCP has removed the cast which encodes the result of the check.
  // - The type of this phi is an interface, and the inputs are classes.
  // - Value calls on inputs might produce fuzzy results.
  //   (Occurrences of this case suggest improvements to Value methods.)
  //
  // However, it is no longer possible to see pure bottom values as phi
  // inputs, due to the use of the ciTypeFlow pre-pass.
  const Type* jt = t->join(_type);// Worst case type
  if( jt->empty() ) {           // Emptied out???

    // Check for evil case of 't' being a class and '_type' expecting an
    // interface.  This can happen because the bytecodes do not contain
    // enough type info to distinguish a Java-level interface variable
    // from a Java-level object variable.  If we meet 2 classes which
    // both implement interface I, but their meet is at 'j/l/O' which
    // doesn't implement I, we have no way to tell if the result should
    // be 'I' or 'j/l/O'.  Thus we'll pick 'j/l/O'.  If this then flows
    // into a Phi which "knows" it's an Interface type we'll have to
    // uplift the type.
    if( !t->empty() && ttip && ttip->is_loaded() && ttip->klass()->is_interface() )
      return _type;             // Uplift to interface
    // Otherwise it's something stupid like non-overlapping int ranges
    // found on dying counted loops.
    return Type::TOP;           // Canonical empty value
  }

  // If we have an interface-typed Phi and we narrow to a class type, the join
  // should report back the class.  However, if we have a J/L/Object
  // class-typed Phi and an interface flows in, it's possible that the meet &
  // join report an interface back out.  This isn't possible but happens
  // because the type system doesn't interact well with interfaces.
  const TypeInstPtr *jtip = jt->isa_instptr();
  if( jtip ) {
    const TypeInstPtr *ttip = _type->is_instptr();
    if( jtip->is_loaded() &&  jtip->klass()->is_interface() && 
        ttip->is_loaded() && !ttip->klass()->is_interface() )
      // Happens in a CTW of rt.jar, 320-341, no extra flags
      return ttip->cast_to_ptr_type(jtip->ptr());
  }
  return jt;
}


//------------------------------is_diamond_phi---------------------------------
// Does this Phi represent a simple well-shaped diamond merge?  Return the
// index of the true path or 0 otherwise.
static int is_diamond_phi( Node *phi ) {
  // Check for a 2-path merge
  Node *region = phi->in(0);
  if( !region ) return 0;
  if( region->req() != 3 ) return 0;
  if( phi   ->req() != 3 ) return 0;
  // Check that both paths come from the same If
  Node *ifp1 = region->in(1);
  Node *ifp2 = region->in(2);
  if( !ifp1 || !ifp2 ) return 0;
  Node *iff = ifp1->in(0);
  if( !iff || !iff->is_If() ) return 0;
  if( iff != ifp2->in(0) ) return 0;
  // Check for a proper bool/cmp
  BoolNode *bool = iff->in(1)->is_Bool();
  if( !bool ) return 0;
  const CmpNode *cmp = bool->in(1)->is_Cmp();
  if( !cmp ) return 0;

  // Check for branching opposite expected
  if( ifp2->Opcode() == Op_IfTrue ) {
    assert( ifp1->Opcode() == Op_IfFalse, "" );
    return 2;
  } else {
    assert( ifp1->Opcode() == Op_IfTrue, "" );
    return 1;
  }
}

//------------------------------Identity---------------------------------------
// Check for Region being Identity.
Node *PhiNode::Identity( PhaseTransform *phase ) {
  Node *r = in(0);              // RegionNode
  if( !r ) return in(1);        // Already degraded to a Copy 

  // Check for a loop-phi merging same value on back edge, where backedge
  // value is itself a phi merging not-null and unknown versions of the
  // same loop-phi.
  if( r->is_Loop() && in(LoopNode::LoopBackControl)->is_Phi() ) {
    Node *phi = in(LoopNode::LoopBackControl);
    uint i;
    for( i = 1; i < phi->req(); i++ )
      if( phi->in(i) != this &&
          (phi->in(i)->Opcode() != Op_CastPP ||
           phi->in(i)->in(1) != phi) )
        break;
    if( i == phi->req() )
      return in(LoopNode::EntryControl);
  }

  // Check for CMove'ing a constant after comparing against the constant.
  // Happens all the time now, since if we compare equality vs a constant in
  // the parser, we "know" the variable is constant on one path and we force
  // it.  Thus code like "if( x==0 ) {/*EMPTY*/}" ends up inserting a
  // conditional move: "x = (x==0)?0:x;".  Yucko.  This fix is slightly more
  // general in that we don't need constants.  Since CMove's are only inserted
  // in very special circumstances, we do it here on generic Phi's.
  int true_path = is_diamond_phi(this);
  if( true_path != 0 ) {
    // phi->region->if_proj->ifnode->bool->cmp
    // All legal without further checks because "is_diamond_phi" has guarenteed
    // nice graph shape.
    Node *iff = r->in(1)->in(0);
    BoolNode *bool = (BoolNode*)iff->in(1);
    Node *cmp = bool->in(1);
    // Check for CMOVE identity
    Node *id = CMoveNode::is_cmove_id( phase, cmp, in(true_path), in(3-true_path), bool );
    if( id ) return id;
  }

  // Check for no merging going on
  Node *input = NULL;           // The one unique input
  // For all inputs...
  for( uint i=1; i<req(); ++i ){// For all paths in
    // Unreachable control path?
    Node *rc = r->in(i);
    if( !rc || phase->type(rc) == Type::TOP ) 
      continue;
    Node *n = in(i);            // Get the input
    if( phase->type(n) == Type::TOP )
      continue;                 // Ignore TOP inputs
    if( phase->eqv(n,this) ) continue; // Ignore simple cycles
    // Ignore more complex cycles using a cast-away-nullness 
    // feeding into a Phi
    if( n->Opcode() == Op_CastPP ) {
      if( phase->eqv(n->in(1),this) )
        continue;
      // Ignore conditional cast-away-nullness (cast-away on one path
      // but use the uncasted value on the other path).
      if( req() == 3 && phase->eqv(n->in(1),in(3-i)) )
        return in(3-i);         // Return unmerged uncasted value
    }
    if( input && input != n )   // Merging different values?
      return this;              // Then not any identity
    input = n;                  // Capture
  }
  return input ? input : phase->C->top(); // Return unique thing we are equal to
}

//------------------------------is_x2logic-------------------------------------
// Check for simple convert-to-boolean pattern
// If:(C Bool) Region:(IfF IfT) Phi:(Region 0 1)
// Convert Phi to an ConvIB.
static Node *is_x2logic( PhaseGVN *phase, Node *phi ) {

  int flipped = is_diamond_phi(phi);
  if( flipped==0 ) return NULL;
  // Convert the true/false index into an expected 0/1 return.
  // Map 2->0 and 1->1.
  flipped = 2-flipped;

  // phi->region->if_proj->ifnode->bool->cmp
  // All legal without further checks because "is_diamond_phi" has guarenteed
  // nice graph shape.
  Node *region = phi->in(0);
  Node *iff = region->in(1)->in(0);
  BoolNode *bool = (BoolNode*)iff->in(1);
  const CmpNode *cmp = (CmpNode*)bool->in(1);

  Node *zero = phi->in(1);
  Node *one  = phi->in(2);
  const Type *tzero = phase->type( zero );
  const Type *tone  = phase->type( one  );

  // Check for compare vs 0
  const Type *tcmp = phase->type(cmp->in(2));
  if( tcmp != TypeInt::ZERO && tcmp != TypePtr::NULL_PTR ) {
    // Allow cmp-vs-1 if the other input is bounded by 0-1
    if( !(tcmp == TypeInt::ONE && phase->type(cmp->in(1)) == TypeInt::BOOL) )
      return NULL;
    flipped = 1-flipped;        // Test is vs 1 instead of 0!
  }

  // Check for setting zero/one opposite expected
  if( tzero == TypeInt::ZERO ) {
    if( tone == TypeInt::ONE ) {
    } else return NULL;
  } else if( tzero == TypeInt::ONE ) {
    if( tone == TypeInt::ZERO ) {
      flipped = 1-flipped;
    } else return NULL;
  } else return NULL;

  // Check for boolean test backwards
  if( bool->_test._test == BoolTest::ne ) {
  } else if( bool->_test._test == BoolTest::eq ) {
    flipped = 1-flipped;
  } else return NULL;

  // Build int->bool conversion
  Node *n = new (2) Conv2BNode( cmp->in(1) );
  if( flipped ) 
    n = new (3) XorINode( phase->transform(n), phase->intcon(1) );

  return n;
}

//------------------------------is_cond_add------------------------------------
// Check for simple conditional add pattern:  "(P < Q) ? X+Y : X;"
// To be profitable the control flow has to disappear; there can be no other
// values merging here.  We replace the test-and-branch with: 
// "(sgn(P-Q))&Y) + X".  Basically, convert "(P < Q)" into 0 or -1 by
// moving the carry bit from (P-Q) into a register with 'sbb EAX,EAX'.  
// Then convert Y to 0-or-Y and finally add.
// This is a key transform for SpecJava _201_compress.
Node* is_cond_add(PhaseGVN *phase, PhiNode *phi) {
  int true_path = is_diamond_phi(phi);
  if( true_path==0 ) return NULL;

  // phi->region->if_proj->ifnode->bool->cmp
  // All legal without further checks because "is_diamond_phi" has guarenteed
  // nice graph shape.
  RegionNode *region = (RegionNode*)phi->in(0);
  Node *iff = region->in(1)->in(0);
  BoolNode *bool = (BoolNode*)iff->in(1);
  const CmpNode *cmp = (CmpNode*)bool->in(1);

  // Make sure only merging this one phi here
  if (region->has_unique_phi() != phi)  return NULL;

  // Make sure each arm of the diamond has exactly one output, which we assume
  // is the region.  Otherwise, the control flow won't disappear.
  if (region->in(1)->outcnt() != 1) return NULL;
  if (region->in(2)->outcnt() != 1) return NULL;

  // Check for "(P < Q)"
  if( bool->_test._test != BoolTest::lt ) return NULL;

  Node *p = cmp->in(1);
  Node *q = cmp->in(2);
  Node *n1 = phi->in(  true_path);
  Node *n2 = phi->in(3-true_path);

  int op = n1->Opcode();
  if( op != Op_AddI           // Need zero as additive identity
      /*&&op != Op_SubI &&
      op != Op_AddP &&
      op != Op_XorI &&
      op != Op_OrI*/ )
    return NULL;

  Node *x = n2;
  Node *y = n1->in(1);
  if( n2 == n1->in(1) ) {
    y = n1->in(2);
  } else if( n2 == n1->in(1) ) {
  } else return NULL;

  // Not so profitable if compare and add are constants
  if( q->is_Con() && phase->type(q) != TypeInt::ZERO && y->is_Con() ) 
    return NULL;
  
  Node *cmplt = phase->transform( new (3) CmpLTMaskNode(p,q) );
  Node *j_and   = phase->transform( new (3) AndINode(cmplt,y) );
  return new (3) AddINode(j_and,x);
}

//------------------------------split_once-------------------------------------
// Helper for split_flow_path
static void split_once(PhaseIterGVN *igvn, Node *phi, Node *val, Node *n, Node *newn) {
  igvn->hash_delete(n);         // Remove from hash before hacking edges

  uint j = 1;
  for( uint i = phi->req()-1; i > 0; i-- ) {
    if( phi->in(i) == val ) {   // Found a path with val?
      // Add to NEW Region/Phi, no DU info
      newn->set_req( j++, n->in(i) );
      // Remove from OLD Region/Phi
      n->del_req(i);
    }
  }

  // Register the new node but do not transform it.  Cannot transform until the
  // entire Region/Phi conglerate has been hacked as a single huge transform.
  igvn->register_new_node_with_optimizer( newn );
  // Now I can point to the new node.
  n->add_req(NULL);
  n->set_req(n->req()-1, newn);
  igvn->_worklist.push(n);
}

//------------------------------split_flow_path--------------------------------
// Check for merging identical values and split flow paths
static Node* split_flow_path(PhaseGVN *phase, PhiNode *phi) {
  BasicType bt = phi->type()->basic_type();
  if( bt == T_ILLEGAL || type2size[bt] <= 0 )
    return NULL;                // Bail out on funny non-value stuff
  if( phi->req() <= 3 )         // Need at least 2 matched inputs and a
    return NULL;                // third unequal input to be worth doing

  // Scan for a constant
  uint i;
  for( i = 1; i < phi->req()-1; i++ ) {
    Node *n = phi->in(i);
    if( !n ) return NULL;
    if( phase->type(n) == Type::TOP ) return NULL;
    if( n->Opcode() == Op_ConP )
      break;
  }
  if( i >= phi->req() )         // Only split for constants
    return NULL;
  
  Node *val = phi->in(i);       // Constant to split for
  uint hit = 0;                 // Number of times it occurs

  for( ; i < phi->req(); i++ ){ // Count occurances of constant
    Node *n = phi->in(i);
    if( !n ) return NULL;
    if( phase->type(n) == Type::TOP ) return NULL;
    if( phi->in(i) == val ) 
      hit++;
  }

  if( hit <= 1 ||               // Make sure we find 2 or more
      hit == phi->req()-1 )     // and not ALL the same value
    return NULL;

  // Now start splitting out the flow paths that merge the same value.
  // Split first the RegionNode.
  PhaseIterGVN *igvn = phase->is_IterGVN();
  Node *r = phi->region();
  RegionNode *newr = new RegionNode(hit+1);
  split_once(igvn, phi, val, r, newr);

  // Now split all other Phis than this one
  for (DUIterator_Fast kmax, k = r->fast_outs(kmax); k < kmax; k++) {
    PhiNode* phi2 = r->fast_out(k)->is_Phi();
    if( phi2 && phi2 != phi ) {
      PhiNode *newphi = PhiNode::make_blank(newr, phi2);
      split_once(igvn, phi, val, phi2, newphi);
    }
  }      

  // Clean up this guy
  igvn->hash_delete(phi);
  for( i = phi->req()-1; i > 0; i-- ) {
    if( phi->in(i) == val ) {
      phi->del_req(i);
    }
  }
  phi->add_req(NULL);
  phi->set_req(phi->req()-1, val);

  return phi;
}


//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  Must preserve
// the CFG, but we can still strip out dead paths.
Node *PhiNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  Node *r = in(0);              // RegionNode
  if( r == NULL )               // Already degraded to a Copy ?
    return NULL;                // No change

  assert(!r->in(0) || !r->is_Region() || !r->in(0)->is_Root(), "not a specially hidden merge");

  // Note: During parsing, phis are often transformed before their regions.
  // This means we have to use type_or_null to defend against untyped regions.
  if( phase->type_or_null(r) == Type::TOP ) // Dead code?
    return NULL;                // No change

  // Check for simple convert-to-boolean pattern
  Node *xbool = is_x2logic(phase,this);
  if( xbool ) return xbool;

  // Check for absolute value
  Node *abs = AbsNode::is_absolute(phase,this);
  if( abs ) return abs;

  // Check for conditional add
  if (can_reshape) {
    Node* add = is_cond_add(phase, this);
    if (add != NULL)  return add;
  }

  // Check for merging identical values and split flow paths
  if (can_reshape) {
    Node* foo = split_flow_path(phase, this);
    if (foo != NULL)  return foo;
  }

  // Check for merging only 1 value
  Node *progress = NULL;        // Record if any progress made
  Node *u = NULL;               // First unique input
  // For all inputs...
  for( uint i=1; i<req(); ++i ){// For all paths in
    // Unreachable control path?
    Node *rc = r->in(i);
    Node *n = in(i);            // Get the input
    if( (!rc || phase->type(rc) == Type::TOP) ) {
      if (n != phase->C->top()) { // Not already top?  
        set_req(i, phase->C->top());// Nuke it down
        progress = this;        // Record progress
      }
      continue;                 // Dead input is ignored
    }
    if( phase->eqv(n,this) ) continue; // Ignore simple cycles
    if( !u ) u = n;             // First unique input
    else if( u != n )           // Do they differ?
      u = NodeSentinel;            // Then flag as a real merge
  }

  // Treat "many inputs" the same as "no unique input".
  if (u == NodeSentinel)  u = NULL;

  if( u != NULL ) {   // Unique value?
    debug_only(Node* ident = Identity(phase));
    // The unique input must eventually be detected by the Identity call.
#ifdef ASSERT
    if( ident != u && !ident->is_top() ) {
      // print this output before failing assert
      r->dump(3);
      this->dump(3);
    }
#endif
    assert(ident == u || ident->is_top(), "Identity must clean this up");
    return NULL;
  }

  // Split phis through memory merges, so that the memory merges will go away.
  // Piggy-back this transformation on the search for a unique input....
  // It will be as if the merged memory is the unique value of the phi.
  // (Do not attempt this optimization unless parsing is complete.
  // It would make the parser's memory-merge logic sick.)
  if (progress == NULL && can_reshape && type() == Type::MEMORY) {
    // see if this phi should be sliced
    uint merge_width = 0;
    bool saw_self = false;
    for( uint i=1; i<req(); ++i ) {// For all paths in
      MergeMemNode* n = in(i)->is_MergeMem();
      if (n != NULL) {
        merge_width = MAX2(merge_width, n->req());
        saw_self |= (n->base_memory() == this); 
      }
    }

    // This restriction is temporarily necessary to ensure termination:
    if (!saw_self && adr_type() == TypePtr::BOTTOM)  merge_width = 0;

    if (merge_width > Compile::AliasIdxRaw) {
      // found at least one non-empty MergeMem
      const TypePtr* at = adr_type();
      if (at != TypePtr::BOTTOM) {
        // Patch the existing phi to select an input from the merge:
        // Phi:AT1(...MergeMem(m0, m1, m2)...) into
        //     Phi:AT1(...m1...)
        int alias_idx = phase->C->get_alias_index(at);
        for (uint i=1; i<req(); ++i) {
          MergeMemNode* n = in(i)->is_MergeMem();
          if (n != NULL) {
            // compress paths and change unreachable cycles to TOP
            // If not, we can update the input infinitely along a MergeMem cycle
            // Equivalent code is in MemNode::Ideal_common
            Node         *m  = phase->transform(n);
            MergeMemNode *mm = m->is_MergeMem();
            // If tranformed to a MergeMem, get the desired slice
            // Otherwise the returned node represents memory for every slice
            Node *new_mem = (mm != NULL)? mm->memory_at(alias_idx) : m;
            // Update input if it is progress over what we have now
            if (new_mem != in(i)) {
              set_req(i, new_mem);
              progress = this;
            }
          }
        }
      } else {
        // Phi(...MergeMem(m0, m1:AT1, m2:AT2)...) into
        //     MergeMem(Phi(...m0...), Phi:AT1(...m1...), Phi:AT2(...m2...))
        PhaseIterGVN *igvn = phase->is_IterGVN();
        Node* hook = new (1) Node((Node*)NULL);
        PhiNode* new_base = (PhiNode*) clone();
        // Must eagerly register phis, since they participate in loops.
        if (igvn) {
          igvn->register_new_node_with_optimizer(new_base);
          hook->add_req(NULL);
          hook->set_req(hook->req()-1, new_base);
        }
        MergeMemNode* result = new MergeMemNode(new_base);
        for (uint i = 1; i < req(); ++i) {
          MergeMemNode* n = in(i)->is_MergeMem();
          if (n != NULL) {
            for (MergeMemStream mms(result, n); mms.next_non_empty2(); ) {
              // If we have not seen this slice yet, make a phi for it.
              bool made_new_phi = false;
              if (mms.is_empty()) {
                Node* new_phi = new_base->slice_memory(mms.adr_type());
                made_new_phi = true;
                if (igvn) {
                  igvn->register_new_node_with_optimizer(new_phi);
                  hook->add_req(NULL);
                  hook->set_req(hook->req()-1, new_phi);
                }
                mms.set_memory(new_phi);
              }
              Node* phi = mms.memory();
              assert(made_new_phi || phi->in(i) == n, "replace the i-th merge by a slice");
              phi->set_req(i, mms.memory2());
            }
          }
        }
        // Distribute all self-loops.
        { // (Extra braces to hide mms.)
          for (MergeMemStream mms(result); mms.next_non_empty(); ) {
            Node* phi = mms.memory();
            for (uint i = 1; i < req(); ++i) {
              if (phi->in(i) == this)  phi->set_req(i, phi);
            }
          }
        }
        // now transform the new nodes, and return the mergemem
        for (MergeMemStream mms(result); mms.next_non_empty(); ) {
          Node* phi = mms.memory();
          mms.set_memory(phase->transform(phi));
        }
        if (igvn) { // Unhook.
          igvn->hash_delete(hook);
          for (uint i = 1; i < hook->req(); i++) {
            hook->set_req(i, NULL);
          }
        }
        // Replace self with the result.
        return result;
      }
    }
  }

  return progress;              // Return any progress
}

//------------------------------out_RegMask------------------------------------
const RegMask &PhiNode::in_RegMask(uint i) const { 
  return i ? out_RegMask() : RegMask::Empty;
}

const RegMask &PhiNode::out_RegMask() const { 
  uint ideal_reg = Matcher::base2reg[_type->base()];
  assert( ideal_reg != Node::NotAMachineReg, "invalid type at Phi" );
  if( ideal_reg == 0 ) return RegMask::Empty;
  return *(Compile::current()->matcher()->idealreg2spillmask[ideal_reg]);
}


//=============================================================================
const Type *GotoNode::Value( PhaseTransform *phase ) const {
  // If the input is reachable, then we are executed.
  // If the input is not reachable, then we are not executed.
  return phase->type(in(0)); 
}

Node *GotoNode::Identity( PhaseTransform *phase ) {
  return in(0);                // Simple copy of incoming control
}

const RegMask &GotoNode::out_RegMask() const { 
  return RegMask::Empty;
}

//=============================================================================
const RegMask &JumpNode::out_RegMask() const { 
  return RegMask::Empty;
}
 
//=============================================================================
const RegMask &JProjNode::out_RegMask() const { 
  return RegMask::Empty;
}

//=============================================================================
const RegMask &CProjNode::out_RegMask() const { 
  return RegMask::Empty;
}



//=============================================================================

uint PCTableNode::hash() const { return Node::hash() + _size; }
uint PCTableNode::cmp( const Node &n ) const
{ return _size == ((PCTableNode&)n)._size; }

const Type *PCTableNode::bottom_type() const {
  const Type** f = TypeTuple::fields(_size);
  for( uint i = 0; i < _size; i++ ) f[i] = Type::CONTROL;
  return TypeTuple::make(_size, f);
}

//------------------------------Value------------------------------------------
// Compute the type of the PCTableNode.  If reachable it is a tuple of 
// Control, otherwise the table targets are not reachable
const Type *PCTableNode::Value( PhaseTransform *phase ) const {
  if( phase->type(in(0)) == Type::CONTROL )
    return bottom_type();
  return Type::TOP;             // All paths dead?  Then so are we
}

//------------------------------Ideal------------------------------------------
// Return a node which is more "ideal" than the current node.  Strip out 
// control copies
Node *PCTableNode::Ideal(PhaseGVN *phase, bool can_reshape) {
  return remove_dead_region(phase, can_reshape) ? this : NULL;
}

//=============================================================================
uint JumpProjNode::hash() const {
  return Node::hash() + _dest_bci;
}

uint JumpProjNode::cmp( const Node &n ) const {
  return ProjNode::cmp(n) &&
    _dest_bci == ((JumpProjNode&)n)._dest_bci;
}

JumpProjNode::JumpProjNode(Node* jumpnode, uint proj_no, int dest_bci, int switch_val)
  : JProjNode(jumpnode, proj_no), _dest_bci(dest_bci), _proj_no(proj_no), _switch_val(switch_val) {
}

#ifndef PRODUCT
void JumpProjNode::dump_spec() const { 
  ProjNode::dump_spec();
   tty->print("@bci %d ",_dest_bci);
}
#endif

//=============================================================================
//------------------------------Value------------------------------------------
// Check for being unreachable, or for coming from a Rethrow.  Rethrow's cannot
// have the default "fall_through_index" path.
const Type *CatchNode::Value( PhaseTransform *phase ) const {
  // Unreachable?  Then so are all paths from here.
  if( phase->type(in(0)) == Type::TOP ) return Type::TOP;
  // First assume all paths are reachable
  const Type** f = TypeTuple::fields(_size);
  for( uint i = 0; i < _size; i++ ) f[i] = Type::CONTROL;
  // Identify cases that will always throw an exception
  // () rethrow call
  // () virtual or interface call with NULL receiver
  // () call is a check cast with incompatible arguments
  if( in(1)->is_Proj() ) {
    CallNode *call = in(1)->in(0)->is_Call();
    if( call ) {
      // Rethrows always throw exceptions, never return
      if (call->entry_point() == OptoRuntime::rethrow_stub()) {
        f[CatchProjNode::fall_through_index] = Type::TOP;
      } else if( call->req() > TypeFunc::Parms ) {
        const Type *arg0 = phase->type( call->in(TypeFunc::Parms) );
        // Check for null reciever to virtual or interface calls
        if( call->is_CallDynamicJava() && 
            arg0->higher_equal(TypePtr::NULL_PTR) ) { 
          f[CatchProjNode::fall_through_index] = Type::TOP;
        }
      } // End of if not a runtime stub
    } // End of if have call above me
  } // End of slot 1 is not a projection
  return TypeTuple::make(_size, f);
}

//=============================================================================
uint CatchProjNode::hash() const {
  return Node::hash() + _handler_bci;
}


uint CatchProjNode::cmp( const Node &n ) const {
  return ProjNode::cmp(n) &&
    _handler_bci == ((CatchProjNode&)n)._handler_bci;
}


CatchProjNode::CatchProjNode(Node* catchnode, uint proj_no, int handler_bci)
: CProjNode(catchnode, proj_no), _handler_bci(handler_bci) {
  assert(proj_no != fall_through_index || handler_bci < 0, "fall through case must have bci < 0");
}


//------------------------------Identity---------------------------------------
// If only 1 target is possible, choose it if it is the main control
Node *CatchProjNode::Identity( PhaseTransform *phase ) {
  // If my value is control and no other value is, then treat as ID
  const TypeTuple *t = phase->type(in(0))->is_tuple();
  if (t->field_at(_con) != Type::CONTROL)  return this;
  // If we remove the last CatchProj and elide the Catch/CatchProj, then we
  // also remove any exception table entry.  Thus we must know the call
  // feeding the Catch will not really throw an exception.  This is ok for
  // the main fall-thru control (happens when we know a call can never throw
  // an exception) or for "rethrow", because a further optimnization will
  // yank the rethrow (happens when we inline a function that can throw an
  // exception and the caller has no handler).  Not legal, e.g., for passing
  // a NULL receiver to a v-call, or passing bad types to a slow-check-cast.
  // These cases MUST throw an exception via the runtime system, so the VM
  // will be looking for a table entry.
  Node *proj = in(0)->in(1);    // Expect a proj feeding CatchNode
  CallNode *call;
  if (_con != TypeFunc::Control && // Bail out if not the main control.
      !(proj->is_Proj() &&      // AND NOT a rethrow
        (call = proj->in(0)->is_Call()) &&
        call->entry_point() == OptoRuntime::rethrow_stub()))
    return this;

  // Search for any other path being control
  for (uint i = 0; i < t->cnt(); i++) {
    if (i != _con && t->field_at(i) == Type::CONTROL)
      return this;
  }
  // Only my path is possible; I am identity on control to the jump
  return in(0)->in(0);
}


#ifndef PRODUCT
void CatchProjNode::dump_spec() const { 
  ProjNode::dump_spec();
  tty->print("@bci %d ",_handler_bci);
}
#endif

//=============================================================================
//------------------------------Identity---------------------------------------
// Check for CreateEx being Identity.
Node *CreateExNode::Identity( PhaseTransform *phase ) {
  if( phase->type(in(1)) == Type::TOP ) return in(1);
  if( phase->type(in(0)) == Type::TOP ) return in(0);
  // We only come from CatchProj, unless the CatchProj goes away.
  // If the CatchProj is optimized away, then we just carry the 
  // exception oop through.
  CallNode *call = in(1)->in(0)->is_Call();
  assert( call, "" );

  return ( in(0)->is_CatchProj() && in(0)->in(0)->in(1) == in(1) ) 
    ? this
    : call->in(TypeFunc::Parms);
}

//=============================================================================
#ifndef PRODUCT
void NeverBranchNode::format( PhaseRegAlloc *ra_ ) const {
  tty->print("%s", Name());
}
#endif

