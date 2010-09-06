#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)markOop.hpp	1.51 03/12/23 16:41:58 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The markOop describes the header of an object.
//
// Note that the mark is not a real oop but just a word. 
// It is placed in the oop hierarchy for historical reasons.
//
// Bit-format of an object header (most significant first):
//
//  
//  unused:0/25 hash:25/31 age:5 lock:2 = 32/64 bits
//
//  - hash contains the identity hash value: largest value is
//    31 bits, see os::random().  Also, 64-bit vm's require
//    a hash value no bigger than 32 bits because they will not
//    properly generate a mask larger than that: see library_call.cpp
//    and c1_CodePatterns_sparc.cpp.
//
//  - the two lock bits are used to describe three states: locked/unlocked and monitor.
//
//    [ptr    | 00]  locked                  ptr points to real header on stack
//    [header | 01]  unlocked                regular object header
//    [ptr    | 10]  monitor                 inflated lock (header is wapped out)
//    [ptr    | 11]  marked                  used by markSweep to mark an object
//                                           not valid at any other time
//
//    We assume that stack/thread pointers have the lowest two bits cleared.

class BasicLock;
class ObjectMonitor;

class markOopDesc: public oopDesc {
 private:
  // Conversion
  intptr_t value() const { return (intptr_t) this; }

 public:
  // Constants
  enum { age_bits                 = 5,
         lock_bits                = 2,
         max_hash_bits            = BitsPerOop - age_bits - lock_bits,
         hash_bits                = max_hash_bits > 31 ? 31 : max_hash_bits
  };

  // For shared read-only symbolOop objects, the mark word is set to its
  // own address (with marked_value) in the lock bit.  Since the same
  // word is used for the identity hash, using the lower order bits
  // (other than the lock bits) for the hash makes these values more
  // random.

  enum { lock_shift               = 0,
         hash_shift               = lock_bits,
         age_shift                = lock_bits + hash_bits
  };

  enum { lock_mask                = right_n_bits(lock_bits),
         lock_mask_in_place       = lock_mask << lock_shift,
         age_mask                 = right_n_bits(age_bits),
         age_mask_in_place        = age_mask << age_shift
#ifndef _WIN64
         ,hash_mask               = right_n_bits(hash_bits),
         hash_mask_in_place       = (address_word)hash_mask << hash_shift
#endif
  };

#ifdef _WIN64
    // These values are too big for Win64
    const static uintptr_t hash_mask = right_n_bits(hash_bits); 
    const static uintptr_t hash_mask_in_place  = 
                            (address_word)hash_mask << hash_shift;
#endif

  enum { locked_value             = 0,
         unlocked_value           = 1,
         monitor_value            = 2,
         marked_value             = 3
  };

  enum { no_hash                  = 0 };  // no hash value assigned

  enum { no_hash_in_place         = (address_word)no_hash << hash_shift,
         no_lock_in_place         = unlocked_value
  };

  enum { max_age                  = age_mask };

  // lock accessors (note that these assume lock_shift == 0)
  bool is_locked()   const {
    return (mask_bits(value(), lock_mask_in_place) != unlocked_value);
  }
  bool is_unlocked() const {
    return (mask_bits(value(), lock_mask_in_place) == unlocked_value);
  }
  bool is_marked()   const {
    return (mask_bits(value(), lock_mask_in_place) == marked_value);
  }

  // Special temporary state of the markOop while being inflated.
  // Code that looks at mark outside a lock need to take this into account.
  bool is_being_inflated() const {
    return (value() == 0);
  }

  // Should this header be preserved during GC?
  bool must_be_preserved() const {
     return (!is_unlocked() || !has_no_hash());
  }

  // WARNING: The following routines are used EXCLUSIVELY by 
  // synchronization functions. They are not really gc safe.
  // They must get updated if markOop layout get changed.
  markOop set_unlocked() const {
    return markOop(value() | unlocked_value);
  }
  bool has_locker() const {
    return ((value() & lock_mask_in_place) == locked_value);
  }
  BasicLock* locker() const {
    assert(has_locker(), "check");
    return (BasicLock*) value();
  }
  bool has_monitor() const {
    return ((value() & monitor_value) != 0);
  }
  ObjectMonitor* monitor() const {
    assert(has_monitor(), "check");
    // Use xor instead of &~ to provide one extra tag-bit check.
    return (ObjectMonitor*) (value() ^ monitor_value);
  }
  bool has_displaced_mark_helper() const {
    return ((value() & unlocked_value) == 0);
  }
  markOop displaced_mark_helper() const {
    assert(has_displaced_mark_helper(), "check");
    intptr_t ptr = (value() & ~monitor_value);
    return *(markOop*)ptr;
  }
  void set_displaced_mark_helper(markOop m) const {
    assert(has_displaced_mark_helper(), "check");
    intptr_t ptr = (value() & ~monitor_value);
    *(markOop*)ptr = m;
  }
  markOop copy_set_hash(intptr_t hash) const {
    intptr_t tmp = value() & (~hash_mask_in_place);
    tmp |= ((hash & hash_mask) << hash_shift);
    return (markOop)tmp;
  }
  // it is only used to be stored into BasicLock as the 
  // indicator that the lock is using heavyweight monitor
  static markOop unused_mark() {
    return (markOop) marked_value;
  }
  // the following two functions create the markOop to be
  // stored into object header, it encodes monitor info
  static markOop encode(BasicLock* lock) {
    return (markOop) lock;
  }
  static markOop encode(ObjectMonitor* monitor) {
    intptr_t tmp = (intptr_t) monitor;
    return (markOop) (tmp | monitor_value);
  }
  
  // used for alignment-based marking to reuse the busy state to encode pointers
  // (see markOop_alignment.hpp)  
  markOop clear_lock_bits() { return markOop(value() & ~lock_mask_in_place); }  

  // age operations
  markOop set_marked()   { return markOop((value() & ~lock_mask_in_place) | marked_value); }

  int     age()               const { return mask_bits(value() >> age_shift, age_mask); }
  markOop set_age(int v) const {
    assert((v & ~age_mask) == 0, "shouldn't overflow age field");
    return markOop((value() & ~age_mask_in_place) | (((intptr_t)v & age_mask) << age_shift));
  }
  markOop incr_age()          const { return age() == max_age ? markOop(this) : set_age(age() + 1); }

  // hash operations
  intptr_t hash() const {     
    return mask_bits(value() >> hash_shift, hash_mask);
  }
  
  bool has_no_hash() const { 
    return hash() == no_hash; 
  }

  // Prototype mark for initialization
  static markOop prototype() {
    return markOop( no_hash_in_place | no_lock_in_place );
  }

  // Debugging
  void print_on(outputStream* st) const;

  // Prepare address of oop for placement into mark
  inline static markOop encode_pointer_as_mark(void* p) { return markOop(p)->set_marked(); }

  // Recover address of oop from encoded form used in mark
  inline void* decode_pointer() { return clear_lock_bits(); }
};
