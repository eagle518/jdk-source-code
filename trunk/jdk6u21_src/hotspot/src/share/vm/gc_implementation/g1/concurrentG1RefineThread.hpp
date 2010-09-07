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

// Forward Decl.
class ConcurrentG1Refine;

// The G1 Concurrent Refinement Thread (could be several in the future).

class ConcurrentG1RefineThread: public ConcurrentGCThread {
  friend class VMStructs;
  friend class G1CollectedHeap;

  double _vtime_start;  // Initial virtual time.
  double _vtime_accum;  // Initial virtual time.
  int _worker_id;
  int _worker_id_offset;

  // The refinement threads collection is linked list. A predecessor can activate a successor
  // when the number of the rset update buffer crosses a certain threshold. A successor
  // would self-deactivate when the number of the buffers falls below the threshold.
  bool _active;
  ConcurrentG1RefineThread* _next;
  Monitor* _monitor;
  ConcurrentG1Refine* _cg1r;

  int _thread_threshold_step;
  // This thread activation threshold
  int _threshold;
  // This thread deactivation threshold
  int _deactivation_threshold;

  void sample_young_list_rs_lengths();
  void run_young_rs_sampling();
  void wait_for_completed_buffers();

  void set_active(bool x) { _active = x; }
  bool is_active();
  void activate();
  void deactivate();

  // For use by G1CollectedHeap, which is a friend.
  static SuspendibleThreadSet* sts() { return &_sts; }

public:
  virtual void run();
  // Constructor
  ConcurrentG1RefineThread(ConcurrentG1Refine* cg1r, ConcurrentG1RefineThread* next,
                           int worker_id_offset, int worker_id);

  void initialize();

  // Printing
  void print() const;
  void print_on(outputStream* st) const;

  // Total virtual time so far.
  double vtime_accum() { return _vtime_accum; }

  ConcurrentG1Refine* cg1r() { return _cg1r;     }

  // Yield for GC
  void yield();
  // shutdown
  void stop();
};
