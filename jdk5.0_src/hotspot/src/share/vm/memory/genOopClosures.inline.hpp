#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)genOopClosures.inline.hpp	1.24 03/12/23 16:41:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

OopsInGenClosure::OopsInGenClosure(Generation* gen) :
  OopClosure(gen->ref_processor()), _orig_gen(gen), _rs(NULL) {
  set_generation(gen);
}

void OopsInGenClosure::set_generation(Generation* gen) {
  _gen = gen;
  _gen_boundary = _gen->reserved().start();
  // Barrier set for the heap, must be set after heap is initialized
  if (_rs == NULL) {
    GenRemSet* rs = SharedHeap::heap()->rem_set();
    assert(rs->rs_kind() == GenRemSet::CardTable, "Wrong rem set kind");
    _rs = (CardTableRS*)rs;
  }
}

void OopsInGenClosure::do_barrier(oop* p) {
  assert(generation()->is_in_reserved(p), "expected ref in generation");
  oop obj = *p;
  assert(obj != NULL, "expected non-null object");
  // If p points to a younger generation, mark the card.
  if ((HeapWord*)obj < _gen_boundary) {
    _rs->inline_write_ref_field_gc(p, obj);
  }
}

// NOTE! Any changes made here should also be made
// in FastScanClosure::do_oop();
void ScanClosure::do_oop(oop* p) {
  oop obj = *p;
  // Should we copy the obj?
  if (obj != NULL) {
    if ((HeapWord*)obj < _boundary) {
      assert(!_g->to()->contains(obj), "Scanning field twice?");
      if (obj->is_forwarded()) {
        *p = obj->forwardee();
      } else {        
        *p = _g->copy_to_survivor_space(obj, p);
      }
    }
    if (_gc_barrier) {
      // Now call parent closure
      do_barrier(p);
    }
  }
}

void ScanClosure::do_oop_nv(oop* p) {
  ScanClosure::do_oop(p);
}

// NOTE! Any changes made here should also be made
// in ScanClosure::do_oop();
void FastScanClosure::do_oop(oop* p) {
  oop obj = *p;
  // Should we copy the obj?
  if (obj != NULL) {
    if ((HeapWord*)obj < _boundary) {
      assert(!_g->to()->contains(obj), "Scanning field twice?");
      if (obj->is_forwarded()) {
        *p = obj->forwardee();
      } else {        
        *p = _g->copy_to_survivor_space(obj, p);
      }
      if (_gc_barrier) {
        // Now call parent closure
        do_barrier(p);
      }
    }
  }
}

void FastScanClosure::do_oop_nv(oop* p) {
  FastScanClosure::do_oop(p);
}

// Note similarity to ScanClosure; the difference is that
// the barrier set is taken care of outside this closure.
void ScanWeakRefClosure::do_oop(oop* p) {
  oop obj = *p;
  assert (obj != NULL, "null weak reference?");
  // weak references are sometimes scanned twice; must check
  // that to-space doesn't already contain this object
  if ((HeapWord*)obj < _boundary && !_g->to()->contains(obj)) {
    if (obj->is_forwarded()) {
      *p = obj->forwardee();
    } else {        
      *p = _g->copy_to_survivor_space(obj, p);
    }
  }
}

void ScanWeakRefClosure::do_oop_nv(oop* p) {
  ScanWeakRefClosure::do_oop(p);
}

void TrainScanClosure::do_oop(oop* p) {
  oop obj = *p;
  // Should we copy the obj?
  if (obj != NULL) {
    if (_tg->car_table()->desc_for(obj)->target()) {
      if (obj->is_forwarded()) {
        *p = obj->forwardee();
      } else {
        assert(_tg->is_in(obj), "invalid ref");
        CarTableDesc* to_desc = _tg->car_table()->desc_for(obj);
        CarTableDesc* first_desc = _tg->first_car_desc();
        julong to_desc_train_number = to_desc->train_number();
        if (to_desc_train_number <= first_desc->train_number()) {
          if (to_desc_train_number ==
              CarTableDesc::special_nonoop_train_number) {
            to_desc->space()->set_marked(true);
          } else {
            assert(to_desc_train_number == first_desc->train_number(),
                   "invalid train number");
            _ref_to_first_train_found = true;
            if (first_desc->car_number() == to_desc->car_number()) {
              *p = _tg->copy_to_train(obj, p);
            }
          }
        }
      }
    }

    if (_rs_update) {
      // Update train remembered set if reference is in this generation.
      // This check is required whether the object moved or not; for
      // example, we might just be scanning followers.
      _tg->oop_update_remembered_set(p);
    }

    if (_gc_barrier) {
      // Scanning oops in an older generation, call parent closure
      do_barrier(p);
    }
  }
}

void TrainScanClosure::do_oop_nv(oop* p) {
  TrainScanClosure::do_oop(p);
}

// Barrier set taken care of outside this closure.
// Note similarity to TrainScanClosure
void TrainScanWeakRefClosure::do_oop(oop* p) {
  oop obj = *p;
  assert (obj != NULL, "null weak reference?");
  if (_tg->car_table()->desc_for(obj)->target()) {
    if (obj->is_forwarded()) {
      *p = obj->forwardee();
    } else {
      assert(_tg->is_in(obj), "invalid ref");
      CarTableDesc* to_desc = _tg->car_table()->desc_for(obj);
      CarTableDesc* first_desc = _tg->first_car_desc();
      julong to_desc_train_number = to_desc->train_number();
      if (to_desc_train_number <= first_desc->train_number()) {
        if (to_desc_train_number ==
            CarTableDesc::special_nonoop_train_number) {
          to_desc->space()->set_marked(true);
        } else {
          assert(to_desc_train_number == first_desc->train_number(),
                 "invalid train number");
          _ref_to_first_train_found = true;
          if (first_desc->car_number() == to_desc->car_number()) {
            *p = _tg->copy_to_train(obj, p);
          }
        }
      }
    }
  }
}

void TrainScanWeakRefClosure::do_oop_nv(oop* p) {
  TrainScanWeakRefClosure::do_oop(p);
}


#define UpdateTrainRSClosure_do_oop_DEFN(subclss, nv_suffix, clss)	\
void clss::do_oop(oop* p) {						\
  /* First, apply client closure */					\
  _cl->do_oop(p);							\
									\
  assert(_tg->is_in_reserved(p), "expected obj in train gen")		\
  _tg->oop_update_remembered_set(p);					\
}
TRAIN_SPECIALIZED_SINCE_SAVE_MARKS_CLOSURE_PAIRS(UpdateTrainRSClosure_do_oop_DEFN)


void UpdateTrainRSCacheFromClosure::do_oop(oop* p) {
  // First, apply client closure if there is one
  if (_cl)_cl->do_oop(p);

  assert(_tg->is_in_reserved(p), "expected obj in train gen")
  oop obj = *p;
  if (obj != NULL) {
    // Find car table descriptor for the object
    CarTableDesc* to_desc = _tg->car_table()->desc_for(obj);
    julong to_train = to_desc->train_number();
#ifdef ASSERT
    CarTableDesc* from_desc = _tg->car_table()->desc_for(p);
    assert(from_desc->train_number() == _from_train, "wrong train number");
    assert(from_desc->car_number() == _from_car, "wrong car number");
#endif
    if (to_train != CarTableDesc::invalid_train_number) {
      if (to_train < _from_train ||
          (to_train == _from_train && to_desc->car_number() < _from_car)) {
        // Update RS
        to_desc->space()->remembered_set()->add_reference(p);
      }
    }
  }
}

void ParScanClosure::par_do_barrier(oop* p) {
  assert(generation()->is_in_reserved(p), "expected ref in generation");
  oop obj = *p;
  assert(obj != NULL, "expected non-null object");
  // If p points to a younger generation, mark the card.
  if ((HeapWord*)obj < gen_boundary()) {
    rs()->write_ref_field_gc_par(p, obj);
  }
}

void ParScanClosure::do_oop_work(oop* p,
				 bool gc_barrier, bool only_two_gens,
				 bool root_scan,
				 bool jvmpi_slow_alloc) {
  oop obj = *p;
  assert((!Universe::heap()->is_in_reserved(p) ||
	  generation()->is_in_reserved(p))
	 && (generation()->level() == 0 || gc_barrier),
	 "The gen must be right, and we must be doing the barrier "
	 "in older generations.");
  if (obj != NULL) {
    if ((HeapWord*)obj < _boundary) {
      assert(!_g->to()->contains(obj), "Scanning field twice?");
      // OK, we need to ensure that it is copied.
      // We read the klass and mark in this order, so that we can reliably 
      // get the size of the object: if the mark we read is not a
      // forwarding pointer, then the klass is valid: the klass is only
      // overwritten with an overflow next pointer after the object is
      // forwarded.
      klassOop objK = obj->klass();
      markOop m = obj->mark();
      if (m->is_marked()) { // Contains forwarding pointer.
	*p = ParNewGeneration::real_forwardee(obj);
      } else {
        size_t obj_sz = obj->size_given_klass(objK->klass_part()); 
        *p = _g->copy_to_survivor_space(_par_scan_state, obj, obj_sz, m, 
					jvmpi_slow_alloc);
	if (root_scan) {
	  // This may have pushed an object.  If we have a root
	  // category with a lot of roots, can't let the queue get too
	  // full:
	  (void)_par_scan_state->trim_queues(10 * ParallelGCThreads);
	}
      }
      if (gc_barrier && only_two_gens) {
	// Now call parent closure
	par_do_barrier(p);
      }
    }
    if (gc_barrier && !only_two_gens) {
      // Now call parent closure
      par_do_barrier(p);
    }
  }
}


// Trim our work_queue so its length is below max at return
inline void Par_MarkRefsIntoAndScanClosure::trim_queue(uint max) {
  while (_work_queue->size() > max) {
    oop newOop;
    if (_work_queue->pop_local(newOop)) {
      assert(newOop->is_oop(), "Expected an oop");
      assert(_bit_map->isMarked((HeapWord*)newOop),
             "only grey objects on this stack");
      // iterate over the oops in this oop, marking and pushing
      // the ones in CMS heap (i.e. in _span).
      newOop->oop_iterate(&_par_pushAndMarkClosure);
    }
  }
}
