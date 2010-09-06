#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)phaseX.cpp	1.234 03/12/23 16:42:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_phaseX.cpp.incl"

//=============================================================================
#define NODE_HASH_MINIMUM_SIZE    255
//------------------------------NodeHash---------------------------------------
NodeHash::NodeHash(uint est_max_size) : 
  _max( round_up(est_max_size < NODE_HASH_MINIMUM_SIZE ? NODE_HASH_MINIMUM_SIZE : est_max_size) ),
  _a(Thread::current()->resource_area()),
  _table( NEW_ARENA_ARRAY( _a , Node* , _max ) ), // (Node**)_a->Amalloc(_max * sizeof(Node*)) ), 
  _inserts(0), _insert_limit( insert_limit() ),
  _look_probes(0), _lookup_hits(0), _lookup_misses(0), 
  _total_insert_probes(0), _total_inserts(0), 
  _insert_probes(0), _grows(0) {
  // _sentinel must be in the current node space
  _sentinel = new (1) ProjNode(NULL, TypeFunc::Control);
  memset(_table,0,sizeof(Node*)*_max);
}

//------------------------------NodeHash---------------------------------------
NodeHash::NodeHash(Arena *arena, uint est_max_size) : 
  _max( round_up(est_max_size < NODE_HASH_MINIMUM_SIZE ? NODE_HASH_MINIMUM_SIZE : est_max_size) ),
  _a(arena),
  _table( NEW_ARENA_ARRAY( _a , Node* , _max ) ), 
  _inserts(0), _insert_limit( insert_limit() ),
  _look_probes(0), _lookup_hits(0), _lookup_misses(0), 
  _delete_probes(0), _delete_hits(0), _delete_misses(0), 
  _total_insert_probes(0), _total_inserts(0), 
  _insert_probes(0), _grows(0) {
  // _sentinel must be in the current node space
  _sentinel = new (1) ProjNode(NULL, TypeFunc::Control);
  memset(_table,0,sizeof(Node*)*_max);
}

//------------------------------NodeHash---------------------------------------
NodeHash::NodeHash(NodeHash *nh) {
  debug_only(_table = (Node**)badAddress);   // interact correctly w/ operator=
  // just copy in all the fields
  *this = *nh;
  // nh->_sentinel must be in the current node space
}

//------------------------------hash_find--------------------------------------
// Find in hash table
Node *NodeHash::hash_find( const Node *n ) {
  // ((Node*)n)->set_hash( n->hash() );
  uint hash = n->hash();
  if (hash == Node::NO_HASH) {
    debug_only( _lookup_misses++ );
    return NULL;
  }
  uint key = hash & (_max-1); 
  uint stride = key | 0x01;
  debug_only( _look_probes++ );
  Node *k = _table[key];        // Get hashed value
  if( !k ) {                    // ?Miss?
    debug_only( _lookup_misses++ );
    return NULL;                // Miss!
  }
  
  int op = n->Opcode();
  uint req = n->req();
  while( 1 ) {                  // While probing hash table
    if( k->req() == req &&      // Same count of inputs
        k->Opcode() == op ) {   // Same Opcode
      for( uint i=0; i<req; i++ )
        if( n->in(i)!=k->in(i)) // Different inputs?
          goto collision;       // "goto" is a speed hack...
      if( n->cmp(*k) ) {        // Check for any special bits
        debug_only( _lookup_hits++ );
        return k;               // Hit!
      }
    }
  collision:
    debug_only( _look_probes++ );
    key = (key + stride/*7*/) & (_max-1); // Stride through table with relative prime
    k = _table[key];            // Get hashed value
    if( !k ) {                  // ?Miss?
      debug_only( _lookup_misses++ );
      return NULL;              // Miss!
    }
  }
  ShouldNotReachHere();
  return NULL;
}

//------------------------------hash_find_insert-------------------------------
// Find in hash table, insert if not already present
// Used to preserve unique entries in hash table
Node *NodeHash::hash_find_insert( Node *n ) {
  // n->set_hash( );
  uint hash = n->hash();
  if (hash == Node::NO_HASH) {
    debug_only( _lookup_misses++ );
    return NULL;
  }
  uint key = hash & (_max-1); 
  uint stride = key | 0x01;     // stride must be relatively prime to table siz
  uint first_sentinel = 0;      // replace a sentinel if seen.
  debug_only( _look_probes++ );
  Node *k = _table[key];        // Get hashed value
  if( !k ) {                    // ?Miss?
    debug_only( _lookup_misses++ );
    _table[key] = n;            // Insert into table!
    debug_only(n->enter_hash_lock()); // Lock down the node while in the table.
    check_grow();               // Grow table if insert hit limit
    return NULL;                // Miss!
  }
  else if( k == _sentinel ) {
    first_sentinel = key;      // Can insert here
  }
  
  int op = n->Opcode();
  uint req = n->req();
  while( 1 ) {                  // While probing hash table
    if( k->req() == req &&      // Same count of inputs
        k->Opcode() == op ) {   // Same Opcode
      for( uint i=0; i<req; i++ )
        if( n->in(i)!=k->in(i)) // Different inputs?
          goto collision;       // "goto" is a speed hack...
      if( n->cmp(*k) ) {        // Check for any special bits
        debug_only( _lookup_hits++ );
        return k;               // Hit!
      }
    }
  collision:
    debug_only( _look_probes++ );
    key = (key + stride) & (_max-1); // Stride through table w/ relative prime
    k = _table[key];            // Get hashed value
    if( !k ) {                  // ?Miss?
      debug_only( _lookup_misses++ );
      key = (first_sentinel == 0) ? key : first_sentinel; // ?saw sentinel?
      _table[key] = n;          // Insert into table!
      debug_only(n->enter_hash_lock()); // Lock down the node while in the table.
      check_grow();             // Grow table if insert hit limit
      return NULL;              // Miss!
    }
    else if( first_sentinel == 0 && k == _sentinel ) {
      first_sentinel = key;    // Can insert here
    }
      
  }
  ShouldNotReachHere();
  return NULL;
}

//------------------------------hash_insert------------------------------------
// Insert into hash table
void NodeHash::hash_insert( Node *n ) {
  // // "conflict" comments -- print nodes that conflict
  // bool conflict = false;
  // n->set_hash();
  uint hash = n->hash();
  if (hash == Node::NO_HASH) {
    return;
  }
  check_grow();
  uint key = hash & (_max-1); 
  uint stride = key | 0x01;
  
  while( 1 ) {                  // While probing hash table
    debug_only( _insert_probes++ );
    Node *k = _table[key];      // Get hashed value
    if( !k || (k == _sentinel) ) break;       // Found a slot
    assert( k != n, "already inserted" );
    // if( PrintCompilation && PrintOptoStatistics && Verbose ) { tty->print("  conflict: "); k->dump(); conflict = true; }
    key = (key + stride) & (_max-1); // Stride through table w/ relative prime
  }
  _table[key] = n;              // Insert into table!
  debug_only(n->enter_hash_lock()); // Lock down the node while in the table.
  // if( conflict ) { n->dump(); }
}

//------------------------------hash_delete------------------------------------
// Replace in hash table with sentinal
bool NodeHash::hash_delete( const Node *n ) {
  Node *k;
  uint hash = n->hash();
  if (hash == Node::NO_HASH) {
    debug_only( _delete_misses++ );
    return false;
  }
  uint key = hash & (_max-1); 
  uint stride = key | 0x01;
  debug_only( uint counter = 0; );
  for( ; /* (k != NULL) && (k != _sentinal) */; ) {
    debug_only( counter++ );
    debug_only( _delete_probes++ );
    k = _table[key];            // Get hashed value
    if( !k ) {                  // Miss?
      debug_only( _delete_misses++ );
#ifdef ASSERT
      if( VerifyOpto ) {
        for( uint i=0; i < _max; i++ )
          assert( _table[i] != n, "changed edges with rehashing" );
      }
#endif
      return false;             // Miss! Not in chain
    }
    else if( n == k ) {
      debug_only( _delete_hits++ );
      _table[key] = _sentinel;  // Hit! Label as deleted entry
      debug_only(((Node*)n)->exit_hash_lock()); // Unlock the node upon removal from table.
      return true;
    }
    else {
      // collision: move through table with prime offset
      key = (key + stride/*7*/) & (_max-1);
      assert( counter <= _insert_limit, "Cycle in hash-table");
    }
  }
  ShouldNotReachHere();
  return false;
}

//------------------------------round_up---------------------------------------
// Round up to nearest power of 2
uint NodeHash::round_up( uint x ) {
  x += (x>>2);                  // Add 25% slop
  if( x <16 ) return 16;        // Small stuff
  uint i=16;
  while( i < x ) i <<= 1;       // Double to fit
  return i;                     // Return hash table size
}

//------------------------------grow-------------------------------------------
// Grow _table to next power of 2 and insert old entries
void  NodeHash::grow() {
  // Record old state
  uint   old_max   = _max;
  Node **old_table = _table;
  // Construct new table with twice the space
  _grows++;
  _total_inserts       += _inserts;
  _total_insert_probes += _insert_probes;
  _inserts         = 0;
  _insert_probes   = 0;
  _max     = _max << 1;
  _table   = NEW_ARENA_ARRAY( _a , Node* , _max ); // (Node**)_a->Amalloc( _max * sizeof(Node*) );
  memset(_table,0,sizeof(Node*)*_max);
  _insert_limit = insert_limit();
  // Insert old entries into the new table
  for( uint i = 0; i < old_max; i++ ) {
    Node *m = *old_table++;
    if( !m || m == _sentinel ) continue;
    debug_only(m->exit_hash_lock()); // Unlock the node upon removal from old table.
    hash_insert(m);
  }
}

//------------------------------clear------------------------------------------
// Clear all entries in _table to NULL but keep storage
void  NodeHash::clear() {
#ifdef ASSERT
  // Unlock all nodes upon removal from table.
  for (uint i = 0; i < _max; i++) {
    Node* n = _table[i];
    if (!n || n == _sentinel)  continue;
    n->exit_hash_lock();
  }
#endif

  memset( _table, NULL, _max * sizeof(Node*) );
}

//-----------------------remove_useless_nodes----------------------------------
// Remove useless nodes from value table,
// implementation does not depend on hash function
void NodeHash::remove_useless_nodes(VectorSet &useful) {

  // Dead nodes in the hash table inherited from GVN should not replace
  // existing nodes, remove dead nodes.
  uint max = size();
  Node *sentinel_node = sentinel();
  for( uint i = 0; i < max; ++i ) {
    Node *n = at(i);
    if(n != NULL && n != sentinel_node && !useful.test(n->_idx)) {
      debug_only(n->exit_hash_lock()); // Unlock the node when removed
      _table[i] = sentinel_node;       // Replace with placeholder
    }
  }
}

#ifndef PRODUCT  
//------------------------------dump-------------------------------------------
// Dump statistics for the hash table
void NodeHash::dump() {
  _total_inserts       += _inserts;
  _total_insert_probes += _insert_probes;
  if( PrintCompilation && PrintOptoStatistics && Verbose && (_inserts > 0) ) { // PrintOptoGVN
    if( PrintCompilation2 ) {
      for( uint i=0; i<_max; i++ )
      if( _table[i] ) 
        tty->print("%d/%d/%d ",i,_table[i]->hash()&(_max-1),_table[i]->_idx);
    }
    tty->print("\nGVN Hash stats:  %d grows to %d max_size\n", _grows, _max);
    tty->print("  %d/%d (%8.1f%% full)\n", _inserts, _max, (double)_inserts/_max*100.0);
    tty->print("  %dp/(%dh+%dm) (%8.2f probes/lookup)\n", _look_probes, _lookup_hits, _lookup_misses, (double)_look_probes/(_lookup_hits+_lookup_misses));
    tty->print("  %dp/%di (%8.2f probes/insert)\n", _total_insert_probes, _total_inserts, (double)_total_insert_probes/_total_inserts);
    // sentinels increase lookup cost, but not insert cost
    assert((_lookup_misses+_lookup_hits)*4+100 >= _look_probes, "bad hash function");
    assert( _inserts+(_inserts>>3) < _max, "table too full" );
    assert( _inserts*3+100 >= _insert_probes, "bad hash function" );
  }
}

Node *NodeHash::find_index(uint idx) { // For debugging
  // Find an entry by its index value
  for( uint i = 0; i < _max; i++ ) {
    Node *m = _table[i];
    if( !m || m == _sentinel ) continue;
    if( m->_idx == (uint)idx ) return m;
  }
  return NULL;
}
#endif

#ifdef ASSERT
NodeHash::~NodeHash() {
  // Unlock all nodes upon destruction of table.
  if (_table != (Node**)badAddress)  clear();
}

void NodeHash::operator=(const NodeHash& nh) {
  // Unlock all nodes upon replacement of table.
  if (&nh == this)  return;
  if (_table != (Node**)badAddress)  clear();
  memcpy(this, &nh, sizeof(*this));
  // Do not increment hash_lock counts again.
  // Instead, be sure we never again use the source table.
  ((NodeHash*)&nh)->_table = (Node**)badAddress;
}


#endif


//=============================================================================
//------------------------------PhaseRemoveUseless-----------------------------
// 1) Use a breadthfirst walk to collect useful nodes reachable from root.
PhaseRemoveUseless::PhaseRemoveUseless( PhaseGVN *gvn, Unique_Node_List *worklist ) : Phase(Remove_Useless),
  _useful(Thread::current()->resource_area()) {

  // Implementation requires 'UseLoopSafepoints == true' and an edge from root
  // to each SafePointNode at a backward branch.  Inserted in add_safepoint().
  if( !UseLoopSafepoints || !OptoRemoveUseless ) return;

  // Identify nodes that are reachable from below, useful.
  C->identify_useful_nodes(_useful);

  // Remove all useless nodes from PhaseValues' recorded types
  // Must be done before disconnecting nodes to preserve hash-table-invariant
  gvn->remove_useless_nodes(_useful.member_set());

  // Remove all useless nodes from future worklist
  worklist->remove_useless_nodes(_useful.member_set());

  // Disconnect 'useless' nodes that are adjacent to useful nodes
  C->remove_useless_nodes(_useful);

  // Remove edges from "root" to each SafePoint at a backward branch.
  // They were inserted during parsing (see add_safepoint()) to make infinite
  // loops without calls or exceptions visible to root, i.e., useful.
  Node *root = C->root();
  if( root != NULL ) {
    for( uint i = root->req(); i < root->len(); ++i ) {
      Node *n = root->in(i);
      if( n != NULL && n->is_SafePoint() ) {
        root->rm_prec(i);
        --i;
      }
    }
  }
}


//=============================================================================
//------------------------------PhaseTransform---------------------------------
PhaseTransform::PhaseTransform( PhaseNumber pnum ) : Phase(pnum), 
  _arena(Thread::current()->resource_area()),
  _nodes(_arena),
  _types(_arena)
{
#ifndef PRODUCT
  clear_progress();
  clear_transforms();
  set_allow_progress(true);
#endif
  // Force allocation for currently existing nodes
  _types.map(C->unique(), NULL);
}

//------------------------------PhaseTransform---------------------------------
PhaseTransform::PhaseTransform( Arena *arena, PhaseNumber pnum ) : Phase(pnum), 
  _arena(arena),
  _nodes(arena),
  _types(arena)
{
#ifndef PRODUCT
  clear_progress();
  clear_transforms();
  set_allow_progress(true);
#endif
  // Force allocation for currently existing nodes
  _types.map(C->unique(), NULL);
}

//------------------------------PhaseTransform---------------------------------
// Initialize with previously generated type information
PhaseTransform::PhaseTransform( PhaseTransform *pt, PhaseNumber pnum ) : Phase(pnum), 
  _arena(pt->_arena),
  _nodes(pt->_nodes),
  _types(pt->_types)
{
#ifndef PRODUCT
  clear_progress();
  clear_transforms();
  set_allow_progress(true);
#endif
}

#ifndef PRODUCT
void PhaseTransform::dump_old2new_map() const {
  _nodes.dump();
}

void PhaseTransform::dump_new( uint nidx ) const {
  for( uint i=0; i<_nodes.Size(); i++ )
    if( _nodes[i] && _nodes[i]->_idx == nidx ) {
      _nodes[i]->dump();
      tty->cr();
      tty->print_cr("Old index= %d",i);
      return;
    }
  tty->print_cr("Node %d not found in the new indices", nidx);
}

//------------------------------dump_types-------------------------------------
void PhaseTransform::dump_types( ) const {
  _types.dump();
}

//------------------------------dump_nodes_and_types---------------------------
void PhaseTransform::dump_nodes_and_types(const Node *root, uint depth, bool only_ctrl) {
  VectorSet visited(Thread::current()->resource_area());
  dump_nodes_and_types_recur( root, depth, only_ctrl, visited );
}

//------------------------------dump_nodes_and_types_recur---------------------
void PhaseTransform::dump_nodes_and_types_recur( const Node *n, uint depth, bool only_ctrl, VectorSet &visited) {
  if( !n ) return;
  if( depth == 0 ) return;
  if( visited.test_set(n->_idx) ) return;
  for( uint i=0; i<n->len(); i++ ) {
    if( only_ctrl && !(n->is_Region()) && i != TypeFunc::Control ) continue;
    dump_nodes_and_types_recur( n->in(i), depth-1, only_ctrl, visited );
  }
  n->dump();
  if (type_or_null(n) != NULL) {
    tty->print("      "); type(n)->dump(); tty->cr();
  }
}

#endif


//=============================================================================
//------------------------------PhaseValues------------------------------------
// Set minimum table size to "255"
PhaseValues::PhaseValues( Arena *arena, uint est_max_size ) : PhaseTransform(arena, GVN), _table(arena, est_max_size) {
  memset(_icons,0,sizeof(_icons));
  memset(_zcons,0,sizeof(_zcons));
  NOT_PRODUCT( clear_new_values(); )
}

//------------------------------PhaseValues------------------------------------
// Set minimum table size to "255"
PhaseValues::PhaseValues( PhaseValues *ptv ) : PhaseTransform( ptv, GVN ),
  _table(&ptv->_table) {
  memset(_icons,0,sizeof(_icons));
  memset(_zcons,0,sizeof(_zcons));
  NOT_PRODUCT( clear_new_values(); )
}

//------------------------------PhaseValues------------------------------------
// Used by +VerifyOpto.  Clear out hash table but copy _types array.
PhaseValues::PhaseValues( PhaseValues *ptv, const char *dummy ) : PhaseTransform( ptv, GVN ),
  _table(ptv->arena(),ptv->_table.size()) {
  memset(_icons,0,sizeof(_icons));
  memset(_zcons,0,sizeof(_zcons));
  NOT_PRODUCT( clear_new_values(); )
}

//------------------------------~PhaseValues-----------------------------------
PhaseValues::~PhaseValues() {
#ifndef PRODUCT
  _table.dump();

  // Statistics for value progress and efficiency
  if( PrintCompilation && Verbose && WizardMode ) {
    tty->print("\n%sValues: %d nodes ---> %d/%d (%d)",
      is_IterGVN() ? "Iter" : "    ", C->unique(), made_progress(), made_transforms(), made_new_values());
    if( made_transforms() != 0 ) { 
      tty->print_cr("  ratio %f", made_progress()/(float)made_transforms() );
    } else {
      tty->cr();
    }
  }
#endif
}

//------------------------------makecon----------------------------------------
// Make an idealized constant - one of ConINode, ConPNode, etc.
ConNode *PhaseValues::makecon( const Type *t ) {
  assert( t->singleton(), "must be a constant" );
  switch (t->base()) {  // fast paths
  case Type::Half: 
  case Type::Top:  return (ConNode*) C->top();  // re. cast see PhaseIterGVN::makecon
  case Type::Int:  return PhaseValues::intcon( t->is_int()->get_con() );
  }
  ConNode *x = ConNode::make(t);
  ConNode *k = (ConNode*)hash_find_insert(x); // Value numbering
  if( !k ) {
    set_type(x, t);             // Missed, provide type mapping
  } else {
    x->destruct();              // Hit, destroy duplicate constant
    x = k;                      // use existing constant
  }
  return x;
}

//------------------------------intcon-----------------------------------------
// Fast integer constant.  Same as "transform(new ConINode(TypeInt::make(i)))"
ConINode *PhaseValues::intcon( int i ) {
  // Small integer?  Check cache! Check that cached node is not dead
  if( i >= _icon_min && i <= _icon_max ) {
    ConINode* icon = _icons[i-_icon_min];
    if ( icon != NULL && icon->in(TypeFunc::Control) != NULL )
      return icon;
  }
  const TypeInt *t = TypeInt::make(i);
  ConINode *n = new (1) ConINode(t);
  ConINode *k = (ConINode*)hash_find_insert(n); // Insert into value-table if new
  if( k == NULL ) {
    set_type(n, t);             // Missed, provide type mapping
  } else {
    n->destruct();              // Hit, destroy duplicate constant
    n = k;                      // use existing constant
  }
  if( i >= _icon_min && i <= _icon_max )
    _icons[i-_icon_min] = n;      // Cache small integers
  return n;
}

//------------------------------longcon----------------------------------------
// Fast long constant.  
ConLNode *PhaseValues::longcon( jlong l ) {
  return (ConLNode*)transform(new ConLNode(TypeLong::make(l)));
}

//------------------------------zerocon-----------------------------------------
// Fast zero or null constant. Same as "transform(ConNode::make(Type::get_zero_type(bt)))"
ConNode *PhaseValues::zerocon( BasicType bt ) {
  assert((uint)bt <= T_CONFLICT, "domain check");
  ConNode *n = _zcons[bt];
  if ( n != NULL && n->in(TypeFunc::Control) != NULL )  return n;  // cache hit
  const Type* t = Type::get_zero_type(bt);
  n = ConNode::make(t);
  ConNode *k = (ConINode*)hash_find_insert(n); // Insert into value-table if new
  if( k == NULL ) {
    set_type(n, t);             // Missed, provide type mapping
  } else {
    n->destruct();              // Hit, destroy duplicate constant
    n = k;                      // use existing constant
  }
  _zcons[bt] = n;
  return n;
}



//=============================================================================
//------------------------------transform--------------------------------------
// Return a node which computes the same function as this node, but in a
// faster or cheaper fashion.  The Node passed in here must have no other 
// pointers to it, as its storage will be reclaimed if the Node can be
// optimized away.
Node *PhaseGVN::transform( Node *n ) {
  NOT_PRODUCT( set_transforms(); )
  
  // Apply the Ideal call in a loop until it no longer applies
  Node *k = n;
  NOT_PRODUCT( int loop_count = 0; )
  while( 1 ) {
    Node *i = k->Ideal(this, /*can_reshape=*/false);
    if( !i ) break;
    assert( i->_idx >= k->_idx, "Idealize should return new nodes, use Identity to return old nodes" );
    // Can never reclaim storage for Ideal calls, because the Ideal call
    // returns a new Node, bumping the High Water Mark and our old Node
    // is caught behind the new one.
    //if( k != i ) {
    //k->destruct();            // Reclaim storage for recent node
    k = i;
    //}
    assert(loop_count++ < K, "infinite loop in PhaseGVN::transform");
  }
  NOT_PRODUCT( if( loop_count != 0 ) { set_progress(); } )

  // If brand new node, make space in type array.
  ensure_type_or_null(k);

  // Cache result of Value call since it can be expensive
  // (abstract interpretation of node 'k' using phase->_types[ inputs ])
  const Type *t = k->Value(this); // Get runtime Value set
  assert(t != NULL, "value sanity");
  if (type_or_null(k) != t) {
#ifndef PRODUCT
    // Do not record transformation or value construction on first visit
    if (type_or_null(k) == NULL) {
      inc_new_values();
      set_progress();
    }
#endif
    set_type(k, t);
    // If k is a TypeNode, capture any more-precise type permanently into Node
    k->raise_bottom_type(t);
  }

  if( t->singleton() && !k->is_Con() ) {
    //k->destruct();              // Reclaim storage for recent node
    NOT_PRODUCT( set_progress(); )
    return makecon(t);          // Turn into a constant
  }

  // Now check for Identities
  Node *i = k->Identity(this);  // Look for a nearby replacement
  if( i != k ) {                // Found? Return replacement!
    //k->destruct();              // Reclaim storage for recent node
    NOT_PRODUCT( set_progress(); )
    return i;
  }

  // Try Global Value Numbering
  i = hash_find_insert(k);      // Found older value when i != NULL
  if( i && i != k ) {           // Hit? Return the old guy
    NOT_PRODUCT( set_progress(); )
    return i;
  }

  // Return Idealized original
  return k;
}

//------------------------------transform--------------------------------------
// Return a node which computes the same function as this node, but
// in a faster or cheaper fashion.  
Node *PhaseGVN::transform_no_reclaim( Node *n ) {
  NOT_PRODUCT( set_transforms(); )

  // Apply the Ideal call in a loop until it no longer applies
  Node *k = n;
  NOT_PRODUCT( int loop_count = 0; )
  while( 1 ) {
    Node *i = k->Ideal(this, /*can_reshape=*/false);
    if( !i ) break;
    assert( i->_idx >= k->_idx, "Idealize should return new nodes, use Identity to return old nodes" );
    k = i;
    assert(loop_count++ < K, "infinite loop in PhaseGVN::transform");
  }
  NOT_PRODUCT( if( loop_count != 0 ) { set_progress(); } )


  // If brand new node, make space in type array.
  ensure_type_or_null(k);

  // Since I just called 'Value' to compute the set of run-time values
  // for this Node, and 'Value' is non-local (and therefore expensive) I'll
  // cache Value.  Later requests for the local phase->type of this Node can
  // use the cached Value instead of suffering with 'bottom_type'.
  const Type *t = k->Value(this); // Get runtime Value set
  assert(t != NULL, "value sanity");
  if (type_or_null(k) != t) {
#ifndef PRODUCT
    // Do not count initial visit to node as a transformation
    if (type_or_null(k) == NULL) {
      inc_new_values();
      set_progress();
    }
#endif
    set_type(k, t);
    // If k is a TypeNode, capture any more-precise type permanently into Node
    k->raise_bottom_type(t);
  }

  if( t->singleton() && !k->is_Con() ) {
    NOT_PRODUCT( set_progress(); )
    return makecon(t);          // Turn into a constant
  }

  // Now check for Identities
  Node *i = k->Identity(this);  // Look for a nearby replacement
  if( i != k ) {                // Found? Return replacement!
    NOT_PRODUCT( set_progress(); )
    return i;
  }

  // Global Value Numbering
  i = hash_find_insert(k);      // Insert if new
  if( i && (i != k) ) {
    // Return the pre-existing node
    NOT_PRODUCT( set_progress(); )
    return i;
  }

  // Return Idealized original
  return k;
}


//=============================================================================
//------------------------------PhaseIterGVN-----------------------------------
// Initialize hash table to fresh and clean for +VerifyOpto
PhaseIterGVN::PhaseIterGVN( PhaseIterGVN *igvn, const char *dummy ) : PhaseGVN(igvn,dummy), _worklist( ) {
}

//------------------------------PhaseIterGVN-----------------------------------
// Initialize with previous PhaseIterGVN info; used by PhaseCCP
PhaseIterGVN::PhaseIterGVN( PhaseIterGVN *igvn ) : PhaseGVN(igvn), 
  _worklist( igvn->_worklist )
{
}

//------------------------------PhaseIterGVN-----------------------------------
// Initialize with previous PhaseGVN info from Parser
PhaseIterGVN::PhaseIterGVN( PhaseGVN *gvn ) : PhaseGVN(gvn), 
  _worklist(*C->for_igvn())
{
  uint max;

  // Dead nodes in the hash table inherited from GVN were not treated as
  // roots during def-use info creation; hence they represent an invisible
  // use.  Clear them out.
  max = _table.size();
  for( uint i = 0; i < max; ++i ) {
    Node *n = _table.at(i);
    if(n != NULL && n != _table.sentinel() && n->outcnt() == 0) {
      if( n->is_top() ) continue;
      assert( false, "Parse::remove_useless_nodes missed this node");
      hash_delete(n);
    }
  }

  // Any Phis or Regions on the worklist probably had uses that could not
  // make more progress because the uses were made while the Phis and Regions 
  // were in half-built states.  Put all uses of Phis and Regions on worklist.
  max = _worklist.size();
  for( uint j = 0; j < max; j++ ) {
    Node *n = _worklist.at(j);
    uint uop = n->Opcode();
    if( uop == Op_Phi || uop == Op_Region ||
        n->is_Type() ||
        n->is_Mem() )
      add_users_to_worklist(n);
  }
}


#ifndef PRODUCT
void PhaseIterGVN::verify_step(Node* n) {
  _verify_window[_verify_counter % _verify_window_size] = n;
  ++_verify_counter;
  VectorSet old_space(Thread::current()->resource_area()), new_space(Thread::current()->resource_area());
  if (C->unique() < 1000 ||
      0 == _verify_counter % (C->unique() < 10000 ? 10 : 100)) {
    ++_verify_full_passes;
    Node::verify_recur(C->root(), -1, old_space, new_space);
  }
  const int verify_depth = 4;
  for ( int i = 0; i < _verify_window_size; i++ ) {
    Node* n = _verify_window[i];
    if ( n == NULL )  continue;
    if( n->in(0) == NodeSentinel ) {  // xform_idom
      _verify_window[i] = n->in(1);
      --i; continue;
    }
    // Typical fanout is 1-2, so this call visits about 6 nodes.
    Node::verify_recur(n, verify_depth, old_space, new_space);
  }
}
#endif


//------------------------------init_worklist----------------------------------
// Initialize worklist for each node.
void PhaseIterGVN::init_worklist( Node *n ) {
  if( _worklist.member(n) ) return;
  _worklist.push(n);
  uint cnt = n->req();
  for( uint i =0 ; i < cnt; i++ ) {
    Node *m = n->in(i);
    if( m ) init_worklist(m);
  }
}

//------------------------------optimize---------------------------------------
void PhaseIterGVN::optimize() {
  debug_only(uint num_processed  = 0;);
#ifndef PRODUCT
  {
    _verify_counter = 0;
    _verify_full_passes = 0;
    for ( int i = 0; i < _verify_window_size; i++ ) {
      _verify_window[i] = NULL;
    }
  }
#endif

  // Pull from worklist; transform node; 
  // If node has changed: update edge info and put uses on worklist.
  while( _worklist.size() ) {
    Node *n  = _worklist.pop();
    if( TraceIterativeGVN ) {
      tty->print("  Pop ");
      NOT_PRODUCT( n->dump(); )
      debug_only(if( (num_processed++ % 100) == 0 ) _worklist.print_set();)
    }

    if (n->outcnt() != 0) {

      Node *nn = transform_old(n);

      if( VerifyIterativeGVN && nn != n ) {
#ifndef PRODUCT
        verify_step((Node*) NULL);  // ignore n, it might be subsumed
#endif
      }
    } else if (!n->is_top()) {
      remove_dead_node(n);
    }
  }

#ifndef PRODUCT
  C->verify_graph_edges();
  if( VerifyOpto && allow_progress() ) {
    // Must turn off allow_progress to enable assert and break recursion
    C->root()->verify();
    { // Check if any progress was missed using IterGVN
      // Def-Use info enables transformations not attempted in wash-pass
      // e.g. Region/Phi cleanup, ...
      // Null-check elision -- may not have reached fixpoint
      //                       do not propagate to dominated nodes
      ResourceMark rm;
      PhaseIterGVN igvn2(this,"Verify"); // Fresh and clean!
      // Fill worklist completely
      igvn2.init_worklist(C->root());

      igvn2.set_allow_progress(false);
      igvn2.optimize();
      igvn2.set_allow_progress(true);
    } 
  }
  if ( VerifyIterativeGVN && PrintOpto ) {
    if ( _verify_counter == _verify_full_passes )
      tty->print_cr("VerifyIterativeGVN: %d transforms and verify passes",
                    _verify_full_passes);
    else
      tty->print_cr("VerifyIterativeGVN: %d transforms, %d full verify passes",
                  _verify_counter, _verify_full_passes);
  }
#endif
}


//------------------register_new_node_with_optimizer---------------------------
// Register a new node with the optimizer.  Update the types array, the def-use
// info.  Put on worklist.
Node *PhaseIterGVN::register_new_node_with_optimizer( Node *n ) {
  set_type_bottom(n);
  _worklist.push(n);
  return n;
}

//------------------------------transform--------------------------------------
// Non-recursive: idealize Node 'n' with respect to its inputs and its value
Node *PhaseIterGVN::transform( Node *n ) {
  // If brand new node, make space in type array, and give it a type.
  ensure_type_or_null(n);
  if (type_or_null(n) == NULL) {
    set_type_bottom(n);
  }

  return transform_old(n);
}

//------------------------------transform_old----------------------------------
Node *PhaseIterGVN::transform_old( Node *n ) {
#ifndef PRODUCT
  debug_only(int loop_count = 0;);
  set_transforms();
#endif
  // Remove 'n' from hash table in case it gets modified
  _table.hash_delete(n);
  if( VerifyIterativeGVN ) {
   assert( !_table.find_index(n->_idx), "found duplicate entry in table");
  }

  // Apply the Ideal call in a loop until it no longer applies
  Node *k = n;
  Node *i = k->Ideal(this, /*can_reshape=*/true);
#ifndef PRODUCT
  if( VerifyIterativeGVN )
    verify_step(k);
  if( i && VerifyOpto ) {
    if( !allow_progress() ) {
      if (i->is_Add() && i->outcnt() == 1) {
        // Switched input to left side because this is the only use
      } else if( i->is_If() && (i->in(0) == NULL) ) {
        // This IF is dead because it is dominated by an equivalent IF When
        // dominating if changed, info is not propagated sparsely to 'this'
        // Propagating this info further will spuriously identify other
        // progress.
        return i;
      } else
        set_progress();
    } else
      set_progress();
  }
#endif

  while( i ) {
#ifndef PRODUCT
    debug_only( if( loop_count >= K ) i->dump(4); )
    assert(loop_count < K, "infinite loop in PhaseGVN::transform");
    debug_only( loop_count++; )
#endif
    assert((i->_idx >= k->_idx) || i->is_top(), "Idealize should return new nodes, use Identity to return old nodes");
    // Made a change; put users of original Node on worklist
    add_users_to_worklist( k );
    // Replacing root of transform tree?  
    if( k != i ) {
      // Make users of old Node now use new.
      subsume_node( k, i );
      k = i;
    }
    // Try idealizing again
    i = k->Ideal(this, /*can_reshape=*/true);
#ifndef PRODUCT
    if( VerifyIterativeGVN )
      verify_step(k);
    if( i && VerifyOpto ) set_progress();
#endif
  }

  // If brand new node, make space in type array.
  ensure_type_or_null(k);

  // See what kind of values 'k' takes on at runtime
  const Type *t = k->Value(this);
  assert(t != NULL, "value sanity");
  
  // Since I just called 'Value' to compute the set of run-time values
  // for this Node, and 'Value' is non-local (and therefore expensive) I'll
  // cache Value.  Later requests for the local phase->type of this Node can
  // use the cached Value instead of suffering with 'bottom_type'.
  if (t != type_or_null(k)) {
    NOT_PRODUCT( set_progress(); )
    NOT_PRODUCT( inc_new_values();)
    set_type(k, t);
    // If k is a TypeNode, capture any more-precise type permanently into Node
    k->raise_bottom_type(t);
    // Move users of node to worklist
    add_users_to_worklist( k );
  }

  // If 'k' computes a constant, replace it with a constant
  if( t->singleton() && !k->is_Con() ) {
    NOT_PRODUCT( set_progress(); )
    Node *con = makecon(t);     // Make a constant
    add_users_to_worklist( k );
    subsume_node( k, con );     // Everybody using k now uses con
    return con;
  } 

  // Now check for Identities
  i = k->Identity(this);        // Look for a nearby replacement
  if( i != k ) {                // Found? Return replacement!
    NOT_PRODUCT( set_progress(); )
    add_users_to_worklist( k );
    subsume_node( k, i );       // Everybody using k now uses i
    return i;
  }

  // Global Value Numbering
  i = hash_find_insert(k);      // Check for pre-existing node
  if( i && (i != k) ) {
    // Return the pre-existing node if it isn't dead
    NOT_PRODUCT( set_progress(); )
    add_users_to_worklist( k );
    subsume_node( k, i );       // Everybody using k now uses i
    return i;
  }

  // Return Idealized original
  return k;
}

//------------------------------remove_globally_dead_node----------------------
// Kill a globally dead Node.  All uses are also globally dead and are
// aggressively trimmed.
void PhaseIterGVN::remove_globally_dead_node( Node *dead ) {
  assert(dead != C->root(), "killing root, eh?");
  if (dead->is_top())  return;
  NOT_PRODUCT( set_progress(); )
  // Remove from iterative worklist
  _worklist.remove(dead);
  // Remove from hash table
  _table.hash_delete( dead );
  // Smash all inputs to 'dead', isolating him completely
  for( uint i = 0; i < dead->req(); i++ ) {
    Node *in = dead->in(i);
    if( in ) {                 // Points to something?
      dead->set_req(i,NULL);  // Kill the edge
      if (in->outcnt() == 0 && in != C->top()) {// Made input go dead?
        remove_dead_node(in); // Recursively remove
      } else if (in->outcnt() <= 2 && dead->is_Phi()) {
        if( in->Opcode() == Op_Region )
          _worklist.push(in);
        else if( in->is_Store() ) {
          DUIterator_Fast imax, i = in->fast_outs(imax);
          _worklist.push(in->fast_out(i));
          i++;
          if(in->outcnt() == 2) {
            _worklist.push(in->fast_out(i));
            i++;
          }
          assert(!(i < imax), "sanity");
        }
      }
    }
  }

  // Aggressively kill globally dead uses
  // (Cannot use DUIterator_Last because of the indefinite number
  // of edge deletions per loop trip.)
  while (dead->outcnt() > 0) {
    remove_globally_dead_node(dead->raw_out(0));
  }
}

//------------------------------subsume_node-----------------------------------
// Remove users from node 'old' and add them to node 'nn'.
void PhaseIterGVN::subsume_node( Node *old, Node *nn ) {
  assert( old != hash_find(old), "should already been removed" );
  assert( old != C->top(), "cannot subsume top node");
  // Move users of node 'old' to node 'nn'
  for (DUIterator_Last imin, i = old->last_outs(imin); i >= imin; ) {
    Node* use = old->last_out(i);  // for each use...
    // use might need re-hashing (but it won't if it's a new node)
    bool is_in_table = _table.hash_delete( use );
    // Update use-def info as well
    // We remove all occurrences of old within use->in,
    // so as to avoid rehashing any node more than once.
    // The hash table probe swamps any outer loop overhead.
    uint num_edges = 0;
    for (uint jmax = use->len(), j = 0; j < jmax; j++) {
      if (use->in(j) == old) {
        use->set_req(j, nn);
        ++num_edges;
      }
    }
    // Insert into GVN hash table if unique
    // If a duplicate, 'use' will be cleaned up when pulled off worklist
    if( is_in_table ) {
      hash_find_insert(use);
    }
    i -= num_edges;    // we deleted 1 or more copies of this edge
  }

  // Smash all inputs to 'old', isolating him completely
  Node *temp = new (1) Node(1);
  temp->set_req(0,nn);        // Add a use to nn to prevent him from dying
  remove_dead_node( old );
  temp->del_req(0);         // Yank bogus edge
#ifndef PRODUCT
  if( VerifyIterativeGVN ) {
    for ( int i = 0; i < _verify_window_size; i++ ) {
      if ( _verify_window[i] == old )
        _verify_window[i] = nn;
    }
  }
#endif
  _worklist.remove(temp);   // this can be necessary
  temp->destruct();         // reuse the _idx of this little guy
}

//------------------------------add_users_to_worklist--------------------------
void PhaseIterGVN::add_users_to_worklist0( Node *n ) {
  for (DUIterator_Fast imax, i = n->fast_outs(imax); i < imax; i++) {
    _worklist.push(n->fast_out(i));  // Push on worklist
  }
}

void PhaseIterGVN::add_users_to_worklist( Node *n ) {
  add_users_to_worklist0(n);

  // Move users of node to worklist
  for (DUIterator_Fast imax, i = n->fast_outs(imax); i < imax; i++) {
    Node* use = n->fast_out(i); // Get use

    if( use->is_Multi() ||      // Multi-definer?  Push projs on worklist
        use->is_Store() )       // Enable store/load same address
      add_users_to_worklist0(use); 

    if( use->is_Cmp() ) {       // Enable CMP/BOOL optimization
      add_users_to_worklist(use); // Put Bool on worklist
      // Look for the 'is_x2logic' pattern: "x ? : 0 : 1" and put the
      // phi merging either 0 or 1 onto the worklist
      if (use->outcnt() > 0) {
        Node* bol = use->raw_out(0);
        if (bol->outcnt() > 0) {
          Node* iff = bol->raw_out(0);
          if (iff->outcnt() == 2) {
            Node* ifproj0 = iff->raw_out(0);
            Node* ifproj1 = iff->raw_out(1);
            if (ifproj0->outcnt() > 0 && ifproj1->outcnt() > 0) {
              Node* region0 = ifproj0->raw_out(0);
              Node* region1 = ifproj1->raw_out(0);
              if( region0 == region1 )
                add_users_to_worklist0(region0);
            }
          }
        }
      }
    }
  
    uint use_op = use->Opcode();
    // If changed Cast input, check Phi users for simple cycles
    if( use_op == Op_CastPP ) {
      for (DUIterator_Fast i2max, i2 = use->fast_outs(i2max); i2 < i2max; i2++) {
        Node* u = use->fast_out(i2);
        if (u->is_Phi())
          _worklist.push(u);
      }
    }
    // If changed LShift inputs, check RShift users for useless sign-ext
    if( use_op == Op_LShiftI ) {
      for (DUIterator_Fast i2max, i2 = use->fast_outs(i2max); i2 < i2max; i2++) {
        Node* u = use->fast_out(i2);
        if (u->Opcode() == Op_RShiftI)
          _worklist.push(u);
      }
    }
    // If changed AddP inputs, check Stores for loop invariant
    if( use_op == Op_AddP ) {
      for (DUIterator_Fast i2max, i2 = use->fast_outs(i2max); i2 < i2max; i2++) {
        Node* u = use->fast_out(i2);
        if (u->is_Mem())
          _worklist.push(u);
      }
    }
  }
}

//------------------------------makecon----------------------------------------
// Make an idealized constant, i.e., one of ConINode, ConPNode, ConFNode, etc
ConNode *PhaseIterGVN::makecon( const Type *t ) {
  // The simple cast here lets us avoid making compile.hpp depend on connode.hpp.
  // Usually asserts accompany casts, but that would be overkill in this case.
  if (t == Type::TOP)  return (ConNode*) C->top();
  return PhaseValues::makecon(t);
}

//=============================================================================
#ifndef PRODUCT
uint PhaseCCP::_total_invokes   = 0;
uint PhaseCCP::_total_constants = 0;
#endif
//------------------------------PhaseCCP---------------------------------------
// Conditional Constant Propagation, ala Wegman & Zadeck
PhaseCCP::PhaseCCP( PhaseIterGVN *igvn ) : PhaseIterGVN(igvn) {
  NOT_PRODUCT( clear_constants(); )
  assert( _worklist.size() == 0, "" );
  // Clear out _nodes from IterGVN.  Must be clear to transform call.
  _nodes.clear();               // Clear out from IterGVN
  analyze();
}

//------------------------------~PhaseCCP--------------------------------------
PhaseCCP::~PhaseCCP() {
#ifndef PRODUCT
  inc_invokes();
  _total_constants += count_constants();
#endif
}


//------------------------------analyze----------------------------------------
void PhaseCCP::analyze() {
  // Initialize all types to TOP, optimistic analysis
  for (int i = C->unique() - 1; i >= 0; i--)  {
    _types.map(i,Type::TOP);
  }

  // Push root onto worklist
  Unique_Node_List worklist;
  worklist.push(C->root());

  // Pull from worklist; compute new value; push changes out.
  // This loop is the meat of CCP.
  while( worklist.size() ) {
    Node *n = worklist.pop();
    const Type *t = n->Value(this);
    if (t != type(n)) {
      assert( t->meet(type(n)) == t, "Not monotonic" );
      if( TracePhaseCCP ) {
#ifndef PRODUCT
        t->dump();
        tty->print(" ");
        n->dump();
#endif
      }
      set_type(n, t);
      for (DUIterator_Fast imax, i = n->fast_outs(imax); i < imax; i++) {
        Node* m = n->fast_out(i);   // Get user
        if( m->is_Region() ) {  // New path to Region?  Must recheck Phis too
          for (DUIterator_Fast i2max, i2 = m->fast_outs(i2max); i2 < i2max; i2++) {
            Node* p = m->fast_out(i2); // Propagate changes to uses
            if( p->bottom_type() != type(p) ) // If not already bottomed out
              worklist.push(p); // Propagate change to user
          }
        }
        // If we changed the reciever type to a call, we need to revisit
        // the Catch following the call.  It's looking for a non-NULL
        // receiver to know when to enable the regular fall-through path
        // in addition to the NullPtrException path
        CallNode *call = m->is_Call();
        if (call != NULL) {
          for (DUIterator_Fast i2max, i2 = call->fast_outs(i2max); i2 < i2max; i2++) {
            ProjNode* p = call->fast_out(i2)->is_Proj();  // Propagate changes to uses
            if (p != NULL && p->_con == TypeFunc::Control && p->outcnt() == 1)
              worklist.push(p->unique_out());
          }
        }
        if( m->bottom_type() != type(m) ) // If not already bottomed out
          worklist.push(m);     // Propagate change to user
      }
    }
  }
}

//------------------------------do_transform-----------------------------------
// Top level driver for the recursive transformer
void PhaseCCP::do_transform() {
  // Correct leaves of new-space Nodes; they point to old-space.
  C->set_root( transform(C->root())->is_Root() );
  assert( C->top(),  "missing TOP node" );
  assert( C->root(), "missing root" );
}

//------------------------------transform--------------------------------------
// Given a Node in old-space, clone him into new-space.
// Convert any of his old-space children into new-space children.
Node *PhaseCCP::transform( Node *n ) {
  Node *new_node = _nodes[n->_idx]; // Check for transformed node
  if( new_node != NULL ) 
    return new_node;                // Been there, done that, return old answer
  new_node = transform_once(n);     // Check for constant
  _nodes.map( n->_idx, new_node );  // Flag as having been cloned

  // Allocate stack of size _nodes.Size()/2 to avoid frequent realloc
  GrowableArray <Node *> trstack(C->unique() >> 1);

  trstack.push(new_node);           // Process children of cloned node
  while ( trstack.is_nonempty() ) {
    Node *clone = trstack.pop();
    uint cnt = clone->req();
    for( uint i = 0; i < cnt; i++ ) {          // For all inputs do
      Node *input = clone->in(i);
      if( input != NULL ) {                    // Ignore NULLs
        Node *new_input = _nodes[input->_idx]; // Check for cloned input node
        if( new_input == NULL ) {
          new_input = transform_once(input);   // Check for constant
          _nodes.map( input->_idx, new_input );// Flag as having been cloned
          trstack.push(new_input);
        }
        assert( new_input == clone->in(i), "insanity check");
      }
    }
  }
  return new_node;
}


//------------------------------transform_once---------------------------------
// For PhaseCCP, transformation is IDENTITY unless Node computed a constant.
Node *PhaseCCP::transform_once( Node *n ) {
  const Type *t = type(n);
  // Constant?  Use constant Node instead
  if( t->singleton() ) {
    Node *nn = n;               // Default is to return the original constant
    if( t == Type::TOP ) {
      // cache my top node on the Compile instance
      if( C->cached_top_node() == NULL || C->cached_top_node()->in(0) == NULL ) {
        C->set_cached_top_node( ConNode::make(Type::TOP) );
        set_type(C->top(), Type::TOP);
      }
      nn = C->top();
    } 
    if( !n->is_Con() ) { 
      add_users_to_worklist(n); // Users of about-to-be-constant 'n'
      nn = makecon(t);          // ConNode::make(t);
      hash_delete(n);           // Removed 'n' from table before subsuming it
      subsume_node(n,nn);       // Update DefUse edges for new constant
      NOT_PRODUCT( inc_constants(); )
    }
    return nn;
  }

  // If x is a TypeNode, capture any more-precise type permanently into Node
  if (t != n->bottom_type()) {
    hash_delete(n);             // changing bottom type may force a rehash
    n->raise_bottom_type(t);
    _worklist.push(n);          // n re-enters the hash table via the worklist
  }

  // Idealize graph using DU info.  Must clone() into new-space.
  // DU info is generally used to show profitability, progress or safety
  // (but generally not needed for correctness).
  Node *nn = n->Ideal_DU_postCCP(this);

  // TEMPORARY fix to ensure that 2nd GVN pass eliminates NULL checks
  switch( n->Opcode() ) {
  case Op_If:
  case Op_CountedLoopEnd:
  case Op_Region:
  case Op_Loop:
  case Op_CountedLoop:
  case Op_Conv2B:
  case Op_Opaque1:
  case Op_Opaque2:
    _worklist.push(n);
    break;
  default:
    break;
  }
  if( nn ) {
    _worklist.push(n);
    // Put users of 'n' onto worklist for second igvn transform
    add_users_to_worklist(n);
    return nn;
  }

  return  n;
}


//------------------------------print_statistics-------------------------------
#ifndef PRODUCT
void PhaseCCP::print_statistics() {
  tty->print_cr("CCP: %d  constants found: %d", _total_invokes, _total_constants);
}
#endif


//=============================================================================
#ifndef PRODUCT
uint PhasePeephole::_total_peepholes = 0;
#endif
//------------------------------PhasePeephole----------------------------------
// Conditional Constant Propagation, ala Wegman & Zadeck
PhasePeephole::PhasePeephole( PhaseRegAlloc *regalloc, PhaseCFG &cfg )
  : PhaseTransform(Peephole), _regalloc(regalloc), _cfg(cfg) {
  NOT_PRODUCT( clear_peepholes(); )
}

//------------------------------~PhasePeephole---------------------------------
PhasePeephole::~PhasePeephole() {
  NOT_PRODUCT( _total_peepholes += count_peepholes(); )
}

//------------------------------transform--------------------------------------
Node *PhasePeephole::transform( Node *n ) {
  ShouldNotCallThis();
  return NULL;
}

//------------------------------do_transform-----------------------------------
void PhasePeephole::do_transform() {
  bool method_name_not_printed = true;

  // Examine each basic block
  for( uint block_number = 1; block_number < _cfg._num_blocks; ++block_number ) {
    Block *block = _cfg._blocks[block_number];
    bool block_not_printed = true;

    // and each instruction within a block
    uint end_index = block->_nodes.size();
    // block->end_idx() not valid after PhaseRegAlloc
    for( uint instruction_index = 1; instruction_index < end_index; ++instruction_index ) {
      Node     *n = block->_nodes.at(instruction_index);
      MachNode *m = n->is_Mach();
      if( m ) {
        int deleted_count = 0;
        // check for peephole opportunities
        MachNode *m2 = m->peephole( block, instruction_index, _regalloc, deleted_count );
        if( m2 != NULL ) {
#ifndef PRODUCT
          if( PrintOptoPeephole ) {
            // Print method, first time only
            if( C->method() && method_name_not_printed ) {
              C->method()->print_short_name(); tty->cr();
              method_name_not_printed = false;
            }
            // Print this block
            if( Verbose && block_not_printed) {
              tty->print_cr("in block");
              block->dump();
              block_not_printed = false;
            }
            // Print instructions being deleted
            for( int i = (deleted_count - 1); i >= 0; --i ) {
              block->_nodes.at(instruction_index-i)->is_Mach()->format(_regalloc); tty->cr();
            }
            tty->print_cr("replaced with");
            // Print new instruction
            m2->format(_regalloc);
            tty->print("\n\n");
          }
#endif
          // Remove old nodes from basic block and update instruction_index
          // (old nodes still exist and may have edges pointing to them
          //  as register allocation info is stored in the allocator using
          //  the node index to live range mappings.)
          uint safe_instruction_index = (instruction_index - deleted_count);
          for( ; (instruction_index > safe_instruction_index); --instruction_index ) {
            block->_nodes.remove( instruction_index );
          }
          // install new node after safe_instruction_index
          block->_nodes.insert( safe_instruction_index + 1, m2 );
          end_index = block->_nodes.size() - 1; // Recompute new block size
          NOT_PRODUCT( inc_peepholes(); )
        }
      }
    }
  }
}

//------------------------------print_statistics-------------------------------
#ifndef PRODUCT
void PhasePeephole::print_statistics() {
  tty->print_cr("Peephole: peephole rules applied: %d",  _total_peepholes);
}
#endif


//=============================================================================
//------------------------------set_req_X--------------------------------------
void Node::set_req_X( uint i, Node *n, PhaseIterGVN *igvn ) {
  assert( igvn->hash_find(this) != this, "Need to remove from hash before changing edges" );
  Node *old = in(i);
  set_req(i, n);

  // old goes dead?
  if( old ) {
    switch (old->outcnt()) {
    case 0:      // Kill all his inputs, and recursively kill other dead nodes.
      if (!old->is_top())
        igvn->remove_dead_node( old );
      break;
    case 1:
      if( old->is_Store() )
        igvn->add_users_to_worklist( old );
      break;
    case 2:
      if( old->is_Store() )
        igvn->add_users_to_worklist( old );
      if( old->Opcode() == Op_Region )
        igvn->_worklist.push(old);
      break;
    case 3:
      if( old->Opcode() == Op_Region ) {
        igvn->_worklist.push(old);
        igvn->add_users_to_worklist( old );
      }
      break;
    default:
      break;
    }
  }

}

//-------------------------------replace_by-----------------------------------
// Using def-use info, replace one node for another.  Follow the def-use info
// to all users of the OLD node.  Then make all uses point to the NEW node.
void Node::replace_by(Node *new_node) {
  assert(!is_top(), "top node has no DU info");
  for (DUIterator_Last imin, i = last_outs(imin); i >= imin; ) {
    Node* use = last_out(i);
    uint uses_found = 0;
    for (uint j = 0; j < use->len(); j++) {
      if (use->in(j) == this) {
        if (j < use->req())
              use->set_req(j, new_node);
        else  use->set_prec(j, new_node);
        uses_found++;
      }
    }
    i -= uses_found;    // we deleted 1 or more copies of this edge
  }
}

//=============================================================================
//-----------------------------------------------------------------------------
void Type_Array::grow( uint i ) {
  if( !_max ) { 
    _max = 1;
    _types = (const Type**)_a->Amalloc( _max * sizeof(Type*) );
    _types[0] = NULL;
  }
  uint old = _max;
  while( i >= _max ) _max <<= 1;        // Double to fit
  _types = (const Type**)_a->Arealloc( _types, old*sizeof(Type*),_max*sizeof(Type*));
  memset( &_types[old], 0, (_max-old)*sizeof(Type*) );
}

//------------------------------dump-------------------------------------------
#ifndef PRODUCT
void Type_Array::dump() const {
  uint max = Size();
  for( uint i = 0; i < max; i++ ) {
    if( _types[i] != NULL ) {
      tty->print("  %d\t== ", i); _types[i]->dump(); tty->cr();
    }
  }
}
#endif
