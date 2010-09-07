/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
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

inline size_t ParallelScavengeHeap::total_invocations()
{
  return UseParallelOldGC ? PSParallelCompact::total_invocations() :
    PSMarkSweep::total_invocations();
}

inline void ParallelScavengeHeap::invoke_scavenge()
{
  PSScavenge::invoke();
}

inline void ParallelScavengeHeap::invoke_full_gc(bool maximum_compaction)
{
  if (UseParallelOldGC) {
    PSParallelCompact::invoke(maximum_compaction);
  } else {
    PSMarkSweep::invoke(maximum_compaction);
  }
}

inline bool ParallelScavengeHeap::is_in_young(oop p) {
  return young_gen()->is_in_reserved(p);
}

inline bool ParallelScavengeHeap::is_in_old_or_perm(oop p) {
  return old_gen()->is_in_reserved(p) || perm_gen()->is_in_reserved(p);
}
