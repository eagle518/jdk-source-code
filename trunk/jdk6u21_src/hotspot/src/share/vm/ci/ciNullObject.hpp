/*
 * Copyright (c) 1999, 2000, Oracle and/or its affiliates. All rights reserved.
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

// ciNullObject
//
// This class represents a null reference in the VM.
class ciNullObject : public ciObject {
  CI_PACKAGE_ACCESS

private:
  ciNullObject() : ciObject() {}

  const char* type_string() { return "ciNullObject"; }

  void print_impl(outputStream* st);

public:
  // Is this ciObject a Java Language Object?  That is,
  // is the ciObject an instance or an array
  bool is_java_object() { return true; }

  // What kind of ciObject is this?
  bool is_null_object() const { return true; }
  bool is_classless() const   { return true; }

  // Get the distinguished instance of this klass.
  static ciNullObject* make();
};
