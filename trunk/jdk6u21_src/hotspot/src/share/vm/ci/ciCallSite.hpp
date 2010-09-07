/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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

// ciCallSite
//
// The class represents a java.dyn.CallSite object.
class ciCallSite : public ciInstance {
public:
  ciCallSite(instanceHandle h_i) : ciInstance(h_i) {}

  // What kind of ciObject is this?
  bool is_call_site() const { return true; }

  // Return the target MethodHandle of this CallSite.
  ciMethodHandle* get_target() const;

  void print();
};
