/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

///////////// Locking verification specific to CMS //////////////
// Much like "assert_lock_strong()", except
// that it relaxes the assertion somewhat for the parallel GC case, where
// main GC thread or the CMS thread might hold the lock on behalf of
// the parallel threads.
class CMSLockVerifier: AllStatic {
 public:
  static void assert_locked(const Mutex* lock, const Mutex* p_lock1, const Mutex* p_lock2)
    PRODUCT_RETURN;
  static void assert_locked(const Mutex* lock, const Mutex* p_lock) {
    assert_locked(lock, p_lock, NULL);
  }
  static void assert_locked(const Mutex* lock) {
    assert_locked(lock, NULL);
  }
};
