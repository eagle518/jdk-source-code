#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadCritical_solaris.cpp	1.9 03/12/23 16:37:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_threadCritical_solaris.cpp.incl"

// OS-includes here
#include <thread.h>
#include <synch.h>

//
// See threadCritical.hpp for details of this class.
//
// For some reason, we don't do locking until the
// os::init() call completes. I'm not sure why this
// is, and have left it that way for now. This should
// be reviewed later.

static  mutex_t  global_mut;
static  thread_t global_mut_owner = -1;
static  int      global_mut_count = 0;
static  bool     initialized = false;

ThreadCritical::ThreadCritical() {
  if (initialized) {
    thread_t owner = thr_self();
    if (global_mut_owner != owner) {
      if (os::Solaris::mutex_lock(&global_mut))
        fatal1("ThreadCritical::ThreadCritical: mutex_lock failed (%s)", strerror(errno));
      assert(global_mut_count == 0, "must have clean count");
      assert(global_mut_owner == -1, "must have clean owner");
    }
    global_mut_owner = owner;
    ++global_mut_count;
  } else {
    assert (Threads::number_of_threads() == 0, "valid only during initialization");
  }
}

ThreadCritical::~ThreadCritical() {
  if (initialized) {
    assert(global_mut_owner == thr_self(), "must have correct owner");
    assert(global_mut_count > 0, "must have correct count");
    --global_mut_count;
    if (global_mut_count == 0) {
      global_mut_owner = -1;
      if (os::Solaris::mutex_unlock(&global_mut))
        fatal1("ThreadCritical::~ThreadCritical: mutex_unlock failed (%s)", strerror(errno));
    }
  } else {
    assert (Threads::number_of_threads() == 0, "valid only during initialization");
  }
}

void ThreadCritical::initialize() { 
  // This method is called at the end of os::init(). Until
  // then, we don't do real locking.
  initialized = true;
}

void ThreadCritical::release() {
}
