#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cmsLockVerifier.hpp	1.3 03/12/23 16:40:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

///////////// Locking verification specific to CMS //////////////
// Much like "assert_lock_strong()", except
// that it relaxes the assertion somewhat for the parallel GC case, where
// main GC thread or the CMS thread might hold the lock on behalf of
// the parallel threads.
class CMSLockVerifier: AllStatic {
 public:
  static void assert_locked(const Mutex* lock, const Mutex* p_lock)
    PRODUCT_RETURN;
  static void assert_locked(const Mutex* lock) {
    assert_locked(lock, NULL);
  }
};
