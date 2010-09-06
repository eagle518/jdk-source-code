#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)loopnode.hpp	1.120 04/02/03 15:22:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class CmpNode;
class CountedLoopEndNode;
class CountedLoopNode;
class IdealLoopTree;
class LoopNode;
class Node;
class PhaseIdealLoop;
class VectorSet;
struct small_cache;

//
//                  I D E A L I Z E D   L O O P S
//
// Idealized loops are the set of loops I perform more interesting
// transformations on, beyond simple hoisting.  

//------------------------------LoopNode---------------------------------------
// Simple loop header.  Fall in path on left, loop-back path on right.
class LoopNode : public RegionNode {
  // Size is bigger to hold the flags.  However, the flags do not change 
  // the semantics so it does not appear in the hash & cmp functions.
  virtual uint size_of() const { return sizeof(*this); }
protected:
  short _flags;
  // Names for flag bitfields
  enum { pre_post_main=0, inner_loop=3  };

public:
  // Names for edge indices
  enum { Self=0, EntryControl, LoopBackControl };

  int is_inner_loop() const { return _flags & (1<<inner_loop); }
  void set_inner_loop() { _flags |= (1<<inner_loop); }

  LoopNode( Node *entry, Node *backedge ); // Original loop header node
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual int Opcode() const;
  virtual LoopNode *is_Loop() { return this; }
  bool can_be_counted_loop(PhaseTransform* phase) const {
    return req() == 3 && in(0) != NULL &&
      in(1) != NULL && phase->type(in(1)) != Type::TOP &&
      in(2) != NULL && phase->type(in(2)) != Type::TOP;
  }
#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};

//------------------------------Counted Loops----------------------------------
// Counted loops are all trip-counted loops, with exactly 1 trip-counter exit
// path (and maybe some other exit paths).  The trip-counter exit is always
// last in the loop.  The trip-counter does not have to stride by a constant,
// but it does have to stride by a loop-invariant amount; the exit value is
// also loop invariant.

// CountedLoopNodes and CountedLoopEndNodes come in matched pairs.  The
// CountedLoopNode has the incoming loop control and the loop-back-control
// which is always the IfTrue before the matching CountedLoopEndNode.  The
// CountedLoopEndNode has an incoming control (possibly not the
// CountedLoopNode if there is control flow in the loop), the post-increment
// trip-counter value, and the limit.  The trip-counter value is always of
// the form (Op old-trip-counter stride).  The old-trip-counter is produced
// by a Phi connected to the CountedLoopNode.  The stride is loop invariant.
// The Op is any commutable opcode, including Add, Mul, Xor.  The
// CountedLoopEndNode also takes in the loop-invariant limit value.

// From a CountedLoopNode I can reach the matching CountedLoopEndNode via the
// loop-back control.  From CountedLoopEndNodes I can reach CountedLoopNodes
// via the old-trip-counter from the Op node.

//------------------------------CountedLoopNode--------------------------------
// CountedLoopNodes head simple counted loops.  CountedLoopNodes have as
// inputs the incoming loop-start control and the loop-back control, so they
// act like RegionNodes.  They also take in the initial trip counter, the
// loop-invariant stride and the loop-invariant limit value.  CountedLoopNodes 
// produce a loop-body control and the trip counter value.  Since 
// CountedLoopNodes behave like RegionNodes I still have a standard CFG model.

class CountedLoopNode : public LoopNode {
  // Size is bigger to hold _main_idx.  However, _main_idx does not change 
  // the semantics so it does not appear in the hash & cmp functions.
  virtual uint size_of() const { return sizeof(*this); }

  // For Pre- and Post-loops during debugging ONLY, this holds the index of
  // the Main CountedLoop.  Used to assert that we understand the graph shape.
  node_idx_t _main_idx;
  
  // Known trip count calculated by policy_maximally_unroll
  int   _trip_count; 

public:
  CountedLoopNode( Node *entry, Node *backedge ); // Original loop header node
  virtual int Opcode() const;
  virtual Node *Ideal(PhaseGVN *phase, bool can_reshape);
  virtual CountedLoopNode *is_CountedLoop() { return this; }

  Node *init_control() const { return in(EntryControl); }
  Node *back_control() const { return in(LoopBackControl); }
  CountedLoopEndNode *loopexit() const;
  Node *init_trip() const;
  Node *stride() const;
  int   stride_con() const;
  bool  stride_is_con() const;
  Node *limit() const;
  Node *incr() const;
  Node *phi() const;


  // A 'main' loop has a pre-loop and a post-loop.  The 'main' loop
  // can run short a few iterations and may start a few iterations in.
  // It will be RCE'd and unrolled and aligned.

  // A following 'post' loop will run any remaining iterations.  Used
  // during Range Check Elimination, the 'post' loop will do any final
  // iterations with full checks.  Also used by Loop Unrolling, where
  // the 'post' loop will do any epilog iterations needed.  Basically,
  // a 'post' loop can not profitably be further unrolled or RCE'd.

  // A preceding 'pre' loop will run at least 1 iteration (to do peeling),
  // it may do under-flow checks for RCE and may do alignment iterations
  // so the following main loop 'knows' that it is striding down cache
  // lines.

  // A 'main' loop that is ONLY unrolled or peeled, never RCE'd or
  // Aligned, may be missing it's pre-loop.
  enum { Normal, Pre, Main, Post, PrePostFlagsMask=3, Main_Has_No_Pre_Loop=2 };
  int is_normal_loop() const { return (_flags&PrePostFlagsMask) == Normal; }
  int is_pre_loop   () const { return (_flags&PrePostFlagsMask) == Pre;    }
  int is_main_loop  () const { return (_flags&PrePostFlagsMask) == Main;   }
  int is_post_loop  () const { return (_flags&PrePostFlagsMask) == Post;   }
  int is_main_no_pre_loop() const { return _flags & (1<<Main_Has_No_Pre_Loop); }
  void set_main_no_pre_loop() { _flags |= (1<<Main_Has_No_Pre_Loop); }


  void set_pre_loop  (CountedLoopNode *main) { assert(is_normal_loop(),""); _flags |= Pre ; _main_idx = main->_idx; }
  void set_main_loop (                     ) { assert(is_normal_loop(),""); _flags |= Main;                         }
  void set_post_loop (CountedLoopNode *main) { assert(is_normal_loop(),""); _flags |= Post; _main_idx = main->_idx; }
  void set_normal_loop(                    ) { _flags &= ~PrePostFlagsMask; }

  void set_trip_count(int tc) { _trip_count = tc; }
  int trip_count()            { return _trip_count; }

#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};

//------------------------------CountedLoopEndNode-----------------------------
// CountedLoopEndNodes end simple trip counted loops.  They act much like
// IfNodes.
class CountedLoopEndNode : public IfNode {
public:
  enum { TestControl, TestValue };

  CountedLoopEndNode( Node *control, Node *test, float prob, float cnt );
  virtual int Opcode() const;

  Node *cmp_node() const            { return (in(TestValue)->req() >=2) ? in(TestValue)->in(1) : NULL; }
  Node *incr() const                { Node *tmp = cmp_node(); return (tmp && tmp->req()==3) ? tmp->in(1) : NULL; }
  Node *limit() const               { Node *tmp = cmp_node(); return (tmp && tmp->req()==3) ? tmp->in(2) : NULL; }
  Node *stride() const              { Node *tmp = incr    (); return (tmp && tmp->req()==3) ? tmp->in(2) : NULL; }
  Node *phi() const                 { Node *tmp = incr    (); return (tmp && tmp->req()==3) ? tmp->in(1) : NULL; }
  Node *init_trip() const           { Node *tmp = phi     (); return (tmp && tmp->req()==3) ? tmp->in(1) : NULL; }
  int stride_con() const;
  bool stride_is_con() const        { Node *tmp = stride  (); return (tmp != NULL && tmp->is_Con()); }
  BoolTest::mask test_trip() const  { return in(TestValue)->is_Bool()->_test._test; }
  CountedLoopNode *loopnode() const { 
    Node *ln = phi()->in(0);
    assert( ln->Opcode() == Op_CountedLoop, "malformed loop" );
    return (CountedLoopNode*)ln; }

#ifndef PRODUCT
  virtual void dump_spec() const;
#endif
};


inline CountedLoopEndNode *CountedLoopNode::loopexit() const { 
  Node *bc = back_control();
  if( bc == NULL ) return NULL;
  Node *le = bc->in(0);
  if( le->Opcode() != Op_CountedLoopEnd )
    return NULL;
  return (CountedLoopEndNode*)le; 
}
inline Node *CountedLoopNode::init_trip() const { return loopexit() ? loopexit()->init_trip() : NULL; }
inline Node *CountedLoopNode::stride() const { return loopexit() ? loopexit()->stride() : NULL; }
inline int CountedLoopNode::stride_con() const { return loopexit() ? loopexit()->stride_con() : NULL; }
inline bool CountedLoopNode::stride_is_con() const { return loopexit() && loopexit()->stride_is_con(); }
inline Node *CountedLoopNode::limit() const { return loopexit() ? loopexit()->limit() : NULL; }
inline Node *CountedLoopNode::incr() const { return loopexit() ? loopexit()->incr() : NULL; }
inline Node *CountedLoopNode::phi() const { return loopexit() ? loopexit()->phi() : NULL; }


// -----------------------------IdealLoopTree----------------------------------
class IdealLoopTree : public ResourceObj {
public:
  IdealLoopTree *_parent;       // Parent in loop tree
  IdealLoopTree *_next;         // Next sibling in loop tree
  IdealLoopTree *_child;        // First child in loop tree

  // The head-tail backedge defines the loop.
  // If tail is NULL then this loop has multiple backedges as part of the
  // same loop.  During cleanup I'll peel off the multiple backedges; merge
  // them at the loop bottom and flow 1 real backedge into the loop.
  Node *_head;                  // Head of loop
  Node *_tail;                  // Tail of loop
  inline Node *tail();          // Handle lazy update of _tail field
  PhaseIdealLoop* _phase;

  Node_List _body;              // Loop body for inner loops
  
  uint8 _nest;                  // Nesting depth
  uint8 _irreducible:1,         // True if irreducible
        _has_call:1,            // True if has call safepoint
        _has_sfpt:1;            // True if has non-call safepoint

  IdealLoopTree( PhaseIdealLoop* phase, Node *head, Node *tail )
    : _parent(0), _next(0), _child(0),
      _head(head), _tail(tail),
      _phase(phase),
      _nest(0), _irreducible(0), _has_call(0), _has_sfpt(0)
  { }

  // Is 'l' a member of 'this'?
  int is_member( const IdealLoopTree *l ) const; // Test for nested membership

  // Set loop nesting depth.  Accumulate has_call bits.
  int set_nest( uint depth );

  // Split out multiple fall-in edges from the loop header.  Move them to a 
  // private RegionNode before the loop.  This becomes the loop landing pad.
  void split_fall_in( PhaseIdealLoop *phase, int fall_in_cnt );

  // Split out the outermost loop from this shared header.
  void split_outer_loop( PhaseIdealLoop *phase );
  
  // Merge all the backedges from the shared header into a private Region.
  // Feed that region as the one backedge to this loop.
  void merge_many_backedges( PhaseIdealLoop *phase );

  // Split shared headers and insert loop landing pads.
  // Insert a LoopNode to replace the RegionNode.
  // Returns TRUE if loop tree is structurally changed.
  bool beautify_loops( PhaseIdealLoop *phase );

  // Perform iteration-splitting on inner loops.  Split iterations to avoid
  // range checks or one-shot null checks.
  void iteration_split( PhaseIdealLoop *phase, Node_List &old_new );

  // Driver for various flavors of iteration splitting
  void iteration_split_impl( PhaseIdealLoop *phase, Node_List &old_new );

  // Given dominators, try to find inner loops with calls that must
  // always be executed (call dominates loop tail).  These loops do
  // not need a seperate safepoint.
  void check_inner_safepts( PhaseIdealLoop *phase );

  // Convert to counted loops where possible
  void counted_loop( PhaseIdealLoop *phase );

  // Check for Node being a loop-breaking test
  Node *is_loop_exit( Node *iff, PhaseIdealLoop *phase ) const;

  // Remove simplistic dead code from loop body
  void DCE_loop_body();

  // Look for loop-exit tests with my 50/50 guesses from the Parsing stage.
  // Replace with a 1-in-10 exit guess.
  void adjust_loop_exit_prob( PhaseIdealLoop *phase );

  // Return TRUE or FALSE if the loop should never be RCE'd or aligned.  
  // Useful for unrolling loops with NO array accesses.
  bool policy_peel_only( PhaseIdealLoop *phase ) const;

  // Micro-benchmark spamming.  Remove empty loops.
  bool policy_do_remove_empty_loop( PhaseIdealLoop *phase );

  // Return TRUE or FALSE if the loop should be peeled or not.  Peel if we can
  // make some loop-invariant test (usually a null-check) happen before the 
  // loop.
  bool policy_peeling( PhaseIdealLoop *phase ) const;

  // Return TRUE or FALSE if the loop should be maximally unrolled. Stash any
  // known trip count in the counted loop node.
  bool policy_maximally_unroll( PhaseIdealLoop *phase ) const;

  // Return TRUE or FALSE if the loop should be unrolled or not.  Unroll if 
  // the loop is a CountedLoop and the body is small enough.
  bool policy_unroll( PhaseIdealLoop *phase ) const;

  // Return TRUE or FALSE if the loop should be range-check-eliminated.
  // Gather a list of IF tests that are dominated by iteration splitting;
  // also gather the end of the first split and the start of the 2nd split.
  bool policy_range_check( PhaseIdealLoop *phase ) const;

  // Return TRUE or FALSE if the loop should be cache-line aligned.
  // Gather the expression that does the alignment.  Note that only
  // one array base can be aligned in a loop (unless the VM guarentees
  // mutual alignment).  Note that if we vectorize short memory ops
  // into longer memory ops, we may want to increase alignment.
  bool policy_align( PhaseIdealLoop *phase ) const;

#ifndef PRODUCT
  void dump_head( ) const;      // Dump loop head only
  void dump() const;            // Dump this loop recursively
  void verify_tree(IdealLoopTree *loop, const IdealLoopTree *parent) const;
#endif

};

// -----------------------------PhaseIdealLoop---------------------------------
// Computes the mapping from Nodes to IdealLoopTrees.  Organizes IdealLoopTrees into a
// loop tree.  Drives the loop-based transformations on the ideal graph.
class PhaseIdealLoop : public PhaseTransform {
  friend class IdealLoopTree;
  // Pre-computed def-use info
  PhaseIterGVN &_igvn;

  // Head of loop tree
  IdealLoopTree *_ltree_root;

  // Array of pre-order numbers, plus post-visited bit.
  // ZERO for not pre-visited.  EVEN for pre-visited but not post-visited.
  // ODD for post-visited.  Other bits are the pre-order number.
  uint *_preorders;
  uint _max_preorder;

  // Allocate _preorders[] array
  void allocate_preorders() {
    _max_preorder = C->unique()+8;
    _preorders = NEW_RESOURCE_ARRAY(uint, _max_preorder);
    memset(_preorders, 0, sizeof(uint) * _max_preorder);
  }

  // Allocate _preorders[] array
  void reallocate_preorders() {
    if ( _max_preorder < C->unique() ) {
      _preorders = REALLOC_RESOURCE_ARRAY(uint, _preorders, _max_preorder, C->unique());
      _max_preorder = C->unique();
    }
    memset(_preorders, 0, sizeof(uint) * _max_preorder);
  }

  // Check to grow _preorders[] array for the case when build_loop_tree_impl() 
  // adds new nodes.
  void check_grow_preorders( ) {
    if ( _max_preorder < C->unique() ) {
      uint newsize = _max_preorder<<1;  // double size of array
      _preorders = REALLOC_RESOURCE_ARRAY(uint, _preorders, _max_preorder, newsize);
      memset(&_preorders[_max_preorder],0,sizeof(uint)*(newsize-_max_preorder));
      _max_preorder = newsize;
    }
  }
  // Check for pre-visited.  Zero for NOT visited; non-zero for visited.
  int is_visited( Node *n ) const { return _preorders[n->_idx]; }
  // Pre-order numbers are written to the Nodes array as low-bit-set values.
  void set_preorder_visited( Node *n, int pre_order ) { 
    assert( !is_visited( n ), "already set" );
    _preorders[n->_idx] = (pre_order<<1);
  };
  // Return pre-order number.
  int get_preorder( Node *n ) const { assert( is_visited(n), "" ); return _preorders[n->_idx]>>1; }

  // Check for being post-visited. 
  // Should be previsited already (checked with assert(is_visited(n))).
  int is_postvisited( Node *n ) const { assert( is_visited(n), "" ); return _preorders[n->_idx]&1; }

  // Mark as post visited
  void set_postvisited( Node *n ) { assert( !is_postvisited( n ), "" ); _preorders[n->_idx] |= 1; }

  // Set/get control node out.  Set lower bit to distinguish from IdealLoopTree
  bool has_ctrl( Node *n ) const { return ((intptr_t)_nodes[n->_idx]) & 1; }

  // clear out dead code after build_loop_late
  Node_List _deadlist;

  // Support for faster execution of get_late_ctrl()/dom_lca()
  // when a node has many uses and dominator depth is deep.
  Node_Array _dom_lca_tags;
  void   init_dom_lca_tags();
  // Inline wrapper for frequent cases:
  // 1) only one use
  // 2) a use is the same as the current LCA passed as 'n1'
  Node *dom_lca_for_get_late_ctrl( Node *lca, Node *n, Node *tag ) {
    assert( n->is_CFG(), "" );
    // Handle NULL original LCA
    if( lca == NULL  ||  lca == n ) {
      return n;
    }
    assert( lca->is_CFG(), "" );

    // find LCA of all uses
    return dom_lca_for_get_late_ctrl_internal( lca, n, tag );
  }
  Node *dom_lca_for_get_late_ctrl_internal( Node *lca, Node *n, Node *tag );

public:
  bool has_node( Node* n ) const { return _nodes[n->_idx] != NULL; }
  // check if transform created new nodes that need _ctrl recorded
  Node *get_late_ctrl( Node *n, Node *early );
  Node *get_early_ctrl( Node *n );
  void set_early_ctrl( Node *n );
  void set_subtree_ctrl( Node *root );
  void set_ctrl( Node *n, Node *ctrl ) {
    assert( !has_node(n) || has_ctrl(n), "" );
    assert( ctrl->in(0), "cannot set dead control node" );
    _nodes.map( n->_idx, (Node*)((intptr_t)ctrl + 1) );
  }
  // Control nodes can be replaced or subsumed.  During this pass they
  // get their replacement Node in slot 1.  Instead of updating the block
  // location of all Nodes in the subsumed block, we lazily do it.  As we
  // pull such a subsumed block out of the array, we write back the final
  // correct block.
  Node *get_ctrl( Node *i ) {
    assert(has_node(i), "");
    Node *n = get_ctrl_no_update(i);
    _nodes.map( i->_idx, (Node*)((intptr_t)n + 1) );
    assert(has_node(i) && has_ctrl(i), "");
    return n;
  }
private:
  Node *get_ctrl_no_update( Node *i ) const {
    assert( has_ctrl(i), "" );
    Node *n = (Node*)(((intptr_t)_nodes[i->_idx]) & ~1);
    while( !n->in(0) )          // Skip dead CFG nodes
      //n = n->in(1);
      n = (Node*)(((intptr_t)_nodes[n->_idx]) & ~1);
    return n;
  }

  // Check for loop being set
  bool has_loop( Node *n ) const { 
    assert(!has_node(n) || !has_ctrl(n), "");
    return has_node(n);
  }
  // Set loop
  void set_loop( Node *n, IdealLoopTree *loop ) {
    _nodes.map(n->_idx, (Node*)loop);
  }
  // Lazy-dazy update of 'get_ctrl' and 'idom_at' mechanisms.  Replace
  // the 'old_node' with 'new_node'.  Kill old-node.  Add a reference
  // from old_node to new_node to support the lazy update.  Reference
  // replaces loop reference, since that is not neede for dead node.
public:
  void lazy_update( Node *old_node, Node *new_node ) {
    assert( old_node != new_node, "no cycles please" );
    //old_node->set_req( 1, new_node /*NO DU INFO*/ );
    // Nodes always have DU info now, so re-use the side array slot
    // for this node to provide the forwarding pointer.
    _nodes.map( old_node->_idx, (Node*)((intptr_t)new_node + 1) );
  }
  void lazy_replace( Node *old_node, Node *new_node ) {
    _igvn.hash_delete(old_node);
    _igvn.subsume_node( old_node, new_node );
    lazy_update( old_node, new_node );
  }
  void lazy_replace_proj( Node *old_node, Node *new_node ) {
    assert( old_node->req() == 1, "use this for Projs" );
    _igvn.hash_delete(old_node); // Must hash-delete before hacking edges
    old_node->add_req( NULL );
    lazy_replace( old_node, new_node );
  }

private:

  // Place 'n' in some loop nest, where 'n' is a CFG node
  void build_loop_tree();
  int build_loop_tree_impl( Node *n, int pre_order );
  // Insert loop into the existing loop tree.  'innermost' is a leaf of the
  // loop tree, not the root.
  IdealLoopTree *sort( IdealLoopTree *loop, IdealLoopTree *innermost );

  // Place Data nodes in some loop nest
  void build_loop_early( VectorSet &visited, Node_List &worklist, Node_Stack &nstack, const PhaseIdealLoop *verify_me );
  void build_loop_late ( VectorSet &visited, Node_List &worklist, Node_Stack &nstack, const PhaseIdealLoop *verify_me );
  void build_loop_late_post ( Node* n, const PhaseIdealLoop *verify_me );

  // Array of immediate dominance info for each CFG node indexed by node idx
private:
  uint _idom_size;
  Node **_idom;                 // Array of immediate dominators
  uint *_dom_depth;           // Used for fast LCA test    
  Node* idom_no_update(Node* d) const {
    assert(d->_idx < _idom_size, "oob"); 
    Node* n = _idom[d->_idx];
    assert(n != NULL,"Bad immediate dominator info.");
    while (n->in(0) == NULL) {  // Skip dead CFG nodes
      //n = n->in(1);
      n = (Node*)(((intptr_t)_nodes[n->_idx]) & ~1);
      assert(n != NULL,"Bad immediate dominator info.");
    }
    return n;
  }
  Node *idom(Node* d) const {
    uint didx = d->_idx;
    Node *n = idom_no_update(d);
    _idom[didx] = n;            // Lazily remove dead CFG nodes from table.
    return n;
  }
  uint dom_depth(Node* d) const {                        
    assert(d->_idx < _idom_size, "");
    return _dom_depth[d->_idx];
  }
  void set_idom(Node* d, Node* n, uint dom_depth);
  // Locally compute IDOM using dom_lca call
  Node *compute_idom( Node *region ) const;

public:
  // Dominators for the sea of nodes
  void Dominators();
  Node *dom_lca( Node *n1, Node *n2 ) const;

  // Compute the Ideal Node to Loop mapping
  PhaseIdealLoop( PhaseIterGVN &igvn, const PhaseIdealLoop *verify_me, bool do_split_ifs );

  // True if the method has at least 1 irreducible loop
  bool _has_irreducible_loops;

  // Per-Node transform
  virtual Node *transform( Node *a_node ) { return 0; }

  Node *is_counted_loop( Node *x, IdealLoopTree *loop );

  // Return a post-walked LoopNode
  IdealLoopTree *get_loop( Node *n ) const {
    // Dead nodes have no loop, so return the top level loop instead
    if (!has_node(n))  return _ltree_root;
    assert(!has_ctrl(n), "");
    return (IdealLoopTree*)_nodes[n->_idx];
  }

  // Is 'n' a (nested) member of 'loop'?
  int is_member( const IdealLoopTree *loop, Node *n ) const { 
    return loop->is_member(get_loop(n)); }

  // This is the basic building block of the loop optimizations.  It clones an
  // entire loop body.  It makes an old_new loop body mapping; with this
  // mapping you can find the new-loop equivalent to an old-loop node.  All
  // new-loop nodes are exactly equal to their old-loop counterparts, all
  // edges are the same.  All exits from the old-loop now have a RegionNode
  // that merges the equivalent new-loop path.  This is true even for the
  // normal "loop-exit" condition.  All uses of loop-invariant old-loop values
  // now come from (one or more) Phis that merge their new-loop equivalents.
  void clone_loop( IdealLoopTree *loop, Node_List &old_new, int dom_depth );

  // If we got the effect of peeling, either by actually peeling or by
  // making a pre-loop which must execute at least once, we can remove
  // all loop-invariant dominated tests in the main body.
  void peeled_dom_test_elim( IdealLoopTree *loop, Node_List &old_new );

  // Generate code to do a loop peel for the given loop (and body).
  // old_new is a temp array.
  void do_peeling( IdealLoopTree *loop, Node_List &old_new );

  // Add pre and post loops around the given loop.  These loops are used
  // during RCE, unrolling and aligning loops.
  void insert_pre_post_loops( IdealLoopTree *loop, Node_List &old_new, bool peel_only );
  // If Node n lives in the back_ctrl block, we clone a private version of n
  // in preheader_ctrl block and return that, otherwise return n.
  Node *clone_up_backedge_goo( Node *back_ctrl, Node *preheader_ctrl, Node *n );

  // Take steps to maximally unroll the loop.  Peel any odd iterations, then
  // unroll to do double iterations.  The next round of major loop transforms
  // will repeat till the doubled loop body does all remaining iterations in 1
  // pass.
  void do_maximally_unroll( IdealLoopTree *loop, Node_List &old_new );

  // Unroll the loop body one step - make each trip do 2 iterations.
  void do_unroll( IdealLoopTree *loop, Node_List &old_new, bool adjust_min_trip );

  // Eliminate range-checks and other trip-counter vs loop-invariant tests.
  void do_range_check( IdealLoopTree *loop, Node_List &old_new );

  // Range Check Elimination uses this function!
  // Constrain the main loop iterations so the affine function:
  //    scale_con * I + offset  <  limit
  // always holds true.  That is, either increase the number of iterations in
  // the pre-loop or the post-loop until the condition holds true in the main
  // loop.  Scale_con, offset and limit are all loop invariant.
  void add_constraint( int stride_con, int scale_con, Node *offset, Node *limit, Node *pre_ctrl, Node **pre_limit, Node **main_limit );

  // Passed in a Phi merging (recursively) some nearly equivalent Bool/Cmps.
  // "Nearly" because all Nodes have been cloned from the original in the loop,
  // but the fall-in edges to the Cmp are different.  Clone bool/Cmp pairs 
  // through the Phi recursively, and return a Bool.
  BoolNode *clone_iff( PhiNode *phi, IdealLoopTree *loop );
  CmpNode *clone_bool( PhiNode *phi, IdealLoopTree *loop );


  // Rework addressing expressions to get the most loop-invariant stuff 
  // moved out.  We'd like to do all associative operators, but it's especially
  // important (common) to do address expressions.
  Node *remix_address_expressions( Node *n );

  // Attempt to use a conditional move instead of a phi/branch
  Node *conditional_move( Node *n );

  // Reorganize offset computations to lower register pressure.
  // Mostly prevent loop-fallout uses of the pre-incremented trip counter
  // (which are then alive with the post-incremented trip counter
  // forcing an extra register move)
  void reorg_offsets( IdealLoopTree *loop );

  // Check for aggressive application of 'split-if' optimization,
  // using basic block level info.
  void  split_if_with_blocks     ( VectorSet &visited, Node_Stack &nstack );
  Node *split_if_with_blocks_pre ( Node *n );
  void  split_if_with_blocks_post( Node *n );
  Node *has_local_phi_input( Node *n );
  // Mark an IfNode as being dominated by a prior test,
  // without actually altering the CFG (and hence IDOM info).
  void dominated_by( Node *prevdom, Node *iff );

  // Split Node 'n' through merge point
  Node *split_thru_region( Node *n, Node *region );
  // Split Node 'n' through merge point if there is enough win.
  Node *split_thru_phi( Node *n, Node *region, int policy );
  // Found an If getting its condition-code input from a Phi in the 
  // same block.  Split thru the Region.
  void do_split_if( Node *iff );
private:
  // Helper functions
  void register_new_node( Node *n, Node *blk );
  Node *spinup( Node *iff, Node *new_false, Node *new_true, Node *region, Node *phi, small_cache *cache );
  Node *find_use_block( Node *use, Node *def, Node *old_false, Node *new_false, Node *old_true, Node *new_true );
  void handle_use( Node *use, Node *def, small_cache *cache, Node *region_dom, Node *new_false, Node *new_true, Node *old_false, Node *old_true );
  bool split_up( Node *n, Node *blk1, Node *blk2 );
  void sink_use( Node *use, Node *post_loop );
  Node *place_near_use( Node *useblock ) const;
public:

#ifndef PRODUCT
  void dump( ) const;
  void dump( IdealLoopTree *loop, uint rpo_idx, Node_List &rpo_list ) const;
  void rpo( Node *n, VectorSet &visited, Node_List &rpo_list ) const;
  void verify() const;          // Major slow  :-)
  void verify_compare( Node *n, const PhaseIdealLoop *loop_verify, VectorSet &visited ) const;
  IdealLoopTree *get_loop_idx(Node* n) const {
    // Dead nodes have no loop, so return the top level loop instead
    return _nodes[n->_idx] ? (IdealLoopTree*)_nodes[n->_idx] : _ltree_root;
  }
  // Print some stats
  static void print_statistics();
  static int _loop_invokes;     // Count of PhaseIdealLoop invokes
  static int _loop_work;        // Sum of PhaseIdealLoop x _unique
#endif
};

inline Node* IdealLoopTree::tail() {
// Handle lazy update of _tail field
  Node *n = _tail;
  //while( !n->in(0) )  // Skip dead CFG nodes
    //n = n->in(1);
  if (n->in(0) == NULL)
    n = _phase->get_ctrl(n);
  _tail = n;
  return n;
}

