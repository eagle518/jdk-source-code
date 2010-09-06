#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cartable.cpp	1.31 03/12/23 16:40:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_cartable.cpp.incl"


void CarTableDesc::initialize(julong train_number, juint car_number,
			      Train* train, CarSpace* sp) { 
  _train_number = train_number;
  _car_number = car_number;
  _train = train;
  _space = sp;
  _target = false;
}

CarTable::CarTable(size_t LogOfCarSpaceSize, MemRegion covered) {
  if (LogOfCarSpaceSize > Generation::LogOfGenGrain)
    vm_exit_during_initialization("Car Space size cannot be larger than "
				  "the generation granularity 2^16.");
  _LogOfCarSpaceSize = LogOfCarSpaceSize;
  size_t covered_start_index = get_index(covered.start());
  size_t covered_end_index = get_index(covered.end());
  _table_size = covered_end_index - covered_start_index;
  _table      = NEW_C_HEAP_ARRAY(CarTableDesc, _table_size);
  _table_base = _table - covered_start_index;
  for (size_t i = covered_start_index; i < covered_end_index; i++) {
    _table_base[i].initialize(CarTableDesc::invalid_train_number,
			      CarTableDesc::invalid_car_number,
			      NULL, NULL);
  }
}

void CarTable::update_entry(CarSpace* car, julong train_number, juint car_number, Train* train) {
  CarTableDesc* d = car->desc();
  size_t blocks = car->blocks();
  while (blocks--) {
    d->initialize(train_number, car_number, train, car);
    d++;
  }
}


void CarTable::clear_entry(CarSpace* car) {
  CarTableDesc* d = car->desc();
  size_t blocks = car->blocks();
  while (blocks--) {
    d->initialize(CarTableDesc::invalid_train_number,
		  CarTableDesc::invalid_car_number, NULL, NULL);
    d++;
  }
}


#ifndef PRODUCT

void CarTable::verify() {
  for (size_t i = 0; i < _table_size; i++) {
    CarTableDesc* d = &(_table[i]);
    if (d->train() == NULL) {
      guarantee(d->train_number() == CarTableDesc::invalid_train_number,
		"Illegal car table entry");
      guarantee(d->car_number() == CarTableDesc::invalid_car_number,
		"Illegal car table entry");
      guarantee(d->space() == NULL, "Illegal car table entry");
    } else {
      guarantee(d->train_number() >= CarTableDesc::special_nonoop_train_number,
		"Illegal car table entry");
      guarantee(d->car_number() >= CarTableDesc::initial_car_number,
		"Illegal car table entry");
      guarantee(((CarSpace*) d->space())->desc()->equals(d),
		"Illegal car table entry");
    }
  }
}

#endif
