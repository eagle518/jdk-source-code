#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)carRememberedSet.cpp	1.51 03/12/23 16:40:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_carRememberedSet.cpp.incl"

void CarRememberedSet::allocate(unsigned capacity) {
  assert(is_power_of_2(capacity), "remembered set capacity must be power of 2");
  _capacity_mask = capacity - 1;
  _set = NEW_C_HEAP_ARRAY(jbyte*, capacity);
  clear();
}


CarRememberedSet::~CarRememberedSet() {
  FREE_C_HEAP_ARRAY(jbyte*, _set);
}


void CarRememberedSet::clear() {
  _size = 0;
  _last_from_card = NULL;
  _being_processed = false;
  Copy::fill_to_words((HeapWord*)_set, _capacity_mask+1);
}


void CarRememberedSet::reinitialize(bool already_empty) {
  if (capacity() != initial_capacity) {
    FreeHeap(_set);
    allocate(initial_capacity);
  } else {
    if (already_empty) {
      _being_processed = false;
      debug_only(verify_is_cleared());
    } else {
      clear();
    }
  }
}


bool CarRememberedSet::grow_and_rehash(unsigned grow_factor) {
  unsigned old_capacity = capacity();
  jbyte** old_set = _set;
  debug_only(unsigned old_size = _size);

  // Check if multiply will wrap because capacity will be too large
  if (old_capacity * grow_factor <= old_capacity) {
    return false;
  }

  allocate(old_capacity * grow_factor);

  if (PrintGC && Verbose) {
    gclog_or_tty->print_cr("[Growing carRememberedSet " INTPTR_FORMAT 
     " from %lu to %lu]", this, old_capacity, capacity());
  }

  for (unsigned old_index = 0; old_index < old_capacity; old_index++) {
    jbyte* old_card = old_set[old_index];
    if (old_card != NULL) {
      unsigned index = index_for(old_card);
      unsigned dhash = double_hash(index);
      while (_set[index] != NULL) {
        index = next_index(index, dhash);
      }
      _set[index] = old_card;
      _size++;
    }
  }
  assert(_size == old_size, "just checking");
  FreeHeap(old_set);
  return true;
}


void CarRememberedSet::add_reference(oop* from) {
  if (!_being_processed) {
    jbyte* from_card = _ct->byte_for(from);
    assert (_tg->is_in(*from), "expected obj in the train gen");
    // Check if this reference is on the same card as the last
    // insertion. If it is, we're done.
    if (_last_from_card != from_card) {
      // Update our cached value.
      _last_from_card = from_card;
      // Resize if load factor > 0.5
      if (_size > capacity() / 2) {
        bool ok = grow_and_rehash(2);
        if (!ok && _size + 1 > capacity()) {
          fatal("remembered set expansion failure");
        }
      }
      unsigned index = index_for(from_card);
      unsigned dhash = double_hash(index);
      debug_only(unsigned loop_count = 0);
      debug_only(unsigned initial_index = index);
      while (true) {
        jbyte* set_index = _set[index];
        if (set_index == from_card) return;
        if (set_index == NULL) break;
        index = next_index(index, dhash);
        debug_only(
          loop_count++; 
          if (loop_count > capacity()) {
            gclog_or_tty->print_cr("Hash failure, addr = " INTPTR_FORMAT
             ", init index = %u dhash = %u", from, initial_index, dhash);
            print(NULL);
            fatal("Train remembered set hash table failure");
          }
        )
      }
      _set[index] = from_card;
      _size++;
    }
  }
}


void CarRememberedSet::scavenge_higher_train_recorded_stores(TrainScanClosure* cl) {
  _being_processed = true;
  Train* first_train = _tg->first_train();
  CarTable* car_tab = _tg->car_table();
  assert(first_train->first_car()->remembered_set() == this, "just checking");
  for (unsigned index = 0; index <= _capacity_mask; index++) {
    jbyte* card = _set[index];
    if (card != NULL) {
      HeapWord* bottom = _ct->addr_for(card);
      assert(_tg->is_in_reserved(bottom), "expected bottom in train gen");
      CarTableDesc* desc = car_tab->desc_for(bottom);
      if (desc->train() != first_train) {
        CarSpace* sp = desc->space();
        HeapWord* top = MIN2(sp->top(),
                             bottom + CardTableModRefBS::card_size_in_words);
	MemRegion mr(bottom, top);
	HeapWord* p = sp->block_start(bottom);  // Will be a fast impl.
        while (p < top) {
          assert(sp->used_region().contains(p), "just checking");
          p += oop(p)->oop_iterate(cl, mr);
        }
        // clear entry for next scan        
        _set[index] = NULL;
        _size--;
      }
    }
  }
}


void CarRememberedSet::scavenge_same_train_recorded_stores(TrainScanClosure* cl) {
  Train* first_train = _tg->first_train();
  CarTable* car_tab = _tg->car_table();
  assert(first_train->first_car()->remembered_set() == this, "just checking");
  for (unsigned index = 0; index <= _capacity_mask; index++) {
    jbyte* card = _set[index];
    if (card != NULL) {
      HeapWord* bottom = _ct->addr_for(card);
      assert(_tg->is_in_reserved(bottom), "expected card in train gen");
      CarTableDesc* desc = car_tab->desc_for(bottom);
      CarSpace* sp = desc->space();
      HeapWord* top =
	MIN2(sp->top(), bottom + CardTableModRefBS::card_size_in_words);
      MemRegion mr(bottom, top);
      assert(first_train == desc->train(), "just checking");
      HeapWord* p = sp->block_start(bottom);  // Impl will be fast.
      while (p < top) {
        assert(p < sp->top(), "just checking");
        p += oop(p)->oop_iterate(cl, mr);
      }
      // done, clear entry
      _set[index] = NULL;
      _size--;
    }
  }
  assert(_size == 0, "checking size");
  // Clear cached last entry
  _last_from_card = NULL;
}


void CarRememberedSet::scavenge_special_nonoop_recorded_stores(TrainScanClosure* cl, 
                                                     bool exclude_lowest_train) {
  // Save all cards in a separate resource array and clear this remembered set 
  // in the process. For a normal car, live objects are copied out so new entries 
  // should never be inserted during scanning. For a special large typearray car, 
  // we leave the object in place if live, and valid remembered cards will thus 
  // get reinserted.
  ResourceMark rm;
  unsigned s = size();
  jbyte** cards = NEW_RESOURCE_ARRAY(jbyte*, s);
  unsigned index = 0;
  for (unsigned array_index = 0; array_index <= _capacity_mask; array_index++) {
    jbyte* card = _set[array_index];
    if (card != NULL) {
      cards[index++] = card;
      // clear entry
      _set[array_index] = NULL;
      _size--;
    }
  }
  // Clear cached last card
  _last_from_card = NULL;
  assert(_size == 0, "checking size");
  assert(index == s, "checking size");
  // Scan the saved cards
  CarTableDesc* first_desc = _tg->first_car_desc();
  CarTable* car_tab = _tg->car_table();
  for (index = 0; index < s; index++) {
    jbyte* card = cards[index];
    HeapWord* bottom = _ct->addr_for(card);
    assert(_tg->is_in_reserved(bottom), "expected card in train gen");
    CarTableDesc* desc = car_tab->desc_for(bottom);
    // Check if card belongs to first train, and if so whether we can skip
    // all references from that train or only references from the first car.
    if (desc->train() != first_desc->train() ||
        (!exclude_lowest_train && !desc->equals(first_desc))) {
      CarSpace* sp = desc->space();
      HeapWord* top = MIN2(sp->top(),
                           bottom + CardTableModRefBS::card_size_in_words);
      MemRegion mr(bottom, top);
      HeapWord* p = sp->block_start(bottom);
      while (p < top) {
        assert(sp->used_region().contains(p), "just checking");
        p += oop(p)->oop_iterate(cl, mr);
      }
    }
  }
}


bool CarRememberedSet::has_other_train_references(Train* t) {
  CarTable* car_tab = _tg->car_table();
  for (unsigned index = 0; index <= _capacity_mask; index++) {
    jbyte* card = _set[index];
    if (card != NULL) {
      HeapWord* card_start = _ct->addr_for(card);
      assert(_tg->is_in_reserved(card_start), "expected card for train gen");
      CarTableDesc* card_desc = car_tab->desc_for(card_start);
      if (card_desc->train() != t) return true;
    }
  }
  return false;
}


#ifndef PRODUCT

void CarRememberedSet::verify(const CarSpace* holder) const {
  guarantee(holder->remembered_set() == this, "just checking");
  CarTable* car_tab = _tg->car_table();
  for (unsigned index = 0; index <= _capacity_mask; index++) {
    jbyte* card = _set[index];
    if (card != NULL) {
      HeapWord* card_start = _ct->addr_for(card);
      guarantee(_tg->is_in_reserved(card_start), "card not in train gen");
      CarTableDesc* card_desc = car_tab->desc_for(card_start);
      guarantee(holder->desc()->less_than(card_desc), 
                "invalid card in remembered set");
    }
  }
}


bool CarRememberedSet::contains_reference(oop* from) const {
  jbyte* from_card = _ct->byte_for(from);
  unsigned index = index_for(from_card);
  unsigned dhash = double_hash(index);
  jbyte* card;
  do {
    card = _set[index]; 
    if (card == from_card) return true;
    index = next_index(index, dhash);
  } while (card != NULL);
  return false;
}


void CarRememberedSet::verify_is_cleared() const {
  guarantee(_last_from_card == NULL, "cached value not cleared");
  for (unsigned index = 0; index <= _capacity_mask; index++) {
    guarantee(_set[index] == NULL, "Remembered set contains non-null entry");
  }
  guarantee(_size == 0, "remembered set size not zero");
}


 void CarRememberedSet::print(CarSpace* holder) const {
  gclog_or_tty->print("Remembered set " INTPTR_FORMAT " ", this);
  if (holder != NULL) {
    assert(holder->remembered_set() == this, "just checking");
    gclog_or_tty->print("for car (");
    gclog_or_tty->print_julong(holder->train_number());
    gclog_or_tty->print_cr(",%u)", holder->car_number());
  }
  for (unsigned index = 0; index <= _capacity_mask; index++) {
    jbyte* card = _set[index];
    if (card != NULL) {
      HeapWord* card_start = _ct->addr_for(card);
      gclog_or_tty->print(" card " INTPTR_FORMAT " [" INTPTR_FORMAT ";" 
               INTPTR_FORMAT "[ in ",
               card,
               card_start,
               card_start + CardTableModRefBS::card_size_in_words);
      if (_tg->is_in_reserved(card_start)) {
        CarTableDesc* card_desc = _tg->car_table()->desc_for(card_start);
        card_desc->space()->print_short();
      } else {
      gclog_or_tty->print("other generation");
      }
      gclog_or_tty->cr();
    }
  }
}
  

#endif
