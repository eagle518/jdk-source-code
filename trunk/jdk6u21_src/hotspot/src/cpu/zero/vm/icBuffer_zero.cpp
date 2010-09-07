/*
 * Copyright (c) 2003, 2006, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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

#include "incls/_precompiled.incl"
#include "incls/_icBuffer_zero.cpp.incl"

int InlineCacheBuffer::ic_stub_code_size() {
  // NB set this once the functions below are implemented
  return 4;
}

void InlineCacheBuffer::assemble_ic_buffer_code(address code_begin,
                                                oop cached_oop,
                                                address entry_point) {
  // NB ic_stub_code_size() must return the size of the code we generate
  ShouldNotCallThis();
}

address InlineCacheBuffer::ic_buffer_entry_point(address code_begin) {
  // NB ic_stub_code_size() must return the size of the code we generate
  ShouldNotCallThis();
}

oop InlineCacheBuffer::ic_buffer_cached_oop(address code_begin) {
  // NB ic_stub_code_size() must return the size of the code we generate
  ShouldNotCallThis();
}
