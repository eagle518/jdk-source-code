#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oop.inline.hpp	1.122 03/12/23 16:42:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved. 
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms. 
 */

// Implementation of all inlined member functions defined in oop.hpp
// We need a separate file to avoid circular references


inline void oopDesc::release_set_mark(markOop m) {
  OrderAccess::release_store_ptr(&_mark, m);
}

inline markOop oopDesc::cas_set_mark(markOop new_mark, markOop old_mark) {
  return (markOop) Atomic::cmpxchg_ptr(new_mark, &_mark, old_mark);
}

inline void oopDesc::set_klass(klassOop k) {
  // since klasses are promoted no store check is needed
  assert(Universe::is_bootstrapping() || k != NULL, "must be a real klassOop");
  assert(Universe::is_bootstrapping() || k->is_klass(), "not a klassOop");
  oop_store_without_check((oop*) &_klass, (oop) k);
}

inline void oopDesc::set_klass_to_list_ptr(oop k) {
  // This is only to be used during GC, for from-space objects, so no
  // barrier is needed.
  _klass = (klassOop)k;
}

inline void   oopDesc::init_mark()                 { set_mark(markOopDesc::prototype()); }
inline Klass* oopDesc::blueprint()           const { return klass()->klass_part(); }

inline bool oopDesc::is_a(klassOop k)        const { return blueprint()->is_subtype_of(k); }

inline bool oopDesc::is_instance()           const { return blueprint()->oop_is_instance(); }
inline bool oopDesc::is_instanceRef()        const { return blueprint()->oop_is_instanceRef(); }
inline bool oopDesc::is_array()              const { return blueprint()->oop_is_array(); }
inline bool oopDesc::is_objArray()           const { return blueprint()->oop_is_objArray(); }
inline bool oopDesc::is_objArray_fast()	     const { return blueprint()->oop_is_objArray_fast(); }
inline bool oopDesc::is_typeArray()          const { return blueprint()->oop_is_typeArray(); }
inline bool oopDesc::is_symbol()             const { return blueprint()->oop_is_symbol(); }
inline bool oopDesc::is_klass()              const { return blueprint()->oop_is_klass(); }
inline bool oopDesc::is_thread()             const { return blueprint()->oop_is_thread(); }
inline bool oopDesc::is_method()             const { return blueprint()->oop_is_method(); }
inline bool oopDesc::is_constMethod()	     const { return blueprint()->oop_is_constMethod(); }
#ifndef CORE
inline bool oopDesc::is_methodData()         const { return blueprint()->oop_is_methodData(); }
#endif // !CORE
inline bool oopDesc::is_constantPool()       const { return blueprint()->oop_is_constantPool(); }
inline bool oopDesc::is_constantPoolCache()  const { return blueprint()->oop_is_constantPoolCache(); }
inline bool oopDesc::is_compiledICHolder()   const { return blueprint()->oop_is_compiledICHolder(); }

inline void*     oopDesc::field_base(int offset)        const { return (void*)&((char*)this)[offset]; }

inline oop*      oopDesc::obj_field_addr(int offset)    const { return (oop*)     field_base(offset); }
inline jbyte*    oopDesc::byte_field_addr(int offset)   const { return (jbyte*)   field_base(offset); }
inline jchar*    oopDesc::char_field_addr(int offset)   const { return (jchar*)   field_base(offset); }
inline jboolean* oopDesc::bool_field_addr(int offset)   const { return (jboolean*)field_base(offset); }
inline jint*     oopDesc::int_field_addr(int offset)    const { return (jint*)    field_base(offset); }
inline jshort*   oopDesc::short_field_addr(int offset)  const { return (jshort*)  field_base(offset); }
inline jlong*    oopDesc::long_field_addr(int offset)   const { return (jlong*)   field_base(offset); }
inline jfloat*   oopDesc::float_field_addr(int offset)  const { return (jfloat*)  field_base(offset); }
inline jdouble*  oopDesc::double_field_addr(int offset) const { return (jdouble*) field_base(offset); }

inline oop oopDesc::obj_field(int offset) const                     { return *obj_field_addr(offset);             }
inline void oopDesc::obj_field_put(int offset, oop value)           { oop_store(obj_field_addr(offset), value);   }

inline jbyte oopDesc::byte_field(int offset) const                  { return (jbyte) *byte_field_addr(offset);    }
inline void oopDesc::byte_field_put(int offset, jbyte contents)     { *byte_field_addr(offset) = (jint) contents; }

inline jboolean oopDesc::bool_field(int offset) const               { return (jboolean) *bool_field_addr(offset); }
inline void oopDesc::bool_field_put(int offset, jboolean contents)  { *bool_field_addr(offset) = (jint) contents; }

inline jchar oopDesc::char_field(int offset) const                  { return (jchar) *char_field_addr(offset);    }
inline void oopDesc::char_field_put(int offset, jchar contents)     { *char_field_addr(offset) = (jint) contents; }

inline jint oopDesc::int_field(int offset) const                    { return *int_field_addr(offset);        }
inline void oopDesc::int_field_put(int offset, jint contents)       { *int_field_addr(offset) = contents;    }

inline jshort oopDesc::short_field(int offset) const                { return (jshort) *short_field_addr(offset);  }
inline void oopDesc::short_field_put(int offset, jshort contents)   { *short_field_addr(offset) = (jint) contents;}

inline jlong oopDesc::long_field(int offset) const                  { return *long_field_addr(offset);       }
inline void oopDesc::long_field_put(int offset, jlong contents)     { *long_field_addr(offset) = contents;   }

inline jfloat oopDesc::float_field(int offset) const                { return *float_field_addr(offset);      }
inline void oopDesc::float_field_put(int offset, jfloat contents)   { *float_field_addr(offset) = contents;  }

inline jdouble oopDesc::double_field(int offset) const              { return *double_field_addr(offset);     }
inline void oopDesc::double_field_put(int offset, jdouble contents) { *double_field_addr(offset) = contents; }

inline oop oopDesc::obj_field_acquire(int offset) const                     { return (oop)OrderAccess::load_ptr_acquire(obj_field_addr(offset)); }
inline void oopDesc::release_obj_field_put(int offset, oop value)           { oop_store((volatile oop*)obj_field_addr(offset), value);           }

inline jbyte oopDesc::byte_field_acquire(int offset) const                  { return OrderAccess::load_acquire(byte_field_addr(offset));     }
inline void oopDesc::release_byte_field_put(int offset, jbyte contents)     { OrderAccess::release_store(byte_field_addr(offset), contents); }

inline jboolean oopDesc::bool_field_acquire(int offset) const               { return OrderAccess::load_acquire(bool_field_addr(offset));     }
inline void oopDesc::release_bool_field_put(int offset, jboolean contents)  { OrderAccess::release_store(bool_field_addr(offset), contents); }

inline jchar oopDesc::char_field_acquire(int offset) const                  { return OrderAccess::load_acquire(char_field_addr(offset));     }
inline void oopDesc::release_char_field_put(int offset, jchar contents)     { OrderAccess::release_store(char_field_addr(offset), contents); }

inline jint oopDesc::int_field_acquire(int offset) const                    { return OrderAccess::load_acquire(int_field_addr(offset));      }
inline void oopDesc::release_int_field_put(int offset, jint contents)       { OrderAccess::release_store(int_field_addr(offset), contents);  }

inline jshort oopDesc::short_field_acquire(int offset) const                { return (jshort)OrderAccess::load_acquire(short_field_addr(offset)); }
inline void oopDesc::release_short_field_put(int offset, jshort contents)   { OrderAccess::release_store(short_field_addr(offset), contents);     }

inline jlong oopDesc::long_field_acquire(int offset) const                  { return OrderAccess::load_acquire(long_field_addr(offset));       }
inline void oopDesc::release_long_field_put(int offset, jlong contents)     { OrderAccess::release_store(long_field_addr(offset), contents);   }

inline jfloat oopDesc::float_field_acquire(int offset) const                { return OrderAccess::load_acquire(float_field_addr(offset));      }
inline void oopDesc::release_float_field_put(int offset, jfloat contents)   { OrderAccess::release_store(float_field_addr(offset), contents);  }

inline jdouble oopDesc::double_field_acquire(int offset) const              { return OrderAccess::load_acquire(double_field_addr(offset));     }
inline void oopDesc::release_double_field_put(int offset, jdouble contents) { OrderAccess::release_store(double_field_addr(offset), contents); }


inline int oopDesc::size_given_klass(Klass* klass)  {
  int s = klass->size_helper();

  // s is now a value computed at class initialization that may hint
  // at the size.  For instances, this is positive and equal to the
  // size.  For arrays, this is negative and one minus log2 of the
  // array element size.  For other oops, it is zero and thus requires
  // a virtual call.
  //
  // We go to all this trouble because the size computation is at the
  // heart of phase 2 of mark-compaction, and called for every object,
  // alive or dead.  So the speed here is equal in importance to the
  // speed of allocation.

  if (s <= 0) {    // The most common case is instances; fall through if so.
    if (s < 0) {
      // Second most common case is arrays.  We have to fetch the
      // length of the array, shift (multiply) it appropriately, 
      // up to wordSize, add the header, and align to object size.
#ifdef _M_IA64
      // The Windows Itanium Aug 2002 SDK hoists this load above
      // the check for s < 0.  An oop at the end of the heap will
      // cause an access violation if this load is performed on a non
      // array oop.  Making the reference volatile prohibits this.
      volatile int *array_length;
      array_length = (volatile int *)( (intptr_t)this + 
                          arrayOopDesc::length_offset_in_bytes() );
      s = *array_length << (-1-s);
#else
      s = ((arrayOop)this)->length() << (-1-s);
#endif
      s += ((arrayKlass*)klass)->array_header_in_bytes();

      // This code could be simplified, but by keeping array_header_in_bytes
      // in units of bytes and doing it this way we can round up just once,
      // skipping the intermediate round to HeapWordSize.  Cast the result
      // of round_to to size_t to guarantee unsigned division == right shift.
      s = (int)((size_t)round_to(s, MinObjAlignmentInBytes) / HeapWordSize);
      assert(s == klass->oop_size(this), "wrong array object size");
    } else {
      // Must be zero, so bite the bullet and take the virtual call.
      s = klass->oop_size(this);
      assert(s == klass->oop_size(this), "Size helper is wrong");
    }
  }

  assert(s % MinObjAlignment == 0, "alignment check");
  return s;
}


inline int oopDesc::size()  {
  return size_given_klass(blueprint());
}

inline bool oopDesc::is_parsable() {
  return blueprint()->oop_is_parsable(this);
}


inline void update_barrier_set(oop *p, oop v) {
  assert(oopDesc::bs() != NULL, "Uninitialized bs in oop!");
  oopDesc::bs()->write_ref_field(p, v);

  if (UseTrainGC) {
    // Each generation has a chance to examine the oop.
    CollectedHeap* gch = Universe::heap();
    // This is even more bogus.
    if (gch->kind() == CollectedHeap::GenCollectedHeap) {
      ((GenCollectedHeap*)gch)->examine_modified_oop(p);
    }
  }
}


inline void oop_store(oop* p, oop v) {
  *p = v;
  update_barrier_set(p, v);
}

inline void oop_store(volatile oop* p, oop v) {
  // Used by release_obj_field_put, so use release_store_ptr.
  OrderAccess::release_store_ptr(p, v);
  update_barrier_set((oop *)p, v);
}

inline void oop_store_without_check(oop* p, oop v) {
  // XXX YSR FIX ME!!!
  if (always_do_update_barrier) {
   oop_store(p, v);
  } else {
    assert(!Universe::heap()->barrier_set()->write_ref_needs_barrier(p, v),
           "oop store without store check failed");
    *p = v;
  }
}

// When it absolutely has to get there.
inline void oop_store_without_check(volatile oop* p, oop v) {
  // XXX YSR FIX ME!!!
  if (always_do_update_barrier) {
    oop_store(p, v);
  } else {
    assert(!Universe::heap()->barrier_set()->
                      write_ref_needs_barrier((oop *)p, v),
           "oop store without store check failed");
    OrderAccess::release_store_ptr(p, v);
  }
}


// Used only for markSweep, scavenging, train
inline bool oopDesc::is_gc_marked() const {
  return mark()->is_marked();
}

inline bool oopDesc::is_locked() const {
  return mark()->is_locked();
}

inline bool oopDesc::is_unlocked() const {
  return mark()->is_unlocked();
}

#ifndef PRODUCT

inline bool check_obj_alignment(oop obj) {
  return (intptr_t)obj % MinObjAlignmentInBytes == 0;
}


// used only for asserts
inline bool oopDesc::is_oop(bool ignore_mark_word) const {
  oop obj = (oop) this;
  if (!check_obj_alignment(obj)) return false;
  if (!Universe::heap()->is_in(obj)) return false;
  // obj is aligned and accessible in heap
  // try to find metaclass cycle safely without seg faulting on bad input
  // we should reach klassKlassObj by following klass link at most 3 times
  for (int i = 0; i < 3; i++) {
    obj = obj->klass();
    // klass should be aligned and in permspace
    if (!check_obj_alignment(obj)) return false;
    if (!Universe::heap()->is_in_permanent(obj)) return false;
  }
  if (obj != Universe::klassKlassObj()) {
    // During a dump, the _klassKlassObj moved to a shared space.
    if (DumpSharedSpaces && Universe::klassKlassObj()->is_shared()) {
      return true;
    }
    return false;
  }

  // Header verification: the mark is typically non-NULL. If we're
  // at a safepoint, it must not be null.
  // Outside of a safepoint, the header could be changing (for example,
  // another thread could be inflating a lock on this object).
  if (ignore_mark_word) {
    return true;
  }
  if (mark() != NULL) {
    return true;
  }
  return !SafepointSynchronize::is_at_safepoint();
}


// used only for asserts
inline bool oopDesc::is_oop_or_null() const {
  return this == NULL ? true : is_oop();
}


// used only for asserts
inline bool oopDesc::is_unlocked_oop() const {
  if (!Universe::heap()->is_in_reserved(this)) return false;
  return mark()->is_unlocked();
}


#endif // PRODUCT

inline void oopDesc::follow_header() { 
  MarkSweep::mark_and_push((oop*)&_klass);
}

inline void oopDesc::follow_contents() {
  assert (is_gc_marked(), "should be marked");
  blueprint()->oop_follow_contents(this);
}

inline bool oopDesc::being_unloaded(BoolObjectClosure* is_alive) {
  // Tells whether we should discard this object in order not to
  // prevent a klass from being unloaded.
  return is_alive->do_object_b(this) ? false :
         blueprint()->oop_being_unloaded(is_alive, this);
}

// Used by scavenger and train GC:

inline bool oopDesc::is_forwarded() const { 
  // The extra heap check is needed since the obj might be locked, in which case the
  // mark would point to a stack location and have the sentinel bit cleared
  return mark()->is_marked();
}


// Used by scavenger and train gc
inline void oopDesc::forward_to(oop p) {
  assert(Universe::heap()->is_in_reserved(p),
         "forwarding to something not in heap");
  markOop m = markOopDesc::encode_pointer_as_mark(p);
  assert(m->decode_pointer() == p, "encoding must be reversable");
  set_mark(m);
}

// Used by parallel scavengers
inline bool oopDesc::cas_forward_to(oop p, markOop compare) {
  assert(Universe::heap()->is_in_reserved(p),
	 "forwarding to something not in heap");
  markOop m = markOopDesc::encode_pointer_as_mark(p);
  assert(m->decode_pointer() == p, "encoding must be reversable");
  return cas_set_mark(m, compare) == compare;
}

// Note that the forwardee is not the same thing as the displaced_mark.
// The forwardee is used when copying on scavenge.  It does not need to worry
// about clearing the waiters bit of the lock bits (not used for locking).

// But, it does need to worry about clearing the busy lock bits for
// those platforms that use the (low-order) lock bits instead of the
// high-order sentinal bit.
inline oop oopDesc::forwardee() const           { return (oop) mark()->decode_pointer(); }


inline bool oopDesc::has_displaced_mark() const {
  return !is_unlocked();
}

inline markOop oopDesc::displaced_mark() const {
  return mark()->displaced_mark_helper();
}

inline void oopDesc::set_displaced_mark(markOop m) {
  mark()->set_displaced_mark_helper(m);
}

// The following method needs to be MT safe.
inline int oopDesc::age() const {
  assert(!is_forwarded(), "Attempt to read age from forwarded mark");
  if (has_displaced_mark()) {
    return displaced_mark()->age();
  } else {
    return mark()->age();
  }
}

inline void oopDesc::incr_age() {
  assert(!is_forwarded(), "Attempt to increment age of forwarded mark");
  if (has_displaced_mark()) {
    set_displaced_mark(displaced_mark()->incr_age());
  } else {
    set_mark(mark()->incr_age());
  }
}


inline intptr_t oopDesc::identity_hash() {
  // Fast case; if the object is unlocked and the hash value is set, no locking is needed
  // Note: The mark must be read into local variable to avoid concurrent updates.
  markOop mrk = mark();
  if (mrk->is_unlocked() && !mrk->has_no_hash()) {
    return mrk->hash();
  } else if (mrk->is_marked()) {
    return mrk->hash();
  } else {
    return slow_identity_hash();
  }
}


inline void oopDesc::oop_iterate_header(OopClosure* blk) {
  blk->do_oop((oop*)&_klass);
}


inline void oopDesc::oop_iterate_header(OopClosure* blk, MemRegion mr) {
  if (mr.contains(&_klass)) blk->do_oop((oop*)&_klass);
}


inline int oopDesc::adjust_pointers() {
  debug_only(int check_size = size());
  int s = blueprint()->oop_adjust_pointers(this);
  assert(s == check_size, "should be the same");
  return s;
}


inline void oopDesc::adjust_header() {
  MarkSweep::adjust_pointer((oop*)&_klass);
}

#define OOP_ITERATE_DEFN(OopClosureType, nv_suffix)                        \
                                                                           \
inline int oopDesc::oop_iterate(OopClosureType* blk) {                     \
  SpecializationStats::record_call();                                      \
  return blueprint()->oop_oop_iterate##nv_suffix(this, blk);               \
}                                                                          \
                                                                           \
inline int oopDesc::oop_iterate(OopClosureType* blk, MemRegion mr) {       \
  SpecializationStats::record_call();                                      \
  return blueprint()->oop_oop_iterate##nv_suffix##_m(this, blk, mr);       \
}

ALL_OOP_OOP_ITERATE_CLOSURES_1(OOP_ITERATE_DEFN) 
ALL_OOP_OOP_ITERATE_CLOSURES_2(OOP_ITERATE_DEFN) 
ALL_OOP_OOP_ITERATE_CLOSURES_3(OOP_ITERATE_DEFN) 


inline bool oopDesc::is_shared() const {
  return CompactingPermGenGen::is_shared(this);
}

inline bool oopDesc::is_shared_readonly() const {
  return CompactingPermGenGen::is_shared_readonly(this);
}

inline bool oopDesc::is_shared_readwrite() const {
  return CompactingPermGenGen::is_shared_readwrite(this);
}
