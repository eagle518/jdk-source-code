#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)recompilationMonitor.hpp	1.19 03/12/23 16:44:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Monitors (re)compilation and ensures that the time spent compiling stays
// between CompilationPercentageThresholdLow and CompilationPercentageThresholdHigh.

class RecompilationMonitor: public PeriodicTask {
  double* _compile_time;          // compile time in past intervals (in seconds)
  double  _compile_sum;           // sum of _compile_time (in seconds)
  double* _interpreter_time;      // interpreter time in past intervals (in seconds)
  double  _interpreter_sum;       // sum of _interpreter_time (in seconds)
  const unsigned _n;              // number of intervals in sliding average
  const double _factor;           // factor to convert to percents (to avoid FP division)
  unsigned _current_interval;     // current interval [0.._n-1]
  int      _nofAdjustments;       // # of adjustments made during this interval
  unsigned _ticks;                // total # of ticks so far  

  static RecompilationMonitor* _this;   // only instance of monitor
  RecompilationMonitor();

 public:
  ~RecompilationMonitor();

  virtual void task();
  static void register_compilation(double seconds);
  static void start_recompilation_monitor_task();
  static void  stop_recompilation_monitor_task();

 private:
  void   register_compile(double seconds);
  void   advance_interval();
  double compilation_percentage() const              { return _compile_sum * _factor; }
  double interpreter_percentage() const              { return _interpreter_sum * _factor; }
  void   adjust_invocation_counter_limit(bool is_new_interval);
  void   log(const char* msg);
  inline double prev_interval_compile_time() const;
};

//
// CounterDecay 
//
// Interates through invocation counters and decrements them. This
// is done at each safepoint. 
//
class CounterDecay : public AllStatic {   
  static jlong _last_timestamp;
 public: 
  static  void decay();
  static  bool is_decay_needed() { return (os::javaTimeMillis() - _last_timestamp) > CounterDecayMinIntervalLength; }  
};

