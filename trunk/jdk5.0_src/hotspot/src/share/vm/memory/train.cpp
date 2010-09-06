#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)train.cpp	1.61 03/12/23 16:41:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_train.cpp.incl"


void Train::save_mark() { 
  set_saved_top_mark(top_mark());
  FOR_EACH_CAR(this, c) {
    c->set_saved_mark();
  }
}

void Train::reset_saved_mark() { 
  set_saved_top_mark(bottom_mark());
  FOR_EACH_CAR(this, c) {
    c->reset_saved_mark();
  }
}

bool Train::no_allocs_since_save_mark() const {
  return saved_top_mark() == top_mark();
}

WaterMark Train::top_mark() const {
  return last_car()->top_mark();
}

WaterMark Train::bottom_mark() const {
  return first_car()->bottom_mark();
}

size_t Train::free() const {   
  // Free chunks at the end of the first n-1 cars cannot 
  // be used and is therefore not considered free
  return last_car()->free(); 
}                                

size_t Train::used() const { 
  // The first n-1 cars are considered as filled
  size_t size = 0;
  CarSpace* c = first_car();
  while (c != last_car()) {
    size += c->capacity();
    c = c->next_car();
  }
  size += c->used();
  return size;
}

size_t Train::capacity() const {
  size_t size = 0;
  FOR_EACH_CAR(this, c) {
    size += c->capacity();
  }
  return size;
}


int Train::length() const {
  int len = 0;
  FOR_EACH_CAR(this, c) {
    len++;
  }
  return len;
}


HeapWord* Train::allocate(size_t size) {
  HeapWord* p = last_car()->allocate(size);
  if (p == NULL) {
    CarSpace* old_last_car = last_car();
    CarSpace* new_last_car = add_new_last_car(size);
    if (new_last_car != NULL) {
      p = new_last_car->allocate(size);

      /* update the counters. capacity counters needs to be updated
       * because we've added a new car.
       */
      _tg->update_capacity_counters();

      /* used is updated to account for the fragmentation in the old
       * last car and the newly allocated object.
       */
      _tg->inc_used_counter((old_last_car->free()/HeapWordSize) + size);
    }
  }
  else {
    // increment the used performance counter. 
    _tg->inc_used_counter(size);
  }
  return p;
}

void Train::initialize(CarSpace* first_car) {
  set_first_car(first_car);
  set_last_car(first_car);
  set_next_train(NULL);
  set_prev_train(NULL);
  reset_saved_mark();
}


CarSpace* Train::add_new_last_car(size_t size) {
  CarSpace* c = _tg->retrieve_from_car_pool(size, train_number(), 
                                            last_car()->car_number() + 1, this);
  if (c != NULL) {
    last_car()->set_next_car(c);
    set_last_car(c);
  }
  return c;
}


#define Train_SINCE_SAVE_MARKS_ITERATE_DEFN(OopClosureType, nv_suffix)      \
                                                                            \
void Train::oop_since_save_marks_iterate##nv_suffix(OopClosureType* blk) {  \
  while (saved_top_mark() != top_mark()) {                                  \
    CarSpace* sp = saved_top_mark().space()->toCarSpace();                  \
    sp->oop_since_save_marks_iterate##nv_suffix(blk);                       \
    if (sp->next_car() != NULL) {                                           \
      set_saved_top_mark(sp->next_car()->bottom_mark());                    \
    } else {                                                                \
      set_saved_top_mark(sp->top_mark());                                   \
    }                                                                       \
  }                                                                         \
} 

ALL_SINCE_SAVE_MARKS_CLOSURES(Train_SINCE_SAVE_MARKS_ITERATE_DEFN)
#undef Train_SINCE_SAVE_MARKS_ITERATE_DEFN

void Train::object_iterate(ObjectClosure* blk) {
  FOR_EACH_CAR(this, c) {
    c->object_iterate(blk);
  }
}


void Train::oop_iterate(OopClosure* blk) {
  FOR_EACH_CAR(this, c) {
    c->oop_iterate(blk);
  }
}


void Train::space_iterate(SpaceClosure* blk) {
  FOR_EACH_CAR(this, c) {
    blk->do_space(c);
  }
}

#ifndef PRODUCT

class VerifyRSClosure: public OopClosure {
private:
  TrainGeneration* _tg;
public:
  VerifyRSClosure(TrainGeneration* gen) : _tg(gen) { }

  // Check that any references contained in this oop
  // which point to younger cars in the train correctly
  // have a remembered set entry for the younger car.
  void do_oop(oop* p) {
    oop obj = *p;
    // p is known to be in the train gen
    if (_tg->is_in_reserved(obj)) {
      CarTableDesc* from_desc = _tg->car_table()->desc_for(p);
      CarTableDesc* to_desc   = _tg->car_table()->desc_for(obj);
      if (to_desc->less_than(from_desc)) {
        guarantee(to_desc->space()->remembered_set()->contains_reference(p),
            "Lower train reference not present");
      }
    }
  }
};

void Train::verify_remembered_sets() {
  CarSpace* c = first_car();
  guarantee(c != NULL, "No first car in train");
  while (c != NULL) {
    // Verify that remembered sets are correct for outgoing refs in this car
    VerifyRSClosure rs_verify(_tg);
    oop_iterate(&rs_verify);
    c = c->next_car();
  }
}

void Train::verify(bool allow_dirty) const {
  julong tn = train_number();
  CarSpace* c = first_car();
  guarantee(c != NULL, "No first car in train");
  while (c != NULL) {
    guarantee(c->train() == this, "Inconsistent car table");    
    guarantee(c->train_number() == tn, "Inconsistent car table");    
    juint cn = c->car_number();
    guarantee(cn > 0, "Illegal car number");
    CarSpace* next_c = c->next_car();
    if (next_c == NULL) {
      guarantee(c == last_car(), "Wrong last car in train");
    } else {
      guarantee(cn < next_c->car_number(), "Car numbers in train not increasing");
    }
    c->verify(allow_dirty);
    c = next_c;
  }
}
  

void Train::print() const {
  print_on(gclog_or_tty);
}

void Train::print_on(outputStream* st) const {
  st->print(" train ");
  st->print_julong(train_number());
  st->cr();
  FOR_EACH_CAR(this, c) {
    c->print_on(st);
  }
}

void Train::print_short() const {
  print_short_on(gclog_or_tty);
}

void Train::print_short_on(outputStream* st) const {
  st->print(" train (");
  st->print_julong(train_number());
  st->print(",%u-%u)", first_car()->car_number(), last_car()->car_number());
}

#endif // PRODUCT


void Train::prepare_for_compaction(CompactPoint* cp) {
  cp->space = first_car();
  cp->threshold = cp->space->initialize_threshold();
  FOR_EACH_CAR(this, c) {
    c->prepare_for_compaction(cp);
    if (c->oversized()) {
      // Don't allow anything else to compact to it!
      // We'll exit the loop if the next car is null.
      if (c->next_car() != NULL) {
        cp->space = c->next_car();
        cp->threshold = cp->space->initialize_threshold();
      }
    }
  }
}


void Train::set_target(bool value) {
  FOR_EACH_CAR(this, c) {
    c->set_target(value);
  }
}


void Train::clear_remembered_sets() {
  FOR_EACH_CAR(this, c) {
    c->remembered_set()->clear();
  }
}


bool Train::release_empty_spaces() {
  CarSpace* c = first_car();  
  while (c->used() == 0) {
    // first car empty, release it and update first_car pointer
    CarSpace* new_first = c->next_car();
    if (new_first == NULL) return true; // entire train empty
    set_first_car(new_first);
    _tg->add_to_car_pool(c, false);
    c = new_first;
  }
  // first car no longer empty, cannot release train
  c->remembered_set()->reinitialize(false);
  CarSpace* prev_c = c;
  c = c->next_car();
  // iterate over remaining cars
  while (c != NULL) {
    CarSpace* next_c = c->next_car();
    if (c->used() == 0) {
      // we found an empty car, release it and update next_car pointer
      prev_c->set_next_car(next_c);
      _tg->add_to_car_pool(c, false);
    } else {
      // car still in use, clear remembered set
      c->remembered_set()->reinitialize(false);
      prev_c = c;
    }
    c = next_c;
  }
  // update last_car pointer
  set_last_car(prev_c);
  return false;
}


bool Train::tail_has_other_train_references() {
  CarSpace* c = first_car(); 
  assert(c != NULL, "just checking");
  c = c->next_car();
  while (c != NULL) {
    if (c->remembered_set()->has_other_train_references(this)) return true;
    c = c->next_car();
  }
  return false;
}


