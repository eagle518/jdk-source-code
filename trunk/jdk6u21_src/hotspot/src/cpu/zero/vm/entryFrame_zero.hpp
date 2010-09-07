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
// | parameter n-1      |       low addresses
// |  ...               |
// | parameter 0        |
// | call_wrapper       |
// | frame_type         |
// | next_frame         |      high addresses
// +--------------------+  ------------------
// |  ...               |

class EntryFrame : public ZeroFrame {
 private:
  EntryFrame() : ZeroFrame() {
    ShouldNotCallThis();
  }

 protected:
  enum Layout {
    call_wrapper_off = jf_header_words,
    header_words
  };

 public:
  static EntryFrame *build(ZeroStack*       stack,
                           const intptr_t*  parameters,
                           int              parameter_words,
                           JavaCallWrapper* call_wrapper);
 public:
  JavaCallWrapper *call_wrapper() const {
    return (JavaCallWrapper *) value_of_word(call_wrapper_off);
  }

 public:
  void identify_word(int   frame_index,
                     int   offset,
                     char* fieldbuf,
                     char* valuebuf,
                     int   buflen) const;
};
