#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_win32_ia64.hpp	1.8 03/12/23 16:38:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage
public:
  static inline intptr_t thread_offset() {return thread_index() * sizeof(intptr_t);}

  // Java Thread
  static inline Thread *ThreadLocalStorage::get_thread() { 
    return (Thread*)TlsGetValue(thread_index());
  }

  static inline Thread* thread() { return get_thread(); }

