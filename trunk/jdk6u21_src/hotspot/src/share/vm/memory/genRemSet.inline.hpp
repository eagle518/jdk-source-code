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

// Inline functions of GenRemSet, which de-virtualize this
// performance-critical call when when the rem set is the most common
// card-table kind.

void GenRemSet::write_ref_field_gc(void* field, oop new_val) {
  if (kind() == CardTableModRef) {
    ((CardTableRS*)this)->inline_write_ref_field_gc(field, new_val);
  } else {
    write_ref_field_gc_work(field, new_val);
  }
}
