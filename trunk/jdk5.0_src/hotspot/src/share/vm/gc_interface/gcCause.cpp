#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcCause.cpp	1.7 03/12/23 16:40:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gcCause.cpp.incl"

const char* GCCause::to_string(GCCause::Cause cause) {
  switch (cause) {
    case _java_lang_system_gc:
      return "System.gc()";

    case _full_gc_alot:
      return "FullGCAlot";

    case _allocation_profiler:
      return "Allocation Profiler";

    case _jvmti_force_gc:
      return "JvmtiEnv ForceGarbageCollection";

    case _no_gc:
      return "No GC";

    case _allocation_failure:
      return "Allocation Failure";

    case _tenured_generation_full:
      return "Tenured Generation Full";

    case _permanent_generation_full:
      return "Permanent Generation Full";

    case _train_generation_full:
      return "Train Generation Full";

    case _cms_generation_full:
      return "CMS Generation Full";

    case _old_generation_expanded_on_last_scavenge:
      return "Old Generation Expanded On Last Scavenge";

    case _old_generation_too_full_to_scavenge:
      return "Old Generation Too Full To Scavenge";

    case _last_ditch_collection:
      return "Last ditch collection";

    case _last_gc_cause:
      return "ILLEGAL VALUE - last gc cause - ILLEGAL VALUE";

    default:
      return "unknown GCCause";
  }
  ShouldNotReachHere();
}

#ifndef PRODUCT

bool GCCause::is_for_full_collection(GCCause::Cause cause) {
  bool result;

  // There are more GCCause::Cause types than listed here.
  // For brevity, we list only those that cause full collections.
  switch (cause) {
    case _allocation_failure:
    case _tenured_generation_full:
    case _permanent_generation_full:
    case _train_generation_full:
    case _cms_generation_full:
    case _last_ditch_collection:
      result = true;
      break;

    default:
      result = false;
      break;
  }
  return result;
}

#endif // PRODUCT
