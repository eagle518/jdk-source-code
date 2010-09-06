#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_win32_amd64.hpp	1.3 03/12/23 16:38:27 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

  // 
  // NOTE: we are back in class os here, not win32
  //
  static jint      (*atomic_xchg_func)          (jint,      volatile jint*);
  static intptr_t  (*atomic_xchg_ptr_func)      (intptr_t,  volatile intptr_t*);

  static jint      (*atomic_cmpxchg_func)       (jint,      volatile jint*,  jint);
  static jlong     (*atomic_cmpxchg_long_func)  (jlong,     volatile jlong*, jlong);

  static jint      (*atomic_add_func)           (jint,      volatile jint*);
  static intptr_t  (*atomic_add_ptr_func)       (intptr_t,  volatile intptr_t*);

  static void      (*fence_func)                ();


  static jint      atomic_xchg_bootstrap        (jint,      volatile jint*);
  static intptr_t  atomic_xchg_ptr_bootstrap    (intptr_t,  volatile intptr_t*);

  static jint      atomic_cmpxchg_bootstrap     (jint,      volatile jint*,  jint);
  static jlong     atomic_cmpxchg_long_bootstrap(jlong,     volatile jlong*, jlong);

  static jint      atomic_add_bootstrap         (jint,      volatile jint*);
  static intptr_t  atomic_add_ptr_bootstrap     (intptr_t,  volatile intptr_t*);

  static void      fence_bootstrap              ();

  static bool      supports_sse() { return true; }


