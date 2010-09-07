/*
 * Copyright (c) 2001, 2008, Oracle and/or its affiliates. All rights reserved.
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

// Methods of protected closure types

template <class T>
inline void DefNewGeneration::KeepAliveClosure::do_oop_work(T* p) {
#ifdef ASSERT
  {
    // We never expect to see a null reference being processed
    // as a weak reference.
    assert (!oopDesc::is_null(*p), "expected non-null ref");
    oop obj = oopDesc::load_decode_heap_oop_not_null(p);
    assert (obj->is_oop(), "expected an oop while scanning weak refs");
  }
#endif // ASSERT

  _cl->do_oop_nv(p);

  // Card marking is trickier for weak refs.
  // This oop is a 'next' field which was filled in while we
  // were discovering weak references. While we might not need
  // to take a special action to keep this reference alive, we
  // will need to dirty a card as the field was modified.
  //
  // Alternatively, we could create a method which iterates through
  // each generation, allowing them in turn to examine the modified
  // field.
  //
  // We could check that p is also in an older generation, but
  // dirty cards in the youngest gen are never scanned, so the
  // extra check probably isn't worthwhile.
  if (Universe::heap()->is_in_reserved(p)) {
    oop obj = oopDesc::load_decode_heap_oop_not_null(p);
    _rs->inline_write_ref_field_gc(p, obj);
  }
}

template <class T>
inline void DefNewGeneration::FastKeepAliveClosure::do_oop_work(T* p) {
#ifdef ASSERT
  {
    // We never expect to see a null reference being processed
    // as a weak reference.
    assert (!oopDesc::is_null(*p), "expected non-null ref");
    oop obj = oopDesc::load_decode_heap_oop_not_null(p);
    assert (obj->is_oop(), "expected an oop while scanning weak refs");
  }
#endif // ASSERT

  _cl->do_oop_nv(p);

  // Optimized for Defnew generation if it's the youngest generation:
  // we set a younger_gen card if we have an older->youngest
  // generation pointer.
  oop obj = oopDesc::load_decode_heap_oop_not_null(p);
  if (((HeapWord*)obj < _boundary) && Universe::heap()->is_in_reserved(p)) {
    _rs->inline_write_ref_field_gc(p, obj);
  }
}
