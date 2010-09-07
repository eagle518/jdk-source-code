/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2008 Red Hat, Inc.
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

// |  ...               |
// +--------------------+  ------------------
// | frame_type         |       low addresses
// | next_frame         |      high addresses
// +--------------------+  ------------------
// |  ...               |

class FakeStubFrame : public ZeroFrame {
 private:
  FakeStubFrame() : ZeroFrame() {
    ShouldNotCallThis();
  }

 protected:
  enum Layout {
    header_words = jf_header_words
  };

 public:
  static FakeStubFrame *build(ZeroStack* stack);

 public:
  void identify_word(int   frame_index,
                     int   offset,
                     char* fieldbuf,
                     char* valuebuf,
                     int   buflen) const {}
};
