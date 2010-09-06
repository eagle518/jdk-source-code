#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)genRemSet.cpp	1.5 03/12/23 16:41:12 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

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
