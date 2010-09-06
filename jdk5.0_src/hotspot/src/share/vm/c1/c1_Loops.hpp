#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Loops.hpp	1.27 03/12/23 16:39:15 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class Loop;
define_array(LoopArray, Loop*)
define_stack(_LoopList, LoopArray)

class LoopList: public _LoopList {
 public:
  LoopList(): _LoopList() {}
  LoopList(const int size): _LoopList(size) {}

  void print(const char* msg = NULL) PRODUCT_RETURN;
  void update_loops(BlockBegin* from, BlockBegin* to, BlockBegin* insert);
};


class BlockLoopInfo: public CompilationResourceObj {
 private:
  static BitMap* _all_blocks;  // bitmap where all bits for all possible block id's are set

  BitMap      _doms_map;       // for each dominating block the id bit is set
  BlockBegin* _block;          // the block that is described by this BLI (BlockLoopInfo)
  bool        _backedge_start; // true if a backedge goes of from this node

  BlockList*  _preds; // list of predecessors for each block id

 public:
  BlockLoopInfo(BlockBegin* bb, int max_blocks);

  BlockBegin* block() const                  { return _block;          }

  // predecessors
  void add_predecessor(BlockBegin* pred)     { _preds->append(pred);    }
  int  nof_preds() const                     { return _preds->length(); }
  BlockBegin* pred_no(int i) const           { return _preds->at(i);    }

  // bitmaps
  BitMap doms_map() const                    { return _doms_map; }
  bool is_dom_block(int id) const            { return _doms_map.at(id); }

  // backedges
  void mark_backedge_start()                 { _backedge_start = true;  }
  bool is_backedge_start() const             { return _backedge_start;  }

  void print() PRODUCT_RETURN;

  static void set_all_blocks_map(BitMap* map) { _all_blocks = map;  }
  static BitMap* all_blocks_map()             { return _all_blocks; }
};


define_array(BlockLoopInfoArray, BlockLoopInfo*)
define_stack(BlockLoopInfoList, BlockLoopInfoArray)


class BlockPair: public CompilationResourceObj {
 private:
  BlockBegin* _from; 
  BlockBegin* _to;   
 public:
  BlockPair(BlockBegin* from, BlockBegin* to): _from(from), _to(to) {}
  BlockBegin* from() const { return _from; }
  BlockBegin* to() const   { return _to;   }
  bool is_same(BlockBegin* from, BlockBegin* to) const { return  _from == from && _to == to; }
  bool is_same(BlockPair* p) const { return  _from == p->from() && _to == p->to(); }
  void set_to(BlockBegin* b)   { _to = b; }
  void set_from(BlockBegin* b) { _from = b; }
};


define_array(BlockPairArray, BlockPair*)
define_stack(BlockPairList, BlockPairArray)


class Loop: public CompilationResourceObj {
 private:
  BlockBegin*                 _loop_start;  // header
  BlockList*                  _loop_ends;   // blocks which return to the header

  BlockList*                  _blocks; // all blocks contained inside the loop (including start and end nodes)
  BlockPairList*              _loop_entries; 
  BlockPairList*              _loop_exits;   

 public:
  Loop(BlockBegin* start, BlockBegin* end): _loop_start(start) {
    _loop_ends = new BlockList();
    _loop_ends->append(end);
    _blocks = new BlockList();
    _loop_entries = new BlockPairList();
    _loop_exits = new BlockPairList();
  }

  BlockBegin* start() const { return _loop_start; }
  BlockList*  ends() const  { return _loop_ends;  }

  bool is_end(BlockBegin* block) const   { return ends()->contains(block); }
  void add_end(BlockBegin* end)          { if (!is_end(end)) ends()->append(end); }

  void append_node(BlockBegin* b)  { _blocks->append(b); }

  int nof_blocks() const            { return _blocks->length(); }
  BlockBegin* block_no(int i) const { return _blocks->at(i);    }

  BlockList* blocks() const { return _blocks; }

  void append_loop_entry(BlockBegin* from, BlockBegin* to) { 
    _loop_entries->append(new BlockPair(from, to));
  }

  void append_loop_exit(BlockBegin* from, BlockBegin* to) { 
    _loop_exits->append(new BlockPair(from, to));
  }

  int nof_loopexits() const           { return _loop_exits->length(); }
  BlockPair* loopexit_no(int i) const { return _loop_exits->at(i);    }

  int nof_loopentries() const          { return _loop_entries->length(); }
  BlockPair* loopentry_no(int i) const { return _loop_entries->at(i);    }

  void update_loop_blocks(BlockBegin* old_from, BlockBegin* old_to, BlockBegin* insert);

  void sort_blocks();

  void blocks_do(void f(BlockBegin*)) {
    for (int n = nof_blocks() - 1; n >= 0; n--) {
      f(block_no(n));
    }
  }

  void iterate_blocks(BlockClosure* closure) {
    const int l = nof_blocks();
    for (int i = 0; i < l; i++) closure->block_do(block_no(i));
  }


#ifndef PRODUCT
  void print();
#endif
};


class LoopFinder: public StackObj {
 private:
  enum {
    max_nof_dom_iterations = 8
  };
  bool _ok;         // set to false if we encounter problems (loop analysis failed)
  bool _changed;    // used by iterative dominator algorithm
  bool _valid_doms; // true if computed dominators are valid
  int  _max_blocks; // maximum number of blocks
  IR*  _ir;
  LoopList* _loops;
  BlockLoopInfoList* _info;

  bool ok() const                       { return _ok;  }
  void set_not_ok()                     { _ok = false; }

  bool changed() const                  { return _changed; }
  void set_changed(bool f)              { _changed = f;    }

  IR* ir() const                        { return _ir; }

  void dominator_walk_sux(BlockBegin* bb, boolArray* visited);
  void compute_dominators(boolArray* visited);

  LoopList* find_backedges(boolArray* visited);
  LoopList* find_loops(LoopList* loops, bool call_free_only);

  void compute_loop_exits_and_entries(LoopList* loops);
  void gather_loop_blocks  (LoopList* loops);
  void insert_blocks       (LoopList* loops);
  void find_loop_exits     (BlockBegin* bb, Loop* loop);
  void find_loop_entries   (BlockBegin* bb, Loop* loop);
  
  BlockBegin* insert_caching_block(LoopList* loops, BlockBegin* b, BlockBegin* pred);
  void add_predecessor     (BlockBegin* b, BlockBegin* pred);

  BlockBegin* new_block(IRScope* scope, int bci);
  void compute_single_precision_flag(LoopList* loops);


  void print_loops(const char* msg, LoopList* loops);


 protected:
  friend class SetPredsClosure;
  friend class CreateInfoClosure;
  friend class PrintBlockDominators;

  int  max_blocks() const                             { return _max_blocks; }
  BlockLoopInfo* get_block_info(BlockBegin* bb) const { return _info->at(bb->block_id()); }
  BlockLoopInfoList* info() const                     { return _info; }

 public:
  LoopFinder(IR* ir, int max_block_id);

  LoopList* compute_loops(bool inner_only = true);

  void print() PRODUCT_RETURN;
};

