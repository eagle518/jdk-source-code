/*
 * Copyright (c) 1998, 2000, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_osThread_windows.cpp.incl"

void OSThread::pd_initialize() {
  set_thread_handle(NULL);
  set_thread_id(NULL);
  set_interrupt_event(NULL);
}

// TODO: this is not well encapsulated; creation and deletion of the
// interrupt_event are done in os_win32.cpp, create_thread and
// free_thread. Should follow pattern of Linux/Solaris code here.
void OSThread::pd_destroy() {
}
