#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIROopMapGenerator.cpp	1.9 03/12/23 16:39:13 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_c1_LIROopMapGenerator.cpp.incl"

#ifndef PRODUCT

LIR_OopMapGenerator::LIR_OopMapGenerator(IR* ir,
                                         FrameMap* frame_map)
  : _ir(ir)
  , _block(NULL)
  , _frame_map(frame_map)
  , _oop_map(frame_map->num_local_names())
{
}


void LIR_OopMapGenerator::generate() {
  // Initialize by iterating down the method's signature and marking
  // oop locals in the state

  assert((int) oop_map()->size() == frame_map()->num_local_names(), "just checking");
  oop_map()->clear();
  ciSignature* sig = ir()->method()->signature();
  int idx = 0;
  // Handle receiver for non-static methods
  if (!ir()->method()->is_static()) {
    mark(frame_map()->name_for_argument(idx));
    ++idx;
  }
  for (int i = 0; i < sig->count(); i++) {
    ciType* type = sig->type_at(i);
    if (!type->is_primitive_type()) {
      mark(frame_map()->name_for_argument(idx));
    }
    idx += type->size();
  }

  // The start block contains a Base instruction, which causes the LIR
  // for ref-uninit conflicts to be generated. We need to handle this
  // block specially so we don't traverse its "osr_entry" successor,
  // because we don't know how to set up the state appropriately for
  // that entry point.
  _base = ir()->start()->end()->as_Base();

  // Always start iterating at the start (even for OSR compiles)
  merge_state(ir()->start());
  
  BlockBegin* block = work_list_dequeue();
  while (block != NULL) {
    oop_map()->set_from(*block->lir_oop_map());
    iterate_one(block);
    block = work_list_dequeue();
  }
}


void LIR_OopMapGenerator::iterate_one(BlockBegin* block) {
#ifndef PRODUCT
  if (TraceLIROopMaps) {
    tty->print_cr("Iterating through block %d", block->block_id());
  }
#endif

  set_block(block);
  block->set(BlockBegin::lir_oop_map_gen_reachable_flag);

  int i;

  if (!is_caching_change_block(block)) {
    LIR_OpVisitState state;
    LIR_OpList* inst = block->lir()->instructions_list();
    int length = inst->length();
    for (i = 0; i < length; i++) {
      LIR_Op* op = inst->at(i);
      LIR_Code code = op->code();

      state.visit(op);
      for (int j = 0; j < state.info_count(); j++) {
        process_info(state.info_at(j));
      }

      if (code == lir_volatile_move ||
          code == lir_move) {
        process_move(op);
      }
    }
  }

  // Process successors
  if (block->end() != _base) {
    for (i = 0; i < block->end()->number_of_sux(); i++) {
      merge_state(block->end()->sux_at(i));
    }
  } else {
    // Do not traverse OSR entry point of the base
    merge_state(_base->std_entry());
  }

  set_block(NULL);
}


bool LIR_OopMapGenerator::is_caching_change_block(BlockBegin* block) {
  // CachingChange blocks must be ignored as they contain
  // incorrectly-typed moves
  return (block->next() != NULL) && (block->next()->as_CachingChange() != NULL);
}


bool LIR_OopMapGenerator::exception_handler_covers(CodeEmitInfo* info,
                                                   BlockBegin* handler)
{
  int bci = info->bci();
  for (IRScope* scope = info->scope(); scope != NULL; bci = scope->caller_bci(), scope = scope->caller()) {
    if (scope == handler->scope()) {
      XHandlers* xh = scope->xhandlers();
      for (int i = 0; i < xh->number_of_handlers(); i++) {
        if (xh->handler_at(i)->handler_bci() == handler->bci()) {
          if (xh->handler_at(i)->covers(bci)) {
#ifndef PRODUCT
            if (TraceLIROopMaps) {
              tty->print_cr("    Found exception handler: block %d", handler->block_id());
            }
#endif
            return true;
          }
        }
      }
    }
  }
#ifndef PRODUCT
  if (TraceLIROopMaps) {
    tty->print_cr("  No exception handler found");
  }
#endif
  return false;
}


void LIR_OopMapGenerator::merge_state(BlockBegin* b) {
  // Even though ref-val and other conflicts have been rewritten, we
  // can still have cases where local slots (even within a given
  // scope) can look like ref-val and ref-uninit situations as long as
  // that slot is not referenced later. In the new register allocator
  // we will necessarily have liveness information, so the iteration
  // can be removed and the oop map generation can become a one-pass
  // operation.


#ifndef PRODUCT
  if (TraceLIROopMaps) {
    tty->print_cr("Merging with Block %d", b->block_id());
  }
#endif


  bool must_iterate = false;
  if (b->lir_oop_map()->size() != oop_map()->size()) {
    b->lir_oop_map()->resize(oop_map()->size());
    b->lir_oop_map()->set_from(*oop_map());
    must_iterate = true;
  } else {
    must_iterate = b->lir_oop_map()->set_intersection_with_result(*oop_map());
  }


#ifndef PRODUCT
  if (TraceLIROopMaps && must_iterate) {
    tty->print_cr("Block %d's state changed. New state:", b->block_id());
    print_oop_map(b->lir_oop_map());
    tty->cr();
  }
#endif


  if (must_iterate) {
    work_list_enqueue(b);
  }
}


void LIR_OopMapGenerator::work_list_enqueue(BlockBegin* b) {
  if (!b->enqueued_for_oop_map_gen()) {
    _work_list.push(b);
    b->set_enqueued_for_oop_map_gen(true);
  }
}


BlockBegin* LIR_OopMapGenerator::work_list_dequeue() {
  if (_work_list.is_empty()) {
    return NULL;
  }
  BlockBegin* b = _work_list.pop();
  b->set_enqueued_for_oop_map_gen(false);
  return b;
}


void LIR_OopMapGenerator::mark(int local_index) {
  oop_map()->at_put(local_index, true);
}


void LIR_OopMapGenerator::clear(int local_name) {
  oop_map()->at_put(local_name, false);
}


void LIR_OopMapGenerator::clear_all(int local_name) {
  // Have to clear out all slots corresponding to this local name
  WordSizeList* name_to_offset_map = ir()->local_name_to_offset_map();
  WordSize offset = name_to_offset_map->at(local_name);
  for (int i = 0; i < name_to_offset_map->length(); i++) {
    WordSize word = name_to_offset_map->at(i);
    if (word == offset) {
      clear(i);
    }
  }
}


bool LIR_OopMapGenerator::is_marked(int local_index) {
  return oop_map()->at(local_index);
}


void LIR_OopMapGenerator::mark(LIR_Opr cache_reg) {
  // Note that we don't know the "oop name" for this caching register
  // so we pick an arbitrary one
  for (int i = local_mapping()->local_names_begin();
       i < local_mapping()->local_names_end(); i++) {
    if (local_mapping()->is_local_name_cached_in_reg(i, cache_reg)) {
      mark(i);
      return;
    }
  }
  assert(false, "Should not reach here");
}


void LIR_OopMapGenerator::clear_all(LIR_Opr cache_reg) {
  for (int i = local_mapping()->local_names_begin();
       i < local_mapping()->local_names_end(); i++) {
    if (local_mapping()->is_local_name_cached_in_reg(i, cache_reg)) {
      clear(i);
      if (cache_reg->is_double_word()) {
        clear(1 + i);
      }
    }
  }
}


void LIR_OopMapGenerator::process_move(LIR_Op* op) {
  LIR_Op1* op1 = op->as_Op1();
  LIR_Opr src = op1->in_opr();
  LIR_Opr dst = op1->result_opr();
  
  assert(!src->is_stack() || !dst->is_stack(), "No memory-memory moves allowed");
  if ((src->is_stack() && frame_map()->is_spill_pos(src)) ||
      (dst->is_stack() && frame_map()->is_spill_pos(dst))) {
    // Oops in the spill area are handled by another mechanism (see
    // CodeEmitInfo::record_spilled_oops)
    return;
  }
  if (dst->is_oop()) {
    assert((src->is_oop() &&
            (src->is_stack() || src->is_register() || src->is_constant())
           ) ||
           src->is_address(), "Wrong tracking of oops/non-oops in LIR");
    assert(!src->is_stack() || is_marked(src->single_stack_ix()),
           "Error in tracking of oop stores to stack");
    if (dst->is_stack()) {
      mark(dst->single_stack_ix());
    } else if (dst->is_register()) {
      if (LIRCacheLocals) {
        if (local_mapping()->is_cache_reg(dst)) {
          mark(dst);
        }
      } else {
        assert(local_mapping() == NULL, "expected no local mapping");
      }
    }
  } else {
    // !dst->is_oop()
    // Note that dst may be an address
    assert(!src->is_single_stack() || !is_marked(src->single_stack_ix()), "Error in tracking of oop stores to stack");
    assert(!src->is_double_stack() || !is_marked(src->double_stack_ix()), "Error in tracking of oop stores to stack");
    assert(!src->is_double_stack() || !is_marked(1 + src->double_stack_ix()), "Error in tracking of oop stores to stack");
    if (dst->is_stack()) {
      if (dst->is_single_stack()) {
        clear_all(dst->single_stack_ix());
      } else {
        clear_all(dst->double_stack_ix());
        clear_all(1 + dst->double_stack_ix());
      }
    } else if (dst->is_register()) {
      if (LIRCacheLocals) {
        if (local_mapping()->is_cache_reg(dst)) {
          clear_all(dst);
        }
      } else {
        assert(local_mapping() == NULL, "expected no local mapping");
      }
    }
  }
}


bool LIR_OopMapGenerator::is_implicit_exception_info(CodeEmitInfo* info) {
  if (info->bci() < 0)
    return false;
  return is_implicit_exception_bytecode(info->scope()->method()->java_code_at_bci(info->bci()));
}


bool LIR_OopMapGenerator::is_implicit_exception_bytecode(Bytecodes::Code code) {
  // See ciOopMap.cpp and usage of implicit exceptions in CodeGenerator
  switch (code) {
    case Bytecodes::_iaload: return true;
    case Bytecodes::_laload: return true;
    case Bytecodes::_faload: return true;
    case Bytecodes::_daload: return true;
    case Bytecodes::_aaload: return true;
    case Bytecodes::_baload: return true;
    case Bytecodes::_caload: return true;
    case Bytecodes::_saload: return true;
    case Bytecodes::_iastore: return true;
    case Bytecodes::_lastore: return true;
    case Bytecodes::_fastore: return true;
    case Bytecodes::_dastore: return true;
    case Bytecodes::_bastore: return true;
    case Bytecodes::_castore: return true;
    case Bytecodes::_sastore: return true;
    case Bytecodes::_idiv: return true;
    case Bytecodes::_ldiv: return true;
    case Bytecodes::_irem: return true;
    case Bytecodes::_lrem: return true;
    case Bytecodes::_arraylength: return true;
    case Bytecodes::_aastore: return true;
    default: return false;
  }
}


#ifndef PRODUCT
bool LIR_OopMapGenerator::first_n_bits_same(BitMap* m1, BitMap* m2, int n) {
  for (int i = 0; i < n; i++) {
    if (m1->at(i) != m2->at(i)) return false;
  }
  return true;
}

void LIR_OopMapGenerator::print_oop_map(BitMap* map) {
  for (unsigned int i = 0; i < map->size(); i++) {
    if (map->at(i)) tty->print(" Name %d", i);
  }
}
#endif


void LIR_OopMapGenerator::process_info(CodeEmitInfo* info) {
  if (info != NULL) {
    // The presence of a CodeEmitInfo typically means an exception can
    // be thrown. At this point we need to take our current oop map
    // state and propagate it to all exception handlers covering this
    // instruction. FIXME: need to determine whether we need to do a
    // "join" or whether we can just assert that the state matches
    // that at the beginning of the exception handler block (if the
    // state there is already present).

#ifndef PRODUCT      
    if (TraceLIROopMaps) {
      tty->print_cr("  Checking %d exception handlers of block %d",
                    cur_block()->number_of_exception_handlers(),
                    cur_block()->block_id());
    }
#endif
    for (int j = 0; j < cur_block()->number_of_exception_handlers(); j++) {
      BlockBegin* exc = cur_block()->exception_handler_at(j);
      if (exception_handler_covers(info, exc)) {
        merge_state(exc); // Causes block to be added to work list if needed
      }
    }

    // Finally, we always have to put the oop map state into the CodeEmitInfo
    info->set_lir_oop_map(oop_map());

    // If this CodeEmitInfo is present only because of an implicit
    // exception, in order to match the CI's oop maps, we need to
    // check to see whether the top scope has any exception handlers
    // and clear all oop locals in the top scope if not. This should
    // be taken care of automatically by the new register allocator.
    if (is_implicit_exception_info(info)) {
      // The test against the aastore bytecode can be removed once we
      // don't have to match the CI's oop maps exactly. With the new
      // subtype check in place, aastore no longer has a runtime call
      // in its slow case so it behaves like the other implicit
      // exception bytecodes.
      if (info->scope()->method()->java_code_at_bci(info->bci())
          != Bytecodes::_aastore) {
        // Same test as in ciOopMap.cpp
        if (!info->scope()->method()->has_exception_handlers() &&
            !info->scope()->method()->is_synchronized()) {
          // Clear oops in topmost scope
          int first = info->scope()->first_local_name();
          int last = info->scope()->last_local_name();
          if (first >= 0 && last >= first) {
            info->lir_oop_map()->at_put_range(first, last + 1, false);
          }
        }
      }
    }
  }
}

#endif // PRODUCT
