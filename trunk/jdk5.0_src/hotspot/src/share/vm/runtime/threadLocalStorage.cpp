#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLocalStorage.cpp	1.40 03/12/23 16:44:22 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_threadLocalStorage.cpp.incl"

// static member initialization
int ThreadLocalStorage::_thread_index = -1;

Thread* ThreadLocalStorage::get_thread_slow() {
  return (Thread*) os::thread_local_storage_at(ThreadLocalStorage::thread_index());
}

void ThreadLocalStorage::set_thread(Thread* thread) {
  pd_set_thread(thread);

  // The following ensure that any optimization tricks we have tried
  // did not backfire on us:
  guarantee(get_thread()      == thread, "must be the same thread, quickly");
  guarantee(get_thread_slow() == thread, "must be the same thread, slowly");
}

void ThreadLocalStorage::init() {
  assert(ThreadLocalStorage::thread_index() == -1, "More than one attempt to initialize threadLocalStorage");
  pd_init();
  set_thread_index(os::allocate_thread_local_storage());
  generate_code_for_get_thread();
}

