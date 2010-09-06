#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)genOopClosures.hpp	1.46 03/12/23 16:41:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Generation;
class HeapWord;
class CardTableRS;
class CardTableModRefBS;
class DefNewGeneration;
class TrainGeneration;

// Closure for iterating roots from a particular generation
// Note: all classes deriving from this MUST call this do_barrier
// method at the end of their own do_oop method!
// Note: no do_oop defined, this is an abstract class.

class OopsInGenClosure : public OopClosure {
 private:
  Generation*         _orig_gen;     // generation originally set in ctor
  Generation*         _gen;          // generation being scanned

 protected:
  // Some subtypes need access.
  HeapWord*           _gen_boundary; // start of generation
  CardTableRS*        _rs;           // remembered set

  // For assertions
  Generation* generation() { return _gen; }
  CardTableRS* rs() { return _rs; }
 
  // Derived classes that modify oops so that they might be old-to-young
  // pointers must call the method below.
  inline void do_barrier(oop* p);

 public:
  OopsInGenClosure() : OopClosure(NULL),
    _orig_gen(NULL), _gen(NULL), _gen_boundary(NULL), _rs(NULL) {};

  inline OopsInGenClosure(Generation* gen);
  inline void set_generation(Generation* gen);

  void reset_generation() { _gen = _orig_gen; }

  // Problem with static closures: must have _gen_boundary set at some point,
  // but cannot do this until after the heap is initialized.
  void set_orig_generation(Generation* gen) { 
    _orig_gen = gen; 
    set_generation(gen);
  }

  HeapWord* gen_boundary() { return _gen_boundary; }
};

// Closure for scanning DefNewGeneration.
//
// This closure will perform barrier store calls for ALL
// pointers in scanned oops.
class ScanClosure: public OopsInGenClosure {
protected:
  DefNewGeneration* _g;
  HeapWord* _boundary;
  bool _gc_barrier;
public:
  ScanClosure(DefNewGeneration* g, bool gc_barrier);
  inline void do_oop(oop* p);
  inline void do_oop_nv(oop* p);
  bool do_header() { return false; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_write;
  } 
};

// Closure for scanning DefNewGeneration.
//
// This closure only performs barrier store calls on
// pointers into the DefNewGeneration. This is less
// precise, but faster, than a ScanClosure
class FastScanClosure: public OopsInGenClosure {
protected:
  DefNewGeneration* _g;
  HeapWord* _boundary;
  bool _gc_barrier;
public:
  FastScanClosure(DefNewGeneration* g, bool gc_barrier);
  inline void do_oop(oop* p);
  inline void do_oop_nv(oop* p);
  bool do_header() { return false; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_write;
  } 
};

class FilteringClosure: public OopClosure {
  HeapWord* _boundary;
  OopClosure* _cl;
public:
  FilteringClosure(HeapWord* boundary, OopClosure* cl) :
    OopClosure(cl->_ref_processor), _boundary(boundary),
    _cl(cl) {}
  void do_oop(oop* p);
  void do_oop_nv(oop* p) {
    oop obj = *p;
    if ((HeapWord*)obj < _boundary && obj != NULL) {
      _cl->do_oop(p);
    }
  }
  bool do_header() { return false; }
};

// Closure for scanning DefNewGeneration's weak references.
// NOTE: very much like ScanClosure but not derived from
//  OopsInGenClosure -- weak references are processed all
//  at once, with no notion of which generation they were in.
class ScanWeakRefClosure: public OopClosure {
protected:
  DefNewGeneration*  _g;
  HeapWord*          _boundary;
public:
  ScanWeakRefClosure(DefNewGeneration* g);
  inline void do_oop(oop* p);
  inline void do_oop_nv(oop* p);
};

// Closures for train generation
class TrainScanClosure: public OopsInGenClosure {
private:
  TrainGeneration*   _tg;
  bool&              _ref_to_first_train_found;
  bool               _gc_barrier;   // gc barrier test required
  bool               _rs_update;    // train RS update required
public:
  TrainScanClosure(TrainGeneration* tg, bool gc_barrier, bool rs_update,
                   bool& ref_to_first_train_found);
  inline void do_oop(oop* p);
  inline void do_oop_nv(oop* p);
  bool do_header() { return false; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_write;
  } 
};

class TrainScanWeakRefClosure: public OopClosure {
private:
  TrainGeneration* _tg;
  bool&            _ref_to_first_train_found;
public:
  TrainScanWeakRefClosure(TrainGeneration* tg, bool& ref_to_first_train_found);
  inline void do_oop(oop* p);
  inline void do_oop_nv(oop* p);
};

class UpdateTrainRSCacheFromClosure : public OopsInGenClosure {
private:
  TrainGeneration*  _tg;
  OopsInGenClosure* _cl;
  julong            _from_train;
  juint             _from_car;
public:
  UpdateTrainRSCacheFromClosure(TrainGeneration *tg, OopsInGenClosure* cl);
  UpdateTrainRSCacheFromClosure(TrainGeneration *tg);

  void set_train_number(julong train) { _from_train = train; }
  void set_car_number(juint car)      { _from_car   = car;   }

  void do_oop_nv(oop* p) { UpdateTrainRSCacheFromClosure::do_oop(p); }
  inline void do_oop(oop* p);
  bool do_header() { return false; }
};

class ParScanThreadState;
class ParNewGeneration;

class ParScanClosure: public OopsInGenClosure {
protected:
  ParScanThreadState* _par_scan_state;
  ParNewGeneration* _g;
  HeapWord* _boundary;
  inline void do_oop_work(oop* p,
			  bool gc_barrier, bool only_two_gens,
			  bool root_scan,
			  bool jvmpi_slow_alloc);

  inline void par_do_barrier(oop* p);

public:
  ParScanClosure(ParNewGeneration* g, ParScanThreadState* par_scan_state);
};

class ParScanWithBarrierClosure: public ParScanClosure {
public:
  void do_oop(oop* p)    { do_oop_work(p, true, true, false, false); }
  void do_oop_nv(oop* p) { do_oop_work(p, true, true, false, false); }
  ParScanWithBarrierClosure(ParNewGeneration* g,
			    ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
};

class ParScanWithoutBarrierClosure: public ParScanClosure {
public:
  ParScanWithoutBarrierClosure(ParNewGeneration* g,
			       ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p)    { do_oop_work(p, false, true, false, false); }
  void do_oop_nv(oop* p) { do_oop_work(p, false, true, false, false); }
};

#define UpdateTrainRSClosure_DECL(subclss, nv_suffix, clss)	\
class clss : public OopsInGenClosure {				\
private:							\
  TrainGeneration*  _tg;					\
  subclss* _cl;							\
public:								\
  clss(TrainGeneration *tg, subclss* cl) :			\
    _tg(tg), _cl(cl) { }					\
								\
  void do_oop_nv(oop* p) { clss::do_oop(p); }			\
  inline void do_oop(oop* p);					\
  bool do_header() { return false; }				\
  Prefetch::style prefetch_style() {                            \
    return Prefetch::do_write;                                  \
  }								\
};

TRAIN_SPECIALIZED_SINCE_SAVE_MARKS_CLOSURE_PAIRS(UpdateTrainRSClosure_DECL)

#ifndef PRODUCT

class VerifyOopClosure: public OopClosure {
public: 
  void do_oop(oop* p) {
    guarantee((*p)->is_oop_or_null(), "invalid oop");
  }
  static VerifyOopClosure verify_oop;
};

#endif


/////////////////////////////////////////////////////////////////
// Closures used by ConcurrentMarkSweepGeneration's collector
/////////////////////////////////////////////////////////////////
class ConcurrentMarkSweepGeneration;
class CMSBitMap;
class CMSMarkStack;
class CMSCollector;
class OopTaskQueue;
class OopTaskQueueSet;

class MarkRefsIntoClosure: public OopsInGenClosure {
  const MemRegion    _span;
  CMSBitMap*         _bitMap;
  const bool         _should_do_nmethods;
 public:
  MarkRefsIntoClosure(MemRegion span, CMSBitMap* bitMap,
                      bool should_do_nmethods);
  void do_oop(oop* p);
  inline void do_oop_nv(oop* p) { MarkRefsIntoClosure::do_oop(p); }
  bool do_header() { return true; }
  virtual const bool do_nmethods() const {
    return _should_do_nmethods;
  }
  Prefetch::style prefetch_style() {
    return Prefetch::do_read;
  }
};

// The non-parallel version (the parallel version appears further below).
class PushAndMarkClosure: public OopClosure {
  CMSCollector*    _collector;
  MemRegion        _span;
  CMSBitMap*       _bit_map;
  CMSBitMap*       _mod_union_table;
  CMSMarkStack*    _mark_stack;
  CMSMarkStack*    _revisit_stack;
  bool             _concurrent_precleaning;
 public:
  PushAndMarkClosure(CMSCollector* collector,
                     MemRegion span,
                     ReferenceProcessor* rp,
                     CMSBitMap* bit_map,
                     CMSBitMap* mod_union_table,
                     CMSMarkStack*  mark_stack,
                     CMSMarkStack*  revisit_stack,
                     bool           concurrent_precleaning):
    OopClosure(rp),
    _collector(collector),
    _span(span),
    _bit_map(bit_map),
    _mod_union_table(mod_union_table),
    _mark_stack(mark_stack),
    _revisit_stack(revisit_stack),
    _concurrent_precleaning(concurrent_precleaning) {
    assert(_ref_processor != NULL, "_ref_processor shouldn't be NULL");
  }

  void do_oop(oop* p);    
  inline void do_oop_nv(oop* p)  { PushAndMarkClosure::do_oop(p); }
  bool do_header() { return true; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_read;
  }
  const inline bool should_remember_klasses() const {
    return CMSClassUnloadingEnabled;
  }
  void remember_klass(Klass* k);
};

// In the parallel case, the revisit stack, the bit map and the
// reference processor are currently all shared. Access to
// these shared mutable structures must use appropriate
// synchronization (for instance, via CAS). The marking stack
// used in the non-parallel case above is here replaced with
// an OopTaskQueue structure to allow efficient work stealing.
class Par_PushAndMarkClosure: public OopClosure {
  CMSCollector*    _collector;
  MemRegion        _span;
  CMSBitMap*       _bit_map;
  OopTaskQueue*    _work_queue;
  CMSMarkStack*    _revisit_stack;
 public:
  Par_PushAndMarkClosure(CMSCollector* collector,
                         MemRegion span,
                         ReferenceProcessor* rp,
                         CMSBitMap* bit_map,
                         OopTaskQueue* work_queue,
                         CMSMarkStack* revisit_stack):
    OopClosure(rp),
    _collector(collector),
    _span(span),
    _bit_map(bit_map),
    _work_queue(work_queue),
    _revisit_stack(revisit_stack) {
    assert(_ref_processor != NULL, "_ref_processor shouldn't be NULL");
  }

  void do_oop(oop* p);
  inline void do_oop_nv(oop* p)  { Par_PushAndMarkClosure::do_oop(p); }
  bool do_header() { return true; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_read;
  }
  const inline bool should_remember_klasses() const {
    return CMSClassUnloadingEnabled;
  }
  void remember_klass(Klass* k);
};


// The non-parallel version (the parallel version appears further below).
class MarkRefsIntoAndScanClosure: public OopsInGenClosure {
  MemRegion                  _span;
  CMSBitMap*                 _bit_map;
  CMSMarkStack*              _mark_stack;
  PushAndMarkClosure         _pushAndMarkClosure;
  CMSCollector*              _collector;
  bool                       _yield;
  // Whether closure is being used for concurrent precleaning
  bool                       _concurrent_precleaning;
  Mutex*                     _freelistLock;
  oop*                       _olh;
 public:
  MarkRefsIntoAndScanClosure(MemRegion span,
                             ReferenceProcessor* rp,
                             CMSBitMap* bit_map,
                             CMSBitMap* mod_union_table,
                             CMSMarkStack*  mark_stack,
                             CMSMarkStack*  revisit_stack,
                             CMSCollector* collector,
                             bool should_yield,
                             bool concurrent_precleaning);
  void do_oop(oop* p);
  inline void do_oop_nv(oop* p) { MarkRefsIntoAndScanClosure::do_oop(p); }
  bool do_header() { return true; }
  virtual const bool do_nmethods() const { return true; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_read;
  }
  void set_freelistLock(Mutex* m) {
    _freelistLock = m;
  }

 private:
  inline void do_yield_check();
  void do_yield_work();
  bool take_from_overflow_list();
};

// Tn this, the parallel avatar of MarkRefsIntoAndScanClosure, the revisit
// stack and the bitMap are shared, so access needs to be suitably
// sycnhronized. An OopTaskQueue structure, supporting efficient
// workstealing, replaces a CMSMarkStack for storing grey objects.
class Par_MarkRefsIntoAndScanClosure: public OopsInGenClosure {
  MemRegion                      _span;
  CMSBitMap*                     _bit_map;
  OopTaskQueue*                  _work_queue;
  const uint                     _low_water_mark;
  Par_PushAndMarkClosure         _par_pushAndMarkClosure;
 public:
  Par_MarkRefsIntoAndScanClosure(CMSCollector* collector,
                                 MemRegion span,
                                 ReferenceProcessor* rp,
                                 CMSBitMap* bit_map,
                                 OopTaskQueue* work_queue,
                                 CMSMarkStack*  revisit_stack);
  void do_oop(oop* p);
  inline void do_oop_nv(oop* p) { Par_MarkRefsIntoAndScanClosure::do_oop(p); }
  bool do_header() { return true; }
  virtual const bool do_nmethods() const { return true; }
  Prefetch::style prefetch_style() {
    return Prefetch::do_read;
  }
  inline void trim_queue(uint size);
};

// For objects in CMS generation, this closure marks
// given objects (transitively) as being reachable/live.
// This is currently used during the (weak) reference object
// processing phase of the CMS final checkpoint step.
class CMSKeepAliveClosure: public OopClosure {
  CMSCollector* _collector;
  MemRegion     _span;
  CMSMarkStack* _mark_stack;
  CMSBitMap*    _bit_map;
 public:
  CMSKeepAliveClosure(CMSCollector* collector, MemRegion span,
                      CMSBitMap* bit_map, CMSMarkStack* mark_stack):
    _collector(collector),
    _span(span),
    _bit_map(bit_map),
    _mark_stack(mark_stack) { }

  void do_oop(oop* p);
  inline void do_oop_nv(oop* p) { CMSKeepAliveClosure::do_oop(p); }
};

class CMSInnerParMarkAndPushClosure: public OopClosure {
  CMSCollector* _collector;
  MemRegion     _span;
  OopTaskQueue* _work_queue;
  CMSBitMap*    _bit_map;
 public:
  CMSInnerParMarkAndPushClosure(CMSCollector* collector,
                                MemRegion span, CMSBitMap* bit_map,
                                OopTaskQueue* work_queue):
    _collector(collector),
    _span(span),
    _bit_map(bit_map),
    _work_queue(work_queue) { }
  void do_oop(oop* p);
  inline void do_oop_nv(oop* p) { CMSInnerParMarkAndPushClosure::do_oop(p); }
};

// A parallel (MT) version of the above, used when
// reference processing is parallel; the only difference
// is in the do_oop method.
class CMSParKeepAliveClosure: public OopClosure {
  CMSCollector* _collector;
  MemRegion     _span;
  OopTaskQueue* _work_queue;
  CMSBitMap*    _bit_map;
  CMSInnerParMarkAndPushClosure _mark_and_push;
  const uint    _low_water_mark;
  void trim_queue(uint max);
 public:
  CMSParKeepAliveClosure(CMSCollector* collector, MemRegion span,
                         CMSBitMap* bit_map, OopTaskQueue* work_queue);
  void do_oop(oop* p);
  inline void do_oop_nv(oop* p) { CMSParKeepAliveClosure::do_oop(p); }
};
