/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009 Red Hat, Inc.
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

// A frame represents a physical stack frame on the Zero stack.

 public:
  enum {
    pc_return_offset = 0
  };

  // Constructor
 public:
  frame(intptr_t* sp, intptr_t* fp);

  // The sp of a Zero frame is the address of the highest word in
  // that frame.  We keep track of the lowest address too, so the
  // boundaries of the frame are available for debug printing.
 private:
  intptr_t* _fp;

 public:
  intptr_t* fp() const {
    return _fp;
  }

#ifdef CC_INTERP
  inline interpreterState get_interpreterState() const;
#endif // CC_INTERP

 public:
  const ZeroFrame *zeroframe() const {
    return (ZeroFrame *) sp();
  }

  const EntryFrame *zero_entryframe() const {
    return zeroframe()->as_entry_frame();
  }
  const InterpreterFrame *zero_interpreterframe() const {
    return zeroframe()->as_interpreter_frame();
  }
  const SharkFrame *zero_sharkframe() const {
    return zeroframe()->as_shark_frame();
  }

 public:
  frame sender_for_nonentry_frame(RegisterMap* map) const;

 public:
  void zero_print_on_error(int           index,
                           outputStream* st,
                           char*         buf,
                           int           buflen) const;
