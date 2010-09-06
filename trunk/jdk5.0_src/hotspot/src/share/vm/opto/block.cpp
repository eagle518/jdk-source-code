#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)block.cpp	1.151 04/05/04 13:26:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Optimization - Graph Style

#include "incls/_precompiled.incl"
#include "incls/_block.cpp.incl"


//-----------------------------------------------------------------------------
void Block_Array::grow( uint i ) {
  assert(i >= Max(), "must be an overflow");
  debug_only(_limit = i+1);
  if( i < _size )  return;
  if( !_size ) { 
    _size = 1;
    _blocks = (Block**)_arena->Amalloc( _size * sizeof(Block*) );
    _blocks[0] = NULL;
  }
  uint old = _size;
  while( i >= _size ) _size <<= 1;      // Double to fit
  _blocks = (Block**)_arena->Arealloc( _blocks, old*sizeof(Block*),_size*sizeof(Block*));
  Copy::zero_to_bytes( &_blocks[old], (_size-old)*sizeof(Block*) );
}

void Block_Array::clear_anti_dep_temps() {
  for( uint i = 0; i < Max(); i++ ) {
    if( _blocks[i] == NULL ) continue;
    _blocks[i]->_anti_dep = 0;
  }
}

void Block_Array::clear_blocker_temps() {
  for( uint i = 0; i < Max(); i++ ) {
    if( _blocks[i] == NULL ) continue;
    _blocks[i]->_blocker = 0;
  }
}

void Block_Array::clear_all_temps() {
  for( uint i = 0; i < Max(); i++ ) {
    if( _blocks[i] == NULL ) continue;
    _blocks[i]->_anti_dep  = 0;
    _blocks[i]->_blocker   = 0;
  }
}

//=============================================================================
void Block_List::remove(uint i) {
  assert(i < _cnt, "index out of bounds");
  Copy::conjoint_words_to_lower((HeapWord*)&_blocks[i+1], (HeapWord*)&_blocks[i], ((_cnt-i-1)*sizeof(Block*)));
  pop(); // shrink list by one block
}

void Block_List::insert(uint i, Block *b) {
  push(b); // grow list by one block
  Copy::conjoint_words_to_higher((HeapWord*)&_blocks[i], (HeapWord*)&_blocks[i+1], ((_cnt-i-1)*sizeof(Block*)));
  _blocks[i] = b;
}


//=============================================================================

uint Block::code_alignment() {
  // Check for Root block
  if( _pre_order == 0 ) return CodeEntryAlignment;
  // Check for Start block
  if( _pre_order == 1 ) return InteriorEntryAlignment;
  // Check for loop alignment
  LoopNode *l = head()->is_Loop();
  if( l && l->is_inner_loop() )  {
    // Pre- and post-loops have low trip count so do not bother with
    // NOPs for align loop head.  The constants are hidden from tuning
    // but only because my "divide by 4" heuristic surely gets nearly
    // all possible gain (a "do not align at all" heuristic has a
    // chance of getting a really tiny gain).
    CountedLoopNode *cl = head()->is_CountedLoop();
    if( cl && (cl->is_pre_loop() || cl->is_post_loop()) )
      return (OptoLoopAlignment > 4) ? (OptoLoopAlignment>>2) : 1;
    // Loops with low backedge frequency should not be aligned.
    MachNode *mach = l->in(LoopNode::LoopBackControl)->in(0)->is_Mach();
    if( mach ) {
      const MachIfNode *miff = mach->is_MachIf();
      if( miff && miff->_prob < 0.01 )
        return 1;             // Loop does not loop, more often than not!
    }
    return OptoLoopAlignment; // Otherwise align loop head
  }
  return 1;                     // no particular alignment
}

//-----------------------------------------------------------------------------
uint Block::find_node( const Node *n ) const {
  for( uint i = 0; i < _nodes.size(); i++ ) {
    if( _nodes[i] == n )
      return i;
  }
  ShouldNotReachHere();
  return 0;
}

// Find and remove n from block list
void Block::find_remove( const Node *n ) {
  _nodes.remove(find_node(n));
}

//------------------------------is_Empty---------------------------------------
// True if the block is empty.  Empty blocks contain only the head, Phi 
// functions, the tail, and projections of the tail.
int Block::is_Empty() const {

  if( _nodes.size() <= 1 ) return 0;  // Root block is not empty
  if( num_preds() == 1 ) return 1; // Unreachable block
  if( !_nodes[_nodes.size()-1]->is_Goto() )
    return 0;                   // Control flow is not empty
  if( _nodes.size()==2 ) return 1; // No room for any interesting instructions
  // Check instruction right before final branch
  if( !_nodes[_nodes.size()-2]->is_Phi() ) return 0;
  return 1;
}

//------------------------------is_uncommon------------------------------------
// True if block is low enough frequency or guarded by a test which 
// mostly does not go here.
bool Block::is_uncommon( Block_Array &bbs ) const {
  // Check for way-low freq
  if( _freq < 0.00001f ) return true;
  // Also check for guarding test
  if( _freq < 0.001 && (bbs[pred(1)->_idx]->_freq > _freq * 10000.0f) )
    return true;
  return false;
}

//------------------------------empty_simple-----------------------------------
// Try to empty simple basic blocks: ones with 1 predecessor, 1 successor 
// and containing only 1 or 2 constants.  Hoist the constants to the prior
// block so they get speculatively loaded.  Puts a load-constant in the
// common path in exchange for removing an unconditional branch on one path.
extern const char must_clone[];
void Block::empty_simple( Block_Array &bbs ) {
  if( num_preds() != 2 ) return;
  if( _num_succs != 1 ) return;
  // Exactly 1 Region, 1 Goto and a constant
  if( _nodes.size() < 3 || _nodes.size() > 4 ) return;
  if( !_nodes[1]->is_Con() ) return;
  if( _nodes.size() == 4 && !_nodes[2]->is_Con() ) return;
  if( is_uncommon(bbs) ) return;  // Do not hoist out of uncommon blocks

  // Make sure I do not already have an empty path from predecessor to
  // successor.
  Block *pb = bbs[pred(1)->_idx];
  assert( pb->_num_succs >= 2, "why is this block not folded into prior?" );
  if( pb->end()->is_Catch() ) return;
  for( uint i=0; i<pb->_num_succs; i++ ) {
   Block *psb = pb->_succs[i];
   if( psb == this ) continue;
   if( psb == _succs[0] ) return;
   if( psb->_nodes.size() == 2 && psb->_succs[0] == _succs[0] ) return;
  }
  // If our block is very unlikely relative to prior block, do not hoist
  if( _freq * 4 < pb->_freq ) return;

  // Figure where to put the constants.  Do not put between a 'must_clone'
  // and its use
  uint idx = pb->end_idx()-1;
  if( pb->_num_succs >= 2 ) {   // Ends in a conditional branch?
    // Find test Node in block
    Node *n = pb->end()->in(1);
    for( uint i = 0; i < pb->_nodes.size(); i++ )
      if( pb->_nodes[i] == n ) { // Found test node
        MachNode *mach = pb->_nodes[i]->is_Mach();
        // Does test node set flags?
        if( mach && must_clone[mach->ideal_Opcode()] ) 
          idx = i;              // Insert before flag-setting Node
        break;
      }
  }
  if( idx == 0 ) idx = 1;

  // Move the constants up to the predecessor
  pb->_nodes.insert( idx, _nodes[1] );
  bbs.map(_nodes[1]->_idx,pb);
  _nodes.remove(1);
  if( _nodes.size() == 3 ) {
    pb->_nodes.insert( idx, _nodes[1] );
    bbs.map(_nodes[1]->_idx,pb);
    _nodes.remove(1);
  }
}

//------------------------------dump-------------------------------------------
#ifndef PRODUCT
static void bidx( const Block *b ) {
  if( b->_pre_order ) tty->print("B%d",b->_pre_order);
  else tty->print("N%d", b->head()->_idx);
}

void Block::dump_head( const Block_Array *bbs ) const { 
  // Print the basic block
  bidx(this);
  tty->print(": #\t");

  // Print the incoming CFG edges and the outgoing CFG edges
  for( uint i=0; i<_num_succs; i++ ) {
    bidx(_succs[i]);
    tty->print(" ");
  }
  tty->print("<- ");
  if( head()->is_block_start() ) {
    for( uint i=1; i<num_preds(); i++ ) {
      Node *s = pred(i);
      if( bbs ) {
        bidx((*bbs)[s->_idx]);
        tty->print(" ");
      } else {
        while( !s->is_block_start() ) s = s->in(0);
        tty->print("N%d ", s->_idx );
      }
    }
  } else 
    tty->print("BLOCK HEAD IS JUNK  ");

  // Print loop, if any
  const Block *bhead = this;    // Head of self-loop
  if( _inner_loop )             // Head of containing inner loop
    bhead = _inner_loop->_head;
  LoopNode *loop = bhead->head()->is_Loop();
  if( bbs && loop != NULL && !head()->is_Root() ) {
    const Block *bx = bhead;
    if( !_inner_loop ) bx = (*bbs)[loop->in(LoopNode::LoopBackControl)->_idx];
    else {
      for( uint j = 1; j < loop->req(); j++ ) {
        bx = (*bbs)[loop->in(j)->_idx];
        if( bx->_inner_loop == _inner_loop )
          break;
      }
    }
    tty->print("\tLoop: B%d-B%d ", bhead->_pre_order, bx->_pre_order);
    // Dump any loop-specific bits, especially for CountedLoops.
    loop->dump_spec();
  }
  tty->print(" Freq: %g",_freq);
  if( Verbose || WizardMode ) {
    tty->print(" Count: %g",_cnt);
    tty->print(" IDom: %d/#%d", _idom ? _idom->_pre_order : 0, _dom_depth);
    tty->print(" RegPressure: %d",_reg_pressure);
    tty->print(" IHRP Index: %d",_ihrp_index);
    tty->print(" FRegPressure: %d",_freg_pressure);
    tty->print(" FHRP Index: %d",_fhrp_index);
    tty->cr();
    tty->print(" AntiDep: %d", _anti_dep);
    tty->print(" Blocker: %d", _blocker);
  }
  tty->print_cr("");
  if( _inner_loop && _inner_loop->_head == this ) {
    tty->print("# Lrgs use/def'd in loop: ");
    _inner_loop->_ud_lrgs.print();
    if( _inner_loop->_isolate_lrgs.Size() > 0 ) {
      tty->print("# Lrgs isolated around loop: ");
      _inner_loop->_isolate_lrgs.print();
    }
  }
}

void Block::dump() const { dump(0); }

void Block::dump( const Block_Array *bbs ) const {
  dump_head(bbs);
  uint cnt = _nodes.size();
  for( uint i=0; i<cnt; i++ )
    _nodes[i]->dump();
  tty->print("\n");  
}
#endif

//=============================================================================
//------------------------------PhaseCFG---------------------------------------
PhaseCFG::PhaseCFG( Arena *a, RootNode *r, Matcher &m ) : Phase(CFG), _bbs(a), _root(r), _loops(NULL) {
  ResourceMark rm;
  // I'll need a few machine-specific GotoNodes.  Make an Ideal GotoNode,
  // then Match it into a machine-specific Node.  Then clone the machine
  // Node on demand.
  Node *x = new (1) GotoNode(0);
  x->set_req(0,x);
  _goto = m.match_tree(x);
  _goto->set_req(0,_goto);

  // Build the CFG in Reverse Post Order
  _num_blocks = build_cfg();
  _broot = _bbs[_root->_idx];
}

//------------------------------build_cfg--------------------------------------
// Build a proper looking CFG.  Make every block begin with either a StartNode
// or a RegionNode.  Make every block end with either a Goto, If or Return.
// The RootNode both starts and ends it's own block.  Do this with a recursive
// backwards walk over the control edges.
uint PhaseCFG::build_cfg() {
  Arena *a = Thread::current()->resource_area();
  VectorSet visited(a);

  // Allocate stack with enough space to avoid frequent realloc
  Node_Stack nstack(a, C->unique() >> 1);
  nstack.push(_root, 0);
  uint sum = 0;                 // Counter for blocks

  while (nstack.is_nonempty()) {
    // node and in's index from stack's top
    // 'np' is _root (see above) or RegionNode, StartNode: we push on stack
    // only nodes which point to the start of basic block (see below).
    Node *np = nstack.node();
    // idx > 0, except for the first node (_root) pushed on stack 
    // at the beginning when idx == 0.
    // We will use the condition (idx == 0) later to end the build.
    uint idx = nstack.index();
    Node *proj = np->in(idx);
    const Node *x = proj->is_block_proj();
    // Does the block end with a proper block-ending Node?  One of Return,
    // If or Goto? (This check should be done for visited nodes also).
    if (x == NULL) {                    // Does not end right...
      Node *g = _goto->clone(); // Force it to end in a Goto
      g->set_req(0, proj);
      np->set_req(idx, g);
      x = proj = g;
    }
    if (!visited.test_set(x->_idx)) { // Visit this block once
      // Skip any control-pinned middle'in stuff
      Node *p = proj;
      do {
        proj = p;                   // Update pointer to last Control
        p = p->in(0);               // Move control forward
      } while( !p->is_block_proj() &&
               !p->is_block_start() );
      // Make the block begin with one of Region or StartNode.
      if( !p->is_block_start() ) {
        RegionNode *r = new (2) RegionNode( 2 );
        r->set_req(1, p);           // Insert RegionNode in the way
        proj->set_req(0, r);        // Insert RegionNode in the way
        p = r;
      }
      // 'p' now points to the start of this basic block

      // Put self in array of basic blocks
      Block *bb = new (_bbs._arena) Block(_bbs._arena,p);
      _bbs.map(p->_idx,bb);
      _bbs.map(x->_idx,bb);
      if( x != p )                  // Only for root is x == p
        bb->_nodes.push((Node*)x);

      // Now handle predecessors
      ++sum;                        // Count 1 for self block
      uint cnt = bb->num_preds();
      for (int i = (cnt - 1); i > 0; i-- ) { // For all predecessors
        Node *prevproj = p->in(i);  // Get prior input
        assert( !prevproj->is_Con(), "dead input not removed" );
        // Check to see if p->in(i) is a "control-dependent" CFG edge - 
        // i.e., it splits at the source (via an IF or SWITCH) and merges
        // at the destination (via a many-input Region).
        // This breaks critical edges.  The RegionNode to start the block
        // will be added when <p,i> is pulled off the node stack
        if ( cnt > 2 ) {             // Merging many things?
          assert( prevproj== bb->pred(i),"");
          if(prevproj->is_block_proj() != prevproj) { // Control-dependent edge?
            // Force a block on the control-dependent edge
            Node *g = _goto->clone();       // Force it to end in a Goto
            g->set_req(0,prevproj);
            p->set_req(i,g);
          }
        }
        nstack.push(p, i);  // 'p' is RegionNode or StartNode
      }
    } else { // Post-processing visited nodes
      nstack.pop();                 // remove node from stack
      // Check if it the fist node pushed on stack at the beginning.
      if (idx == 0) break;          // end of the build
      // Find predecessor basic block
      Block *pb = _bbs[x->_idx];
      // Insert into nodes array, if not already there
      if( !_bbs.lookup(proj->_idx) ) {
        assert( x != proj, "" );
        // Map basic block of projection
        _bbs.map(proj->_idx,pb);
        pb->_nodes.push(proj);
      }
      // Insert self as a child of my predecessor block
      pb->_succs.map(pb->_num_succs++, _bbs[np->_idx]);
      assert( pb->_nodes[ pb->_nodes.size() - pb->_num_succs ]->is_block_proj(),
              "too many control users, not a CFG?" );
    }
  }
  // Return number of basic blocks for all children and self
  return sum;
}

//------------------------------mark_inner_loops-------------------------------
static void mark_inner_loops( PhaseCFG *cfg, Block *b ) {
  // Check to see if already marked
  if( b->_inner_loop ) return;
  // Mark as belonging to current inner loop
  b->_inner_loop = cfg->_loops;
  // Recursively mark predecessors
  for( uint i=1; i < b->num_preds(); i++ ) 
    mark_inner_loops( cfg, cfg->_bbs[b->pred(i)->_idx] );
}

//------------------------------Find_Inner_Loops-------------------------------
// Simple inner loop structure used by allocator
void PhaseCFG::Find_Inner_Loops() {

  // Scan for inner loop heads
  for( uint i = 0; i < _num_blocks; i++ ) {
    Block *b = _blocks[i];
    LoopNode *loop = b->head()->is_Loop();
    if( !loop ) continue;
    if( loop == C->root() ) continue; // Ignore rootnode loop
    if( !loop->is_inner_loop() ) continue;
    CountedLoopNode *cl = loop->is_CountedLoop();
    // Don't bother with special handling of pre- and post-loops.
    // They have short trip counts and don't deserve special treatment.
    if( cl && (cl->is_pre_loop() || cl->is_post_loop()) )
      continue;                 // Pre/post loops have short trip cnts

    // Found an inner loop head, create loop, mark head
    _loops = new InnerLoop(_loops, b, C->comp_arena());
    b->_inner_loop = _loops;    // Mark loop head
    
    // Visit loop members recursively.  Start with the backedge and
    // work forwards to loop head.
    mark_inner_loops(this, _bbs[loop->in(LoopNode::LoopBackControl)->_idx]);
  }

}

//------------------------------insert_goto_at---------------------------------
// Inserts a goto & corresponding basic block between
// block[block_no] and its succ_no'th successor block
void PhaseCFG::insert_goto_at(uint block_no, uint succ_no) {
  // get block with block_no
  assert(block_no < _num_blocks, "illegal block number");
  Block* in  = _blocks[block_no];
  // get successor block succ_no
  assert(succ_no < in->_num_succs, "illegal successor number");
  Block* out = in->_succs[succ_no];
  // get ProjNode corresponding to the succ_no'th successor of the in block
  ProjNode* proj = in->_nodes[in->_nodes.size() - in->_num_succs + succ_no]->is_Proj();
  // create region for basic block
  RegionNode* region = new (2) RegionNode(2);
  region->set_req(1, proj);
  // setup corresponding basic block
  Block* block = new (_bbs._arena) Block(_bbs._arena, region);
  _bbs.map(region->_idx, block);
  C->regalloc()->set_bad(region->_idx);
  // add a goto node
  Node* gto = _goto->clone(); // get a new goto node
  gto->set_req(0, region);
  // add it to the basic block
  block->_nodes.push(gto);
  _bbs.map(gto->_idx, block);
  C->regalloc()->set_bad(gto->_idx);
  // hook up successor block
  block->_succs.map(block->_num_succs++, out);
  // remap successor's predecessors if necessary
  for (uint i = 1; i < out->num_preds(); i++) {
    if (out->pred(i) == proj) out->head()->set_req(i, gto);
  }
  // remap predecessor's successor to new block
  in->_succs.map(succ_no, block);
  // add new basic block to basic block list
  _blocks.insert(block_no + 1, block);
  _num_blocks++;
}

//------------------------------no_flip_branch---------------------------------
// Does this block end in a multiway branch that cannot have the default case
// flipped for another case?
static bool no_flip_branch( Block *b ) {
  int branch_idx = b->_nodes.size() - b->_num_succs-1;
  if( branch_idx < 1 ) return false;
  Node *bra = b->_nodes[branch_idx];
  if( bra->is_Catch() ) return true;
  MachNode *mach = bra->is_Mach();
  if( mach ) {
    if( mach->is_MachNullCheck() ) return true;
    int iop = mach->ideal_Opcode();
    if( iop == Op_FastLock || iop == Op_FastUnlock )
      return true;
  }
  return false;
}

//------------------------------convert_NeverBranch_to_Goto--------------------
// Check for NeverBranch at block end.  This needs to become a GOTO to the
// true target.  NeverBranch are treated as a conditional branch that always
// goes the same direction for most of the optimizer and are used to give a
// fake exit path to infinite loops.  At this late stage they need to turn
// into Goto's so that when you enter the infinite loop you indeed hang.
void PhaseCFG::convert_NeverBranch_to_Goto(Block *b) {
  // Find true target
  int end_idx = b->end_idx();
  int idx = b->_nodes[end_idx+1]->is_Proj()->_con;
  Block *succ = b->_succs[idx];
  Node* gto = _goto->clone(); // get a new goto node
  gto->set_req(0, b->head());
  Node *bp = b->_nodes[end_idx];
  b->_nodes.map(end_idx,gto); // Slam over NeverBranch
  _bbs.map(gto->_idx, b);
  C->regalloc()->set_bad(gto->_idx);
  b->_nodes.pop();              // Yank projections
  b->_nodes.pop();              // Yank projections
  b->_succs.map(0,succ);        // Map only successor
  b->_num_succs = 1;
  // remap successor's predecessors if necessary
  uint j;
  for( j = 1; j < succ->num_preds(); j++) 
    if( succ->pred(j)->in(0) == bp ) 
      succ->head()->set_req(j, gto);
  // Kill alternate exit path
  Block *dead = b->_succs[1-idx];
  for( j = 1; j < dead->num_preds(); j++) 
    if( dead->pred(j)->in(0) == bp ) 
      break;
  // Scan through block, yanking dead path from
  // all regions and phis.
  dead->_nodes[0]->del_req(j);
  for( int k = 1; dead->_nodes[k]->is_Phi(); k++ )
    dead->_nodes[k]->del_req(j);
}

//------------------------------RemoveEmpty------------------------------------
// Remove empty basic blocks and useless branches.  
void PhaseCFG::RemoveEmpty() {
  // Move uncommon blocks to the end
  uint last = _num_blocks;
  uint i;
  assert( _blocks[0] == _broot, "" );
  for( i = 1; i < last; i++ ) {
    Block *b = _blocks[i];

    // Check for NeverBranch at block end.  This needs to become a GOTO to the
    // true target.  NeverBranch are treated as a conditional branch that
    // always goes the same direction for most of the optimizer and are used
    // to give a fake exit path to infinite loops.  At this late stage they
    // need to turn into Goto's so that when you enter the infinite loop you
    // indeed hang.
    if( b->_nodes[b->end_idx()]->Opcode() == Op_NeverBranch ) 
      convert_NeverBranch_to_Goto(b);

    // Look for uncommon blocks inside loops; move to end.
    // Uncommon blocks not in loops should already be at the end by the
    // action of the LayoutBlocks pass.
    if( b->is_uncommon(_bbs) ) {
      _blocks.remove(i);        // Move block to end
      _blocks.push(b);
      last--;                   // No longer check for being uncommon!
      if( no_flip_branch(b) ) { // Fall-thru case must follow?
        b = _blocks[i];         // Find the fall-thru block
        _blocks.remove(i);      // Move fall-thru case to end as well
        _blocks.push(b);
        last--;
      }
      i--;                      // backup loop counter post-increment
    }
  }

  // Remove empty blocks
  uint full_blocks = 0;
  uint j1;
  for( i=0; i<_num_blocks; i++ ) {
    Block *b = _blocks[i];
    for( j1=1; j1<b->num_preds(); j1++ ) {
      Block *pb = _bbs[b->pred(j1)->_idx];
      if( pb->is_Empty() ) {
        // If pb has any Phis, make sure b does too
        for( uint k=1; k<pb->_nodes.size(); k++ ) {
          PhiNode *phi = pb->_nodes[k]->is_Phi();
          if( !phi ) break;
          Node *phi_b = NULL;   // Scan for a phi in b which points to phi in pb
          uint l;
          for( l=1; l<b->_nodes.size(); l++ ) {
            Node *phi2 = b->_nodes[l];
            if( !phi2->is_Phi() ) break;
            if( phi2->in(j1) == phi ) {
              phi_b = phi2;
              break;
            }
          }
          if( !phi_b ) {        // Not found.  Make one.
            phi_b = new PhiNode( b->head(), phi->type(), phi->adr_type() );
            for( uint m = 1; m<b->num_preds(); m++ )
              phi_b->set_req(m,phi_b);
            phi->replace_by(phi_b);
            phi_b->set_req(j1,phi);
            b->_nodes.insert(l,phi_b);
            _bbs.map(phi_b->_idx,b);
            PhaseRegAlloc *R = C->regalloc();
            R->set_pair(phi_b->_idx, R->get_reg_hi(phi),R->get_reg_lo(phi));
            R->set_oop(phi_b, R->is_oop(phi));
          }
        }
        // Fixup any missed phis in b (that did not merge in pb),
        // adding in edges that flowed from pb
        for( uint k2=1; k2<b->_nodes.size(); k2++ ) {
          PhiNode *phi_b = b->_nodes[k2]->is_Phi();
          if( !phi_b ) break;
          Node *x = phi_b->in(j1);
          phi_b->del_req(j1);    // remove now defunct input path
          if( _bbs[x->_idx] == pb ) {
            assert( x->is_Phi(), "prior block is empty except for phis" );
            for( uint l=x->req()-1; l > 0; l-- )
              phi_b->add_req(x->in(l)); // Add replicas on all merged-in paths
          } else {
            for( uint l=1; l<pb->num_preds(); l++ )
              phi_b->add_req(x); // Add replicas on all merged-in paths
          }
        }

        // Remove empty block from *my* predecessor list.
        b->head()->del_req(j1);
        // Add empty blocks predecessors to my predecessor list
        for( uint k3=pb->num_preds()-1; k3>0; k3-- ) {
          b->head()->add_req(pb->pred(k3));
          // Also fixup the successor list of the pred-pred block
          Block *pbb = _bbs[pb->pred(k3)->_idx];
          uint l;
          for( l=0; l < pbb->_num_succs; l++ )
            if( pbb->_succs[l] == pb )
              break;
          assert( l < pbb->_num_succs, "" );
          pbb->_succs.map(l,b);
          pb->head()->del_req(k3);
        }

        j1--;                   // Re-run same iteration
      }
    } // End of for all predecessor blocks

    // Compress empty blocks out of block array
    if( !b->is_Empty() ) 
      _blocks.map(full_blocks++,b);

  } // End of for all blocks
  _num_blocks = full_blocks;

  // Fixup final control flow for the blocks.  Remove jump-to-next
  // block.  If neither arm of a IF follows the conditional branch, we
  // have to add a second jump after the conditional.  We place the
  // TRUE branch target in succs[0] for both GOTOs and IFs.
  for( i=0; i<_num_blocks; i++ ) {
    Block *b = _blocks[i];
    Block *bnext = (i < _num_blocks-1) ? _blocks[i+1] : NULL;
    Block *bs0 = b->_succs[0];
    b->_pre_order = i;          // turn pre-order into block-index

    // Check for multi-way branches where I cannot negate the test to
    // exchange the true and false targets.
    if( no_flip_branch( b ) ) {
      // Find fall through case - if must fall into its target
      int branch_idx = b->_nodes.size() - b->_num_succs;
      for (uint j2 = 0; j2 < b->_num_succs; j2++) {
        const ProjNode* p = b->_nodes[branch_idx + j2]->is_Proj();
        if (p->_con == 0) {
          // successor j2 is fall through case
          if (b->_succs[j2] != bnext) 
            // but it is not the next block => insert a goto
            insert_goto_at(i, j2);
          // Put taken branch in slot 0
          if( j2 == 0 && b->_num_succs == 2) {
            Block *bs0 = b->_succs[0];
            Block *bs1 = b->_succs[1];
            b->_succs.map( 0, bs1 );// Flip targets in succs map
            b->_succs.map( 1, bs0 );
          }
          break;
        }
      }
      // Remove all CatchProjs
      for (j1 = 0; j1 < b->_num_succs; j1++) b->_nodes.pop();        

    } else if( b->_num_succs == 1 ) {   // Block ends in a Goto?
      if( bnext == bs0 ) 
        b->_nodes.pop();        // We fall into next block; remove the Goto

    } else if( b->_num_succs == 2 ) { // Block ends in a If?
      Block *bs1 = b->_succs[1];
      // Check for neither block following the conditional.
      // If so, and one of the blocks has not been scheduled and no
      // predecessor precedes it in the schedule (i.e., all paths to it
      // involve a branch) then move it up behind me.
      if( bnext != bs0 && bnext != bs1 ) {  // neither follows in schedule
        // Find b->succ[1] in block list
        uint j3=0; 
        Block *bx = bs1->is_uncommon(_bbs) ? bs0 : bs1;
        while( _blocks[j3] != bx ) j3++;
        if( j3 > i ) {          // 
          // Make sure all preds branch to bx and do not fall into it.
          uint k;
          for( k = 1; k < bx->num_preds(); k++ )
            if( _bbs[bx->pred(k)->_idx] == _blocks[j3-1] )
              break;
          if( k < bx->num_preds() ) {  // Need a double branch!
          } else {                  // Else a little shuffle fixes us
            // Reinsert bx just past 'b'
            _blocks.remove(j3);
            _blocks.insert(i+1,bx);
            bnext = bx;
          }
        } // Else get double branch
      }

      // Get opcode of 1st projection (matches _succs[0])
      // Note: Since this basic block has 2 exits, the last 2 nodes must
      //       be projections (in any order), the 3rd last node must be
      //       the IfNode (we have excluded other 2-way exits such as
      //       CatchNodes already).
      MachNode *iff   = b->_nodes[b->_nodes.size()-3]->is_Mach();
      ProjNode *proj0 = b->_nodes[b->_nodes.size()-2]->is_Proj();
      ProjNode *proj1 = b->_nodes[b->_nodes.size()-1]->is_Proj();

      // Check for conditional branching the wrong way.  Negate
      // conditional, if needed, so it falls into the following block
      // and branches to the not-following block.
      
      // Check for the next block being in succs[0].  We are going to branch
      // to succs[0], so we want the fall-thru case as the next block in 
      // succs[1].
      if( bnext == bs0 ) {      // Fall-thru case in succs[0]?
        b->_succs.map( 0, bs1 );// Flip targets in succs map
        b->_succs.map( 1, bs0 );
        // Flip projection for each target
        { ProjNode *tmp = proj0; proj0 = proj1; proj1 = tmp; }

      } else if( bnext == bs1 ) { // Fall-thru is already in succs[1]

      } else {                  // Else need a double-branch

        // The existing conditional branch need not change.
        // Add a unconditional branch to the false target.
        // Alas, it must appear in its own block and adding a
        // block this late in the game is complicated.  Sigh.
        insert_goto_at(i, 1);
      }

      // Make sure we TRUE branch to the target
      if( proj0->Opcode() == Op_IfFalse )
        iff->negate();
      
      b->_nodes.pop();          // Remove IfFalse & IfTrue projections
      b->_nodes.pop();
      
    } else {
      // Multi-exit block, e.g. a switch statement
      // But we don't need to do anything here
    }

  } // End of for all blocks

}


//------------------------------dump-------------------------------------------
#ifndef PRODUCT
void PhaseCFG::_dump_cfg( const Node *end, VectorSet &visited  ) const {
  const Node *x = end->is_block_proj();
  assert( x, "not a CFG" );

  // Do not visit this block again
  if( visited.test_set(x->_idx) ) return;

  // Skip through this block
  const Node *p = x;
  do {
    p = p->in(0);               // Move control forward
    assert( !p->is_block_proj() || ((Node*)p)->is_Root(), "not a CFG" );
  } while( !p->is_block_start() );

  // Recursively visit
  for( uint i=1; i<p->req(); i++ )
    _dump_cfg(p->in(i),visited);

  // Dump the block
  _bbs[p->_idx]->dump(&_bbs);
}

void PhaseCFG::dump( ) const {
  tty->print("\n--- CFG --- %d BBs\n",_num_blocks);
  if( _blocks.size() ) {        // Did we do basic-block layout?
    for( uint i=0; i<_num_blocks; i++ )
      _blocks[i]->dump(&_bbs);
  } else {                      // Else do it with a DFS
    VectorSet visited(_bbs._arena);
    _dump_cfg(_root,visited);
  }
}

void PhaseCFG::dump_headers() {
  for( uint i = 0; i < _num_blocks; i++ ) {
    if( _blocks[i] == NULL ) continue;
    _blocks[i]->dump_head(&_bbs);
  }
}

void PhaseCFG::verify( ) const {
  // Verify sane CFG
  for( uint i = 0; i < _num_blocks; i++ ) {
    Block *b = _blocks[i];
    uint cnt = b->_nodes.size();
    uint j;
    for( j = 0; j < cnt; j++ ) {
      Node *n = b->_nodes[j];
      assert( _bbs[n->_idx] == b, "" );
      if( j >= 1 && n->is_Mach() &&
          n->is_Mach()->ideal_Opcode() == Op_CreateEx ) {
        assert( j == 1 || b->_nodes[j-1]->is_Phi(),
                "CreateEx must be first instruction in block" );
      }
      for( uint k = 0; k < n->req(); k++ ) {
        Node *use = n->in(k);
        if( use && use != n ) {
          assert( _bbs[use->_idx] || use->is_Con(), 
                  "must have block; constants for debug info ok" );
        }
      }
    }

    j = b->end_idx();
    Node *bp = (Node*)b->_nodes[b->_nodes.size()-1]->is_block_proj();
    assert( bp, "last instruction must be a block proj" );
    assert( bp == b->_nodes[j], "wrong number of successors for this block" );
    if( bp->is_Catch() ) {
      while( b->_nodes[--j]->Opcode() == Op_MachProj ) ;
      assert( b->_nodes[j]->is_Call(), "CatchProj must follow call" );
    }
    else if( bp->is_Mach() && bp->is_Mach()->ideal_Opcode() == Op_If ) {
      assert( b->_num_succs == 2, "Conditional branch must have two targets");
    }
  }
}
#endif

//=============================================================================
//------------------------------UnionFind--------------------------------------
UnionFind::UnionFind( uint max ) : _cnt(max), _max(max), _indices(NEW_RESOURCE_ARRAY(uint,max)) {
  Copy::zero_to_bytes( _indices, sizeof(uint)*max );
}

void UnionFind::extend( uint from_idx, uint to_idx ) {
  _nesting.check();
  if( from_idx >= _max ) {
    uint size = 16; 
    while( size <= from_idx ) size <<=1;
    _indices = REALLOC_RESOURCE_ARRAY( uint, _indices, _max, size );
    _max = size;
  }
  while( _cnt <= from_idx ) _indices[_cnt++] = 0;
  _indices[from_idx] = to_idx;
}

void UnionFind::reset( uint max ) {
  assert( max <= max_uint, "Must fit within uint" );
  // Force the Union-Find mapping to be at least this large
  extend(max,0);
  // Initialize to be the ID mapping.  
  for( uint i=0; i<_max; i++ ) map(i,i);
}

//------------------------------Find_compress----------------------------------
// Straight out of Tarjan's union-find algorithm
uint UnionFind::Find_compress( uint idx ) {
  uint cur  = idx;
  uint next = lookup(cur); 
  while( next != cur ) {        // Scan chain of equivalences
    assert( next < cur, "always union smaller" );
    cur = next;                 // until find a fixed-point
    next = lookup(cur);
  }
  // Core of union-find algorithm: update chain of
  // equivalences to be equal to the root.
  while( idx != next ) {
    uint tmp = lookup(idx);
    map(idx, next);
    idx = tmp;
  }
  return idx;
}

//------------------------------Find_const-------------------------------------
// Like Find above, but no path compress, so bad asymptotic behavior
uint UnionFind::Find_const( uint idx ) const {
  if( idx == 0 ) return idx;    // Ignore the zero idx
  // Off the end?  This can happen during debugging dumps 
  // when data structures have not finished being updated.
  if( idx >= _max ) return idx;
  uint next = lookup(idx); 
  while( next != idx ) {        // Scan chain of equivalences
    assert( next < idx, "always union smaller" );
    idx = next;                 // until find a fixed-point
    next = lookup(idx);
  }
  return next;
}

//------------------------------Union------------------------------------------
// union 2 sets together.
void UnionFind::Union( uint idx1, uint idx2 ) {
  uint src = Find(idx1);
  uint dst = Find(idx2);
  assert( src, "" );
  assert( dst, "" );
  assert( src < _max, "oob" );
  assert( dst < _max, "oob" );
  assert( src < dst, "always union smaller" );
  map(dst,src);
}
