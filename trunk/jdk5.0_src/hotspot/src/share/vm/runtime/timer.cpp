#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)timer.cpp	1.26 03/12/23 16:44:22 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_timer.cpp.incl"


void elapsedTimer::add(elapsedTimer t) {
  _counter += t._counter;
}

void elapsedTimer::start() {
  if (!_active) {
    _active = true;
    _start_counter = os::elapsed_counter();
  }
}

void elapsedTimer::stop() {
  if (_active) {
    _counter += os::elapsed_counter() - _start_counter;
    _active = false;
  }
}

double elapsedTimer::seconds() const {
  double count = (double) _counter;
  double freq  = (double) os::elapsed_frequency();
  return count/freq;
}

jlong elapsedTimer::milliseconds() const {
  jlong ticks_per_ms = os::elapsed_frequency() / 1000;
  return _counter / ticks_per_ms;
}

jlong elapsedTimer::active_ticks() const {
  if (!_active) {
    return ticks();
  }
  jlong counter = _counter + os::elapsed_counter() - _start_counter; 
  return counter;
}

void TimeStamp::update_to(jlong ticks) {
  _counter = ticks;
  if (_counter == 0)  _counter = 1;
  assert(is_updated(), "must not look clear");
}

void TimeStamp::update() { 
  update_to(os::elapsed_counter());
}

double TimeStamp::seconds() const {
  assert(is_updated(), "must not be clear");
  jlong new_count = os::elapsed_counter();
  double count = (double) new_count - _counter;
  double freq  = (double) os::elapsed_frequency();
  return count/freq;
}

jlong TimeStamp::milliseconds() const {
  assert(is_updated(), "must not be clear");

  jlong new_count = os::elapsed_counter();
  jlong count = new_count - _counter;
  jlong ticks_per_ms = os::elapsed_frequency() / 1000;
  return count / ticks_per_ms;
}

jlong TimeStamp::ticks_since_update() const {
  assert(is_updated(), "must not be clear");
  return os::elapsed_counter() - _counter;
}

TraceTime::TraceTime(const char* title,
		     bool doit,
		     bool print_cr,
		     outputStream* logfile) {
  _active   = doit;
  _verbose  = true;
  _print_cr = print_cr;
  _logfile = (logfile != NULL) ? logfile : tty;

  if (_active) {
    _accum = NULL;
    if (PrintGCTimeStamps) {
      _logfile->stamp();
      _logfile->print(": ");
    }
    _logfile->print("[%s", title);
    _logfile->flush();
    _t.start();
  }
}

TraceTime::TraceTime(const char* title,
		     elapsedTimer* accumulator,
		     bool doit,
		     bool verbose,
		     outputStream* logfile) {
  _active = doit;
  _verbose = verbose;
  _print_cr = true;
  _logfile = (logfile != NULL) ? logfile : tty;
  if (_active) {
    if (_verbose) {
      if (PrintGCTimeStamps) {
	_logfile->stamp();
	_logfile->print(": ");
      }
      _logfile->print("[%s", title);
      _logfile->flush();
    }
    _accum = accumulator;
    _t.start();
  }
}

TraceTime::~TraceTime() {
  if (_active) {
    _t.stop();
    if (_accum!=NULL) _accum->add(_t);
    if (_verbose) {
      if (_print_cr) {
        _logfile->print_cr(", %3.7f secs]", _t.seconds());
      } else {
        _logfile->print(", %3.7f secs]", _t.seconds());
      }
      _logfile->flush();
    }
  }
}
