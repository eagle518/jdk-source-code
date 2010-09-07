/*
 * Copyright (c) 1999, 2004, Oracle and/or its affiliates. All rights reserved.
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

// do not include  precompiled  header file
# include "incls/_osThread_linux.cpp.incl"


void OSThread::pd_initialize() {
  assert(this != NULL, "check");
  _thread_id        = 0;
  _pthread_id       = 0;
  _siginfo = NULL;
  _ucontext = NULL;
  _expanding_stack = 0;
  _alt_sig_stack = NULL;

  sigemptyset(&_caller_sigmask);

  _startThread_lock = new Monitor(Mutex::event, "startThread_lock", true);
  assert(_startThread_lock !=NULL, "check");
}

void OSThread::pd_destroy() {
  delete _startThread_lock;
}
