#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Loops.cpp	1.56 03/12/23 16:39:15 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_Loops.cpp.incl"


void LoopList::update_loops(BlockBegin* from, BlockBegin* to, BlockBegin* insert) {
  int n = length() - 1;
  for (; n >= 0; n--) {
    at(n)->update_loop_blocks(from, to, insert);
  }
}


#ifndef PRODUCT
void LoopList::print(const char* msg) {
  if (length() != 0) {
    if (msg) tty->print_cr(msg);
    for (int i = 0; i < length(); i++) {
      at(i)->print();
    }
  }
}
#endif

//-------------------BlockLoopInfo----------------------------

BitMap* BlockLoopInfo::_all_blocks = NULL;

BlockLoopInfo::BlockLoopInfo(BlockBegin* bb, int max_blocks): 
      _block(bb), _doms_map(max_blocks), _backedge_start(false) {
  if (_all_blocks != NULL) {
    _doms_map.set_from(*_all_blocks);
  }
  _preds = new BlockList();
}


#ifndef PRODUCT
void BlockLoopInfo::print() {
  tty->print_cr("BlockLoopInfo for %d",  _block->block_id());
  tty->print ("  Preds [");
  int i;
  for (i = 0; i < _preds->length(); i++) {
    tty->print(" B%d ", _preds->at(i)->block_id());
  }
  tty->print_cr("]");
  tty->print ("  Doms  [");
  for (size_t n = 0; n < _doms_map.size(); n++) {
    if (is_dom_block(n)) {
      tty->print(" B" SIZE_FORMAT " ", n);
    }
  }
  tty->print_cr("]");
}
#endif


//-------------------CheckDomClosure--------------------------

class CheckDomClosure: public BlockClosure {
private:
  boolArray* _visited;

public:
  CheckDomClosure(boolArray* visited) : _visited (visited) {}
  void block_do(BlockBegin* bb) {
    if (bb->is_set(BlockBegin::subroutine_entry_flag)) {
      // jsr destination
      assert(!_visited->at(bb->block_id()), "should not have visited this node");
    }
  }
};

//-------------------CreateInfoClosure--------------------------------
class CreateInfoClosure: public BlockClosure {
 private:
  LoopFinder* _lf;
 public:
  CreateInfoClosure(LoopFinder* lf): _lf(lf) {}
  void block_do(BlockBegin* bb) {
    assert(_lf->info()->at(bb->block_id()) == NULL, "already allocated?");
    BlockLoopInfo* bli = new BlockLoopInfo(bb, _lf->max_blocks());
    _lf->info()->at_put(bb->block_id(), bli);
  }
};

//-------------------SetPredsClosure----------------------------------


class SetPredsClosure: public BlockClosure {
 private:
  LoopFinder* _lf;
 public:
  SetPredsClosure(LoopFinder* lf) : _lf(lf) {}
  void block_do(BlockBegin* bb) {
    BlockEnd* be = bb->end();
    int n = be->number_of_sux();
    for (int i = 0; i < n; i++) {
      BlockBegin* sux = be->sux_at(i);
      BlockLoopInfo* sux_bli = _lf->get_block_info(sux);
      sux_bli->add_predecessor(bb);
    }
  }
};

//-------------------PrintBlockDominators--------------------------------

#ifndef PRODUCT
class PrintBlockDominators: public BlockClosure {
 private:
  LoopFinder* _lf;
 public:
  PrintBlockDominators(LoopFinder* lf): _lf(lf) {}
  void block_do(BlockBegin* bb) {
    assert(_lf->info()->at(bb->block_id()) != NULL, "already allocated?");
    BlockLoopInfo* bli = _lf->info()->at(bb->block_id());
    tty->print (" B%d doms [", bb->block_id());
    for (int i = 0; i < _lf->max_blocks(); i++) {
      if (bli->is_dom_block(i)) {
        tty->print(" B%d ", i);
      }
    }
    tty->print_cr("]");
  }
};
#endif

//-------------------PairCollector--------------------------------

class PairCollector: public BlockClosure {
 private:
  BlockPairList* _pairs;
  intStack*      _tags;

 public:
  PairCollector(intStack* tags): _pairs(new BlockPairList()), _tags(tags) {}

  BlockPairList* pairs() const { return _pairs; }

  void block_do(BlockBegin* from) {
    int n = from->end()->number_of_sux();
    int tag = _tags->at(from->block_id());
    for (int i = 0; i < n; i++) {
      BlockBegin* to = from->end()->sux_at(i);
      if (tag != _tags->at(to->block_id())) {
        // this edge is a transition between two different
        // caching regions, so we need to insert a CachingChange
        _pairs->append(new BlockPair(from, to));
      }
    }
  }
};

//-------------------LoopFinder---------------------------------------


// The CFG has one entry node that is predecessor of all method-entries: 
// - standard method entry
// - exception handlers
// JSR-entries are not analyzed in LoopFinder; however, we have to check that no 
// block that has not been analyzed jumps into or is jumped from an analyzed block.

LoopFinder::LoopFinder(IR* ir, int max_block_id)
: _ir(ir), _max_blocks(max_block_id), _ok(true), _changed(false), _loops(NULL)
{
  _info = new BlockLoopInfoList(max_block_id, NULL);
  _changed = false;
  _valid_doms = false;
}


// Compute dominators for bb and 
// walk the successors of bb in depth first order
void LoopFinder::dominator_walk_sux(BlockBegin* bb, boolArray* visited) {
  // we may not visit a block that is jsr-target
  if (bb->is_set(BlockBegin::subroutine_entry_flag)) set_not_ok();
  BlockEnd*      be  = bb->end();
  BlockLoopInfo* bli = get_block_info(bb);
  visited->at_put(bb->block_id(), true);

  // compute new dominators using predecessors
  BitMap map(max_blocks());
  map.set_from(*BlockLoopInfo::all_blocks_map());
  { // Compute dominators for myself (looking at predecessors)
    int nof_preds = bli->nof_preds();
    for (int i = 0; i < nof_preds; i++) {
      BlockBegin* pred     = bli->pred_no(i);
      BitMap      pred_map = get_block_info(pred)->doms_map();
      map.set_intersection(pred_map);
    }
    // add itself
    map.at_put(bb->block_id(), true);
    
    // if the computed dominators differ from the one stored,
    // then we need another iteration
    BitMap bb_map = bli->doms_map();
    if (!bb_map.is_same(map)) {
      set_changed(true);
      bb_map.set_from(map);
    }
  }
  { // Visit all successors
    int n = be->number_of_sux();
    for (int i = 0; i < n; i++) {
      BlockBegin* sux = be->sux_at(i);
      if (!visited->at(sux->block_id())) {
        dominator_walk_sux(sux, visited);
      }
    }
  }
}



//-------------------------Loop-----------------------------------------------

static int cmp(BlockBegin** a, BlockBegin** b)   { return (*a)->weight() - (*b)->weight(); }

void Loop::sort_blocks() {
  _blocks->sort(cmp);
}

// Factor out visiting of all entries/exits
void Loop::update_loop_blocks(BlockBegin* old_from, BlockBegin* old_to, BlockBegin* insert) {
  int n = _loop_entries->length() - 1;
  for (; n >= 0; n--) {
    BlockPair* bp = _loop_entries->at(n);
    if (bp->is_same(old_from, old_to)) {
      bp->set_from(insert);
    }
  }
  n  = _loop_exits->length() - 1;
  for (; n >= 0; n--) {
    BlockPair* bp = _loop_exits->at(n);
    if (bp->is_same(old_from, old_to)) {
      bp->set_to(insert);
    }
  }
}


#ifndef PRODUCT
void Loop::print() {
  int i;
  tty->print("!Loop header B%d (%d), backedge%s ", start()->block_id(), start()->bci(),
             (ends()->length() > 1) ? "s" : "");
  for (i = 0; i < ends()->length(); i++) {
    tty->print(" B%d (%d)", ends()->at(i)->block_id(), ends()->at(i)->bci());
  }
  tty->cr();
  tty->print("   blocks[");
  if (Verbose) {
    for (i = 0; i < _blocks->length(); i++) {
      tty->print(" B%d", _blocks->at(i)->block_id());
    }
    tty->print_cr("]");
    for (i = 0; i < _loop_entries->length(); i++) {
      tty->print_cr("   entry [B%d -> B%d]", _loop_entries->at(i)->from()->block_id(), _loop_entries->at(i)->to()->block_id());
    }
    for (i = 0; i < _loop_exits->length(); i++) {
      tty->print_cr("   exit  [B%d -> B%d]", _loop_exits->at(i)->from()->block_id(), _loop_exits->at(i)->to()->block_id());
    }
  } else {
    tty->print_cr("]");
  }
}
#endif


void LoopFinder::compute_dominators(boolArray* visited) {
  // set up a bitmap that contains all blocks
  BitMap all_map(max_blocks());
  all_map.clear();
  for (int i = 0; i < max_blocks(); i++) all_map.at_put(i, true);
  BlockLoopInfo::set_all_blocks_map(&all_map);
  { // initialize block loop info and set all predecessors
    CreateInfoClosure c(this);
    ir()->iterate_preorder(&c);
    SetPredsClosure s(this);
    ir()->iterate_preorder(&s);
  }
  { // compute dominators
    // init dominators
    // set entry block (only one exists) and its dominators
    BlockBegin* root_bb = ir()->start();
    assert(!root_bb->is_set(BlockBegin::subroutine_entry_flag), "root may not be jsr target");

    { // initialize root dominators (only itself)
      BitMap root_doms(max_blocks());
      root_doms.clear();
      root_doms.at_put(root_bb->block_id(), true);
      BlockLoopInfo* bli = get_block_info(root_bb);
      bli->doms_map().set_from(root_doms);
    }

    // iterate until either iter_count exceeds or dominators stable
    int iter_count = 0;
    do {
      iter_count++;
      visited->at_put(root_bb->block_id(), true);
      set_changed(false);
      BlockEnd* be = root_bb->end();
      int n = be->number_of_sux();
      for (int i = 0; i < n; i++) {
        BlockBegin* sux = be->sux_at(i);
        if (!visited->at(sux->block_id())) {
          dominator_walk_sux(sux, visited);
        }
      }
      if (changed()) {
        for (int i = visited->length() - 1; i >= 0; i--) {
          visited->at_put(i, false);
        }
      }
    } while (changed() && iter_count <= max_nof_dom_iterations);
    if (iter_count == max_nof_dom_iterations) {
      if (PrintLoops) {
        tty->print_cr("could not computer dominators");
      }
      set_not_ok();
    }

    // if (PrintLoops) {
    //   tty->print_cr("  Dominators: %d iterations", iter_count);
    //   PrintBlockDominators p(this);
    //   ir()->iterate_topological(&p);
    // }
  }  
  BlockLoopInfo::set_all_blocks_map(NULL);


  // go through all blocks; if a block has not been analyzed, then check its
  // predecessors and successors: all must be also un-analyzed;
  // Note: the block may not be JSR blocks
  if (ok()) {
    _valid_doms = true;
#ifdef ASSERT
    CheckDomClosure cc(visited);
    ir()->iterate_preorder(&cc);
#endif
  }

}


//-------------------------Loop-----------------------------------------------

static int sort_by_start_block(Loop** a, Loop** b)   { return (*a)->start()->block_id() - (*b)->start()->block_id(); }

// Gather backedges of natural loops: an edge a -> b where b dominates a
LoopList* LoopFinder::find_backedges(boolArray* visited) {
  int i;
  LoopList* backedges = new LoopList();
  for (i = 0; i < max_blocks(); i++) {
    if (visited->at(i)) {
      BlockLoopInfo* bli = _info->at(i);
      BlockBegin*    bb  = bli->block();
      BlockEnd*      be  = bb->end();
      int n = be->number_of_sux();
      for (int i = 0; i < n; i++) {
        BlockBegin* sux = be->sux_at(i);
        if (bli->is_dom_block(sux->block_id())) {
          bli->mark_backedge_start();
          backedges->push(new Loop(sux, bb));
        }
      }
    }
  }

  // backedges contains single pairs of blocks which are a backedge.
  // some of these loops may share entry points, so walk over the backedges
  // and merge loops which have the same entry point
  if (backedges->length() > 1) {
    backedges->sort(sort_by_start_block);
    Loop* current_loop = backedges->at(0);
    for (i = 1; i < backedges->length();) {
      Loop* this_loop = backedges->at(i);
      if (current_loop->start() == this_loop->start()) {
        // same entry point
        assert(this_loop->ends()->length() == 1, "should only have one end at this point");
#ifndef PRODUCT
        if (PrintLoops && Verbose) {
          tty->print_cr("Merging loops with same start");
          current_loop->print();
          this_loop->print();
        }
#endif
        BlockBegin* e = this_loop->ends()->at(0);
        current_loop->add_end(e);
        backedges->remove(this_loop);
      } else {
        // start processing the next loop entry point
        i++;
      }
    }
  }

  return backedges;
}


void LoopFinder::gather_loop_blocks(LoopList* loops) {
  int lng = loops->length();
  BitMap blocks_in_loop(max_blocks());
  for (int i = 0; i < lng; i++) {
    // for each loop do the following
    blocks_in_loop.clear();
    Loop* loop = loops->at(i);
    BlockList* ends = loop->ends();
    if (!loop->is_end(loop->start())) {
      GrowableArray<BlockBegin*>* stack = new GrowableArray<BlockBegin*>();
      blocks_in_loop.at_put(loop->start()->block_id(), true);
      
      // insert all the ends into the list
      for (int i = 0; i < ends->length(); i++) {
        blocks_in_loop.at_put(ends->at(i)->block_id()  , true);
        stack->push(ends->at(i));
      }
      
      while (!stack->is_empty()) {
        BlockBegin* bb = stack->pop();
        BlockLoopInfo* bli = get_block_info(bb);
        // push all predecessors that are not yet in loop
        int npreds = bli->nof_preds();
        for (int m = 0; m < npreds; m++) {
          BlockBegin* pred = bli->pred_no(m);
          if (!blocks_in_loop.at(pred->block_id())) {
            blocks_in_loop.at_put(pred->block_id(), true);
            loop->append_node(pred);
            stack->push(pred);
          }
        }
      }
      loop->append_node(loop->start());
    }
    // insert all the ends into the loop
    for (int i = 0; i < ends->length(); i++) {
      loop->append_node(ends->at(i));
    }
  }
}


void LoopFinder::find_loop_entries(BlockBegin* bb, Loop* loop) {
  BlockLoopInfo* bli = get_block_info(bb);
  int loop_index = bb->loop_index();
  // search all predecessors and locate the ones that do not have the same loop_index
  int n = bli->nof_preds() - 1;
  assert(bli->nof_preds() >= 2, "at least two predecessors required for a loop header");
  for (; n >= 0; n--) {
    BlockBegin* pred_bb = bli->pred_no(n);
    BlockLoopInfo* pred_bli = get_block_info(pred_bb);
    if (pred_bb->loop_index() != loop_index) {
      loop->append_loop_entry(pred_bb, bb);
    }
  }
  assert(loop->nof_loopentries() > 0, "a loop must be entered from somewhere");
}


void LoopFinder::find_loop_exits(BlockBegin* bb, Loop* loop) {
  BlockLoopInfo* bli = get_block_info(bb);
  int loop_index = bb->loop_index();
  // search all successors and locate the ones that do not have the same loop_index
  BlockEnd* be = bb->end();
  int n = be->number_of_sux() - 1;
  for(; n >= 0; n--) {
    BlockBegin* sux = be->sux_at(n);
    BlockLoopInfo* sux_bli = get_block_info(sux);
    if (sux->loop_index() != loop_index) {
      loop->append_loop_exit(bb, sux);
    }
  }
}



void LoopFinder::compute_loop_exits_and_entries(LoopList* loops) {
  // create loop exits for each loop
  int loop_index = loops->length() - 1;
  for (; loop_index >= 0; loop_index--) {
    Loop* loop = loops->at(loop_index);
    int n = loop->nof_blocks() - 1;
    // mark all nodes belonging to this loop 
    for (; n >= 0; n --) {
      BlockBegin* bb = loop->block_no(n);
      bb->set_loop_index(loop_index);
    }
    find_loop_entries(loop->start(), loop);
    // search for blocks that have successors outside the loop
    n = loop->nof_blocks() - 1;
    for (; n >= 0; n--) {
      BlockBegin* bb = loop->block_no(n);
      find_loop_exits(bb, loop);
    }
  }
}


// Returns inner loops that have may or may not have calls
LoopList* LoopFinder::find_loops(LoopList* loops, bool call_free_only) {
  LoopList* inner = new LoopList();
  LoopList* outer = new LoopList();
  int lng = loops->length();
  // First step: find loops that have no calls and no backedges
  //             in the loop except its own
  int i;
  for (i = 0; i < lng; i++) {
    Loop* loop = loops->at(i);
    int k = loop->nof_blocks() - 1;
    bool is_outer = false;
    for (; k >= 0; k--) {
      BlockBegin* b = loop->block_no(k);
      // Is this innermost loop:
      // - no block, except end block, may be a back edge start,
      //   otherwise we have an outer loop
      if (!loop->is_end(b)) {
        BlockLoopInfo* bli = get_block_info(b);
        if (bli->is_backedge_start()) {
          if (WantsLargerLoops) {
            is_outer = true;
          } else {
            loop = NULL;
          }
          break;
        }
      }
    }

    if (loop != NULL) {
      ScanBlocks scan(loop->blocks());
      ScanResult scan_result;
      scan.scan(&scan_result);
      if (!call_free_only || (!scan_result.has_calls() && !scan_result.has_slow_cases())) {
        if (is_outer) {
          outer->append(loop);
        } else {
          inner->append(loop);
        }
      } else {
#ifndef PRODUCT
        if (PrintLoops && Verbose) {
          tty->print("Discarding loop with calls: ");
          loop->print();
        }
#endif // PRODUCT
      }
    }
  }
  // find all surviving outer loops and delete any inner loops contained inside them
  if (inner->length() > 0) {
    for (i = 0; i < outer->length() ; i++) {
      Loop* loop = outer->at(i);
      int k = loop->nof_blocks() - 1;
      for (; k >= 0; k--) {
        BlockBegin* b = loop->block_no(k);
        if (!loop->is_end(b)) {
          BlockLoopInfo* bli = get_block_info(b);
          if (bli->is_backedge_start()) {
            // find the loop contained inside this loop
            int j;
            for (j = 0; j < inner->length(); j++) {
              Loop* inner_loop = inner->at(j);
              if (inner_loop->is_end(b)) {
                inner->remove(inner_loop);
                break;
              }
            }
            for (j = 0; j < outer->length(); j++) {
              Loop* outer_loop = outer->at(j);
              if (outer_loop == loop) {
                continue;
              }

              if (outer_loop->is_end(b)) {
                outer->remove(outer_loop);
                break;
              }
            }
          }
        }
      }
    }
    inner->appendAll(outer);
  }


  // Second step: if several loops have the same loop-end, select the one
  //              with fewer blocks.
  //              if several loops have the same loop-start, select the one
  //              with fewer blocks
  // now check for loops that have the same header and eliminate one
  for (i = 0; i < inner->length() ; i++) {
    Loop* current_loop = inner->at(i);
    BlockBegin* header = current_loop->start();
    for (int n = i + 1; n < inner->length(); n++) {
      Loop*  test_loop = inner->at(n);
      BlockBegin* test = test_loop->start();
      Loop* discarded = NULL;
      bool same_end = false;
      for (int e = 0; e < current_loop->ends()->length(); e++) {
        if (test_loop->is_end(current_loop->ends()->at(e))) {
          same_end = true;
        }
      }
      if (header == test_loop->start() || same_end) {
        // discard loop with fewer blocks
        if (test_loop->nof_blocks() > current_loop->nof_blocks()) {
          if (WantsLargerLoops) {
            discarded = current_loop;
          } else {
            discarded = test_loop;
          }
        } else {
          if (WantsLargerLoops) {
            discarded = test_loop;
          } else {
            discarded = current_loop;
          }
        }
        inner->remove(discarded);
#ifndef PRODUCT
        if (PrintLoops && Verbose && discarded) {
          tty->print("Discarding overlapping loop: ");
          discarded->print();
        }
#endif // PRODUCT
        // restart the computation
        i = -1;
        break;
      }
    }
  }

  if (inner->length() == 0) {
    // we removed all the loops
    if (PrintLoops && Verbose) {
      tty->print_cr("*** deleted all loops in %s", __FILE__);
    }
    set_not_ok();
    return NULL;
  } else {
    return inner;
  }
}



BlockBegin* LoopFinder::new_block(IRScope* scope, int bci) {
  BlockBegin* b = new BlockBegin(bci);
  _valid_doms = false;
  assert(b->block_id() == max_blocks(), "illegal block_id");
  _max_blocks++;
  BlockLoopInfo* bli = new BlockLoopInfo(b, max_blocks());
  _info->append(bli);
  assert(_info->length() == max_blocks(), "operation failed");
  return b;
}



void LoopFinder::add_predecessor(BlockBegin* b, BlockBegin* pred) {
  BlockLoopInfo* bli = get_block_info(b);
  bli->add_predecessor(pred);
}



BlockBegin* LoopFinder::insert_caching_block(LoopList* loops, BlockBegin* from, BlockBegin* to) {
  if (from->next() && from->next()->as_CachingChange() != NULL &&
      from->end()->default_sux() == to) {
    // we already have a caching change block
    // check that the precision flags are the same
#ifdef ASSERT
    CachingChange* cc = from->next()->as_CachingChange();
    assert(cc->pred_block()->is_set(BlockBegin::single_precision_flag) == from->is_set(BlockBegin::single_precision_flag), "consistency check");
    assert(cc->sux_block()->is_set(BlockBegin::single_precision_flag) == to->is_set(BlockBegin::single_precision_flag), "consistency check");
#endif
    return NULL;
  } else {
    // insert a caching change block, making it close to any single successor
    int bci = -1;
    BlockLoopInfo* bli = get_block_info(to);
    if (bli->nof_preds() == 1) {
      bci = to->bci();
    } else {
      bci = from->end()->bci();
    }
    BlockBegin* cc = new_block(to->scope(), bci);
    BlockEnd* e = new Goto(to, false);
    cc->set_end(e);
    cc->set_next(new CachingChange(from, to), bci)->set_next(e, bci);
    if (PrintLoops && Verbose) {
      tty->print_cr("Added caching block B%d (dest B%d)", cc->block_id(), to->block_id());
    }
    BlockEnd* from_end = from->end();
    from_end->substitute_sux(to, cc);
    cc->join(from_end->state());
    assert(cc->state() != NULL, "illegal operation");

    ValueStack* end_state = cc->state()->copy();
    cc->end()->set_state(end_state);
    to->join(end_state);

    assert(cc->end()->state() != NULL, "should have state");

    loops->update_loops(from, to, cc);
    return cc;
  }
}


class Tagger: public BlockClosure {
 private:
  intStack* _tags;
  int       _tag;

 public:
  Tagger(intStack* tags, int tag) : _tags(tags), _tag(tag) {}

  virtual void block_do(BlockBegin* block) {
    assert(_tags->at_grow(block->block_id(), -1) == -1 || _tags->at_grow(block->block_id(), -1) == _tag, "this block is part of two loops");
    _tags->at_put_grow(block->block_id(), _tag, -1);
  }
  
};

static int sort_by_block_ids(BlockPair** a, BlockPair** b) {
  if ((*a)->from()->block_id() != (*b)->from()->block_id()) {
    return ((*a)->from()->block_id() - (*b)->from()->block_id());
  } else {
    return ((*a)->to()->block_id() - (*b)->to()->block_id());
  }
}


// Add preheader and loop exit blocks; after this pass,
// dominators are not valid anymore
void LoopFinder::insert_blocks(LoopList* loops) {
  if (LIRCacheLocals) {
    intStack* tags = new intStack(max_blocks(), -1);
    int i;

    for (i = 0; i < loops->length(); i++) {
      Tagger t(tags, i);
      loops->at(i)->blocks()->iterate_forward(&t);
    }

    PairCollector pc(tags);
    ir()->iterate_preorder(&pc);

    BlockPairList* pairs = pc.pairs();

    pairs->sort(sort_by_block_ids);

    BlockPair* last_pair = NULL;
    for (i = 0; i < pairs->length(); i++) {
      BlockPair* p = pairs->at(i);
      if (last_pair != NULL && p->is_same(last_pair)) {
        // last_pair is not null and p is same as last_pair -> skip inserting of blocks
      } else {
        insert_caching_block(loops, p->from(), p->to());
        last_pair = p;
      }
    }
    
  }
}


class FlagSetter: public BlockClosure {
 private:
  BlockBegin::Flag _flags;
 public:
  FlagSetter(BlockBegin::Flag flags) : _flags(flags) {}

  virtual void block_do(BlockBegin* block) {
    block->set(_flags);
  }
};



// Sets single precision flag for loop blocks where loops

void LoopFinder::compute_single_precision_flag(LoopList* loops) {
  NEEDS_CLEANUP; // factor out the loop iterator
  // create loop exits for each loop
  int loop_index = loops->length() - 1;
  for (; loop_index >= 0; loop_index--) {
    Loop* loop = loops->at(loop_index);
    ScanBlocks scan(loop->blocks());
    ScanResult scan_result;
    scan.scan(&scan_result);
    if (!scan_result.has_calls()
     && !scan_result.has_slow_cases()
     && !scan_result.has_class_init()
     && !scan_result.has_doubles() 
     &&  scan_result.has_floats()) {
        FlagSetter fs(BlockBegin::single_precision_flag);
        loop->blocks()->iterate_forward(&fs);
    }
  }
}

// Note that OSR compilation prevents the OSR loop from being detected
// as the loop is being jumped in the middle and not at loop entry

// returns an array of call-free inner-loops
LoopList* LoopFinder::compute_loops(bool call_free_only) {
  // visited has flags set for each block that has been visited
  boolArray visited (max_blocks(), false);
  if (ok()) compute_dominators(&visited);

  if (ir()->method()->has_exception_handlers()) set_not_ok();

  LoopList* all_loops = NULL;
  if (ok()) all_loops = find_backedges(&visited);
  if (ok())             gather_loop_blocks(all_loops);
  if (ok())    _loops = find_loops(all_loops, call_free_only);
  if (ok())             compute_loop_exits_and_entries(_loops);

  if (ok() && OptimizeSinglePrecision) {
    compute_single_precision_flag(_loops);
  }
  // asserts are using the precision computed before
  if (ok())             insert_blocks(_loops);
  if (ok() && PrintLoops && Verbose) {
    all_loops->print("+All Loops:");
    _loops->print("+Inner Loops:");
    ir()->print(false);
  }
  return _loops;
}



