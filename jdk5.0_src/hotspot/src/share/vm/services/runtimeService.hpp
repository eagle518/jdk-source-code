#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)runtimeService.hpp	1.3 03/12/23 16:44:37 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

class RuntimeService : public AllStatic {
private:
  static PerfCounter* _sync_time_ticks;        // Accumulated time spent getting to safepoints
  static PerfCounter* _total_safepoints;
  static PerfCounter* _safepoint_time_ticks;   // Accumulated time at safepoints
  static PerfCounter* _application_time_ticks; // Accumulated time not at safepoints

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
};
