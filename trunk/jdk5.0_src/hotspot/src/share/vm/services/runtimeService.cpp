#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)runtimeService.cpp	1.5 04/02/06 20:38:37 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_runtimeService.cpp.incl"

TimeStamp RuntimeService::_app_timer;
TimeStamp RuntimeService::_safepoint_timer;
PerfCounter*  RuntimeService::_sync_time_ticks = NULL;
PerfCounter*  RuntimeService::_total_safepoints = NULL;
PerfCounter*  RuntimeService::_safepoint_time_ticks = NULL;
PerfCounter*  RuntimeService::_application_time_ticks = NULL;

void RuntimeService::init() {
  EXCEPTION_MARK;

  if (UsePerfData) {

    _sync_time_ticks =
              PerfDataManager::create_counter(SUN_RT, "safepointSyncTime",
                                              PerfData::U_Ticks, CHECK);
    
    _total_safepoints =
              PerfDataManager::create_counter(SUN_RT, "safepoints",
                                              PerfData::U_Events, CHECK);
    
    _safepoint_time_ticks =
              PerfDataManager::create_counter(SUN_RT, "safepointTime",
                                              PerfData::U_Ticks, CHECK);
    
    _application_time_ticks =
              PerfDataManager::create_counter(SUN_RT, "applicationTime",
                                              PerfData::U_Ticks, CHECK);
  }

}

void RuntimeService::record_safepoint_begin() {
  // update the time stamp to begin recording safepoint time
  _safepoint_timer.update();
  if (UsePerfData) {
    _total_safepoints->inc();
    if (_app_timer.is_updated()) {
      _application_time_ticks->inc(_app_timer.ticks_since_update());
    }
  }
}

void RuntimeService::record_safepoint_synchronized() {
  if (UsePerfData) {
    _sync_time_ticks->inc(_safepoint_timer.ticks_since_update());
  }
}

void RuntimeService::record_safepoint_end() {
  // update the time stamp to begin recording app time
  _app_timer.update();
  if (UsePerfData) {
    _safepoint_time_ticks->inc(_safepoint_timer.ticks_since_update());
  }
}

void RuntimeService::record_application_start() {
  // update the time stamp to begin recording app time
  _app_timer.update();
}

// Don't need to record application end because we currently
// exit at a safepoint and record_safepoint_begin() handles updating
// the application time counter at VM exit.

jlong RuntimeService::safepoint_sync_time_ms() {
  return UsePerfData ? 
    Management::ticks_to_ms(_sync_time_ticks->get_value()) : -1;
}

jlong RuntimeService::safepoint_count() {
  return UsePerfData ? 
    _total_safepoints->get_value() : -1;
}
jlong RuntimeService::safepoint_time_ms() {
  return UsePerfData ? 
    Management::ticks_to_ms(_safepoint_time_ticks->get_value()) : -1;
}

jlong RuntimeService::application_time_ms() {
  return UsePerfData ? 
    Management::ticks_to_ms(_application_time_ticks->get_value()) : -1;
}
