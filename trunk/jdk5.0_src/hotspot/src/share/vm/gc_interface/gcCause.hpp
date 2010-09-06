#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcCause.hpp	1.9 03/12/23 16:40:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// This class exposes implementation details of the various
// collector(s), and we need to be very careful with it. If
// use of this class grows, we should split it into public
// and implemenation-private "causes".
//

class GCCause : public AllStatic {
 public:
  enum Cause {
    /* public */
    _java_lang_system_gc,
    _full_gc_alot,
    _allocation_profiler,
    _jvmti_force_gc,

    /* implementation independant, but not for VM use */
    _no_gc,
    _allocation_failure,

    /* implementation specific */
    _tenured_generation_full,
    _permanent_generation_full,
    _train_generation_full,
    _cms_generation_full,
    _old_generation_expanded_on_last_scavenge,
    _old_generation_too_full_to_scavenge,
    _adaptive_size_policy,
    _last_ditch_collection,
    _last_gc_cause
  };

  inline static bool is_user_requested_gc(GCCause::Cause cause) {
    return (cause == GCCause::_java_lang_system_gc ||
            cause == GCCause::_jvmti_force_gc);
  };
  // Return a string describing the GCCause.
  static const char* to_string(GCCause::Cause cause);
  // Return true if the GCCause is for a full collection.
  static bool is_for_full_collection(GCCause::Cause cause) PRODUCT_RETURN0;
};
