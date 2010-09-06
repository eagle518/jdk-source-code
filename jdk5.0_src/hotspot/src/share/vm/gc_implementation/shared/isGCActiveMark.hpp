#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)isGCActiveMark.hpp	1.3 03/12/23 16:40:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This class provides a method for block structured setting of the
// _is_gc_active state without requiring accessors in CollectedHeap

class IsGCActiveMark : public StackObj {
 public:
  IsGCActiveMark() {
    CollectedHeap* heap = Universe::heap();
    assert(!heap->is_gc_active(), "Not reentrant");
    heap->_is_gc_active = true;
  }

  ~IsGCActiveMark() {
    CollectedHeap* heap = Universe::heap();
    assert(heap->is_gc_active(), "Sanity");
    heap->_is_gc_active = false;
  }
};
