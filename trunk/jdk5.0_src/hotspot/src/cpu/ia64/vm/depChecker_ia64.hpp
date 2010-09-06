#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)depChecker_ia64.hpp	1.3 03/12/23 16:36:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class DepChecker {
#ifndef PRODUCT
 private:
  typedef address (*check_func)(address start, int length);
  // points to the library.
  static void*    _library;
  // tries to load library and return whether it succedded.
  static bool load_library();
  static check_func _check;
  static bool load_failed;
#endif
 public:
  static void check(address begin, int length) PRODUCT_RETURN;
};
