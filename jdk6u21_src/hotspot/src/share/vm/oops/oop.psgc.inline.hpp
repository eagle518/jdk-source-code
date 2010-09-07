/*
 * Copyright (c) 2002, 2007, Oracle and/or its affiliates. All rights reserved.
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

// ParallelScavengeHeap methods

inline void oopDesc::copy_contents(PSPromotionManager* pm) {
  Klass* klass = blueprint();
  if (!klass->oop_is_typeArray()) {
    // It might contain oops beyond the header, so take the virtual call.
    klass->oop_copy_contents(pm, this);
  }
  // Else skip it.  The typeArrayKlass in the header never needs scavenging.
}

inline void oopDesc::push_contents(PSPromotionManager* pm) {
  Klass* klass = blueprint();
  if (!klass->oop_is_typeArray()) {
    // It might contain oops beyond the header, so take the virtual call.
    klass->oop_push_contents(pm, this);
  }
  // Else skip it.  The typeArrayKlass in the header never needs scavenging.
}
