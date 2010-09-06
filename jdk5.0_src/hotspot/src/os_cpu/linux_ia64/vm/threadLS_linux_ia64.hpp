#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadLS_linux_ia64.hpp	1.8 03/12/23 16:38:14 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Processor dependent parts of ThreadLocalStorage

public:
  static Thread* thread() {
    return (Thread *) os::thread_local_storage_at(thread_index());
  }
