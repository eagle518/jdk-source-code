#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)loopTransform.cpp	1.92 04/06/04 09:07:32 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_loopTransform.cpp.incl"

//------------------------------is_loop_exit-----------------------------------
// Given an IfNode, return the loop-exiting projection or NULL if both 
// arms remain in the loop.
Node *IdealLoopTree::is_loop_exit( Node *iff, PhaseIdealLoop *phase ) const {
  if( iff->outcnt() != 2 ) return NULL; // Ignore partially dead tests

  // Test is an IfNode, has 2 projections.  If BOTH are in the loop
  // we need loop unswitching instead of peeling.
  if( !is_member(phase->get_loop( iff->raw_out(0) )) )
    return iff->raw_out(0);
  if( !is_member(phase->get_loop( iff->raw_out(1) )) )
    return iff->raw_out(1);
  return NULL;
}


//=============================================================================

//------------------------------policy_peeling---------------------------------
// Return TRUE or FALSE if the loop should be peeled or not.  Peel if we can
// make some loop-invariant test (usually a null-check) happen before the loop.
bool IdealLoopTree::policy_peeling( PhaseIdealLoop *phase ) const {
  Node *test = ((IdealLoopTree*)this)->tail();
  int  body_size = ((IdealLoopTree*)this)->_body.size();
  int  uniq      = phase->C->unique();
  // Peeling does loop cloning which can result in O(N^2) node construction
  if( body_size > 255 /* Prevent overflow for large body_size */
      || (body_size * body_size + uniq > MaxNodeLimit) ) {
    return false;           // too large to safely clone
  }
  while( test != _head ) {      // Scan till run off top of loop
    if( test->is_If() ) {       // Test?
      Node *ctrl = phase->get_ctrl(test->in(1));
      if (ctrl->is_top())
        return false;           // Found dead test on live IF?  No peeling!
      // Standard IF only has one input value to check for loop invariance
      assert( test->Opcode() == Op_If || test->Opcode() == Op_CountedLoopEnd, "Check this code when new subtype is added");
      // Condition is not a member of this loop?
      if( !is_member(phase->get_loop(ctrl)) &&
          is_loop_exit(test,phase) )
        return true;            // Found reason to peel!
    }
    // Walk up dominators to loop _head looking for test which is
    // executed on every path thru loop.
    test = phase->idom(test);
  }
  return false;
}

//------------------------------peeled_dom_test_elim---------------------------
// If we got the effect of peeling, either by actually peeling or by making
// a pre-loop which must execute at least once, we can remove all 
// loop-invariant dominated tests in the main body.
void PhaseIdealLoop::peeled_dom_test_elim( IdealLoopTree *loop, Node_List &old_new ) {
  bool progress = true;
  while( progress ) {
    progress = false;           // Reset for next iteration
    Node *prev = loop->_head->in(LoopNode::LoopBackControl);//loop->tail();
    Node *test = prev->in(0);
    while( test != loop->_head ) { // Scan till run off top of loop
      
      int p_op = prev->Opcode();
      if( (p_op == Op_IfFalse || p_op == Op_IfTrue) &&
          test->is_If() &&      // Test?
          !test->in(1)->is_Con() && // And not already obvious?
          // Condition is not a member of this loop?
          !loop->is_member(get_loop(get_ctrl(test->in(1))))){
        // Walk loop body looking for instances of this test
        for( uint i = 0; i < loop->_body.size(); i++ ) {
          Node *n = loop->_body.at(i);
          IfNode *iff = n->is_If();
          if( iff && iff->in(1) == test->in(1) /*&& iff != loop->tail()->in(0)*/ ) {
            // IfNode was dominated by version in peeled loop body
            progress = true;
            dominated_by( old_new[prev->_idx], iff );
          }
        }
      }
      prev = test;
      test = idom(test);
    } // End of scan tests in loop

  } // End of while( progress )
}

//------------------------------do_peeling-------------------------------------
// Peel the first iteration of the given loop.  
// Step 1: Clone the loop body.  The clone becomes the peeled iteration.
//         The pre-loop illegally has 2 control users (old & new loops).
// Step 2: Make the old-loop fall-in edges point to the peeled iteration.
//         Do this by making the old-loop fall-in edges act as if they came
//         around the loopback from the prior iteration (follow the old-loop
//         backedges) and then map to the new peeled iteration.  This leaves
//         the pre-loop with only 1 user (the new peeled iteration), but the
//         peeled-loop backedge has 2 users.
// Step 3: Cut the backedge on the clone (so its not a loop) and remove the
//         extra backedge user.
void PhaseIdealLoop::do_peeling( IdealLoopTree *loop, Node_List &old_new ) {

  C->set_major_progress();
  // Peeling a 'main' loop in a pre/main/post situation obfuscates the
  // 'pre' loop from the main and the 'pre' can no longer have it's
  // iterations adjusted.  Therefore, we need to declare this loop as
  // no longer a 'main' loop; it will need new pre and post loops before
  // we can do further RCE.
  CountedLoopNode *cl = loop->_head->is_CountedLoop();
  if( cl ) {
    assert(cl->trip_count() > 0, "peeling a fully unrolled loop");
    cl->set_trip_count(cl->trip_count() - 1);
    if( cl->is_main_loop() ) {
      cl->set_normal_loop();
#ifndef PRODUCT
      if( PrintOpto && VerifyLoopOptimizations ) {
        tty->print("Peeling a 'main' loop; resetting to 'normal' ");
        loop->dump_head();
      }
#endif
    }
  }

  // Step 1: Clone the loop body.  The clone becomes the peeled iteration.
  //         The pre-loop illegally has 2 control users (old & new loops).
  clone_loop( loop, old_new, dom_depth(loop->_head) );


  // Step 2: Make the old-loop fall-in edges point to the peeled iteration.
  //         Do this by making the old-loop fall-in edges act as if they came
  //         around the loopback from the prior iteration (follow the old-loop
  //         backedges) and then map to the new peeled iteration.  This leaves
  //         the pre-loop with only 1 user (the new peeled iteration), but the
  //         peeled-loop backedge has 2 users.
  for (DUIterator_Fast jmax, j = loop->_head->fast_outs(jmax); j < jmax; j++) {
    Node* old = loop->_head->fast_out(j);
    if( old->in(0) == loop->_head && old->req() == 3 &&
        (old->is_Loop() || old->is_Phi()) ) {
      Node *new_exit_value = old_new[old->in(LoopNode::LoopBackControl)->_idx];
      if( !new_exit_value )     // Backedge value is ALSO loop invariant?
        // Then loop body backedge value remains the same.
        new_exit_value = old->in(LoopNode::LoopBackControl);
      _igvn.hash_delete(old);
      old->set_req(LoopNode::EntryControl, new_exit_value);
    }
  }
  

  // Step 3: Cut the backedge on the clone (so its not a loop) and remove the
  //         extra backedge user.
  Node *nnn = old_new[loop->_head->_idx];
  _igvn.hash_delete(nnn);
  nnn->set_req(LoopNode::LoopBackControl, C->top());
  for (DUIterator_Fast j2max, j2 = nnn->fast_outs(j2max); j2 < j2max; j2++) {
    Node* use = nnn->fast_out(j2);
    if( use->in(0) == nnn && use->req() == 3 && use->is_Phi() ) {
      _igvn.hash_delete(use);
      use->set_req(LoopNode::LoopBackControl, C->top());
    }
  }


  // Step 4: Correct dom-depth info.  Set to loop-head depth.
  int dd = dom_depth(loop->_head);
  set_idom(loop->_head, loop->_head->in(1), dd);
  for (uint j3 = 0; j3 < loop->_body.size(); j3++) {
    Node *old = loop->_body.at(j3);
    Node *nnn = old_new[old->_idx];
    if (!has_ctrl(nnn))
      set_idom(nnn, idom(nnn), dd-1);
    // While we're at it, remove any SafePoints from the peeled code
    if( old->Opcode() == Op_SafePoint ) {
      Node *nnn = old_new[old->_idx];
      lazy_replace(nnn,nnn->in(TypeFunc::Control));
    }
  }

  // Now force out all loop-invariant dominating tests.  The optimizer 
  // finds some, but we _know_ they are all useless.
  peeled_dom_test_elim(loop,old_new);
   
}

//------------------------------policy_maximally_unroll------------------------
// Return exact loop trip count, or 0 if not maximally unrolling
bool IdealLoopTree::policy_maximally_unroll( PhaseIdealLoop *phase ) const {
  assert( _head->is_CountedLoop(), "" );
  CountedLoopNode *cl = (CountedLoopNode*)_head;
  assert( cl->is_normal_loop(), "" );

  Node *init = cl->init_trip();
  Node *limit = cl->limit();

  // Non-constant bounds
  if( init   == NULL || !init->is_Con()  ||
      limit  == NULL || !limit->is_Con() ||
      // protect against stride not being a constant
      !cl->stride_is_con() ) {
    return false;
  }
  int span = limit->get_int() - init->get_int();
  int stride = cl->stride_con();
  uint trip_count = span/stride;   // trip_count can be greater than 2 Gig.
  assert( (int)trip_count*stride == span, "must divide evenly" );
  // Note: A span of zero implies a trip_count of zero and this is OK.  We'll
  // return a zero (no maximally unroll) and the regular unroll/peel route
  // will make a small mess which CCP will fold away.

  // Real policy: if we maximally unroll, does it get too big?
  // Allow the unrolled mess to get larger than standard loop
  // size.  After all, it will no longer be a loop.
  uint body_size    = _body.size();
  uint unroll_limit = (uint)LoopUnrollLimit * 4;
  assert( (intx)unroll_limit == LoopUnrollLimit * 4, "LoopUnrollLimit must fit in 32bits");
  cl->set_trip_count(trip_count);
  if( trip_count <= unroll_limit && body_size <= unroll_limit &&
      (body_size * trip_count <= unroll_limit) ) {
    return true;    // maximally unroll
  }

  return false;               // Do not maximally unroll
}


//------------------------------policy_unroll----------------------------------
// Return TRUE or FALSE if the loop should be unrolled or not.  Unroll if 
// the loop is a CountedLoop and the body is small enough.
bool IdealLoopTree::policy_unroll( PhaseIdealLoop *phase ) const {

  assert( _head->is_CountedLoop(), "" );
  CountedLoopNode *cl = (CountedLoopNode*)_head;
  assert( cl->is_normal_loop() || cl->is_main_loop(), "" );

  // protect against stride not being a constant
  if( !cl->stride_is_con() ) return false;

  // protect against over-unrolling
  if( cl->trip_count() <= 1 ) return false;

  // Adjust body_size to determine if we unroll or not
  uint body_size = _body.size();
  // Key test to unroll CaffeineMark's Logic test
  int xors_in_loop = 0;
  // Also count ModL, DivL and MulL which expand mightly
  for( uint k = 0; k < _body.size(); k++ ) {
    switch( _body.at(k)->Opcode() ) {
    case Op_XorI: xors_in_loop++; break; // CaffeineMark's Logic test
    case Op_ModL: body_size += 30; break;
    case Op_DivL: body_size += 30; break;
    case Op_MulL: body_size += 10; break;
    }
  }

  // Check for being too big
  if( body_size > (uint)LoopUnrollLimit ) { 
    if( xors_in_loop >= 4 && body_size < (uint)LoopUnrollLimit*4) return true;
    // Normal case: loop too big
    return false;
  }
  
  // Check for stride being a small enuf constant
  if( abs(cl->stride_con()) > (1<<3) ) return false;

  // Unroll once!  (Each trip will soon do double iterations)
  return true;
}

//------------------------------policy_align-----------------------------------
// Return TRUE or FALSE if the loop should be cache-line aligned.  Gather the
// expression that does the alignment.  Note that only one array base can be
// aligned in a loop (unless the VM guarentees mutual alignment).  Note that
// if we vectorize short memory ops into longer memory ops, we may want to
// increase alignment.
bool IdealLoopTree::policy_align( PhaseIdealLoop *phase ) const {
  return false;
}

//------------------------------policy_range_check-----------------------------
// Return TRUE or FALSE if the loop should be range-check-eliminated.
// Actually we do iteration-splitting, a more powerful form of RCE.
bool IdealLoopTree::policy_range_check( PhaseIdealLoop *phase ) const {
  if( !RangeCheckElimination ) return false;

  assert( _head->is_CountedLoop(), "" );
  CountedLoopNode *cl = (CountedLoopNode*)_head;
  // If we unrolled with no intention of doing RCE and we later
  // changed our minds, we got no pre-loop.  Either we need to
  // make a new pre-loop, or we gotta disallow RCE.
  if( cl->is_main_no_pre_loop() ) return false; // Disallowed for now.
  Node *trip_counter = cl->phi();

  // Check loop body for tests of trip-counter plus loop-invariant vs
  // loop-invariant.
  for( uint i = 0; i < _body.size(); i++ ) {
    Node *iff = _body[i];
    if( iff->Opcode() == Op_If ) { // Test?

      // Comparing trip+off vs limit
      Node *bol = iff->in(1);
      if( bol->req() != 2 ) continue; // dead constant test
      Node *cmp = bol->in(1);

      Node *limit = cmp->in(2);

      Node *limit_c = phase->get_ctrl(limit);
      if( limit_c == phase->C->top() ) 
        return false;           // Found dead test on live IF?  No RCE!
      if( is_member(phase->get_loop(limit_c) ) )
        continue;               // Loop-varying limit

      Node *add = cmp->in(1);
      if( add == trip_counter ) {
      } else {
        if( add->Opcode() != Op_AddI && add->Opcode() != Op_SubI ) continue; // Odd compare
        Node *off;              // Try to find trip+offset
        if( add->in(1) == trip_counter ) 
          off = add->in(2);     // Found offset
        else if( add->in(2) == trip_counter )
          off = add->in(1);     // Found offset
        else continue;          // NOT trip+offset

        Node *off_c = phase->get_ctrl(off);
        if( off_c == phase->C->top() ) 
          return false;         // Found dead test on live IF?  No RCE!
        if( is_member(phase->get_loop(off_c) ) )
          continue;             // Offset is not really loop invariant
      } 
      // Yeah!  Found a test like 'trip+off vs limit'
      // Test is an IfNode, has 2 projections.  If BOTH are in the loop
      // we need loop unswitching instead of iteration splitting.
      if( is_loop_exit(iff,phase) )
        return true;            // Found reason to split iterations
    } // End of is IF
  }

  return false;
}

//------------------------------policy_peel_only-------------------------------
// Return TRUE or FALSE if the loop should NEVER be RCE'd or aligned.  Useful
// for unrolling loops with NO array accesses.
bool IdealLoopTree::policy_peel_only( PhaseIdealLoop *phase ) const {

  for( uint i = 0; i < _body.size(); i++ )
    if( _body[i]->is_Mem() )
      return false;

  // No memory accesses at all!
  return true;
}

//------------------------------clone_up_backedge_goo--------------------------
// If Node n lives in the back_ctrl block and cannot float, we clone a private 
// version of n in preheader_ctrl block and return that, otherwise return n.
Node *PhaseIdealLoop::clone_up_backedge_goo( Node *back_ctrl, Node *preheader_ctrl, Node *n ) {
  if( get_ctrl(n) != back_ctrl ) return n;

  Node *x = NULL;               // If required, a clone of 'n'
  // Check for 'n' being pinned in the backedge.
  if( n->in(0) && n->in(0) == back_ctrl ) {
    x = n->clone();             // Clone a copy of 'n' to preheader
    x->set_req( 0, preheader_ctrl ); // Fix x's control input to preheader
  }

  // Recursive fixup any other input edges into x.
  // If there are no changes we can just return 'n', otherwise
  // we need to clone a private copy and change it.
  for( uint i = 1; i < n->req(); i++ ) {
    Node *g = clone_up_backedge_goo( back_ctrl, preheader_ctrl, n->in(i) );
    if( g != n->in(i) ) {
      if( !x )
        x = n->clone();
      x->set_req(i, g);
    }
  }
  if( x ) {                     // x can legally float to pre-header location
    register_new_node( x, preheader_ctrl );
    return x;
  } else {                      // raise n to cover LCA of uses
    set_ctrl( n, back_ctrl->in(0) );
  }
  return n;
}

//------------------------------insert_pre_post_loops--------------------------
// Insert pre and post loops.  If peel_only is set, the pre-loop can not have
// more iterations added.  It acts as a 'peel' only, no lower-bound RCE, no
// alignment.  Useful to unroll loops that do no array accesses.
void PhaseIdealLoop::insert_pre_post_loops( IdealLoopTree *loop, Node_List &old_new, bool peel_only ) {

  C->set_major_progress();

  // Find common pieces of the loop being guarded with pre & post loops
  assert( loop->_head->is_CountedLoop(), "" );
  CountedLoopNode *main_head = (CountedLoopNode*)loop->_head;
  assert( main_head->is_normal_loop(), "" );
  CountedLoopEndNode *main_end = main_head->loopexit();
  assert( main_end->outcnt() == 2, "1 true, 1 false path only" );
  uint dd_main_head = dom_depth(main_head);
  uint max = main_head->outcnt();

  Node *pre_header= main_head->in(LoopNode::EntryControl);
  Node *init      = main_head->init_trip();
  Node *incr      = main_end ->incr();
  Node *limit     = main_end ->limit();
  Node *stride    = main_end ->stride();
  Node *cmp       = main_end ->cmp_node();
  BoolTest::mask b_test = main_end->test_trip();

  // Need only 1 user of 'bol' because I will be hacking the loop bounds.
  Node *bol = main_end->in(CountedLoopEndNode::TestValue);
  if( bol->outcnt() != 1 ) {
    bol = bol->clone();
    register_new_node(bol,main_end->in(CountedLoopEndNode::TestControl));
    _igvn.hash_delete(main_end);
    main_end->set_req(CountedLoopEndNode::TestValue, bol);
  }
  // Need only 1 user of 'cmp' because I will be hacking the loop bounds.
  if( cmp->outcnt() != 1 ) {
    cmp = cmp->clone();
    register_new_node(cmp,main_end->in(CountedLoopEndNode::TestControl));
    _igvn.hash_delete(bol);
    bol->set_req(1, cmp);
  }

  //------------------------------
  // Step A: Create Post-Loop.
  Node* main_exit = main_end->proj_out(false);
  assert( main_exit->Opcode() == Op_IfFalse, "" );
  int dd_main_exit = dom_depth(main_exit);

  // Step A1: Clone the loop body.  The clone becomes the post-loop.  The main
  // loop pre-header illegally has 2 control users (old & new loops).
  clone_loop( loop, old_new, dd_main_exit );
  assert( old_new[main_head->_idx]->is_CountedLoop   (), "" );
  assert( old_new[main_end ->_idx]->Opcode() == Op_CountedLoopEnd, "" );
  CountedLoopNode    *post_head = (CountedLoopNode   *)old_new[main_head->_idx];
  post_head->set_post_loop(main_head);

  // Build the main-loop normal exit.
  IfFalseNode *new_main_exit = new (1) IfFalseNode(main_end);
  _igvn.register_new_node_with_optimizer( new_main_exit );
  set_idom(new_main_exit, main_end, dd_main_exit );
  set_loop(new_main_exit, loop->_parent);

  // Step A2: Build a zero-trip guard for the post-loop.  After leaving the
  // main-loop, the post-loop may not execute at all.  We 'opaque' the incr
  // (the main-loop trip-counter exit value) because we will be changing
  // the exit value (via unrolling) so we cannot constant-fold away the zero
  // trip guard until all unrolling is done.
  Node *zer_opaq = new (2) Opaque1Node(incr);
  Node *zer_cmp  = new (3) CmpINode( zer_opaq, limit );
  Node *zer_bol  = new (2) BoolNode( zer_cmp, b_test );
  register_new_node( zer_opaq, new_main_exit );
  register_new_node( zer_cmp , new_main_exit );
  register_new_node( zer_bol , new_main_exit );

  // Build the IfNode
  IfNode *zer_iff = new (2) IfNode( new_main_exit, zer_bol, PROB_FAIR, COUNT_UNKNOWN );
  _igvn.register_new_node_with_optimizer( zer_iff );
  set_idom(zer_iff, new_main_exit, dd_main_exit);
  set_loop(zer_iff, loop->_parent);

  // Plug in the false-path, taken if we need to skip post-loop
  _igvn.hash_delete( main_exit );
  main_exit->set_req(0, zer_iff);
  _igvn._worklist.push(main_exit);
  set_idom(main_exit, zer_iff, dd_main_exit);
  set_idom(main_exit->unique_out(), zer_iff, dd_main_exit);
  // Make the true-path, must enter the post loop
  Node *zer_taken = new (1) IfTrueNode( zer_iff );
  _igvn.register_new_node_with_optimizer( zer_taken );
  set_idom(zer_taken, zer_iff, dd_main_exit);
  set_loop(zer_taken, loop->_parent);
  // Plug in the true path
  _igvn.hash_delete( post_head );
  post_head->set_req(LoopNode::EntryControl, zer_taken);
  set_idom(post_head, zer_taken, dd_main_exit);

  // Step A3: Make the fall-in values to the post-loop come from the
  // fall-out values of the main-loop.
  for (DUIterator_Fast imax, i = main_head->fast_outs(imax); i < imax; i++) {
    Node* main_phi = main_head->fast_out(i);
    if( main_phi->is_Phi() && main_phi->in(0) == main_head ) {
      Node *post_phi = old_new[main_phi->_idx];
      Node *fallmain  = clone_up_backedge_goo(main_head->back_control(),
                                              post_head->init_control(),
                                              main_phi->in(LoopNode::LoopBackControl));
      _igvn.hash_delete(post_phi);
      post_phi->set_req( LoopNode::EntryControl, fallmain );
    }
  }

  // Update local caches for next stanza
  main_exit = new_main_exit;


  //------------------------------
  // Step B: Create Pre-Loop.

  // Step B1: Clone the loop body.  The clone becomes the pre-loop.  The main
  // loop pre-header illegally has 2 control users (old & new loops).
  clone_loop( loop, old_new, dd_main_head );
  assert( old_new[main_head->_idx]->is_CountedLoop   (), "" );
  assert( old_new[main_end ->_idx]->Opcode() == Op_CountedLoopEnd, "" );
  CountedLoopNode    *pre_head = (CountedLoopNode   *)old_new[main_head->_idx];
  CountedLoopEndNode *pre_end  = (CountedLoopEndNode*)old_new[main_end ->_idx];
  pre_head->set_pre_loop(main_head);
  Node *pre_incr = old_new[incr->_idx];

  // Find the pre-loop normal exit.
  Node* pre_exit = pre_end->proj_out(false);
  assert( pre_exit->Opcode() == Op_IfFalse, "" );
  IfFalseNode *new_pre_exit = new (1) IfFalseNode(pre_end);
  _igvn.register_new_node_with_optimizer( new_pre_exit );
  set_idom(new_pre_exit, pre_end, dd_main_head);
  set_loop(new_pre_exit, loop->_parent);

  // Step B2: Build a zero-trip guard for the main-loop.  After leaving the
  // pre-loop, the main-loop may not execute at all.  Later in life this
  // zero-trip guard will become the minimum-trip guard when we unroll
  // the main-loop.
  Node *min_opaq = new (2) Opaque1Node(limit);
  Node *min_cmp  = new (3) CmpINode( pre_incr, min_opaq );
  Node *min_bol  = new (2) BoolNode( min_cmp, b_test );
  register_new_node( min_opaq, new_pre_exit );
  register_new_node( min_cmp , new_pre_exit );
  register_new_node( min_bol , new_pre_exit );

  // Build the IfNode
  IfNode *min_iff = new (2) IfNode( new_pre_exit, min_bol, PROB_FAIR, COUNT_UNKNOWN );
  _igvn.register_new_node_with_optimizer( min_iff );
  set_idom(min_iff, new_pre_exit, dd_main_head);
  set_loop(min_iff, loop->_parent);

  // Plug in the false-path, taken if we need to skip main-loop
  _igvn.hash_delete( pre_exit );
  pre_exit->set_req(0, min_iff);
  set_idom(pre_exit, min_iff, dd_main_head);
  set_idom(pre_exit->unique_out(), min_iff, dd_main_head);
  // Make the true-path, must enter the main loop
  Node *min_taken = new (1) IfTrueNode( min_iff );
  _igvn.register_new_node_with_optimizer( min_taken );
  set_idom(min_taken, min_iff, dd_main_head);
  set_loop(min_taken, loop->_parent);
  // Plug in the true path
  _igvn.hash_delete( main_head );
  main_head->set_req(LoopNode::EntryControl, min_taken);
  set_idom(main_head, min_taken, dd_main_head);

  // Step B3: Make the fall-in values to the main-loop come from the
  // fall-out values of the pre-loop.
  for (DUIterator_Fast i2max, i2 = main_head->fast_outs(i2max); i2 < i2max; i2++) {
    Node* main_phi = main_head->fast_out(i2);
    if( main_phi->is_Phi() && main_phi->in(0) == main_head ) {
      Node *pre_phi = old_new[main_phi->_idx];
      Node *fallpre  = clone_up_backedge_goo(pre_head->back_control(),
                                             main_head->init_control(),
                                             pre_phi->in(LoopNode::LoopBackControl));
      _igvn.hash_delete(main_phi);
      main_phi->set_req( LoopNode::EntryControl, fallpre );
    }
  }

  // Step B4: Shorten the pre-loop to run only 1 iteration (for now).
  // RCE and alignment may change this later.
  Node *cmp_end = pre_end->cmp_node();
  assert( cmp_end->in(2) == limit, "" );
  Node *pre_limit = new (3) AddINode( init, stride );
  Node *pre_opaq  = new (2) Opaque1Node(pre_limit);
  register_new_node( pre_limit, pre_head->in(0) );
  register_new_node( pre_opaq , pre_head->in(0) );

  // Since no other users of pre-loop compare, I can hack limit directly
  assert( cmp_end->outcnt() == 1, "no other users" );
  _igvn.hash_delete(cmp_end);
  cmp_end->set_req(2, peel_only ? pre_limit : pre_opaq);

  // Flag main loop
  main_head->set_main_loop();
  if( peel_only ) main_head->set_main_no_pre_loop();

  // Now force out all loop-invariant dominating tests.  The optimizer 
  // finds some, but we _know_ they are all useless.
  peeled_dom_test_elim(loop,old_new);
}


//------------------------------do_unroll--------------------------------------
// Unroll the loop body one step - make each trip do 2 iterations.
void PhaseIdealLoop::do_unroll( IdealLoopTree *loop, Node_List &old_new, bool adjust_min_trip ) {
  assert( LoopUnrollLimit, "" );
#ifndef PRODUCT
  if( PrintOpto && VerifyLoopOptimizations ) {
    tty->print("Unrolling ");
    loop->dump_head();
  }
#endif
  assert( loop->_head->is_CountedLoop(), "" );
  CountedLoopNode *loop_head = (CountedLoopNode*)loop->_head;
  CountedLoopEndNode *loop_end = loop_head->loopexit();
  assert( loop_end, "" );

  Node *ctrl  = loop_head->in(LoopNode::EntryControl);
  Node *limit = loop_head->limit();
  Node *init  = loop_head->init_trip();
  Node *strid = loop_head->stride();

  Node *opaq = NULL;
  if( adjust_min_trip ) {       // If not maximally unrolling, need adjustment
    assert( loop_head->is_main_loop(), "" );
    assert( ctrl->Opcode() == Op_IfTrue || ctrl->Opcode() == Op_IfFalse, "" );
    Node *iff = ctrl->in(0);
    assert( iff->Opcode() == Op_If, "" );
    Node *bol = iff->in(1);
    assert( bol->Opcode() == Op_Bool, "" );
    Node *cmp = bol->in(1);
    assert( cmp->Opcode() == Op_CmpI, "" );
    opaq = cmp->in(2);
    // Occasionally it's possible for a pre-loop Opaque1 node to be
    // optimized away and then another round of loop opts attempted.
    // We can not optimize this particular loop in that case.
    if( opaq->Opcode() != Op_Opaque1 )
      return;                   // Cannot find pre-loop!  Bail out!
  }

  C->set_major_progress();

  // Adjust max trip count. The trip count is intentionally rounded
  // down here (e.g. 15-> 7-> 3-> 1) because if we unwittingly over-unroll,
  // the main, unrolled, part of the loop will never execute as it is protected
  // by the min-trip test.  See bug 4834191 for a case where we over-unrolled
  // and later determined that part of the unrolled loop was dead.
  loop_head->set_trip_count(loop_head->trip_count() / 2);

  // -----------
  // Step 2: Cut back the trip counter for an unroll amount of 2.
  // Loop will normally trip (limit - init)/stride_con.  Since it's a
  // CountedLoop this is exact (stride divides limit-init exactly).
  // We are going to double the loop body, so we want to knock off any
  // odd iteration: (trip_cnt & ~1).  Then back compute a new limit.
  Node *span = new (3) SubINode( limit, init );
  register_new_node( span, ctrl );
  Node *trip = new (3) DivINode( 0, span, strid );
  register_new_node( trip, ctrl );
  Node *mtwo = _igvn.intcon(-2);
  set_ctrl(mtwo, C->root());
  Node *rond = new (3) AndINode( trip, mtwo );
  register_new_node( rond, ctrl );
  Node *spn2 = new (3) MulINode( rond, strid );
  register_new_node( spn2, ctrl );
  Node *lim2 = new (3) AddINode( spn2, init );
  register_new_node( lim2, ctrl );

  // Hammer in the new limit
  Node *ctrl2 = loop_end->in(0);
  Node *cmp2 = new (3) CmpINode( loop_head->incr(), lim2 );
  register_new_node( cmp2, ctrl2 );
  Node *bol2 = new (2) BoolNode( cmp2, loop_end->test_trip() );
  register_new_node( bol2, ctrl2 );
  _igvn.hash_delete(loop_end);
  loop_end->set_req(CountedLoopEndNode::TestValue, bol2);

  // Step 3: Find the min-trip test guaranteed before a 'main' loop.
  // Make it a 1-trip test (means at least 2 trips).
  if( adjust_min_trip ) {
    // Guard test uses an 'opaque' node which is not shared.  Hence I
    // can edit it's inputs directly.  Hammer in the new limit for the
    // minimum-trip guard.
    assert( opaq->outcnt() == 1, "" );
    _igvn.hash_delete(opaq);
    opaq->set_req(1, lim2);
  }

  // ---------
  // Step 4: Clone the loop body.  Move it inside the loop.  This loop body 
  // represents the odd iterations; since the loop trips an even number of
  // times its backedge is never taken.  Kill the backedge.
  uint dd = dom_depth(loop_head);
  clone_loop( loop, old_new, dd );

  // Make backedges of the clone equal to backedges of the original.
  // Make the fall-in from the original come from the fall-out of the clone.
  for (DUIterator_Fast jmax, j = loop_head->fast_outs(jmax); j < jmax; j++) {
    Node* phi = loop_head->fast_out(j);
    if( phi->is_Phi() && phi->in(0) == loop_head ) {
      Node *newphi = old_new[phi->_idx];
      _igvn.hash_delete( phi );
      _igvn.hash_delete( newphi );

      phi   ->set_req(LoopNode::   EntryControl, newphi->in(LoopNode::LoopBackControl));
      newphi->set_req(LoopNode::LoopBackControl, phi   ->in(LoopNode::LoopBackControl));
      phi   ->set_req(LoopNode::LoopBackControl, C->top());
    }
  }  
  Node *clone_head = old_new[loop_head->_idx];
  _igvn.hash_delete( clone_head );
  loop_head ->set_req(LoopNode::   EntryControl, clone_head->in(LoopNode::LoopBackControl));
  clone_head->set_req(LoopNode::LoopBackControl, loop_head ->in(LoopNode::LoopBackControl));
  loop_head ->set_req(LoopNode::LoopBackControl, C->top());
  loop->_head = clone_head;     // New loop header

  set_idom(loop_head,  loop_head ->in(LoopNode::EntryControl), dd);
  set_idom(clone_head, clone_head->in(LoopNode::EntryControl), dd);

  // Kill the clone's backedge
  Node *newcle = old_new[loop_end->_idx];
  _igvn.hash_delete( newcle );
  Node *one = _igvn.intcon(1);
  set_ctrl(one, C->root());
  newcle->set_req(1, one);
  // Force clone into same loop body
  uint max = loop->_body.size();
  for( uint k = 0; k < max; k++ ) {
    Node *old = loop->_body.at(k);
    Node *nnn = old_new[old->_idx];
    loop->_body.push(nnn);
    if (!has_ctrl(old))
      set_loop(nnn, loop);
  }
}

//------------------------------do_maximally_unroll----------------------------

void PhaseIdealLoop::do_maximally_unroll( IdealLoopTree *loop, Node_List &old_new ) {
  assert( loop->_head->is_CountedLoop(), "" );
  CountedLoopNode *cl = (CountedLoopNode*)loop->_head;
  assert( cl->trip_count() > 0, "");

  // If loop is tripping an odd number of times, peel odd iteration
  if( (cl->trip_count() & 1) == 1 ) {
    do_peeling( loop, old_new );
  }

  // Now its tripping an even number of times remaining.  Double loop body.
  // Do not adjust pre-guards; they are not needed and do not exist.
  if( cl->trip_count() > 0 ) { 
    do_unroll( loop, old_new, false );
  }
}

//------------------------------add_constraint---------------------------------
// Constrain the main loop iterations so the condition:
//    scale_con * I + offset  <  limit
// always holds true.  That is, either increase the number of iterations in
// the pre-loop or the post-loop until the condition holds true in the main 
// loop.  Stride, scale, offset and limit are all loop invariant.  Further, 
// stride and scale are constants (offset and limit often are).
void PhaseIdealLoop::add_constraint( int stride_con, int scale_con, Node *offset, Node *limit, Node *pre_ctrl, Node **pre_limit, Node **main_limit ) {

  // Compute "I :: (limit-offset)/scale_con"
  Node *con = new (3) SubINode( limit, offset );
  register_new_node( con, pre_ctrl );
  Node *scale = _igvn.intcon(scale_con);
  set_ctrl(scale, C->root());
  Node *X = new (3) DivINode( 0, con, scale );
  register_new_node( X, pre_ctrl );

  // For positive stride, the pre-loop limit always uses a MAX function 
  // and the main loop a MIN function.  For negative stride these are
  // reversed.  
  
  // Also for positive stride*scale the affine function is increasing, so the 
  // pre-loop must check for underflow and the post-loop for overflow.
  // Negative stride*scale reverses this; pre-loop checks for overflow and
  // post-loop for underflow.
  if( stride_con*scale_con > 0 ) {
    // Compute I < (limit-offset)/scale_con
    // Adjust main-loop last iteration to be MIN/MAX(main_loop,X)
    *main_limit = (stride_con > 0) 
      ? (Node*)(new (3) MinINode( *main_limit, X ))
      : (Node*)(new (3) MaxINode( *main_limit, X ));
    register_new_node( *main_limit, pre_ctrl );

  } else {
    // Compute (limit-offset)/scale_con + SGN(-scale_con) <= I
    // Add the negation of the main-loop constraint to the pre-loop.
    // See footnote [++] below for a derivation of the limit expression.
    Node *incr = _igvn.intcon(scale_con > 0 ? -1 : 1);
    set_ctrl(incr, C->root());
    Node *adj = new (3) AddINode( X, incr );
    register_new_node( adj, pre_ctrl );
    *pre_limit = (scale_con > 0)
      ? (Node*)new (3) MinINode( *pre_limit, adj )
      : (Node*)new (3) MaxINode( *pre_limit, adj );
    register_new_node( *pre_limit, pre_ctrl );

//   [++] Here's the algebra that justifies the pre-loop limit expression:
//   
//   NOT( scale_con * I + offset  <  limit )
//      ==
//   scale_con * I + offset  >=  limit
//      ==
//   SGN(scale_con) * I  >=  (limit-offset)/|scale_con|
//      ==
//   (limit-offset)/|scale_con|   <=  I * SGN(scale_con)
//      ==
//   (limit-offset)/|scale_con|-1  <  I * SGN(scale_con)
//      ==
//   ( if (scale_con > 0) /*common case*/
//       (limit-offset)/scale_con - 1  <  I
//     else  
//       (limit-offset)/scale_con + 1  >  I
//    )
//   ( if (scale_con > 0) /*common case*/
//       (limit-offset)/scale_con + SGN(-scale_con)  <  I
//     else  
//       (limit-offset)/scale_con + SGN(-scale_con)  >  I
  }
}

//------------------------------do_range_check---------------------------------
// Eliminate range-checks and other trip-counter vs loop-invariant tests.
void PhaseIdealLoop::do_range_check( IdealLoopTree *loop, Node_List &old_new ) {
#ifndef PRODUCT
  if( PrintOpto && VerifyLoopOptimizations ) {
    tty->print("Range Check Elimination ");
    loop->dump_head();
  }
#endif
  assert( RangeCheckElimination, "" );
  assert( loop->_head->is_CountedLoop(), "" );
  CountedLoopNode *cl = (CountedLoopNode*)loop->_head;
  assert( cl->is_main_loop(), "" );

  // Find the trip counter; we are iteration splitting based on it
  Node *trip_counter = cl->phi();
  // Find the main loop limit; we will trim it's iterations 
  // to not ever trip end tests
  Node *main_limit = cl->limit();
  // Find the pre-loop limit; we will expand it's iterations to
  // not ever trip low tests.
  Node *ctrl  = cl->in(LoopNode::EntryControl);
  assert( ctrl->Opcode() == Op_IfTrue || ctrl->Opcode() == Op_IfFalse, "" );
  Node *iffm = ctrl->in(0);
  assert( iffm->Opcode() == Op_If, "" );
  Node *p_f = iffm->in(0);
  assert( p_f->Opcode() == Op_IfFalse, "" );
  assert( p_f->in(0)->Opcode() == Op_CountedLoopEnd, "" );
  CountedLoopEndNode *pre_end = (CountedLoopEndNode*)p_f->in(0);
  assert( pre_end->loopnode()->is_pre_loop(), "" );
  Node *pre_opaq = pre_end->limit();
  // Occasionally it's possible for a pre-loop Opaque1 node to be
  // optimized away and then another round of loop opts attempted.
  // We can not optimize this particular loop in that case.
  if( pre_opaq->Opcode() != Op_Opaque1 )
    return;
  Node *pre_limit = pre_opaq->in(1);

  // Where do we put new limit calculations
  Node *pre_ctrl = pre_end->loopnode()->in(LoopNode::EntryControl);

  // Need to find the main-loop zero-trip guard
  Node *bolzm = iffm->in(1);
  assert( bolzm->Opcode() == Op_Bool, "" );
  Node *cmpzm = bolzm->in(1);
  assert( cmpzm->is_Cmp(), "" );
  Node *opqzm = cmpzm->in(2);
  if( opqzm->Opcode() != Op_Opaque1 )
    return;
  assert( opqzm->in(1) == main_limit, "do not understand situation" );

  // Must know if its a count-up or count-down loop

  // protect against stride not being a constant
  if ( !cl->stride_is_con() ) {
    return;
  }
  int stride_con = cl->stride_con();
  Node *zero = _igvn.intcon(0); 
  Node *one  = _igvn.intcon(1);
  set_ctrl(zero, C->root());
  set_ctrl(one,  C->root());

  // Check loop body for tests of trip-counter plus loop-invariant vs
  // loop-invariant.
  for( uint i = 0; i < loop->_body.size(); i++ ) {
    Node *iff = loop->_body[i];
    if( iff->Opcode() == Op_If ) { // Test?

      // Test is an IfNode, has 2 projections.  If BOTH are in the loop
      // we need loop unswitching instead of iteration splitting.
      Node *exit = loop->is_loop_exit(iff,this);
      if( !exit ) continue;
      int flip = (exit->Opcode() == Op_IfTrue) ? 1 : 0;

      // Get boolean condition to test
      BoolNode *bol = iff->in(1)->is_Bool();
      if( !bol ) continue;
      BoolTest b_test = bol->_test;
      // Flip sense of test if exit condition is flipped
      if( flip )
        b_test = b_test.negate();

      // Get compare
      Node *cmp = bol->in(1);

      // Look for trip_counter + offset vs limit
      Node *offset = cmp->in(1);
      Node *limit  = cmp->in(2);
      jint scale_con= 1;        // Assume trip counter not scaled

      Node *limit_c = get_ctrl(limit);
      if( loop->is_member(get_loop(limit_c) ) ) {
        // Compare might have operands swapped; commute them
        b_test = b_test.commute();
        offset = cmp->in(2);
        limit  = cmp->in(1);
        limit_c = get_ctrl(limit);
        if( loop->is_member(get_loop(limit_c) ) ) 
          continue;             // Both inputs are loop varying; cannot RCE
      }
      // Here we know 'limit' is loop invariant

      // 'limit' maybe pinned below the zero trip test (probably from a
      // previous round of rce), in which case, it can't be used in the
      // zero trip test expression which must occur before the zero test's if.
      if( limit_c == ctrl ) {
        continue;  // Don't rce this check but continue looking for other candidates.
      }
      if( offset == trip_counter ) {
        offset = zero;          // Use zero offset
      } else if( offset->Opcode() == Op_AddI ) {
        // These pattern matches are not repeated for the Op_SubI below
        // because they are not needed.  IGVN converts 
        //   SubI(MulI(trip_counter, #scale_con),  offset)
        // into
        //   AddI(MulI(trip_counter, #scale_con), -offset)
        // and then falls into the desired pattern.  It requires a trip
        // through IGVN which happens after the loop opts.  The only reason to
        // do it early is to avoid another round of loop-opts, so all I really
        // need to recognize is the patterns that the loop-opts themselves
        // insert.  I specifically pick up on the patterns produced by the
        // induction-variable recognition code.
        if( offset->in(1) == trip_counter ) {
          offset = offset->in(2);       // Found offset
        } else if( offset->in(1)->Opcode() == Op_MulI && // scaled trip ctr?
                   offset->in(1)->in(1) == trip_counter &&
                   offset->in(1)->in(2)->get_int(&scale_con) ) {
          offset = offset->in(2);       // Found offset          
        } else if( offset->in(2) == trip_counter ) {
          offset = offset->in(1);       // Found offset
        } else if( offset->in(2)->Opcode() == Op_MulI && // scaled trip ctr?
                   offset->in(2)->in(1) == trip_counter &&
                   offset->in(2)->in(2)->get_int(&scale_con) ) {
          offset = offset->in(1);       // Found offset          
        } else continue;          // Not really (trip_counter+offset)  
      } else if( offset->Opcode() == Op_SubI) {
        if( offset->in(1) == trip_counter ) {
          Node *ctrl_off2 = get_ctrl(offset->in(2));
          offset = new (3) SubINode( zero, offset->in(2) );// Found offset
          register_new_node( offset, ctrl_off2 );
        } else if( offset->in(2) == trip_counter ) {
          offset = offset->in(1);       // Found offset
          scale_con = -1;       // Flip scale
        } else continue;        // Not really (trip_counter+offset)  
      } else continue;          // Not really (trip_counter+offset) 

      Node *offset_c = get_ctrl(offset);
      if( loop->is_member( get_loop(offset_c) ) )
        continue;               // Offset is not really loop invariant
      // Here we know 'offset' is loop invariant.

      // As above for the 'limit', the 'offset' maybe pinned below the
      // zero trip test.
      if( offset_c == ctrl ) {
        continue; // Don't rce this check but continue looking for other candidates.
      }

      // At this point we have the expression as:
      //   scale_con * trip_counter + offset :: limit
      // where scale_con, offset and limit are loop invariant.  Trip_counter 
      // monotonically increases by stride_con, a constant.  Both (or either) 
      // stride_con and scale_con can be negative which will flip about the 
      // sense of the test.

      // Adjust pre and main loop limits to guard the correct iteration set
      if( cmp->Opcode() == Op_CmpU ) {// Unsigned compare is really 2 tests
        if( b_test._test == BoolTest::lt ) { // Range checks always use lt
          // The overflow limit: scale*I+offset < limit
          add_constraint( stride_con, scale_con, offset, limit, pre_ctrl, &pre_limit, &main_limit );
          // The underflow limit: 0 <= scale*I+offset.
          // Some math yields: -scale*I-(offset+1) < 0
          Node *plus_one = new (3) AddINode( offset, one );
          register_new_node( plus_one, pre_ctrl );
          Node *neg_offset = new (3) SubINode( zero, plus_one );
          register_new_node( neg_offset, pre_ctrl );
          add_constraint( stride_con, -scale_con, neg_offset, zero, pre_ctrl, &pre_limit, &main_limit );
        } else {
#ifndef PRODUCT
          if( PrintOpto ) 
            tty->print_cr("missed RCE opportunity");
#endif
          continue;             // In release mode, ignore it
        }
      } else {                  // Otherwise work on normal compares
        switch( b_test._test ) {
        case BoolTest::ge:      // Convert X >= Y to -X <= -Y
          scale_con = -scale_con;
          offset = new (3) SubINode( zero, offset );
          register_new_node( offset, pre_ctrl );
          limit  = new (3) SubINode( zero, limit  );
          register_new_node( limit, pre_ctrl );
          // Fall into LE case
        case BoolTest::le:      // Convert X <= Y to X < Y+1
          limit = new (3) AddINode( limit, one );
          register_new_node( limit, pre_ctrl );
          // Fall into LT case
        case BoolTest::lt: 
          add_constraint( stride_con, scale_con, offset, limit, pre_ctrl, &pre_limit, &main_limit );
          break;
        default:
#ifndef PRODUCT
          if( PrintOpto ) 
            tty->print_cr("missed RCE opportunity");
#endif
          continue;             // Unhandled case
        }
      }

      // Kill the eliminated test
      C->set_major_progress();
      Node *kill_con = _igvn.intcon( 1-flip );
      set_ctrl(kill_con, C->root());
      _igvn.hash_delete(iff);
      iff->set_req(1, kill_con);
      _igvn._worklist.push(iff);
      // Find surviving projection
      assert(iff->is_If(), "");
      ProjNode* dp = ((IfNode*)iff)->proj_out(1-flip);
      // Find loads off the surviving projection; remove their control edge
      for (DUIterator_Fast imax, i = dp->fast_outs(imax); i < imax; i++) {
        Node* cd = dp->fast_out(i); // Control-dependent node
        if( cd->is_Load() ) {   // Loads can now float around in the loop
          _igvn.hash_delete(cd);
          // Allow the load to float around in the loop, or before it
          // but NOT before the pre-loop.
          cd->set_req(0, ctrl);   // ctrl, not NULL
          _igvn._worklist.push(cd);
          --i;
          --imax;
        }
      }

    } // End of is IF

  }

  // Update loop limits
  _igvn.hash_delete(pre_opaq);
  pre_opaq->set_req(1, pre_limit);

  // Note:: we are making the main loop limit no longer precise;
  // need to round up based on stride.
  if( stride_con != 1 && stride_con != -1 ) { // Cutout for common case
    // "Standard" round-up logic:  ([main_limit-init+(y-1)]/y)*y+init
    // Hopefully, compiler will optimize for powers of 2.
    Node *ctrl = get_ctrl(main_limit);
    Node *stride = cl->stride();
    Node *init = cl->init_trip();
    Node *span = new (3) SubINode(main_limit,init);
    register_new_node(span,ctrl);
    Node *rndup = _igvn.intcon(stride_con + ((stride_con>0)?-1:1));
    Node *add = new (3) AddINode(span,rndup);
    register_new_node(add,ctrl);
    Node *div = new (3) DivINode(0,add,stride);
    register_new_node(div,ctrl);
    Node *mul = new (3) MulINode(div,stride);
    register_new_node(mul,ctrl);
    Node *newlim = new (3) AddINode(mul,init);
    register_new_node(newlim,ctrl);
    main_limit = newlim;
  }

  Node *main_cle = cl->loopexit();
  Node *main_bol = main_cle->in(1);
  // Hacking loop bounds; need private copies of exit test
  if( main_bol->outcnt() > 1 ) {// BoolNode shared?
    _igvn.hash_delete(main_cle);
    main_bol = main_bol->clone();// Clone a private BoolNode
    register_new_node( main_bol, main_cle->in(0) );
    main_cle->set_req(1,main_bol);
  }
  Node *main_cmp = main_bol->in(1);
  if( main_cmp->outcnt() > 1 ) { // CmpNode shared?
    _igvn.hash_delete(main_bol);
    main_cmp = main_cmp->clone();// Clone a private CmpNode
    register_new_node( main_cmp, main_cle->in(0) );
    main_bol->set_req(1,main_cmp);
  }
  // Hack the now-private loop bounds
  _igvn.hash_delete(main_cmp);
  main_cmp->set_req(2, main_limit);
  _igvn._worklist.push(main_cmp);
  // The OpaqueNode is unshared by design
  _igvn.hash_delete(opqzm);
  assert( opqzm->outcnt() == 1, "cannot hack shared node" );
  opqzm->set_req(1,main_limit);
  _igvn._worklist.push(opqzm);
}

//------------------------------DCE_loop_body----------------------------------
// Remove simplistic dead code from loop body
void IdealLoopTree::DCE_loop_body() {
  for( uint i = 0; i < _body.size(); i++ ) 
    if( _body.at(i)->outcnt() == 0 ) 
      _body.map( i--, _body.pop() );
}
  

//------------------------------adjust_loop_exit_prob--------------------------
// Look for loop-exit tests with the 50/50 (or worse) guesses from the parsing stage.
// Replace with a 1-in-10 exit guess.
void IdealLoopTree::adjust_loop_exit_prob( PhaseIdealLoop *phase ) {
  Node *test = tail();
  while( test != _head ) {
    uint top = test->Opcode();
    if( top == Op_IfTrue || top == Op_IfFalse ) {
      int test_con = ((ProjNode*)test)->_con;
      assert(top == (uint)(test_con? Op_IfTrue: Op_IfFalse), "sanity");
      IfNode *iff = test->in(0)->is_If();
      if( iff->outcnt() == 2 ) {        // Ignore dead tests
        Node *bol = iff->in(1);
        if( bol && bol->req() > 1 && bol->in(1) && 
            ((bol->in(1)->Opcode() == Op_StorePConditional ) ||
             (bol->in(1)->Opcode() == Op_StoreLConditional ) ||
             (bol->in(1)->Opcode() == Op_CompareAndSwapI ) ||
             (bol->in(1)->Opcode() == Op_CompareAndSwapL ) ||
             (bol->in(1)->Opcode() == Op_CompareAndSwapP )))
          return;               // Allocation loops RARELY take backedge
        // Find the OTHER exit path from the IF
        Node* ex = iff->proj_out(1-test_con);
        float p = iff->_prob;
        if( !phase->is_member( this, ex ) && iff->_fcnt == COUNT_UNKNOWN ) {
          if( top == Op_IfTrue ) {
            if( p < (PROB_FAIR + PROB_UNLIKELY_MAG(3))) {
              iff->_prob = PROB_STATIC_FREQUENT;
            }
          } else {
            if( p > (PROB_FAIR - PROB_UNLIKELY_MAG(3))) {
              iff->_prob = PROB_STATIC_INFREQUENT;
            }
          }
        }
      }
    }
    test = phase->idom(test);
  }
}
  

//------------------------------policy_do_remove_empty_loop--------------------
// Micro-benchmark spamming.  Policy is to always remove empty loops.
// The 'DO' part is to replace the trip counter with the value it will
// have on the last iteration.  This will break the loop.
bool IdealLoopTree::policy_do_remove_empty_loop( PhaseIdealLoop *phase ) {
  // Minimum size must be empty loop
  if( _body.size() > 7/*number of nodes in an empty loop*/ ) return false;

  CountedLoopNode *cl = _head->is_CountedLoop();
  if( !cl ) return false;     // Dead loop
  if( !phase->is_member(this,phase->get_ctrl(cl->loopexit()->in(CountedLoopEndNode::TestValue)) ) )
    return false;             // Infinite loop
#ifndef PRODUCT
  if( PrintOpto ) 
    tty->print_cr("Removing empty loop");
#endif
  assert( cl->outcnt() == 3, "Self, CLE and phi" );
  // Replace the phi at loop head with the final value of the last
  // iteration.  Then the CountedLoopEnd will collapse (backedge never
  // taken) and all loop-invariant uses of the exit values will be correct.
  Node *phi = cl->phi();
  Node *final = new (3) SubINode( cl->limit(), cl->stride() );
  phase->register_new_node(final,cl->in(LoopNode::EntryControl));
  phase->_igvn.hash_delete(phi);
  phase->_igvn.subsume_node(phi,final);
  phase->C->set_major_progress();
  return true;
}
  

//=============================================================================
//------------------------------iteration_split_impl---------------------------
void IdealLoopTree::iteration_split_impl( PhaseIdealLoop *phase, Node_List &old_new ) {
  // Check and remove empty loops (spam micro-benchmarks)
  if( policy_do_remove_empty_loop(phase) ) 
    return;                     // Here we removed an empty loop

  bool should_peel = policy_peeling(phase); // Should we peel?

  // Non-counted loops may be peeled; exactly 1 iteration is peeled.
  // This removes loop-invariant tests (usually null checks).
  CountedLoopNode *cl = _head->is_CountedLoop();
  if( !cl ) {                   // Non-counted loop
    if( should_peel )           // Should we peel?
      phase->do_peeling(this,old_new);
    return;
  }

  if( !cl->loopexit() ) return; // Ignore various kinds of broken loops

  // Do nothing special to pre- and post- loops
  if( cl->is_pre_loop() || cl->is_post_loop() ) return;

  // Before attempting fancy unrolling, RCE or alignment, see if we want
  // to completely unroll this loop.
  if( cl->is_normal_loop() ) {
    bool should_maximally_unroll =  policy_maximally_unroll(phase);
    if( should_maximally_unroll ) {
      // Here we did some unrolling and peeling.  Eventually we will 
      // completely unroll this loop and it will no longer be a loop.
      phase->do_maximally_unroll(this,old_new);
      return;
    }
  }


  // Counted loops may be peeled, may need some iterations run up
  // front for RCE, and may want to align loop refs to a cache
  // line.  Thus we clone a full loop up front whose trip count is
  // at least 1 (if peeling), but may be several more.
        
  // The main loop will start cache-line aligned with at least 1
  // iteration of the unrolled body (zero-trip test required) and
  // will have some range checks removed.
        
  // A post-loop will finish any odd iterations (leftover after
  // unrolling), plus any needed for RCE purposes.

  bool should_unroll = policy_unroll(phase);
  
  bool should_rce = policy_range_check(phase);

  bool should_align = policy_align(phase);

  // If not RCE'ing (iteration splitting) or Aligning, then we do not
  // need a pre-loop.  We may still need to peel an initial iteration but
  // we will not be needing an unknown number of pre-iterations.
  //
  // Basically, if may_rce_align reports FALSE first time through, 
  // we will not be able to later do RCE or Aligning on this loop.
  bool may_rce_align = !policy_peel_only(phase) || should_rce || should_align;

  // If we have any of these conditions (RCE, alignment, unrolling) met, then
  // we switch to the pre-/main-/post-loop model.  This model also covers
  // peeling.
  if( should_rce || should_align || should_unroll ) {
    if( cl->is_normal_loop() )  // Convert to 'pre/main/post' loops
      phase->insert_pre_post_loops(this,old_new, !may_rce_align);

    // Adjust the pre- and main-loop limits to let the pre and post loops run
    // with full checks, but the main-loop with no checks.  Remove said
    // checks from the main body.
    if( should_rce ) 
      phase->do_range_check(this,old_new);

    // Double loop body for unrolling.  Adjust the minimum-trip test (will do
    // twice as many iterations as before) and the main body limit (only do
    // an even number of trips).  If we are peeling, we might enable some RCE
    // and we'd rather unroll the post-RCE'd loop SO... do not unroll if
    // peeling.
    if( should_unroll && !should_peel ) 
      phase->do_unroll(this,old_new, true);

    // Adjust the pre-loop limits to align the main body
    // iterations.
    if( should_align )
      Unimplemented();

  } else {                      // Else we have an unchanged counted loop
    if( should_peel )           // Might want to peel but do nothing else
      phase->do_peeling(this,old_new);
  }
}


//=============================================================================
//------------------------------iteration_split--------------------------------
void IdealLoopTree::iteration_split( PhaseIdealLoop *phase, Node_List &old_new ) {
  // Recursively iteration split nested loops
  if( _child ) _child->iteration_split( phase, old_new );

  // Clean out prior deadwood
  DCE_loop_body();


  // Look for loop-exit tests with my 50/50 guesses from the Parsing stage.
  // Replace with a 1-in-10 exit guess.
  if( _parent /*not the root loop*/ && 
      !_irreducible && 
      // Also ignore the occasional dead backedge
      !tail()->is_top() ) {
    adjust_loop_exit_prob(phase);
  }


  // Gate unrolling, RCE and peeling efforts.
  if( !_child &&                // If not an inner loop, do not split
      !_has_call &&             // Do not bother if call; fix inlining instead
      !_irreducible &&
      !tail()->is_top() ) {     // Also ignore the occasional dead backedge
    iteration_split_impl( phase, old_new );
  }

  // Minor offset re-organization to remove loop-fallout uses of 
  // trip counter.
  CountedLoopNode *cl = _head->is_CountedLoop();
  if( cl ) phase->reorg_offsets( this );
  /*
  if( OptoReorgOffsets && !_child ) {
    for( uint i = 0; i < _body.size(); i++ ) {
      // Check for a Q+con value alive at the same time as Q
      // Reorganize to lower register pressure.
      Node *add = _body.at(i);
      if( add->Opcode() == Op_AddI && add->in(2)->is_Con() )
        reorg_offsets( add, phase );
    }
  }
  */

  if( _next ) _next->iteration_split( phase, old_new );
}
