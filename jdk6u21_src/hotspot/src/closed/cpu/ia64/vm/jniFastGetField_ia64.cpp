/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jniFastGetField_ia64.cpp.incl"

// Fast versions not implemented yet. When implemented, be careful about
// LD-LD barriers and separate, terminated, bundles.

address JNI_FastGetField::generate_fast_get_boolean_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_byte_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_char_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_short_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_int_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_long_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_float_field() {
  return (address)-1;
}

address JNI_FastGetField::generate_fast_get_double_field() {
  return (address)-1;
}
