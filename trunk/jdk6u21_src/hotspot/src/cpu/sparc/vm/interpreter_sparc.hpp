/*
 * Copyright (c) 1997, 2007, Oracle and/or its affiliates. All rights reserved.
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

  // Support for Tagged Stacks

  // Stack index relative to tos (which points at value)
  static int expr_index_at(int i)     {
    return stackElementWords() * i;
  }

  static int expr_tag_index_at(int i) {
    assert(TaggedStackInterpreter, "should not call this");
    // tag is one word above java stack element
    return stackElementWords() * i + 1;
  }

  static int expr_offset_in_bytes(int i) { return stackElementSize()*i + wordSize; }
  static int expr_tag_offset_in_bytes (int i) {
    assert(TaggedStackInterpreter, "should not call this");
    return expr_offset_in_bytes(i) + wordSize;
  }

  // Already negated by c++ interpreter
  static int local_index_at(int i)     {
    assert(i<=0, "local direction already negated");
    return stackElementWords() * i + (value_offset_in_bytes()/wordSize);
  }

  static int local_tag_index_at(int i) {
    assert(i<=0, "local direction already negated");
    assert(TaggedStackInterpreter, "should not call this");
    return stackElementWords() * i + (tag_offset_in_bytes()/wordSize);
  }
