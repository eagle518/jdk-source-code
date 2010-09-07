/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2008, 2009 Red Hat, Inc.
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
// | stack slot n-1     |       low addresses
// |  ...               |
// | stack slot 0       |
// | monitor m-1        |
// |  ...               |
// | monitor 0          |
// | oop_tmp            |
// | method             |
// | unextended_sp      |
// | pc                 |
// | frame_type         |
// | next_frame         |      high addresses
// +--------------------+  ------------------
// |  ...               |

class SharkFrame : public ZeroFrame {
  friend class SharkStack;

 private:
  SharkFrame() : ZeroFrame() {
    ShouldNotCallThis();
  }

 protected:
  enum Layout {
    pc_off = jf_header_words,
    unextended_sp_off,
    method_off,
    oop_tmp_off,
    header_words
  };

 public:
  address pc() const {
    return (address) value_of_word(pc_off);
  }

  intptr_t* unextended_sp() const {
    return (intptr_t *) value_of_word(unextended_sp_off);
  }

  methodOop method() const {
    return (methodOop) value_of_word(method_off);
  }

 public:
  void identify_word(int   frame_index,
                     int   offset,
                     char* fieldbuf,
                     char* valuebuf,
                     int   buflen) const;
};
