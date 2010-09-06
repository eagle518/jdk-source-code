#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)carRememberedSet.hpp	1.32 03/12/23 16:40:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Remembered set for single car. Records areas in higher cars in the train gen
// holding references into this car. The entries are pointers into the card
// marking array, with the recorded cards mapping to the areas holding references.
// The information is thus not exact, requiring scanning of a small memory area
// to locate the actual reference(s).
//
// Implemented as a closed hash table.

class CardTableRS;
class TrainGeneration;
class TrainScanClosure;

class CarRememberedSet: public CHeapObj {
  friend class VMStructs;
 private:
  TrainGeneration*   _tg;
  CardTableRS*       _ct;

  enum SomeConstants {
    initial_capacity = 512
  };
  unsigned _size;              // number of elements inserted
  unsigned _capacity_mask;     // set capacity minus one
  jbyte**  _set;               // array
  jbyte*   _last_from_card;    // cached value of last addition
  bool     _being_processed;   // set during scavenge of this set

  void allocate(unsigned capacity);
  bool grow_and_rehash(unsigned grow_factor);

  inline unsigned index_for(jbyte* card) const { return (unsigned) 
			((uintptr_t) card) & _capacity_mask; }

  // Double hash function. Must never return zero; must be relatively
  // prime with the capacity. 
  // Our capacity is a power of 2 (invariant maintained in allocate).
  // Thus, easiest thing is to return an odd number here.
  inline unsigned double_hash(unsigned index) const  
       { return ((index >> 4) & _capacity_mask) | 0x1; }
  inline unsigned next_index(unsigned index, unsigned dhash) const
       { return (index + dhash) &  _capacity_mask; }

 public:
  // Constructors
  CarRememberedSet(TrainGeneration* tg, CardTableRS* ct) :
    _tg(tg), _ct(ct) {
    allocate(initial_capacity);
  }

  ~CarRememberedSet();

  // Current number of elements
  unsigned size() const      { return _size; }
  // Set capacity
  unsigned capacity() const  { return _capacity_mask + 1; }
  // Insert new entry (card corresponding to root from)
  void add_reference(oop* from);
  // Reset
  void clear();
  void reinitialize(bool already_empty);
  // Garbage collection support
  void scavenge_higher_train_recorded_stores(TrainScanClosure* cl);
  void scavenge_same_train_recorded_stores(TrainScanClosure* cl);
  bool has_other_train_references(Train* t);
  void scavenge_special_nonoop_recorded_stores(TrainScanClosure* cl, 
                                               bool exclude_lowest_train);

  // Debugging
  void verify(const CarSpace* holder) const PRODUCT_RETURN;

#ifndef PRODUCT
  bool contains_reference(oop* from) const;
  void verify_is_cleared() const;
  void print(CarSpace* holder) const;
#endif
};
