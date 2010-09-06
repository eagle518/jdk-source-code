#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)loopopts.cpp	1.193 04/02/25 16:47:59 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_loopopts.cpp.incl"

//=============================================================================
//------------------------------split_thru_phi---------------------------------
// Split Node 'n' through merge point if there is enough win.
Node *PhaseIdealLoop::split_thru_phi( Node *n, Node *region, int policy ) {
  int wins = 0;
  assert( !n->is_CFG(), "" );
  assert( region->is_Region(), "" );
  Node *phi = new PhiNode( region, n->bottom_type() );
  uint old_unique = C->unique();
  for( uint i = 1; i < region->req(); i++ ) {
    Node *x;
    Node* the_clone = NULL;
    if( region->in(i) == C->top() ) {
      x = C->top();             // Dead path?  Use a dead data op
    } else {
      x = n->clone();           // Else clone up the data op
      the_clone = x;            // Remember for possible deletion.
      // Alter data node to use pre-phi inputs
      if( n->in(0) == region )
        x->set_req( 0, region->in(i) );
      for( uint j = 1; j < n->req(); j++ ) {
        Node *in = n->in(j);
        if( in->is_Phi() && in->in(0) == region ) 
          x->set_req( j, in->in(i) ); // Use pre-Phi input for the clone
      }
    }
    // Check for a 'win' on some paths
    const Type *t = x->Value(&_igvn);

    bool singleton = t->singleton();

    // A TOP singleton indicates that there are no possible values incoming
    // along a particular edge. In most cases, this is OK, and the Phi will
    // be eliminated later in an Ideal call. However, we can't allow this to 
    // happen if the singleton occurs on loop entry, as the elimination of 
    // the PhiNode may cause the resulting node to migrate back to a previous 
    // loop iteration.  
    if( singleton && t == Type::TOP ) {
      // Is_Loop() == false does not confirm the absence of a loop (e.g., an
      // irreducible loop may not be indicated by an affirmative is_Loop());
      // therefore, the only top we can split thru a phi is on a backedge of
      // a loop.
      singleton &= region->is_Loop() && (i != LoopNode::EntryControl);
    }

    if( singleton ) {
      wins++;
      x = ((PhaseGVN&)_igvn).makecon(t);
    } else {
      // We now call Identity to try to simplify the cloned node.
      // Note that some Identity methods call phase->type(this).
      // Make sure that the type array is big enough for
      // our new node, even though we may throw the node away.
      // (Note: This tweaking with igvn only works because x is a new node.)
      _igvn.set_type(x, t);
      Node *y = x->Identity(&_igvn);
      if( y != x ) {
        wins++;
        x = y;
      } else {
        y = _igvn.hash_find(x);
        if( y ) {
          wins++;
          x = y;
        } else {
          // Else x is a new node we are keeping
          // We do not need register_new_node_with_optimizer
          // because set_type has already been called.
          _igvn._worklist.push(x);
        }
      }
    }
    if (x != the_clone && the_clone != NULL)
      _igvn.remove_dead_node(the_clone);
    phi->set_req( i, x );
  }
  // Too few wins?
  if( wins <= policy ) {
    _igvn.remove_dead_node(phi);
    return NULL;
  }

  // Record Phi 
  register_new_node( phi, region );

  for( uint i2 = 1; i2 < phi->req(); i2++ ) {
    Node *x = phi->in(i2);
    // If we commoned up the cloned 'x' with another existing Node, 
    // the existing Node picks up a new use.  We need to make the
    // existing Node occur higher up so it dominates its uses.
    Node *old_ctrl;
    IdealLoopTree *old_loop;

    // The occasional new node
    if( x->_idx >= old_unique ) {   // Found a new, unplaced node?  
      old_ctrl = x->is_Con() ? C->root() : NULL;
      old_loop = NULL;              // Not in any prior loop
    } else {
      old_ctrl = get_ctrl(x);
      old_loop = get_loop(old_ctrl); // Get prior loop
    }
    // New late point must dominate new use
    Node *new_ctrl = dom_lca( old_ctrl, region->in(i2) );
    // Set new location
    set_ctrl(x, new_ctrl);
    IdealLoopTree *new_loop = get_loop( new_ctrl );
    // If changing loop bodies, see if we need to collect into new body
    if( old_loop != new_loop ) {
      if( old_loop && !old_loop->_child ) 
        old_loop->_body.yank(x);
      if( !new_loop->_child )
        new_loop->_body.push(x);  // Collect body info
    }
  }

  return phi;
}

//------------------------------dominated_by------------------------------------
// Replace the dominated test with an obvious true or false.  Place it on the
// IGVN worklist for later cleanup.  Move control-dependent data Nodes on the
// live path up to the dominating control.
void PhaseIdealLoop::dominated_by( Node *prevdom, Node *iff ) {
#ifndef PRODUCT
  if( VerifyLoopOptimizations && PrintOpto ) tty->print_cr("dominating test");
#endif
 

  // prevdom is the dominating projection of the dominating test.
  assert( iff->is_If(), "" );
  assert( iff->Opcode() == Op_If || iff->Opcode() == Op_CountedLoopEnd, "Check this code when new subtype is added");
  int pop = prevdom->Opcode();
  assert( pop == Op_IfFalse || pop == Op_IfTrue, "" );
  // 'con' is set to true or false to kill the dominated test.
  Node *con = _igvn.makecon(pop == Op_IfTrue ? TypeInt::ONE : TypeInt::ZERO);
  set_ctrl(con, C->root()); // Constant gets a new use
  // Hack the dominated test
  _igvn.hash_delete(iff);
  iff->set_req(1, con);
  _igvn._worklist.push(iff);

  // If I dont have a reachable TRUE and FALSE path following the IfNode then
  // I can assume this path reaches an infinite loop.  In this case it's not
  // important to optimize the data Nodes - either the whole compilation will
  // be tossed or this path (and all data Nodes) will go dead.
  if( iff->outcnt() != 2 ) return;

  // Make control-dependent data Nodes on the live path (path that will remain
  // once the dominated IF is removed) become control-dependent on the 
  // dominating projection.
  Node* dp = ((IfNode*)iff)->proj_out(pop == Op_IfTrue);
  IdealLoopTree *old_loop = get_loop(dp);

  for (DUIterator_Fast imax, i = dp->fast_outs(imax); i < imax; i++) {
    Node* cd = dp->fast_out(i); // Control-dependent node
    if( cd->depends_only_on_test() ) {
      assert( cd->in(0) == dp, "" );
      _igvn.hash_delete( cd );
      cd->set_req(0, prevdom);
      set_early_ctrl( cd );
      _igvn._worklist.push(cd);
      IdealLoopTree *new_loop = get_loop(get_ctrl(cd));
      if( old_loop != new_loop ) {
        if( !old_loop->_child ) old_loop->_body.yank(cd);
        if( !new_loop->_child ) new_loop->_body.push(cd);
      }
      --i;
      --imax;
    }
  }
}

//------------------------------has_local_phi_input----------------------------
// Return TRUE if 'n' has Phi inputs from its local block and no other
// block-local inputs (all non-local-phi inputs come from earlier blocks)
Node *PhaseIdealLoop::has_local_phi_input( Node *n ) {
  Node *n_ctrl = get_ctrl(n);
  // See if some inputs come from a Phi in this block, or from before
  // this block.
  uint i;
  for( i = 1; i < n->req(); i++ ) {
    Node *phi = n->in(i)->is_Phi();
    if( phi && phi->in(0) == n_ctrl )
      break;
  }
  if( i >= n->req() )
    return NULL;                // No Phi inputs; nowhere to clone thru

  // Check for inputs created between 'n' and the Phi input.  These
  // must split as well; they have already been given the chance
  // (courtesy of a post-order visit) and since they did not we must
  // recover the 'cost' of splitting them by being very profitable
  // when splitting 'n'.  Since this is unlikely we simply give up.
  for( i = 1; i < n->req(); i++ ) {
    Node *m = n->in(i);
    if( get_ctrl(m) == n_ctrl && !m->is_Phi() ) {
      // We allow the special case of AddP's with no local inputs.
      // This allows us to split-up address expressions.
      if (m->is_AddP() && 
          get_ctrl(m->in(2)) != n_ctrl &&
          get_ctrl(m->in(3)) != n_ctrl) {
        // Move the AddP up to dominating point
        set_ctrl(m, idom(n_ctrl));
        continue;
      }
      return NULL;
    }
  }

  return n_ctrl;
}

//------------------------------remix_address_expressions----------------------
// Rework addressing expressions to get the most loop-invariant stuff 
// moved out.  We'd like to do all associative operators, but it's especially
// important (common) to do address expressions.
Node *PhaseIdealLoop::remix_address_expressions( Node *n ) {
  if (!has_ctrl(n))  return NULL;
  Node *n_ctrl = get_ctrl(n);
  IdealLoopTree *n_loop = get_loop(n_ctrl); 

  // See if 'n' mixes loop-varying and loop-invariant inputs and
  // itself is loop-varying.

  // Only interested in binary ops (and AddP)
  if( n->req() < 3 || n->req() > 4 ) return NULL;

  Node *n1_ctrl = get_ctrl(n->in(                    1));
  Node *n2_ctrl = get_ctrl(n->in(                    2));
  Node *n3_ctrl = get_ctrl(n->in(n->req() == 3 ? 2 : 3));
  IdealLoopTree *n1_loop = get_loop( n1_ctrl ); 
  IdealLoopTree *n2_loop = get_loop( n2_ctrl );
  IdealLoopTree *n3_loop = get_loop( n3_ctrl ); 

  // Does one of my inputs spin in a tighter loop than self?
  if( (n_loop->is_member( n1_loop ) && n_loop != n1_loop) ||
      (n_loop->is_member( n2_loop ) && n_loop != n2_loop) ||
      (n_loop->is_member( n3_loop ) && n_loop != n3_loop) )
    return NULL;                // Leave well enough alone

  // Is at least one of my inputs loop-invariant?
  if( n1_loop == n_loop &&
      n2_loop == n_loop &&
      n3_loop == n_loop )
    return NULL;                // No loop-invariant inputs


  int n_op = n->Opcode();

  // Replace expressions like ((V+I) << 2) with (V<<2 + I<<2).
  if( n_op == Op_LShiftI ) {
    // Scale is loop invariant
    Node *scale = n->in(2);
    Node *scale_ctrl = get_ctrl(scale);
    IdealLoopTree *scale_loop = get_loop(scale_ctrl ); 
    if( n_loop == scale_loop || !scale_loop->is_member( n_loop ) )
      return NULL;
    const TypeInt *scale_t = scale->bottom_type()->isa_int();
    if( scale_t && scale_t->is_con() && scale_t->get_con() >= 16 )
      return NULL;              // Dont bother with byte/short masking
    // Add must vary with loop (else shift would be loop-invariant)
    Node *add = n->in(1);
    Node *add_ctrl = get_ctrl(add);
    IdealLoopTree *add_loop = get_loop(add_ctrl); 
    //assert( n_loop == add_loop, "" );
    if( n_loop != add_loop ) return NULL;  // happens w/ evil ZKM loops

    // Convert I-V into I+ (0-V); same for V-I
    if( add->Opcode() == Op_SubI &&
        _igvn.type( add->in(1) ) != TypeInt::ZERO ) {
      Node *zero = _igvn.intcon(0);
      set_ctrl(zero, C->root());
      Node *neg = new (3) SubINode( _igvn.intcon(0), add->in(2) );
      register_new_node( neg, get_ctrl(add->in(2) ) );
      add = new (3) AddINode( add->in(1), neg );
      register_new_node( add, add_ctrl );
    }
    if( add->Opcode() != Op_AddI ) return NULL;
    // See if one add input is loop invariant
    Node *add_var = add->in(1);
    Node *add_var_ctrl = get_ctrl(add_var);
    IdealLoopTree *add_var_loop = get_loop(add_var_ctrl ); 
    Node *add_invar = add->in(2);
    Node *add_invar_ctrl = get_ctrl(add_invar);
    IdealLoopTree *add_invar_loop = get_loop(add_invar_ctrl ); 
    if( add_var_loop == n_loop ) {
    } else if( add_invar_loop == n_loop ) {
      // Swap to find the invariant part
      add_invar = add_var;
      add_invar_ctrl = add_var_ctrl;
      add_invar_loop = add_var_loop;
      add_var = add->in(2);
      Node *add_var_ctrl = get_ctrl(add_var);
      IdealLoopTree *add_var_loop = get_loop(add_var_ctrl ); 
    } else                      // Else neither input is loop invariant
      return NULL;
    if( n_loop == add_invar_loop || !add_invar_loop->is_member( n_loop ) )
      return NULL;              // No invariant part of the add?

    // Yes!  Reshape address expression!
    Node *inv_scale = new (3) LShiftINode( add_invar, scale );
    register_new_node( inv_scale, add_invar_ctrl );
    Node *var_scale = new (3) LShiftINode( add_var, scale );
    register_new_node( var_scale, n_ctrl );
    Node *var_add = new (3) AddINode( var_scale, inv_scale );
    register_new_node( var_add, n_ctrl );
    _igvn.hash_delete( n );
    _igvn.subsume_node( n, var_add );
    return var_add;
  }

  // Replace (I+V) with (V+I)
  if( n_op == Op_AddI ||
      n_op == Op_AddL ||
      n_op == Op_AddF ||
      n_op == Op_AddD ||
      n_op == Op_MulI ||
      n_op == Op_MulL ||
      n_op == Op_MulF ||
      n_op == Op_MulD ) {
    if( n2_loop == n_loop ) {
      assert( n1_loop != n_loop, "" );
      n->swap_edges(1, 2);
    }
  }

  // Replace ((I1 +p V) +p I2) with ((I1 +p I2) +p V),
  // but not if I2 is a constant.
  if( n_op == Op_AddP ) {
    if( n2_loop == n_loop && n3_loop != n_loop ) {
      if( n->in(2)->Opcode() == Op_AddP && !n->in(3)->is_Con() ) {
        Node *n22_ctrl = get_ctrl(n->in(2)->in(2));
        Node *n23_ctrl = get_ctrl(n->in(2)->in(3));
        IdealLoopTree *n22loop = get_loop( n22_ctrl ); 
        IdealLoopTree *n23_loop = get_loop( n23_ctrl );
        if( n22loop != n_loop && n22loop->is_member(n_loop) && 
            n23_loop == n_loop ) {
          Node *add1 = new (4) AddPNode( n->in(1), n->in(2)->in(2), n->in(3) );
          register_new_node( add1, n22_ctrl );
          Node *add2 = new (4) AddPNode( n->in(1), add1, n->in(2)->in(3) );
          register_new_node( add2, n_ctrl );
          _igvn.hash_delete( n );
          _igvn.subsume_node( n, add2 );
          return add2;
        }
      }
    }

    // Replace (I1 +p (I2 + V)) with ((I1 +p I2) +p V)
    if( n2_loop != n_loop && n3_loop == n_loop ) {
      if( n->in(3)->Opcode() == Op_AddI ) {
        Node *V = n->in(3)->in(1);
        Node *I = n->in(3)->in(2);
        if( is_member(n_loop,get_ctrl(V)) ) {
        } else {
          Node *tmp = V; V = I; I = tmp;
        }
        if( !is_member(n_loop,get_ctrl(I)) ) {
          Node *add1 = new (4) AddPNode( n->in(1), n->in(2), I );
          // Stuff new AddP in the loop preheader
          register_new_node( add1, n_loop->_head->in(LoopNode::EntryControl) );
          Node *add2 = new (4) AddPNode( n->in(1), add1, V );
          register_new_node( add2, n_ctrl );
          _igvn.hash_delete( n );
          _igvn.subsume_node( n, add2 );
          return add2;
        }
      }
    }
  }

  return NULL;
}

//------------------------------conditional_move-------------------------------
// Attempt to replace a Phi with a conditional move.  We have some pretty
// strict profitability requirements.  All Phis at the merge point must
// be converted, so we can remove the control flow.  We need to limit the
// number of c-moves to a small handful.  All code that was in the side-arms
// of the CFG diamond is now speculatively executed.  This code has to be 
// "cheap enough".  We are pretty much limited to CFG diamonds that merge
// 1 or 2 items with a total of 1 or 2 ops executed speculatively.
Node *PhaseIdealLoop::conditional_move( Node *region ) {

  assert( region->is_Region(), "sanity check" );
  if( region->req() != 3 ) return NULL;

  // Check for CFG diamond
  Node *lp = region->in(1);
  Node *rp = region->in(2);
  if( !lp || !rp ) return NULL;
  if( lp->in(0) != rp->in(0) ) return NULL;
  if( !lp->in(0) ) return NULL;
  IfNode *iff = lp->in(0)->is_If();
  if( !iff ) return NULL;

  // Check for highly predictable branch.  No point in CMOV'ing if
  // we are going to predict accurately all the time.
  if( iff->_prob < PROB_UNLIKELY_MAG(3) ||
      iff->_prob > PROB_LIKELY_MAG(3) )
    return NULL;

  // Check for ops pinned in an arm of the diamond.
  // Can't remove the control flow in this case
  if( lp->outcnt() > 1 ) return NULL;
  if( rp->outcnt() > 1 ) return NULL;

  // Check profitability
  int cost = 0;
  for (DUIterator_Fast imax, i = region->fast_outs(imax); i < imax; i++) {
    PhiNode* phi = region->fast_out(i)->is_Phi();
    if( !phi ) continue;        // Ignore other control edges,etc
    switch (phi->type()->basic_type()) {
    case T_LONG:
      cost++;                   // Probably encodes as 2 CMOV's
    case T_INT:                 // These all CMOV fine
    case T_FLOAT:
    case T_DOUBLE:
    case T_ADDRESS:             // (RawPtr)
      cost++;
      break;
    case T_OBJECT: {            // Base oops are OK, but not derived oops
      const TypeOopPtr *tp = phi->type()->isa_oopptr();
      // Derived pointers are Bad (tm): what's the Base (for GC purposes) of a
      // CMOVE'd derived pointer?  It's a CMOVE'd derived base.  Thus
      // CMOVE'ing a derived pointer requires we also CMOVE the base.  If we
      // have a Phi for the base here that we convert to a CMOVE all is well
      // and good.  But if the base is dead, we'll not make a CMOVE.  Later
      // the allocator will have to produce a base by creating a CMOVE of the
      // relevant bases.  This puts the allocator in the business of
      // manufacturing expensive instructions, generally a bad plan.
      // Just Say No to Conditionally-Moved Derived Pointers.
      if( tp && tp->offset() != 0 )
        return NULL;
      cost++;
      break;
    }
    default:
      return NULL;              // In particular, can't do memory or I/O
    }
    // Add in cost any speculative ops
    for( uint j = 1; j < region->req(); j++ ) {
      Node *proj = region->in(j);
      Node *inp = phi->in(j);
      if (get_ctrl(inp) == proj) { // Found local op
        cost++;
        // Check for a chain of dependent ops; these will all become
        // speculative in a CMOV.
        for( uint k = 1; k < inp->req(); k++ ) 
          if (get_ctrl(inp->in(k)) == proj)
            return NULL;        // Too much speculative goo
      }
    }
    // See if the Phi is used by a Cmp.  This will likely Split-If, a
    // higher-payoff operation.
    for (DUIterator_Fast kmax, k = phi->fast_outs(kmax); k < kmax; k++) {
      Node* use = phi->fast_out(k);
      if( use->is_Cmp() )
        return NULL;
    }
  }
  if( cost >= ConditionalMoveLimit ) return NULL; // Too much goo

  // --------------
  // Now replace all Phis with CMOV's
  Node *cmov_ctrl = iff;
  uint flip = (lp->Opcode() == Op_IfTrue);
  while( 1 ) {
    PhiNode* phi = NULL;
    for (DUIterator_Fast imax, i = region->fast_outs(imax); i < imax; i++) {
      phi = region->fast_out(i)->is_Phi();
      if (phi != NULL) 
        break;
    }
    if (phi == NULL)  break;
#ifndef PRODUCT
    if( PrintOpto && VerifyLoopOptimizations ) tty->print_cr("CMOV");
#endif
    // Move speculative ops
    for( uint j = 1; j < region->req(); j++ ) {
      Node *proj = region->in(j);
      Node *inp = phi->in(j);
      if (get_ctrl(inp) == proj) { // Found local op
#ifndef PRODUCT
        if( PrintOpto && VerifyLoopOptimizations ) {
          tty->print("  speculate: ");
          inp->dump();
        }
#endif
        set_ctrl(inp, cmov_ctrl);
      }
    }
    Node *cmov = CMoveNode::make( cmov_ctrl->in(0), iff->in(1), phi->in(1+flip), phi->in(2-flip), _igvn.type(phi) );
    register_new_node( cmov, cmov_ctrl );
    _igvn.hash_delete(phi);
    _igvn.subsume_node( phi, cmov );
#ifndef PRODUCT
    if( VerifyLoopOptimizations ) verify();
#endif
  }

  // The useless CFG diamond will fold up later; see the optimization in
  // RegionNode::Ideal.
  _igvn._worklist.push(region);

  return iff->in(1);
}

//------------------------------split_if_with_blocks_pre-----------------------
// Do the real work in a non-recursive function.  Data nodes want to be
// cloned in the pre-order so they can feed each other nicely.
Node *PhaseIdealLoop::split_if_with_blocks_pre( Node *n ) {
  // Cloning these guys is unlikely to win
  int n_op = n->Opcode();
  if( n_op == Op_MergeMem ) return n;
  if( n->is_Proj() ) return n;
  // Do not clone-up CmpFXXX variations, as these are always
  // followed by a CmpI
  if( n->is_Cmp() ) return n;
  // Attempt to use a conditional move instead of a phi/branch
  if( ConditionalMoveLimit > 0 && n_op == Op_Region ) {
    Node *cmov = conditional_move( n );
    if( cmov ) return cmov;
  }
  if( n->is_CFG() || n_op == Op_StorePConditional || n_op == Op_StoreLConditional || n_op == Op_CompareAndSwapI || n_op == Op_CompareAndSwapL ||n_op == Op_CompareAndSwapP)  return n;
  if( n_op == Op_Opaque1 ||     // Opaque nodes cannot be mod'd
      n_op == Op_Opaque2 ) {
    if( !C->major_progress() )   // If chance of no more loop opts...
      _igvn._worklist.push(n);  // maybe we'll remove them
    return n;
  }

  Node *n_ctrl = get_ctrl(n);
  if( !n_ctrl ) return n;       // Dead node

  // Attempt to remix address expressions for loop invariants
  Node *m = remix_address_expressions( n );
  if( m ) return m;

  // Determine if the Node has inputs from some local Phi.
  // Returns the block to clone thru.
  Node *n_blk = has_local_phi_input( n );
  if( !n_blk ) return n;
  // Do not clone the trip counter through on a CountedLoop
  // (messes up the canonical shape).
  CountedLoopNode *cl = n_blk->is_CountedLoop();
  if( cl && n->Opcode() == Op_AddI ) return n;

  // Check for having no control input; not pinned.  Allow 
  // dominating control.
  if( n->in(0) ) {
    Node *dom = idom(n_blk);
    if( dom_lca( n->in(0), dom ) != n->in(0) )
      return n;
  }
  // Policy: when is it profitable.  You must get more wins than
  // policy before it is considered profitable.  Policy is usually 0,
  // so 1 win is considered profitable.  Big merges will require big
  // cloning, so get a larger policy.
  int policy = n_blk->req() >> 2;

  // Split 'n' through the merge point if it is profitable
  Node *phi = split_thru_phi( n, n_blk, policy );
  if( !phi ) return n;

  // Found a Phi to split thru!
  // Replace 'n' with the new phi
  _igvn.hash_delete(n);
  _igvn.subsume_node( n, phi );
  // Moved a load around the loop, 'en-registering' something.
  if( n_blk->Opcode() == Op_Loop && n->is_Load() && 
      !phi->in(LoopNode::LoopBackControl)->is_Load() )
    C->set_major_progress();

  return phi;
}

static bool merge_point_too_heavy(Compile* C, Node* region) {
  // Bail out if the region and its phis have too many users.
  int weight = 0;
  for (DUIterator_Fast imax, i = region->fast_outs(imax); i < imax; i++) {
    weight += region->fast_out(i)->outcnt();
  }
  int nodes_left = MaxNodeLimit - C->unique();
  if (weight * 8 > nodes_left) {
#ifndef PRODUCT
    if (PrintOpto)
      tty->print_cr("*** Split-if bails out:  %d nodes, region weight %d", C->unique(), weight);
#endif
    return true;
  } else {
    return false;
  }
}

#ifdef _LP64
static bool merge_point_safe(Node* region) {
  // 4799512: Stop split_if_with_blocks from splitting a block with a ConvI2LNode
  // having a PhiNode input. This sidesteps the dangerous case where the split 
  // ConvI2LNode may become TOP if the input Value() does not
  // overlap the ConvI2L range, leaving a node which may not dominate its
  // uses.
  // A better fix for this problem can be found in the BugTraq entry, but
  // expediency for Mantis demands this hack.
  for (DUIterator_Fast imax, i = region->fast_outs(imax); i < imax; i++) {
    Node* n = region->fast_out(i);
    if (n->is_Phi()) {
      for (DUIterator_Fast jmax, j = n->fast_outs(jmax); j < jmax; j++) {
        Node* m = n->fast_out(j);
        if (m->Opcode() == Op_ConvI2L) {
          return false;
        }
      }
    } 
  }
  return true;
}
#endif


//------------------------------place_near_use---------------------------------
// Place some computation next to use but not inside inner loops.
// For inner loop uses move it to the preheader area.
Node *PhaseIdealLoop::place_near_use( Node *useblock ) const {
  IdealLoopTree *u_loop = get_loop( useblock );
  return (u_loop->_irreducible || u_loop->_child) 
    ? useblock
    : u_loop->_head->in(LoopNode::EntryControl);
}


//------------------------------set_ctrl_for_Load------------------------------
/*
 A control edge to a CFG node outside of the loop is put on LoadNode clones
 (fix for 4641526) to force them to not combine and return back inside the loop 
 during GVN optimization. But MultiNode should have only ProjNode as outs.
 For example, IfNode should have only 2 outs: IfTrueNode and IfFalseNode.
 Additional outs in IfNode don't allow to remove a dead IfNode (4985384). 
 Also final_graph_reshaping() checks IfNode explicitly for 2 outs. 
*/
inline static Node *set_ctrl_for_Load( Node *n, Node *ctrl ) {
  if (ctrl != NULL && ctrl->is_Multi())
    ctrl = ctrl->in(0); // Use MultiNode's control edge instead
  n->set_req(0, ctrl);
  return ctrl;
}


//------------------------------split_if_with_blocks_post----------------------
// Do the real work in a non-recursive function.  CFG hackery wants to be
// in the post-order, so it can dirty the I-DOM info and not use the dirtied
// info.
void PhaseIdealLoop::split_if_with_blocks_post( Node *n ) {

  // Cloning Cmp through Phi's involves the split-if transform.
  if( n->is_Cmp() ) {
    if( C->unique() > 35000 ) return; // Method too big

    // Do not do 'split-if' if irreducible loops are present.
    if( _has_irreducible_loops )
      return;

    Node *n_ctrl = get_ctrl(n);
    // Determine if the Node has inputs from some local Phi.
    // Returns the block to clone thru.
    Node *n_blk = has_local_phi_input( n );
    if( n_blk != n_ctrl ) return;

    if( merge_point_too_heavy(C, n_ctrl) )
      return;

    if( n->outcnt() != 1 ) return; // Multiple bool's from 1 compare?
    Node *bol = n->unique_out();
    assert( bol->is_Bool(), "expect a bool here" );
    if( bol->outcnt() != 1 ) return;// Multiple branches from 1 compare?
    Node *iff = bol->unique_out();

    // Check some safety conditions
    if( iff->is_If() ) {        // Classic split-if?
      if( iff->in(0) != n_ctrl ) return; // Compare must be in same blk as if
    } else {                    // Trying to split-up a CMOVE
      if( get_ctrl(iff->in(2)) == n_ctrl ||
          get_ctrl(iff->in(3)) == n_ctrl )
        return;                 // Inputs not yet split-up
    }

    // Do not do 'split-if' if some paths are dead.  First do dead code
    // elimination and then see if its still profitable.
    for( uint i = 1; i < n_ctrl->req(); i++ )
      if( n_ctrl->in(i) == C->top() )
        return;

    // When is split-if profitable?  Every 'win' on means some control flow
    // goes dead, so it's almost always a win.
    int policy = 0;
    // If trying to do a 'Split-If' at the loop head, it is only
    // profitable if the cmp folds up on BOTH paths.  Otherwise we
    // risk peeling a loop forever.

    // CNC - Disabled for now.  Requires careful handling of loop
    // body selection for the cloned code.  Also, make sure we check
    // for any input path not being in the same loop as n_ctrl.  For
    // irreducible loops we cannot check for 'n_ctrl->is_Loop()'
    // because the alternative loop entry points won't be converted
    // into LoopNodes.
    IdealLoopTree *n_loop = get_loop(n_ctrl);
    for( uint j = 1; j < n_ctrl->req(); j++ )
      if( get_loop(n_ctrl->in(j)) != n_loop )
        return;

#ifdef _LP64
    // Check for safety of the merge point.
    if( !merge_point_safe(n_ctrl) ) {
      return;
    }
#endif

    // Split compare 'n' through the merge point if it is profitable
    Node *phi = split_thru_phi( n, n_ctrl, policy );
    if( !phi ) return;

    // Found a Phi to split thru!
    // Replace 'n' with the new phi
    _igvn.hash_delete(n);
    _igvn.subsume_node( n, phi );

    // Now split the bool up thru the phi
    Node *bolphi = split_thru_phi( bol, n_ctrl, -1 );
    _igvn.hash_delete(bol);
    _igvn.subsume_node( bol, bolphi );
    assert( iff->in(1) == bolphi, "" );
    if( bolphi->Value(&_igvn)->singleton() ) 
      return;

    // Conditional-move?  Must split up now
    if( !iff->is_If() ) {
      Node *cmovphi = split_thru_phi( iff, n_ctrl, -1 );      
      _igvn.hash_delete(iff);
      _igvn.subsume_node( iff, cmovphi );
      return;
    }

    // Now split the IF
    do_split_if( iff );
    return;
  }

  // Check for an IF ready to split; one that has its 
  // condition codes input coming from a Phi at the block start.
  int n_op = n->Opcode();

  // Check for an IF being dominated by another IF same test
  if( n_op == Op_If ) {
    Node *bol = n->in(1);
    uint max = bol->outcnt();
    // Check for same test used more than once?
    if( n_op == Op_If && max > 1 && bol->is_Bool() ) { 
      // Search up IDOMs to see if this IF is dominated.
      Node *cutoff = get_ctrl(bol);

      // Now search up IDOMs till cutoff, looking for a dominating test
      Node *prevdom = n;
      Node *dom = idom(prevdom);
      while( dom != cutoff ) {
        if( dom->req() > 1 && dom->in(1) == bol && prevdom->in(0) == dom ) {
          // Replace the dominated test with an obvious true or false.
          // Place it on the IGVN worklist for later cleanup.
          C->set_major_progress();
          dominated_by( prevdom, n );
#ifndef PRODUCT
          if( VerifyLoopOptimizations ) verify();
#endif
          return;
        }
        prevdom = dom;
        dom = idom(prevdom);
      }
    }
  }

  // See if a shared loop-varying computation has no loop-varying uses.
  // Happens if something is only used for JVM state in uncommon trap exits,
  // like various versions of induction variable+offset.  Clone the 
  // computation per usage to allow it to sink out of the loop.
  if (has_ctrl(n) && !n->in(0)) {// n not dead and has no control edge (can float about)
    IdealLoopTree *n_loop = get_loop(get_ctrl(n));
    if( n_loop != _ltree_root ) {
      DUIterator_Fast imax, i = n->fast_outs(imax);
      for (; i < imax; i++) {
        Node* u = n->fast_out(i);
        if( !has_ctrl(u) )     break; // Found control user
        IdealLoopTree *u_loop = get_loop(get_ctrl(u));
        if( u_loop == n_loop ) break; // Found loop-varying use
        if( n_loop->is_member( u_loop ) ) break; // Found use in inner loop
        if( u->Opcode() == Op_Opaque1 ) break; // Found loop limit, bugfix for 4677003
      }
      bool did_break = (i < imax);  // Did we break out of the previous loop?
      if (!did_break && n->outcnt() > 1) { // All uses in outer loops!
        for (DUIterator_Last jmin, j = n->last_outs(jmin); j >= jmin; ) {
          Node *u = n->last_out(j); // Clone private computation per use
          _igvn.hash_delete(u);
          _igvn._worklist.push(u);
          Node *x = n->clone(); // Clone computation
          if( u->is_Phi() ) {
            // Replace all uses of normal nodes.  Replace Phi uses 
            // individually, so the seperate Nodes can sink down
            // different paths.
            uint k = 1;
            while( u->in(k) != n ) k++;
            // x goes next to Phi input path
            Node *u_ctrl = place_near_use(u->in(0)->in(k));
            if( x->is_Load() ) 
              u_ctrl = set_ctrl_for_Load(x, u_ctrl);
            register_new_node( x, u_ctrl );
            u->set_req( k, x ); 
            --j;
          } else {              // Normal use
            // Assume 'x' goes next to use but not inside inner loops.
            // For inner loop uses move 'x' to the preheader area.
            Node *u_ctrl = place_near_use(get_ctrl(u));
            if( x->is_Load() ) 
              u_ctrl = set_ctrl_for_Load(x, u_ctrl);
            register_new_node( x, u_ctrl );
            for( uint k = 0; k < u->req(); k++ )
              if( u->in(k) == n ) { // Replace all uses
                u->set_req( k, x );
                --j;
              }
          }

          // Some institutional knowledge is needed here: 'x' is
          // yanked because if the optimizer runs GVN on it all the
          // cloned x's will common up and undo this optimization and
          // be forced back in the loop.  This is annoying because it
          // makes +VerifyOpto report false-positives on progress.  I
          // tried setting control edges on the x's to force them to
          // not combine, but the matching gets worried when it tries
          // to fold a StoreP and an AddP together (as part of an
          // address expression) and the AddP and StoreP have
          // different controls.
          if( !x->is_Load() ) _igvn._worklist.yank(x);
        }
        _igvn.remove_dead_node(n);
      }
    }
  }

  // Check for Opaque2's who's loop has disappeared - who's input is in the
  // same loop nest as their output.  Remove 'em, they are no longer useful.
  if( n_op == Op_Opaque2 && 
      get_loop(get_ctrl(n)) == get_loop(get_ctrl(n->in(1))) ) {
    _igvn.add_users_to_worklist(n);
    _igvn.hash_delete(n);
    _igvn.subsume_node( n, n->in(1) );
  }
}

//------------------------------split_if_with_blocks---------------------------
// Check for aggressive application of 'split-if' optimization,
// using basic block level info.
void PhaseIdealLoop::split_if_with_blocks( VectorSet &visited, Node_Stack &nstack ) {
  Node *n = C->root();
  visited.set(n->_idx); // first, mark node as visited
  // Do pre-visit work for root
  n = split_if_with_blocks_pre( n );
  uint cnt = n->outcnt();
  uint i   = 0;
  while (true) {
    // Visit all children
    if (i < cnt) {
      Node* use = n->raw_out(i);
      ++i;
      if (use->outcnt() != 0 && !visited.test_set(use->_idx)) {
        // Now do pre-visit work for this use
        use = split_if_with_blocks_pre( use );
        nstack.push(n, i); // Save parent and next use's index.
        n   = use;         // Process all children of current use.
        cnt = use->outcnt();
        i   = 0;
      }
    }
    else {
      // All of n's children have been processed, complete post-processing.
      if (cnt != 0) {
        assert(has_node(n), "no dead nodes");
        split_if_with_blocks_post( n );
      }
      if (nstack.is_empty()) {
        // Finished all nodes on stack. 
        break;
      }
      // Get saved parent node and next use's index. Visit the rest of uses.
      n   = nstack.node(); 
      cnt = n->outcnt();
      i   = nstack.index();
      nstack.pop();
    }
  }
}


//=============================================================================
//
//                   C L O N E   A   L O O P   B O D Y
//

//------------------------------clone_iff--------------------------------------
// Passed in a Phi merging (recursively) some nearly equivalent Bool/Cmps.
// "Nearly" because all Nodes have been cloned from the original in the loop,
// but the fall-in edges to the Cmp are different.  Clone bool/Cmp pairs 
// through the Phi recursively, and return a Bool.
BoolNode *PhaseIdealLoop::clone_iff( PhiNode *phi, IdealLoopTree *loop ) {

  // Convert this Phi into a Phi merging Bools
  uint i;
  for( i = 1; i < phi->req(); i++ ) {
    Node *b = phi->in(i);
    PhiNode *pp = b->is_Phi();
    if( pp ) {
      _igvn.hash_delete(phi);
      _igvn._worklist.push(phi);
      phi->set_req(i, clone_iff( pp, loop ));
    } else {
      assert( b->is_Bool(), "" );
    }
  }

  Node *sample_bool = phi->in(1);
  Node *sample_cmp  = sample_bool->in(1);

  // Make Phis to merge the Cmp's inputs.
  PhiNode *phi1 = new PhiNode( phi->in(0), Type::TOP );
  PhiNode *phi2 = new PhiNode( phi->in(0), Type::TOP );
  for( i = 1; i < phi->req(); i++ ) {
    Node *n1 = phi->in(i)->in(1)->in(1);
    Node *n2 = phi->in(i)->in(1)->in(2);
    phi1->set_req( i, n1 );
    phi2->set_req( i, n2 );
    phi1->set_type( phi1->type()->meet(n1->bottom_type()) );
    phi2->set_type( phi2->type()->meet(n2->bottom_type()) );
  }
  // See if these Phis have been made before.
  // Register with optimizer
  Node *hit1 = _igvn.hash_find_insert(phi1);
  if( hit1 ) {                  // Hit, toss just made Phi
    _igvn.remove_dead_node(phi1); // Remove new phi
    assert( hit1->is_Phi(), "" );
    phi1 = (PhiNode*)hit1;      // Use existing phi
  } else {                      // Miss
    _igvn.register_new_node_with_optimizer(phi1);
  }
  Node *hit2 = _igvn.hash_find_insert(phi2);
  if( hit2 ) {                  // Hit, toss just made Phi
    _igvn.remove_dead_node(phi2); // Remove new phi
    assert( hit2->is_Phi(), "" );
    phi2 = (PhiNode*)hit2;      // Use existing phi
  } else {                      // Miss
    _igvn.register_new_node_with_optimizer(phi2);
  }
  // Register Phis with loop/block info
  set_ctrl(phi1, phi->in(0));
  set_ctrl(phi2, phi->in(0));
  // Make a new Cmp
  Node *cmp = sample_cmp->clone();
  cmp->set_req( 1, phi1 );
  cmp->set_req( 2, phi2 );
  _igvn.register_new_node_with_optimizer(cmp);
  set_ctrl(cmp, phi->in(0));

  // Make a new Bool
  Node *b = sample_bool->clone();
  b->set_req(1,cmp);
  _igvn.register_new_node_with_optimizer(b);
  set_ctrl(b, phi->in(0));
  
  assert( b->is_Bool(), "" );
  return (BoolNode*)b;
}

//------------------------------clone_bool-------------------------------------
// Passed in a Phi merging (recursively) some nearly equivalent Bool/Cmps.
// "Nearly" because all Nodes have been cloned from the original in the loop,
// but the fall-in edges to the Cmp are different.  Clone bool/Cmp pairs 
// through the Phi recursively, and return a Bool.
CmpNode *PhaseIdealLoop::clone_bool( PhiNode *phi, IdealLoopTree *loop ) {
  uint i;
  // Convert this Phi into a Phi merging Bools
  for( i = 1; i < phi->req(); i++ ) {
    Node *b = phi->in(i);
    PhiNode *pp = b->is_Phi();
    if( pp ) {
      _igvn.hash_delete(phi);
      _igvn._worklist.push(phi);
      phi->set_req(i, clone_bool( pp, loop ));
    } else {
      assert( b->is_Cmp() || b->is_top(), "inputs are all Cmp or TOP" );
    }
  }

  Node *sample_cmp = phi->in(1);

  // Make Phis to merge the Cmp's inputs.
  PhiNode *phi1 = new PhiNode( phi->in(0), Type::TOP );
  PhiNode *phi2 = new PhiNode( phi->in(0), Type::TOP );
  for( uint j = 1; j < phi->req(); j++ ) {
    Node *cmp_top = phi->in(j); // Inputs are all Cmp or TOP
    Node *n1, *n2;
    if( cmp_top->is_Cmp() ) {
      n1 = cmp_top->in(1);
      n2 = cmp_top->in(2);
    } else {
      n1 = n2 = cmp_top;
    }
    phi1->set_req( j, n1 );
    phi2->set_req( j, n2 );
    phi1->set_type( phi1->type()->meet(n1->bottom_type()) );
    phi2->set_type( phi2->type()->meet(n2->bottom_type()) );
  }

  // See if these Phis have been made before.
  // Register with optimizer
  Node *hit1 = _igvn.hash_find_insert(phi1);
  if( hit1 ) {                  // Hit, toss just made Phi
    _igvn.remove_dead_node(phi1); // Remove new phi
    assert( hit1->is_Phi(), "" );
    phi1 = (PhiNode*)hit1;      // Use existing phi
  } else {                      // Miss
    _igvn.register_new_node_with_optimizer(phi1);  
  }
  Node *hit2 = _igvn.hash_find_insert(phi2);
  if( hit2 ) {                  // Hit, toss just made Phi
    _igvn.remove_dead_node(phi2); // Remove new phi
    assert( hit2->is_Phi(), "" );
    phi2 = (PhiNode*)hit2;      // Use existing phi
  } else {                      // Miss
    _igvn.register_new_node_with_optimizer(phi2);  
  }
  // Register Phis with loop/block info
  set_ctrl(phi1, phi->in(0));
  set_ctrl(phi2, phi->in(0));
  // Make a new Cmp
  Node *cmp = sample_cmp->clone();
  cmp->set_req( 1, phi1 );
  cmp->set_req( 2, phi2 );
  _igvn.register_new_node_with_optimizer(cmp);
  set_ctrl(cmp, phi->in(0));

  assert( cmp->is_Cmp(), "" );
  return (CmpNode*)cmp;
}

//------------------------------sink_use---------------------------------------
// If 'use' was in the loop-exit block, it now needs to be sunk
// below the post-loop merge point.
void PhaseIdealLoop::sink_use( Node *use, Node *post_loop ) {
  if (!use->is_CFG() && get_ctrl(use) == post_loop->in(2)) {
    set_ctrl(use, post_loop);
    for (DUIterator j = use->outs(); use->has_out(j); j++)
      sink_use(use->out(j), post_loop);
  }
}

//------------------------------clone_loop-------------------------------------
//
//                   C L O N E   A   L O O P   B O D Y
//
// This is the basic building block of the loop optimizations.  It clones an
// entire loop body.  It makes an old_new loop body mapping; with this mapping
// you can find the new-loop equivalent to an old-loop node.  All new-loop 
// nodes are exactly equal to their old-loop counterparts, all edges are the 
// same.  All exits from the old-loop now have a RegionNode that merges the
// equivalent new-loop path.  This is true even for the normal "loop-exit"
// condition.  All uses of loop-invariant old-loop values now come from (one
// or more) Phis that merge their new-loop equivalents.  
//
// This operation leaves the graph in an illegal state: there are two valid 
// control edges coming from the loop pre-header to both loop bodies.  I'll
// definitely have to hack the graph after running this transform.
//
// From this building block I will further edit edges to perform loop peeling
// or loop unrolling or iteration splitting (Range-Check-Elimination), etc.
void PhaseIdealLoop::clone_loop( IdealLoopTree *loop, Node_List &old_new, int dd ) {

  // Step 1: Clone the loop body.  Make the old->new mapping.
  uint i;
  for( i = 0; i < loop->_body.size(); i++ ) {
    Node *old = loop->_body.at(i);
    Node *nnn = old->clone();
    old_new.map( old->_idx, nnn );
    _igvn.register_new_node_with_optimizer(nnn);
  }


  // Step 2: Fix the edges in the new body.  If the old input is outside the 
  // loop use it.  If the old input is INside the loop, use the corresponding
  // new node instead.
  for( i = 0; i < loop->_body.size(); i++ ) {
    Node *old = loop->_body.at(i);
    Node *nnn = old_new[old->_idx];
    // Fix CFG/Loop controlling the new node
    if (has_ctrl(old)) {
      set_ctrl(nnn, old_new[get_ctrl(old)->_idx]);
    } else {
      set_loop(nnn, loop->_parent);
      if (old->outcnt() > 0) {
        set_idom( nnn, old_new[idom(old)->_idx], dd );
      }
    }
    // Correct edges to the new node
    for( uint j = 0; j < nnn->req(); j++ ) {
        Node *n = nnn->in(j);
        if( n ) {
          IdealLoopTree *old_in_loop = get_loop( has_ctrl(n) ? get_ctrl(n) : n );
          if( loop->is_member( old_in_loop ) )
            nnn->set_req(j, old_new[n->_idx]);
        }
    }
    _igvn.hash_find_insert(nnn);
  }
  Node *newhead = old_new[loop->_head->_idx];
  set_idom(newhead, newhead->in(LoopNode::EntryControl), dd);

  
  // Step 3: Now fix control uses.  Loop varying control uses have already
  // been fixed up (as part of all input edges in Step 2).  Loop invariant
  // control uses must be either an IfFalse or an IfTrue.  Make a merge
  // point to merge the old and new IfFalse/IfTrue nodes; make the use 
  // refer to this.
  ResourceArea *area = Thread::current()->resource_area();
  Node_List worklist(area);
  uint new_counter = C->unique();
  for( i = 0; i < loop->_body.size(); i++ ) {
    Node* old = loop->_body.at(i);
    if( !old->is_CFG() ) continue;
    Node* nnn = old_new[old->_idx];

    // Copy uses to a worklist, so I can munge the def-use info
    // with impunity.
    for (DUIterator_Fast jmax, j = old->fast_outs(jmax); j < jmax; j++)
      worklist.push(old->fast_out(j));
    
    while( worklist.size() ) {  // Visit all uses
      Node *use = worklist.pop();
      if (!has_node(use))  continue; // Ignore dead nodes
      IdealLoopTree *use_loop = get_loop( has_ctrl(use) ? get_ctrl(use) : use );
      if( !loop->is_member( use_loop ) && use->is_CFG() ) {
        // Both OLD and USE are CFG nodes here.
        assert( use->is_Proj(), "" );

        // Clone the loop exit control projection
        Node *newuse = use->clone();
        newuse->set_req(0,nnn);
        _igvn.register_new_node_with_optimizer(newuse);
        set_loop(newuse, use_loop);
        set_idom(newuse, nnn, dom_depth(nnn) );

        // We need a Region to merge the exit from the peeled body and the
        // exit from the old loop body.
        RegionNode *r = new RegionNode(3);
        // Map the old use to the new merge point
        old_new.map( use->_idx, r );
        uint dd_r = MIN2(dom_depth(newuse),dom_depth(use));
        assert( dd_r >= dom_depth(dom_lca(newuse,use)), "" );

        // The original user of 'use' uses 'r' instead.
        for (DUIterator_Last lmin, l = use->last_outs(lmin); l >= lmin; --l) {
          Node* useuse = use->last_out(l);
          _igvn.hash_delete(useuse);
          _igvn._worklist.push(useuse);
          if( useuse->in(0) == use ) {
            useuse->set_req(0, r);
            if( useuse->is_CFG() ) {
              assert( dom_depth(useuse) > dd_r, "" );
              set_idom(useuse, r, dom_depth(useuse));
            }
          }
          for( uint k = 1; k < useuse->req(); k++ )
            if( useuse->in(k) == use )
              useuse->set_req(k, r);
        }

        // Now finish up 'r'       
        r->set_req( 1, newuse );
        r->set_req( 2,    use );
        _igvn.register_new_node_with_optimizer(r);
        set_loop(r, use_loop);
        set_idom(r, newuse->in(0), dd_r);
      } // End of if a loop-exit test
    }
  }

  // Step 4: If loop-invariant use is not control, it must be dominated by a
  // loop exit IfFalse/IfTrue.  Find "proper" loop exit.  Make a Region
  // there if needed.  Make a Phi there merging old and new used values.
  Node_List *split_if_set = NULL;
  Node_List *split_bool_set = NULL;
  Node_List *split_cex_set = NULL;
  for( i = 0; i < loop->_body.size(); i++ ) {
    Node* old = loop->_body.at(i);
    Node* nnn = old_new[old->_idx];
    // Copy uses to a worklist, so I can munge the def-use info
    // with impunity.
    for (DUIterator_Fast jmax, j = old->fast_outs(jmax); j < jmax; j++)
      worklist.push(old->fast_out(j));

    while( worklist.size() ) {
      Node *use = worklist.pop();
      if (!has_node(use))  continue; // Ignore dead nodes
      if (use->in(0) == C->top())  continue;
      IdealLoopTree *use_loop = get_loop( has_ctrl(use) ? get_ctrl(use) : use );
      // Check for data-use outside of loop - at least one of OLD or USE
      // must not be a CFG node.
      if( !loop->is_member( use_loop ) && (!old->is_CFG() || !use->is_CFG())) {

        // If the Data use is an IF, that means we have an IF outside of the
        // loop that is switching on a condition that is set inside of the 
        // loop.  Happens if people set a loop-exit flag; then test the flag
        // in the loop to break the loop, then test is again outside of the
        // loop to determine which way the loop exited.
        if( use->is_If() || use->is_CMove() ) {
          // Since this code is highly unlikely, we lazily build the worklist
          // of such Nodes to go split.
          if( !split_if_set ) 
            split_if_set = new Node_List(area);
          split_if_set->push(use);
        }
        if( use->is_Bool() ) {
          if( !split_bool_set ) 
            split_bool_set = new Node_List(area);
          split_bool_set->push(use);
        }
        if( use->Opcode() == Op_CreateEx ) {
          if( !split_cex_set ) 
            split_cex_set = new Node_List(area);
          split_cex_set->push(use);
        }


        // Get "block" use is in
        uint idx = 0; 
        while( use->in(idx) != old ) idx++;
        Node *prev = use->is_CFG() ? use : get_ctrl(use);
        assert( !loop->is_member( get_loop( prev ) ), "" );
        Node *cfg = prev->_idx >= new_counter 
          ? prev->in(2) 
          : idom(prev);
        if( use->is_Phi() )     // Phi use is in prior block
          cfg = prev->in(idx);  // NOT in block of Phi itself
        if (cfg->is_top()) {    // Use is dead?
          _igvn.hash_delete(use);
          _igvn._worklist.push(use);
          use->set_req(idx, C->top());
          continue;
        }

        while( !loop->is_member( get_loop( cfg ) ) ) {
          prev = cfg;
          cfg = cfg->_idx >= new_counter ? cfg->in(2) : idom(cfg);
        }
        // If the use occurs after merging several exits from the loop, then
        // old value must have dominated all those exits.  Since the same old 
        // value was used on all those exits we did not need a Phi at this
        // merge point.  NOW we do need a Phi here.  Each loop exit value
        // is now merged with the peeled body exit; each exit gets its own
        // private Phi and those Phis need to be merged here.
        Node *phi;
        if( prev->is_Region() ) {
          if( idx == 0 ) {      // Updating control edge?  
            phi = prev;         // Just use existing control
          } else {              // Else need a new Phi
            phi = PhiNode::make( prev, old );
            // Now recursively fix up the new uses of old!
            for( uint i = 1; i < prev->req(); i++ ) {
              worklist.push(phi); // Onto worklist once for each 'old' input
            }
          }
        } else {
          // Get new RegionNode merging old and new loop exits
          prev = old_new[prev->_idx];
          assert( prev, "just made this in step 7" );
          if( idx == 0 ) {      // Updating control edge?  
            phi = prev;         // Just use existing control
          } else {              // Else need a new Phi
            // Make a new Phi merging data values properly
            phi = PhiNode::make( prev, old );
            phi->set_req( 1, nnn );
          }
        }
        // If inserting a new Phi, check for prior hits
        if( idx != 0 ) {
          Node *hit = _igvn.hash_find_insert(phi);
          if( hit == NULL ) { 
           _igvn.register_new_node_with_optimizer(phi); // Register new phi
          } else {                                      // or
            // Remove the new phi from the graph and use the hit
            _igvn.remove_dead_node(phi);
            phi = hit;                                  // Use existing phi
          }
          set_ctrl(phi, prev);
        }
        // Make 'use' use the Phi instead of the old loop body exit value
        _igvn.hash_delete(use);
        _igvn._worklist.push(use);
        use->set_req(idx, phi);
        if( use->_idx >= new_counter ) { // If updating new phis
          // Not needed for correctness, but prevents a weak assert
          // in AddPNode from tripping (when we end up with different
          // base & derived Phis that will become the same after
          // IGVN does CSE).
          Node *hit = _igvn.hash_find_insert(use);
          if( hit )             // Go ahead and re-hash for hits.
            _igvn.subsume_node( use, hit );
        }
        
        // If 'use' was in the loop-exit block, it now needs to be sunk
        // below the post-loop merge point.
        sink_use( use, prev );
      }
    }
  }

  // Check for IFs that need splitting/cloning.  Happens if an IF outside of
  // the loop uses a condition set in the loop.  The original IF probably
  // takes control from one or more OLD Regions (which in turn get from NEW
  // Regions).  In any case, there will be a set of Phis for each merge point
  // from the IF up to where the original BOOL def exists the loop.
  if( split_if_set ) {
    while( split_if_set->size() ) {
      Node *iff = split_if_set->pop();
      PhiNode *phi = iff->in(1)->is_Phi();
      if( phi ) {
        BoolNode *b = clone_iff( phi, loop );
        _igvn.hash_delete(iff);
        _igvn._worklist.push(iff);
        iff->set_req(1, b);
      }
    }
  }
  if( split_bool_set ) {
    while( split_bool_set->size() ) {
      Node *b = split_bool_set->pop();
      Node *phi = b->in(1);
      assert( phi->is_Phi(), "" );
      CmpNode *cmp = clone_bool( (PhiNode*)phi, loop );
      _igvn.hash_delete(b);
      _igvn._worklist.push(b);
      b->set_req(1, cmp);
    }
  }
  if( split_cex_set ) {
    while( split_cex_set->size() ) {
      Node *b = split_cex_set->pop();
      assert( b->in(0)->is_Region(), "" );
      assert( b->in(1)->is_Phi(), "" );
      assert( b->in(0)->in(0) == b->in(1)->in(0), "" );
      split_up( b, b->in(0), NULL );
    }
  }

}

//------------------------------reorg_offsets----------------------------------
// Reorganize offset computations to lower register pressure.  Mostly
// prevent loop-fallout uses of the pre-incremented trip counter (which are
// then alive with the post-incremented trip counter forcing an extra
// register move)
void PhaseIdealLoop::reorg_offsets( IdealLoopTree *loop ) {

  assert( loop->_head->is_CountedLoop(), "" );
  CountedLoopNode *cl = (CountedLoopNode*)loop->_head;
  CountedLoopEndNode *cle = cl->loopexit();
  if( !cle ) return;            // The occasional dead loop
  // Find loop exit control
  Node *exit = cle->proj_out(false);
  assert( exit->Opcode() == Op_IfFalse, "" );

  // Check for the special case of folks using the pre-incremented
  // trip-counter on the fall-out path (forces the pre-incremented
  // and post-incremented trip counter to be live at the same time).
  // Fix this by adjusting to use the post-increment trip counter.
  Node *phi = cl->phi();
  if( !phi ) return;            // Dead infinite loop
  bool progress = true;
  while (progress) {
    progress = false;
    for (DUIterator_Fast imax, i = phi->fast_outs(imax); i < imax; i++) {
      Node* use = phi->fast_out(i);   // User of trip-counter
      if (!has_ctrl(use))  continue;
      Node *u_ctrl = get_ctrl(use);
      if( use->is_Phi() ) {
        u_ctrl = NULL;
        for( uint j = 1; j < use->req(); j++ )
          if( use->in(j) == phi )
            u_ctrl = dom_lca( u_ctrl, use->in(0)->in(j) );
      }
      IdealLoopTree *u_loop = get_loop(u_ctrl);
      // Look for loop-invariant use
      if( u_loop == loop ) continue;
      if( loop->is_member( u_loop ) ) continue;
      // Check that use is live out the bottom.  Assuming the trip-counter
      // update is right at the bottom, uses of of the loop middle are ok.
      if( dom_lca( exit, u_ctrl ) != exit ) continue;
      // protect against stride not being a constant
      if( !cle->stride_is_con() ) continue;
      // Hit!  Refactor use to use the post-incremented tripcounter.
      // Compute a post-increment tripcounter.
      Node *opaq = new (2) Opaque2Node( cle->incr() );
      register_new_node( opaq, u_ctrl );
      Node *post = new (3) AddINode( opaq, _igvn.intcon(-cle->stride_con()));
      register_new_node( post, u_ctrl );
      _igvn.hash_delete(use);
      _igvn._worklist.push(use);
      for( uint j = 1; j < use->req(); j++ )
        if( use->in(j) == phi )
          use->set_req(j, post);
      // Since DU info changed, rerun loop
      progress = true;
      break;
    }
  }

}

//=============================================================================
//------------------------------reorg_offsets----------------------------------
// Reorganize uses of 'q' and 'q+con' such that only 1 is live at a time.  
// This lowers register pressure.  I can map a use of 'q' to 'q+con' by
// subtracting 'con' and vice-versa.  Partition the uses of 'q' and 'q+con'
// into 'soft' and 'hard' uses.  Soft uses can be moved between 'q' and 
// 'q+con' with little (zero?) cost.  Hard uses cost a full add are not worth 
// moving.  Soft uses are all uses outside the current use (the extra add or 
// sub is also outside the loop), addressing expressions (extra offset folds in 
// cheaply) or CmpI.  Hard uses define hard limits on motion.

// Next determine where the 'q+con' add goes.  Moving it above 'q's hard uses
// is not worth it.  Moving the add below the add's hard limit is not worth 
// it.  These limits define a range to move the add in.  Move the add to 
// the add's hard limit.  If this is below or at 'q's hard limit then we can
// make progress.  
//
// Look at each soft use: if the use occurs before the add's block, make the
// use come from 'q'; if the use occurs after the add's block make the use
// come from the add.  If the use occurs in the add's block, make it come 
// from 'q' and assume the block can be scheduled so the add is late.

  // Check for constant offsets being alive at the same time as the base
  // and reorganize.  Changes "A[i++]" code like this:
  //   -OptoReorgOffsets        +OptoReorgOffsets
  //
  //   MOV   AX,BX              CMPu  BX,[range]
  //   INC   AX                 Jge   range-check
  //   CMPu  BX,[range]         INC   BX
  //   Jge   range-check
  //   MOV   BX,AX
  //   ...                    range-check:
  // range-check:               INC   BX
  //   CALL  uncommon_trap      CALL  uncommon_trap
  //
  // In the few loops I studied hard I saw the transform remove one or
  // two copies and 1 or 2 spills.  On the Specs it appears to slow down
  // all Specs by 0.25% to 0.5% - within noise but always in the wrong
  // direction.  It causes CaffeineMarks 'loop' benchmark to slow down
  // by 10% - but it only moves a few instructions around.  In other words,
  // the scheduling change was worth a 10% loss!  Turned off for now.
  // CNC 7/30/1999
/*
static Node *get_use_blk( Node *use, uint uidx, PhaseIdealLoop *phase ) {
  return use->is_Phi() ? use->in(0)->in(uidx) : phase->get_ctrl(use);
}

static bool is_soft_op( Node *use ) {
  uint uop = use->Opcode();
  switch( uop ) {
  case Op_AddP:
  case Op_AddI:
  case Op_CmpI:
    return true;
  }
  return false;
}

// Compute LCA of hard uses
static Node *hard_uses(Node *x, IdealLoopTree *loop, PhaseIdealLoop *phase) {
  Node *lca = NULL;
  int max = x->outcnt();
  for( int i = 0; i < max; i++ ) {  // Check all uses
    Node* use = x->out(i);
    if( is_soft_op( use ) )     // Soft use?  
      continue;                 // Skip it
    // Check for hard use(s)
    if( use->is_Phi() ) {
      for( uint j = 1; j < use->req(); j++ ) 
        if( use->in(j) == x ) {
          Node *use_blk = use->in(0)->in(j);
          if( use_blk->is_top() ) continue;
          if( phase->is_member( loop, use_blk ) )
            lca = phase->dom_lca( lca, use_blk );
        }
    } else {                     // Normal (non-phi) use
      Node *use_blk = use->is_CFG() ? use : phase->get_ctrl(use);
      if( phase->is_member( loop, use_blk ) )
        lca = phase->dom_lca( lca, use_blk );
    }
  }
  return lca;
}

// Move this use to either 'add' or 'q'.
static void alter_use( Node *use, uint idx, Node *use_blk, Node *x, Node *x_blk, Node *con, IdealLoopTree *loop, PhaseIdealLoop *phase, PhaseIterGVN *igvn, const char *msg ) {
#ifndef PRODUCT
  if( PrintOpto ) tty->print( msg );
#endif
  igvn->hash_delete( use );

  // Is use a CmpI, with a loop-invariant other input?
  // Then move adjustment out of the loop
  if( use->Opcode() == Op_CmpI ) {
    Node *li = use->in(3-idx);  // The loop-invariant other input
    Node *li_blk = li->is_CFG() ? li : phase->get_ctrl(i);
    if( !phase->is_member( loop, li_blk ) ) {
#ifndef PRODUCT
      if( PrintOpto ) tty->print("(cmp twist)");
#endif
      // Subtract from L.I.
      Node *sub = new (3) SubINode( li, con );
      igvn->register_new_node_with_optimizer( sub );
      // Locate subtract next to L.I.
      phase->set_ctrl(sub, li_blk);
      // Use SUB instead of L.I.
      use->set_req(3-idx, sub);
      // Use X instead of previous add
      use->set_req(idx, x);
      igvn->_worklist.push( use );
      return;
    }
  } 

  // 'opaque' is used to prevent the optimizer from undoing my 
  // remapping of uses.
  Node *opaque = new (2) OpaqueNode( x );
  igvn->register_new_node_with_optimizer( opaque );
  phase->set_ctrl(opaque, x_blk);

  Node *new_add = new (3) AddINode( opaque, con );
  igvn->register_new_node_with_optimizer( new_add );
  phase->set_ctrl(new_add, use_blk);
  use->set_req(idx, new_add);
  igvn->_worklist.push( use );
}

static void do_remap_soft_use(Node *use, uint idx, Node *use_blk, Node *q, Node *add, Node *q_ctrl, Node *a_hard, IdealLoopTree *loop, PhaseIdealLoop *phase, PhaseIterGVN *igvn) {

  if( a_hard == phase->dom_lca( use_blk, a_hard ) && use != add ) {
    // Use will come from 'add'
    if( use->in(idx) == q ) { // Use comes from 'q', must move it
      Node *conode = ((PhaseGVN*)igvn)->intcon( -add->in(2)->get_int() );
      alter_use( use, idx, use_blk, add, a_hard, conode, loop, phase, igvn, ", move to add" );
    }
  } else {
    // Use will come from 'q'
    if( use->in(idx) == add ) {   // Use comes from 'add', must move it  
      alter_use( use, idx, use_blk, q, q_ctrl, add->in(2), loop, phase, igvn, ", move to q" );
    }
  }
}

// Remap soft uses ONLY.  Hard uses (ones that cost to relocate) remain put.
// Soft uses are relocated to either 'q' or 'add' from their current location
// by adding or subtracting the appropriate constant.
static void remap_soft_uses(Node *x, Node *q, Node *add, Node *q_ctrl, Node *a_hard, IdealLoopTree *loop, PhaseIdealLoop *phase, PhaseIterGVN *igvn) {
  Node *uses[256];
  int max  = x->outcnt();
  if( max > 256 ) max = 256;
  for( int j = 0; j < max; j++) // Copy all uses
    uses[j] = x->out(j);

  for( int i = 0; i < max; i++ ) {  // Check all uses
    Node *use = uses[i];
    // Check for hard use(s)
    if( use->is_Phi() ) {       // Phis must check all inputs
      for( uint j = 1; j < use->req(); j++ ) 
        if( use->in(j) == x ) {
          Node *use_blk = use->in(0)->in(j);
          if( use_blk->is_top() ) continue;
          if( !phase->is_member( loop, use_blk ) )
            do_remap_soft_use(use,j,use_blk,q,add,q_ctrl,a_hard,loop,phase,igvn);
        }
    } else {                    // Normal (non-phi) use
      Node *use_blk = use->is_CFG() ? use : phase->get_ctrl(use);
      if( !use_blk ) continue;  // The occasional dead node
      uint j;
      for( j = 1; j < use->req(); j++ ) 
        if( use->in(j) == x ) 
          break;
      if( !phase->is_member( loop, use_blk ) || is_soft_op( use ) )
        do_remap_soft_use(use,j,use_blk,q,add,q_ctrl,a_hard,loop,phase,igvn);
    }
  }
}

void IdealLoopTree::reorg_offsets( Node *add, PhaseIdealLoop *phase ) {
  PhaseIterGVN *igvn = &phase->_igvn;

  // The 'q' being added
  Node *q = add->in(1);

  // Compute hard limit - the LCA of hard uses
  Node *q_hard = hard_uses(   q, this, phase );
  Node *a_hard = hard_uses( add, this, phase );
  Node *q_ctrl = phase->get_ctrl(q);
  Node *a_ctrl = phase->get_ctrl(add);

  // Lift a_hard out of a loop back edge, but not past 'q'
  if( a_hard == _head->in(LoopNode::LoopBackControl) &&
      q_ctrl != a_hard &&       // Do not try to slip past 'q'
      q_ctrl != a_hard->in(0) &&
      q_ctrl != a_hard->in(0)->in(0) )
    a_hard = a_hard->in(0)->in(0);

  // Check for being forced to have q live past add
  // because q's hard uses are dominated by add's hard uses.
  if( q_hard ) {
    Node *n = q_hard;
    while( n != q_ctrl ) {
      n = phase->idom(n);
      if( n == a_hard )         // q MUST overlap add's lifetime
        return;                 // No progress
    }
  }

  // Move the add to its hard limit.  If it actually moves then call
  // 'remap_soft_uses'.  Remapping soft uses makes uses that are
  // above the add's new location use 'q' instead.
  if( a_hard != a_ctrl ) {      // Add is going to move?
#ifndef PRODUCT
    tty->print("sinking add");
#endif
    phase->set_ctrl(add, a_hard);
    remap_soft_uses( add, q, add, q_ctrl, a_hard, this, phase, igvn );
  }
  // Now remap all q's soft uses to use add if possible or q otherwise
  remap_soft_uses( q, q, add, q_ctrl, a_hard, this, phase, igvn );

#ifndef PRODUCT
  tty->cr();
#endif
}
*/

