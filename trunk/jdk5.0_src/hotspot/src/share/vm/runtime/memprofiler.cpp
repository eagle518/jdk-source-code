#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)memprofiler.cpp	1.17 03/12/23 16:43:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_memprofiler.cpp.incl"

#ifndef PRODUCT

// --------------------------------------------------------
// MemProfilerTask

class MemProfilerTask : public PeriodicTask {
 public:
  MemProfilerTask(int interval_time) : PeriodicTask(interval_time) {}
  void task();
};


void MemProfilerTask::task() {
  // Get thread lock to provide mutual exclusion, and so we can iterate safely over the thread list.
  MutexLocker mu(Threads_lock);
  MemProfiler::do_trace();
}


//----------------------------------------------------------
// Implementation of MemProfiler

MemProfilerTask* MemProfiler::_task   = NULL;
FILE*            MemProfiler::_log_fp = NULL;


bool MemProfiler::is_active() {
  return _task != NULL;
}


void MemProfiler::engage() {
  const char *log_name = "mprofile.log";
  if (!is_active()) {
    // Create log file
    _log_fp = fopen(log_name , "w+");
    if (_log_fp == NULL) {
      fatal1("MemProfiler: Cannot create log file: %s", log_name);
    }
    fprintf(_log_fp, "MemProfiler: sizes are in Kb, time is in seconds since startup\n\n");
    fprintf(_log_fp, "  time, #thr, #cls,  heap,  heap,  perm,  perm,  code, hndls, rescs, oopmp\n");
    fprintf(_log_fp, "                     used, total,  used, total, total, total, total, total\n"); 
    fprintf(_log_fp, "--------------------------------------------------------------------------\n");     

    _task = new MemProfilerTask(MemProfilingInterval);
    _task->enroll();    
  }
}


void MemProfiler::disengage() {
  if (!is_active()) return;
  // Do one last trace at disengage time
  do_trace();

  // Close logfile
  fprintf(_log_fp, "MemProfiler detached\n");
  fclose(_log_fp);

  // remove MemProfilerTask
  assert(_task != NULL, "sanity check");
  _task->disenroll();
  delete _task;
  _task = NULL;
}


void MemProfiler::do_trace() {  
  // Calculate thread local sizes
  size_t handles_memory_usage    = VMThread::vm_thread()->handle_area()->size_in_bytes();
  size_t resource_memory_usage   = VMThread::vm_thread()->resource_area()->size_in_bytes();
  JavaThread *cur = Threads::first();
  while (cur != NULL) {    
    handles_memory_usage  += cur->handle_area()->size_in_bytes();
    resource_memory_usage += cur->resource_area()->size_in_bytes();
    cur = cur->next();
  }
  
  // Print trace line in log
  fprintf(_log_fp, "%6.1f,%5d,%5d,%6ld,%6ld,%6ld,%6ld,", 
      os::elapsedTime(),
      Threads::number_of_threads(),
      SystemDictionary::number_of_classes(),
      Universe::heap()->used() / K,
      Universe::heap()->capacity() / K,
      Universe::heap()->permanent_used() / HWperKB,
      Universe::heap()->permanent_capacity() / HWperKB);

  NOT_CORE(fprintf(_log_fp, "%6ld,", CodeCache::capacity() / K);)

  fprintf(_log_fp, "%6ld,%6ld,%6ld\n",
      handles_memory_usage / K,
      resource_memory_usage / K,
      OopMapCache::memory_usage() / K);
  fflush(_log_fp);
}

#endif 
