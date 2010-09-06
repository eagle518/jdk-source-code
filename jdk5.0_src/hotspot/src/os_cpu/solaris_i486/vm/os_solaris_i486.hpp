#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_solaris_i486.hpp	1.14 04/03/28 16:24:59 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

  //
  // NOTE: we are back in class os here, not Solaris
  //
  static jint  (*atomic_xchg_func)        (jint,  volatile jint*);
  static jint  (*atomic_cmpxchg_func)     (jint,  volatile jint*,  jint);
  static jlong (*atomic_cmpxchg_long_func)(jlong, volatile jlong*, jlong);
  static jint  (*atomic_add_func)         (jint,  volatile jint*);
  static void  (*fence_func)              ();

  static jint  atomic_xchg_bootstrap        (jint,  volatile jint*);
  static jint  atomic_cmpxchg_bootstrap     (jint,  volatile jint*,  jint);
  static jlong atomic_cmpxchg_long_bootstrap(jlong, volatile jlong*, jlong);
  static jint  atomic_add_bootstrap         (jint,  volatile jint*);
  static void  fence_bootstrap              ();

  static bool supports_sse();
  static bool is_allocatable(size_t bytes);
