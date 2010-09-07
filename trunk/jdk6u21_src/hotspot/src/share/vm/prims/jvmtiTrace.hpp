/*
 * Copyright (c) 1999, 2007, Oracle and/or its affiliates. All rights reserved.
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

///////////////////////////////////////////////////////////////
//
// class JvmtiTrace
//
// Support for JVMTI tracing code
//

// Support tracing except in product build on the client compiler
#ifndef PRODUCT
#define JVMTI_TRACE 1
#else
#ifdef COMPILER2
#define JVMTI_TRACE 1
#endif
#endif

#ifdef JVMTI_TRACE

class JvmtiTrace : AllStatic {

  static bool        _initialized;
  static bool        _on;
  static bool        _trace_event_controller;
  static jbyte       _trace_flags[];
  static jbyte       _event_trace_flags[];
  static const char* _event_names[];
  static jint        _max_function_index;
  static jint        _max_event_index;
  static short       _exclude_functions[];
  static const char* _function_names[];

public:

  enum {
    SHOW_IN =              01,
    SHOW_OUT =             02,
    SHOW_ERROR =           04,
    SHOW_IN_DETAIL =      010,
    SHOW_OUT_DETAIL =     020,
    SHOW_EVENT_TRIGGER =  040,
    SHOW_EVENT_SENT =    0100
  };

  static bool tracing()                     { return _on; }
  static bool trace_event_controller()      { return _trace_event_controller; }
  static jbyte trace_flags(int num)         { return _trace_flags[num]; }
  static jbyte event_trace_flags(int num)   { return _event_trace_flags[num]; }
  static const char* function_name(int num) { return _function_names[num]; } // To Do: add range checking

  static const char* event_name(int num) {
    static char* ext_event_name = (char*)"(extension event)";
    if (num >= JVMTI_MIN_EVENT_TYPE_VAL && num <= JVMTI_MAX_EVENT_TYPE_VAL) {
      return _event_names[num];
    } else {
      return ext_event_name;
    }
  }

  static const char* enum_name(const char** names, const jint* values, jint value);

  static void initialize();
  static void shutdown();

  // return a valid string no matter what state the thread is in
  static const char *safe_get_thread_name(Thread *thread);

  // return the name of the current thread
  static const char *safe_get_current_thread_name();

  // return a valid string no matter what the state of k_mirror
  static const char *get_class_name(oop k_mirror);
};

#endif /*JVMTI_TRACE */
