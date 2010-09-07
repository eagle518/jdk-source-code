/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_threadLS_win32_ia64.cpp.incl"

void ThreadLocalStorage::generate_code_for_get_thread() { return; }

void ThreadLocalStorage::pd_init()                      { return; }
void ThreadLocalStorage::pd_set_thread(Thread* thread)  {
  os::thread_local_storage_at_put(ThreadLocalStorage::thread_index(), thread);
}
