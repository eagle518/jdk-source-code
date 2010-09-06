#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_win32_i486.hpp	1.12 03/12/23 16:38:31 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

  // 
  // NOTE: we are back in class os here, not win32
  //
  static jlong (*atomic_cmpxchg_long_func)  (jlong, volatile jlong*, jlong);
  static jlong atomic_cmpxchg_long_bootstrap(jlong, volatile jlong*, jlong);

  static bool supports_sse() { return true; }
