#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psPromotionManager.cpp	1.16 04/04/20 13:30:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_psPromotionManager.cpp.incl"

PSPromotionManager** PSPromotionManager::_manager_array = NULL;
OopTaskQueueSet*     PSPromotionManager::_stack_array = NULL;
PSOldGen*            PSPromotionManager::_old_gen = NULL;
MutableSpace*        PSPromotionManager::_young_space = NULL;
int		     PSPromotionManager::_promotion_failure_a_lot_count = 0;

void PSPromotionManager::initialize() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  _old_gen = heap->old_gen();
  _young_space = heap->young_gen()->to_space();
  _promotion_failure_a_lot_count = PromotionFailureALotCount;

  assert(_manager_array == NULL, "Attempt to initialize twice");
  _manager_array = NEW_C_HEAP_ARRAY(PSPromotionManager*, ParallelGCThreads+1 );
  guarantee(_manager_array != NULL, "Could not initialize promotion manager");
  
  _stack_array = new OopTaskQueueSet(ParallelGCThreads);
  guarantee(_stack_array != NULL, "Count not initialize promotion manager");

  // Create and register the PSPromotionManager(s) for the worker threads.
  for(uint i=0; i<ParallelGCThreads; i++) {
    _manager_array[i] = new PSPromotionManager();
    guarantee(_manager_array[i] != NULL, "Could not create PSPromotionManager");
    stack_array()->register_queue(i, _manager_array[i]->claimed_stack());
  }

  // The VMThread gets its own PSPromotionManager, which is not available
  // for work stealing. 
  _manager_array[ParallelGCThreads] = new PSPromotionManager();
  guarantee(_manager_array[ParallelGCThreads] != NULL, "Could not create PSPromotionManager");

}

PSPromotionManager* PSPromotionManager::gc_thread_promotion_manager(int index) {
  assert(index >= 0 && index < (int)ParallelGCThreads, "index out of range");
  assert(_manager_array != NULL, "Sanity");
  return _manager_array[index];
}

PSPromotionManager* PSPromotionManager::vm_thread_promotion_manager() {
  assert(_manager_array != NULL, "Sanity");
  return _manager_array[ParallelGCThreads];
}

void PSPromotionManager::pre_scavenge() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  _young_space = heap->young_gen()->to_space();

  for(uint i=0; i<ParallelGCThreads+1; i++) {
    manager_array(i)->reset();
  }
}

void PSPromotionManager::post_scavenge() {
  for(uint i=0; i<ParallelGCThreads+1; i++) {
    manager_array(i)->flush_labs();
  }
}

PSPromotionManager::PSPromotionManager() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // We set the old lab's start array.
  _old_lab.set_start_array(old_gen()->start_array());

  claimed_stack()->initialize();

  // We want the overflow stack to be permanent
  _overflow_stack = new (ResourceObj::C_HEAP) GrowableArray<oop>(10, true);

  reset();
}

void PSPromotionManager::reset() {
  assert(claimed_stack()->size() == 0, "reset of non-empty claimed stack");
  assert(overflow_stack()->length() == 0, "reset of non-empty overflow stack");

  // We need to get an assert in here to make sure the labs are always flushed.

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // Do not prefill the LAB's, save heap wastage!
  HeapWord* lab_base = young_space()->top();
  _young_lab.initialize(MemRegion(lab_base, (size_t)0));
  _young_gen_is_full = false;

  lab_base = old_gen()->object_space()->top();
  _old_lab.initialize(MemRegion(lab_base, (size_t)0));
  _old_gen_is_full = false;

  _prefetch_queue.clear();
}

void PSPromotionManager::drain_stacks() {
#ifdef ASSERT
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
  MutableSpace* to_space = heap->young_gen()->to_space();
  MutableSpace* old_space = heap->old_gen()->object_space();
  MutableSpace* perm_space = heap->perm_gen()->object_space();
#endif /* ASSERT */
  
  do {

    // Drain overflow stack first, so other threads can steal from
    // claimed stack while we work.
    while(!overflow_stack()->is_empty()) {
      oop obj = overflow_stack()->pop();
      obj->copy_contents(this);
    }

    oop obj;
    // obj is a reference!!!
    while (claimed_stack()->pop_local(obj)) {
      // It would be nice to assert about the type of objects we might
      // pop, but they can come from anywhere, unfortunately.
      obj->copy_contents(this);
    }

    // If we could not find any other work, flush the prefetch queue
    if (claimed_stack()->size() == 0 && (overflow_stack()->length() == 0))
      flush_prefetch_queue();
  } while((claimed_stack()->size() != 0) || (overflow_stack()->length() != 0));

  assert(claimed_stack()->size() == 0, "Sanity");
  assert(overflow_stack()->length() == 0, "Sanity");
}

void PSPromotionManager::flush_labs() {
  assert(claimed_stack()->size() <= 0, "Attempt to flush lab with live stack");
  assert(overflow_stack()->length() <= 0, "Attempt to flush lab with live overflow stack");

  // If either promotion lab fills up, we can flush the
  // lab but not refill it, so check first.
  assert(!_young_lab.is_flushed() || _young_gen_is_full, "Sanity");
  if (!_young_lab.is_flushed())
    _young_lab.flush();
 
  assert(!_old_lab.is_flushed() || _old_gen_is_full, "Sanity");
  if (!_old_lab.is_flushed())
    _old_lab.flush();

  // Let PSScavenge know if we overflowed
  if (_young_gen_is_full) {
    PSScavenge::set_survivor_overflow(true);
  }
}

//
// This method is pretty bulky. It would be nice to split it up
// into smaller submethods, but we need to be careful not to hurt
// performance.
//

oop PSPromotionManager::copy_to_survivor_space(oop o) {
  assert(PSScavenge::should_scavenge(o), "Sanity");

  oop new_obj = NULL;
  
  // NOTE! We must be very careful with any methods that access the mark
  // in o. There may be multiple threads racing on it, and it may be forwarded
  // at any time. Do not use oop methods for accessing the mark!
  markOop test_mark = o->mark();

  // The same test as "o->is_forwarded()"
  if (!test_mark->is_marked()) {
    bool new_obj_is_tenured = false;
    size_t new_obj_size = o->size();

    // Find the objects age, MT safe.
    int age = (!test_mark->is_unlocked() /* o->has_displaced_mark() */) ?
      test_mark->displaced_mark_helper()->age() : test_mark->age();

    // Try allocating obj in to-space (unless too old)
    if (age < PSScavenge::tenuring_threshold()) {
      new_obj = (oop) _young_lab.allocate(new_obj_size);
      if (new_obj == NULL && !_young_gen_is_full) {
        // Do we allocate directly, or flush and refill?
        if (new_obj_size > (YoungPLABSize / 2)) {
          // Allocate this object directly
          new_obj = (oop)young_space()->cas_allocate(new_obj_size);
        } else {
          // Flush and fill
          _young_lab.flush();
          
          HeapWord* lab_base = young_space()->cas_allocate(YoungPLABSize);
          if (lab_base != NULL) {
            _young_lab.initialize(MemRegion(lab_base, YoungPLABSize));
            // Try the young lab allocation again.
            new_obj = (oop) _young_lab.allocate(new_obj_size);
          } else {
            _young_gen_is_full = true;
          }
        }
      }
    }
    
    // Otherwise try allocating obj tenured
    if (new_obj == NULL) {
      NOT_PRODUCT(
        if (PromotionFailureALot && dec_promotion_failure_a_lot_count() <= 0) {
	  set_promotion_failure_a_lot_count(PromotionFailureALotCount);
          return oop_promotion_failed(o, test_mark);
        }
      )
      new_obj = (oop) _old_lab.allocate(new_obj_size);
      new_obj_is_tenured = true;

      if (new_obj == NULL) {
        
        if (!_old_gen_is_full) {
          // Do we allocate directly, or flush and refill?
          if (new_obj_size > (OldPLABSize / 2)) {
            // Allocate this object directly
            new_obj = (oop)old_gen()->cas_allocate(new_obj_size);
          } else {
            // Flush and fill
            _old_lab.flush();
            
            HeapWord* lab_base = old_gen()->cas_allocate(OldPLABSize);
            if(lab_base != NULL) {
              _old_lab.initialize(MemRegion(lab_base, OldPLABSize));
              // Try the old lab allocation again.
              new_obj = (oop) _old_lab.allocate(new_obj_size);
            }
          }
        }

        // This is the promotion failed test, and code handling.
        // The code belongs here for two reasons. It is slightly
        // different thatn the code below, and cannot share the
        // CAS testing code. Keeping the code here also minimizes
        // the impact on the common case fast path code.

        if (new_obj == NULL) {
          _old_gen_is_full = true;
          return oop_promotion_failed(o, test_mark);
        }
      }
    }

    assert(new_obj != NULL, "allocation should have succeeded");

    // Copy obj
    Copy::aligned_disjoint_words((HeapWord*)o, (HeapWord*)new_obj, new_obj_size);
    
    // Now we have to CAS in the header.
    if (o->cas_forward_to(new_obj, test_mark)) {
      // We won any races, we "own" this object.
      assert(new_obj == o->forwardee(), "Sanity");
      
      // Increment age if obj still in new generation. Now that
      // we're dealing with a markOop that cannot change, it is
      // okay to use the non mt safe oop methods.
      if (!new_obj_is_tenured) { 
        new_obj->incr_age();
        assert(young_space()->contains(new_obj), "Attempt to push non-promoted obj");
      }
      // If we loop too many times, handle_stack_overflow will assert.
      // It may be worth adding loop count asserts anyway.
      if (!claimed_stack()->push(new_obj)) {
        overflow_stack()->push(new_obj);
      }
    }  else {
      // We lost, someone else "owns" this object
      guarantee(o->is_forwarded(), "Object must be forwarded if the cas failed.");

      // Unallocate the space used. NOTE! We may have directly allocated
      // the object. If so, we cannot deallocate it, so we have to test!
      if (new_obj_is_tenured) {
        if (!_old_lab.unallocate_object(new_obj)) {
          // The promotion lab failed to unallocate the object.
          // We need to overwrite the object with a filler that
          // contains no interior pointers. 
          MemRegion mr((HeapWord*)new_obj, new_obj_size);
          // Clean this up and move to oopFactory (see bug 4718422)
          SharedHeap::fill_region_with_object(mr);
        }
      } else {
        if (!_young_lab.unallocate_object(new_obj)) {
          // The promotion lab failed to unallocate the object.
          // We need to overwrite the object with a filler that
          // contains no interior pointers. 
          MemRegion mr((HeapWord*)new_obj, new_obj_size);
          // Clean this up and move to oopFactory (see bug 4718422)
          SharedHeap::fill_region_with_object(mr);
        }
      }

      // don't update this before the unallocation!
      new_obj = o->forwardee();
    }
  } else {
    assert(o->is_forwarded(), "Sanity");
    new_obj = o->forwardee();
  }

#ifdef DEBUG
  // This code must come after the CAS test, or it will print incorrect
  // information.
  if (TraceScavenge) {
    gclog_or_tty->print_cr("{%s %s 0x%x -> 0x%x (%d)}", 
       PSScavenge::should_scavenge(new_obj) ? "copying" : "tenuring", 
       new_obj->blueprint()->internal_name(), o, new_obj, new_obj->size());

  }
#endif

  return new_obj;
}

oop PSPromotionManager::oop_promotion_failed(oop obj, markOop obj_mark) {
  assert(_old_gen_is_full || PromotionFailureALot, "Sanity");

  // Attempt to CAS in the header.
  // This tests if the header is still the same as when
  // this started.  If it is the same (i.e., no forwarding
  // pointer has been installed), then this thread owns
  // it.
  if (obj->cas_forward_to(obj, obj_mark)) {
    // We won any races, we "own" this object.
    assert(obj == obj->forwardee(), "Sanity");
    
    // Don't bother incrementing the age, just push
    // onto the claimed_stack..
    if(!claimed_stack()->push(obj)) {
      overflow_stack()->push(obj);
    }

    // Save the mark if needed
    PSScavenge::oop_promotion_failed(obj, obj_mark);
  }  else {
    // We lost, someone else "owns" this object
    guarantee(obj->is_forwarded(), "Object must be forwarded if the cas failed.");
    
    // No unallocation to worry about.
    obj = obj->forwardee();
  }
  
#ifdef DEBUG
  if (TraceScavenge) {
    gclog_or_tty->print_cr("{%s %s 0x%x (%d)}", 
                           "promotion-failure", 
                           obj->blueprint()->internal_name(),
                           obj, obj->size());
    
  }
#endif

  return obj;
}

