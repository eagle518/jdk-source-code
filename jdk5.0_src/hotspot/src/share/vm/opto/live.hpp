#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)live.hpp	1.36 03/12/23 16:42:34 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Block;
class LRG_List;
class PhaseCFG;
class VectorSet;
class IndexSet;

//------------------------------PhaseLive--------------------------------------
// Compute live-in/live-out
class PhaseLive : public Phase {
  // Array of Sets of values live at the start of a block.
  // Indexed by block pre-order number.
  IndexSet *_live;

  // Array of Sets of values defined locally in the block
  // Indexed by block pre-order number.
  IndexSet *_defs;

  // Array of delta-set pointers, indexed by block pre-order number
  IndexSet **_deltas;
  IndexSet *_free_IndexSet;     // Free list of same

  Block_List *_worklist;        // Worklist for iterative solution

  const PhaseCFG &_cfg;         // Basic blocks
  LRG_List &_names;             // Mapping from Nodes to live ranges
  uint _maxlrg;                 // Largest live-range number
  Arena *_arena;

  IndexSet *getset( Block *p );
  IndexSet *getfreeset( );
  void freeset( const Block *p );
  void add_liveout( Block *p, uint r, VectorSet &first_pass );
  void add_liveout( Block *p, IndexSet *lo, VectorSet &first_pass );

public:
  PhaseLive( const PhaseCFG &cfg, LRG_List &names, Arena *arena );
  ~PhaseLive();
  // Compute liveness info
  void compute(uint maxlrg);
  // Reset arena storage
  void reset() { _live = NULL; }

  // Return the live-out set for this block
  IndexSet *live( const Block * b ) { return &_live[b->_pre_order-1]; }

#ifndef PRODUCT
  void dump( const Block *b ) const;
  void stats(uint iters) const;
#endif
};
