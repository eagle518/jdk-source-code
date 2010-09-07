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

// ciMethodHandle
//
// The class represents a java.dyn.MethodHandle object.
class ciMethodHandle : public ciInstance {
private:
  ciMethod* _callee;

  // Return an adapter for this MethodHandle.
  ciMethod* get_adapter(bool is_invokedynamic) const;

protected:
  void print_impl(outputStream* st);

public:
  ciMethodHandle(instanceHandle h_i) : ciInstance(h_i) {};

  // What kind of ciObject is this?
  bool is_method_handle() const { return true; }

  ciMethod* callee() const { return _callee; }
  void  set_callee(ciMethod* m) { _callee = m; }

  // Return an adapter for a MethodHandle call.
  ciMethod* get_method_handle_adapter() const {
    return get_adapter(false);
  }

  // Return an adapter for an invokedynamic call.
  ciMethod* get_invokedynamic_adapter() const {
    return get_adapter(true);
  }
};
