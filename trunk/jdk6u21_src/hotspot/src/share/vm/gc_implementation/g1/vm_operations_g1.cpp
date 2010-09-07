/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_vm_operations_g1.cpp.incl"

void VM_G1CollectForAllocation::doit() {
  JvmtiGCForAllocationMarker jgcm;
  G1CollectedHeap* g1h = G1CollectedHeap::heap();
  _res = g1h->satisfy_failed_allocation(_size);
  assert(g1h->is_in_or_null(_res), "result not in heap");
}

void VM_G1CollectFull::doit() {
  JvmtiGCFullMarker jgcm;
  G1CollectedHeap* g1h = G1CollectedHeap::heap();
  GCCauseSetter x(g1h, _gc_cause);
  g1h->do_full_collection(false /* clear_all_soft_refs */);
}

void VM_G1IncCollectionPause::doit() {
  JvmtiGCForAllocationMarker jgcm;
  G1CollectedHeap* g1h = G1CollectedHeap::heap();
  GCCauseSetter x(g1h, _gc_cause);
  g1h->do_collection_pause_at_safepoint();
}

void VM_CGC_Operation::doit() {
  gclog_or_tty->date_stamp(PrintGC && PrintGCDateStamps);
  TraceCPUTime tcpu(PrintGCDetails, true, gclog_or_tty);
  TraceTime t(_printGCMessage, PrintGC, true, gclog_or_tty);
  SharedHeap* sh = SharedHeap::heap();
  // This could go away if CollectedHeap gave access to _gc_is_active...
  if (sh != NULL) {
    IsGCActiveMark x;
    _cl->do_void();
  } else {
    _cl->do_void();
  }
}

bool VM_CGC_Operation::doit_prologue() {
  Heap_lock->lock();
  SharedHeap::heap()->_thread_holds_heap_lock_for_gc = true;
  return true;
}

void VM_CGC_Operation::doit_epilogue() {
  SharedHeap::heap()->_thread_holds_heap_lock_for_gc = false;
  Heap_lock->unlock();
}
