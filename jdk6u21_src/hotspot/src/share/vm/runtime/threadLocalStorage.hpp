/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
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

// Interface for thread local storage

// Fast variant of ThreadLocalStorage::get_thread_slow
extern "C" Thread*   get_thread();

// Get raw thread id: e.g., %g7 on sparc, fs or gs on x86
extern "C" uintptr_t _raw_thread_id();

class ThreadLocalStorage : AllStatic {
 public:
  static void    set_thread(Thread* thread);
  static Thread* get_thread_slow();
  static void    invalidate_all() { pd_invalidate_all(); }

  // Machine dependent stuff
  #include "incls/_threadLS_pd.hpp.incl"

 public:
  // Accessor
  static inline int  thread_index()              { return _thread_index; }
  static inline void set_thread_index(int index) { _thread_index = index; }

  // Initialization
  // Called explicitly from VMThread::activate_system instead of init_globals.
  static void init();
  static bool is_initialized();

 private:
  static int     _thread_index;

  static void    generate_code_for_get_thread();

  // Processor dependent parts of set_thread and initialization
  static void pd_set_thread(Thread* thread);
  static void pd_init();
  // Invalidate any thread cacheing or optimization schemes.
  static void pd_invalidate_all();

};
