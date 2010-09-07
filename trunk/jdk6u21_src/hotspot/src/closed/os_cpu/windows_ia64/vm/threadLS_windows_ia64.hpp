/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage
public:
  static inline intptr_t thread_offset() {return thread_index() * sizeof(intptr_t);}

  // Java Thread
  static inline Thread *ThreadLocalStorage::get_thread() {
    return (Thread*)TlsGetValue(thread_index());
  }

  static inline Thread* thread() { return get_thread(); }
