#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadls_win32_amd64.hpp	1.2 03/12/23 16:38:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage

protected:

  static int _thread_ptr_offset;

public:

  // Java Thread
  static inline Thread* thread() { 
    return (Thread*)TlsGetValue(thread_index());
  }

  static inline Thread* get_thread() { 
    return (Thread*)TlsGetValue(thread_index());
  }

  static inline void set_thread_ptr_offset( int offset ) { _thread_ptr_offset = offset; }

  static inline int get_thread_ptr_offset() { return _thread_ptr_offset; }

