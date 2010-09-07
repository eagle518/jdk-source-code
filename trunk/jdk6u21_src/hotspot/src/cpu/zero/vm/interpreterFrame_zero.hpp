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

#ifdef CC_INTERP
// |  ...               |
// +--------------------+  ------------------
// | stack slot n-1     |       low addresses
// |  ...               |
// | stack slot 0       |
// | monitor 0 (maybe)  |
// |  ...               |
// | interpreter state  |
// |  ...               |
// | frame_type         |
// | next_frame         |      high addresses
// +--------------------+  ------------------
// |  ...               |

class InterpreterFrame : public ZeroFrame {
  friend class AbstractInterpreter;

 private:
  InterpreterFrame() : ZeroFrame() {
    ShouldNotCallThis();
  }

 protected:
  enum Layout {
    istate_off = jf_header_words +
      (align_size_up_(sizeof(BytecodeInterpreter),
                      wordSize) >> LogBytesPerWord) - 1,
    header_words
  };

 public:
  static InterpreterFrame *build(ZeroStack*      stack,
                                 const methodOop method,
                                 JavaThread*     thread);
  static InterpreterFrame *build(ZeroStack* stack, int size);

 public:
  interpreterState interpreter_state() const {
    return (interpreterState) addr_of_word(istate_off);
  }

 public:
  void identify_word(int   frame_index,
                     int   offset,
                     char* fieldbuf,
                     char* valuebuf,
                     int   buflen) const;
};
#endif // CC_INTERP
