#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oop.hpp	1.97 03/12/23 16:42:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// oopDesc is the top baseclass for objects classes.  The {name}Desc classes describe
// the format of Java objects so the fields can be accessed from C++.
// oopDesc is abstract.
// (see oopHierarchy for complete oop class hierarchy)
//
// no virtual functions allowed

// store into oop with store check
inline void oop_store(oop* p, oop v);
inline void oop_store(volatile oop* p, oop v);

// store into oop without store check
inline void oop_store_without_check(oop* p, oop v);
inline void oop_store_without_check(volatile oop* p, oop v);

extern bool always_do_update_barrier;

// Forward declarations.
class OopClosure;
class ScanClosure;
class FastScanClosure;
class FilteringClosure;
class TrainScanClosure;
class UpdateTrainRSWrapScanClosure;
class UpdateTrainRSWrapTrainScanClosure;
class BarrierSet;
class CMSIsAliveClosure;

class PSPromotionManager;

class oopDesc {
  friend class VMStructs;
 private:
  volatile markOop  _mark;
  klassOop _klass;

  // Fast access to barrier set.  Must be initialized.
  static BarrierSet* _bs;

 public:
  markOop  mark() const         { return _mark; }
  markOop* mark_addr() const    { return (markOop*) &_mark; }

  void set_mark()               { init_mark(); }
  void set_mark(markOop m)      { _mark = m;   }

  inline void    release_set_mark(markOop m);
  inline markOop cas_set_mark(markOop new_mark, markOop old_mark);

  inline void init_mark();

  klassOop klass() const        { return _klass; }
  oop* klass_addr() const       { return (oop*) &_klass; }

  inline void set_klass(klassOop k);
  // For when the klass pointer is being used as a linked list "next" field.
  inline void set_klass_to_list_ptr(oop k);

  // size of object header
  static int header_size()      { return sizeof(oopDesc)/HeapWordSize; }
  static int header_size_in_bytes() { return sizeof(oopDesc); }

  inline Klass* blueprint() const;

  // Returns whether this is an instance of k or an instance of a subclass of k
  inline bool is_a(klassOop k)  const;

  // Returns the actual oop size of the object
  inline int size();

  // Sometimes (for complicated concurrency-related reasons), it is useful
  // to be able to figure out the size of an object knowing its klass.
  inline int size_given_klass(Klass* klass);

  // Some perm gen objects are not parseble immediately after
  // installation of their klass pointer.
  inline bool is_parsable();

  // type test operations (inlined in oop.inline.h)
  inline bool is_instance()           const;
  inline bool is_instanceRef()        const;
  inline bool is_array()              const;
  inline bool is_objArray()           const;
  inline bool is_objArray_fast()      const;
  inline bool is_symbol()             const;
  inline bool is_klass()              const;
  inline bool is_thread()             const;
  inline bool is_method()             const;
  inline bool is_constMethod()        const;
#ifndef CORE
  inline bool is_methodData()         const;
#endif // !CORE
  inline bool is_constantPool()       const;
  inline bool is_constantPoolCache()  const;
  inline bool is_typeArray()          const;
  inline bool is_compiledICHolder()   const;

 private:
  // field addresses in oop
  // byte/char/bool/short fields are always stored as full words
  inline void*     field_base(int offset)        const;

  inline jbyte*    byte_field_addr(int offset)   const;
  inline jchar*    char_field_addr(int offset)   const;
  inline jboolean* bool_field_addr(int offset)   const;
  inline jint*     int_field_addr(int offset)    const;
  inline jshort*   short_field_addr(int offset)  const;
  inline jlong*    long_field_addr(int offset)   const;
  inline jfloat*   float_field_addr(int offset)  const;
  inline jdouble*  double_field_addr(int offset) const;

 public:
  // need this as public for garbage collection
  inline oop* obj_field_addr(int offset) const;

  inline oop obj_field(int offset) const;
  inline void obj_field_put(int offset, oop value);

  inline jbyte byte_field(int offset) const;
  inline void byte_field_put(int offset, jbyte contents);

  inline jchar char_field(int offset) const;
  inline void char_field_put(int offset, jchar contents);

  inline jboolean bool_field(int offset) const;
  inline void bool_field_put(int offset, jboolean contents);

  inline jint int_field(int offset) const;
  inline void int_field_put(int offset, jint contents);

  inline jshort short_field(int offset) const;
  inline void short_field_put(int offset, jshort contents);

  inline jlong long_field(int offset) const;
  inline void long_field_put(int offset, jlong contents);

  inline jfloat float_field(int offset) const;
  inline void float_field_put(int offset, jfloat contents);

  inline jdouble double_field(int offset) const;
  inline void double_field_put(int offset, jdouble contents);

  inline oop obj_field_acquire(int offset) const;
  inline void release_obj_field_put(int offset, oop value);

  inline jbyte byte_field_acquire(int offset) const;
  inline void release_byte_field_put(int offset, jbyte contents);

  inline jchar char_field_acquire(int offset) const;
  inline void release_char_field_put(int offset, jchar contents);

  inline jboolean bool_field_acquire(int offset) const;
  inline void release_bool_field_put(int offset, jboolean contents);

  inline jint int_field_acquire(int offset) const;
  inline void release_int_field_put(int offset, jint contents);

  inline jshort short_field_acquire(int offset) const;
  inline void release_short_field_put(int offset, jshort contents);

  inline jlong long_field_acquire(int offset) const;
  inline void release_long_field_put(int offset, jlong contents);

  inline jfloat float_field_acquire(int offset) const;
  inline void release_float_field_put(int offset, jfloat contents);

  inline jdouble double_field_acquire(int offset) const;
  inline void release_double_field_put(int offset, jdouble contents);

  // printing functions for VM debugging
  void print_on(outputStream* st) const;         // First level print 
  void print_value_on(outputStream* st) const;   // Second level print.
  void print_address_on(outputStream* st) const; // Address printing

  // printing on default output stream
  void print();
  void print_value();
  void print_address();

  // return the print strings
  char* print_string();
  char* print_value_string();

  // verification operations
  void verify_on(outputStream* st);
  void verify();
  void verify_old_oop(oop* p, bool allow_dirty);

  // tells whether this oop is partially constructed (gc during class loading)
  bool partially_loaded();
  void set_partially_loaded();

  // locking operations
  inline bool is_locked()   const;
  inline bool is_unlocked() const;

  // asserts
#ifndef PRODUCT
  inline bool is_oop(bool ignore_mark_word = false) const;
  inline bool is_oop_or_null() const;
  inline bool is_unlocked_oop() const;
#endif

  // garbage collection
  inline bool is_gc_marked() const;
  // Apply "MarkSweep::mark_and_push" to (the address of) every non-NULL
  // reference field in "this".
  inline void follow_contents();
  inline void follow_header();
  inline bool being_unloaded(BoolObjectClosure* is_alive);

  // Parallel Scavenge
  inline void copy_contents(PSPromotionManager* pm);

  inline bool is_perm() const;
  inline bool is_shared() const;
  inline bool is_shared_readonly() const;
  inline bool is_shared_readwrite() const;

  // Forward pointer operations for scavenge
  inline bool is_forwarded() const;

  inline void forward_to(oop p);
  inline bool cas_forward_to(oop p, markOop compare);

  // Like "forward_to", but inserts the forwarding pointer atomically.
  // Exactly one thread succeeds in inserting the forwarding pointer, and
  // this call returns "NULL" for that thread; any other thread has the
  // value of the forwarding pointer returned and does not modify "this".
  oop forward_to_atomic(oop p);

  inline oop forwardee() const;

  // Age of object during scavenge
  inline int age() const;
  inline void incr_age();

  // Adjust all pointers in this object to point at it's forwarded location and
  // return the size of this oop.  This is used by the MarkSweep collector.
  inline int adjust_pointers();
  inline void adjust_header();

  // mark-sweep support
  inline void follow_body(int begin, int end);

  // Fast access to barrier set
  static BarrierSet* bs()            { return _bs; }
  static void set_bs(BarrierSet* bs) { _bs = bs; }

  // iterators, returns size of object
#define OOP_ITERATE_DECL(OopClosureType, nv_suffix)                             \
  inline int oop_iterate(OopClosureType* blk);                                  \
  inline int oop_iterate(OopClosureType* blk, MemRegion mr);  // Only in mr.

  ALL_OOP_OOP_ITERATE_CLOSURES_1(OOP_ITERATE_DECL) 
  ALL_OOP_OOP_ITERATE_CLOSURES_2(OOP_ITERATE_DECL) 
  ALL_OOP_OOP_ITERATE_CLOSURES_3(OOP_ITERATE_DECL) 

  inline void oop_iterate_header(OopClosure* blk);
  inline void oop_iterate_header(OopClosure* blk, MemRegion mr);

  // identity hash; returns the identity hash key (computes it if necessary)
  inline intptr_t identity_hash();
  intptr_t slow_identity_hash();

  // marks are forwarded to stack when object is locked
  inline bool     has_displaced_mark() const;
  inline markOop  displaced_mark() const;
  inline void     set_displaced_mark(markOop m);

  // for code generation
  static int klass_offset_in_bytes()   { return (intptr_t)&(oop(NULL)->_klass); }
  static int mark_offset_in_bytes()    { return (intptr_t)&(oop(NULL)->_mark); }
};
