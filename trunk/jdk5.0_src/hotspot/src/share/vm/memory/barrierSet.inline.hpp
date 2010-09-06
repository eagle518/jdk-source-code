#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)barrierSet.inline.hpp	1.6 03/12/23 16:40:51 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Inline functions of BarrierSet, which de-virtualize certain
// performance-critical calls when when the barrier is the most common
// card-table kind.

void BarrierSet::write_ref_field(oop* field, oop new_val) {
  if (kind() == CardTableModRef) {
    ((CardTableModRefBS*)this)->inline_write_ref_field(field, new_val);
  } else {
    write_ref_field_work(field, new_val);
  }
}

void BarrierSet::write_ref_array(MemRegion mr) {
  if (kind() == CardTableModRef) {
    ((CardTableModRefBS*)this)->inline_write_ref_array(mr);
  } else {
    write_ref_array_work(mr);
  }
}

void BarrierSet::write_region(MemRegion mr) {
  if (kind() == CardTableModRef) {
    ((CardTableModRefBS*)this)->inline_write_region(mr);
  } else {
    write_region_work(mr);
  }
}
