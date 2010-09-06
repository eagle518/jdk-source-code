#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)timer.hpp	1.28 03/12/23 16:44:23 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Timers for simple measurement.

class elapsedTimer VALUE_OBJ_CLASS_SPEC {
  friend class VMStructs;
 private:
  jlong _counter;
  jlong _start_counter;
  bool  _active;
 public:
  elapsedTimer()             { _active = false; reset(); }
  void add(elapsedTimer t);
  void start();
  void stop();
  void reset()               { _counter = 0; }
  double seconds() const;
  jlong milliseconds() const;
  jlong ticks() const        { return _counter; }
  jlong active_ticks() const;
};

// TimeStamp is used for recording when an event took place.
class TimeStamp VALUE_OBJ_CLASS_SPEC {
 private:
  jlong _counter;
 public:
  TimeStamp()  { _counter = 0; }
  void clear() { _counter = 0; }
  // has the timestamp been updated since being created or cleared?
  bool is_updated() const { return _counter != 0; }
  // update to current elapsed time
  void update();
  // update to given elapsed time
  void update_to(jlong ticks);
  // returns seconds since updated
  // (must not be in a cleared state:  must have been previously updated)
  double seconds() const;
  jlong milliseconds() const;
  // ticks elapsed between VM start and last update
  jlong ticks() const { return _counter; }
  // ticks elapsed since last update
  jlong ticks_since_update() const;
};

// TraceTime is used for tracing the execution time of a block
// Usage:
//  { TraceTime t("block time")
//    some_code();
//  }
//

class TraceTime: public StackObj {
 private:
  bool          _active;    // do timing
  bool          _verbose;   // report every timing
  bool          _print_cr;  // add a CR to the end of the timer report
  elapsedTimer  _t;         // timer
  elapsedTimer* _accum;     // accumulator
  outputStream* _logfile;   // output log file
 public:
  // Constuctors
  TraceTime(const char* title,
	    bool doit = true,
	    bool print_cr = true,
	    outputStream *logfile = NULL);
  TraceTime(const char* title,
	    elapsedTimer* accumulator,
	    bool doit = true,
            bool verbose = false,
	    outputStream *logfile = NULL );
  ~TraceTime();

  // Accessors
  void set_verbose(bool verbose)  { _verbose = verbose; }
  bool verbose() const            { return _verbose;    }

  // Activation
  void suspend()  { if (_active) _t.stop();  }
  void resume()   { if (_active) _t.start(); }
};
