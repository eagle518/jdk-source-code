#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)allocation.inline.hpp	1.15 03/12/23 16:40:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Explicit C-heap memory management

void trace_heap_malloc(size_t size, const char* name, void *p);
void trace_heap_free(void *p);
void vm_exit_out_of_memory(size_t size, const char* name);


// allocate using malloc; will fail if no memory available
inline char* AllocateHeap(size_t size, const char* name = NULL) {
  char* p = (char*) os::malloc(size);
  #ifdef ASSERT
  if (PrintMallocFree) trace_heap_malloc(size, name, p);
  #else
  Unused_Variable(name);
  #endif
  if (p == NULL) vm_exit_out_of_memory(size, name);
  return p;
}

inline char* ReallocateHeap(char *old, size_t size, const char* name = NULL) {
  char* p = (char*) os::realloc(old,size);
  #ifdef ASSERT
  if (PrintMallocFree) trace_heap_malloc(size, name, p);
  #else
  Unused_Variable(name);
  #endif
  if (p == NULL) vm_exit_out_of_memory(size, name);
  return p;
}

inline void FreeHeap(void* p) {
  #ifdef ASSERT
  if (PrintMallocFree) trace_heap_free(p);
  #endif
  os::free(p);
}
