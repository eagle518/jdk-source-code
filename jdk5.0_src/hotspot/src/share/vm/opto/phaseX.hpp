#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)phaseX.hpp	1.106 03/12/23 16:42:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Compile;
class ConINode;
class ConLNode;
class Node;
class Type;
class PhaseTransform;
class   PhaseGVN;
class     PhaseIterGVN;
class       PhaseCCP;
class   PhasePeephole;
class   PhaseRegAlloc;


//-----------------------------------------------------------------------------
// Expandable closed hash-table of nodes, initialized to NULL.
// Note that the constructor just zeros things
// Storage is reclaimed when the Arena's lifetime is over.
class NodeHash : public StackObj {
protected:
  Arena *_a;                    // Arena to allocate in
  uint   _max;                  // Size of table (power of 2)
  uint   _inserts;              // For grow and debug, count of hash_inserts
  uint   _insert_limit;         // 'grow' when _inserts reaches _insert_limit
  Node **_table;                // Hash table of Node pointers
  Node  *_sentinel;             // Replaces deleted entries in hash table

public:
  NodeHash(uint est_max_size);
  NodeHash(Arena *arena, uint est_max_size);
  NodeHash(NodeHash *use_this_state);
#ifdef ASSERT
  ~NodeHash();                  // Unlock all nodes upon destruction of table.
  void operator=(const NodeHash&); // Unlock all nodes upon replacement of table.
#endif
  Node  *hash_find(const Node*);// Find an equivalent version in hash table
  Node  *hash_find_insert(Node*);// If not in table insert else return found node
  void   hash_insert(Node*);    // Insert into hash table
  bool   hash_delete(const Node*);// Replace with _sentinel in hash table
  void   check_grow() {
    _inserts++;
    if( _inserts == _insert_limit ) { grow(); }
    assert( _inserts <= _insert_limit, "hash table overflow");
    assert( _inserts < _max, "hash table overflow" );
  }
  static uint round_up(uint);   // Round up to nearest power of 2
  void   grow();                // Grow _table to next power of 2 and rehash
  // Return 75% of _max, rounded up.
  uint   insert_limit() const { return _max - (_max>>2); }

  void   clear();               // Set all entries to NULL, keep storage.
  // Size of hash table
  uint   size()         const { return _max; }
  // Return Node* at index in table
  Node  *at(uint table_index) {
    assert(table_index < _max, "Must be within table");
    return _table[table_index];
  }

  void   remove_useless_nodes(VectorSet &useful); // replace with sentinel

  Node  *sentinel() { return _sentinel; }

#ifndef PRODUCT  
  Node  *find_index(uint idx);  // For debugging
  void   dump();                // For debugging, dump statistics
#endif
  uint   _grows;                // For debugging, count of table grow()s
  uint   _look_probes;          // For debugging, count of hash probes
  uint   _lookup_hits;          // For debugging, count of hash_finds
  uint   _lookup_misses;        // For debugging, count of hash_finds
  uint   _insert_probes;        // For debugging, count of hash probes
  uint   _delete_probes;        // For debugging, count of hash probes for deletes
  uint   _delete_hits;          // For debugging, count of hash probes for deletes
  uint   _delete_misses;        // For debugging, count of hash probes for deletes
  uint   _total_inserts;        // For debugging, total inserts into hash table
  uint   _total_insert_probes;  // For debugging, total probes while inserting
};


//-----------------------------------------------------------------------------
// Map dense integer indices to Types.  Uses classic doubling-array trick.
// Abstractly provides an infinite array of Type*'s, initialized to NULL.
// Note that the constructor just zeros things, and since I use Arena 
// allocation I do not need a destructor to reclaim storage.
// Despite the general name, this class is customized for use by PhaseTransform.
class Type_Array : public StackObj {
  Arena *_a;                    // Arena to allocate in
  uint   _max;
  const Type **_types;
  void grow( uint i );          // Grow array node to fit
  const Type *operator[] ( uint i ) const // Lookup, or NULL for not mapped
  { return (i<_max) ? _types[i] : (Type*)NULL; }
  friend class PhaseTransform;
public:
  Type_Array(Arena *a) : _a(a), _max(0), _types(0) {}
  Type_Array(Type_Array *ta) : _a(ta->_a), _max(ta->_max), _types(ta->_types) { }
  const Type *fast_lookup(uint i) const{assert(i<_max,"oob");return _types[i];}
  // Extend the mapping: index i maps to Type *n.
  void map( uint i, const Type *n ) { if( i>=_max ) grow(i); _types[i] = n; }
  uint Size() const { return _max; }
#ifndef PRODUCT
  void dump() const;
#endif
};


//------------------------------PhaseRemoveUseless-----------------------------
// Remove useless nodes from GVN hash-table, worklist, and graph
class PhaseRemoveUseless : public Phase {
protected:
  Unique_Node_List _useful;   // Nodes reachable from root
                              // list is allocated from current resource area
public:
  PhaseRemoveUseless( PhaseGVN *gvn, Unique_Node_List *worklist );

  Unique_Node_List *get_useful() { return &_useful; }
};


//------------------------------PhaseTransform---------------------------------
// Phases that analyze, then transform.  Constructing the Phase object does any
// global or slow analysis.  The results are cached later for a fast 
// transformation pass.  When the Phase object is deleted the cached analysis
// results are deleted.
class PhaseTransform : public Phase {
protected:
  Arena*     _arena;
  Node_Array _nodes;           // Map old node indices to new nodes.
  Type_Array _types;           // Map old node indices to Types.

public:
  PhaseTransform( PhaseNumber pnum );
  PhaseTransform( Arena *arena, PhaseNumber pnum );
  PhaseTransform( PhaseTransform *phase, PhaseNumber pnum );

  Arena*      arena()   { return _arena; }
  Type_Array& types()   { return _types; }
  // _nodes is used in varying ways by subclasses, which define local accessors

public:
  // Get a previously recorded type for the node n.
  // This type must already have been recorded.
  // If you want the type of a very new (untransformed) node,
  // you must use type_or_null, and test the result for NULL.
  const Type* type(const Node* n) const {
    const Type* t = _types.fast_lookup(n->_idx);
    assert(t != NULL, "must set before get");
    return t;
  }
  // Get a previously recorded type for the node n,
  // or else return NULL if there is none.
  const Type* type_or_null(const Node* n) const {
    return _types.fast_lookup(n->_idx);
  }
  // Record a type for a node.
  void    set_type(const Node* n, const Type *t) {
    assert(t != NULL, "type must not be null");
    _types.map(n->_idx, t);
  }
  // Record an initial type for a node, the node's bottom type.
  void    set_type_bottom(const Node* n) {
    // Use this for initialization when bottom_type() (or better) is not handy.
    // Usually the initialization shoudl be to n->Value(this) instead,
    // or a hand-optimized value like Type::MEMORY or Type::CONTROL.
    assert(_types[n->_idx] == NULL, "must set the initial type just once");
    _types.map(n->_idx, n->bottom_type());
  }
  // Make sure the types array is big enough to record a size for the node n.
  // (In product builds, we never want to do range checks on the types array!)
  void ensure_type_or_null(const Node* n) {
    if (n->_idx >= _types.Size())
      _types.map(n->_idx, NULL);   // Grow the types array as needed.
  }
  
  // Return a node which computes the same function as this node, but
  // in a faster or cheaper fashion.  
  virtual Node *transform( Node *n ) = 0;

  // Return whether two Nodes are equivalent.  
  // Must not be recursive, since the recursive version is built from this.
  // For pessimistic optimizations this is simply pointer equivalence.
  int  eqv( const Node *n1, const Node *n2 ) const { return n1 == n2; }

  // For pessimistic passes, the return type must monotonically narrow.
  // For optimistic  passes, the return type must monotonically widen.
  // In any case, only widen or narrow a few times before going to the
  // correct flavor of top or bottom.
  virtual const Type *widen(const Type *new_type,const Type *old_type) const
  { ShouldNotCallThis(); return NULL; }

#ifndef PRODUCT
  void dump_old2new_map() const;
  void dump_new( uint new_lidx ) const;
  void dump_types() const;
  void dump_nodes_and_types(const Node *root, uint depth, bool only_ctrl = true);
  void dump_nodes_and_types_recur( const Node *n, uint depth, bool only_ctrl, VectorSet &visited);

  uint   _count_progress;       // For profiling, count transforms that make progress
  void   set_progress()        { ++_count_progress; assert( allow_progress(),"No progress allowed during verification") }
  void   clear_progress()      { _count_progress = 0; }
  uint   made_progress() const { return _count_progress; }

  uint   _count_transforms;     // For profiling, count transforms performed
  void   set_transforms()      { ++_count_transforms; }
  void   clear_transforms()    { _count_transforms = 0; }
  uint   made_transforms() const{ return _count_transforms; }

  bool   _allow_progress;      // progress not allowed during verification pass
  void   set_allow_progress(bool allow) { _allow_progress = allow; }
  bool   allow_progress()               { return _allow_progress; }
#endif
};

//------------------------------PhaseValues------------------------------------
// Phase infrastructure to support values
class PhaseValues : public PhaseTransform {
protected:
  NodeHash  _table;             // Hash table for value-numbering

  enum { _icon_min = -4,
         _icon_max = 32
  };
  ConINode *_icons[_icon_max - _icon_min + 1];   // cached integer constant nodes
  ConNode  *_zcons[T_CONFLICT+1];   // cached zero nodes

public:
  PhaseValues( Arena *arena, uint est_max_size );
  PhaseValues( PhaseValues *pt );
  PhaseValues( PhaseValues *ptv, const char *dummy );
  ~PhaseValues();
  virtual PhaseIterGVN *is_IterGVN() { return 0; }

  // Some Ideal and other transforms delete --> modify --> insert values
  bool   hash_delete(Node *n)     { return _table.hash_delete(n); }
  void   hash_insert(Node *n)     { _table.hash_insert(n); }
  Node  *hash_find_insert(Node *n){ return _table.hash_find_insert(n); }
  Node  *hash_find(const Node *n) { return _table.hash_find(n); }

  // Used after parsing to eliminate values that are no longer in program
  void   remove_useless_nodes(VectorSet &useful) { _table.remove_useless_nodes(useful); }

  // Make an idealized constant, i.e., one of ConINode, ConPNode, ConFNode, etc
  virtual ConNode *makecon( const Type *t );

  // Fast integer constant. Same as "transform(new ConINode(TypeInt::make(i)))"
  ConINode *intcon ( jint  i );
  ConLNode *longcon( jlong l );

  // Fast zero or null constant. Same as "transform(ConNode::make(Type::get_zero_type(bt)))"
  virtual ConNode *zerocon( BasicType bt );

  virtual const Type *widen( const Type *new_type, const Type *old_type ) const
  { return new_type; };

#ifndef PRODUCT
  uint   _count_new_values;     // For profiling, count new values produced
  void    inc_new_values()        { ++_count_new_values; }
  void    clear_new_values()      { _count_new_values = 0; }
  uint    made_new_values() const { return _count_new_values; }
#endif
};


//------------------------------PhaseGVN---------------------------------------
// Phase for performing local, pessimistic GVN-style optimizations.
class PhaseGVN : public PhaseValues {
public:
  PhaseGVN( Arena *arena, uint est_max_size ) : PhaseValues( arena, est_max_size ) {}
  PhaseGVN( PhaseGVN *gvn ) : PhaseValues( gvn ) {}
  PhaseGVN( PhaseGVN *gvn, const char *dummy ) : PhaseValues( gvn, dummy ) {}

  // Return a node which computes the same function as this node, but
  // in a faster or cheaper fashion.  
  Node  *transform( Node *n );
  Node  *transform_no_reclaim( Node *n );
};

//------------------------------PhaseIterGVN-----------------------------------
// Phase for iteratively performing local, pessimistic GVN-style optimizations.
// and ideal transformations on the graph.
class PhaseIterGVN : public PhaseGVN {
  // Idealize old Node 'n' with respect to its inputs and its value
  virtual Node *transform_old( Node *a_node );
protected:

  // Idealize new Node 'n' with respect to its inputs and its value
  virtual Node *transform( Node *a_node );

  // Warm up hash table, type table and initial worklist
  void init_worklist( Node *a_root );

  virtual const Type *widen( const Type *new_type, const Type *old_type ) const
  { return new_type; };

public:
  PhaseIterGVN( PhaseIterGVN *igvn ); // Used by CCP constructor
  PhaseIterGVN( PhaseGVN *gvn ); // Used after Parser
  PhaseIterGVN( PhaseIterGVN *igvn, const char *dummy ); // Used after +VerifyOpto

  virtual PhaseIterGVN *is_IterGVN() { return this; } 

  Unique_Node_List _worklist;       // Iterative worklist 

  // Given def-use info and an initial worklist, apply Node::Ideal, 
  // Node::Value, Node::Identity, hash-based value numbering, Node::Ideal_DU
  // and dominator info to a fixed point.
  void optimize();

  // Make an idealized constant, i.e., one of ConINode, ConPNode, ConFNode, etc
  ConNode *makecon( const Type *t );

  // Register a new node with the iter GVN pass without transforming it.
  // Used when we need to restructure a Region/Phi area and all the Regions
  // and Phis need to complete this one big transform before any other
  // transforms can be triggered on the region.
  Node *register_new_node_with_optimizer( Node *n );

  // Kill a globally dead Node.   It is allowed to have uses which are
  // assumed dead and left 'in limbo'.
  void remove_globally_dead_node( Node *dead );

  // Kill all inputs to a dead node, recursively making more dead nodes.
  // The Node must be dead locally, i.e., have no uses.
  void remove_dead_node( Node *dead ) {
    assert(dead->outcnt() == 0 && !dead->is_top(), "node must be dead");
    remove_globally_dead_node(dead);
  }

  // Subsume users of node 'old' into node 'nn'
  // If no Def-Use info existed for 'nn' it will after call.
  void subsume_node( Node *old, Node *nn );

  // Add users of 'n' to worklist
  void add_users_to_worklist0( Node *n );
  void add_users_to_worklist ( Node *n );

#ifndef PRODUCT
protected:
  // Sub-quadratic implementation of VerifyIterativeGVN.
  unsigned long _verify_counter;
  unsigned long _verify_full_passes;
  enum { _verify_window_size = 30 };
  Node* _verify_window[_verify_window_size];
  void verify_step(Node* n);
#endif
};

//------------------------------PhaseCCP---------------------------------------
// Phase for performing global Conditional Constant Propagation.
// Should be replaced with combined CCP & GVN someday.
class PhaseCCP : public PhaseIterGVN {
  // Non-recursive.  Use analysis to transform single Node.
  virtual Node *transform_once( Node *n );

public:
  PhaseCCP( PhaseIterGVN *igvn ); // Compute conditional constants
  ~PhaseCCP();

  // Worklist algorithm identifies constants
  void analyze();
  // Recursive traversal of program.  Used analysis to modify program.
  virtual Node *transform( Node *n );
  // Do any transformation after analysis
  void          do_transform();

  virtual const Type *widen( const Type *new_type, const Type *old_type ) const 
  { return new_type->widen(old_type); };

#ifndef PRODUCT
  static uint _total_invokes;    // For profiling, count invocations
  void    inc_invokes()          { ++PhaseCCP::_total_invokes; }

  static uint _total_constants;  // For profiling, count constants found
  uint   _count_constants;
  void    clear_constants()      { _count_constants = 0; }
  void    inc_constants()        { ++_count_constants; }
  uint    count_constants() const { return _count_constants; }

  static void print_statistics();
#endif
};


//------------------------------PhasePeephole----------------------------------
// Phase for performing peephole optimizations on register allocated basic blocks.
class PhasePeephole : public PhaseTransform {
  PhaseRegAlloc *_regalloc;
  PhaseCFG     &_cfg;
  // Recursive traversal of program.  Pure function is unused in this phase
  virtual Node *transform( Node *n );

public:
  PhasePeephole( PhaseRegAlloc *regalloc, PhaseCFG &cfg );
  ~PhasePeephole();

  // Do any transformation after analysis
  void          do_transform();

#ifndef PRODUCT
  static uint _total_peepholes;  // For profiling, count peephole rules applied
  uint   _count_peepholes;
  void    clear_peepholes()      { _count_peepholes = 0; }
  void    inc_peepholes()        { ++_count_peepholes; }
  uint    count_peepholes() const { return _count_peepholes; }

  static void print_statistics();
#endif
};

