#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)train.hpp	1.45 03/12/23 16:41:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A Train instance models an individual train in the train generation
// and provides support for allocation and garbage collection.

class CarSpace;
class ObjectClosure;
class OopClosure;
class SpaceClosure;
class TrainGeneration;
class CompactPoint;

class Train: public CHeapObj {
  friend class TrainGeneration;
  friend class VMStructs;
 private:
  TrainGeneration* _tg;		        // The generation holding this train.
  CarSpace* _first_car;                 // The oldest car in this train
  CarSpace* _last_car;                  // The youngest car in this train
  Train*    _next_train;                // Next train (or NULL)
  Train*    _prev_train;                // Previous train (or NULL)
  WaterMark _saved_top_mark;            // Watermark used for gc/allocation profiling

  // Watermark
  WaterMark saved_top_mark() const      { return _saved_top_mark; }
  void set_saved_top_mark(WaterMark m)  { _saved_top_mark = m; }
  WaterMark* addr_saved_top_mark()      { return &_saved_top_mark; }

  void save_mark();
  void reset_saved_mark();
  bool no_allocs_since_save_mark() const;

  WaterMark top_mark() const;
  WaterMark bottom_mark() const;

  // Accessors
  void set_first_car(CarSpace* c)       { _first_car = c; }
  void set_last_car(CarSpace* c)        { _last_car = c; }
  void set_next_train(Train* t)         { _next_train = t; }
  void set_prev_train(Train* t)         { _prev_train = t; }

  // Add new car at end of train
  CarSpace* add_new_last_car(size_t size);

 public:
  Train(TrainGeneration* tg) : _tg(tg) {}

  // Accessors
  CarSpace* first_car() const           { return _first_car; }
  CarSpace* last_car() const            { return _last_car; }
  Train* next_train() const             { return _next_train; }
  Train* prev_train() const             { return _prev_train; }

  // Number of this train
  inline julong train_number() const;

  // Length computations
  inline bool has_single_car() const;
  int length() const;

  // Size computations (in bytes)
  size_t capacity() const;
  size_t used() const;
  size_t free() const;

  // Allocate oop of given size in train, append new car if necessary
  HeapWord* allocate(size_t size);

  // Allocate oop of given size in train, return NULL if full
  inline HeapWord* allocate_within(size_t size);

  // Initialization.
  void initialize(CarSpace* first_car);

  // Iterate from saved marks to top marks and update saved marks.
#define Train_SINCE_SAVE_MARKS_DECL(OopClosureType, nv_suffix)          \
  void oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl);

  ALL_SINCE_SAVE_MARKS_CLOSURES(Train_SINCE_SAVE_MARKS_DECL)
#undef Train_SINCE_SAVE_MARKS_DECL

  // Iterate over entire train
  void object_iterate(ObjectClosure* blk);
  void oop_iterate(OopClosure* blk);
  void space_iterate(SpaceClosure* blk);

  // GC support
  void clear_remembered_sets();
  bool tail_has_other_train_references();
  void set_target(bool value);

  // Debugging
  void verify(bool allow_dirty) const         PRODUCT_RETURN;
  void verify_remembered_sets()               PRODUCT_RETURN;
  void print() const                          PRODUCT_RETURN;
  void print_on(outputStream* st)  const      PRODUCT_RETURN;
  void print_short() const                    PRODUCT_RETURN;
  void print_short_on(outputStream* st) const PRODUCT_RETURN;

 private:
  // Mark sweep support
  bool release_empty_spaces();
  void prepare_for_compaction(CompactPoint* cp);
};

#define FOR_EACH_CAR(t, c) \
  for (CarSpace* c = t->first_car(); c != NULL; c = c->next_car())
