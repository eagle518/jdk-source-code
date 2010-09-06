#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psMarkSweepDecorator.hpp	1.8 03/12/23 16:40:14 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// 
// A PSMarkSweepDecorator is used to add "ParallelScavenge" style mark sweep operations
// to a MutableSpace.
//

class ObjectStartArray;

class PSMarkSweepDecorator: public CHeapObj {
 private:
  static PSMarkSweepDecorator* _destination_decorator;

 protected:
  MutableSpace* _space;
  ObjectStartArray* _start_array;
  HeapWord* _first_dead;
  HeapWord* _end_of_live;
  HeapWord* _compaction_top;
  unsigned int _allowed_dead_ratio;

  bool insert_deadspace(ssize_t& allowed_deadspace_words, HeapWord* q, size_t word_len);

 public:
  PSMarkSweepDecorator(MutableSpace* space, ObjectStartArray* start_array,
                       unsigned int allowed_dead_ratio) :
    _space(space), _start_array(start_array), _allowed_dead_ratio(allowed_dead_ratio) { }

  // During a compacting collection, we need to collapse objects into
  // spaces in a given order. We want to fill space A, space B, and so
  // on. The code that controls that order is in the following methods.
  static void set_destination_decorator_tenured();
  static void set_destination_decorator_perm_gen();
  static void advance_destination_decorator();
  static PSMarkSweepDecorator* destination_decorator();

  // Accessors
  MutableSpace* space()                           { return _space; }
  ObjectStartArray* start_array()                 { return _start_array; }

  HeapWord* compaction_top()                      { return _compaction_top; }
  void set_compaction_top(HeapWord* value)        { _compaction_top = value; }

  unsigned int allowed_dead_ratio()               { return _allowed_dead_ratio; }
  void set_allowed_dead_ratio(unsigned int value) { _allowed_dead_ratio = value; }

  // Work methods
  void adjust_pointers();
  void precompact();
  void compact(bool mangle_free_space);
};




