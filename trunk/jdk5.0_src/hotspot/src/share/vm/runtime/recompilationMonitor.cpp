#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)recompilationMonitor.cpp	1.36 03/12/23 16:44:02 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_recompilationMonitor.cpp.incl"

// Adjusts compile threshold based on compilation overhead and interpretation overhead.
// Unresolved problem: in multithreaded programs, the interpretation overhead cannot be
// easily measured since we don't know which thread was on the CPU when the tick happened.

RecompilationMonitor* RecompilationMonitor::_this = NULL;


RecompilationMonitor::RecompilationMonitor() 
: PeriodicTask(RecompilationMonitorIntervalLength),  
  _n(RecompilationMonitorIntervals),  
  _factor(100.0 / (RecompilationMonitorIntervalLength * 1e-3 * RecompilationMonitorIntervals)) {
  assert(UseAdaptiveThresholds, "must be turned on");
  _compile_sum = _interpreter_sum = 0;
  _compile_time     = NEW_C_HEAP_ARRAY(double, _n);
  _interpreter_time = NEW_C_HEAP_ARRAY(double, _n);
  for (unsigned i = 0; i < _n; i++) _compile_time[i] = _interpreter_time[i] = 0;
  _current_interval = 0;
  _nofAdjustments = 0;
  _ticks = 0;
  _this = this;
  if (UseCompiler && (UseAdaptiveThresholds || UseCounterDecay)) enroll();
}

void RecompilationMonitor::task() {
  assert(UseAdaptiveThresholds, "must be turned on");  
  MutexLocker ml(RecompilationMonitor_lock);
  ++_ticks;
  advance_interval();
  adjust_invocation_counter_limit(true);
  if (PrintRecompilationMonitoring && Verbose) 
    tty->print("\n*tick %d: %3.1f%% comp, %3.1f%% int\n", _ticks, compilation_percentage(), interpreter_percentage());
}

void RecompilationMonitor::register_compile(double seconds) {
  // Since the interval timer isn't completely accurate, we may end up with >100%
  // if ticks are delayed or dropped.  It can also happen with very long compiles
  // (> RecompilationMonitorIntervalLength*RecompilationMonitorIntervals).
  if (!UseAdaptiveThresholds) return;
  { MutexLocker ml(RecompilationMonitor_lock);
    _compile_time[_current_interval] += seconds;
    _compile_sum += seconds;
  adjust_invocation_counter_limit(false);
  }
}


void RecompilationMonitor::advance_interval() {
  // record interpreter ticks
  static int old_all_int_ticks = 0;
  int delta = (FlatProfiler::all_int_ticks - old_all_int_ticks);
  old_all_int_ticks = FlatProfiler::all_int_ticks;
  _interpreter_time[_current_interval] = delta * (FlatProfiler::MillisecsPerTick * 1e-3);
  _interpreter_sum += _interpreter_time[_current_interval];

  // advance current interval
  _current_interval = (_current_interval == _n - 1) ? 0 : _current_interval + 1;
  _nofAdjustments = 0;
  // remove oldest interval from sum and clear it
  _compile_sum     -= _compile_time[_current_interval];
  _interpreter_sum -= _interpreter_time[_current_interval];

  // sanity check on compile_sum
  if (_compile_sum < 0.0) {
    // shouldn't happen (except for rounding errors); simply is defensive programming
    if (PrintRecompilationMonitoring && _compile_sum < -0.01) 
      tty->print("\n*tick %d: sum < 0!!! (%g)\n", _ticks, _compile_sum);
    _compile_sum = 0;           
  }
  _compile_time[_current_interval] = _interpreter_time[_current_interval] = 0;
}

inline double RecompilationMonitor::prev_interval_compile_time() const {
  // compile time during previous interval
  int i = _current_interval ? _current_interval - 1 : _n - 1;
  return _compile_time[i];
}

inline bool compile_overhead_too_high(int compile_overhead) {
  return compile_overhead > CompilationPercentageThresholdHigh;
}

inline bool compile_overhead_is_low(int compile_overhead) {
  return compile_overhead <= CompilationPercentageThresholdLow;
}

#ifdef CAN_MEASURE_INTERPRETER_OVERHEAD
inline bool interpreter_overhead_too_low(int interpreter_overhead) {
  return interpreter_overhead <= InterpreterOverheadThresholdLow;
}

inline bool interpreter_overhead_too_high(int interpreter_overhead) {
  return interpreter_overhead > InterpreterOverheadThresholdHigh;
}
#else
// on Win32, there's no simple way to measure the interpreter overhead
// of multithreaded programs
inline bool interpreter_overhead_too_low(int interpreter_overhead) {
  return false;
}

inline bool interpreter_overhead_too_high(int interpreter_overhead) {
  return true;
}
#endif

void RecompilationMonitor::adjust_invocation_counter_limit(bool is_new_interval) {
  if (!UseAdaptiveThresholds) return;
  if (_nofAdjustments > 0) return;    // only one per interval
  if (_ticks < _n) return;            // startup period -- don't adjust yet

  int compile_overhead     = (int)compilation_percentage();
  int interpreter_overhead = (int)interpreter_percentage();
  if (compile_overhead_too_high(compile_overhead) || interpreter_overhead_too_low(interpreter_overhead)) {
    // may need to adjust invocation counter upwards
    if (CompileThreshold < CompileThresholdMax) {
      // only adjust if the interval just completed contributed to the bad situation
      if (!is_new_interval || 
          prev_interval_compile_time() > RecompilationMonitorIntervalLength * 0.01 * CompilationPercentageThresholdLow) {
        CompileThreshold = MIN2(CompileThresholdMax, intx(CompileThreshold * (CompileThresholdAdjustFactor * 0.01)));
        log("upwards");
        InvocationCounter::reinitialize(false);
      }
    }
  } else if ((interpreter_overhead_too_high(interpreter_overhead) && compile_overhead_is_low(compile_overhead))
          || interpreter_overhead > CompileThresholdMax) {
    // need to adjust invocation counter downwards
    // (the second condition above is intended to accelerate compilation when the interpretation
    // overhead is very high)
    if (CompileThreshold > CompileThresholdMin) {
      CompileThreshold = MAX2(CompileThresholdMin, intx(CompileThreshold / (CompileThresholdAdjustFactor * 0.01)));
      log("downwards");
      InvocationCounter::reinitialize(false);
    }
  }
}

void RecompilationMonitor::log(const char* msg) {
  _nofAdjustments++;
  Events::log("adjusting recompilation counter %s to %d", msg, (intptr_t)CompileThreshold);
  if (PrintRecompilationMonitoring) {
    tty->stamp();
    tty->print(": ");
    tty->print("*at tick %d: adjusting recompilation %s to %d (%d%% comp, %d%% int)\n", 
               _ticks, msg, CompileThreshold, (int)compilation_percentage(), (int)interpreter_percentage());
  }
}

void RecompilationMonitor::start_recompilation_monitor_task() {
  if (UseAdaptiveThresholds) {
    new RecompilationMonitor();
  }
}

void RecompilationMonitor::stop_recompilation_monitor_task() {
  if (_this != NULL)  RecompilationMonitor::_this->disenroll();
  _this = NULL;
}

void RecompilationMonitor::register_compilation(double seconds) {
  if (_this != NULL)  _this->register_compile(seconds);
}

//----------------------------------------------------------------------------------------------

jlong CounterDecay::_last_timestamp = 0;


static void do_method(methodOop m) {
  m->invocation_counter()->decay();
}


void CounterDecay::decay() {  
  if (!is_decay_needed()) return;
  _last_timestamp = os::javaTimeMillis();

  // This operation is going to be performed only at the beginning of a safepoint
  // and hence GC's will not be going on, all Java mutators are suspended
  // at this point and hence SystemDictionary_lock is also not needed
  assert(SafepointSynchronize::is_at_safepoint(), "can only be executed at a safepoint");

  int nclasses = SystemDictionary::number_of_classes();
  double classes_per_tick = nclasses * (CounterDecayMinIntervalLength * 1e-3 / CounterHalfLifeTime);

  for (int i = 0; i < classes_per_tick; i++) {
    klassOop k = SystemDictionary::try_get_next_class();
    if (k != NULL && k->klass_part()->oop_is_instance()) {
      instanceKlass::cast(k)->methods_do(do_method);
    }
  }  
}
