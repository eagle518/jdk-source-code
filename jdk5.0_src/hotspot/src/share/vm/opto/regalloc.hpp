#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)regalloc.hpp	1.15 04/03/31 19:43:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Node;
class Matcher;
class PhaseCFG;

#define  MAX_REG_ALLOCATORS   10

//------------------------------PhaseRegAlloc------------------------------------
// Abstract register allocator
class PhaseRegAlloc : public Phase {
  static void (*_alloc_statistics[MAX_REG_ALLOCATORS])();
  static int _num_allocators;

protected:
  OptoRegPair *_node_regs;
  uint         _node_regs_max_index;
  VectorSet    _node_oops;         // Mapping from node indices to oopiness

  void alloc_node_regs(int size);  // allocate _node_regs table with at least "size" elements

  PhaseRegAlloc( uint unique, PhaseCFG &cfg, Matcher &matcher,
                 void (*pr_stats)());
public:
  PhaseCFG &_cfg;               // Control flow graph
  uint _framesize;              // Size of frame in stack-slots. not counting preserve area
  OptoReg::Name _max_reg;       // Past largest register seen
  Matcher &_matcher;            // Convert Ideal to MachNodes
  uint node_regs_max_index() const { return _node_regs_max_index; }

  // Get the register associated with the Node
  OptoReg::Name get_reg_lo( const Node *n ) const {
    debug_only( if( n->_idx >= _node_regs_max_index ) n->dump(); );
    assert( n->_idx < _node_regs_max_index, "Exceeded _node_regs array");
    return _node_regs[n->_idx].lo();
  }
  OptoReg::Name get_reg_hi( const Node *n ) const {
    debug_only( if( n->_idx >= _node_regs_max_index ) n->dump(); );
    assert( n->_idx < _node_regs_max_index, "Exceeded _node_regs array");
    return _node_regs[n->_idx].hi();
  }

  // Do all the real work of allocate
  virtual void Register_Allocate() = 0;


  // notify the register allocator that "node" is a new reference
  // to the value produced by "old_node"
  virtual void add_reference( const Node *node, const Node *old_node) = 0;


  // Set the register associated with a new Node
  void set_bad( uint idx ) {
    assert( idx < _node_regs_max_index, "Exceeded _node_regs array");
    _node_regs[idx].set_bad();
  }
  void set1( uint idx, OptoReg::Name reg ) {
    assert( idx < _node_regs_max_index, "Exceeded _node_regs array");
    _node_regs[idx].set1(reg);
  }
  void set2( uint idx, OptoReg::Name reg ) {
    assert( idx < _node_regs_max_index, "Exceeded _node_regs array");
    _node_regs[idx].set2(reg);
  }
  void set_pair( uint idx, OptoReg::Name hi, OptoReg::Name lo ) {
    assert( idx < _node_regs_max_index, "Exceeded _node_regs array");
    _node_regs[idx].set_pair(hi,lo);
  }
  void set_ptr( uint idx, OptoReg::Name reg ) {
    assert( idx < _node_regs_max_index, "Exceeded _node_regs array");
    _node_regs[idx].set_ptr(reg);
  }
  // Set and query if a node produces an oop
  void set_oop( const Node *n, bool );
  bool is_oop( const Node *n ) const;

  // Convert a register number to a stack offset
  int reg2offset          ( OptoReg::Name reg ) const;
  int reg2offset_unchecked( OptoReg::Name reg ) const;

  // Convert a stack offset to a register number
  OptoReg::Name offset2reg( int stk_offset ) const;

  // Get the register encoding associated with the Node
  int get_encode( const Node *n ) const {
    assert( n->_idx < _node_regs_max_index, "Exceeded _node_regs array");
    OptoReg::Name lo = _node_regs[n->_idx].lo();
    OptoReg::Name hi = _node_regs[n->_idx].hi();
    assert( hi == OptoReg::Bad || hi == lo+1, "" );
    assert(lo >= 0 && lo < REG_COUNT, "out of range");
    return Matcher::_regEncode[lo];
  }  

  // Platform dependent hook for actions prior to allocation
  void  pd_preallocate_hook();

#ifdef ASSERT
  // Platform dependent hook for verification after allocation.  Will
  // only get called when compiling with asserts.
  void  pd_postallocate_verify_hook();
#endif

#ifndef PRODUCT
  static int _total_framesize;
  static int _max_framesize;

  virtual void dump_frame() const = 0;
  virtual char *dump_register( const Node *n, char *buf  ) const = 0;
  static void print_statistics();
#endif
};
