/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
