#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)handles.inline.hpp	1.22 03/12/23 16:43:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// these inline functions are in a separate file to break an include cycle
// between Thread and Handle

inline Handle::Handle(oop obj) { 
  if (obj == NULL) {
    _handle = NULL;
  } else {
    _handle = Thread::current()->handle_area()->allocate_handle(obj);     
  }
}


#ifndef ASSERT
inline Handle::Handle(Thread* thread, oop obj) {
  assert(thread == Thread::current(), "sanity check");
  if (obj == NULL) {
    _handle = NULL;
  } else {
    _handle = thread->handle_area()->allocate_handle(obj);    
  }
}
#endif // ASSERT


inline HandleMark::HandleMark() {
  initialize(Thread::current());
}


inline void HandleMark::push() {
  // This is intentionally a NOP. pop_and_restore will reset
  // values to the HandleMark further down the stack, typically
  // in JavaCalls::call_helper.
  debug_only(_area->_handle_mark_nesting++);
}

inline void HandleMark::pop_and_restore() {
  HandleArea* area = _area;   // help compilers with poor alias analysis
  // Delete later chunks
  if( _chunk->next() ) {
    _chunk->next_chop();
  }
  // Roll back arena to saved top markers
  area->_chunk = _chunk;
  area->_hwm = _hwm;
  area->_max = _max;
  NOT_PRODUCT(area->set_size_in_bytes(_size_in_bytes);)
  debug_only(area->_handle_mark_nesting--);
}
