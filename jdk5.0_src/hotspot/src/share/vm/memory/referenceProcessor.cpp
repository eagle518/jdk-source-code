#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)referenceProcessor.cpp	1.31 04/01/13 12:42:57 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_referenceProcessor.cpp.incl"

oop  ReferenceProcessor::_sentinelRef = NULL;

const int subclasses_of_ref = REF_PHANTOM - REF_OTHER;

void referenceProcessor_init() {
  ReferenceProcessor::init_statics();
}

void ReferenceProcessor::init_statics() {
  assert(_sentinelRef == NULL, "should be initialized precsiely once");
  EXCEPTION_MARK;
  _sentinelRef = instanceKlass::cast(
                   SystemDictionary::object_klass())->
                     allocate_permanent_instance(THREAD);

  // Initialize the master soft ref clock.
  java_lang_ref_SoftReference::set_clock(os::javaTimeMillis());

  if (HAS_PENDING_EXCEPTION) {
      Handle ex(THREAD, PENDING_EXCEPTION);
      vm_exit_during_initialization(ex);
  }
  assert(_sentinelRef != NULL && _sentinelRef->is_oop(),
         "Just constructed it!");
  guarantee(RefDiscoveryPolicy == ReferenceBasedDiscovery ||
            RefDiscoveryPolicy == ReferentBasedDiscovery,
            "Unrecongnized RefDiscoveryPolicy");
}

ReferenceProcessor::ReferenceProcessor(MemRegion span,
  bool atomic_discovery, bool mt_discovery, int mt_degree) :
  _discovering_refs(false),
  _enqueuing_is_done(false),
  _is_alive_non_header(NULL),
  _notify_ref_lock(false),
  _policy(NULL),
  _is_alive(NULL), _keep_alive(NULL), _complete_gc(NULL) {
  _span = span;
  _discovery_is_atomic = atomic_discovery;
  _discovery_is_mt     = mt_discovery;
  _num_q               = mt_degree;
  _discoveredSoftRefs  = NEW_C_HEAP_ARRAY(oop, _num_q * subclasses_of_ref);
  if (_discoveredSoftRefs == NULL) {
    vm_exit_during_initialization("Could not allocated RefProc Array");
  }
  _discoveredWeakRefs    = &_discoveredSoftRefs[_num_q];
  _discoveredFinalRefs   = &_discoveredWeakRefs[_num_q];
  _discoveredPhantomRefs = &_discoveredFinalRefs[_num_q];
  assert(_sentinelRef != NULL, "_sentinelRef is NULL");
  // Initialized all entries to _sentinelRef
  for (int i = 0; i < _num_q * subclasses_of_ref; i++) {
    _discoveredSoftRefs[i] = _sentinelRef;
  }
}

#ifndef PRODUCT
void ReferenceProcessor::verify_no_references_recorded() {
  guarantee(!_discovering_refs, "Discovering refs?");
  for (int i = 0; i < _num_q * subclasses_of_ref; i++) {
    guarantee(_discoveredSoftRefs[i] == _sentinelRef,
              "Found non-empty discovered list");
  }
}
#endif

void ReferenceProcessor::oops_do(OopClosure* f) {
  for (int i = 0; i < _num_q * subclasses_of_ref; i++) {
    f->do_oop((oop*)&_discoveredSoftRefs[i]);
  }
}

void ReferenceProcessor::oops_do_statics(OopClosure* f) {
  f->do_oop((oop*)&_sentinelRef);
}

void
ReferenceProcessor::process_discovered_references() {
  NOT_PRODUCT(verify_ok_to_handle_reflists());

  assert(!enqueuing_is_done(), "If here enqueuing should not be complete");
  // Stop treating discovered references specially.
  disable_discovery();

  // Soft references
  {
    NOT_PRODUCT(TraceTime("SoftReference", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    process_discovered_reflist(_discoveredSoftRefs, _policy, true);
  }

  {
    // Update (advance) the soft ref master clock field. This must be done
    // after processing the soft ref list.
    jlong now = os::javaTimeMillis();
    jlong clock = java_lang_ref_SoftReference::clock();
    NOT_PRODUCT(
      if (now < clock) {
        warning("time warp: %d to %d", clock, now);
      }
    )
    // In product mode, protect ourselves from system time being adjusted
    // externally and going backward; see note in the implementation of
    // GenCollectedHeap::time_since_last_gc() for the right way to fix
    // this uniformly throughout the VM; see bug-id 4741166. XXX
    if (now > clock) {
      java_lang_ref_SoftReference::set_clock(now);
    }
    // Else leave clock stalled at its old value until time progresses
    // past clock value.
  }
  
  // Weak references
  {
    NOT_PRODUCT(TraceTime("WeakReference", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    process_discovered_reflist(_discoveredWeakRefs, NULL, true);
  }

  // Final references
  {
    NOT_PRODUCT(TraceTime("FinalReference", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    process_discovered_reflist(_discoveredFinalRefs, NULL, false);
  }

  // Phantom references
  {
    NOT_PRODUCT(TraceTime("PhantomReference", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    process_discovered_reflist(_discoveredPhantomRefs, NULL, false);
  }

  // Weak global JNI references. It would make more sense (semantically) to
  // traverse these simultaneously with the regular weak references above, but
  // that is not how the JDK1.2 specification is. See #4126360. Native code can
  // thus use JNI weak references to circumvent the phantom references and
  // resurrect a "post-mortem" object.
  { 
    NOT_PRODUCT(TraceTime("JNI Weak Reference", PrintGCDetails && PrintReferenceGC,
              false, gclog_or_tty);)
    process_phaseJNI();
  }
}

void ReferenceProcessor::process_phaseJNI() {
  JNIHandles::weak_oops_do(_is_alive, _keep_alive);
  // Finally remember to keep sentinel around
  _keep_alive->do_oop(&_sentinelRef);
  _complete_gc->do_void();
}


bool ReferenceProcessor::enqueue_discovered_references() {
  NOT_PRODUCT(verify_ok_to_handle_reflists());
  // Remember old value of pending references list
  oop* pending_list_addr = java_lang_ref_Reference::pending_list_addr();
  oop old_pending_list_value = *pending_list_addr;

  // Enqueue references that are not made active again, and
  // clear the decks for the next collection (cycle).
  enqueue_discovered_reflists(pending_list_addr);
  // Do the oop-check on pending_list_addr missed in
  // enqueue_discovered_reflist. We should probably
  // do a raw oop_check so that future such idempotent
  // oop_stores relying on the oop-check side-effect
  // may be elided automatically and safely without
  // affecting correctness.
  oop_store(pending_list_addr, *(pending_list_addr));

  // Stop treating discovered references specially.
  disable_discovery();

  // Return true if new pending references were added
  return old_pending_list_value != *pending_list_addr;
}

void ReferenceProcessor::enqueue_discovered_reflist(oop refs_list,
  oop* pending_list_addr) {
  // Given a list of refs linked through the "discovered" field
  // (java.lang.ref.Reference.discovered) chain them through the
  // "next" field (java.lang.ref.Reference.next) and prepend
  // to the pending list.
  if (TraceReferenceGC && PrintGCDetails) {
    gclog_or_tty->print_cr("ReferenceProcessor::enqueue_discovered_reflist list "
                           INTPTR_FORMAT, refs_list);
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
      gclog_or_tty->print_cr("	obj " INTPTR_FORMAT "/next " INTPTR_FORMAT,
                             obj, next);
    }
    assert(*java_lang_ref_Reference::next_addr(obj) == NULL, 
      "The reference should not be enqueued");
    if (next == _sentinelRef) {  // obj is last
      // Swap refs_list into pendling_list_addr and
      // set obj's next to what we read from pending_list_addr.
      oop old = *pending_list_addr;
      *pending_list_addr = refs_list;
      // Need oop_check on pending_list_addr above;
      // see special oop-check code at the end of
      // enqueue_discovered_reflists() further below.
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
void ReferenceProcessor::enqueue_discovered_reflists(oop* pending_list_addr) {
  for (int i = 0; i < _num_q * subclasses_of_ref; i++) {
    enqueue_discovered_reflist(_discoveredSoftRefs[i],
                               pending_list_addr);
    _discoveredSoftRefs[i] = _sentinelRef;
  }
}

// (SoftReferences only) Traverse the list and remove any SoftReferences whose
// referents are not alive, but that should be kept alive for policy reasons.
// Keep alive the transitive closure of all such referents.
void
ReferenceProcessor::process_phase1(oop* refs_list_addr,
                                   ReferencePolicy *policy,
                                   BoolObjectClosure* is_alive,
                                   OopClosure* keep_alive,
                                   VoidClosure* complete_gc) {

  oop* prev_next = refs_list_addr;
  oop obj = *refs_list_addr;
  DEBUG_ONLY(oop first_seen = obj;) // cyclic linked list check
  NOT_PRODUCT(size_t counter1 = 0;)
  NOT_PRODUCT(size_t counter2 = 0;)

  assert(policy != NULL, "Must have a non-NULL policy");
  // Decide which softly reachable refs should be kept alive.
  while (obj != _sentinelRef) {
    oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
    assert(discovered_addr && (*discovered_addr)->is_oop_or_null(),
      "discovered field is bad");
    oop next = *discovered_addr;
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
      "Wrong oop found in java.lang.Reference object");
    bool referent_is_dead = !is_alive->do_object_b(referent);
    if (referent_is_dead && !policy->should_clear_reference(obj)) {
      if (TraceReferenceGC) {
#ifdef PRODUCT 
        gclog_or_tty->print_cr("Dropping reference (" INTPTR_FORMAT     ") by policy",  
                               obj                                   ); 
#else 
        gclog_or_tty->print_cr("Dropping reference (" INTPTR_FORMAT ": %s) by policy",  
                               obj, obj->blueprint()->internal_name()); 
#endif 
      }
      // Make the Reference object active again
      java_lang_ref_Reference::set_next(obj, NULL);
      keep_alive->do_oop(referent_addr);    // keep the referent around
      // Remove Reference object from list
      *discovered_addr = NULL;
      *prev_next = next;
      NOT_PRODUCT(counter1++);
    } else {
      prev_next = discovered_addr;
    }
    obj = next;
    assert(obj != first_seen, "cyclic ref_list found");
    NOT_PRODUCT(counter2++);
  }
  // Close the reachable set
  complete_gc->do_void();
  NOT_PRODUCT(
    if (PrintGCDetails && TraceReferenceGC) {
      gclog_or_tty->print(" Dropped %d dead Refs out of %d "
        "discovered Refs by policy ", counter1, counter2);
    }
  )
}

// Traverse the list and remove any refs whose referents are alive.
void
ReferenceProcessor::process_phase2(oop* refs_list_addr,
                                   BoolObjectClosure* is_alive,
                                   OopClosure* keep_alive) {
  oop* prev_next = refs_list_addr;
  oop obj = *refs_list_addr;
  DEBUG_ONLY(oop first_seen = obj;) // cyclic linked list check
  NOT_PRODUCT(size_t counter1 = 0;)
  NOT_PRODUCT(size_t counter2 = 0;)

  while (obj != _sentinelRef) {
    oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
    assert(discovered_addr && (*discovered_addr)->is_oop_or_null(),
      "discovered field is bad");
    oop next = *discovered_addr;
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
      "Wrong oop found in java.lang.Reference object");
    if (is_alive->do_object_b(referent)) {
      if (TraceReferenceGC) {
#ifdef PRODUCT 
        gclog_or_tty->print_cr("Dropping strongly reachable reference (" INTPTR_FORMAT     ")",  
                               obj                                   ); 
#else 
        gclog_or_tty->print_cr("Dropping strongly reachable reference (" INTPTR_FORMAT ": %s)",  
                               obj, obj->blueprint()->internal_name()); 
#endif 
      }
      // The referent is reachable after all, make the Reference object active again
      java_lang_ref_Reference::set_next(obj, NULL);
      // Update the referent pointer as necessary: Note that this
      // should not entail any recursive marking because the
      // referent must already have been traversed.
      keep_alive->do_oop(referent_addr);
      assert(obj->is_oop(UseConcMarkSweepGC), "Dropping a bad reference");
      assert(referent->is_oop(UseConcMarkSweepGC), "Dropping a bad referent");
      // Remove Reference object from list
      *prev_next = next;
      // Clear the discovered_addr field so that the object doe
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
    if (PrintGCDetails && TraceReferenceGC) {
      gclog_or_tty->print(" Dropped %d active Refs out of %d "
        "Refs in discovered list ", counter1, counter2);
    }
  )
}

// Traverse the list and process the referents, by either
// either clearing them or keeping them (and their reachable
// closure) alive.
void
ReferenceProcessor::process_phase3(oop* refs_list_addr,
                                   bool clear_referent,
                                   BoolObjectClosure* is_alive,
                                   OopClosure* keep_alive,
                                   VoidClosure* complete_gc) {
  oop* prev_next = refs_list_addr;
  oop obj = *refs_list_addr;
  DEBUG_ONLY(oop first_seen = obj;) // cyclic linked list check

  while (obj != _sentinelRef) {
    keep_alive->do_oop(prev_next);
    prev_next = java_lang_ref_Reference::discovered_addr(obj);
    assert(prev_next && (*prev_next)->is_oop_or_null(),
      "discovered field is bad");
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
           "Wrong oop found in java.lang.Reference object");
    if (clear_referent) {
      // NULL out referent pointer
      *referent_addr = NULL;
    } else {
      // keep the referent around
      keep_alive->do_oop(referent_addr);
    }
    if (TraceReferenceGC) {
#ifdef PRODUCT
      gclog_or_tty->print_cr("Adding %sreference (" INTPTR_FORMAT     ") as pending", 
                             clear_referent ? "cleared " : "",  
                             obj                                   ); 
#else 
      gclog_or_tty->print_cr("Adding %sreference (" INTPTR_FORMAT ": %s) as pending",  
                             clear_referent ? "cleared " : "",  
                             obj, obj->blueprint()->internal_name()); 
#endif 
    }
    assert(obj->is_oop(UseConcMarkSweepGC), "Adding a bad reference");
    assert(referent->is_oop(UseConcMarkSweepGC), "Adding a bad referent");
    obj = *prev_next;
    assert(obj != first_seen, "cyclic ref_list found");
  }
  // Remember to keep sentinel pointer around
  keep_alive->do_oop(prev_next);
  // Close the reachable set
  complete_gc->do_void();
}

void
ReferenceProcessor::process_discovered_reflist(oop* refs_list_addr,
			                ReferencePolicy *policy,
				        bool clear_referent) {
  int i;

  // (SoftReferences only) Traverse the list and remove any SoftReferences whose
  // referents are not alive, but that should be kept alive for policy reasons.
  // Keep alive the transitive closure of all such referents.
  if (policy != NULL) {
    for (i = 0; i < _num_q; i++) {
      process_phase1(&refs_list_addr[i], policy,
                     _is_alive, _keep_alive, _complete_gc);
    }
  }

  // . Traverse the list and remove any refs whose referents are alive.
  for (i = 0; i < _num_q; i++) {
    process_phase2(&refs_list_addr[i], _is_alive, _keep_alive);
  }

  // . Traverse the list and process referents as appropriate.
  for (i = 0; i < _num_q; i++) {
    process_phase3(&refs_list_addr[i], clear_referent,
                   _is_alive, _keep_alive, _complete_gc);
  }
}

void ReferenceProcessor::delete_null_referents_from_lists() {
  // loop over the lists
  for (int i = 0; i < _num_q * subclasses_of_ref; i++) {
    if (TraceReferenceGC && PrintGCDetails && ((i % _num_q) == 0)) {
      gclog_or_tty->print_cr(
        "\nScrubbing %s discovered list of Null referents",
        list_name(i));
    }
    delete_null_referents(&_discoveredSoftRefs[i]);
  }
}

void ReferenceProcessor::delete_null_referents(oop* refs_list_addr) {
  oop* prev_next = refs_list_addr;
  oop obj = *refs_list_addr;
  DEBUG_ONLY(oop first_seen = obj;) // cyclic linked list check
  NOT_PRODUCT(size_t counter1 = 0;)
  NOT_PRODUCT(size_t counter2 = 0;)
  while (obj != _sentinelRef) {
    oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
    assert(discovered_addr && (*discovered_addr)->is_oop_or_null(),
      "discovered field is bad");
    oop next = *discovered_addr;
    oop* referent_addr = java_lang_ref_Reference::referent_addr(obj);
    oop referent = *referent_addr;
    assert(Universe::heap()->is_in_or_null(referent),
      "Wrong oop found in java.lang.Reference object");
    if (referent == NULL) {
      // If the reference is being discovered, then it
      // should be active.  Assert that.
      debug_only(
        oop* next_addr = java_lang_ref_Reference::next_addr(obj);
	if ((*next_addr != NULL) && PrintGCDetails && TraceReferenceGC) {
	  gclog_or_tty->print_cr("delete_null_referents: Reference: " 
	    INTPTR_FORMAT " and next field: " INTPTR_FORMAT 
	    " with NULL referent", 
	    obj, *next_addr);
	}
        assert(*next_addr == NULL, "Reference should not be active");
      )
      // Remove Reference object from list
      *prev_next = next;
      // Clear the discovered_addr field so that the object does
      // not look like it has been discovered.
      *discovered_addr = NULL;
      NOT_PRODUCT(counter1++);
    } else {
      prev_next = discovered_addr;
    }
    obj = next;
    NOT_PRODUCT(counter2++);
    assert(obj != first_seen, "cyclic ref_list found");
  }
  NOT_PRODUCT(
    if (PrintGCDetails && TraceReferenceGC) {
      gclog_or_tty->print(
	" Removed %d Refs with NULL referents out of %d discovered Refs", 
	counter1, counter2);
    }
  )
}

oop* ReferenceProcessor::get_discovered_list(ReferenceType rt) {
  // Get the discovered queue to which we will add
  oop* list;
  switch (rt) {
    case REF_OTHER:
      list = NULL;   // Unknown reference type, no special treatment
      break;
    case REF_SOFT:
      list = _discoveredSoftRefs;
      break;
    case REF_WEAK:
      list = _discoveredWeakRefs;
      break;
    case REF_FINAL:
      list = _discoveredFinalRefs;
      break;
    case REF_PHANTOM:
      list = _discoveredPhantomRefs;
      break;
    case REF_NONE:
      // we should not reach here if we are an instanceRefKlass
    default:
      ShouldNotReachHere();
  }
  return list;
}

void ReferenceProcessor::add_to_discovered_list_mt(oop* list, oop obj,
  oop* discovered_addr) {
  assert(_discovery_is_mt, "!_dicovery_is_mt handled at caller");
  // First we must make sure this object is only enqueued once. CAS in a non null
  // discovered_addr.
  oop retest = (oop)Atomic::cmpxchg_ptr(*list, discovered_addr, NULL);
  if (retest == NULL) {
    // This thread just won the right to enqueue the object.
    // We now need to actually enqueue the object to a single list
    // to which other threads may be concurrently enqueueing;
    // we use a CAS to synchronize with other threads modifying
    // the list.
    oop current_head;
    do {
      current_head = *list;
      *discovered_addr = current_head;
    } while (Atomic::cmpxchg_ptr(obj, list, current_head) != current_head);
  } else {
    // If retest was non NULL, another thread beat
    // us to it: the reference has already been discovered...
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


// We mention two of several possible choices here:
// #0: if the reference object is not in the "originating generation"
//     (or part of the heap being collected, indicated by our "span"
//     we don't treat it specially (i.e. we scan it as we would
//     a normal oop, treating its references as strong references).
//     This means that references can't be enqueued unless their
//     referent is also in the same span. This is the simplest,
//     most "local" and most conservative approach, albeit one
//     that may cause weak references to be enqueued least promptly.
//     We call this choice the "ReferenceBasedDiscovery" policy.
// #1: the reference object may be in any generation (span), but if
//     the referent is in the generation (span) being currently collected
//     then we can discover the reference object, provided
//     the object has not already been discovered by
//     a different concurrently running collector (as may be the
//     case, for instance, if the reference object is in CMS and
//     the referent in DefNewGeneration), and provided the processing
//     of this reference object by the current collector will
//     appear atomic to every other collector in the system.
//     (Thus, for instance, a concurrent collector may not
//     discover references in other generations even if the
//     referent is in its own generation). This policy may,
//     in certain cases, enqueue references somewhat sooner than
//     might Policy #0 above, but at marginally increased cost
//     and complexity in processing these references.
//     We call this choice the "RefeferentBasedDiscovery" policy.
bool ReferenceProcessor::discover_reference(oop obj, ReferenceType rt) {
  // We enqueue references only if we are discovering refs
  // (rather than processing discovered refs).
  if (!_discovering_refs || !RegisterReferences) {
    return false;
  }
  // We only enqueue active references.
  oop* next_addr = java_lang_ref_Reference::next_addr(obj);
  if (*next_addr != NULL) {
    return false;
  }

  HeapWord* obj_addr = (HeapWord*)obj;
  if (RefDiscoveryPolicy == ReferenceBasedDiscovery &&
      !_span.contains(obj_addr)) {
    // Reference is not in the originating generation;
    // don't treat it specially (i.e. we want to scan it as a normal
    // object with strong references).
    return false;
  }

  // We only enqueue references whose referents are not (yet) strongly
  // reachable.
  if (_is_alive_non_header != NULL) {
    oop referent = java_lang_ref_Reference::referent(obj);
    // We'd like to assert the following:
    // assert(referent != NULL, "Refs with null referents already filtered");
    // However, since this code may be executed concurrently with
    // mutators, which can clear() the referent, it is not
    // guaranteed that the referent is non-NULL.
    if (_is_alive_non_header->do_object_b(referent)) {
      return false;  // referent is reachable
    }
  }

  oop* discovered_addr = java_lang_ref_Reference::discovered_addr(obj);
  assert(discovered_addr != NULL && (*discovered_addr)->is_oop_or_null(),
         "bad discovered field");
  if (*discovered_addr != NULL) {
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
    if (RefDiscoveryPolicy == ReferentBasedDiscovery) {
      // assumes that an object is not processed twice;
      // if it's been already discovered it must be on another
      // generation's discovered list; so we won't discover it.
      return false;
    } else {
      assert(RefDiscoveryPolicy == ReferenceBasedDiscovery,
             "Unrecognized policy");
      // check assumption that an object is not potentially
      // discovered twice except by collectors whose discovery is
      // not atomic wrt other collectors in the configuration (for
      // instance, a concurrent collector).
      assert(!discovery_is_atomic(),
             "Only non-atomic collectors are allowed to even "
             "attempt rediscovery in their own span");
      return true;
    }
  }

  if (RefDiscoveryPolicy == ReferentBasedDiscovery) {
    oop referent = java_lang_ref_Reference::referent(obj);
    assert(referent->is_oop(), "bad referent");
    // enqueue if and only if either:
    // reference is in our span or
    // we are an atomic collector and referent is in our span
    if (_span.contains(obj_addr) ||
        (discovery_is_atomic() && _span.contains(referent))) {
      // should_enqueue = true;
    } else {
      return false;
    }
  } else {
    assert(RefDiscoveryPolicy == ReferenceBasedDiscovery &&
           _span.contains(obj_addr), "code inconsistency");
  }

  // Get the right type of discovered queue head.
  oop* list = get_discovered_list(rt);
  if (list == NULL) {
    return false;   // nothing special needs to be done
  }

  // We do a raw store here, the field will be visited later when
  // processing the discovered references. 
  if (_discovery_is_mt) {
    // virtual call
    add_to_discovered_list_mt(list, obj, discovered_addr);
  } else {
    *discovered_addr = *list;
    *list = obj;
  }

  // In the MT discovery case, it is currently possible to see
  // the following message multiple times if several threads
  // discover a reference about the same time. Only one will
  // however have actually added it to the disocvered queue.
  // One could let add_to_discovered_list_mt() return an
  // indication for success in queueing (by 1 thread) or
  // failure (by all other threads), but I decided the extra
  // code was not worth the effort for something that is
  // only used for debugging support.
  if (TraceReferenceGC) {
    oop referent = java_lang_ref_Reference::referent(obj);
    if (PrintGCDetails) {
#ifdef PRODUCT 
      gclog_or_tty->print_cr("Enqueued reference (" INTPTR_FORMAT     ")",  
                             obj                                   ); 
#else 
      gclog_or_tty->print_cr("Enqueued reference (" INTPTR_FORMAT ": %s)",  
                             obj, obj->blueprint()->internal_name()); 
#endif 
    }
    assert(referent->is_oop(), "Enqueued a bad referent");
  }
  assert(obj->is_oop(), "Enqueued a bad reference");
  return true;
}

const char* ReferenceProcessor::list_name(int i) {
   assert(i >= 0 && i <= _num_q * subclasses_of_ref, "Out of bounds index");
   int j = i / _num_q;
   switch (j) {
     case 0: return "SoftRef";
     case 1: return "WeakRef";
     case 2: return "FinalRef";
     case 3: return "PhantomRef";
   }
   ShouldNotReachHere();
   return NULL;
}

#ifndef PRODUCT
void ReferenceProcessor::verify_ok_to_handle_reflists() {
  // empty for now
}
#endif PRODUCT
