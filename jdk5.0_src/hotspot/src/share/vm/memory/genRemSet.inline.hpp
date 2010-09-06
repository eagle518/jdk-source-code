#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)genRemSet.inline.hpp	1.4 03/12/23 16:41:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Inline functions of GenRemSet, which de-virtualize this
// performance-critical call when when the rem set is the most common
// card-table kind.

void GenRemSet::write_ref_field_gc(oop* field, oop new_val) {
  if (kind() == CardTableModRef) {
    ((CardTableRS*)this)->inline_write_ref_field_gc(field, new_val);
  } else {
    write_ref_field_gc_work(field, new_val);
  }
}
