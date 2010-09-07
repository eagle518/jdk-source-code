/*
 * Copyright (c) 1998, 2004, Oracle and/or its affiliates. All rights reserved.
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

// An ExtendedPC contains the _pc from a signal handler in a platform
// independent way.

class ExtendedPC VALUE_OBJ_CLASS_SPEC {
 private:
  address _pc;

 public:
  address pc() const { return _pc; }
  ExtendedPC(address pc) { _pc  = pc;   }
  ExtendedPC()           { _pc  = NULL; }
};
