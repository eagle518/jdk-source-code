#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oop.psgc.inline.hpp	1.8 03/12/23 16:42:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved. 
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms. 
 */

// ParallelScavengeHeap methods

inline void oopDesc::copy_contents(PSPromotionManager* pm) {
  Klass* klass = blueprint();
  int s = klass->size_helper();
  // If the size_helper in the Klass is either -1 or -2, then
  // the object must be a char[], short[] or byte[].  In any 
  // case there will be no pointers to scavenge; the typeArrayKlass 
  // itself never needs scavenging.
  if ( ~(s | 1) ) {
      // Not a char[], short[] or byte[], so take the virtual call.
    klass->oop_copy_contents(pm, this);
  }
}

