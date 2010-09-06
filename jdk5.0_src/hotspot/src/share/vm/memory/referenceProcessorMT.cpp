#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)referenceProcessorMT.cpp	1.9 03/12/23 16:41:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_referenceProcessorMT.cpp.incl"

ReferenceProcessorMT::ReferenceProcessorMT(MemRegion span,
  bool atomic_discovery, bool mt_discovery, int mt_degree):
  ReferenceProcessor(span, atomic_discovery, mt_discovery, mt_degree),
  _next_id(0),
  _par_task(NULL) { 
  assert(_num_q == mt_degree, "consistency check");
  assert(_num_q > 0, "Should at least be 1");
  if (_num_q > 1) {
    _processing_is_mt = true;
  } else {
    _processing_is_mt = false;
    assert(!ParallelRefProcEnabled,
           "Should you be using ReferenceProcessor instead?");
  }
}

void ReferenceProcessorMT::process_phaseJNI() {
  // Since we do not yet do this phase multi-threaded,
  // we'd just like to call the parent class's implementation;
  // unfortunately, in the case that processing_is_mt(),
  // the closures themselves are constructed
  // in the AbstractGangTask, so we can't just do this: 
  //
  // ReferenceProcessor::process_phaseJNI();
  //
  // Rather, we need to do this:
  if (_processing_is_mt) {
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    // use CollectedHeap in general XXX FIX ME !!!
    WorkGang* workers = gch->workers();
    assert(workers != NULL, "Need parallel worker threads");
    assert(_par_task != NULL, "No processing task?");
    _par_task->set_phase(AbstractRefProcTask::PhaseJNI);
    // Single-threaded, so execute in the context of the
    // current thread itself.
    _par_task->work(0);
  } else {
    // single-threaded processing, call parent class's
    // implemenetation
    ReferenceProcessor::process_phaseJNI();
  }
}


// Preclean the discovered references by removing those
// whose referents are alive. These lists can be handled here
// in any order and, indeed, concurrently.
void ReferenceProcessorMT::preclean_discovered_references(
  BoolObjectClosure* is_alive, YieldClosure* yield) {

  NOT_PRODUCT(verify_ok_to_handle_reflists());
  // Soft references
  {
    NOT_PRODUCT(TraceTime("Preclean SoftReferences", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    for (int i = 0; i < _num_q; i++) {
      preclean_discovered_reflist(&_discoveredSoftRefs[i], is_alive, yield);
    }
  }
  if (yield->should_return()) {
    return;
  }

  // Weak references
  {
    NOT_PRODUCT(TraceTime("Preclean WeakReferences", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    for (int i = 0; i < _num_q; i++) {
      preclean_discovered_reflist(&_discoveredWeakRefs[i], is_alive, yield);
    }
  }
  if (yield->should_return()) {
    return;
  }

  // Final references
  {
    NOT_PRODUCT(TraceTime("Preclean FinalReferences", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    for (int i = 0; i < _num_q; i++) {
      preclean_discovered_reflist(&_discoveredFinalRefs[i], is_alive, yield);
    }
  }
  if (yield->should_return()) {
    return;
  }

  // Phantom references
  {
    NOT_PRODUCT(TraceTime("Preclean PhantomReferences", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    for (int i = 0; i < _num_q; i++) {
      preclean_discovered_reflist(&_discoveredPhantomRefs[i], is_alive, yield);
    }
  }
}

// Parallel enqueue task
class RefProcEnqueueTask: public AbstractGangTask {
  ReferenceProcessorMT* _rp;
  oop*                _dsr;   // discoveredSoftRefs array
  oop*                _pla;   // pending list addr
  oop                 _sr;    // sentinel ref
  int                 _num_q;
 public:
  RefProcEnqueueTask(ReferenceProcessorMT* rp,
                     oop* dsr, oop* pla,
                     oop   sr, int  num_q):
    AbstractGangTask("Enqueue reference objects in parallel"),
    _rp(rp), _dsr(dsr), _pla(pla),
    _sr(sr), _num_q(num_q) { }
  void work(int i) {
    assert(0 <= i && i < _rp->num_q(), "Index out-of-bounds");
    // Simplest first cut: static partitioning.
    int index = i;
    for (int j = 0; j < 4; j++, index += _num_q) {
      _rp->enqueue_discovered_reflist(_dsr[index], _pla);
      _dsr[index] = _sr;
    }
  }
};

// Try to factor code so as to avoid duplication with superclass. XXX !!!
void ReferenceProcessorMT::enqueue_discovered_reflist(oop refs_list,
  oop* pending_list_addr) {
  // Given a list of refs linked through the "discovered" field
  // (java.lang.ref.Reference.discovered) chain them through the
  // "next" field (java.lang.ref.Reference.next) and prepend
  // to the pending list.
  if (TraceReferenceGC && PrintGCDetails) {
    gclog_or_tty->print_cr("ReferenceProcessorMT::enqueue_discovered_reflist list " INTPTR_FORMAT, 
      refs_list);
  }
  oop obj = refs_list;
  // Walk down the list, copying the discovered field into
  // the next field and clearing it (except for the last
  // non-sentinel object which is treated specially to avoid
  // confusion with an active reference).
  while (obj != _sentinelRef) {
    assert(obj->is_instanceRef(), "should be reference object");
    oop next = java_lang_ref_Reference::discovered(obj);
    if (TraceReferenceGC && PrintGCDetails) {
      gclog_or_tty->print_cr("	obj " INTPTR_FORMAT "/next " INTPTR_FORMAT, obj, next);
    }
    assert(*java_lang_ref_Reference::next_addr(obj) == NULL, 
      "The reference should not be enqueued");
    if (next == _sentinelRef) {  // obj is last
      // Swap refs_list into pendling_list_addr and
      // set obj's next to what we read from pending_list_addr.
      oop old = (oop)Atomic::xchg_ptr(refs_list, pending_list_addr);
      // Need oop_check on pending_list_addr above;
      // this is currently done at the end of our caller,
      // ReferenceProcessor::enqueue_discovered_references(). XXX
      if (old == NULL) {
        // obj should be made to point to itself, since
        // pending list was empty.
        java_lang_ref_Reference::set_next(obj, obj);
      } else {
        java_lang_ref_Reference::set_next(obj, old);
      }
    } else {
      java_lang_ref_Reference::set_next(obj, next);
    }
    java_lang_ref_Reference::set_discovered(obj, (oop) NULL);
    obj = next;
  }
}

// Enqueue references that are not made active again
void ReferenceProcessorMT::enqueue_discovered_reflists(oop* pending_list_addr) {
  if (_processing_is_mt) {
    // Parallel code
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    // use CollectedHeap in general XXX FIX ME !!!
    WorkGang* workers = gch->workers();
    assert(workers != NULL, "Need parallel worker threads");
    RefProcEnqueueTask tsk(this, _discoveredSoftRefs,
                           pending_list_addr, _sentinelRef, _num_q);
    assert(_num_q > 1, "Internal inconsistency");
    workers->run_task(&tsk);
  } else {
    // Serial code: call the parent class's implementation
    ReferenceProcessor::enqueue_discovered_reflists(pending_list_addr);
  }
}

// Walk the given discovered ref list, and remove all reference objects
// whose referents are still alive. NOTE: For this to work correctly,
// refs discovery can not be happening concurrently with this step.
void ReferenceProcessorMT::preclean_discovered_reflist(
  oop* refs_list_addr, BoolObjectClosure* is_alive, YieldClosure* yield) {

  NOT_PRODUCT(size_t counter1 = 0;)
  NOT_PRODUCT(size_t counter2 = 0;)

  oop* prev_next = refs_list_addr;
  oop  obj = *refs_list_addr;
  DEBUG_ONLY(oop first_seen = obj;) // cyclic linked list check

  // Traverse the list and remove any refs whose referents are alive.
  while (obj != _sentinelRef && !yield->should_return()) {
    oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
    assert(discovered_addr && (*discovered_addr)->is_oop_or_null(),
      "discovered field is bad");
    oop next = *discovered_addr;
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
      "Wrong oop found in java.lang.Reference object");
    if (is_alive->do_object_b(referent)) {
      // The referent is reachable after all, make the Reference object active again
      java_lang_ref_Reference::set_next(obj, NULL);
      if (TraceReferenceGC) {
#ifdef PRODUCT 
        gclog_or_tty->print_cr("Dropping strongly reachable reference (" INTPTR_FORMAT     ")",  
                               obj                                   ); 
#else 
        gclog_or_tty->print_cr("Dropping strongly reachable reference (" INTPTR_FORMAT ": %s)",  
                               obj, obj->blueprint()->internal_name()); 
#endif 
      }
      // Remove Reference object from list
      *prev_next = next;
      // Clear the discovered_addr field so that the object does
      // not look like it has been discovered.
      *discovered_addr = NULL;
      NOT_PRODUCT(counter1++;)
    } else {
      prev_next = discovered_addr;
    }
    obj = next;
    assert(obj != first_seen, "cyclic ref_list found");
    NOT_PRODUCT(counter2++;)
  }
  NOT_PRODUCT(
    if (PrintGCDetails && PrintReferenceGC) {
      gclog_or_tty->print(" Dropped %d active Refs out of %d "
        "Refs in discovered list ", counter1, counter2);
    }
  )
}

void
ReferenceProcessorMT::process_discovered_reflist(oop* refs_list_addr,
			                ReferencePolicy *policy,
				        bool clear_referent) {

  if (!_processing_is_mt) {
    // Call the serial version implemented by super-class
    ReferenceProcessor::process_discovered_reflist(
      refs_list_addr, policy, clear_referent);
    return;
  }

  // The parallel version
  // Use CollectedHeap in general XXX FIX ME !!!
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  WorkGang* workers = gch->workers();
  assert(workers != NULL, "Need parallel worker threads");
  assert(_par_task != NULL, "No policy task?");
  assert(_num_q > 1, "Internal inconsistency");

  _par_task->set_ref_list(refs_list_addr);

  // Phase 1 (soft refs only):
  // . Traverse the list and remove any SoftReferences whose
  //   referents are not alive, but that should be kept alive for
  //   policy reasons. Keep alive the transitive closure of all
  //   such referents.
  if (policy != NULL) {
    _par_task->set_phase(AbstractRefProcTask::Phase1);
    _par_task->set_policy(policy);
    workers->run_task(_par_task);
    _par_task->reset();     // reset for next round, if any
  }

  // Phase 2:
  // . Traverse the list and remove any refs whose referents are alive.
  _par_task->set_phase(AbstractRefProcTask::Phase2);
  workers->run_task(_par_task);
  _par_task->reset();     // reset for next round, if any

  // Phase 3:
  // . Traverse the list and process referents as appropriate.
  _par_task->set_phase(AbstractRefProcTask::Phase3);
  _par_task->set_clear_ref(clear_referent);
  workers->run_task(_par_task);
  _par_task->reset();      // reset for next round, if any
}

oop* ReferenceProcessorMT::get_discovered_list(ReferenceType rt) {
  int id = 0;
  if (_processing_is_mt) {
    // Determine the queue index to use for this object.
    if (_discovery_is_mt) {
      // During a multi-threaded discovery phase,
      // each thread saves to its "own" list.
      Thread* thr = Thread::current();
      assert(thr->is_GC_task_thread(),
             "Dubious cast from Thread* to GangWorker*?");
      id = ((GangWorker*)thr)->id();
    } else {
      // single-threaded discovery, we save in round-robin
      // fashion to each of the lists.
      id = next_id();
    }
  }
  assert(0 <= id && id < _num_q, "Id is out-of-bounds (call Freud?)");

  // Get the discovered queue to which we will add
  oop* list = NULL;
  switch (rt) {
    case REF_OTHER:
      // Unknown reference type, no special treatment
      break;
    case REF_SOFT:
      list = &_discoveredSoftRefs[id];
      break;
    case REF_WEAK:
      list = &_discoveredWeakRefs[id];
      break;
    case REF_FINAL:
      list = &_discoveredFinalRefs[id];
      break;
    case REF_PHANTOM:
      list = &_discoveredPhantomRefs[id];
      break;
    case REF_NONE:
      // we should not reach here if we are an instanceRefKlass
    default:
      ShouldNotReachHere();
  }
  return list;
}

void ReferenceProcessorMT::add_to_discovered_list_mt(oop* list,
  oop obj, oop* discovered_addr) {
  assert(_discovery_is_mt, "!_discovery_is_mt should have been handled by caller");
  // First we must make sure this object is only enqueued once. CAS in a non null
  // discovered_addr.
  oop retest = (oop)Atomic::cmpxchg_ptr(*list, discovered_addr, NULL);
  if (retest == NULL) {
    // This thread just won the right to enqueue the object.
    if (processing_is_mt()) {
      // We have separate lists for enqueueing so no synchronization
      // is necessary.
      *list = obj;
    } else {
      // We now need to actually enqueue the object to a single list
      // to which other threads may be concurrently enqueueing;
      // we use a CAS to synchronize with other threads modifying
      // the list.
      oop current_head;
      do {
        current_head = *list;
        *discovered_addr = current_head;
      } while (Atomic::cmpxchg_ptr(obj, list, current_head) != current_head);
    }
  } else {
    // If retest was non NULL, another thread beat us to it:
    // The reference has already been discovered...
    if (TraceReferenceGC) {
#ifdef PRODUCT 
      gclog_or_tty->print_cr("Already enqueued reference (" INTPTR_FORMAT     ")",  
                             obj                                   ); 
#else 
      gclog_or_tty->print_cr("Already enqueued reference (" INTPTR_FORMAT ": %s)",  
                             obj, obj->blueprint()->internal_name()); 
#endif 
    }
  }
}
