#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)block.hpp	1.83 03/12/23 16:42:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Optimization - Graph Style

class Block;
class MachCallNode;
class Matcher;
class RootNode;
class VectorSet;
struct Tarjan;

//------------------------------Loop-------------------------------------------
// Simple inner loop container class.  Holds some info used by allocator.
struct InnerLoop : public ResourceObj {
  InnerLoop( InnerLoop *next, Block *head, Arena *a ) : _next(next), _head(head), _ud_lrgs(a), _isolate_lrgs(a) {}
  InnerLoop *_next;             // Next inner loop
  Block *_head;                 // Block heading the inner loop
  VectorSet _ud_lrgs;           // Set of live ranges use/def'd in loop
  VectorSet _isolate_lrgs;      // Set of live ranges to split at boundaries
};

//------------------------------Block_Array------------------------------------
// Map dense integer indices to Blocks.  Uses classic doubling-array trick.
// Abstractly provides an infinite array of Block*'s, initialized to NULL.
// Note that the constructor just zeros things, and since I use Arena 
// allocation I do not need a destructor to reclaim storage.
class Block_Array : public ResourceObj {
  uint _size;                   // allocated size, as opposed to formal limit
  debug_only(uint _limit;)      // limit to formal domain
protected:
  Block **_blocks;
  void grow( uint i );          // Grow array node to fit

public:
  Arena *_arena;                // Arena to allocate in

  Block_Array(Arena *a) : _arena(a), _size(OptoBlockListSize) {
    debug_only(_limit=0);
    _blocks = NEW_ARENA_ARRAY( a, Block *, OptoBlockListSize );
    for( int i = 0; i < OptoBlockListSize; i++ ) {
      _blocks[i] = NULL;
    }
  }
  Block *lookup( uint i ) const // Lookup, or NULL for not mapped
  { return (i<Max()) ? _blocks[i] : (Block*)NULL; }
  Block *operator[] ( uint i ) const // Lookup, or assert for not mapped
  { assert( i < Max(), "oob" ); return _blocks[i]; }
  // Extend the mapping: index i maps to Block *n.
  void map( uint i, Block *n ) { if( i>=Max() ) grow(i); _blocks[i] = n; }
  uint Max() const { debug_only(return _limit); return _size; }
  // Clear a temporary at each block, generally holding a node index.
  void clear_splitting_temps();
  void clear_anti_dep_temps();
  void clear_blocker_temps();
  void clear_clone_temps();
  void clear_all_temps();

};


class Block_List : public Block_Array {
public:
  uint _cnt;
  Block_List() : Block_Array(Thread::current()->resource_area()), _cnt(0) {}
  void push( Block *b ) { map(_cnt++,b); }
  Block *pop() { return _blocks[--_cnt]; }
  Block *rpop() { Block *b = _blocks[0]; _blocks[0]=_blocks[--_cnt]; return b;}
  void remove( uint i );
  void insert( uint i, Block *n );
  uint size() const { return _cnt; }
  void reset() { _cnt = 0; }
};


//------------------------------Block------------------------------------------
// This class defines a Basic Block.
// Basic blocks are used during the output routines, and are not used during
// any optimization pass.  They are created late in the game.
class Block : public ResourceObj {
 public:
  // Nodes in this block, in order
  Node_List _nodes;

  // Basic blocks have a Node which defines Control for all Nodes pinned in
  // this block.  This Node is a RegionNode.  Exception-causing Nodes
  // (division, subroutines) and Phi functions are always pinned.  Later,
  // every Node will get pinned to some block.
  Node *head() const { return _nodes[0]; }

  // CAUTION: num_preds() is ONE based, so that predecessor numbers match
  // input edges to Regions and Phis.
  uint num_preds() const { return head()->req(); }
  Node *pred(uint i) const { return head()->in(i); }

  // Array of successor blocks, same size as projs array
  Block_Array _succs;

  // Basic blocks have some number of Nodes which split control to all
  // following blocks.  These Nodes are always Projections.  The field in
  // the Projection and the block-ending Node determine which Block follows.
  uint _num_succs;

  // Basic blocks also carry all sorts of good old fashioned DFS information
  // used to find loops, loop nesting depth, dominators, etc.
  uint _pre_order;              // Pre-order DFS number

  // Dominator tree
  uint _dom_depth;              // Depth in dominator tree for fast LCA
  Block *_idom;                 // Immediate dominator block
  InnerLoop *_inner_loop;       // Containing inner loop, or null

  // Return NULL, if no blocker_block occurs between LCA and early
  // Else return a new LCA that dominates LCA and blocker_block
  Block *hoist_LCA_above_defs( Block *early, Block *LCA, 
                               node_idx_t index, Block_Array &bbs );
  
  // Report the alignment required by this block.  Must be a power of 2.
  // The previous block will insert nops to get this alignment.
  uint code_alignment();

  // Execution frequency (estimate)
  float _freq;
  float _cnt;

#ifdef ASSERT
  // Validate _cnt and _freq
  bool has_valid_counts() const { 
    if (_freq <= 0.0f) return false;
    if ((_cnt <= 0.0f) && (_cnt != COUNT_UNKNOWN)) return false;
    return true;
  }
#endif

  // Register Pressure (estimate) for Splitting heuristic
  uint _reg_pressure;
  uint _ihrp_index;
  uint _freg_pressure;
  uint _fhrp_index;

  // -- Used for anti-dependences, holds the index of a load while finding
  // -- the latest block at which the load may be scheduled
  node_idx_t _anti_dep;
  void    set_anti_dep(node_idx_t index)  { _anti_dep = index; };
  node_idx_t  anti_dep_index() const      { return _anti_dep; }
  // -- Used for anti-dependences, indicates this block may contain a store
  // -- that conflicts with node_index.
  node_idx_t   _blocker;
  void      set_blocker(node_idx_t index) { _blocker = index; };
  node_idx_t    blocker_index() const     { return _blocker; }

  // Create a new Block with given head Node.
  // Creates the (empty) predecessor arrays.
  Block( Arena *a, Node *headnode ) 
    : _nodes(a), 
      _succs(a), 
      _num_succs(0), 
      _pre_order(0), 
      _idom(0), 
      _inner_loop(0), 
      _freq(0.0f), 
      _cnt(COUNT_UNKNOWN), 
      _reg_pressure(0), 
      _ihrp_index(1), 
      _freg_pressure(0), 
      _fhrp_index(1), 
      _anti_dep(0), 
      _blocker(0) { 
    _nodes.push(headnode); 
  }

  // Index of 'end' Node
  uint end_idx() const {
    // %%%%% add a proj after every goto 
    // so (last->is_block_proj() != last) always, then simplify this code
    // This will not give correct end_idx for block 0 when it only contains root.
    int last_idx = _nodes.size() - 1;
    Node *last  = _nodes[last_idx];
    assert(last->is_block_proj() == last || last->is_block_proj() == _nodes[last_idx - _num_succs], "");
    return (last->is_block_proj() == last) ? last_idx : (last_idx - _num_succs);
  }

  // Basic blocks have a Node which ends them.  This Node determines which 
  // basic block follows this one in the program flow.  This Node is either an
  // IfNode, a GotoNode, a JmpNode, or a ReturnNode.
  Node *end() const { return _nodes[end_idx()]; }

  // Add an instruction to an existing block.  It must go after the head
  // instruction and before the end instruction.
  void add_inst( Node *n ) { _nodes.insert(end_idx(),n); }
  // Find node in block
  uint find_node( const Node *n ) const;
  // Find and remove n from block list
  void find_remove( const Node *n ); 

  // Schedule a call next in the block
  uint sched_call(Matcher &m, Block_Array &bbs, uint node_cnt, Node_List &worklist, int *ready_cnt, MachCallNode *mcall, VectorSet &next_call);

  // Perform basic-block local scheduling
  Node *select(Node_List &worklist, Block_Array &bbs, int *ready_cnt, VectorSet &next_call, uint sched_slot, GrowableArray<uint> &node_latency);
  void set_next_call( Node *n, VectorSet &next_call, Block_Array &bbs );
  void needed_for_next_call(Node *this_call, VectorSet &next_call, Block_Array &bbs);
  bool schedule_local(Matcher &m, Block_Array &bbs, int *ready_cnt, VectorSet &next_call, GrowableArray<uint> &node_latency);
  // Cleanup if any code lands between a Call and his Catch
  void call_catch_cleanup(Block_Array &bbs);
  // Detect implicit-null-check opportunities.  Basically, find NULL checks 
  // with suitable memory ops nearby.  Use the memory op to do the NULL check.
  // I can generate a memory op if there is not one nearby.
  void implicit_null_check(Block_Array &bbs, GrowableArray<uint> &latency, Node *pro, Node *val);
  void remove_dead(Node *dead);

  // Try to empty simple basic blocks: ones with 1 predecessor, 1 successor 
  // and containing only 1 or 2 constants.  Hoist the constants to the prior
  // block so they get speculatively loaded.  Puts a load-constant in the
  // common path in exchange for removing an unconditional branch on one path.
  void empty_simple( Block_Array &bbs );

  // True if block is empty
  int is_Empty() const;
  // True if block is way uncommon
  bool is_uncommon( Block_Array &bbs ) const;

#ifndef PRODUCT
  // Debugging print of basic block
  void dump_head( const Block_Array *bbs ) const;
  void dump( ) const;
  void dump( const Block_Array *bbs ) const;
#endif
};


//------------------------------PhaseCFG---------------------------------------
// Build an array of Basic Block pointers, one per Node.
class PhaseCFG : public Phase {
 private:
  // Build a proper looking cfg.  Return count of basic blocks
  uint build_cfg();

  // Perform DFS search.  
  // Setup 'vertex' as DFS to vertex mapping.  
  // Setup 'semi' as vertex to DFS mapping.  
  // Set 'parent' to DFS parent.             
  uint DFS( Tarjan *tarjan );

  // Set the basic block for pinned Nodes
  void schedule_pinned_nodes( VectorSet &visited );

  // I'll need a few machine-specific GotoNodes.  Clone from this one.
  MachNode *_goto;
  void insert_goto_at(uint block_no, uint succ_no);

 public:
  PhaseCFG( Arena *a, RootNode *r, Matcher &m );

  uint _num_blocks;             // Count of basic blocks
  Block_List _blocks;           // List of basic blocks  
  RootNode *_root;              // Root of whole program
  Block_Array _bbs;             // Map Nodes to owning Basic Block
  Block *_broot;                // Basic block of root
  InnerLoop *_loops;            // Head of list of inner loops
  uint _rpo_ctr;

  // Build dominators
  void Dominators();

  // Find inner loops; collect them for allocator
  void Find_Inner_Loops();

  // Estimate block frequencies based on IfNode probabilities
  void Estimate_Block_Frequency();

  // Global Code Motion.  See Click's PLDI95 paper.  Place Nodes in specific
  // basic blocks; i.e. _bbs now maps _idx for all Nodes to some Block.
  void GlobalCodeMotion( Matcher &m, uint unique, Node_List &proj_list );
 
  // Schedule Nodes early in their basic blocks.
  bool schedule_early(VectorSet &visited, Node_List &roots, Block_Array &bbs);

  // Now schedule all codes as LATE as possible.  This is the LCA in the 
  // dominator tree of all USES of a value.  Pick the block with the least
  // loop nesting depth that is lowest in the dominator tree.
  void schedule_late(VectorSet &visited, Node_List &stack, GrowableArray<uint> &latency);

  // Compute a reverse-postorder walk order over the instructions
  int BuildNodeWalk(Node_Array &order);

  // Compute the instruction global latency with a backwards walk
  void ComputeLatenciesBackwards(VectorSet &visited, Node_List &stack, GrowableArray<uint> &latency);

  // Compute the instruction local (per-bb) latency with a backwards walk
  void ComputeLocalLatenciesBackwards(VectorSet &visited, Node_List &stack, GrowableArray<uint> &latency);

  // Remove empty basic blocks
  void RemoveEmpty();

  // Check for NeverBranch at block end.  This needs to become a GOTO to the
  // true target.  NeverBranch are treated as a conditional branch that always
  // goes the same direction for most of the optimizer and are used to give a
  // fake exit path to infinite loops.  At this late stage they need to turn
  // into Goto's so that when you enter the infinite loop you indeed hang.
  void convert_NeverBranch_to_Goto(Block *b);

  // Insert a node into a block, and update the _bbs
  void insert( Block *b, uint idx, Node *n ) { 
    b->_nodes.insert( idx, n ); 
    _bbs.map( n->_idx, b ); 
  } 

#ifndef PRODUCT
  // Debugging print of CFG
  void dump( ) const;           // CFG only
  void _dump_cfg( const Node *end, VectorSet &visited  ) const;
  void verify() const;
  void dump_headers();
#endif
};


//------------------------------UnionFindInfo----------------------------------
// Map Block indices to a block-index for a cfg-cover.
// Array lookup in the optimized case.
class UnionFind : public ResourceObj {
  uint _cnt, _max;
  uint* _indices;
  ReallocMark _nesting;  // assertion check for reallocations
public:
  UnionFind( uint max );
  void reset( uint max );  // Reset to identity map for [0..max]

  uint lookup( uint nidx ) const {
    return _indices[nidx];
  }
  uint operator[] (uint nidx) const { return lookup(nidx); }

  void map( uint from_idx, uint to_idx ) {
    assert( from_idx < _cnt, "oob" );
    _indices[from_idx] = to_idx;
  }
  void extend( uint from_idx, uint to_idx );

  uint Size() const { return _cnt; }

  uint Find( uint idx ) {
    assert( idx < 65536, "Must fit into uint");
    uint uf_idx = lookup(idx);
    return (uf_idx == idx) ? uf_idx : Find_compress(idx);
  }
  uint Find_compress( uint idx );
  uint Find_const( uint idx ) const;
  void Union( uint idx1, uint idx2 );

};

