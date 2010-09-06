#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_linux_amd64.cpp	1.2 03/12/23 16:38:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_threadLS_linux_amd64.cpp.incl"

void ThreadLocalStorage::generate_code_for_get_thread()
{
  // nothing we can do here for user-level thread
}

void ThreadLocalStorage::pd_init()
{
  // Nothing to do
}

void ThreadLocalStorage::pd_set_thread(Thread* thread)
{
  os::thread_local_storage_at_put(ThreadLocalStorage::thread_index(), thread);
}
