#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIROopMapGenerator.hpp	1.7 03/12/23 16:39:14 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

// Iterates through the LIR and tracks motion of locals to and from
// the stack. This replaces the use of the ciLocalMapIterator as an
// intermediate step toward having the register allocator generate oop
// maps.

#ifndef PRODUCT

class LIR_OopMapGenerator : public CompilationResourceObj {
 private:
  IR* _ir;
  BlockBegin* _block;
  FrameMap* _frame_map;

  // Base (for special handling of ref-uninit conflicts)
  Base* _base;

  // Work list
  BlockList _work_list;

  // Result
  BitMap _oop_map;

  IR*           ir() const                       { return _ir; }

  void          set_block(BlockBegin* block)     { _block = block; }
  BlockBegin*   cur_block() const                { return _block;  }
  LocalMapping* local_mapping() const            { return cur_block()->local_mapping(); }
  BitMap*       oop_map()                        { return &_oop_map;      }
  FrameMap*     frame_map() const                { return _frame_map;     }

  void          iterate_one(BlockBegin* block);

  bool          is_caching_change_block(BlockBegin* block);

  bool          exception_handler_covers(CodeEmitInfo* info, BlockBegin* handler);

  // Merge the generator's current state into the given basic block.
  // If the block didn't have any state recorded for it yet, enqueue
  // it on the work list; otherwise, ensure that the states are equal.
  void          merge_state(BlockBegin* b);

  void          work_list_enqueue(BlockBegin* b);
  // Returns NULL when no more work to be done
  BlockBegin*   work_list_dequeue();

  // Mark a "slot" in the oop map (corresponding to a local's name) as
  // being an oop
  void          mark(int local_name);
  void          clear(int local_name);
  void          clear_all(int local_name);
  bool          is_marked(int local_name);

  // For handling caching of locals in conjunction with local names,
  // where the same cache register appears to be associated with
  // multiple locals. Once the new register allocator is in place and
  // we have liveness information, these routines will no longer be
  // needed.
  void          mark(LIR_Opr cache_reg);
  void          clear_all(LIR_Opr cache_reg);

  void          process_move(LIR_Op* op);

  bool          is_implicit_exception_info(CodeEmitInfo* info);
  bool          is_implicit_exception_bytecode(Bytecodes::Code code);

  NOT_PRODUCT(bool first_n_bits_same(BitMap* m1, BitMap* m2, int n);)
  NOT_PRODUCT(void print_oop_map(BitMap* map);)

  void process_info(CodeEmitInfo* info);

 public:
  LIR_OopMapGenerator(IR* ir, FrameMap* frame_map);

  void          generate();
};

#endif // PRODUCT
