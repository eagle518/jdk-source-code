/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

# include "incls/_precompiled.incl"
# include "incls/_psMemoryPool.cpp.incl"

PSGenerationPool::PSGenerationPool(PSOldGen* gen,
                                   const char* name,
                                   PoolType type,
                                   bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->capacity_in_bytes(),
                      gen->reserved().byte_size(), support_usage_threshold), _gen(gen) {
}

PSGenerationPool::PSGenerationPool(PSPermGen* gen,
                                   const char* name,
                                   PoolType type,
                                   bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->capacity_in_bytes(),
                      gen->reserved().byte_size(), support_usage_threshold), _gen(gen) {
}

MemoryUsage PSGenerationPool::get_memory_usage() {
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);
  size_t used      = used_in_bytes();
  size_t committed = _gen->capacity_in_bytes();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

// The max size of EdenMutableSpacePool =
//     max size of the PSYoungGen - capacity of two survivor spaces
//
// Max size of PS eden space is changing due to ergonomic.
// PSYoungGen, PSOldGen, Eden, Survivor spaces are all resizable.
//
EdenMutableSpacePool::EdenMutableSpacePool(PSYoungGen* gen,
                                           MutableSpace* space,
                                           const char* name,
                                           PoolType type,
                                           bool support_usage_threshold) :
  CollectedMemoryPool(name, type, space->capacity_in_bytes(),
                      (gen->max_size() - gen->from_space()->capacity_in_bytes() - gen->to_space()->capacity_in_bytes()),
                       support_usage_threshold),
  _gen(gen), _space(space) {
}

MemoryUsage EdenMutableSpacePool::get_memory_usage() {
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);
  size_t used = used_in_bytes();
  size_t committed = _space->capacity_in_bytes();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

// The max size of SurvivorMutableSpacePool =
//     current capacity of the from-space
//
// PS from and to survivor spaces could have different sizes.
//
SurvivorMutableSpacePool::SurvivorMutableSpacePool(PSYoungGen* gen,
                                                   const char* name,
                                                   PoolType type,
                                                   bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->from_space()->capacity_in_bytes(),
                      gen->from_space()->capacity_in_bytes(),
                      support_usage_threshold), _gen(gen) {
}

MemoryUsage SurvivorMutableSpacePool::get_memory_usage() {
  size_t maxSize = (available_for_allocation() ? max_size() : 0);
  size_t used    = used_in_bytes();
  size_t committed = committed_in_bytes();
  return MemoryUsage(initial_size(), used, committed, maxSize);
}
