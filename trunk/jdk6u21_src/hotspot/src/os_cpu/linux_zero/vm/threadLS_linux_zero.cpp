/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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
#include "incls/_threadLS_linux_zero.cpp.incl"

void ThreadLocalStorage::generate_code_for_get_thread() {
  // nothing to do
}

void ThreadLocalStorage::pd_init() {
  // nothing to do
}

void ThreadLocalStorage::pd_set_thread(Thread* thread) {
  os::thread_local_storage_at_put(ThreadLocalStorage::thread_index(), thread);
}
