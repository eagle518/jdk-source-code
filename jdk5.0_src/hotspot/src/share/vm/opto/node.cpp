#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)node.cpp	1.202 03/12/23 16:42:45 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_node.cpp.incl"

class RegMask;
// #include "phase.hpp"
class PhaseTransform;
class PhaseGVN;

// Arena we are currently building Nodes in
const uint Node::NotAMachineReg = 0xffff0000;

#ifndef PRODUCT
extern int nodes_created;
#endif

#ifdef ASSERT

//-------------------------- construct_node------------------------------------
// Set a breakpoint here to identify where a particular node index is built.
void Node::verify_construction( ) {
  int old_debug_idx = Compile::debug_idx();
  int new_debug_idx = old_debug_idx+1;
  if (new_debug_idx > 0) {
    // Arrange that the lowest five decimal digits of _debug_idx
    // will repeat thos of _idx.  In case this is somehow pathological,
    // we continue to assign negative numbers (!) consecutively.
    const int mod = 100000;
    int bump = (int)(_idx - new_debug_idx) % mod;
    if (bump < 0)  bump += mod;
    assert(bump >= 0 && bump < mod, "");
    new_debug_idx += bump;
  }
  Compile::set_debug_idx(new_debug_idx);
  set_debug_idx( new_debug_idx );
  assert(Compile::current()->unique() < (uint)MaxNodeLimit, "Node limit exceeded");
  if (BreakAtNode != 0 && (_debug_idx == BreakAtNode || (int)_idx == BreakAtNode)) {
    tty->print_cr("BreakAtNode: _idx=%d _debug_idx=%d", _idx, _debug_idx);
    BREAKPOINT;
  }
#if OPTO_DU_ITERATOR_ASSERT
  _last_del = NULL;
  _del_tick = 0;
#endif
  _hash_lock = 0;
}


// #ifdef ASSERT ...

#if OPTO_DU_ITERATOR_ASSERT
void DUIterator_Common::sample(const Node* node) {
  _vdui     = VerifyDUIterators;
  _node     = node;
  _outcnt   = node->_outcnt;
  _del_tick = node->_del_tick;
  _last     = NULL;
}

void DUIterator_Common::verify(const Node* node, bool at_end_ok) {
  assert(_node     == node, "consistent iterator source");
  assert(_del_tick == node->_del_tick, "no unexpected deletions allowed");
}

void DUIterator_Common::verify_resync() {
  // Ensure that the loop body has just deleted the last guy produced.
  const Node* node = _node;
  // Ensure that at least one copy of the last-seen edge was deleted.
  // Note:  It is OK to delete multiple copies of the last-seen edge.
  // Unfortunately, we have no way to verify that all the deletions delete
  // that same edge.  On this point we must use the Honor System.
  assert(node->_del_tick >= _del_tick+1, "must have deleted an edge");
  assert(node->_last_del == _last, "must have deleted the edge just produced");
  // We liked this deletion, so accept the resulting outcnt and tick.
  _outcnt   = node->_outcnt;
  _del_tick = node->_del_tick;
}

void DUIterator_Common::reset(const DUIterator_Common& that) {
  if (this == &that)  return;  // ignore assignment to self
  if (!_vdui) {
    // We need to initialize everything, overwriting garbage values.
    _last = that._last;
    _vdui = that._vdui;
  }
  // Note:  It is legal (though odd) for an iterator over some node x
  // to be reassigned to iterate over another node y.  Some doubly-nested
  // progress loops depend on being able to do this.
  const Node* node = that._node;
  // Re-initialize everything, except _last.
  _node     = node;
  _outcnt   = node->_outcnt;
  _del_tick = node->_del_tick;
}

void DUIterator::sample(const Node* node) {
  DUIterator_Common::sample(node);      // Initialize the assertion data.
  _refresh_tick = 0;                    // No refreshes have happened, as yet.
}

void DUIterator::verify(const Node* node, bool at_end_ok) {
  DUIterator_Common::verify(node, at_end_ok);
  assert(_idx      <  node->_outcnt + (uint)at_end_ok, "idx in range");
}

void DUIterator::verify_increment() {
  if (_refresh_tick & 1) {
    // We have refreshed the index during this loop.
    // Fix up _idx to meet asserts.
    if (_idx > _outcnt)  _idx = _outcnt;
  }
  verify(_node, true);
}

void DUIterator::verify_resync() {
  // Note:  We do not assert on _outcnt, because insertions are OK here.
  DUIterator_Common::verify_resync();
  // Make sure we are still in sync, possibly with no more out-edges:
  verify(_node, true);
}

void DUIterator::reset(const DUIterator& that) {
  if (this == &that)  return;  // self assignment is always a no-op
  assert(that._refresh_tick == 0, "assign only the result of Node::outs()");
  assert(that._idx          == 0, "assign only the result of Node::outs()");
  assert(_idx               == that._idx, "already assigned _idx");
  if (!_vdui) {
    // We need to initialize everything, overwriting garbage values.
    sample(that._node);
  } else {
    DUIterator_Common::reset(that);
    if (_refresh_tick & 1) {
      _refresh_tick++;                  // Clear the "was refreshed" flag.
    }
    assert(_refresh_tick < 2*100000, "DU iteration must converge quickly");
  }
}

void DUIterator::refresh() {
  DUIterator_Common::sample(_node);     // Re-fetch assertion data.
  _refresh_tick |= 1;                   // Set the "was refreshed" flag.
}

void DUIterator::verify_finish() {
  // If the loop has killed the node, do not require it to re-run.
  if (_node->_outcnt == 0)  _refresh_tick &= ~1;
  // If this assert triggers, it means that a loop used refresh_out_pos
  // to re-synch an iteration index, but the loop did not correctly
  // re-run itself, using a "while (progress)" construct.
  // This iterator enforces the rule that you must keep trying the loop
  // until it "runs clean" without any need for refreshing.
  assert(!(_refresh_tick & 1), "the loop must run once with no refreshing");
}


void DUIterator_Fast::verify(const Node* node, bool at_end_ok) {
  DUIterator_Common::verify(node, at_end_ok);
  Node** out    = node->_out;
  uint   cnt    = node->_outcnt;
  assert(cnt == _outcnt, "no insertions allowed");
  assert(_outp >= out && _outp <= out + cnt - !at_end_ok, "outp in range");
  // This last check is carefully designed to work for NO_OUT_ARRAY.
}

void DUIterator_Fast::verify_limit() {
  const Node* node = _node;
  verify(node, true);
  assert(_outp == node->_out + node->_outcnt, "limit still correct");
}

void DUIterator_Fast::verify_resync() {
  const Node* node = _node;
  if (_outp == node->_out + _outcnt) {
    // Note that the limit imax, not the pointer i, gets updated with the
    // exact count of deletions.  (For the pointer it's always "--i".)
    assert(node->_outcnt+node->_del_tick == _outcnt+_del_tick, "no insertions allowed with deletion(s)");
    // This is a limit pointer, with a name like "imax".
    // Fudge the _last field so that the common assert will be happy.
    _last = (Node*) node->_last_del;
    DUIterator_Common::verify_resync();
  } else {
    assert(node->_outcnt < _outcnt, "no insertions allowed with deletion(s)");
    // A normal internal pointer.
    DUIterator_Common::verify_resync();
    // Make sure we are still in sync, possibly with no more out-edges:
    verify(node, true);
  }
}

void DUIterator_Fast::verify_relimit(uint n) {
  const Node* node = _node;
  assert((int)n > 0, "use imax -= n only with a positive count");
  // This must be a limit pointer, with a name like "imax".
  assert(_outp == node->_out + node->_outcnt, "apply -= only to a limit (imax)");
  // The reported number of deletions must match what the node saw.
  assert(node->_del_tick == _del_tick + n, "must have deleted n edges");
  // Fudge the _last field so that the common assert will be happy.
  _last = (Node*) node->_last_del;
  DUIterator_Common::verify_resync();
}

void DUIterator_Fast::reset(const DUIterator_Fast& that) {
  assert(_outp              == that._outp, "already assigned _outp");
  DUIterator_Common::reset(that);
}

void DUIterator_Last::verify(const Node* node, bool at_end_ok) {
  // at_end_ok means the _outp is allowed to underflow by 1
  _outp += at_end_ok;
  DUIterator_Fast::verify(node, at_end_ok);  // check _del_tick, etc.
  _outp -= at_end_ok;
  assert(_outp == (node->_out + node->_outcnt) - 1, "pointer must point to end of nodes");
}

void DUIterator_Last::verify_limit() {
  // Do not require the limit address to be resynched.
  //verify(node, true);
  assert(_outp == _node->_out, "limit still correct");
}

void DUIterator_Last::verify_step(uint num_edges) {
  assert((int)num_edges > 0, "need non-zero edge count for loop progress");
  _outcnt   -= num_edges;
  _del_tick += num_edges;
  // Make sure we are still in sync, possibly with no more out-edges:
  const Node* node = _node;
  verify(node, true);
  assert(node->_last_del == _last, "must have deleted the edge just produced");
}

#endif //OPTO_DU_ITERATOR_ASSERT


#endif //ASSERT


// This constant used to initialize _out may be any non-null value.
// The value NULL is reserved for the top node only.
#define NO_OUT_ARRAY ((Node**)-1)

//------------------------------Node-------------------------------------------
// Create a Node, with a given number of required edges.
Node::Node( uint req ) : _idx(Compile::current()->next_unique()), _cnt(req), _max(req) {
  assert( req < (uint)(MaxNodeLimit - NodeLimitFudgeFactor), "Input limit exceeded" );
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  if (req == 0)
    _in = NULL;
  else {
    _in = (Node**)Compile::current()->node_arena()->Amalloc(req*sizeof(Node*)); // Input array
    Copy::zero_to_bytes(_in, req*sizeof(Node *));
  }
  _outcnt = 0; _outmax = 0;
  _out = NO_OUT_ARRAY;
}

//------------------------------Node-------------------------------------------
Node::Node( Node *n0 ) : _idx( Compile::current()->next_unique() ), _cnt(1), _max(1) {
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Assert we allocated space for input array already
  assert( ((void**)this)[-1] == this, "Must pass arg count to 'new'" );
  _out = NO_OUT_ARRAY;
  _in = ((Node**)this) - 1;  _outcnt = 0; _outmax = 0;
  _in[0] = n0; if (n0 != NULL) n0->add_out((Node *)this);
}

//------------------------------Node-------------------------------------------
Node::Node( Node *n0, Node *n1 ) : _idx( Compile::current()->next_unique() ), _cnt(2), _max(2) {
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Assert we allocated space for input array already
  assert( ((void**)this)[-2] == this, "Must pass arg count to 'new'" );
  _out = NO_OUT_ARRAY;
  _in = ((Node**)this) - 2;  _outcnt = 0; _outmax = 0;
  _in[0] = n0; if (n0 != NULL) n0->add_out((Node *)this);
  _in[1] = n1; if (n1 != NULL) n1->add_out((Node *)this);
}

//------------------------------Node-------------------------------------------
Node::Node( Node *n0, Node *n1, Node *n2 ) : 
  _idx( Compile::current()->next_unique() ), _cnt(3), _max(3), _outcnt(0), _outmax(0) {
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Assert we allocated space for input array already
  assert( ((void**)this)[-3] == this, "Must pass arg count to 'new'" );
  _out = NO_OUT_ARRAY;
  _in = ((Node**)this) - 3;  _outcnt = 0; _outmax = 0;
  _in[0] = n0; if (n0 != NULL) n0->add_out((Node *)this);
  _in[1] = n1; if (n1 != NULL) n1->add_out((Node *)this);
  _in[2] = n2; if (n2 != NULL) n2->add_out((Node *)this);
}

//------------------------------Node-------------------------------------------
Node::Node( Node *n0, Node *n1, Node *n2, Node *n3 ) : 
  _idx( Compile::current()->next_unique() ), _cnt(4), _max(4), _outcnt(0), _outmax(0) {
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Assert we allocated space for input array already
  assert( ((void**)this)[-4] == this, "Must pass arg count to 'new'" );
  _out = NO_OUT_ARRAY;
  _in = ((Node**)this) - 4;  _outcnt = 0; _outmax = 0;
  _in[0] = n0; if (n0 != NULL) n0->add_out((Node *)this);
  _in[1] = n1; if (n1 != NULL) n1->add_out((Node *)this);
  _in[2] = n2; if (n2 != NULL) n2->add_out((Node *)this);
  _in[3] = n3; if (n3 != NULL) n3->add_out((Node *)this);
}

//------------------------------Node-------------------------------------------
Node::Node( Node *n0, Node *n1, Node *n2, Node *n3, Node *n4 ) : 
  _idx( Compile::current()->next_unique() ), _cnt(5), _max(5), _outcnt(0), _outmax(0) {
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Assert we allocated space for input array already
  assert( ((void**)this)[-5] == this, "Must pass arg count to 'new'" );
  _out = NO_OUT_ARRAY;
  _in = ((Node**)this) - 5;  _outcnt = 0; _outmax = 0;
  _in[0] = n0; if (n0 != NULL) n0->add_out((Node *)this);
  _in[1] = n1; if (n1 != NULL) n1->add_out((Node *)this);
  _in[2] = n2; if (n2 != NULL) n2->add_out((Node *)this);
  _in[3] = n3; if (n3 != NULL) n3->add_out((Node *)this);
  _in[4] = n4; if (n4 != NULL) n4->add_out((Node *)this);
}

//------------------------------Node-------------------------------------------
Node::Node( Node *n0, Node *n1, Node *n2, Node *n3,
                    Node *n4, Node *n5, Node *n6 ) : 
  _idx( Compile::current()->next_unique() ), _cnt(7), _max(7), _outcnt(0), _outmax(0) {
  debug_only( verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Assert we allocated space for input array already
  assert( ((void**)this)[-7] == this, "Must pass arg count to 'new'" );
  _out = NO_OUT_ARRAY;
  _in = ((Node**)this) - 7;  _outcnt = 0; _outmax = 0;
  _in[0] = n0; if (n0 != NULL) n0->add_out((Node *)this);
  _in[1] = n1; if (n1 != NULL) n1->add_out((Node *)this);
  _in[2] = n2; if (n2 != NULL) n2->add_out((Node *)this);
  _in[3] = n3; if (n3 != NULL) n3->add_out((Node *)this);
  _in[4] = n4; if (n4 != NULL) n4->add_out((Node *)this);
  _in[5] = n5; if (n5 != NULL) n5->add_out((Node *)this);
  _in[6] = n6; if (n6 != NULL) n6->add_out((Node *)this);
}


//------------------------------clone------------------------------------------
// Clone a Node.  
Node *Node::clone() const {
  Compile *compile = Compile::current();
  uint s = size_of();           // Size of inherited Node
  Node *n = (Node*)compile->node_arena()->Amalloc_4(size_of() + _max*sizeof(Node*));
  Copy::conjoint_words_to_lower((HeapWord*)this, (HeapWord*)n, s);
  // Address of new input pointer array
  Node **nn = (Node**)(((char*)n)+s);
  n->_in = nn;                  // Set the new input pointer array
  // Cannot share the old output pointer array, so kill it
  n->_out = NO_OUT_ARRAY;
  // And reset the counters to 0
  n->_outcnt = n->_outmax = 0;
  // Unlock this guy, since he is not in any hash table.
  debug_only(n->_hash_lock = 0);
  // Walk the old node's input list to duplicate its edges with set_req
  uint i;
  for( i = 0; i < len(); i++ ) {
    // first null out the undefined memory
    n->_in[i] = NULL;
    // then set the proper edges
    if ( i < req() )
      n->set_req(i, in(i));
    else
      n->set_prec(i, in(i));
  }

  n->set_idx(compile->next_unique()); // Get new unique index as well
  debug_only( n->verify_construction() );
  NOT_PRODUCT(nodes_created++);
  // Do not patch over the debug_idx of a clone, because it makes it
  // impossible to break on the clone's moment of creation.
  //debug_only( n->set_debug_idx( debug_idx() ) );
  return n;                     // Return the clone
}

//---------------------------setup_is_top--------------------------------------
// Call this when changing the top node, to reassert the invariants
// required by Node::is_top.  See Compile::set_cached_top_node.
void Node::setup_is_top() {
  if (this == (Node*)Compile::current()->top()) {
    // This node has just become top.  Kill its out array.
    _outcnt = _outmax = 0;
    _out = NULL;                           // marker value for top
    assert(is_top(), "must be top");
  } else {
    if (_out == NULL)  _out = NO_OUT_ARRAY;
    assert(!is_top(), "must not be top");
  }
}


//------------------------------~Node------------------------------------------
// Fancy destructor; eagerly attempt to reclaim Node numberings and storage
extern int reclaim_idx ;
extern int reclaim_in  ;
extern int reclaim_node;
void Node::destruct() { 
  // Eagerly reclaim unique Node numberings
  Compile* compile = Compile::current();
  if ((uint)_idx+1 == compile->unique()) {
    compile->set_unique(compile->unique()-1); 
#ifdef ASSERT
    reclaim_idx++;
#endif
  }
  // Walk the input array, freeing the corresponding output edges
  _cnt = _max;  // forget req/prec distinction
  uint i;
  for( i = 0; i < _max; i++ ) {
    set_req(i, NULL);
    //assert(def->out(def->outcnt()-1) == (Node *)this,"bad def-use hacking in reclaim");
  }
  assert(outcnt() == 0, "deleting a node must not leave a dangling use");
  // See if the input array was allocated just prior to the object
  int edge_size = _max*sizeof(void*);
  int out_edge_size = _outmax*sizeof(void*);
  char *edge_end = ((char*)_in) + edge_size;
  char *out_array = (char*)(_out == NO_OUT_ARRAY? NULL: _out);
  char *out_edge_end = out_array + out_edge_size;
  int node_size = size_of();

  // Free the output edge array 
  if (out_edge_size > 0) {
#ifdef ASSERT
    if( out_edge_end == compile->node_arena()->hwm() )
      reclaim_in  += out_edge_size;  // count reclaimed out edges with in edges
#endif
    compile->node_arena()->Afree(out_array, out_edge_size);
  }

  // Free the input edge array and the node itself
  if( edge_end == (char*)this ) {
#ifdef ASSERT
    if( edge_end+node_size == compile->node_arena()->hwm() ) {
      reclaim_in  += edge_size;
      reclaim_node+= node_size;
    }
#else
    // It was; free the input array and object all in one hit
    compile->node_arena()->Afree(_in,edge_size+node_size);
#endif 
  } else {

    // Free just the input array
#ifdef ASSERT
    if( edge_end == compile->node_arena()->hwm() ) 
      reclaim_in  += edge_size;
#endif 
    compile->node_arena()->Afree(_in,edge_size);

    // Free just the object
#ifdef ASSERT
    if( ((char*)this) + node_size == compile->node_arena()->hwm() ) 
      reclaim_node+= node_size;
#else
    compile->node_arena()->Afree(this,node_size);
#endif 
  }

#ifdef ASSERT
  // We will not actually delete the storage, but we'll make the node unusable.
  *(address*)this = badAddress;  // smash the C++ vtbl, probably
  _in = _out = (Node**) badAddress;
  _max = _cnt = _outmax = _outcnt = 0;
#endif
}

//------------------------------grow-------------------------------------------
// Grow the input array, making space for more edges
void Node::grow( uint len ) {
  uint new_max = _max;
  if( !new_max ) {
    _max = 4;
    _in =(Node**)Compile::current()->node_arena()->Amalloc(_max*sizeof(Node*));
    Copy::zero_to_bytes(&_in[0] ,(_max)*sizeof(Node*));    // NULL all new space
    return;
  }
  while( new_max <= len ) new_max <<= 1; // Find next power-of-2
  // Trimming to limit allows a uint8 to handle up to 255 edges.
  // Previously I was using only powers-of-2 which peaked at 128 edges.
  //if( new_max >= limit ) new_max = limit-1;
  _in =(Node**)Compile::current()->node_arena()->Arealloc(_in,_max*sizeof(Node*),new_max*sizeof(Node*));
  Copy::zero_to_bytes(&_in[_max], (new_max-_max)*sizeof(Node*));    // NULL all new space
  _max = new_max;               // Record new max length
  // This assertion makes sure that Node::_max is wide enough to
  // represent the numerical value of new_max.
  assert(_max == new_max && _max > len, "int width of _max is too small");
}

//-----------------------------out_grow----------------------------------------
// Grow the input array, making space for more edges
void Node::out_grow( uint len ) {
  assert(!is_top(), "cannot grow a top node's out array");
  uint new_max = _outmax;
  if( !new_max ) {
    _outmax = 4;
    _out = (Node **)Compile::current()->node_arena()->Amalloc(_outmax*sizeof(Node*));
    return;
  }
  while( new_max <= len ) new_max <<= 1; // Find next power-of-2
  // Trimming to limit allows a uint8 to handle up to 255 edges.
  // Previously I was using only powers-of-2 which peaked at 128 edges.
  //if( new_max >= limit ) new_max = limit-1;
  assert(_out != NULL && _out != NO_OUT_ARRAY, "out must have sensible value");
  _out =(Node**)Compile::current()->node_arena()->Arealloc(_out,_outmax*sizeof(Node*),new_max*sizeof(Node*));
  //Copy::zero_to_bytes(&_out[_outmax], (new_max-_outmax)*sizeof(Node*)); // NULL all new space
  _outmax = new_max;               // Record new max length
  // This assertion makes sure that Node::_max is wide enough to
  // represent the numerical value of new_max.
  assert(_outmax == new_max && _outmax > len, "int width of _outmax is too small");
}

//------------------------------add_req----------------------------------------
// Add a new required input at the end
void Node::add_req( Node *n ) {

  // Look to see if I can move precedence down one without reallocating
  if( (_cnt >= _max) || (in(_max-1) != NULL) ) 
    grow( _max+1 );

  // Find a precedence edge to move
  if( in(_cnt) != NULL ) {       // Next precedence edge is busy?
    uint i;
    for( i=_cnt; i<_max; i++ ) 
      if( in(i) == NULL )       // Find the NULL at end of prec edge list
        break;                  // There must be one, since we grew the array
    _in[i] = in(_cnt);          // Move prec over, making space for req edge
  }
  _in[_cnt++] = n;            // Stuff over old prec edge
  if (n != NULL) n->add_out((Node *)this);
}

//---------------------------add_req_batch-------------------------------------
// Add a new required input at the end
void Node::add_req_batch( Node *n, uint m ) {
  // check various edge cases
  if ((int)m <= 1) {
    assert((int)m >= 0, "oob");
    if (m != 0)  add_req(n);
    return;
  }

  // Look to see if I can move precedence down one without reallocating
  if( (_cnt+m) > _max || _in[_max-m] ) 
    grow( _max+m );

  // Find a precedence edge to move
  if( _in[_cnt] != NULL ) {     // Next precedence edge is busy?
    uint i;
    for( i=_cnt; i<_max; i++ ) 
      if( _in[i] == NULL )      // Find the NULL at end of prec edge list
        break;                  // There must be one, since we grew the array
    // Slide all the precs over by m positions (assume #prec << m).
    Copy::conjoint_words_to_higher((HeapWord*)&_in[_cnt], (HeapWord*)&_in[_cnt+m], ((i-_cnt)*sizeof(Node*)));
  }

  // Stuff over the old prec edges
  for(uint i=0; i<m; i++ ) {
    _in[_cnt++] = n;
  }

  // Insert multiple out edges on the node.
  if (n != NULL && !n->is_top()) {
    for(uint i=0; i<m; i++ ) {
      n->add_out((Node *)this);
    }
  }
}

//------------------------------del_req----------------------------------------
// Delete the required edge and compact the edge array
void Node::del_req( uint idx ) {
  // First remove corresponding def-use edge
  Node *n = in(idx);
  if (n != NULL) n->del_out((Node *)this);
  _in[idx] = in(--_cnt);  // Compact the array
  _in[_cnt] = NULL;       // NULL out emptied slot
}

//------------------------------ins_req----------------------------------------
// Insert a new required input at the end
void Node::ins_req( uint idx, Node *n ) {
  add_req(NULL);                // Make space
  assert( idx < _max, "Must have allocated enough space");
  // Slide over
  if(_cnt-idx-1 > 0) {
    Copy::conjoint_words_to_higher((HeapWord*)&_in[idx], (HeapWord*)&_in[idx+1], ((_cnt-idx-1)*sizeof(Node*)));
  }
  _in[idx] = n;                            // Stuff over old required edge
  if (n != NULL) n->add_out((Node *)this); // Add reciprocal def-use edge
}

//-------------------------disconnect_inputs-----------------------------------
// NULL out all inputs to eliminate incoming Def-Use edges.
// Return the number of edges between 'n' and 'this'
int Node::disconnect_inputs(Node *n) {
  int edges_to_n = 0;

  uint cnt = req();
  for( uint i = 0; i < cnt; ++i ) {
    if( in(i) == 0 ) continue;
    if( in(i) == n ) ++edges_to_n;
    set_req(i, NULL);
  }
  // Remove precedence edges if any exist
  // Note: Safepoints may have precedence edges, even during parsing
  if( (req() != len()) && (in(req()) != NULL) ) {
    uint max = len();
    for( uint i = 0; i < max; ++i ) {
      if( in(i) == 0 ) continue;
      if( in(i) == n ) ++edges_to_n;
      set_prec(i, NULL);
    }
  }

  // Node::destruct requires all out edges be deleted first
  // debug_only(destruct();)   // no reuse benefit expected
  return edges_to_n;
}


//------------------------------add_prec---------------------------------------
// Add a new precedence input.  Precedence inputs are unordered, with 
// duplicates removed and NULLs packed down at the end.
void Node::add_prec( Node *n ) {

  // Check for NULL at end
  if( _cnt >= _max || in(_max-1) ) 
    grow( _max+1 );

  // Find a precedence edge to move
  uint i = _cnt;
  while( in(i) != NULL ) i++;
  _in[i] = n;                                // Stuff prec edge over NULL
  if ( n != NULL) n->add_out((Node *)this);  // Add mirror edge
}

//------------------------------rm_prec----------------------------------------
// Remove a precedence input.  Precedence inputs are unordered, with 
// duplicates removed and NULLs packed down at the end.
void Node::rm_prec( uint j ) {

  // Find end of precedence list to pack NULLs
  uint i;
  for( i=j; i<_max; i++ ) 
    if( !_in[i] )               // Find the NULL at end of prec edge list
      break;
  if (_in[j] != NULL) _in[j]->del_out((Node *)this);
  _in[j] = _in[--i];            // Move last element over removed guy
  _in[i] = NULL;                // NULL out last element
}

//------------------------------size_of----------------------------------------
uint Node::size_of() const { return sizeof(*this); }

//------------------------------ideal_reg--------------------------------------
uint Node::ideal_reg() const { return 0; }

//------------------------------jvms-------------------------------------------
JVMState* Node::jvms() const { return NULL; }

#ifdef ASSERT
//------------------------------jvms-------------------------------------------
bool Node::verify_jvms(const JVMState* using_jvms) const {
  for (JVMState* jvms = this->jvms(); jvms != NULL; jvms = jvms->caller()) {
    if (jvms == using_jvms)  return true;
  }
  return false;
}
#endif

//------------------------------oop_map----------------------------------------
OopMap *Node::oop_map() const { ShouldNotCallThis(); return NULL; }

//------------------------------set_oop_map------------------------------------
void Node::set_oop_map(OopMap *) { ShouldNotCallThis(); }

//------------------------------format-----------------------------------------
// Print as assembly
void Node::format( PhaseRegAlloc * ) const {}
//------------------------------emit-------------------------------------------
// Emit bytes starting at parameter 'ptr'.  
void Node::emit(CodeBuffer &cbuf, PhaseRegAlloc *ra_) const {} 
//------------------------------size-------------------------------------------
// Size of instruction in bytes
uint Node::size(PhaseRegAlloc *ra_) const { return 0; }

//------------------------------CFG Construction-------------------------------
// Nodes that begin basic blocks, e.g. RootNode, RegionNode, StartNode.
// I also need the machine specific versions of these.
int Node::is_block_start() const { return 0; }

// Nodes that end basic blocks, e.g. IfTrue/IfFalse, JumpProjNode, Root,
// Goto and Return.  
const Node *Node::is_block_proj() const { return 0; }

// Minimum guaranteed type
const Type *Node::bottom_type() const { return Type::BOTTOM; }

//------------------------------Identity---------------------------------------
// Return a node that the given node is equivalent to.
Node *Node::Identity( PhaseTransform * ) {
  return this;                  // Default to no identities
}

//------------------------------Value------------------------------------------
// Compute a new Type for a node using the Type of the inputs.
const Type *Node::Value( PhaseTransform * ) const {
  return bottom_type();         // Default to worst-case Type
}

//------------------------------Ideal------------------------------------------
//
// 'Idealize' the graph rooted at this Node.
//
// In order to be efficient and flexible there are some subtle invariants
// these Ideal calls need to hold.  Running with '+VerifyIterativeGVN' checks
// these invariants, although its too slow to have on by default.  If you are
// hacking an Ideal call, be sure to test with +VerifyIterativeGVN!
//
// The Ideal call almost arbitrarily reshape the graph rooted at the 'this'
// pointer.  If ANY change is made, it must return the root of the reshaped
// graph - even if the root is the same Node.  Example: swapping the inputs
// to an AddINode gives the same answer and same root, but you still have to
// return the 'this' pointer instead of NULL.
//
// You cannot return an OLD Node, except for the 'this' pointer.  Use the
// Identity call to return an old Node; basically if Identity can find 
// another Node have the Ideal call make no change and return NULL.
// Example: AddINode::Ideal must check for add of zero; in this case it
// returns NULL instead of doing any graph reshaping.
//
// You cannot modify any old Nodes except for the 'this' pointer.  Due to
// sharing there may be other users of the old Nodes relying on their current
// semantics.  Modifying them will break the other users.
// Example: when reshape "(X+3)+4" into "X+7" you must leave the Node for
// "X+3" unchanged in case it is shared.
//
// If you modify the 'this' pointer's inputs, you must use 'set_req' with
// def-use info.  If you are making a new Node (either as the new root or
// some new internal piece) you must NOT use set_req with def-use info.
// You can make a new Node with either 'new' or 'clone'.  In either case,
// def-use info is (correctly) not generated.  
// Example: reshape "(X+3)+4" into "X+7":
//    set_req(1,in(1)->in(1) /* grab X */, du /* must use DU on 'this' */);
//    set_req(2,phase->intcon(7),du);
//    return this;
// Example: reshape "X*4" into "X<<1"
//    return new (2) LShiftINode( in(1), phase->intcon(1) );
// 
// You must call 'phase->transform(X)' on any new Nodes X you make, except
// for the returned root node.  Example: reshape "X*31" with "(X<<5)-1".
//    Node *shift =phase->transform(new(2)LShiftINode(in(1),phase->intcon(5)));
//    return new (2) AddINode(shift, phase->intcon(-1));
//
// When making a Node for a constant use 'phase->makecon' or 'phase->intcon'.
// These forms are faster than 'phase->transform(new (1) ConNode())' and Do
// The Right Thing with def-use info.
//
// You cannot bury the 'this' Node inside of a graph reshape.  If the reshaped
// graph uses the 'this' Node it must be the root.  If you want a Node with 
// the same Opcode as the 'this' pointer use 'clone'.
//
Node *Node::Ideal(PhaseGVN *phase, bool can_reshape) {
  return NULL;                  // Default to being Ideal already
}

//------------------------------remove_dead_region-----------------------------
// This control node is dead.  Follow the subgraph below it making everything
// using it dead as well.  This will happen normally via the usual IterGVN
// worklist but this call is more efficient.  Do not update use-def info
// inside the dead region, just at the borders.
static int kill_dead_code( Node *dead, PhaseIterGVN *igvn ) {
  // Con's are a popular node to re-hit in the hash table again.
  if( dead->is_Con() ) return 0;
  Node *top = igvn->C->top();
  igvn->_worklist.remove(dead);
  igvn->hash_delete(dead);
  igvn->set_type(dead, Type::TOP);
  int progress = 0;

  // For all Users of the Dead...    ;-)
  for (DUIterator_Last kmin, k = dead->last_outs(kmin); k >= kmin; ) {
    Node* use = dead->last_out(k);
    if( use->in(0) == dead ) {  // Found another dead node
      progress |= kill_dead_code(use,igvn); // Recurse
      // Refresh the iterator, since any number of kills might have happened.
      k = dead->last_outs(kmin);
    } else {                    // Else found a not-dead user
      igvn->hash_delete(use);   // Yank from hash table prior to mod
      for (uint j = 1; j < use->req(); j++) {
        if (use->in(j) == dead) { // Turn all dead uses into TOP uses
          use->set_req(j, top);
          --k;
        }
      }
      igvn->_worklist.push(use);
    }
  }
  // Kill all inputs to the dead guy
  for( uint i=0; i < dead->req(); i++ ) {
    Node *n = dead->in(i);      // Get input to dead guy
    if( n && n != top ) {       // Input is valid?
      progress = 1;
      dead->set_req( i, top );  // Smash input away
      if (n->outcnt() == 0)     // Input also goes dead?
        kill_dead_code( n, igvn );// Clear it out as well
    }
  }
  return progress;
}

//------------------------------remove_dead_region-----------------------------
bool Node::remove_dead_region(PhaseGVN *phase, bool can_reshape) {
  Node *n = in(0);
  if( !n ) return false;
  // Lost control into this guy?  I.e., it became unreachable?
  // Aggressively kill all unreachable code.
  if (can_reshape && n->is_top()) {
    return kill_dead_code(this, phase->is_IterGVN());
  }

  const RegionNode *region = n->is_Region();
  if( region && region->is_copy() ) {
    Node *m = region->nonnull_req();
    set_req(0, m);
    return true;
  }
  return false;
}

//------------------------------Ideal_DU_postCCP-------------------------------
// Idealize graph, using DU info.  Must clone result into new-space
Node *Node::Ideal_DU_postCCP( PhaseCCP * ) {
  return NULL;                 // Default to no change
}

//------------------------------hash-------------------------------------------
// Hash function over Nodes.
uint Node::hash() const {
  uint sum = 0;
  for( uint i=0; i<_cnt; i++ )  // Add in all inputs
    sum = (sum<<1)-(uintptr_t)in(i);        // Ignore embedded NULLs
  return (sum>>2) + _cnt + Opcode();
}

//------------------------------cmp--------------------------------------------
// Compare special parts of simple Nodes
uint Node::cmp( const Node &n ) const {
  return 1;                     // Must be same
}


// Get an integer constant from a ConstNode.
// Returns True if it is an integer ConstNode and puts the constant
// into 'con', otherwise it returns False.
int Node::get_int( jint * ) const { return false; }

jint Node::get_int( ) const {
  if( Opcode() == Op_ConI ) {
    return ((ConINode*)this)->type()->is_int()->get_con();
  } else {
#ifdef _LP64
    assert( false, "ints are not ptrs");
#endif
    assert( Opcode() == Op_ConP, "" );
    return ((ConPNode*)this)->type()->is_ptr()->get_con();
  }
}

// Get a pointer constant from a ConstNode.
// Returns the constant if it is a pointer ConstNode
intptr_t Node::get_ptr( ) const {
  assert( Opcode() == Op_ConP, "" );
  return ((ConPNode*)this)->type()->is_ptr()->get_con();
}

// Get a long constant from a ConstNode.
// Returns the constant if it is a long ConstNode
jlong Node::get_long( ) const {
  assert( Opcode() == Op_ConL, "" );
  return ((ConLNode*)this)->type()->is_long()->get_con();
}

// Get a double constant from a ConstNode.
// Returns the constant if it is a double ConstNode
jdouble Node::getd( ) const { 
  assert( Opcode() == Op_ConD, "" ); 
  return ((ConDNode*)this)->type()->is_double_constant()->getd(); 
}

// Get a float constant from a ConstNode.
// Returns the constant if it is a float ConstNode
jfloat Node::getf( ) const { 
  assert( Opcode() == Op_ConF, "" ); 
  return ((ConFNode*)this)->type()->is_float_constant()->getf(); 
}


//----------------------------NotANode----------------------------------------
// Used in debugging code to avoid walking across dead or uninitialized edges.
static inline bool NotANode(const Node* n) {
  if (n == NULL)                   return true;
  if (((intptr_t)n & 1) != 0)      return true;  // uninitialized, etc.
  if (*(address*)n == badAddress)  return true;  // kill by Node::destruct
  return false;
}


//------------------------------find------------------------------------------
// Find a neighbor of this Node with the given _idx
// If idx is negative, find its absolute value, following both _in and _out.
static void find_recur( Node* &result, Node *n, int idx, bool only_ctrl, 
                        VectorSet &old_space, VectorSet &new_space ) {
  int node_idx = (idx >= 0) ? idx : -idx;
  if (NotANode(n))  return;  // Gracefully handle NULL, -1, 0xabababab, etc.
  // Contained in new_space or old_space?
  VectorSet *v = Compile::current()->node_arena()->contains(n) ? &new_space : &old_space;
  if( v->test(n->_idx) ) return;
  if( (int)n->_idx == node_idx) {
    if (result != NULL)
      tty->print("find: " INTPTR_FORMAT " and " INTPTR_FORMAT " both have idx==%d\n",
                 (uintptr_t)result, (uintptr_t)n, node_idx);
    result = n;
  }
  v->set(n->_idx);
  for( uint i=0; i<n->len(); i++ ) {
    if( only_ctrl && !(n->is_Region()) && (n->Opcode() != Op_Root) && (i != TypeFunc::Control) ) continue;
    find_recur( result, n->in(i), idx, only_ctrl, old_space, new_space );
  }
  // Search along forward edges also:
  if (idx < 0 && !only_ctrl) {
    for( uint j=0; j<n->outcnt(); j++ ) {
      find_recur( result, n->raw_out(j), idx, only_ctrl, old_space, new_space );
    }
  }
}

#ifndef PRODUCT
// call this from debugger:
Node* find_node(Node* n, int idx) {
  return n->find(idx);
}

//------------------------------find-------------------------------------------
Node* Node::find(int idx) const {
  ResourceArea *area = Thread::current()->resource_area();
  VectorSet old_space(area), new_space(area);
  Node* result = NULL;
  find_recur( result, (Node*) this, idx, false, old_space, new_space );
  return result;
}

//------------------------------find_ctrl--------------------------------------
// Find an ancestor to this node in the control history with given _idx
Node* Node::find_ctrl(int idx) const {
  ResourceArea *area = Thread::current()->resource_area();
  VectorSet old_space(area), new_space(area);
  Node* result = NULL;
  find_recur( result, (Node*) this, idx, true, old_space, new_space );
  return result;
}
#endif



#ifndef PRODUCT
int Node::_in_dump_cnt = 0;

// -----------------------------Name-------------------------------------------
extern const char *NodeClassNames[];
const char *Node::Name() const { return NodeClassNames[Opcode()]; }

//------------------------------dump------------------------------------------
// Dump a Node
void Node::dump() const {
  _in_dump_cnt++;
  tty->print("%c%d\t%s\t=== ",
    Compile::current()->node_arena()->contains(this) ? ' ' : 'o', _idx, Name());

  // Dump the required and precedence inputs
  dump_req();
  dump_prec();
  // Dump the outputs
  dump_out();

  // Dump node-specific info
  dump_spec();
#ifdef ASSERT
  // Dump the non-reset _debug_idx
  if( Verbose && WizardMode ) {
    tty->print("  [%d]",debug_idx());
  }
#endif

  const Type *t = bottom_type();

  if (t != NULL && (t->isa_instptr() || t->isa_klassptr())) {
    const TypeInstPtr  *toop = t->isa_instptr();
    const TypeKlassPtr *tkls = t->isa_klassptr();
    ciKlass*           klass = toop ? toop->klass() : (tkls ? tkls->klass() : NULL );
    if( klass && klass->is_loaded() && klass->is_interface() ) {
      tty->print("  Interface:");
    } else if( toop ) {
      tty->print("  Oop:");
    } else if( tkls ) {
      tty->print("  Klass:");
    }
    t->dump();
  } else if( t == Type::MEMORY ) {
    tty->print("  Memory:");
    MemNode::dump_adr_type(this, adr_type());
  } else if( Verbose || WizardMode ) {
    tty->print("  Type:");
    if( t ) {
      t->dump();
    } else {
      tty->print("no type");
    }
  }
  tty->print("\n");
  _in_dump_cnt--;
}

//------------------------------dump_req--------------------------------------
void Node::dump_req() const {
  // Dump the required input edges
  for (uint i = 0; i < req(); i++) {    // For all required inputs
    Node* d = in(i);
    if (d == NULL) {
      tty->print("_ ");
    } else if (NotANode(d)) {
      tty->print("NotANode ");  // uninitialized, sentinel, garbage, etc.
    } else {
      tty->print("%c%d ", Compile::current()->node_arena()->contains(d) ? ' ' : 'o', d->_idx);
    }
  }
}


//------------------------------dump_prec-------------------------------------
void Node::dump_prec() const {
  // Dump the precedence edges
  int any_prec = 0;
  for (uint i = req(); i < len(); i++) {       // For all precedence inputs
    Node* p = in(i);
    if (p != NULL) {
      if( !any_prec++ ) tty->print(" |");
      if (NotANode(p)) { tty->print("NotANode "); continue; }
      tty->print("%c%d ", Compile::current()->node_arena()->contains(in(i)) ? ' ' : 'o', in(i)->_idx);
    }
  }
}

//------------------------------dump_out--------------------------------------
void Node::dump_out() const {
  // Delimit the output edges
  tty->print(" [[");
  // Dump the output edges
  for (uint i = 0; i < _outcnt; i++) {    // For all outputs
    Node* u = _out[i];
    if (u == NULL) {
      tty->print("_ ");
    } else if (NotANode(u)) {
      tty->print("NotANode ");
    } else {
      tty->print("%c%d ", Compile::current()->node_arena()->contains(u) ? ' ' : 'o', u->_idx);
    }
  }
  tty->print("]] ");
}

//------------------------------dump_recur-------------------------------------
static void dump_recur( const Node *n, int d, bool only_ctrl,
                        VectorSet &old_space, VectorSet &new_space ) {
  if (NotANode(n))  return;  // Gracefully handle NULL, -1, 0xabababab, etc.
  if (d == 0)       return;  // Done with recursion
  // Contained in new_space or old_space?
  VectorSet *v = Compile::current()->node_arena()->contains(n) ? &new_space : &old_space;
  if( v->test_set(n->_idx) ) return; // Dumped already
  // Only dumping control edges?
  if( only_ctrl && !n->is_CFG() ) return;

  if( d > 0 ) {                 // Forward dump
    for( uint i=0; i<n->len(); i++ ) 
      dump_recur( n->in (i), d-1, only_ctrl, old_space, new_space );
    n->dump();
  } else {                      // Backwards dump
    n->dump();
    for( uint i=0; i<n->outcnt(); i++ ) 
      dump_recur( n->raw_out(i), d+1, only_ctrl, old_space, new_space );
  }
}

//------------------------------dump-------------------------------------------
void Node::dump(int d) const {
  ResourceArea *area = Thread::current()->resource_area();
  VectorSet old_space(area), new_space(area);
  dump_recur( this, d, false, old_space, new_space );
}

//------------------------------dump_ctrl--------------------------------------
// Dump a Node's control history to depth
void Node::dump_ctrl(int d) const {
  //this was a merge-o: //unique_top = NULL;
  ResourceArea *area = Thread::current()->resource_area();
  VectorSet old_space(area), new_space(area);
  dump_recur( this, d, true, old_space, new_space );
}

// VERIFICATION CODE
// For each input edge to a node (ie - for each Use-Def edge), verify that
// there is a corresponding Def-Use edge.
//------------------------------verify_edges-----------------------------------
void Node::verify_edges(Unique_Node_List &visited) {
  uint i, j, idx;
  int  cnt;
  Node *n;

  // Recursive termination test
  if (visited.member(this))  return;
  visited.push(this);

  // Walk over all input edges, checking for correspondance
  for( i = 0; i < len(); i++ ) {
    n = in(i);
    if (n != NULL && !n->is_top()) {
      // Count instances of (Node *)this
      cnt = 0;
      for (idx = 0; idx < n->_outcnt; idx++ ) {
        if (n->_out[idx] == (Node *)this)  cnt++;
      }
      assert( cnt > 0,"Failed to find Def-Use edge." );
      // Check for duplicate edges
      // walk the input array downcounting the input edges to n
      for( j = 0; j < len(); j++ ) {
        if( in(j) == n ) cnt--;
      }
      assert( cnt == 0,"Mismatched edge count.");
    } else if (n == NULL) {
      assert(i >= req() || i == 0 || is_Region() || is_Phi(), "only regions or phis have null data edges");
    } else {
      assert(n->is_top(), "sanity");
      // Nothing to check.
    }
  }
  // Recursive walk over all input edges
  for( i = 0; i < len(); i++ ) {
    n = in(i);
    if( n != NULL )
      in(i)->verify_edges(visited);
  }
}

//------------------------------verify_recur-----------------------------------
static const Node *unique_top = NULL;

void Node::verify_recur(const Node *n, int verify_depth,
                        VectorSet &old_space, VectorSet &new_space) {
  if ( verify_depth == 0 )  return;
  if (verify_depth > 0)  --verify_depth;

  Compile* C = Compile::current();

  // Contained in new_space or old_space?
  VectorSet *v = C->node_arena()->contains(n) ? &new_space : &old_space;
  // Check for visited in the proper space.  Numberings are not unique
  // across spaces so we need a seperate VectorSet for each space.
  if( v->test_set(n->_idx) ) return;

  if (n->is_Con() && n->bottom_type() == Type::TOP) {
    if (C->cached_top_node() == NULL)
      C->set_cached_top_node((Node*)n);
    assert(C->cached_top_node() == n, "TOP node must be unique");
  }

  for( uint i = 0; i < n->len(); i++ ) {
    Node *x = n->in(i);
    if (!x || x->is_top()) continue;

    // Verify my input has a def-use edge to me
    if (true /*VerifyDefUse*/) {
      // Count use-def edges from n to x
      int cnt = 0;
      for( uint j = 0; j < n->len(); j++ ) 
        if( n->in(j) == x )
          cnt++;
      // Count def-use edges from x to n
      uint max = x->_outcnt;
      for( uint k = 0; k < max; k++ )
        if (x->_out[k] == n)
          cnt--;
      assert( cnt == 0, "mismatched def-use edge counts" );
    }

    verify_recur(x, verify_depth, old_space, new_space);
  }

}

//------------------------------verify-----------------------------------------
// Check Def-Use info for my subgraph
void Node::verify() const {
  Compile* C = Compile::current();
  Node* old_top = C->cached_top_node();
  ResourceArea *area = Thread::current()->resource_area();
  VectorSet old_space(area), new_space(area);
  verify_recur(this, -1, old_space, new_space);
  C->set_cached_top_node(old_top);
}
#endif


//------------------------------walk-------------------------------------------
// Graph walk, with both pre-order and post-order functions
void Node::walk(NFunc pre, NFunc post, void *env) {
  VectorSet visited(Thread::current()->resource_area()); // Setup for local walk
  walk_(pre, post, env, visited); 
}

void Node::walk_(NFunc pre, NFunc post, void *env, VectorSet &visited) {
  if( visited.test_set(_idx) ) return;
  pre(*this,env);               // Call the pre-order walk function
  for( uint i=0; i<_max; i++ )
    if( in(i) )                 // Input exists and is not walked?
      in(i)->walk_(pre,post,env,visited); // Walk it with pre & post functions
  post(*this,env);              // Call the post-order walk function
}

void Node::nop(Node &, void*) {}

//------------------------------Registers--------------------------------------
// Do we Match on this edge index or not?  Generally false for Control
// and true for everything else.  Weird for calls & returns.
uint Node::match_edge(uint idx) const {
  return idx;                   // True for other than index 0 (control)
}

// Register classes are defined for specific machines
const RegMask &Node::out_RegMask() const { 
  ShouldNotCallThis();
  return *(new RegMask());
}

const RegMask &Node::in_RegMask(uint) const { 
  ShouldNotCallThis();
  return *(new RegMask());
}

//=============================================================================
//-----------------------------------------------------------------------------
void Node_Array::reset( Arena *new_arena ) {
  _a->Afree(_nodes,_max*sizeof(Node*));
  _max   = 0;
  _nodes = NULL;
  _a     = new_arena;
}

//------------------------------clear------------------------------------------
// Clear all entries in _nodes to NULL but keep storage
void Node_Array::clear( ) {
  Copy::zero_to_bytes( _nodes, _max*sizeof(Node*) );
}

//-----------------------------------------------------------------------------
void Node_Array::grow( uint i ) {
  if( !_max ) { 
    _max = 1;
    _nodes = (Node**)_a->Amalloc( _max * sizeof(Node*) );
    _nodes[0] = NULL;
  }
  uint old = _max;
  while( i >= _max ) _max <<= 1;        // Double to fit
  _nodes = (Node**)_a->Arealloc( _nodes, old*sizeof(Node*),_max*sizeof(Node*));
  Copy::zero_to_bytes( &_nodes[old], (_max-old)*sizeof(Node*) );
}

//-----------------------------------------------------------------------------
void Node_Array::insert( uint i, Node *n ) {
  if( _nodes[_max-1] ) grow(_max);      // Get more space if full
  Copy::conjoint_words_to_higher((HeapWord*)&_nodes[i], (HeapWord*)&_nodes[i+1], ((_max-i-1)*sizeof(Node*)));
  _nodes[i] = n;
}

//-----------------------------------------------------------------------------
void Node_Array::remove( uint i ) {
  Copy::conjoint_words_to_lower((HeapWord*)&_nodes[i+1], (HeapWord*)&_nodes[i], ((_max-i-1)*sizeof(Node*)));
  _nodes[_max-1] = NULL;
}

//-----------------------------------------------------------------------------
void Node_Array::sort( C_sort_func_t func) {
  qsort( _nodes, _max, sizeof( Node* ), func );
}

//-----------------------------------------------------------------------------
void Node_Array::dump() const {
#ifndef PRODUCT
  for( uint i = 0; i < _max; i++ ) {
    Node *nn = _nodes[i];
    if( nn != NULL ) {
      tty->print("%5d--> ",i); nn->dump();
    }
  }
#endif
}

//=============================================================================
//------------------------------yank-------------------------------------------
// Find and remove
void Node_List::yank( Node *n ) {
  uint i;
  for( i = 0; i < _cnt; i++ )
    if( _nodes[i] == n )
      break;

  if( i < _cnt )
    _nodes[i] = _nodes[--_cnt];
}

//------------------------------dump-------------------------------------------
void Node_List::dump() const {
#ifndef PRODUCT
  for( uint i = 0; i < _cnt; i++ )
    if( _nodes[i] ) {
      tty->print("%5d--> ",i); 
      _nodes[i]->dump();
    }
#endif
}

//=============================================================================
//------------------------------remove-----------------------------------------
void Unique_Node_List::remove( Node *n ) {
  if( _in_worklist[n->_idx] ) {
    for( uint i = 0; i < size(); i++ )
      if( _nodes[i] == n ) {
        map(i,Node_List::pop());
        _in_worklist >>= n->_idx;
        return;
      }
    ShouldNotReachHere();
  }
}

//-----------------------remove_useless_nodes----------------------------------
// Remove useless nodes from worklist
void Unique_Node_List::remove_useless_nodes(VectorSet &useful) {

  for( uint i = 0; i < size(); ++i ) {
    Node *n = at(i);
    assert( n != NULL, "Did not expect null entries in worklist");
    if( ! useful.test(n->_idx) ) {
      _in_worklist >>= n->_idx;
      map(i,Node_List::pop());
      // Node *replacement = Node_List::pop();
      // if( i != size() ) { // Check if removing last entry
      //   _nodes[i] = replacement;
      // }
      --i;  // Visit popped node
      // If it was last entry, loop terminates since size() was also reduced
    }
  }
}

//=============================================================================
void Node_Stack::grow() {
  size_t old_top = _inode_top - _inodes; // save _top
  size_t old_max = _inode_max - _inodes;
  size_t max = old_max << 1;             // max * 2
  _inodes = REALLOC_ARENA_ARRAY(_a, INode, _inodes, old_max, max);
  _inode_max = _inodes + max;
  _inode_top = _inodes + old_top;        // restore _top
}

//=============================================================================
uint TypeNode::size_of() const { return sizeof(*this); }
#ifndef PRODUCT
void TypeNode::dump_spec() const { 
  if( !Verbose && !WizardMode ) {
    // standard dump does this in Verbose and WizardMode
    tty->print(" #"); _type->dump();
  }
}
#endif
uint TypeNode::hash() const { 
  return Node::hash() + _type->hash();
}
uint TypeNode::cmp( const Node &n ) const
{ return !Type::cmp( _type, ((TypeNode&)n)._type ); }
const Type *TypeNode::bottom_type() const { return _type; }
const Type *TypeNode::Value( PhaseTransform * ) const { return _type; }

void TypeNode::raise_bottom_type(const Type* new_type) {
  if (VerifyAliases) {
    assert(new_type->higher_equal(_type), "new type must refine old type");
  }
  set_type(new_type);
}

//------------------------------ideal_reg--------------------------------------
uint TypeNode::ideal_reg() const { 
  return Matcher::base2reg[_type->base()];
}


