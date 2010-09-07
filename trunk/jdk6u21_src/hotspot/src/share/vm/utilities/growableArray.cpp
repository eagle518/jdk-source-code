/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
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
# include "incls/_precompiled.incl"
# include "incls/_growableArray.cpp.incl"

#ifdef ASSERT
void GenericGrowableArray::set_nesting() {
  if (on_stack()) {
    _nesting = Thread::current()->resource_area()->nesting();
  }
}

void GenericGrowableArray::check_nesting() {
  // Check for insidious allocation bug: if a GrowableArray overflows, the
  // grown array must be allocated under the same ResourceMark as the original.
  // Otherwise, the _data array will be deallocated too early.
  if (on_stack() &&
      _nesting != Thread::current()->resource_area()->nesting()) {
    fatal("allocation bug: GrowableArray could grow within nested ResourceMark");
  }
}
#endif

void* GenericGrowableArray::raw_allocate(int elementSize) {
  assert(_max >= 0, "integer overflow");
  size_t byte_size = elementSize * (size_t) _max;
  if (on_stack()) {
    return (void*)resource_allocate_bytes(byte_size);
  } else if (on_C_heap()) {
    return (void*)AllocateHeap(byte_size, "GrET in " __FILE__);
  } else {
    return _arena->Amalloc(byte_size);
  }
}
