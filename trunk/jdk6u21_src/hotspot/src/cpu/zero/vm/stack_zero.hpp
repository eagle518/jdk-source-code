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

class ZeroStack {
 private:
  intptr_t *_base; // the last available word
  intptr_t *_top;  // the word past the end of the stack
  intptr_t *_sp;   // the top word on the stack

 public:
  ZeroStack()
    : _base(NULL), _top(NULL), _sp(NULL) {}

  bool needs_setup() const {
    return _base == NULL;
  }

  void setup(void *mem, size_t size) {
    assert(needs_setup(), "already set up");
    assert(!(size & WordAlignmentMask), "unaligned");

    _base = (intptr_t *) mem;
    _top  = _base + (size >> LogBytesPerWord);
    _sp   = _top;
  }
  void teardown() {
    assert(!needs_setup(), "not set up");
    assert(_sp == _top, "stuff on stack at teardown");

    _base = NULL;
    _top  = NULL;
    _sp   = NULL;
  }

  intptr_t *sp() const {
    return _sp;
  }
  void set_sp(intptr_t *new_sp) {
    assert(_top >= new_sp && new_sp >= _base, "bad stack pointer");
    _sp = new_sp;
  }

  int available_words() const {
    return _sp - _base;
  }

  void push(intptr_t value) {
    assert(_sp > _base, "stack overflow");
    *(--_sp) = value;
  }
  intptr_t pop() {
    assert(_sp < _top, "stack underflow");
    return *(_sp++);
  }

  void *alloc(size_t size) {
    int count = align_size_up(size, wordSize) >> LogBytesPerWord;
    assert(count <= available_words(), "stack overflow");
    return _sp -= count;
  }

 public:
  static ByteSize base_offset() {
    return byte_offset_of(ZeroStack, _base);
  }
  static ByteSize top_offset() {
    return byte_offset_of(ZeroStack, _top);
  }
  static ByteSize sp_offset() {
    return byte_offset_of(ZeroStack, _sp);
  }
};


class EntryFrame;
class InterpreterFrame;
class SharkFrame;
class FakeStubFrame;

//
// |  ...               |
// +--------------------+  ------------------
// |  ...               |       low addresses
// | frame_type         |
// | next_frame         |      high addresses
// +--------------------+  ------------------
// |  ...               |

class ZeroFrame {
  friend class frame;
  friend class ZeroStackPrinter;

 protected:
  ZeroFrame() {
    ShouldNotCallThis();
  }

  enum Layout {
    next_frame_off,
    frame_type_off,
    jf_header_words
  };

  enum FrameType {
    ENTRY_FRAME = 1,
    INTERPRETER_FRAME,
    SHARK_FRAME,
    FAKE_STUB_FRAME
  };

 protected:
  intptr_t *addr_of_word(int offset) const {
    return (intptr_t *) this - offset;
  }
  intptr_t value_of_word(int offset) const {
    return *addr_of_word(offset);
  }

 public:
  ZeroFrame *next() const {
    return (ZeroFrame *) value_of_word(next_frame_off);
  }

 protected:
  FrameType type() const {
    return (FrameType) value_of_word(frame_type_off);
  }

 public:
  bool is_entry_frame() const {
    return type() == ENTRY_FRAME;
  }
  bool is_interpreter_frame() const {
    return type() == INTERPRETER_FRAME;
  }
  bool is_shark_frame() const {
    return type() == SHARK_FRAME;
  }
  bool is_fake_stub_frame() const {
    return type() == FAKE_STUB_FRAME;
  }

 public:
  EntryFrame *as_entry_frame() const {
    assert(is_entry_frame(), "should be");
    return (EntryFrame *) this;
  }
  InterpreterFrame *as_interpreter_frame() const {
    assert(is_interpreter_frame(), "should be");
    return (InterpreterFrame *) this;
  }
  SharkFrame *as_shark_frame() const {
    assert(is_shark_frame(), "should be");
    return (SharkFrame *) this;
  }
  FakeStubFrame *as_fake_stub_frame() const {
    assert(is_fake_stub_frame(), "should be");
    return (FakeStubFrame *) this;
  }

 public:
  void identify_word(int   frame_index,
                     int   offset,
                     char* fieldbuf,
                     char* valuebuf,
                     int   buflen) const;

 protected:
  void identify_vp_word(int       frame_index,
                        intptr_t* addr,
                        intptr_t* monitor_base,
                        intptr_t* stack_base,
                        char*     fieldbuf,
                        int       buflen) const;
};
