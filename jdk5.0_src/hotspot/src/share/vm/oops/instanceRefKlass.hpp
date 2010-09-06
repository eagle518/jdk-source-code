#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)instanceRefKlass.hpp	1.50 03/12/23 16:41:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An instanceRefKlass is a specialized instanceKlass for Java 
// classes that are subclasses of java/lang/ref/Reference.
//
// These classes are used to implement soft/weak/final/phantom 
// references and finalization, and need special treatment by the
// garbage collector.
//
// During GC discovered reference objects are added (chained) to one
// of the four lists below, depending on the type of reference.
// The linked occurs through the next field in class java/lang/ref/Reference.
//
// Afterwards, the discovered references are processed in decreasing
// order of reachability. Reference objects eligible for notification
// are linked to the static pending_list in class java/lang/ref/Reference,
// and the pending list lock object in the same class is notified.


class instanceRefKlass: public instanceKlass {
 public:
  // Type testing
  bool oop_is_instanceRef() const             { return true; }

  // Casting from klassOop
  static instanceRefKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_instanceRef(), "cast to instanceRefKlass");
    return (instanceRefKlass*) k->klass_part(); 
  }

  // allocation
  DEFINE_ALLOCATE_PERMANENT(instanceRefKlass);

  // Garbage collection
  int  oop_adjust_pointers(oop obj);
  void oop_follow_contents(oop obj);

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  int oop_oop_iterate(oop obj, OopClosure* blk) {
    return oop_oop_iterate_v(obj, blk);
  }
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
    return oop_oop_iterate_v_m(obj, blk, mr);
  }

#define InstanceRefKlass_OOP_OOP_ITERATE_DECL(OopClosureType, nv_suffix)                \
  int oop_oop_iterate##nv_suffix(oop obj, OopClosureType* blk);                         \
  int oop_oop_iterate##nv_suffix##_m(oop obj, OopClosureType* blk, MemRegion mr);

  ALL_OOP_OOP_ITERATE_CLOSURES_1(InstanceRefKlass_OOP_OOP_ITERATE_DECL)
  ALL_OOP_OOP_ITERATE_CLOSURES_2(InstanceRefKlass_OOP_OOP_ITERATE_DECL)
  ALL_OOP_OOP_ITERATE_CLOSURES_3(InstanceRefKlass_OOP_OOP_ITERATE_DECL)

  static void release_and_notify_pending_list_lock(bool notify_ref_lock,
                                                   BasicLock *pending_list_basic_lock);
  static void acquire_pending_list_lock(BasicLock *pending_list_basic_lock);

  // Update non-static oop maps so 'referent', 'nextPending' and
  // 'discovered' will look like non-oops
  static void update_nonstatic_oop_maps(klassOop k);

#ifndef PRODUCT
 public:
  // Verification
  void oop_verify_on(oop obj, outputStream* st);
#endif
};
