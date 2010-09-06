#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cartable.hpp	1.38 03/12/23 16:40:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Descriptor for individual entry in car table.
// 

class CarTableDesc VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;

 private:
  julong    _train_number;
  juint     _car_number;
  Train*    _train;
  CarSpace* _space;
  bool      _target;       // true if being evacuated

 public:
   // Constants
  enum SomePublicConstants {
    invalid_train_number        = (julong)  0,
    newgen_train_number         = (julong)  1,
    special_nonoop_train_number = (julong)  2,
    initial_train_number        = (julong)  3,

    invalid_car_number          = (juint)   0,
    initial_car_number          = (juint)   1
  };


  // Accessors
  julong train_number() const         { return _train_number; }
  juint car_number() const            { return _car_number; }
  Train* train() const                { return _train; }
  CarSpace* space() const             { return _space; }
  void set_space(CarSpace* s)         { _space = s; }
  bool target() const                 { return _target; }
  void set_target(bool t)             { _target = t; }

  bool equals(CarTableDesc* d) const {
    return (train_number() == d->train_number()) && (car_number() == d->car_number());
  }

  bool less_than(CarTableDesc* d) const {
    return (train_number() < d->train_number()) || (train_number() == d->train_number() && car_number() < d->car_number());
  }

  // Initialization
  void initialize(julong train_number, juint car_number,
		  Train* train, CarSpace* sp);
};



// A cartable maintains a fast mapping of heap addresses to [train number,
// car number, train, space]
//
// It is only necessary to map the train heap, but in order to avoid bounds
// checks we map the entire heap. The extra space overhead is neglible.

class CarTable: public CHeapObj {
  friend class TrainGeneration;
  friend class VMStructs;

 private:
  size_t        _LogOfCarSpaceSize;// Log of the car size.
  CarTableDesc* _table;            // The actual table
  CarTableDesc* _table_base;       // Adjusted base pointer
  size_t        _table_size;       // Table size

  // Updating table entries
  void update_entry(CarSpace* car, julong train_number,
		    juint car_number, Train* train);
  void clear_entry(CarSpace* car);

  int get_index_work(const void* p) const {
    uintptr_t pi = (uintptr_t)p;
    return ((uintptr_t)low(pi)) >> LogOfCarSpaceSize;
  }

  int get_index(const void* p) const {
    uintptr_t pi = (uintptr_t)p;
    assert((pi & ((1 << LogOfCarSpaceSize)-1)) == 0,
	   "boundary pointer must be valid for CarTable");
    assert(sizeof(pi) == sizeof(jint) || high(pi) == -1,
           "High word must be all ones");
    return get_index_work(p);
  }

 public:
  // Table allocation and initialization.  (The entire reserved region of
  // the TrainGeneration should be provided, since the space overhead is
  // negligable.)
  CarTable(size_t LogOfCarSpaceSize, MemRegion covered);

  // Return train number holding address
  julong train_number_for(const void* p) const {
    return desc_for(p)->train_number();
  }
  // Return car number holding address
  juint car_number_for(const void* p) const {
    return desc_for(p)->car_number();
  }
  // Return train holding address
  Train* train_for(const void* p) const {
    return desc_for(p)->train();
  }
  // Return space holding address
  CarSpace* space_for(const void* p) const {
    return desc_for(p)->space();
  }
  // Return table descriptor for address
  CarTableDesc* desc_for(const void* p) const { 
    CarTableDesc* entry = _table_base + get_index_work(p);
    assert(entry >= _table && entry < _table + _table_size, "Invalid oop");
    return entry;
  }

  // Debugging. Like space_for, but returns NULL if p is outside heap.
  CarSpace* space_for_or_null(const void* p) {
    CarTableDesc* entry = _table_base + get_index_work(p);
    return (entry >= _table && entry < _table + _table_size) ?
      entry->space()
      : NULL;
  }

  // Verification
  void verify() PRODUCT_RETURN;
};
