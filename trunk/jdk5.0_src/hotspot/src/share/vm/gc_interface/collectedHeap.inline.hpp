#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)collectedHeap.inline.hpp	1.19 04/02/24 20:13:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Inline allocation implementations.

void CollectedHeap::post_allocation_setup_common(KlassHandle klass,
                                                 HeapWord* objPtr,
                                                 size_t size) {
  // These asserts are kind of complicated because of klassKlass
  // and the beginning of the world.
  assert(klass() != NULL || !Universe::is_fully_initialized(), "NULL klass");
  assert(klass() == NULL || klass()->is_klass(), "not a klass");
  assert(objPtr != NULL, "NULL object pointer");
  oop obj = (oop)objPtr;
  obj->set_mark();
  obj->set_klass(klass());
  if (Universe::jvmpi_alloc_event_enabled()) {
    Universe::jvmpi_object_alloc((oop)obj, size * wordSize /* no. of bytes */);
  }
 
  // support for JVMTI VMObjectAlloc event (no-op if not enabled) 
  JvmtiExport::vm_object_alloc_event_collector(obj);

  // support low memory notifications (no-op if not enabled)
  LowMemoryDetector::detect_low_memory_for_collected_pools();
}

void CollectedHeap::post_allocation_setup_obj(KlassHandle klass,
                                              HeapWord* obj,
                                              size_t size) {
  post_allocation_setup_common(klass, obj, size);
  assert(Universe::is_bootstrapping() ||
         !((oop)obj)->is_array(), "must not be an array");
} 

void CollectedHeap::post_allocation_setup_array(KlassHandle klass,
                                                HeapWord* obj,
                                                size_t size,
                                                int length) {
  // Set array length before posting jvmpi object alloc event 
  // in post_allocation_setup_common()
  assert(length >= 0, "length should be non-negative");
  ((arrayOop)obj)->set_length(length);
  post_allocation_setup_common(klass, obj, size);
  assert(((oop)obj)->is_array(), "must be an array");
}

HeapWord* CollectedHeap::common_mem_allocate_noinit(size_t size, bool is_noref, TRAPS) {
  // We may want to update this, is_noref objects might not be allocated in TLABs.
  HeapWord* result = NULL;
  if (UseTLAB && !Universe::jvmpi_slow_allocation()) {
    result = CollectedHeap::allocate_from_tlab(THREAD, size);
    if (result != NULL) {
      return result;
    }
  }
  result = Universe::heap()->mem_allocate(size, is_noref, false);
  if (result != NULL) {
    NOT_PRODUCT(Universe::heap()->
      check_for_non_bad_heap_word_value(result, size));
    return result;
  }
  THROW_OOP_0(Universe::out_of_memory_error_java_heap());
}

HeapWord* CollectedHeap::common_mem_allocate_init(size_t size, bool is_noref, TRAPS) {
  HeapWord* obj = common_mem_allocate_noinit(size, is_noref, CHECK_0);
  init_obj(obj, size);
  return obj;
}

// Need to investigate, do we really want to throw OOM exception here?
HeapWord* CollectedHeap::common_permanent_mem_allocate_noinit(size_t size, TRAPS) {
  HeapWord* result = Universe::heap()->permanent_mem_allocate(size);
  if (result != NULL) {
    NOT_PRODUCT(Universe::heap()->
      check_for_non_bad_heap_word_value(result, size));
    return result;
  }
  THROW_OOP_0(Universe::out_of_memory_error_perm_gen());
}

HeapWord* CollectedHeap::common_permanent_mem_allocate_init(size_t size, TRAPS) {
  HeapWord* obj = common_permanent_mem_allocate_noinit(size, CHECK_0);
  init_obj(obj, size);
  return obj;
}

HeapWord* CollectedHeap::allocate_from_tlab(Thread* thread, size_t size) {
  assert(UseTLAB, "should use UseTLAB");

  HeapWord* obj = thread->tlab().allocate(size);
  if (obj != NULL) {
    return obj;
  }
  // Otherwise...
  return allocate_from_tlab_slow(thread, size);
}

void CollectedHeap::init_obj(HeapWord* obj, size_t size) {
  assert(obj != NULL, "cannot initialize NULL object");
  const size_t hs = oopDesc::header_size();
  assert(size >= hs, "unexpected object size");
  Copy::fill_to_aligned_words(obj + hs, size - hs);
}

oop CollectedHeap::obj_allocate(KlassHandle klass, int size, TRAPS) {
  debug_only(check_for_valid_allocation_state());
  assert(!Universe::heap()->is_gc_active(), "Allocation during gc not allowed");
  assert(size >= 0, "int won't convert to size_t");
  HeapWord* obj = common_mem_allocate_init(size, false, CHECK_0);
  post_allocation_setup_obj(klass, obj, size);
  NOT_PRODUCT(Universe::heap()->check_for_bad_heap_word_value(obj, size));
  return (oop)obj;  
}

oop CollectedHeap::array_allocate(KlassHandle klass,
                                  int size,
                                  int length,
                                  TRAPS) {
  debug_only(check_for_valid_allocation_state());
  assert(!Universe::heap()->is_gc_active(), "Allocation during gc not allowed");
  assert(size >= 0, "int won't convert to size_t");
  HeapWord* obj = common_mem_allocate_init(size, false, CHECK_0);
  post_allocation_setup_array(klass, obj, size, length);
  NOT_PRODUCT(Universe::heap()->check_for_bad_heap_word_value(obj, size));
  return (oop)obj;  
}

oop CollectedHeap::large_typearray_allocate(KlassHandle klass,
                                            int size,
                                            int length,
                                            TRAPS) {
  debug_only(check_for_valid_allocation_state());
  assert(!Universe::heap()->is_gc_active(), "Allocation during gc not allowed");
  assert(size >= 0, "int won't convert to size_t");
  HeapWord* obj = common_mem_allocate_init(size, true, CHECK_0);
  post_allocation_setup_array(klass, obj, size, length);
  NOT_PRODUCT(Universe::heap()->check_for_bad_heap_word_value(obj, size));
  return (oop)obj;  
}

oop CollectedHeap::permanent_obj_allocate(KlassHandle klass, int size, TRAPS) {
  debug_only(check_for_valid_allocation_state());
  assert(!Universe::heap()->is_gc_active(), "Allocation during gc not allowed");
  assert(size >= 0, "int won't convert to size_t");
  HeapWord* obj = common_permanent_mem_allocate_init(size, CHECK_0);
  post_allocation_setup_obj(klass, obj, size);
  NOT_PRODUCT(Universe::heap()->check_for_bad_heap_word_value(obj, size));
  return (oop)obj;  
}

oop CollectedHeap::permanent_array_allocate(KlassHandle klass,
                                            int size,
                                            int length,
                                            TRAPS) {
  debug_only(check_for_valid_allocation_state());
  assert(!Universe::heap()->is_gc_active(), "Allocation during gc not allowed");
  assert(size >= 0, "int won't convert to size_t");
  HeapWord* obj = common_permanent_mem_allocate_init(size, CHECK_0);
  post_allocation_setup_array(klass, obj, size, length);
  NOT_PRODUCT(Universe::heap()->check_for_bad_heap_word_value(obj, size));
  return (oop)obj;  
}
