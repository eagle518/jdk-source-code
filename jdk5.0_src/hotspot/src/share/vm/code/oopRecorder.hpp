#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oopRecorder.hpp	1.14 03/12/23 16:39:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Recording and retrieval of oop relocations in compiled code.

class CodeBlob;

class OopRecorder : public ResourceObj {
 public:
  // A mapping from oops to positive indexes.
  // The zero index is reserved for a constant (sharable) null.
  // Indexes may not be negative.

  // Uses a caller-specified area (but not a resource area) to record its state.
  // By default, uses the current ResourceArea instead.
  OopRecorder(Arena* arena = NULL);

  // generate a new index (on which set_oop_at will work)
  // allocate_index and find_index never return the same index,
  // and allocate_index never returns the same index twice
  // In fact, two successive calls to allocate_index return successive ints.
  int allocate_index(jobject h);

  // this may or may not return the same index (for the same oop) repeatedly
  // the index can later be given to oop_at to retrieve the oop,
  // however, the oop must not be changed with set_oop_at
  int     find_index(jobject h);

  // returns the size of the generated oop table, for sizing the CodeBlob.
  // must be called after all oops are allocated!
  int oop_size();

	// retrieve the handle at a given index.
  jobject handle_at(int index) {
    assert(index < _handles_length, "index out of bounds");
    return _handles[index];
  }

  // copy the generated oop table to CodeBlob
  void copy_to(CodeBlob* code);

 private:
  // Array of JNI handles
  jobject*    _handles;
  int         _handles_size;
  int         _handles_length;

  Arena*      _handles_arena;
  ReallocMark _nesting;

  bool        _complete;
};
