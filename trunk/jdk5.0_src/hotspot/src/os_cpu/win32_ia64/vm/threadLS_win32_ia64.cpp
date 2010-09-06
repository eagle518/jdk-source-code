#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_win32_ia64.cpp	1.6 03/12/23 16:38:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_threadLS_win32_ia64.cpp.incl"

void ThreadLocalStorage::generate_code_for_get_thread() { return; }

void ThreadLocalStorage::pd_init()                      { return; }
void ThreadLocalStorage::pd_set_thread(Thread* thread)  { 
  os::thread_local_storage_at_put(ThreadLocalStorage::thread_index(), thread);
}
