#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)task.cpp	1.19 03/12/23 16:44:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_task.cpp.incl"

int PeriodicTask::_num_tasks = 0;
PeriodicTask* PeriodicTask::_tasks[PeriodicTask::max_tasks];
#ifndef PRODUCT
elapsedTimer PeriodicTask::_timer;
int PeriodicTask::_intervalHistogram[PeriodicTask::max_interval];
int PeriodicTask::_ticks;

void PeriodicTask::print_intervals() {
  if (ProfilerCheckIntervals) {
    for (int i = 0; i < PeriodicTask::max_interval; i++) {
      int n = _intervalHistogram[i];
      if (n > 0) tty->print_cr("%3d: %5d (%4.1f%%)", i, n, 100.0 * n / _ticks);
    }
  }
}
#endif

void PeriodicTask::real_time_tick(size_t delay_time) {
#ifndef PRODUCT
  if (ProfilerCheckIntervals) {
    _ticks++;
    _timer.stop();
    int ms = (int)(_timer.seconds() * 1000.0);
    _timer.reset();
    _timer.start();
    if (ms >= PeriodicTask::max_interval) ms = PeriodicTask::max_interval - 1;
    _intervalHistogram[ms]++;
  }
#endif
  for(int index = 0; index < _num_tasks; index++) {
    _tasks[index]->execute_if_pending(delay_time);
  }
}


PeriodicTask::PeriodicTask(size_t interval_time) :
  _counter(0), _interval(interval_time) {
  assert(is_init_completed(), "Periodic tasks should not start during VM initialization");
  // Sanity check the interval time
  assert(_interval >= PeriodicTask::min_interval &&
         _interval <= PeriodicTask::max_interval &&
         _interval %  PeriodicTask::interval_gran == 0,
              "improper PeriodicTask interval time");
}

PeriodicTask::~PeriodicTask() {
  if (is_enrolled())
    disenroll();
}

bool PeriodicTask::is_enrolled() const {
  for(int index = 0; index < _num_tasks; index++) 
    if (_tasks[index] == this) return true;
  return false;
}

void PeriodicTask::enroll() {
  if (_num_tasks == PeriodicTask::max_tasks)
    fatal("Overflow in PeriodicTask table");
  _tasks[_num_tasks++] = this;
}

void PeriodicTask::disenroll() {
  assert(WatcherThread::watcher_thread() == NULL, "watcher thread should not be running");

  int index;
  for(index = 0; index < _num_tasks && _tasks[index] != this; index++);
  if (index == _num_tasks) return;
  _num_tasks--;
  for (; index < _num_tasks; index++) {
    _tasks[index] = _tasks[index+1];
  }
}

