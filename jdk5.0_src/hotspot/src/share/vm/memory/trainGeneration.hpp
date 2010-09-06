#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)trainGeneration.hpp	1.19 04/01/11 09:13:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// TrainGeneration models the (optional) heap containing old objects.
//
// Garbage collection is performed using the train algorithm, or alternatively, 
// a full mark-compact.

// A TrainGeneration is assumed to have at least 1 train and a train is assumed
// to have at least 1 car.  See retrieve_from_train_pool() which returns NULL if
// no car is available.

class TrainGeneration: public CardGeneration {
  friend class Train;
  friend class VMStructs;
 private:
  Train* _first_train;	                // The oldest train
  Train* _last_train;                   // The youngest train
  Train* _special_nonoop_train;         // Special train holding nonoop
					// data (typically large arrays)

  // XX These two are used because we cannot relink as soon as we find
  //    an oversized car. We currently believe the collection set *is*
  //    the first car, so we cannot change the first car in the middle
  //    of a collection. We need a separation of the notion of what's
  //    the first car vs. what's in the collection set -- which we'll
  //    probably implement when we add the ability to collect more than
  //    a single car at a time.
  CarSpace* _relinked_oversized_car;    // Oversized car to be relinked
  Train*    _relinked_train;            // Train it'll be linked to

  CarTable _car_tab;		        // CarTable covering this generation.

  CarTableDesc* _first_car_desc;
  CarSpace* _car_free_list;	        // Free pool of cars
  Train*    _train_free_list;           // Free pool of trains

  julong _total_promoted;               // accumulated size of promoted objects (in words)
  julong _total_processed;              // accumulated size of processed objects (in words)
  julong _delta_promoted;               // saved value of _total_promoted
  julong _delta_processed;              // saved value of _total_processed

  int   _tick_interval;                 // current interval
  int   _next_invoke_count;             // next invoke_count to do train tick
  int   _delay_adjustment_count;        // ticks to skip before starting
                                        //  adaptive adjustment
  CardTableRS* _ct;	                // Card table remset.

  Mutex _train_alloc_lock;              // Mutex to protect par_alloc in train.

  GenerationCounters*  _gen_counters;   // generation specific perf counters
  GSpaceCounters*      _space_counters; // old space specific perf counters

#ifndef PRODUCT
  size_t _starting_length;
  size_t _process_count;
#endif

  void set_first_train(Train* t);
  void set_last_train(Train* t)         { _last_train = t; }
  void set_special_nonoop_train(Train* t);

  CarSpace* car_free_list() const       { return _car_free_list; }
  void set_car_free_list(CarSpace* c)   { _car_free_list = c; }

  Train* train_free_list() const        { return _train_free_list; }
  void set_train_free_list(Train* t)    { _train_free_list = t; }

  julong delta_promoted() const         { return _delta_promoted;  }
  julong delta_processed() const        { return _delta_processed; }
  julong total_promoted() const         { return _total_promoted;  }
  julong total_processed() const        { return _total_processed; }
  void add_total_promoted(size_t n)     { _total_promoted += n;    }
  void add_total_processed(size_t n)    { _total_processed += n;   }

  void coalesce_cars(CarSpace* c1, CarSpace* c2);
  CarSpace* split_car(CarSpace* c, size_t blocks);

  size_t release_train(Train* t);

  // train tick calculations
  void compute_invocation_rate();
  void reset_invocation_rate();

 public:
  TrainGeneration(ReservedSpace rs, 
                  size_t initial_byte_size,
                  int level, 
                  CardTableRS* ct);

  Generation::Name kind() { return Generation::TrainGen; }

  // Accessors
  Train* first_train() const            { return _first_train; }
  Train* last_train() const             { return _last_train; }	
  Train* special_nonoop_train() const   { return _special_nonoop_train; }	

  // Cached block table descriptor of car to be processed
  CarTableDesc* first_car_desc() const  { return _first_car_desc; }
  void set_first_car_desc();

  // The cartable covering this train.
  CarTable* car_table() { return &_car_tab; }

  // Add new highest numbered train. First car must be large enough to 
  // hold 'size' oops.
  Train* add_new_highest_train(size_t size);

  // Release lowest numbered train.
  size_t release_lowest_train()           { return release_train(first_train()); }
  
  // Release lowest numbered car.
  size_t release_lowest_car();

  // Allocate oop of given size highest train
  HeapWord* allocate(size_t size, bool is_large_noref, bool is_tlab);
  HeapWord* par_allocate(size_t size, bool is_large_noref, bool is_tlab);

  oop promote(oop obj, size_t obj_size, oop* ref);

  void save_marks();
  void reset_saved_marks();
  bool no_allocs_since_save_marks();

  oop copy_to_train(oop old, oop* from);

  void scavenge_higher_train_recorded_stores(TrainScanClosure* cl);
  void scavenge_same_train_recorded_stores(TrainScanClosure* cl);
  void scavenge_special_nonoop_train_recorded_stores(TrainScanClosure* cl, 
                                                     bool exclude_lowest_train);
  void tail_trains_set_target(bool value);
  void release_special_nonoop_train_contents();

  void oop_update_remembered_set(oop* p);
  void update_remembered_sets();          
  void clear_remembered_sets();
  inline void weak_ref_barrier_check(oop* p);

  // Iteration
  void object_iterate(ObjectClosure* blk);
  void space_iterate(SpaceClosure* blk, bool usedOnly = false);

  void object_iterate_since_last_GC(ObjectClosure* cl);

  void younger_refs_iterate(OopsInGenClosure* blk);

#define Train_SINCE_SAVE_MARKS_DECL(OopClosureType, nv_suffix)		\
  void oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl);

  ALL_SINCE_SAVE_MARKS_CLOSURES(Train_SINCE_SAVE_MARKS_DECL)

#undef Train_SINCE_SAVE_MARKS_DECL

  CompactibleSpace* first_compaction_space() const;
  void compact();

  // Space enquiries
  size_t capacity() const;
  size_t used() const;
  size_t free() const;
  size_t contiguous_available() const;
  size_t unsafe_max_alloc_nogc() const;
  // These two are the capacity and free space in the
  // trains we know about (not the entire generation).
  size_t train_capacity() const;
  size_t train_free() const;

  HeapWord* block_start(const void* addr) const;
  size_t block_size(const HeapWord* addr) const;
  bool block_is_obj(const HeapWord* addr) const;

  // Pool of unused trains

  // retrieve_from_train_pool() returns NULL is a train with a car cannot be
  // returned.
  Train* retrieve_from_train_pool(size_t size, julong train_number);
  size_t add_to_train_pool(Train* t);

  // Pool of unused cars
  CarSpace* retrieve_from_car_pool(size_t size, julong train_number, 
                                   juint car_number, Train* train);
  void add_to_car_pool(CarSpace* c, bool remset_already_empty);

  // Collapses all consecutive single-car trains into a single train.
  void collapse_cars();

  // Collection.
  virtual bool should_collect(bool   full,
                              size_t word_size,
                              bool   is_large_noref,
                              bool   is_tlab);

  virtual void collect(bool   full,
                       bool   clear_all_soft_refs,
                       size_t size, 
		       bool   is_large_noref,
                       bool   is_tlab);
  HeapWord* expand_and_allocate(size_t size,
				bool is_large_noref, bool is_tlab,
				bool parallel = false);

  // Bails out to MarkSweep for a full collection.
  // Does a "full" (forced) collection invoked on this generation collect
  // all younger generations as well? Note that this is a
  // hack to allow the collection of the younger gen first if the flag is
  // set. This is better than using th policy's should_collect_gen0_first()
  // since that causes us to do an extra unnecessary pair of restart-&-stop-world.
  virtual bool full_collects_younger_generations() const {
    return !CollectGen0First;
  }

  // If the oop is in the train generation, update the train's internal
  // remembered sets.
  void examine_modified_oop(oop* p) {
    if (is_in_reserved(p)) {
      oop_update_remembered_set(p);
    }
  }

  // Mark sweep support
  void compute_new_size();
  void gc_prologue(bool full);
  void gc_epilogue(bool full);
  void release_empty_spaces();
  void prepare_for_compaction(CompactPoint* cp);

  const char* name() const;
  const char* short_name() const { return "Train"; }
  bool must_be_youngest() const { return false; }
  bool must_be_oldest() const { return true; }

  // Performance Counter support
  void update_counters();
  void update_capacity_counters();
  inline void inc_used_counter(size_t size);

  // PrintHeapAtGC support.
  void print() const { print_on(gclog_or_tty); }
  void print_on(outputStream* st) const;

  // Debugging
  void verify(bool allow_dirty)   PRODUCT_RETURN;
  void compute_references_into_lowest_car(size_t* nof_cards, size_t* nof_refs) PRODUCT_RETURN;
  void print_train_processing_statistics(julong prev_first_number) PRODUCT_RETURN;
};
