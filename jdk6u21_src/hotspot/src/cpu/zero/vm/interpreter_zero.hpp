/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008 Red Hat, Inc.
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

 public:
  static void invoke_method(methodOop method, address entry_point, TRAPS) {
    ((ZeroEntry *) entry_point)->invoke(method, THREAD);
  }
  static void invoke_osr(methodOop method,
                         address   entry_point,
                         address   osr_buf,
                         TRAPS) {
    ((ZeroEntry *) entry_point)->invoke_osr(method, osr_buf, THREAD);
  }

 public:
  static int expr_index_at(int i) {
    return stackElementWords() * i;
  }
  static int expr_tag_index_at(int i) {
    assert(TaggedStackInterpreter, "should not call this");
    Unimplemented();
  }

  static int expr_offset_in_bytes(int i) {
    return stackElementSize() * i;
  }
  static int expr_tag_offset_in_bytes(int i) {
    assert(TaggedStackInterpreter, "should not call this");
    Unimplemented();
  }

  static int local_index_at(int i) {
    assert(i <= 0, "local direction already negated");
    return stackElementWords() * i + (value_offset_in_bytes() / wordSize);
  }
  static int local_tag_index_at(int i) {
    assert(TaggedStackInterpreter, "should not call this");
    Unimplemented();
  }
