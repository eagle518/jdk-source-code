/*
 * Copyright (c) 2001, 2002, Oracle and/or its affiliates. All rights reserved.
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

// This kind of "BarrierSet" allows a "CollectedHeap" to detect and
// enumerate ref fields that have been modified (since the last
// enumeration.)

# include "incls/_precompiled.incl"
# include "incls/_genRemSet.cpp.incl"

uintx GenRemSet::max_alignment_constraint(Name nm) {
  switch (nm) {
  case GenRemSet::CardTable:
    return CardTableRS::ct_max_alignment_constraint();
  default:
    guarantee(false, "Unrecognized GenRemSet type.");
    return (0); // Make Windows compiler happy
  }
}
