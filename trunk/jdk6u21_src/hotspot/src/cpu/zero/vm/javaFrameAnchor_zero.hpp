/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
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
  // Each arch must define reset, save, restore
  // These are used by objects that only care about:
  //  1 - initializing a new state (thread creation, javaCalls)
  //  2 - saving a current state (javaCalls)
  //  3 - restoring an old state (javaCalls)

  void clear() {
    // clearing _last_Java_sp must be first
    _last_Java_sp = NULL;
    // fence?
    _last_Java_pc = NULL;
  }

  void copy(JavaFrameAnchor* src) {
    // In order to make sure the transition state is valid for "this"
    // We must clear _last_Java_sp before copying the rest of the new
    // data
    //
    // Hack Alert: Temporary bugfix for 4717480/4721647 To act like
    // previous version (pd_cache_state) don't NULL _last_Java_sp
    // unless the value is changing
    //
    if (_last_Java_sp != src->_last_Java_sp)
      _last_Java_sp = NULL;

    _last_Java_pc = src->_last_Java_pc;
    // Must be last so profiler will always see valid frame if
    // has_last_frame() is true
    _last_Java_sp = src->_last_Java_sp;
  }

  bool walkable() {
    return true;
  }

  void make_walkable(JavaThread* thread) {
    // nothing to do
  }

  intptr_t* last_Java_sp() const {
    return _last_Java_sp;
  }

  void set_last_Java_sp(intptr_t* sp) {
    _last_Java_sp = sp;
  }
