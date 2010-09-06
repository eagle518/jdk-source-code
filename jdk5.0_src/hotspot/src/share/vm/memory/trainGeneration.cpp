#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)trainGeneration.cpp	1.43 04/02/05 12:49:18 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_trainGeneration.cpp.incl"


/////////////////////////////////////////////////////////////////////
// TrainGeneration
/////////////////////////////////////////////////////////////////////

#define FOR_EACH_TRAIN(t) \
  for (Train* t = first_train(); t != NULL; t = t->next_train())


#define FOR_EACH_TRAIN_REVERSE(t) \
  for (Train* t = last_train(); t != NULL; t = t->prev_train())

TrainGeneration::TrainGeneration(ReservedSpace rs, 
                                 size_t initial_byte_size,
                                 int level,
				 CardTableRS* ct) :
  CardGeneration(rs, initial_byte_size, level, ct),
  _car_tab(Generation::LogOfGenGrain, 
           GenCollectedHeap::heap()->reserved_region()),
  _train_alloc_lock(Mutex::nonleaf, "Train gen par_alloc", true)
{
  _ct = ct;

  // Clear free lists
  set_car_free_list(NULL);
  set_train_free_list(NULL);
  set_special_nonoop_train(NULL);
  _total_promoted  = 0;
  _total_processed = 0;
  _delta_promoted  = 0;
  _delta_processed = 0;

  reset_invocation_rate();

#ifndef PRODUCT
  _starting_length = 1;
  _process_count = 0;
#endif

  // Put all the committed space in the car free list
  HeapWord* low  = (HeapWord*)_virtual_space.low();
  HeapWord* high = (HeapWord*)_virtual_space.high();
  MemRegion mr = MemRegion(low, high);

  size_t blocks = CarSpace::car_block_size(mr.word_size());
  CarSpace* firstcar = new CarSpace(this, _ct, _bts, mr, blocks,
                                    _car_tab.desc_for(low));
  add_to_car_pool(firstcar, false);

  // Make initial train consisting of one initial car
  set_first_train(retrieve_from_train_pool(CarSpace::car_size_in_words(),
					   CarTableDesc::initial_train_number));
  assert(first_train() != NULL, "Should be able to get a train at VM startup.");
  set_last_train(first_train());
  set_first_car_desc();

  // initialize performance counters

  const char* gen_name = "old";

  // Generation Counters -- generation 1, 1 subspace
  _gen_counters = new GenerationCounters(gen_name, 1, 1, &_virtual_space);

  _gc_counters = new CollectorCounters("Train", 1);

  const char* ns = _gen_counters->name_space();
  _space_counters = new GSpaceCounters(gen_name, 0,
                                       _virtual_space.reserved_size(),
                                       this, _gen_counters, false);
}

void TrainGeneration::gc_epilogue(bool full) {
  // update generation and space performance counters
  update_counters();
  assert(!UsePerfData || (_space_counters->used() == used()),
         "counter value and reality should match");
}


HeapWord* TrainGeneration::allocate(size_t size, bool is_large_noref, bool is_tlab) {
  HeapWord* obj = NULL;
  assert(!is_tlab, "TrainGeneration does not support TLAB allocation");
  if (is_large_noref && UseSpecialLargeObjectHandling) {
    assert(size >= Universe::heap()->large_typearray_limit(), "should be large type array");
    if (_special_nonoop_train == NULL) {
      GCMutexLocker ml(ExpandHeap_lock);
      set_special_nonoop_train(
         retrieve_from_train_pool(size, 
                                  CarTableDesc::special_nonoop_train_number));
      if (_special_nonoop_train != NULL) {
        update_capacity_counters();
        if (TraceSpecialLargeObjectHandling) {
          gclog_or_tty->print_cr("Allocated large type array train");
        }
      }
    }
    if (_special_nonoop_train != NULL) {
      // allocate can expand the heap
      GCMutexLocker ml(ExpandHeap_lock);
      obj = _special_nonoop_train->allocate(size);
    }
    if (obj != NULL) {
      CarTableDesc* desc = _car_tab.desc_for(obj);
      assert(desc->train_number() == CarTableDesc::special_nonoop_train_number,
              "just checking");
      if (TraceSpecialLargeObjectHandling) {
        gclog_or_tty->print("Allocated large type array " INTPTR_FORMAT " (" SIZE_FORMAT " bytes)",
		   obj, size*wordSize);
        desc->space()->print_short_on(gclog_or_tty);
        tty->cr();
      }
    }
  }
  return obj != NULL ? obj : last_train()->allocate_within(size);
}

HeapWord* TrainGeneration::par_allocate(size_t size, bool is_large_noref,
					bool is_tlab) {
  MutexLocker x(&_train_alloc_lock);
  return allocate(size, is_large_noref, is_tlab);
}

oop TrainGeneration::promote(oop obj, size_t obj_size, oop* ref) {
  assert(obj_size == (size_t)obj->size(), "bad obj_size passed in");
  // Update total_promoted even though obj may not wind up in this gen.  The
  // desired count is the number of words promoted by the younger gen.
  _total_promoted += obj_size;
  HeapWord* res;
  bool largeTypeArray = obj->is_typeArray() && 
    obj_size >= Universe::heap()->large_typearray_limit();
  if (largeTypeArray) {
    res = allocate(obj_size, true, false);
  } else {
    // ag has some qualms about this strategy -- it doesn't
    // guarantee that the object gets copied to the oldest referencing
    // train...(-- dld 2/9/00)
    Train* t = NULL;
    if (ref != NULL && is_in_reserved(ref))
      t = _car_tab.train_for(ref);
    if (t == NULL) {
      res = allocate(obj_size, false, false);
    } else {
      res = t->allocate(obj_size);
    }
  }
  if (res == NULL) {
    res = expand_and_allocate(obj_size, largeTypeArray, false);
  }

  if (res != NULL) {
    Copy::aligned_disjoint_words((HeapWord*)obj, res, obj_size);
    return oop(res);
  } else {
    Generation* next = next_gen();
    if (next != NULL) {
      return next->promote(obj, obj_size, ref);
    }
  }
  return NULL;
}

void TrainGeneration::set_first_car_desc() {
  _first_car_desc = first_train()->first_car()->desc();
}


Train* TrainGeneration::add_new_highest_train(size_t size) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  Train* old_last_train = last_train();
  Train* new_last_train = retrieve_from_train_pool(size, 
                                             last_train()->train_number() + 1);
  if (new_last_train != NULL) {
    set_last_train(new_last_train);
    old_last_train->set_next_train(new_last_train);
    new_last_train->set_prev_train(old_last_train);

    // update the space and generation capacity counters
    update_capacity_counters();
  }
  return new_last_train;
}


size_t TrainGeneration::release_train(Train* t) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  if (t == first_train()) {
    // releasing first train
    Train* new_first_train = t->next_train();
    set_first_train(new_first_train);
    if (new_first_train != NULL) {
      new_first_train->set_prev_train(NULL);
      set_first_car_desc();
    }
    // else this is the only train and a new train will be
    // allocated below (setting _first_train, _last_train, etc.).
  } else if (t == last_train()) {
    // releasing last train
    Train* new_last_train = t->prev_train();
    set_last_train(new_last_train);
    assert(new_last_train != NULL, "Train being releases was not the first train.");
    new_last_train->set_next_train(NULL);
  } else {
    // releasing interior train
    assert(t->next_train() != NULL && t->prev_train() != NULL, "just checking");
    t->next_train()->set_prev_train(t->prev_train());
    t->prev_train()->set_next_train(t->next_train());
  }

  size_t result =  add_to_train_pool(t); // will release all cars as well

  // Allocate a new first train (if needed) after the release of train t to guarantee
  // the allocation of a new train.  A train can generally be allocated (given space in
  // the C heap) but if there is no space available in the TrainGeneration heap, 
  // retrieve_from_train_pool() will return NULL.
  if (first_train() == NULL) {
    Train* new_first_train = retrieve_from_train_pool(
						 CarSpace::car_size_in_words(), 
                                                 CarTableDesc::initial_train_number);
    // It is expected that a train successfully will be retrieved since one was
    // just released, but defensively check.
    if (new_first_train != NULL) {
      set_last_train(new_first_train);
      set_first_train(new_first_train);
      new_first_train->set_prev_train(NULL);
      set_first_car_desc();
      new_first_train->set_next_train(NULL);
    }
  }

  assert(first_train() != NULL, "There should always be at least one train");
  return result;
}

HeapWord* TrainGeneration::expand_and_allocate(size_t size,
                                               bool is_large_noref,
                                               bool is_tlab,
					       bool parallel) {
  assert(!is_tlab, "TrainGeneration does not support TLAB allocation");
  GCMutexLocker x(ExpandHeap_lock);
  if (parallel) {
    MutexLocker x(ParGCRareEvent_lock);
    Train* t = add_new_highest_train(size);
    return t != NULL ? t->allocate_within(size) : NULL;
  } else {
    Train* t = add_new_highest_train(size);
    return t != NULL ? t->allocate_within(size) : NULL;
  }
}

size_t TrainGeneration::release_lowest_car() {
  assert_locked_or_safepoint(ExpandHeap_lock);
  CarSpace* first_car = first_train()->first_car();
  CarSpace* next_car = first_car->next_car();

  // Return value: the amount of space released
  size_t used = 0;

  // Collection sets will contain one and only one oversized car
  // or we need mark the relinked car in the car itself.
  assert(_relinked_oversized_car == NULL ||
         _relinked_oversized_car == first_car, "relink car confusion");

  if (_relinked_oversized_car != NULL) {
    // We need to relink the car to the end of _relinked_train
    Train* train = _relinked_train;
    _car_tab.update_entry(first_car,
                          train->train_number(),
                          train->last_car()->car_number()+1,
                          train);
    // Reset remembered set, should already be empty. This
    // also resets the size of the rs.
    first_car->remembered_set()->reinitialize(true);

    // XX If we change the relink to occur earlier, after the
    //    notion of "collection set" is changed, we can avoid this.
    first_car->reset_saved_mark();

    train->last_car()->set_next_car(first_car);
    train->set_last_car(first_car);

    first_car->set_next_car(NULL);
  }

  if (next_car == NULL) {
    used = release_lowest_train();
  } else {
    used = first_car->used();
    first_train()->set_first_car(next_car);
    add_to_car_pool(first_car, true);
    set_first_car_desc();
  }

  return used;
}


void TrainGeneration::coalesce_cars(CarSpace* c1, CarSpace* c2) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  assert(c1 && c2 && c1->next_car() == c2, "just checking");
  assert(c1->bottom() < c2->bottom(), "just checking");
  // merge c1 and c2 if they are adjacent
  if (c1->end() == c2->bottom()) {
    c1->set_next_car(c2->next_car());
    c1->set_blocks(c1->blocks() + c2->blocks());
    c1->set_end(c2->end());
    delete c2;
  }
}


CarSpace* TrainGeneration::split_car(CarSpace* c, size_t blocks) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  assert(c->blocks() > blocks, "just checking");
  // split first part of c into new car
  size_t new_blocks = c->blocks() - blocks;
  // create new car
  HeapWord* old_bottom = c->bottom();
  HeapWord* new_bottom = old_bottom + (CarSpace::car_size_in_words() * blocks);
  // update c but leave it in free list
  c->commit();
  c->set_blocks(new_blocks);
  c->set_bottom(new_bottom);
  // This will set top to bottom, and also initialize the offset table
  // threshold.
  c->clear();
  CarTableDesc* d = c->desc();
  c->set_desc(_car_tab.desc_for(new_bottom));
  c->uncommit();
  // create new car
  return new CarSpace(this, _ct, _bts,
		      MemRegion(old_bottom, new_bottom), blocks, d);
}


void TrainGeneration::add_to_car_pool(CarSpace* c, bool remset_already_empty) {
  assert_locked_or_safepoint(ExpandHeap_lock);

  // Oversized cars might have just been moved; we cannot add them
  // to the pool of free cars.
  if (_relinked_oversized_car == c) return;

  c->set_oversized(false);
  c->clear();
  c->uncommit();
  _car_tab.clear_entry(c);
  // reset remembered set, should already be empty
  c->remembered_set()->reinitialize(remset_already_empty);
  // First check free list is null
  CarSpace* p = car_free_list();
  if (p == NULL) {
    c->set_next_car(NULL);
    set_car_free_list(c);
    return;
  }
  // First check if c should go before p
  if (c->bottom() < p->bottom()) {
    c->set_next_car(p);
    set_car_free_list(c);
    coalesce_cars(c, p);
    return;
  } 
  // Iterate until c should be inserted after p
  while (p->next_car() != NULL && p->next_car()->bottom() < c->bottom()) {
    p = p->next_car();
  }
  c->set_next_car(p->next_car());
  p->set_next_car(c);
  if (c->next_car() != NULL) {
    coalesce_cars(c, c->next_car());
  }
  coalesce_cars(p, c);
}


CarSpace* TrainGeneration::retrieve_from_car_pool(size_t size, 
                                                  julong train_number, 
                                                  juint car_number, 
                                                  Train* train) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  assert(size>0, "Invalid size");
  size_t blocks = CarSpace::car_block_size(size);
  // Look for best fit in free list
  CarSpace* c = car_free_list();
  CarSpace* prev_c = NULL;
  CarSpace* best = NULL;
  CarSpace* prev_best = NULL;
  size_t best_blocks = spec()->max_size();  // just a big number; we recognize
				            // that max is in bytes, not blocks.
  while (c != NULL) {
    if (c->blocks() >= blocks && c->blocks() < best_blocks) {
      best_blocks = c->blocks();
      best = c;
      prev_best = prev_c;
    }
    prev_c = c;
    c = c->next_car();
  }
  // Check if we found existing usable car
  if (best != NULL) {
    if (best_blocks == blocks) {
      // best is exact fit, take it out of free list
      if (prev_best != NULL) {
        prev_best->set_next_car(best->next_car());
      } else {
        set_car_free_list(best->next_car());
      }
      best->set_next_car(NULL);
    } else {
      // best is too large, split it and leave remaining part in free list
      best = split_car(best, blocks);
    }
  } else {
    // No usable car found, expand train heap
    char* old_high = _virtual_space.high();
    assert((intptr_t) old_high % CarSpace::car_size() == 0, 
           "CarSpace not aligned");
    assert(prev_c == NULL || prev_c->next_car() == NULL, 
           "prev_c should be last in free list");
    size_t expand_blocks = blocks;
    if (prev_c != NULL && prev_c->end() == (HeapWord*)old_high) {
      // Last car in free list is adjacent to end of heap, extend it
      expand_blocks -= prev_c->blocks();
    }
    if (!_virtual_space.expand_by(expand_blocks * CarSpace::car_size())) {
      return NULL;
    }
    // expand shared offset array to cover virtual space expansion
    _bts->resize(heap_word_size(_virtual_space.committed_size()));
    // expand card marking array to cover virtual space expansion
    MemRegion mr((HeapWord*)_virtual_space.low(), 
                 heap_word_size(_virtual_space.committed_size()));
    assert(_ct != NULL, "Card table not set yet!");
    _ct->resize_covered_region(mr);
    if (expand_blocks == blocks) {
      // we have to create entire new car
      best = new CarSpace(this, _ct, _bts,
			  MemRegion((HeapWord*)old_high,
				    (HeapWord*) _virtual_space.high()),
			  expand_blocks,
			  _car_tab.desc_for(old_high));
    } else {
      // extend prev_c to cover expanded heap area
      prev_c->set_blocks(blocks);
      prev_c->set_end((HeapWord*) _virtual_space.high());
      // retry, free list allocation will succeed with best fit now
      return retrieve_from_car_pool(size, train_number, car_number, train);
    }
  }
  // best contains car
  _car_tab.update_entry(best, train_number, car_number, train);
  if (TraceCarAllocation) {
    gclog_or_tty->print("Assigned");
    best->print();
  }
  _ct->clear(MemRegion(best->bottom(), best->end()));
  best->commit();
  // Fix for bug #4678700 - mangle the memory region
  // No harm in clear() getting called more than once. 
  best->clear();
  return best;
}


Train* TrainGeneration::retrieve_from_train_pool(size_t size, 
                                                 julong train_number)
{
  assert_locked_or_safepoint(ExpandHeap_lock);
  Train *t;
  if (train_free_list() == NULL) {
    t = new Train(this);
  } else {
    t = train_free_list();
    set_train_free_list(t->next_train());
  }
  if (t == NULL) {
    return NULL;
  }

  CarSpace* c = retrieve_from_car_pool(size, train_number, 
                                       CarTableDesc::initial_car_number, t);
  if (c == NULL) {
    t->set_first_car(NULL);
    add_to_train_pool(t);
    return NULL;
  } else {
    t->initialize(c);
    return t;
  }
}


size_t TrainGeneration::add_to_train_pool(Train* t) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  // Release all cars in train
  size_t used = 0;
  CarSpace* c = t->first_car();
  while (c != NULL) {
    used += c->used();
    CarSpace* next_c = c->next_car();
    add_to_car_pool(c, false);
    c = next_c;
  }
  // Add to train free list (threaded through next_train fields)
  if (train_free_list() == NULL) {
    set_train_free_list(t);
    t->set_next_train(NULL);
  } else {
    t->set_next_train(train_free_list());
    set_train_free_list(t);
  }
  t->set_prev_train(NULL);
  return used;
}

TrainScanWeakRefClosure::
TrainScanWeakRefClosure(TrainGeneration* tg, bool& ref_to_first_train_found):
  OopClosure(tg->ref_processor()),
  _tg(tg), _ref_to_first_train_found(ref_to_first_train_found) { }

UpdateTrainRSCacheFromClosure::
UpdateTrainRSCacheFromClosure(TrainGeneration *tg, 
			      OopsInGenClosure* cl) : 
    OopsInGenClosure(tg),
    _tg(tg), _cl(cl), _from_train(CarTableDesc::invalid_train_number),
    _from_car(CarTableDesc::invalid_car_number) { }

UpdateTrainRSCacheFromClosure::UpdateTrainRSCacheFromClosure(TrainGeneration *tg) : 
    OopsInGenClosure(tg),
    _tg(tg), _cl(NULL), _from_train(CarTableDesc::invalid_train_number),
    _from_car(CarTableDesc::invalid_car_number) { }

void TrainGeneration::update_remembered_sets() {
  UpdateTrainRSCacheFromClosure update_rs(this);

  // Must not clear dirty cards: this scan is done before
  // other generations might have a chance to look at the
  // cards.
  FOR_EACH_TRAIN(t) {
    update_rs.set_train_number(t->train_number());
    FOR_EACH_CAR(t, c) {
      update_rs.set_car_number(c->car_number());
      _ct->ct_bs()->mod_oop_in_space_iterate(c, &update_rs);
    }
  }
}


// No need to save marks in oop_since_save_marks_iterate; it happens
// as we go through the trains.
#define TrainGen_INCLUDE_SINCE_SAVE_MARKS_ITERATE_DEFN(OopClosureType, nv_suffix, WrapClosureType) \
										\
void TrainGeneration::								\
oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl) {			\
  WrapClosureType update_rs(this, cl);						\
  cl->set_generation(this);							\
  FOR_EACH_TRAIN_REVERSE(t) {							\
    t->oop_since_save_marks_iterate##nv_suffix(&update_rs);			\
  }										\
  cl->reset_generation();							\
}

// These are excluded so as to avoid infinite chains of "wraps" (see above).
// This should strictly not be necessary if we don't override the virtaul methods
// defined in Generation. However, that elicits warnings from the SC++ compilers.
#define TrainGen_EXCLUDE_SINCE_SAVE_MARKS_ITERATE_DEFN(OopClosureType, nv_suffix) \
										\
void TrainGeneration::								\
oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl) {			\
  ShouldNotCallThis();								\
}

TRAIN_SPECIALIZED_SINCE_SAVE_MARKS_CLOSURE_PAIRS(
  TrainGen_INCLUDE_SINCE_SAVE_MARKS_ITERATE_DEFN)
TRAIN_EXCLUDE_SINCE_SAVE_MARKS_CLOSURES(
  TrainGen_EXCLUDE_SINCE_SAVE_MARKS_ITERATE_DEFN)

#undef TrainGen_INCLUDE_SINCE_SAVE_MARKS_ITERATE_DEFN
#undef TrainGen_EXCLUDE_SINCE_SAVE_MARKS_ITERATE_DEFN

void TrainGeneration::scavenge_higher_train_recorded_stores(TrainScanClosure* cl) {
  first_train()->first_car()->remembered_set()->scavenge_higher_train_recorded_stores(cl);
}

void TrainGeneration::scavenge_same_train_recorded_stores(TrainScanClosure* cl) {
  first_train()->first_car()->remembered_set()->scavenge_same_train_recorded_stores(cl);
}

void TrainGeneration::scavenge_special_nonoop_train_recorded_stores(TrainScanClosure* cl, 
                                                               bool exclude_lowest_train) {
  if (!UseSpecialLargeObjectHandling) return;
  if (_special_nonoop_train != NULL) {
    CarSpace* c = _special_nonoop_train->first_car();
    while (c != NULL) {
      c->remembered_set()->scavenge_special_nonoop_recorded_stores(cl, exclude_lowest_train);
      c = c->next_car();
    }
  }
}

void TrainGeneration::release_special_nonoop_train_contents() {
  if (!UseSpecialLargeObjectHandling) return;
  if (_special_nonoop_train != NULL) {
    CarSpace* prev = NULL;
    CarSpace* current = _special_nonoop_train->first_car();
    CarSpace* next = NULL;
    while (current != NULL) {
      next = current->next_car();
      if (current->marked()) {
        current->set_marked(false);
        prev = current;
      } else {
        if (prev == NULL) {
          _special_nonoop_train->set_first_car(next);
        } else {
          prev->set_next_car(next);
        }
        if (TraceSpecialLargeObjectHandling) {
          gclog_or_tty->print("Reclaimed large type array " INTPTR_FORMAT " (" SIZE_FORMAT " bytes)", 
                     current->bottom(), current->used());
          current->print_short_on(gclog_or_tty);
          gclog_or_tty->cr();
        }
        // Special nonoop cars are never copied, so we don't expect
        // to see the _relinked_oversized_car to be this car.
        assert(_relinked_oversized_car != current, 
                "attempting to release special nonoop with relinked car");
        add_to_car_pool(current, true);
      }
      current = next;
    }
    if (_special_nonoop_train->first_car() == NULL) {
      add_to_train_pool(_special_nonoop_train);
      _special_nonoop_train = NULL;
      if (TraceSpecialLargeObjectHandling) {
        gclog_or_tty->print_cr("Reclaimed large type array train");
      }
    } else {
       _special_nonoop_train->set_last_car(prev);
    }
  }
}


void TrainGeneration::tail_trains_set_target(bool value) {
  Train* t = first_train()->next_train();
  while (t != NULL) {
    t->set_target(value);
    t = t->next_train();
  }
}


void TrainGeneration::clear_remembered_sets() {
  if (special_nonoop_train() != NULL) {
    special_nonoop_train()->clear_remembered_sets();
  }
  FOR_EACH_TRAIN(t) {
    t->clear_remembered_sets();
  }
}


void TrainGeneration::collapse_cars() {
  Train* t = first_train(); 
  while (t != NULL) {
    Train* next_t = t->next_train();
    if (t->has_single_car()) {
      while (next_t != NULL && next_t->has_single_car()) {
        CarSpace* c = next_t->first_car();
        juint  last_car_number = t->last_car()->car_number();
        size_t last_car_free   = t->last_car()->free();
        t->last_car()->set_next_car(c);
        t->set_last_car(c);
        next_t->set_first_car(NULL);
        _car_tab.update_entry(c, t->train_number(), last_car_number+1, t);
        release_train(next_t);
        next_t = t->next_train();

        // used is updated to account for the fragmentation in the old last car
        inc_used_counter(last_car_free/HeapWordSize);
      }
    }
    t = next_t;
  }
}

void TrainGeneration::gc_prologue(bool full) {

  assert(!UsePerfData || (_space_counters->used() == used()),
         "counter value and reality should match");

  // Scan over dirtied cards for train remembered set updates.
  // Really only need to scan over mutator-dirtied cards.
  // Currently, this is very slow (an extra scan of modified
  // cards is made, doesn't use the filtering closure, and
  // all closure invocations are virtual).
  // See code in younger_refs_iterate which uses reserved.start().

  if (!full || ScavengeALot) {
    // Don't need to update the remembered sets if a MSC is
    // about to be done.  However, ScavengeALot forces the full
    // flag to true but does not necessarially do a MSC.

    // Don't discover refs while we're doing these updates.
    NoRefDiscovery no_refs(ref_processor());
    update_remembered_sets();
  }

  if (!full) {
    // Try to ensure that at least two trains exist.  This allows
    // for better object placement.  The train collector will try 
    // to collect the entire first first train during a tick.  If objects
    // are being promoted into the first train because it is the only
    // train, then this inhibits collecting it.  Failure to get
    // a second train is not fatal.
    if (first_train() == last_train()) {
      add_new_highest_train(CarSpace::car_size_in_words());
    }
  } else {
    // Collapse all consecutive single-car trains to a single train 
    // in order to improve compaction.
    collapse_cars();
  }
}


void TrainGeneration::release_empty_spaces() {
  // Remove nonoop train if empty
  if (_special_nonoop_train != NULL) {
    if (_special_nonoop_train->release_empty_spaces()) {
      add_to_train_pool(_special_nonoop_train);
      _special_nonoop_train = NULL;
      if (TraceSpecialLargeObjectHandling) {
        gclog_or_tty->print_cr("Reclaimed large type array train");
      }
    }
  }
  Train* t = first_train(); 
  while (t != NULL) {
    Train* next_t = t->next_train();
    if (t->release_empty_spaces()) {
      release_train(t);
    }
    t = next_t;
  }
  set_first_car_desc();
}

void TrainGeneration::prepare_for_compaction(CompactPoint* cp) {

  // Note that don't really use the incoming CompactPoint here
  assert(cp->gen == this, "sanity check");
  if (_special_nonoop_train != NULL) {
    _special_nonoop_train->prepare_for_compaction(cp);
  }
  FOR_EACH_TRAIN(t) {
    t->prepare_for_compaction(cp);
  }
  // Don't let younger generations compact into us.
  // If we use train gc we currently have to invalidate the remembered
  // set anyway, so we might as well compact new_gen locally.
  cp->gen = GenCollectedHeap::heap()->prev_gen(cp->gen);
  assert(cp->gen != NULL, "Must have a generation");
  cp->space = NULL;
  cp->threshold = NULL;
}

void TrainGeneration::object_iterate(ObjectClosure* blk) {
  if (_special_nonoop_train != NULL) {
    _special_nonoop_train->object_iterate(blk);
  }
  FOR_EACH_TRAIN(t) {
    t->object_iterate(blk);
  }
}

void TrainGeneration::space_iterate(SpaceClosure* blk, bool usedOnly) {
  if (_special_nonoop_train != NULL) {
    _special_nonoop_train->space_iterate(blk);
  }
  FOR_EACH_TRAIN(t) {
    t->space_iterate(blk);
  }
}


void TrainGeneration::save_marks() {
  if (_special_nonoop_train != NULL) {
    _special_nonoop_train->save_mark();
  }
  FOR_EACH_TRAIN(t) {
    t->save_mark();
  }
}


void TrainGeneration::reset_saved_marks() {
  if (_special_nonoop_train != NULL) {
    _special_nonoop_train->reset_saved_mark();
  }
  FOR_EACH_TRAIN(t) {
    t->reset_saved_mark();
  }
}


bool TrainGeneration::no_allocs_since_save_marks() {
  // Skip _special_nonoop_train, it is never scanned in 
  // scavenge_contents_from_saved_marks
  FOR_EACH_TRAIN(t) {
    if (!t->no_allocs_since_save_mark()) return false;
  }
  return true;
}

// Use train_capacity if you're looking for the
// capacity of each CarSpace that's in use.
size_t TrainGeneration::capacity() const {
  return _virtual_space.committed_size();
}

// Use train_free if you're looking for the
// free amount of each CarSpace that's in use.
size_t TrainGeneration::free() const {
  return capacity() - used();
}


size_t TrainGeneration::used() const {
  size_t size = 0;
  if (_special_nonoop_train != NULL) {
    size += _special_nonoop_train->used();
  }
  FOR_EACH_TRAIN(t) {
    size += t->used();
  }
  return size;
}

size_t TrainGeneration::train_capacity() const {
  size_t size = 0;
  if (_special_nonoop_train != NULL) {
    size += _special_nonoop_train->capacity();
  }
  FOR_EACH_TRAIN(t) {
    size += t->capacity();
  }
  return size;
}

size_t TrainGeneration::train_free() const {
  size_t size = 0;
  if (_special_nonoop_train != NULL) {
    size += _special_nonoop_train->free();
  }
  FOR_EACH_TRAIN(t) {
    size += t->free();
  }
  return size;
}


size_t TrainGeneration::unsafe_max_alloc_nogc() const {
  // compute maximum available space in free list
  size_t max_free_list_capacity = 0;
  CarSpace* c = car_free_list();
  CarSpace* last = NULL;
  while (c != NULL) {
    size_t size = c->capacity();
    if (size > max_free_list_capacity) {
      max_free_list_capacity = size;
    }
    last = c;
    c = c->next_car();
  }
  return max_free_list_capacity;
}

size_t TrainGeneration::contiguous_available() const {
  // compute maximum available space in free list
  size_t max_free_list_capacity = unsafe_max_alloc_nogc();
  // compute available uncommitted space
  size_t uncommitted_capacity = _virtual_space.uncommitted_size();
  CarSpace* last = NULL;
  FOR_EACH_TRAIN(t)
    FOR_EACH_CAR(t, c) last = c;
  if (last == NULL) {
    // If there are no cars, there is no space.  All the available space may
    // be in the special non-oop train.
    assert(used() == 0 || used() == _special_nonoop_train->used(), 
      "There are non special cars somewhere");
    return 0;
  } else if (last->end() == (HeapWord*)_virtual_space.high()) {
    // check for the case where the last free list entry is adjacent to the
    // uncommitted part
    uncommitted_capacity += last->free();
  }
  size_t res = MAX2(max_free_list_capacity, uncommitted_capacity);
  // Due to fragmentation, we might only be able to allocate half
  // the space (consider if each car is filled with an object exactly
  // half a car + 1 byte in size).
  // This isn't strictly correct, interface-wise, but this method is
  // currently used to guarantee that there are never promotion failures.
  return res/2;
}

void TrainGeneration::object_iterate_since_last_GC(ObjectClosure* blk) {
  assert(false, "NYI");
}


HeapWord* TrainGeneration::block_start(const void* p) const { 
  CarSpace* car = _car_tab.space_for(p);
  return car->block_start(p); 
}

size_t TrainGeneration::block_size(const HeapWord* p) const { 
  CarSpace* car = _car_tab.space_for(p);
  return car->block_size(p); 
}

bool TrainGeneration::block_is_obj(const HeapWord* p) const { 
  CarSpace* car = _car_tab.space_for(p);
  return car->block_is_obj(p); 
}


void TrainGeneration::compute_new_size() {
  // do nothing for now

  // Note this means the train won't uncommit pages. That'd only
  // be possible if the car free list bumps up against the end of
  // heap, which would mean a walk through the free list.
  //
  // In general, the more running room the train has, the better.
}

CompactibleSpace* TrainGeneration::first_compaction_space() const {
  // We compact trains individually, and currently don't allow compaction
  // into a train generation.
  return NULL;
}

void TrainGeneration::compact() {
  if (_special_nonoop_train != NULL) {
    FOR_EACH_CAR(_special_nonoop_train, c) {
      c->compact();
    }
  }
  FOR_EACH_TRAIN(t) {
    FOR_EACH_CAR(t, c) {
      c->compact();
    }
  }
}

const char* TrainGeneration::name() const {
  return "train generation";
}

bool TrainGeneration::should_collect(bool full, 
                                     size_t size, 
                                     bool is_large_noref,
                                     bool is_tlab) {
  return (full || 
    (GenCollectedHeap::heap()->get_gen(0)->stat_record()->invocations >=
     _next_invoke_count));
}


class TrainEvacuateFollowersClosure: public VoidClosure {
  TrainGeneration* _tg;
  TrainScanClosure* _scan_cur;
public:
  TrainEvacuateFollowersClosure(TrainGeneration* gen,
			        TrainScanClosure* cur) :
    _tg(gen), _scan_cur(cur) {}
  void do_void() {
    do {
      _tg->oop_since_save_marks_iterate_nv(_scan_cur);
    } while (!_tg->no_allocs_since_save_marks());
  }
};

TrainScanClosure::TrainScanClosure(TrainGeneration* tg, 
                                   bool gc_barrier, bool rs_update,
                                   bool& ref_to_first_train_found)  :
  OopsInGenClosure(tg), _tg(tg), _gc_barrier(gc_barrier), 
  _rs_update(rs_update), _ref_to_first_train_found(ref_to_first_train_found) {}

class TrainIsAliveClosure: public BoolObjectClosure {
  TrainGeneration* _g;
public:
  TrainIsAliveClosure(TrainGeneration* g) : _g(g) {}

  void do_object(oop p) {
    assert(false, "Do not call.");
  }

  bool do_object_b(oop p) {
    if (p->is_forwarded()) return true;
    // Note: cannot merely call check target here, since target will return true
    // for all cars in the first train. 
    // We want to know whether p is in the lowest car.
    CarTableDesc* p_desc = _g->car_table()->desc_for(p);
    if (!p_desc->target()) return true;
    // Otherwise...
    CarTableDesc* first_desc = _g->first_car_desc();
    return !p_desc->equals(first_desc);
  }
};

class TrainKeepAliveClosure: public OopClosure {
  TrainGeneration*         _tg;
  TrainScanWeakRefClosure* _cl;
public:
  TrainKeepAliveClosure(TrainGeneration* tg, TrainScanWeakRefClosure* cl) :
    _tg(tg), _cl(cl) { }

  void do_oop(oop* p) {
    oop obj = *p;
    assert (obj != NULL, "expected non-null ref");
    assert (obj->is_oop(), "expected an oop while scanning weak refs");

    if (_tg->is_in_reserved(obj)) {
      _cl->do_oop_nv(p);
    }
    _tg->weak_ref_barrier_check(p);
  }
};

class EntireTrainIsAliveClosure: public BoolObjectClosure {
  TrainGeneration* _tg;
public:
  EntireTrainIsAliveClosure(TrainGeneration* tg) : _tg(tg) {}

  void do_object(oop p) {
    assert(false, "Do not call.");
  }

  bool do_object_b(oop p) {
    if (p->is_forwarded()) return true;
    // Note: cannot merely call check target here, since target will return true
    // for all cars in the first train. 
    // We want to know whether p is in the lowest car.
    CarTableDesc* p_desc  = _tg->car_table()->desc_for(p);
    if (!p_desc->target()) return true;
    // Otherwise...
    CarTableDesc* first_desc = _tg->first_car_desc();
    return first_desc->train_number() != p_desc->train_number();
  }
};


oop TrainGeneration::copy_to_train(oop old, oop* from) {
  assert(car_table()->desc_for(old)->target() && !old->is_forwarded(), 
         "shouldn't be scavenging this oop");
  assert(first_train()->first_car()->contains(old), "should be in first car");
  int s = old->size();
  assert(s >= 0, "int won't convert to size_t");

  // Figure out where to copy to
  Train* t;
  if (from == NULL || !is_in_reserved(from)) {
    t = last_train();
  } else {
    t = car_table()->train_for(from);
  }

  oop obj = NULL;

  // NOTE car will be NULL if old is not in the train generation
  CarSpace* car = _car_tab.space_for(old);
   
  // Large objects already in this generation are simply relinked to
  // the selected train. 
  if (car != NULL && car->oversized()) {
    assert(is_in_reserved(old), 
             "car marked oversized for object not in train generation");
    assert(CarSpace::is_oversized_size(s), 
             "car marked as oversized but object is not oversized");

    // Because we don't install forwarding pointers, we might hit this object
    // more than once (if an interior ref in the oversized object points
    // to "this"). 
    assert(_relinked_oversized_car == NULL || _relinked_oversized_car == car, 
              "different oversized car already found?");
    _relinked_train           = t;
    _relinked_oversized_car   = car;

    // This will ensure there is no further processing of this 
    // car based on remembered sets!
    // This is only safe if we know that "t" is the youngest referencing
    // train (we're going to relink this car to the end of the youngest
    // referencing train, so it will have no remembered set information).
    // 
    // That's true, since we know that the reference "old" is in the
    // train generation, and we scan the train generation in reverse
    // train order.
    car->remembered_set()->clear();

#ifndef PRODUCT
      if (TraceScavenge) {
        gclog_or_tty->print("{relinking %s (" SIZE_FORMAT " train: ",
                    old->blueprint()->internal_name(), (size_t)s);
        gclog_or_tty->print_julong(t->train_number());
        gclog_or_tty->print_cr(" car: %u}",
                      t->last_car()->car_number()+1);
      }
#endif

    obj = old;

  } else {
    // Allocate obj
    obj = (oop) (t->allocate(s));
    if (obj == NULL) {
      vm_exit_out_of_memory(s, "heap expansion in copy_to_train");
    }

    // Prefetch beyond obj
    Prefetch::write(obj, PrefetchCopyIntervalInBytes);
  
    // Copy obj
    assert(!first_train()->first_car()->contains(obj), 
           "shouldn't copy to same car");
    Copy::aligned_disjoint_words((HeapWord*)old, (HeapWord*)obj, s);
  
#ifndef PRODUCT
    if (TraceScavenge) {
      gclog_or_tty->print_cr("{copying %s " INTPTR_FORMAT " -> " INTPTR_FORMAT " (" SIZE_FORMAT ")}", 
                  obj->blueprint()->internal_name(), old, obj, (size_t)s);
    }
#endif

    // Done, insert forward pointer to obj in this header
    old->forward_to(obj);
  }

  return obj;
}

void TrainGeneration::oop_update_remembered_set(oop* p) {
  oop obj = *p;
  if (obj != NULL) {
    // Find car table descriptor for the reference and object
    CarTableDesc* from_desc = car_table()->desc_for(p);
    CarTableDesc* to_desc = car_table()->desc_for(obj);
    if (to_desc->train_number() != CarTableDesc::invalid_train_number) {
      if (to_desc->less_than(from_desc)) {
        // and mark its remembered set
        to_desc->space()->remembered_set()->add_reference(p);
      }
    }
  }
}

void TrainGeneration::collect(bool full, 
                              bool clear_all_soft_refs,
			      size_t word_size, 
                              bool is_large_noref,
                              bool is_tlab) {

  // Oversized cars may be relinked. 
  // XX If they are, we do the relink late in the collection,
  //    after the normal processing, so as not to modify the
  //    car_desc or first_car(). This is not ideal, but is 
  //    necessary while the collection set is first_car().

  // When we find an oversized car which needs to be relinked,
  // we immediately clear its remembered set:  we know that
  // the oversized object is the only object in the car; if it's
  // live, no further processing is required. The remembered set
  // will need to be cleared anyway, as all information will be
  // stale.
  // It would be good to avoid some steps if we can, but that's not
  // possible yet in the interface (e.g. we can avoid dirty card
  // scanning if the oversized car object is found to be live while
  // processing other strong roots from outside the heap).

  _relinked_oversized_car = NULL;
  _relinked_train = NULL;

  GenCollectedHeap* gch = GenCollectedHeap::heap();

  if (!full && !clear_all_soft_refs) {
    // Partial train collection work.
    size_t available = contiguous_available();
    // FIXTHIS  accomodate multi-car collections
    size_t worstcase = first_train()->first_car()->car_size() *
                       first_train()->first_car()->blocks();
    bool oversized = first_train()->first_car()->oversized();
    if ((available < worstcase) && !oversized) {
      if (PrintGC && Verbose) {
        gclog_or_tty->print_cr("TrainGeneration::collect"
                      " contiguous_available: " SIZE_FORMAT " < worstcase: " SIZE_FORMAT,
                      available, worstcase);
      }
    } else if ((available < (word_size * HeapWordSize)) && !oversized) {
      // Bail out to MarkSweep, maybe compaction will help
      if (PrintGC && Verbose) {
        gclog_or_tty->print_cr("TrainGeneration::collect"
                      " contiguous_available: " SIZE_FORMAT " < size: " SIZE_FORMAT,
                      available, word_size*HeapWordSize);
      }
    } else if (gch->incremental_collection_will_fail()) {
      // We're getting short on heap space and must regain as much
      // as we can. Bail out to MarkSweep, compaction might help.
      if (PrintGC && Verbose) {
        gclog_or_tty->print_cr("TrainGeneration::collect,"
                      " incremental_collection_will_fail is true");
      }
    } else {
      TraceTime t1("Inc GC", PrintGC && !PrintGCDetails, true, gclog_or_tty);
      // Capture heap used before collection (for printing)
      size_t gch_prev_used = gch->used();

      NOT_COMPILER2(ReferencePolicy *soft_ref_policy = new LRUCurrentHeapPolicy();)
      COMPILER2_ONLY(ReferencePolicy *soft_ref_policy = new LRUMaxHeapPolicy();)
      bool free_lowest_train = false;
      bool ref_to_first_train_found = false;
      NOT_PRODUCT(_process_count++;)

      assert(gch->no_allocs_since_save_marks(0), 
        "save marks have not been newly set.");

      SpecializationStats::clear();

#ifndef PRODUCT
      if (PrintGC && WizardMode) {
        first_train()->first_car()->remembered_set()->print(first_train()->first_car());
      }

      if (VerifyRememberedSets) {
        // We're about to start walking the oops. We don't want
        // to do any discovery just now!
        NoRefDiscovery no_refs(ref_processor());
        gclog_or_tty->print("[Verify remembered sets...");
        // Verify that remembered sets are correct for outgoing refs in this car
        FOR_EACH_TRAIN(t) {
          t->verify_remembered_sets();
        }
        gclog_or_tty->print_cr("]");
      }
#endif

      // Update car table target entries
      first_train()->set_target(true);
      if (_special_nonoop_train != NULL) {
        _special_nonoop_train->set_target(true);
      }
      tail_trains_set_target(false);    // FIXTHIS-SG, should always be false?

      // Process other roots
      // Closure for scanning train generation: 
      //   no gc barrier needed, as we know both ref and object are in
      //   the same generation
      //   need RS update
      TrainScanClosure train_scan_train_gen(this, false,   // no gc barrier
                                                  true,    // train RS update
                                                  ref_to_first_train_found);
      // Closure for processing strong roots and evacuating followers: these 
      // closures will be wrapped by the train generation's own methods, which
      // will perform a train RS update check.
      TrainScanClosure train_scan_no_barrier(this, false,  // no gc barrier
                                                   false,  // no train RS update
                                                   ref_to_first_train_found);
      TrainScanClosure train_scan_barrier(this, true,      // gc barrier 
                                                false,     // no train RS update
                                                ref_to_first_train_found);

      // Barrier check required when evacuating followers: we will be scanning
      // oops that have moved into the train gen; they might contain references
      // back into younger generations.
      // RS update made through oop_since_save_marks_iterate for train gen.
      TrainEvacuateFollowersClosure evacuate_followers(this, 
                                                       &train_scan_barrier);
      
      // Process roots from car remembered set(s)
      scavenge_higher_train_recorded_stores(&train_scan_train_gen);

      // Evacuate followers from the above operation; we do this here
      // for better object placement
      evacuate_followers.do_void();

      gch->process_strong_roots(_level,
                      true,  // Process younger gens, if any, as strong roots.
                      false, // not collecting permanent generation.
                      GenCollectedHeap::CSO_AllClasses,
                      &train_scan_barrier, 
                      &train_scan_no_barrier);

      // Evacuate followers from the above operation
      evacuate_followers.do_void();

      // Check whether the remaining cars in this train has any references into 
      // them. This can be done by simply looking through their remembered sets 
      // and checking the card locations.

      // For weak references.
      TrainScanWeakRefClosure scan_weak_ref(this, ref_to_first_train_found);
      TrainKeepAliveClosure keep_alive(this, &scan_weak_ref);

      if (!ref_to_first_train_found && 
          !first_train()->tail_has_other_train_references()) {
        // No references to entire lowest train, we may be able to reclaim 
        // entire train.
        // Now process reference objects discovered during scavenge
        {
          EntireTrainIsAliveClosure is_alive(this);
          ReferenceProcessorSerial serial_rp(ref_processor(),
                                             soft_ref_policy, &is_alive,
                                             &keep_alive, &evacuate_followers);
          serial_rp.process_discovered_references();
        }
        if (!ref_to_first_train_found) {
          // Still no references to entire lowest train, we can reclaim it
          free_lowest_train = true;
        } else {
          // Processing references encountered reference to lowest train, 
          // cannot reclaim it after all
          scavenge_same_train_recorded_stores(&train_scan_train_gen);

          // Evacuate followers from the above operation
          evacuate_followers.do_void();
        }
      } else {
        // We have references into lowest train, evacuate objects reachable  
        // from higher cars in lowest train
        scavenge_same_train_recorded_stores(&train_scan_train_gen);

        // Evacuate followers from the above operation
        evacuate_followers.do_void();

        // Process reference objects discovered during scavenge
        {
          TrainIsAliveClosure is_alive(this);
          ReferenceProcessorSerial serial_rp(ref_processor(),
                                             soft_ref_policy, &is_alive,
                                             &keep_alive, &evacuate_followers);
          serial_rp.process_discovered_references();
        }
      }

      // Special large object handling
      scavenge_special_nonoop_train_recorded_stores(&train_scan_train_gen, 
                                                    free_lowest_train);

      julong prev_first = first_train()->train_number();
      size_t released = 0;
 
      // Print debug info before we rearrange any cars
      if (free_lowest_train) {
        if (PrintGC && Verbose) {
          first_train()->print_short();
        }
        released = release_lowest_train();
      } else {
        if (PrintGC && Verbose) {
          // More properly, the collection set
          first_train()->first_car()->print_short();
        }
        released = release_lowest_car();
      }

      if (_relinked_oversized_car != NULL) {

        assert(!_relinked_oversized_car->oversized() ||
                (_relinked_oversized_car->bottom() + 
                ((oop)_relinked_oversized_car->bottom())->size() == _relinked_oversized_car->top()),
               "more than one object in oversized car");

        // Evacuate followers from the relink just performed.
        //
        // Because we know only one object was in the collection
        // set (the oversized car contains only one object), this
        // operation will only update remembered sets: there are
        // no references to update because an object moved, and the
        // card table under the oversized car is unchanged.
        //
        // XX: Cleanup, can add a different closure to do what we
        //     need, which would also be used for general remembered
        //     set updates from a particular point (so could be used
        //     to remove wrapping closure from evacuate followers).
        //    For now, simply clear the target to do what we want.
        first_train()->set_target(false);
        
        // We don't need to update the card table here...
        TrainEvacuateFollowersClosure relink_evacuate_followers(this, 
                                                       &train_scan_no_barrier);
        relink_evacuate_followers.do_void();
      }

     add_total_processed(released);

     assert((_relinked_oversized_car == NULL && _relinked_train == NULL) ||
            (_relinked_oversized_car != NULL && _relinked_train != NULL), 
            "pair of relinked car or train must both be null or non-null");

     _relinked_oversized_car = NULL;
     _relinked_train         = NULL;

      release_special_nonoop_train_contents();

      compute_invocation_rate();

      SpecializationStats::print();

      if (PrintTrainGCProcessingStats) {
        print_train_processing_statistics(prev_first);
      }
    
      if (PrintGC && !PrintGCDetails) {
        gch->print_heap_change(gch_prev_used);
      }

      // Update heap occupancy information which is used as
      // input to soft ref clearing policy at the next gc.
      Universe::update_heap_info_at_gc();

      // Partial train collection work done.
      update_time_of_last_gc(os::javaTimeMillis());
      return;
    }
  }

  SpecializationStats::clear();

  // If full, the gc_prologue would have taken care of this.
  if (!full) {
    // Collapse all consecutive single-car trains to a single train 
    // in order to improve compaction.
    collapse_cars();
  }

  // Otherwise, do a full mark-sweep-compact collection of train
  // generation, temporarily expanding the span of its ref processor, so 
  // refs discovery is over the entire heap, not just this generation
  {
    ReferenceProcessorSpanMutator
      x(ref_processor(), GenCollectedHeap::heap()->reserved_region());
    GenMarkSweep::invoke_at_safepoint(_level, ref_processor(), clear_all_soft_refs);
  }
  // the above call also updated the time of last gc, so we don't
  // need to do so specifically for this generation here.

  if (PrintGCDetails) {
    // Give an indication that we picked a different algorithm.
    gclog_or_tty->print(" MSC");
  }

  reset_invocation_rate();

  clear_remembered_sets();
  // Release trains and cars now completely empty
  release_empty_spaces();

  // Now dirty all cards for the train generation. This will allow
  // us to update our remembered sets during the next garbage collection 
  // cycle. Must also invalidate perm gen cards!
  // Note on card invalidation: We invalidate only the "used" part of
  // perm gen, but the entire committed part of Train Gen. It would
  // have sufficed to dirty just the "used" part of Train Gen. In
  // that case, we would have computed the "used_region" as the
  // convex union of used_region for each constituent CarSpace
  // and dirtied precisely that region and cleared its compliment
  // (wrt the committed space). However, the arithmetic is slightly
  // more involved in that case (and needs special treatment in
  // CardTableRS::invalidate_or_clear() since the compliment is
  // not a convex set); it's something we could contemplate
  // doing in the future if time and resources permit.
  assert(level() == gch->n_gens() - 1,
         "Assumption that this generation is the oldest");
  _ct->invalidate_or_clear(this,
                           false /* not younger  */,
                           true  /* inv/clr perm */);

  SpecializationStats::print();
}


void TrainGeneration::reset_invocation_rate() {
  StatRecord* young_stat = GenCollectedHeap::heap()->get_gen(0)->stat_record();
  int invoke_count = young_stat->invocations;

  _tick_interval = DefaultTickInterval;
  _next_invoke_count = invoke_count + 1;
  _delay_adjustment_count = DelayTickAdjustment;
  _delta_promoted = _total_promoted;
  _delta_processed = _total_processed;
}

void TrainGeneration::compute_invocation_rate() {
  StatRecord* young_stat = GenCollectedHeap::heap()->get_gen(0)->stat_record();
  int invoke_count = young_stat->invocations;

  if (_delay_adjustment_count > 0) {
    _delay_adjustment_count--;
    _next_invoke_count = invoke_count + 1;
    _delta_promoted = _total_promoted;
    _delta_processed = _total_processed;
  } else {
    int prev_tick_interval = _tick_interval;
    julong promoted = _total_promoted - _delta_promoted;
    julong processed = _total_processed - _delta_processed;
    if (promoted * ProcessingToTenuringRatio > processed) {
      // increase speed
      if (_tick_interval > MinTickInterval) {
        _tick_interval--;
      }
    } else {
      // decrease speed
      if (_tick_interval < MaxTickInterval) {
        _tick_interval++;
      } else {
        _delta_promoted = _total_promoted;
        _delta_processed = _total_processed;
      }
    }
    if (PrintGC && Verbose && prev_tick_interval != _tick_interval) {
      gclog_or_tty->print(" (tick interval %d->%d) ", 
                 prev_tick_interval, _tick_interval);
    }
    if (_tick_interval < 1) {
      // decrease eden size temporarily
      // This will go away when the train tick calculation is 
      // improved so we collect more or less cars.
      GenCollectedHeap* gch = GenCollectedHeap::heap();
      assert (gch->_gens[0]->kind() == Generation::DefNew, "expected DefNew");
      DefNewGeneration* newgen = (DefNewGeneration*) gch->_gens[0];
      assert(newgen->to()->is_empty(), "eden to space should be empty here");
      int factor = 2 - _tick_interval;
      assert(factor > 1, "sanity check");
      newgen->eden()->allocate_temporary_filler(factor);
      if (PrintGC && Verbose) {
        gclog_or_tty->print(" (decreasing eden by factor of %d) ", factor);
      }
    }
    _next_invoke_count = invoke_count + 
                         (_tick_interval > 0 ? _tick_interval : 1);
  }
}

void TrainGeneration::younger_refs_iterate(OopsInGenClosure* blk) {
  // Make sure that we catch dirty stores here and update car remembered sets
  UpdateTrainRSCacheFromClosure update_rs(this, blk);

  blk->set_generation(this);
  FOR_EACH_TRAIN(t) {
    update_rs.set_train_number(t->train_number());
    FOR_EACH_CAR(t, c) {
      update_rs.set_car_number(c->car_number());
      // See the comment in gc_prologue: we avoid an extra scan of the
      // dirty cards to make train RS updates by specifying reserved().end()
      // here (causing all dirtied cards in this generation to be examined;
      // our wrapping closure will make the train RS check).

      younger_refs_in_space_iterate(c, &update_rs);
    }
  }
  blk->reset_generation();
}

void TrainGeneration::set_special_nonoop_train(Train* t) {
  _special_nonoop_train = t;
  if (t != NULL) {
    t->set_prev_train(NULL);
    // Necessary to make next_compaction_space work right.
    t->set_next_train(first_train());
  }
}

void TrainGeneration::set_first_train(Train* t) {
  _first_train = t;
  // Necessary to make next_compaction_space work right.
  if (_special_nonoop_train != NULL)
    _special_nonoop_train->set_next_train(t);
}

void TrainGeneration::print_on(outputStream* st) const {
  Generation::print_on(st);
  if (Verbose) {
    if (special_nonoop_train() != NULL) {
      special_nonoop_train()->print_on(st);
    }
    FOR_EACH_TRAIN(t) {
      t->print_on(st);
    }
  }
  if (Verbose && WizardMode) {
    st->print_cr("Train generation car free list");
    for (CarSpace* c = car_free_list(); c != NULL; c = c->next_car()) {
      c->print_on(st);
    }
  }
}

void TrainGeneration::update_counters() {
  if (UsePerfData) {
    _space_counters->update_all();
    _gen_counters->update_all();
  }
}

void TrainGeneration::update_capacity_counters() {
  if (UsePerfData) {
    _space_counters->update_capacity();
    _gen_counters->update_all();
  }
}

#ifndef PRODUCT

void TrainGeneration::verify(bool allow_dirty) {
  Train* t = first_train();
  guarantee(t != NULL, "No first train in train_gen");
  guarantee(t->first_car()->desc() == first_car_desc(), "Invalid first car desc");
  while (t != NULL) {
    julong tn = t->train_number();
    guarantee(tn > 0, "Illegal train number");
    Train* next_t = t->next_train();
    if (next_t == NULL) {
      guarantee(t == last_train(), "Wrong last train in train_gen");
    } else {
      guarantee(tn < next_t->train_number(), 
                "Train numbers in train_gen not ascending");
    }
    t->verify(allow_dirty);
    t = next_t;
  }
  if (special_nonoop_train() != NULL) {
    assert(UseSpecialLargeObjectHandling, "just checking");
    special_nonoop_train()->verify(true);
    julong tn = special_nonoop_train()->train_number();
    guarantee(tn == CarTableDesc::special_nonoop_train_number, 
              "Illegal train number");
    CarSpace* c = special_nonoop_train()->first_car();
    guarantee(c != NULL, "First car should be present");
    while (c != NULL) {
      oop obj = (oop) (c->bottom());
      guarantee(obj->is_typeArray(), 
                "Non-typearray found in special large typearray train");
      guarantee(obj->size() >= (int) Universe::heap()->large_typearray_limit(),
		"Small typearray found in special large typearray train");
      c = c->next_car();
    }
  }
  CarSpace* c = car_free_list();
  while (c != NULL) {
    CarSpace* next_c = c->next_car();
    if (next_c != NULL) {
      guarantee(c->end() < next_c->bottom(), 
                "free list should be in address order and coalesced")
    }
    c = next_c;
  }
}

class LowestCarRefOopClosure: public OopClosure {
 public:
  size_t cards;
  size_t refs;
  size_t last_card;
  HeapWord* train_gen_start;
  CarSpace* lowest_car;
  CardTableRS* ct;

  void do_oop(oop* from) {
    // This used to assert that "from" came from and the 'old' or 'perm'
    // generations.  To translate this to > 2 gens, I'm assuming that
    // preceding generations will be scanned in their entirety, but
    // following ones (including perm) will be tracked by remembered sets.
    assert((HeapWord*)from >= train_gen_start
	   && !lowest_car->contains(from),
	   "just checking");
    oop to = *from;
    if (to != NULL && lowest_car->contains(to)) {
      refs++;
      size_t from_card = ct->ct_bs()->index_for(from);
      if (from_card != last_card) {
        cards++;
        last_card = from_card;
      }
    }
  }

  LowestCarRefOopClosure(HeapWord* _tg_start, CardTableRS* _ct) {
    cards = 0;
    refs = 0;
    last_card = -1;
    train_gen_start = _tg_start;
    ct = _ct;
  }
};

class LowestCarRefSpaceClosure: public SpaceClosure {
 public:
  LowestCarRefOopClosure* oop_blk;
  CarSpace* lowest_car;

  void do_space(Space* s) {
    if (s != lowest_car) {
      s->oop_iterate(oop_blk);
    }
  }
};


void TrainGeneration::compute_references_into_lowest_car(size_t* nof_cards, 
                                                         size_t* nof_refs) {
  LowestCarRefOopClosure oop_blk(_reserved.start(), _ct);
  oop_blk.lowest_car = first_train()->first_car();
  LowestCarRefSpaceClosure space_blk;
  space_blk.lowest_car = first_train()->first_car();
  space_blk.oop_blk = &oop_blk;
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  for (int i = level(); i < gch->_n_gens; i++) {
    gch->_gens[i]->space_iterate(&space_blk);
  }
  gch->perm_gen()->oop_iterate(&oop_blk);
  
  *nof_cards = oop_blk.cards;
  *nof_refs = oop_blk.refs;
}

void TrainGeneration::print_train_processing_statistics(julong prev_first_number)
{
  Train* t = first_train();
  if (t->train_number() != prev_first_number) {
    gclog_or_tty->cr();
    double passes = (double) _process_count / _starting_length;
    gclog_or_tty->print("Train ");
    gclog_or_tty->print_julong(prev_first_number);
    gclog_or_tty->print_cr(" processed in %d/%d=%lf passes", _process_count, 
                  _starting_length, passes);
    _process_count = 0;
    _starting_length = t->length();
    assert(_starting_length > 0, "just checking");
    while (t != NULL) {
      gclog_or_tty->print_cr("- Train ");
      gclog_or_tty->print_julong(t->train_number());
      gclog_or_tty->print_cr(" length %d", t->length());
      t = t->next_train();
    }
  }
}

#endif // PRODUCT

