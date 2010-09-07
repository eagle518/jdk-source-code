/*
 * Copyright (c) 1999, 2001, Oracle and/or its affiliates. All rights reserved.
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

// ciArray
//
// This class represents an arrayOop in the HotSpot virtual
// machine.
class ciArray : public ciObject {
private:
  int _length;

protected:
  ciArray(    arrayHandle h_a) : ciObject(h_a), _length(h_a()->length()) {}
  ciArray( objArrayHandle h_a) : ciObject(h_a), _length(h_a()->length()) {}
  ciArray(typeArrayHandle h_a) : ciObject(h_a), _length(h_a()->length()) {}

  ciArray(ciKlass* klass, int len) : ciObject(klass), _length(len) {}

  arrayOop get_arrayOop() { return (arrayOop)get_oop(); }

  const char* type_string() { return "ciArray"; }

  void print_impl(outputStream* st);

public:
  int length() { return _length; }

  // What kind of ciObject is this?
  bool is_array()        { return true; }
  bool is_java_object()  { return true; }
};
