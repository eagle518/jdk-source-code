/*
 * Copyright (c) 2001, 2006, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_threadCritical_linux.cpp.incl"

// put OS-includes here
# include <pthread.h>

//
// See threadCritical.hpp for details of this class.
//

static pthread_t             tc_owner = 0;
static pthread_mutex_t       tc_mutex = PTHREAD_MUTEX_INITIALIZER;
static int                   tc_count = 0;

void ThreadCritical::initialize() {
}

void ThreadCritical::release() {
}

ThreadCritical::ThreadCritical() {
  pthread_t self = pthread_self();
  if (self != tc_owner) {
    int ret = pthread_mutex_lock(&tc_mutex);
    guarantee(ret == 0, "fatal error with pthread_mutex_lock()");
    assert(tc_count == 0, "Lock acquired with illegal reentry count.");
    tc_owner = self;
  }
  tc_count++;
}

ThreadCritical::~ThreadCritical() {
  assert(tc_owner == pthread_self(), "must have correct owner");
  assert(tc_count > 0, "must have correct count");

  tc_count--;
  if (tc_count == 0) {
    tc_owner = 0;
    int ret = pthread_mutex_unlock(&tc_mutex);
    guarantee(ret == 0, "fatal error with pthread_mutex_unlock()");
  }
}
