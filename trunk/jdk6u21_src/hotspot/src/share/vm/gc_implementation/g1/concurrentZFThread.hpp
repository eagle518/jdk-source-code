/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// The Concurrent ZF Thread.  Performs concurrent zero-filling.

class ConcurrentZFThread: public ConcurrentGCThread {
  friend class VMStructs;
  friend class ZeroFillRegionClosure;

 private:

  // Zero fill the heap region.
  void processHeapRegion(HeapRegion* r);

  // Stats
  //   Allocation (protected by heap lock).
  static int _region_allocs;  // Number of regions allocated
  static int _sync_zfs;       //   Synchronous zero-fills +
  static int _zf_waits;      //   Wait for conc zero-fill completion.

  // Number of regions CFZ thread fills.
  static int _regions_filled;

  double _vtime_start;  // Initial virtual time.

  // These are static because the "print_summary_info" method is, and
  // it currently assumes there is only one ZF thread.  We'll change when
  // we need to.
  static double _vtime_accum;  // Initial virtual time.
  static double vtime_accum() { return _vtime_accum; }

  // Offer yield for GC.  Returns true if yield occurred.
  bool offer_yield();

 public:
  // Constructor
  ConcurrentZFThread();

  // Main loop.
  virtual void run();

  // Printing
  void print_on(outputStream* st) const;
  void print() const;

  // Waits until "r" has been zero-filled.  Requires caller to hold the
  // ZF_mon.
  static void wait_for_ZF_completed(HeapRegion* r);

  // Get or clear the current unclean region.  Should be done
  // while holding the ZF_needed_mon lock.

  // shutdown
  void stop();

  // Stats
  static void note_region_alloc() {_region_allocs++; }
  static void note_sync_zfs() { _sync_zfs++; }
  static void note_zf_wait() { _zf_waits++; }
  static void note_region_filled() { _regions_filled++; }

  static void print_summary_info();
};
