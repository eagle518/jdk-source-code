#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ageTable.hpp	1.23 03/12/23 16:40:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/* Copyright 1992 Sun Microsystems, Inc. and Stanford University.
   See the LICENSE file for license information. */

// Age table for adaptive feedback-mediated tenuring (scavenging)
//
// Note: all sizes are in oops

class ageTable VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;

 public:
  // constants
  enum { table_size = markOopDesc::max_age + 1 };

  // instance variables
  size_t sizes[table_size];

  // constructor.  "global" indicates that this is the global age table
  // (as opposed to gc-thread-local)
  ageTable(bool global = true);

  // clear table
  void clear();

  // add entry
  void add(oop p, size_t oop_size) {
    int age = p->age();
    assert(age > 0 && age < table_size, "invalid age of object");
    sizes[age] += oop_size; 
  }

  // Merge another age table with the current one.  Used
  // for parallel young generation gc.
  void merge(ageTable* subTable);

  // calculate new tenuring threshold based on age information
  int compute_tenuring_threshold(size_t survivor_capacity);

 private:
  PerfVariable* _perf_sizes[table_size];
};
