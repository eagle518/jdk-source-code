#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)oopRecorder.cpp	1.14 03/12/23 16:39:56 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_oopRecorder.cpp.incl"

OopRecorder::OopRecorder(Arena* arena) {
  _handles_arena  = arena;
  _handles_size   = 0;
  _handles        = NULL;
  _handles_length = 0;

  _complete       = false;
}


int OopRecorder::oop_size() {
  _complete = true;
  return _handles_length * sizeof(oop);
}


void OopRecorder::copy_to(CodeBlob* code) {
#ifdef CORE
  ShouldNotReachHere();
#else
  assert(_complete, "must be frozen");
  code->copy_oops(_handles, _handles_length);
#endif
}


int OopRecorder::allocate_index(jobject h) {
  assert(!_complete, "cannot allocate more elements after size query");
  if (_handles_arena == NULL)  _nesting.check();

  if (_handles_size == 0) {
    _handles_size = 100;
    if (_handles_arena == NULL)
      _handles = NEW_RESOURCE_ARRAY(jobject, _handles_size);
    else
      _handles = (jobject*) _handles_arena->Amalloc(_handles_size * sizeof(jobject));
  } else if (_handles_length == _handles_size) {
    // Expand
    int     new_handles_size = _handles_size * 2;
    if (_handles_arena == NULL)
      _handles = REALLOC_RESOURCE_ARRAY(jobject, _handles, _handles_size, new_handles_size);
    else
      _handles = (jobject*)_handles_arena->Arealloc(_handles, _handles_size * sizeof(jobject), new_handles_size * sizeof(jobject));
    _handles_size = new_handles_size;
  }

  assert(_handles_size > _handles_length, "There must be room for after expanding");
  _handles[_handles_length++] = h;
  // indexing uses 1 as an origin--0 means null
  return _handles_length;
}


int OopRecorder::find_index(jobject h) {
  assert(!_complete, "cannot allocate more elements after size query");

  if (h == NULL)  return 0;

  const int old_oops_to_check = 10;
  for (int i = MAX2(0, _handles_length - old_oops_to_check); i < _handles_length; i++) {
    if (_handles[i] == h)  return i+1;  // indexing uses 1 as an origin--0 means null
  }

  return allocate_index(h);
}
