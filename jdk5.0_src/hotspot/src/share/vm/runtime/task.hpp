#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)task.hpp	1.15 03/12/23 16:44:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A PeriodicTask has the sole purpose of executing its task
// function with regular intervals.
// Usage:
//   PeriodicTask pf(10);
//   pf.enroll();
//   ...
//   pf.disenroll();

class PeriodicTask: public CHeapObj {
 public:
  // Useful constants.
  // The interval constants are used to ensure the declared interval
  // is appropriate;  it must be between min_interval and max_interval,
  // and have a granularity of interval_gran (all in millis).
  enum { max_tasks     = 10,       // Max number of periodic tasks in system
         interval_gran = 10,       
         min_interval  = 10, 
         max_interval  = 5000 };

  static int num_tasks()   { return _num_tasks; }

 private:
  size_t _counter;
  const size_t _interval;

  static int _num_tasks;
  static PeriodicTask* _tasks[PeriodicTask::max_tasks];
  static void real_time_tick(size_t delay_time);

#ifndef PRODUCT
  static elapsedTimer _timer;                      // measures time between ticks
  static int _ticks;                               // total number of ticks
  static int _intervalHistogram[max_interval];     // to check spacing of timer interrupts
 public:
  static void print_intervals();
#endif
  // Only the WatcherThread can cause us to execute PeriodicTasks
  friend class WatcherThread;
 public:
  PeriodicTask(size_t interval_time); // interval is in milliseconds of elapsed time
  ~PeriodicTask();

  // Tells whether is enrolled
  bool is_enrolled() const;

  // Make the task active
  void enroll();

  // Make the task deactive
  void disenroll();

  void execute_if_pending(size_t delay_time) {
    _counter += delay_time;
    if (_counter >= _interval) {
      task();
      _counter = 0;
    }
  }

  // Returns how long (time in milliseconds) before the next time we should
  // execute this task.
  size_t time_to_next_interval() const {
    assert(_interval > _counter,  "task counter greater than interval?");
    return _interval - _counter;
  }

  // Calculate when the next periodic task will fire.
  // Called by the WatcherThread's run method.
  // This assumes that periodic tasks aren't entering
  // and leaving the system dynamically, except for
  // during startup.
  static size_t time_to_wait() {
    // The WatcherThread should only be started if there are
    // Periodic tasks in the system.
    assert(_num_tasks > 0, "no tasks?" );

    size_t delay = _tasks[0]->time_to_next_interval();
    for (int index = 1; index < _num_tasks; index++) {
      delay = MIN2(delay, _tasks[index]->time_to_next_interval());
    }
    return delay;
  }

  // The task to perform at each period
  virtual void task() = 0;
};

