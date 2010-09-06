#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)specialized_oop_closures.hpp	1.21 03/12/23 16:41:31 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The following OopClosure types get specialized versions of
// "oop_oop_iterate" that invoke the closures' do_oop methods
// non-virtually, using a mechanism defined in this file.  Extend these
// macros in the obvious way to add specializations for new closures.

// Forward declarations.
class OopClosure;
class OopsInGenClosure;
// DefNew
class ScanClosure;
class FastScanClosure;
class FilteringClosure;
// ParNew
class ParScanWithBarrierClosure;
class ParScanWithoutBarrierClosure;
//CMS
class MarkRefsIntoAndScanClosure;
class PushAndMarkClosure;
class Par_MarkRefsIntoAndScanClosure;
class Par_PushAndMarkClosure;
class CMSKeepAliveClosure;
class CMSInnerParMarkAndPushClosure;
// Train
class TrainScanClosure;
class UpdateTrainRSWrapOopsInGenClosure;
class UpdateTrainRSWrapScanClosure;
class UpdateTrainRSWrapFastScanClosure;
class UpdateTrainRSWrapTrainScanClosure;
class UpdateTrainRSWrapParScanWithBarrierClosure;
class UpdateTrainRSWrapParScanWithoutBarrierClosure;

// This macro applies an argument macro to all OopClosures for which we
// want specialized bodies of "oop_oop_iterate".  The arguments to "f" are:
//   "f(closureType, non_virtual)"
// where "closureType" is the name of the particular subclass of OopClosure,
// and "non_virtual" will be the string "_nv" if the closure type should
// have its "do_oop" method invoked non-virtually, or else the
// string "_v".  ("OopClosure" itself will be the only class in the latter
// category.)

// This is split into several because of a Visual C++ 6.0 compiler bug
// where very long macros cause the compiler to crash

#define SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_1(f)       \
  f(ScanClosure,_nv)                                    \
  f(FastScanClosure,_nv)                                \
  f(FilteringClosure,_nv)                               \
  f(ParScanWithBarrierClosure,_nv)	        	\
  f(ParScanWithoutBarrierClosure,_nv)

#define SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_2(f)       \
  f(TrainScanClosure,_nv)                               \
  f(UpdateTrainRSWrapScanClosure,_nv)                   \
  f(UpdateTrainRSWrapFastScanClosure,_nv)               \
  f(UpdateTrainRSWrapTrainScanClosure,_nv)

#define SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_3(f)       \
  f(MarkRefsIntoAndScanClosure,_nv)			\
  f(PushAndMarkClosure,_nv)				\
  f(Par_MarkRefsIntoAndScanClosure,_nv)			\
  f(Par_PushAndMarkClosure,_nv)                         \
  f(CMSKeepAliveClosure,_nv)                            \
  f(CMSInnerParMarkAndPushClosure,_nv)

// We separate these out, because sometime the general one has
// a different definition from the specialized ones, and sometimes it
// doesn't.
#define ALL_OOP_OOP_ITERATE_CLOSURES_1(f)               \
  f(OopClosure,_v)                                      \
  SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_1(f)

#define ALL_OOP_OOP_ITERATE_CLOSURES_2(f)               \
  SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_2(f)

#define ALL_OOP_OOP_ITERATE_CLOSURES_3(f)               \
  SPECIALIZED_OOP_OOP_ITERATE_CLOSURES_3(f)


// This macro applies an argument macro to all OopClosures for which we
// want specialized bodies of a family of methods related to
// "par_oop_iterate".  The arguments to f are the same as above.
// The "root_class" is the most general class to define; this may be
// "OopClosure" in some applications and "OopsInGenClosure" in others.
#define SPECIALIZED_PAR_OOP_ITERATE_CLOSURES(f)        \
  f(MarkRefsIntoAndScanClosure,_nv)                    \
  f(PushAndMarkClosure,_nv)                            \
  f(Par_MarkRefsIntoAndScanClosure,_nv)                \
  f(Par_PushAndMarkClosure,_nv)

#define ALL_PAR_OOP_ITERATE_CLOSURES(f)                \
  f(OopClosure,_v)                                     \
  SPECIALIZED_PAR_OOP_ITERATE_CLOSURES(f)


// This macro applies an argument macro to all OopClosures for which we
// want specialized bodies of a family of methods related to
// "oops_since_save_marks_do".  The arguments to f are the same as above.
// The "root_class" is the most general class to define; this may be
// "OopClosure" in some applications and "OopsInGenClosure" in others.


#define SPECIALIZED_SINCE_SAVE_MARKS_CLOSURES_YOUNG(f)	\
  f(ScanClosure,_nv)					\
  f(FastScanClosure,_nv)				\
  f(ParScanWithBarrierClosure,_nv)			\
  f(ParScanWithoutBarrierClosure,_nv)

#define SPECIALIZED_SINCE_SAVE_MARKS_CLOSURES_TRAIN(f)  \
  f(TrainScanClosure,_nv)                               \
  f(UpdateTrainRSWrapScanClosure,_nv)                   \
  f(UpdateTrainRSWrapFastScanClosure,_nv)               \
  f(UpdateTrainRSWrapTrainScanClosure,_nv)              \
  f(UpdateTrainRSWrapParScanWithBarrierClosure,_nv)     \
  f(UpdateTrainRSWrapParScanWithoutBarrierClosure,_nv)

#define SPECIALIZED_SINCE_SAVE_MARKS_CLOSURES(f)        \
  SPECIALIZED_SINCE_SAVE_MARKS_CLOSURES_YOUNG(f)        \
  SPECIALIZED_SINCE_SAVE_MARKS_CLOSURES_TRAIN(f)

// We separate these out, because sometime the general one has
// a different definition from the specialized ones, and sometimes it
// doesn't.
// NOTE:   One of the valid criticisms of this
// specialize-oop_oop_iterate-for-specific-closures idiom is that it is
// easy to have a silent performance bug: if you fail to de-virtualize,
// things still work, just slower.  The "SpecializationStats" mode is
// intended to at least make such a failure easy to detect.
// *Not* using the ALL_SINCE_SAVE_MARKS_CLOSURES(f) macro defined
// below means that *only* closures for which oop_oop_iterate specializations
// exist above may be applied to "oops_since_save_marks".  That is,
// this form of the performance bug is caught statically.  When you add
// a definition for the general type, this property goes away.
// Make sure you test with SpecializationStats to find such bugs
// when introducing a new closure where you don't want virtual dispatch.

#define ALL_SINCE_SAVE_MARKS_CLOSURES(f)                \
  f(OopsInGenClosure,_v)                                \
  SPECIALIZED_SINCE_SAVE_MARKS_CLOSURES(f)

// The train requires some closures to occur in pairs.  In this macro, the
// argument macro is passed the second closure type as a third argument

#define TRAIN_SPECIALIZED_SINCE_SAVE_MARKS_CLOSURE_PAIRS(f)     \
  f(OopsInGenClosure,_v,UpdateTrainRSWrapOopsInGenClosure)      \
  f(ScanClosure,_nv,UpdateTrainRSWrapScanClosure)               \
  f(FastScanClosure,_nv,UpdateTrainRSWrapFastScanClosure)       \
  f(TrainScanClosure,_nv,UpdateTrainRSWrapTrainScanClosure)     \
  f(ParScanWithBarrierClosure,_nv,				\
    UpdateTrainRSWrapParScanWithBarrierClosure)			\
  f(ParScanWithoutBarrierClosure,_nv,				\
    UpdateTrainRSWrapParScanWithoutBarrierClosure)

#define TRAIN_EXCLUDE_SINCE_SAVE_MARKS_CLOSURES(f)              \
  f(UpdateTrainRSWrapScanClosure,_nv)                           \
  f(UpdateTrainRSWrapFastScanClosure,_nv)                       \
  f(UpdateTrainRSWrapTrainScanClosure,_nv)                      \
  f(UpdateTrainRSWrapParScanWithBarrierClosure,_nv)             \
  f(UpdateTrainRSWrapParScanWithoutBarrierClosure,_nv)

// For keeping stats on effectiveness.
#define ENABLE_SPECIALIZATION_STATS 0


class SpecializationStats {
public:
  enum Kind {
    ik,             // instanceKlass
    irk,            // instanceRefKlass
    oa,             // objArrayKlass
    NUM_Kinds
  };

#if ENABLE_SPECIALIZATION_STATS
private:
  static int _numCallsAll;

  static int _numCallsTotal[NUM_Kinds];
  static int _numCalls_nv[NUM_Kinds];

  static int _numDoOopCallsTotal[NUM_Kinds];
  static int _numDoOopCalls_nv[NUM_Kinds];
public:
#endif
  static void clear()  PRODUCT_RETURN;

  static inline void record_call()  PRODUCT_RETURN;
  static inline void record_iterate_call_v(Kind k)  PRODUCT_RETURN;
  static inline void record_iterate_call_nv(Kind k)  PRODUCT_RETURN;
  static inline void record_do_oop_call_v(Kind k)  PRODUCT_RETURN;
  static inline void record_do_oop_call_nv(Kind k)  PRODUCT_RETURN;

  static void print() PRODUCT_RETURN;
};  

#ifndef PRODUCT
#if ENABLE_SPECIALIZATION_STATS

inline void SpecializationStats::record_call() {
  _numCallsAll++;;
}
inline void SpecializationStats::record_iterate_call_v(Kind k) {
  _numCallsTotal[k]++;
}
inline void SpecializationStats::record_iterate_call_nv(Kind k) {
  _numCallsTotal[k]++;
  _numCalls_nv[k]++;
}

inline void SpecializationStats::record_do_oop_call_v(Kind k) {
  _numDoOopCallsTotal[k]++;
}
inline void SpecializationStats::record_do_oop_call_nv(Kind k) {
  _numDoOopCallsTotal[k]++;
  _numDoOopCalls_nv[k]++;
}

#else   // !ENABLE_SPECIALIZATION_STATS

inline void SpecializationStats::record_call() {}
inline void SpecializationStats::record_iterate_call_v(Kind k) {}
inline void SpecializationStats::record_iterate_call_nv(Kind k) {}
inline void SpecializationStats::record_do_oop_call_v(Kind k) {}
inline void SpecializationStats::record_do_oop_call_nv(Kind k) {}
inline void SpecializationStats::clear() {}
inline void SpecializationStats::print() {}

#endif  // ENABLE_SPECIALIZATION_STATS
#endif  // !PRODUCT
