#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psTasks.cpp	1.18 03/12/23 16:40:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_psTasks.cpp.incl"

//
// ScavengeRootsTask
//

// Define before use
class PSScavengeRootsClosure: public OopClosure {
 private:
  PSPromotionManager* _promotion_manager;

 public:
  PSScavengeRootsClosure(PSPromotionManager* pm) : _promotion_manager(pm) { }

  virtual void do_oop(oop* p) {
    if (PSScavenge::should_scavenge(*p)) {
      // We never card mark roots, maybe call a func without test?
      PSScavenge::copy_and_push_safe_barrier(_promotion_manager, p);
    }
  }
};

void ScavengeRootsTask::do_it(GCTaskManager* manager, uint which) {
  assert(Universe::heap()->is_gc_active(), "called outside gc");

  PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
  PSScavengeRootsClosure roots_closure(pm);
  
  switch (_root_type) {
    case universe:
      Universe::oops_do(&roots_closure);
      break;

    case jni_handles:
      JNIHandles::oops_do(&roots_closure);
      break;

    case threads:
    {
      ResourceMark rm;
      Threads::oops_do(&roots_closure);
    }
    break;

    case object_synchronizer:
      ObjectSynchronizer::oops_do(&roots_closure);
      break;

    case flat_profiler:
      FlatProfiler::oops_do(&roots_closure);
      break;

    case system_dictionary:
      SystemDictionary::oops_do(&roots_closure);
      break;

    case management:
      Management::oops_do(&roots_closure);
      break;

    case jvmti:
      JvmtiExport::oops_do(&roots_closure);
      break;

    default:
      fatal("Unknown root type");
  }

  // Do the real work
  pm->drain_stacks();
}

//
// ThreadRootsTask
//

void ThreadRootsTask::do_it(GCTaskManager* manager, uint which) {
  assert(Universe::heap()->is_gc_active(), "called outside gc");

  PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
  PSScavengeRootsClosure roots_closure(pm);
  
  if (_java_thread != NULL)
    _java_thread->oops_do(&roots_closure);

  if (_vm_thread != NULL)
    _vm_thread->oops_do(&roots_closure);

  // Do the real work
  pm->drain_stacks();
}

//
// StealTask
//

volatile bool StealTask::_terminate = false;

StealTask::StealTask(bool is_termination_task) : _is_termination_task(is_termination_task)
{
  if (is_termination_task) {
    _terminate = false;
  }
}

void StealTask::do_it(GCTaskManager* manager, uint which) {
  assert(Universe::heap()->is_gc_active(), "called outside gc");

  // If we're the termination task, just set the flag and exit.
  if (_is_termination_task) {
    _terminate = true;
    return;
  }

  PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
  oop obj = NULL;
  int random_seed = 17;
  while(!_terminate) {
    oop obj;
    if (PSPromotionManager::steal(which, &random_seed, obj)) {
      obj->copy_contents(pm);
      pm->drain_stacks();
    }
  }
}

//
// SerialOldToYoungRootsTask
//

void SerialOldToYoungRootsTask::do_it(GCTaskManager* manager, uint which) {
  assert(_gen != NULL, "Sanity");
  assert(_gen->object_space()->contains(_gen_top) || _gen_top == _gen->object_space()->top(), "Sanity");
  
  { 
    PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
    
    assert(Universe::heap()->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
    CardTableExtension* card_table = (CardTableExtension *)Universe::heap()->barrier_set();
    // FIX ME! Assert that card_table is the type we believe it to be.
    
    card_table->scavenge_contents(_gen->start_array(),
                                  _gen->object_space(),
                                  _gen_top,
                                  pm);

    // Do the real work
    pm->drain_stacks();
  }
}

//
// OldToYoungRootsTask
//

void OldToYoungRootsTask::do_it(GCTaskManager* manager, uint which) {
  assert(_gen != NULL, "Sanity");
  assert(_gen->object_space()->contains(_gen_top) || _gen_top == _gen->object_space()->top(), "Sanity");
  assert(_stripe_number < ParallelGCThreads, "Sanity");

  { 
    PSPromotionManager* pm = PSPromotionManager::gc_thread_promotion_manager(which);
    
    assert(Universe::heap()->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
    CardTableExtension* card_table = (CardTableExtension *)Universe::heap()->barrier_set();
    // FIX ME! Assert that card_table is the type we believe it to be.
    
    card_table->scavenge_contents_parallel(_gen->start_array(),
                                           _gen->object_space(),
                                           _gen_top,
                                           pm,
                                           _stripe_number);
    
    // Do the real work
    pm->drain_stacks();    
  }
}


