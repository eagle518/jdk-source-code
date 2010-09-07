/*
 * Copyright (c) 2003, 2006, Oracle and/or its affiliates. All rights reserved.
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

class RuntimeService : public AllStatic {
private:
  static PerfCounter* _sync_time_ticks;        // Accumulated time spent getting to safepoints
  static PerfCounter* _total_safepoints;
  static PerfCounter* _safepoint_time_ticks;   // Accumulated time at safepoints
  static PerfCounter* _application_time_ticks; // Accumulated time not at safepoints
  static PerfCounter* _thread_interrupt_signaled_count;// os:interrupt thr_kill
  static PerfCounter* _interrupted_before_count;  // _INTERRUPTIBLE OS_INTRPT
  static PerfCounter* _interrupted_during_count;  // _INTERRUPTIBLE OS_INTRPT

  static TimeStamp _safepoint_timer;
  static TimeStamp _app_timer;

public:
  static void init();

  static jlong safepoint_sync_time_ms();
  static jlong safepoint_count();
  static jlong safepoint_time_ms();
  static jlong application_time_ms();

  static double last_safepoint_time_sec()      { return _safepoint_timer.seconds(); }
  static double last_application_time_sec()    { return _app_timer.seconds(); }

  // callbacks
  static void record_safepoint_begin();
  static void record_safepoint_synchronized();
  static void record_safepoint_end();
  static void record_application_start();

  // interruption events
  static void record_interrupted_before_count();
  static void record_interrupted_during_count();
  static void record_thread_interrupt_signaled_count();
};
