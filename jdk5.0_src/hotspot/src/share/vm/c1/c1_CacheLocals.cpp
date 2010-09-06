#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CacheLocals.cpp	1.42 03/12/23 16:38:58 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_CacheLocals.cpp.incl"


define_array(RegisterManagerArray, RegisterManager*);

//---------------------------LIR local caching----------------------------

// The loop information (innermost loops only) is stored in the HIR.
// There are callee saved registers and caller saved registers. 
// There is caching of locals across the whole method or across innermost
// leaf loops (i.e., loops not containing any other loops or calls that could
// destroy caller saved registers)

class BlockListScanInfo: public StackObj {
 private:
  RegisterManager*  _info;
  LIR_OpVisitState  _state;
  bool              _had_call;

  void traverse(BlockBegin* bb, LIR_OpList* inst) {
    int length = inst->length();
    for (int i = 0; i < length; i++) {
      LIR_Op* op = inst->at(i);
      _state.visit(op);
      for (LIR_OpVisitState::OprMode mode = LIR_OpVisitState::firstMode;
           mode < LIR_OpVisitState::numModes;
           mode = (LIR_OpVisitState::OprMode)(mode + 1)) {
        for (int i = 0; i < _state.opr_count(mode); i++) {
          LIR_Opr opr = _state.opr_at(mode, i);
          if (opr->is_register()) {
            _info->lock(opr->as_rinfo());
          }
        }
      }
      
      if (_state.has_call()) {
        _had_call = true;
      }
    }
  }
  
  void check_stack(ValueStack* stack) {
    if (!stack->stack_is_empty()) {
      int i = stack->stack_size();
      Value tos_val = stack->stack_at_dec(i);
      RInfo tos_reg = ValueGen::result_register_for(tos_val->type());
      info()->lock(tos_reg);
    }
  }

 public:
  BlockListScanInfo(BlockList* blocks) : _info(new RegisterManager()), _had_call(false) {
    for (int n = 0; n < blocks->length(); n++) {
      BlockBegin* b = blocks->at(n);
      if (b->lir() != NULL) {
        traverse(b, b->lir()->instructions_list());
      }
      // Registers may be used to hold the value
      // on the top of stack so check for that.
      check_stack(b->state());
      check_stack(b->end()->state());
    }
    if (_had_call) {
      for (int i = 0; i < FrameMap::nof_caller_save_cpu_regs; i++) {
        _info->lock(FrameMap::caller_save_cpu_reg_at(i)->as_rinfo());
      }
    }
  }

  RegisterManager* info() { return _info; }
};

//------------------------------------------------------

void LocalMapping::init_cached_regs() {
  _cached_regs = new RegisterManager();
  for (int i = 0; i < _mapping->length(); i++) {
    RInfo reg = _mapping->at(i);
    if (reg.is_valid()) {
      _cached_regs->lock(reg);
      assert(_free_regs == NULL || !_free_regs->is_free_reg(reg), "shouldn't be free");
    }
  }
  if (_offset_to_register_mapping == NULL) {
    // cache the computation once
    _offset_to_register_mapping = new RInfoCollection();
    for (int i = 0; i < _local_name_to_offset_map->length(); i++) {
      RInfo r = get_cache_reg(i);
      if (r.is_valid()) _offset_to_register_mapping->at_put(in_words(_local_name_to_offset_map->at(i)), r);
    }
  }
}


LIR_Opr LocalMapping::get_cache_reg(LIR_Opr opr) const {
  RInfo reg;
  if (opr->is_single_stack()) {
    reg = get_cache_reg(opr->single_stack_ix());
  } else if (opr->is_double_stack() && CacheDoubleWord) {
    reg = get_cache_reg(opr->double_stack_ix());
  }
  if (reg.is_illegal()) {
    return LIR_OprFact::illegalOpr;
  }
  switch (opr->type()) {
  case T_INT:
  case T_OBJECT:
    if (!reg.is_word()) {
      return LIR_OprFact::illegalOpr;
    }
    break;
    
  case T_FLOAT:
    if (!reg.is_float()) {
      return LIR_OprFact::illegalOpr;
    }
    break;
    
  case T_DOUBLE:
    if (!reg.is_double()) {
      return LIR_OprFact::illegalOpr;
    }
    break;
    
  case T_LONG:
    if (!reg.is_long()) {
      return LIR_OprFact::illegalOpr;
    }
    break;
  }
  return LIR_OprFact::rinfo(reg, opr->type());
}


RInfo LocalMapping::get_cache_reg(int local_index, ValueTag tag) const {
  if (local_index < _mapping->length()) {
    RInfo reg = _mapping->at(local_index);
    if (reg.is_valid()) {
      assert(_offset_to_register_mapping->at(in_words(_local_name_to_offset_map->at(local_index))).is_same(reg),
             "should be in both maps.");
    }

    switch (tag) {
      case intTag:
      case objectTag:
        if (reg.is_word()) {
          return reg;
        }
        break;

      case floatTag:
        if (reg.is_float()) {
          return reg;
        }
        break;

      case doubleTag:
        if (reg.is_double()) {
          return reg;
        }
        break;

      case longTag:
        if (reg.is_long()) {
          return reg;
        }
        break;

      case illegalTag:
        return reg;

      case addressTag:
        return norinfo;

      default:
        ShouldNotReachHere();
    }
  }
  return norinfo;
}


RInfo LocalMapping::get_cache_reg(int index) const {
  if (index >= 0 && index < length()) {
    return _mapping->at(index);
  } else {
    return norinfo;
  }
}


RInfo LocalMapping::get_cache_reg_for_local_offset(int local_offset) const {
  if (local_offset >= 0 && local_offset < _offset_to_register_mapping->length()) {
    return _offset_to_register_mapping->at(local_offset);
  } else {
    return norinfo;
  }
}


bool LocalMapping::is_cache_reg(LIR_Opr opr) const {
  if (opr->is_register()) {
    return is_cache_reg(opr->rinfo());
  } else {
    return false;
  }
}


bool LocalMapping::is_cache_reg(RInfo reg) const {
#ifdef SPARC
  if (reg.is_word() && (reg.as_register()->is_global() || reg.is_same(FrameMap::_O7_RInfo))) {
    return false;
  }
#endif // SPARC
  bool result = false;
  if (reg.is_word() || reg.is_float()) {
    result = !_cached_regs->is_free_reg(reg);
#ifdef ASSERT
    // make sure we can find this reg in the mapping
    bool found = false;
    for (int i = 0; i < _mapping->length(); i++) {
      if (_mapping->at(i).is_same(reg)) {
        found = true;
        break;
      }
    }
    assert(!(result ^ found), "cached_regs mismatch");
#endif
  }
  return result;
}


int LocalMapping::local_names_begin() {
  return 0;
}


int LocalMapping::local_names_end() {
  return _mapping->length();
}


bool LocalMapping::is_local_name_cached_in_reg(int local_name, LIR_Opr opr) {
  return _mapping->at(local_name).is_same(opr->rinfo());
}


void LocalMapping::add(int name, RInfo reg) {
  assert(_free_regs != NULL, "shouldn't be adding things when register state is unknown");
  _free_regs->lock(reg);
  int offset = in_words(_local_name_to_offset_map->at(name));
  LIR_LocalCaching::add_at_all_names(_mapping, offset, reg, 
                                     _local_name_to_offset_map);
  _offset_to_register_mapping->at_put(offset, reg);
}


void LocalMapping::remove(int name) {
  int offset = in_words(_local_name_to_offset_map->at(name));
  LIR_LocalCaching::remove_at_all_names(_mapping, in_words(_local_name_to_offset_map->at(name)),
                                        _local_name_to_offset_map);
  _offset_to_register_mapping->at_put(offset, norinfo);
}


void LocalMapping::intersection(LocalMapping* other) {
  for (int i = 0; i < other->length(); i++) {
    RInfo other_reg = other->get_cache_reg(i);
    if (i < length()) {
      RInfo reg = get_cache_reg(i);
      if (!reg.is_same(other_reg)) {
        remove(i);
      }
    }
  }
  init_cached_regs();
}


void LocalMapping::join(LocalMapping* other) {
  assert(_free_regs != NULL, "don't know the register state");
  for (int i = 0; i < other->length(); i++) {
    RInfo reg = other->get_cache_reg(i);
    if (reg.is_valid()) {
      // if the current mapping has this register free, then
      // include it in the current mapping
      if (_free_regs->is_free_reg(reg)) {
        add(i, reg);
        assert(!_free_regs->is_free_reg(reg), "should be gone");
        assert(get_cache_reg(i).is_same(reg), "should be cached");
      }
    }
  }
  init_cached_regs();
}


void LocalMapping::merge(LocalMapping* other) {
  for (int i = 0; i < other->length(); i++) {
    RInfo reg = other->get_cache_reg(i);
    if (reg.is_valid()) {
      // if the current mapping doesn't contain anything for this
      // index then let's use the
      if (get_cache_reg(i).is_illegal()) {
        int offset = in_words(_local_name_to_offset_map->at(i));
        LIR_LocalCaching::add_at_all_names(_mapping, offset, reg, 
                                           _local_name_to_offset_map);
        _offset_to_register_mapping->at_put(offset, reg);
      }
    }
  }
}


void LocalMapping::emit_transition(LIR_List* lir, LocalMapping* pred_mapping, LocalMapping* sux_mapping, IR* ir) {
  BitMap offset_bitmap(in_words(ir->highest_used_offset()));
  offset_bitmap.clear();
  WordSizeList* local_name_to_offset_map = ir->local_name_to_offset_map();

  // spill preceding block cached locals
  if (pred_mapping) {
    for (int i = 0; i < pred_mapping->length(); i++) {
      RInfo reg = pred_mapping->get_cache_reg(i);
      if (reg.is_valid()) {
        if (sux_mapping) {
          RInfo current = sux_mapping->get_cache_reg(i);
          if (current.is_same(reg)) {
            continue;
          }
        }
        int offset = in_words(local_name_to_offset_map->at(i));
        if (offset_bitmap.at(offset)) {
          continue;
        } else {
          offset_bitmap.at_put(offset, true);
        }
        if (reg.is_word()) {
          lir->reg2single_stack(reg, i, T_INT);
        } else if (reg.is_long()) {
          lir->reg2double_stack(reg, i, T_LONG);
        } else if (reg.is_float()) {
          lir->reg2single_stack(reg, i, T_FLOAT);
        } else if (reg.is_double()) {
          lir->reg2double_stack(reg, i, T_DOUBLE);
        } else {
          ShouldNotReachHere();
        }
        if (C1InvalidateCachedOopLocation) {
          lir->int2reg(-1, reg);
        }
      }
    }
  }

  offset_bitmap.clear();

  // load successor block cached locals
  if (sux_mapping) {
    for (int i = 0; i < sux_mapping->length(); i++) {
      RInfo reg = sux_mapping->get_cache_reg(i);
      if (reg.is_valid()) {
        if (pred_mapping) {
          RInfo previous = pred_mapping->get_cache_reg(i);
          if (previous.is_same(reg)) {
            continue;
          }
        }
        int offset = in_words(local_name_to_offset_map->at(i));
        if (offset_bitmap.at(offset)) {
          continue;
        } else {
          offset_bitmap.at_put(offset, true);
        }
        if (reg.is_word()) {
          lir->single_stack2reg(i, reg, T_INT);
        } else if (reg.is_long()) {
          lir->double_stack2reg(i, reg, T_LONG);
        } else if (reg.is_float()) {
          lir->single_stack2reg(i, reg, T_FLOAT);
        } else if (reg.is_double()) {
          lir->double_stack2reg(i, reg, T_DOUBLE);
        } else {
          ShouldNotReachHere();
        }
        if (C1InvalidateCachedOopLocation) {
          lir->int2stack(-1, i);
        }
      }
    }
    if (C1InvalidateCachedOopLocation) {
      c1_RegMask free_regs = sux_mapping->_free_regs->free_cpu_registers();
      while (!free_regs.is_empty()) {
        RInfo reg = free_regs.get_first_reg();
        free_regs.remove_reg(reg);
        lir->int2reg(-1, reg);
      }
    }
  }
}


#ifndef PRODUCT
void LocalMapping::print() const {
  tty->print("cached");
  for (int i = 0 ; i < length(); i++) {
    RInfo reg = get_cache_reg(i);
    if (reg.is_valid()) {
      if (reg.is_double() || reg.is_long()) {
        tty->print(" [dbl_stack:%d]=", i);
      } else {
        tty->print(" [stack:%d]=", i);
      }
      LIR_Opr opr = LIR_OprFact::rinfo(reg);
      opr->print();
    }
  }
  tty->cr();
}
#endif // PRODUCT

//------------------------------------------------------

void LocalMappingSetter::block_do(BlockBegin* block) {
  assert(block->local_mapping() == NULL, "destroying information");
  assert(block->next()->as_CachingChange() == NULL, "encountered transition block");
  block->set_local_mapping(_mapping);
}

//------------------------------------------------------


LocalMapping* LIR_LocalCaching::cache_locals_for_blocks(BlockList* blocks,
                                                        RegisterManager* blocks_scan_info,
                                                        bool is_reference) {
  ScanBlocks scan(blocks);
  ScanResult scan_result;
  scan.scan(&scan_result, true);
  ALocalList* locals = scan.most_used_locals();

#ifndef PRODUCT
  if (TraceCachedLocals) {
    tty->print_cr("MERGED BLOCKS RegisterManager");
    blocks_scan_info->print();
    scan.print(&scan_result);
  }
#endif // PRODUCT

  if (scan_result.has_calls() || scan_result.has_slow_cases()) {
#ifdef ASSERT
    for (int i = 0; i < FrameMap::nof_caller_save_cpu_regs; i++) {
      assert(!blocks_scan_info->is_free_reg(FrameMap::caller_save_cpu_reg_at(i)->rinfo()),
             "usage should be record somewhere");
    }
#endif
  } else if (ir()->method()->has_exception_handlers()) {
    // can't store things in caller save registers when we have exception handlers
    for (int i = 0; i < FrameMap::nof_caller_save_cpu_regs; i++) {
      blocks_scan_info->lock(FrameMap::caller_save_cpu_reg_at(i)->as_rinfo());
    }
  }

  if (CacheFloats) {
    // when computing a reference mapping, ignore the fact that we have calls
    // so that mappings are produced even for caller save registers so that
    // subgraphs which can use caller save registers use them in the same way
    if (!is_reference && (scan_result.has_calls() || scan_result.has_slow_cases())) {
      // mark all the float registers busy since they are killed by calls
      blocks_scan_info->lock_all_fpu();
    } else {
      // merge the floats into the generic list of locals to be cached and re-sort the list
      ALocalList* floats = scan.most_used_float_locals();
      locals->appendAll(floats);
      locals->sort(ALocal::sort_by_access_count);
    }
  }

#ifndef PRODUCT
  if (TraceCachedLocals) {
    tty->print_cr("FREE REGS for loops");
    blocks_scan_info->print();
    tty->cr();
  }
#endif // PRODUCT

  LocalMapping* item = compute_caching(locals, blocks_scan_info);

#ifndef PRODUCT
  if (TraceCachedLocals) {
    tty->print("caching for [");
    for (int i = 0; i < blocks->length(); i++) {
      tty->print(" B%d", blocks->at(i)->block_id());
    }
    tty->print_cr("]");
    item->print();
  }
#endif // PRODUCT

  return item;
}


void LIR_LocalCaching::cache_locals() {
  LoopList* loops = ir()->loops();
  BlockList* all_blocks = ir()->code();
  WordSizeList* local_name_to_offset_map = ir()->local_name_to_offset_map();

  if (loops == NULL) {
    // collect global scan information
    BlockListScanInfo gsi(ir()->code());
    RegisterManager* global_scan_info = gsi.info();

    // just cache registers globally.
    LocalMappingSetter setter(cache_locals_for_blocks(all_blocks, global_scan_info));
    all_blocks->iterate_forward(&setter);
  } else {
    assert(loops->length() != 0, "should be at least one loop");
    int i;

    // collect all the blocks that are outside of the loops
    BlockList* non_loop_blocks = new BlockList;
    for (i = 0; i < all_blocks->length(); i++) {
      BlockBegin* b = all_blocks->at(i);
      if (b->loop_index() == -1 && b->next()->as_CachingChange() == NULL) {
        non_loop_blocks->append(b);
      }
    }

    RegisterManager* global_scan_info = new RegisterManager();

    // scan each of the loops and the remaining blocks recording register usage
    // so we know what registers are free.
    RegisterManagerArray scan_infos(loops->length() + 1);
    for (i = 0; i < loops->length(); i++) {
      Loop* loop = loops->at(i);

      BlockListScanInfo lsi(loop->blocks());
      scan_infos.at_put(i, lsi.info());
      // accumulate the global state
      global_scan_info->merge(lsi.info());
    }

    BlockListScanInfo lsi(non_loop_blocks);
    scan_infos.at_put(loops->length(), lsi.info());
    // accumulate the global state
    global_scan_info->merge(lsi.info());
    
    // use the global mapping as a guide in the rest of the register selection process.
    LocalMapping* global = cache_locals_for_blocks(all_blocks, global_scan_info, true);
    LocalMapping* pref = new LocalMapping(local_name_to_offset_map);
    pref->merge(global);
    pref->merge(_preferred);
    _preferred = pref;

    for (i = 0; i < loops->length(); i++) {
      if (i < LIRCacheLoopStart || (uint)i >= (uint)LIRCacheLoopStop) {
        continue;
      }

      Loop* loop = loops->at(i);

      LocalMapping* mapping = cache_locals_for_blocks(loop->blocks(), scan_infos.at(i));
      LocalMappingSetter setter(mapping);
      loop->blocks()->iterate_forward(&setter);
      _preferred->merge(mapping);
      mapping->join(global);
    }

    LocalMapping* mapping = cache_locals_for_blocks(non_loop_blocks, scan_infos.at(loops->length()));
    mapping->join(global);
    LocalMappingSetter setter(mapping);
    non_loop_blocks->iterate_forward(&setter);
  }
}


LocalMapping* LIR_LocalCaching::compute_caching(ALocalList* locals, 
                                                RegisterManager* registers) {
  int i;
  int num_free_cpu_regs = registers->num_free_cpu();

  ALocalList* reg_locals = new ALocalList(num_free_cpu_regs);
  // insert all the loop locals first
  for (i = 0; i < locals->length(); i++) {
    ALocal* local = locals->at(i);
    int index = local->index();
    int size = 1;
    switch (local->type()) {
    case longTag:
      if (!CacheDoubleWord) {
        break;
      }
      size = 2;
    case objectTag:
    case intTag:
      if (num_free_cpu_regs >= size) {
        assert(!reg_locals->contains(local), "shouldn't be in there yet");
        reg_locals->append(local);
        num_free_cpu_regs -= size;
      }
      break;
      
    case doubleTag:
      if (!CacheDoubleWord) {
        break;
      }
    case floatTag:
      reg_locals->append(local);
      break;
    }
  }
  reg_locals->sort(ALocal::sort_by_index);
  
  RInfoCollection* mapping = new RInfoCollection();
  WordSizeList* local_name_to_offset_map = ir()->local_name_to_offset_map();

  // first allocate all locations which have preferred registers
  for (i = 0; i < reg_locals->length(); i++) {
    ALocal* local = reg_locals->at(i);
    int index = local->index();
      
    RInfo reg = preferred()->get_cache_reg(index, local->type());
    // if there's no preferred mapping or the register is unavailable, skip it
    if (reg.is_illegal() || !registers->is_free_reg(reg)) {
      continue;
    }
    assert(!reg.is_illegal(), "we should always have something");
    assert(is_illegal_at_all_names(mapping, index, local_name_to_offset_map), "shouldn't be mapped yet");
    registers->lock(reg);
    assert(!registers->is_free_reg(reg), "must be locked");

    // Must translate this index back into all local names which map to it
    add_at_all_names(mapping, index, reg, local_name_to_offset_map);
  }

  // allocate everything else to available registers
  for (i = reg_locals->length() - 1; i >= 0; i--) {
    ALocal* local = reg_locals->at(i);
    int index = local->index();

    {
      debug_only(bool found_one = false;);
      int first_index;
      for (int j = 0; j < local_name_to_offset_map->length(); j++) {
        if (index == in_words(local_name_to_offset_map->at(j))) {
          first_index = j;
          debug_only(found_one = true;)
          break;
        }
      }
#ifdef ASSERT
      if (!found_one) {
        tty->print_cr("Contents of name to local map:");
        for (int q = 0; q < local_name_to_offset_map->length(); q++) {
          tty->print("Name %d: %d  ", q, in_words(local_name_to_offset_map->at(q)));
        }
        tty->print_cr("");
      }
#endif // ASSERT

      assert(found_one, "bug in local name to offset map");
      if (mapping->at(first_index).is_valid()) {
        continue;
      }
    }

    // pick a free register for caching
    ValueTag tag = local->type();
    if (!registers->has_free_reg(tag)) {
      assert(tag == floatTag || tag == doubleTag, "should only happen for floats");
      continue;
    }
    RInfo reg = registers->lock_free_reg(tag);
    assert(!registers->is_free_reg(reg), "must be locked");
    assert(!reg.is_illegal(), "we should always have something");
    assert(is_illegal_at_all_names(mapping, index, local_name_to_offset_map), "shouldn't be mapped yet");

    // Must translate this index back into all local names which map to it
    add_at_all_names(mapping, index, reg, local_name_to_offset_map);
  }

#ifndef PRODUCT
  {
    // maybe mask out registers to aid debugging
    uint mask = LIRLocalCachingMask;
    for (int i = 0; i < reg_locals->length(); i++, mask >>= 1) {
      if ((mask & 1) != 0) {
        ALocal* local = reg_locals->at(i);
        remove_at_all_names(mapping, local->index(), local_name_to_offset_map);
      }
    }
  }
#endif // PRODUCT

  return new LocalMapping(local_name_to_offset_map, mapping, registers);
}

define_array(LocalMappingArray, LocalMapping*)
define_stack(LocalMappingList, LocalMappingArray)


class BlockTransition: public BlockClosure {
 private:
  LocalMapping* _tag;
 public:
  BlockTransition(LocalMapping* tag): _tag(tag) {}
  
  virtual void block_do(BlockBegin* block) {
    int n = block->end()->number_of_sux();
    for (int i = 0; i < n; i++) {
      BlockBegin* next = block->end()->sux_at(i);
      CachingChange* cc = next->next()->as_CachingChange();
      if (cc == NULL) {
        assert(block->local_mapping() == NULL || block->local_mapping() == _tag || block->local_mapping() == next->local_mapping(), "if they aren't equal then we should have had a caching block here");
        continue;
      }
      if (next->local_mapping() == _tag) {
        // we've already processed this caching change block
        continue;
      }
      assert(next->end()->number_of_sux() == 1, "caching change should only have one sux");
      BlockBegin* sux = next->end()->default_sux();
      
      assert(block->local_mapping() != sux->local_mapping(), "if they aren't different then we don't need a caching block here");
      assert(next->local_mapping() == NULL, "caching change block should be NULL at this point");
      emit_local_mapping_transition(next->lir(), block->local_mapping(), sux->local_mapping(), block->scope()->compilation()->hir());
      next->set_local_mapping(_tag);
    }
  }



  void emit_local_mapping_transition(LIR_List* lir, LocalMapping* pred_mapping, LocalMapping* sux_mapping, IR* ir) {
    LIR_OpList* lirops = lir->instructions_list();
    int n = lirops->length();
    LIR_Op* op = lirops->at(n - 1);
    LIR_OpBranch* branch = op->as_OpBranch();
    if (branch && branch->cond() == LIR_OpBranch::always) {
      lirops->remove_at(n - 1);
    }

    LocalMapping::emit_transition(lir, pred_mapping, sux_mapping, ir);

    if (branch) {
      lirops->append(branch);
    }
  }
};

void LIR_LocalCaching::insert_transition_blocks() {
  BlockTransition bt(new LocalMapping(ir()->local_name_to_offset_map()));
  ir()->code()->iterate_forward(&bt);
}


void LIR_LocalCaching::compute_cached_locals() {
  cache_locals();
  insert_transition_blocks();
}


LIR_LocalCaching::LIR_LocalCaching(IR* ir):
    _ir(ir)
  , _preferred(preferred_locals(ir->method())) {
}


void LIR_LocalCaching::add_at_all_names(RInfoCollection* mapping, int offset, RInfo reg, WordSizeList* local_name_to_offset_map) {
  // Must translate this offset back into all local names which map to it
  debug_only(bool found_one = false;)
  for (int j = 0; j < local_name_to_offset_map->length(); j++) {
    if (offset == in_words(local_name_to_offset_map->at(j))) {
      mapping->at_put(j, reg);
      assert(mapping->at(j).is_same(reg), "should be mapped");
      debug_only(found_one = true;)
    }
  }
  assert(found_one, "bug in local name to offset map");
}


void LIR_LocalCaching::remove_at_all_names(RInfoCollection* mapping, int offset, WordSizeList* local_name_to_offset_map) {
  // Must translate this offset back into all local names which map to it
  debug_only(bool found_one = false;);
  for (int j = 0; j < local_name_to_offset_map->length(); j++) {
    if (offset == in_words(local_name_to_offset_map->at(j))) {
      mapping->at_put(j, norinfo);
      debug_only(found_one = true;);
    }
  }
  assert(found_one, "bug in local name to offset map");
}


bool LIR_LocalCaching::is_illegal_at_all_names(RInfoCollection* mapping, int offset, WordSizeList* local_name_to_offset_map) {
  bool illegal_at_all_names = true;
  debug_only(bool found_one = false;);
  for (int j = 0; j < local_name_to_offset_map->length(); j++) {
    if (offset == in_words(local_name_to_offset_map->at(j))) {
      if (!mapping->at(j).is_illegal()) {
        illegal_at_all_names = false;
      }
      debug_only(found_one = true;);
    }
  }
  assert(found_one, "bug in local name to offset map");
  return illegal_at_all_names;
}
