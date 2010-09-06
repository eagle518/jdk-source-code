#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)monitorChunk.hpp	1.13 03/12/23 16:43:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Data structure for holding monitors for one activation during
// deoptimization.

class MonitorChunk: public CHeapObj {
 private:
  int              _number_of_monitors;
  BasicObjectLock* _monitors;
  BasicObjectLock* monitors() const { return _monitors; }
  MonitorChunk*    _next;
 public:
  // Constructor
  MonitorChunk(int number_on_monitors);
  ~MonitorChunk();

  // link operations
  MonitorChunk* next() const                { return _next; }
  void set_next(MonitorChunk* next)         { _next = next; }

  // Tells whether the monitor chunk is linked into the JavaThread
  bool is_linked() const                    { return next() != NULL; }

  // Returns the number of monitors
  int number_of_monitors() const { return _number_of_monitors; }

  // Returns the index'th monitor
  BasicObjectLock* at(int index)            { assert(index >= 0 && index < number_of_monitors(), "out of bounds check"); return &monitors()[index]; }

  
  // Memory management
  void oops_do(OopClosure* f);

  // Tells whether the addr point into the monitors.
  bool contains(void* addr) const           { return (addr >= (void*) monitors()) && (addr <  (void*) (monitors() + number_of_monitors())); }
};
