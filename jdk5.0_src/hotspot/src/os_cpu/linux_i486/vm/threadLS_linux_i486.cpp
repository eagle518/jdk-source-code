#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_linux_i486.cpp	1.7 03/12/23 16:38:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_threadLS_linux_i486.cpp.incl"

// Map stack pointer (%esp) to thread pointer for faster TLS access
//
// Here we use a flat table for better performance. Getting current thread
// is down to one memory access (read _sp_map[%esp>>12]) in generated code 
// and two in runtime code (-fPIC code needs an extra load for _sp_map).
//
// This code assumes stack page is not shared by different threads. It works
// in 32-bit VM when page size is 4K (or a multiple of 4K, if that matters).
//
// Notice that _sp_map is allocated in the bss segment, which is ZFOD 
// (zero-fill-on-demand). While it reserves 4M address space upfront,
// actual memory pages are committed on demand.
//
// If an application creates and destroys a lot of threads, usually the
// stack space freed by a thread will soon get reused by new thread
// (this is especially true in NPTL or LinuxThreads in fixed-stack mode).
// No memory page in _sp_map is wasted.
//
// However, it's still possible that we might end up populating &
// committing a large fraction of the 4M table over time, but the actual 
// amount of live data in the table could be quite small. The max wastage
// is less than 4M bytes. If it becomes an issue, we could use madvise()
// with MADV_DONTNEED to reclaim unused (i.e. all-zero) pages in _sp_map.
// MADV_DONTNEED on Linux keeps the virtual memory mapping, but zaps the 
// physical memory page (i.e. similar to MADV_FREE on Solaris).

Thread* ThreadLocalStorage::_sp_map[1UL << (SP_BITLENGTH - PAGE_SHIFT)];

void ThreadLocalStorage::generate_code_for_get_thread() {
    // nothing we can do here for user-level thread
}

void ThreadLocalStorage::pd_init() {
  assert(align_size_down(os::vm_page_size(), PAGE_SIZE) == os::vm_page_size(),
         "page size must be multiple of PAGE_SIZE");
}

void ThreadLocalStorage::pd_set_thread(Thread* thread) {
  os::thread_local_storage_at_put(ThreadLocalStorage::thread_index(), thread);

  address stack_top = os::current_stack_base();
  size_t stack_size = os::current_stack_size();

  for (address p = stack_top - stack_size; p < stack_top; p += PAGE_SIZE) {
    // pd_set_thread() is called with non-NULL value when a new thread is 
    // created/attached, or with NULL value when a thread is about to exit.
    // If both "thread" and the corresponding _sp_map[] entry are non-NULL,
    // they should have the same value. Otherwise it might indicate that the 
    // stack page is shared by multiple threads. However, a more likely cause 
    // for this assertion to fail is that an attached thread exited without 
    // detaching itself from VM, which is a program error and could cause VM 
    // to crash.
    assert(thread == NULL || _sp_map[(uintptr_t)p >> PAGE_SHIFT] == NULL ||
           thread == _sp_map[(uintptr_t)p >> PAGE_SHIFT], 
           "thread exited without detaching from VM??");
    _sp_map[(uintptr_t)p >> PAGE_SHIFT] = thread;
  }
}

