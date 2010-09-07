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

#include "incls/_precompiled.incl"
#include "incls/_cmsPermGen.cpp.incl"

CMSPermGen::CMSPermGen(ReservedSpace rs, size_t initial_byte_size,
             CardTableRS* ct,
             FreeBlockDictionary::DictionaryChoice dictionaryChoice) {
  CMSPermGenGen* g =
    new CMSPermGenGen(rs, initial_byte_size, -1, ct);
  if (g == NULL) {
    vm_exit_during_initialization("Could not allocate a CompactingPermGen");
  }

  g->initialize_performance_counters();

  _gen = g;
}

HeapWord* CMSPermGen::mem_allocate(size_t size) {
  Mutex* lock = _gen->freelistLock();
  bool lock_owned = lock->owned_by_self();
  if (lock_owned) {
    MutexUnlocker mul(lock);
    return mem_allocate_in_gen(size, _gen);
  } else {
    return mem_allocate_in_gen(size, _gen);
  }
}

void CMSPermGen::compute_new_size() {
  _gen->compute_new_size();
}

void CMSPermGenGen::initialize_performance_counters() {

  const char* gen_name = "perm";

  // Generation Counters - generation 2, 1 subspace
  _gen_counters = new GenerationCounters(gen_name, 2, 1, &_virtual_space);

  _gc_counters = NULL;

  _space_counters = new GSpaceCounters(gen_name, 0,
                                       _virtual_space.reserved_size(),
                                       this, _gen_counters);
}
