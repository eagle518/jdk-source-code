#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)instanceRefKlass.cpp	1.74 03/12/23 16:41:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_instanceRefKlass.cpp.incl"

void instanceRefKlass::oop_follow_contents(oop obj) {
  oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
  oop referent = *referent_addr;
  debug_only(
    if(TraceReferenceGC && PrintGCDetails) {
      gclog_or_tty->print_cr("instanceRefKlass::oop_follow_contents " INTPTR_FORMAT, obj);
    }
  )
  if (referent != NULL) {
    if (!referent->is_gc_marked() &&
        MarkSweep::ref_processor()->
          discover_reference(obj, reference_type())) {
      // reference already enqueued, referent will be traversed later
      instanceKlass::oop_follow_contents(obj);
      debug_only( 
        if(TraceReferenceGC && PrintGCDetails) {
          gclog_or_tty->print_cr("       Non NULL enqueued " INTPTR_FORMAT, obj); 
        }
      )
      return;
    } else {
      // treat referent as normal oop      
      debug_only(
        if(TraceReferenceGC && PrintGCDetails) {
          gclog_or_tty->print_cr("       Non NULL normal " INTPTR_FORMAT, obj);
        }
      )
      MarkSweep::mark_and_push(referent_addr);
    }
  }
  // treat next as normal oop.  next is a link in the pending list.
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);
  debug_only(
    if(TraceReferenceGC && PrintGCDetails) {
      gclog_or_tty->print_cr("   Process next as normal " INTPTR_FORMAT, next_addr);
    }
  )
  MarkSweep::mark_and_push(next_addr);
  instanceKlass::oop_follow_contents(obj);
}


int instanceRefKlass::oop_adjust_pointers(oop obj) {
  int size = size_helper();
  instanceKlass::oop_adjust_pointers(obj);
  
  oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
  MarkSweep::adjust_pointer(referent_addr);
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);
  MarkSweep::adjust_pointer(next_addr);
  oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
  assert(discovered_addr || (*discovered_addr)->is_oop_or_null(),
    "discovered field is bad");
  MarkSweep::adjust_pointer(discovered_addr);
#ifdef ASSERT
  if(TraceReferenceGC && PrintGCDetails) {
    gclog_or_tty->print_cr("instanceRefKlass::oop_adjust_pointers obj " INTPTR_FORMAT, obj);
    gclog_or_tty->print_cr("     referent_addr/* " INTPTR_FORMAT " / " INTPTR_FORMAT, 
      referent_addr, referent_addr ? *referent_addr : NULL);
    gclog_or_tty->print_cr("     next_addr/* " INTPTR_FORMAT " / " INTPTR_FORMAT, next_addr,
      next_addr ? *next_addr : NULL);
    gclog_or_tty->print_cr("     discovered_addr/* " INTPTR_FORMAT " / " INTPTR_FORMAT, 
      discovered_addr, discovered_addr ? *discovered_addr : NULL);
  }
#endif
  return size;
}

#define InstanceRefKlass_OOP_OOP_ITERATE_DEFN(OopClosureType, nv_suffix)        \
                                                                                \
int instanceRefKlass::                                                          \
oop_oop_iterate##nv_suffix(oop obj, OopClosureType* closure) {                  \
  /* Get size before changing pointers */                                       \
  SpecializationStats::record_iterate_call##nv_suffix(SpecializationStats::irk);\
                                                                                \
  int size = instanceKlass::oop_oop_iterate##nv_suffix(obj, closure);           \
                                                                                \
  oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);             \
  oop referent = *referent_addr;                                                \
  if (referent != NULL) {                                                       \
    ReferenceProcessor* rp = closure->_ref_processor;                           \
    if (!referent->is_gc_marked() && (rp != NULL) &&                            \
        rp->discover_reference(obj, reference_type())) {              \
      return size;                                                              \
    } else {                                                                    \
      /* treat referent as normal oop */                                        \
      SpecializationStats::record_do_oop_call##nv_suffix(SpecializationStats::irk);\
      closure->do_oop##nv_suffix(referent_addr);                                \
    }                                                                           \
  }                                                                             \
                                                                                \
  /* treat next as normal oop */                                                \
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);                     \
  SpecializationStats::record_do_oop_call##nv_suffix(SpecializationStats::irk); \
  closure->do_oop##nv_suffix(next_addr);                                        \
  return size;                                                                  \
}

#define InstanceRefKlass_OOP_OOP_ITERATE_DEFN_m(OopClosureType, nv_suffix)      \
                                                                                \
int instanceRefKlass::                                                          \
oop_oop_iterate##nv_suffix##_m(oop obj,                                         \
                               OopClosureType* closure,                         \
                               MemRegion mr) {                                  \
  SpecializationStats::record_iterate_call##nv_suffix(SpecializationStats::irk);\
                                                                                \
  int size = instanceKlass::oop_oop_iterate##nv_suffix##_m(obj, closure, mr);   \
                                                                                \
  oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);             \
  oop referent = *referent_addr;                                                \
  if (referent != NULL && mr.contains(referent_addr)) {                         \
    ReferenceProcessor* rp = closure->_ref_processor;                           \
    if (!referent->is_gc_marked() && (rp != NULL) &&                            \
        rp->discover_reference(obj, reference_type())) {              \
      return size;                                                              \
    } else {                                                                    \
      /* treat referent as normal oop */                                        \
      SpecializationStats::record_do_oop_call##nv_suffix(SpecializationStats::irk);\
      closure->do_oop##nv_suffix(referent_addr);                                \
    }                                                                           \
  }                                                                             \
                                                                                \
  /* treat next as normal oop */                                                \
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);                     \
  if (mr.contains(next_addr)) {                                                 \
    SpecializationStats::record_do_oop_call##nv_suffix(SpecializationStats::irk);\
    closure->do_oop##nv_suffix(next_addr);                                      \
  }                                                                             \
  return size;                                                                  \
}

ALL_OOP_OOP_ITERATE_CLOSURES_1(InstanceRefKlass_OOP_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_2(InstanceRefKlass_OOP_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_3(InstanceRefKlass_OOP_OOP_ITERATE_DEFN)
ALL_OOP_OOP_ITERATE_CLOSURES_1(InstanceRefKlass_OOP_OOP_ITERATE_DEFN_m)
ALL_OOP_OOP_ITERATE_CLOSURES_2(InstanceRefKlass_OOP_OOP_ITERATE_DEFN_m)
ALL_OOP_OOP_ITERATE_CLOSURES_3(InstanceRefKlass_OOP_OOP_ITERATE_DEFN_m)


void instanceRefKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
  if (PSScavenge::should_scavenge(*referent_addr)) {
    ReferenceProcessor* rp = PSScavenge::reference_processor();
    if (rp->discover_reference(obj, reference_type())) {
      // reference already enqueued, referent and next will be traversed later
      instanceKlass::oop_copy_contents(pm, obj);
      return;
    } else {
      // treat referent as normal oop
      pm->claim_or_forward(referent_addr);
    }
  }
  // treat next as normal oop
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);
  if (PSScavenge::should_scavenge(*next_addr)) {
    pm->claim_or_forward(next_addr);
  }
  instanceKlass::oop_copy_contents(pm, obj);
}

void instanceRefKlass::update_nonstatic_oop_maps(klassOop k) {
  // Clear the nonstatic oop-map entries corresponding to referent
  // and nextPending field.  They are treated specially by the 
  // garbage collector.
  // The discovered field is used only by the garbage collector
  // and is also treated specially.
  instanceKlass* ik = instanceKlass::cast(k);

  // Check that we have the right class
  debug_only(static bool first_time = true);
  assert(k == SystemDictionary::reference_klass() && first_time,
         "Invalid update of maps");
  debug_only(first_time = false);
  assert(ik->nonstatic_oop_map_size() == 1, "just checking");

  OopMapBlock* map = ik->start_of_nonstatic_oop_maps();

  // Check that the current map is (2,4) - currently points at field with
  // offset 2 (words) and has 4 map entries.
  debug_only(int offset = java_lang_ref_Reference::referent_offset);
  debug_only(int length = ((java_lang_ref_Reference::discovered_offset - 
    java_lang_ref_Reference::referent_offset)/wordSize) + 1);

  if (UseSharedSpaces) {
    assert(map->offset() == java_lang_ref_Reference::queue_offset &&
           map->length() == 1, "just checking");
  } else {
    assert(map->offset() == offset && map->length() == length,
           "just checking");

    // Update map to (3,1) - point to offset of 3 (words) with 1 map entry.
    map->set_offset(java_lang_ref_Reference::queue_offset);
    map->set_length(1);
  }
}



#ifndef PRODUCT

// Verification

void instanceRefKlass::oop_verify_on(oop obj, outputStream* st) {
  instanceKlass::oop_verify_on(obj, st);
  // Verify referent field
  oop referent = java_lang_ref_Reference::referent(obj);

  // We should make this general to all heaps
  GenCollectedHeap* gch = NULL;
  if (Universe::heap()->kind() == CollectedHeap::GenCollectedHeap)
    gch = GenCollectedHeap::heap();
  
  if (referent != NULL) {
    guarantee(referent->is_oop(), "referent field heap failed");
    if (gch != NULL && !gch->is_in_youngest(obj))
      // We do a specific remembered set check here since the referent
      // field is not part of the oop mask and therefore skipped by the
      // regular verify code.
      obj->verify_old_oop(java_lang_ref_Reference::referent_addr(obj), true);
  }
  // Verify next field
  oop next = java_lang_ref_Reference::next(obj);
  if (next != NULL) {
    guarantee(next->is_oop(), "next field verify failed");    
    guarantee(next->is_instanceRef(), "next field verify failed");
    if (gch != NULL && !gch->is_in_youngest(obj)) {
      // We do a specific remembered set check here since the next field is
      // not part of the oop mask and therefore skipped by the regular
      // verify code.
      obj->verify_old_oop(java_lang_ref_Reference::next_addr(obj), true);
    }
  }
}
#endif

void instanceRefKlass::acquire_pending_list_lock(BasicLock *pending_list_basic_lock) {
  // we may enter this with pending exception set
  PRESERVE_EXCEPTION_MARK;  // exceptions are never thrown, needed for TRAPS argument
  Handle h_lock(THREAD, java_lang_ref_Reference::pending_list_lock());
  ObjectSynchronizer::fast_enter(h_lock, pending_list_basic_lock, THREAD);
  assert(ObjectSynchronizer::current_thread_holds_lock(
           JavaThread::current(), h_lock),
         "Locking should have succeeded");
  if (HAS_PENDING_EXCEPTION) CLEAR_PENDING_EXCEPTION;
}

void instanceRefKlass::release_and_notify_pending_list_lock(bool notify_ref_lock,
  BasicLock *pending_list_basic_lock) {
  // we may enter this with pending exception set
  PRESERVE_EXCEPTION_MARK;  // exceptions are never thrown, needed for TRAPS argument
  //
  Handle h_lock(THREAD, java_lang_ref_Reference::pending_list_lock());
  assert(ObjectSynchronizer::current_thread_holds_lock(
           JavaThread::current(), h_lock),
         "Lock should be held");
  if (notify_ref_lock) {
    // Notify waiters on pending lists lock. The _notify_ref_lock flag is set by doit.
    ObjectSynchronizer::notifyall(h_lock, THREAD);
  }
  ObjectSynchronizer::fast_exit(h_lock(), pending_list_basic_lock, THREAD);
  if (HAS_PENDING_EXCEPTION) CLEAR_PENDING_EXCEPTION;
}
