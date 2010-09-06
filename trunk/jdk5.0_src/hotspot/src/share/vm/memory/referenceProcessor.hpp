#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)referenceProcessor.hpp	1.27 04/01/13 12:43:11 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ReferenceProcessor class encapsulates the per-"collector" processing
// of "weak" references for GC. The interface is useful for supporting
// a generational abstraction, in particular when there are multiple
// generations that are being independently collected -- possibly
// concurrently and/or incrementally.  Note, however, that the
// ReferenceProcessor class abstracts away from a generational setting
// by using only a heap interval (called "span" below), thus allowing
// its use in a straightforward manner in a general, non-generational
// setting.
//
// The basic idea is that each ReferenceProcessor object concerns
// itself with ("weak") reference processing in a specific "span"
// of the heap of interest to a specific collector. Currently,
// the span is a convex interval of the heap, but, efficiency
// apart, there seems to be no reason it couldn't be extended
// (with appropriate modifications) to any "non-convex interval".

// forward references
class ReferencePolicy;

class ReferenceProcessor : public CHeapObj {
 protected:
  // End of list marker
  static oop  _sentinelRef;
  MemRegion   _span; // (right-open) interval of heap
                     // subject to wkref discovery
  bool        _discovering_refs;      // true when discovery enabled
  bool        _discovery_is_atomic;   // if discovery is atomic wrt
				      // other collectors in configuration
  bool        _discovery_is_mt;       // true if reference discovery is MT.
  bool        _notify_ref_lock;       // used only when !_discovery_is_atomic
  bool	      _enqueuing_is_done;     // true if all weak references enqueued

  // For collectors that do not keep GC marking information
  // in the object header, this field holds a closure that
  // helps the reference processor determine the reachability
  // of an oop (the field is currently initialized to NULL for
  // all collectors but the CMS collector).
  BoolObjectClosure* _is_alive_non_header;

  // The discovered ref lists themselves
  int        _num_q;                // the MT'ness degree of the queues below
  oop*       _discoveredSoftRefs;   // pointer to array of oops
  oop*       _discoveredWeakRefs;
  oop*       _discoveredFinalRefs;
  oop*       _discoveredPhantomRefs;

 public:
  int  num_q()                { return _num_q; }
  oop* discovered_soft_refs() { return _discoveredSoftRefs; }
  oop* sentinel_ref()         { return &_sentinelRef; }

 protected:
  friend class ReferenceProcessorInitializer;
  friend class ReferenceProcessorSerial;
  friend class ReferenceProcessorParallel;

  ReferencePolicy*   _policy;
  BoolObjectClosure* _is_alive;
  OopClosure*        _keep_alive;
  VoidClosure*       _complete_gc;

 public:
  // Process references with a certain reachability level.
  virtual void process_discovered_reflist(oop* refs_list_addr,
                                  ReferencePolicy *policy,
                                  bool clear_referent);
  virtual void process_phaseJNI();

  // Work methods used by the method process_discovered_reflist
  // Phase1: keep alive all those referents that are otherwise
  // dead but which must be kept alive by policy (and their closure).
  void process_phase1(oop* refs_list_addr,
                      ReferencePolicy *policy,
                      BoolObjectClosure* is_alive,
                      OopClosure* keep_alive,
                      VoidClosure* complete_gc);
  // Phase2: remove all those references whose referents are
  // reachable.
  void process_phase2(oop* refs_list_addr,
                      BoolObjectClosure* is_alive,
                      OopClosure* keep_alive);
  // Phase3: process the referents by either clearing them
  // or keeping them alive (and their closure)
  void process_phase3(oop* refs_list_addr,
                      bool clear_referent,
                      BoolObjectClosure* is_alive,
                      OopClosure* keep_alive,
                      VoidClosure* complete_gc);

  // Enqueue references with a certain reachability level
  virtual void enqueue_discovered_reflist(oop refs_list,
                                  oop* pending_list_addr);

  // Delete entries in the discovered lists that have
  // a null referent.  An entry with a null referent can
  // result if the discovered list was maintained during
  // concurrent marking.
  // At present concurrent marking only occurs if the
  // CMS collector is in use and these are only expected
  // to be used by the CMS collector (although they
  // are safe for use elsewhere).
  void delete_null_referents_from_lists();
  void delete_null_referents(oop* refs_list_addr);

  // Returns the name of the discovered reference list 
  // occupying the i / _num_q slot.
  const char* ReferenceProcessor::list_name(int i);

 protected:
  virtual void enqueue_discovered_reflists(oop* pending_list_addr);
  virtual oop* get_discovered_list(ReferenceType rt);
  virtual void add_to_discovered_list_mt(oop* list, oop obj,
                                         oop* discovered_addr);

  void verify_ok_to_handle_reflists() PRODUCT_RETURN;

 public:
  // constructor
  ReferenceProcessor():
    _span((HeapWord*)NULL, (HeapWord*)NULL),
    _discoveredSoftRefs(NULL),  _discoveredWeakRefs(NULL),
    _discoveredFinalRefs(NULL), _discoveredPhantomRefs(NULL),
    _discovering_refs(false),
    _discovery_is_atomic(true),
    _enqueuing_is_done(false),
    _discovery_is_mt(false),
    _notify_ref_lock(false),
    _is_alive_non_header(NULL),
    _num_q(0),
    _policy(NULL),
    _is_alive(NULL), _keep_alive(NULL), _complete_gc(NULL)
  {}

  ReferenceProcessor(MemRegion span, bool atomic_discovery,
                     bool mt_discovery, int mt_degree = 1);

  // RefDiscoveryPolicy values
  enum {
    ReferenceBasedDiscovery = 0,
    ReferentBasedDiscovery  = 1
  };

  static void init_statics();

 public:
  // get and set "is_alive_non_header" field
  BoolObjectClosure* is_alive_non_header() {
    return _is_alive_non_header;
  }
  void set_is_alive_non_header(BoolObjectClosure* is_alive_non_header) {
    _is_alive_non_header = is_alive_non_header;
  }

  // get and set span
  MemRegion span()                   { return _span; }
  void      set_span(MemRegion span) { _span = span; }

  // start and stop weak ref discovery
  void enable_discovery()   { _discovering_refs = true;  }
  void disable_discovery()  { _discovering_refs = false; }
  bool discovery_enabled()  { return _discovering_refs;  }

  // whether discovery is atomic wrt other collectors
  bool discovery_is_atomic() const { return _discovery_is_atomic; }
  // whether discovery is done by multiple threads same-old-timeously
  bool discovery_is_mt() const { return _discovery_is_mt; }
  void set_mt_discovery(bool mt) { _discovery_is_mt = mt; }

  // whether all enqueuing of weak references is complete
  bool enqueuing_is_done()  { return _enqueuing_is_done; }
  void set_enqueuing_is_done(bool v) { _enqueuing_is_done = v; }

  // In the case of non-atomic discovery we need a way of
  // exchanging info regarding notification on the ref lock (PLL).
  bool notify_ref_lock()           { return _notify_ref_lock; }
  bool read_and_reset_notify_ref_lock() {
    assert(!discovery_is_atomic(), "Else why use this method?");
    if (_notify_ref_lock) {
      _notify_ref_lock = false;
      return true;
    }
    return false;
  }
  void set_notify_ref_lock(bool b) { _notify_ref_lock = b; }

  // iterate over oops
  void oops_do(OopClosure* f);
  static void oops_do_statics(OopClosure* f);

  // Discover a Reference object, using appropriate discovery criteria
  bool discover_reference(oop obj, ReferenceType rt);

 protected:
  friend class ReferenceProcessorInitializer;
  // Process references found during GC (called by the garbage collector)
  void process_discovered_references();

 public:
  // Enqueue references at end of GC (called by the garbage collector)
  bool enqueue_discovered_references();

  // debugging
  void verify_no_references_recorded() PRODUCT_RETURN;
};

// A utility class used to "seed" the given ReferenceProcessor
// instance with appropriate "input" values, and used subsequerntly
// for reference prcoessing via a call to
// ReferenceProcessor::process_discovered_references().
class ReferenceProcessorInitializer: public StackObj {
 protected: 
  ReferenceProcessor* _rp;
  virtual void is_clean() const {
    assert(_rp != NULL, "Need non-NULL reference processor");
    assert(_rp->_policy == NULL, "dirty decks");
    assert(_rp->_is_alive == NULL, "dirty decks");
    assert(_rp->_keep_alive == NULL, "dirty decks");
    assert(_rp->_complete_gc == NULL, "dirty decks");
  }
 public:
  ReferenceProcessorInitializer(ReferenceProcessor* rp):
    _rp(rp) {
    is_clean();   // check that we are not clobbering anything useful
  }
  ~ReferenceProcessorInitializer() {
    is_clean();   // check that we have cleaned up after ourselves
  }
  void process_discovered_references() {
    _rp->process_discovered_references();
  }
};

class ReferenceProcessorSerial: public ReferenceProcessorInitializer {
 public:
  ReferenceProcessorSerial(ReferenceProcessor* rp,
                           ReferencePolicy* policy,
                           BoolObjectClosure* is_alive,
                           OopClosure*        keep_alive,
                           VoidClosure*       complete_gc):
    ReferenceProcessorInitializer(rp) {
    _rp->_policy = policy;
    _rp->_is_alive = is_alive;
    _rp->_keep_alive = keep_alive;
    _rp->_complete_gc = complete_gc;
  }
  ~ReferenceProcessorSerial() {
    _rp->_policy = NULL;
    _rp->_is_alive = NULL;
    _rp->_keep_alive = NULL;
    _rp->_complete_gc = NULL;
  }
};

// A utility class to disable reference discovery in
// the scope which contains it, for given ReferenceProcessor.
class NoRefDiscovery: StackObj {
 private:
  ReferenceProcessor* _rp;
  bool _was_discovering_refs;
 public:
  NoRefDiscovery(ReferenceProcessor* rp) : _rp(rp) {
    if (_was_discovering_refs = _rp->discovery_enabled()) {
      _rp->disable_discovery();
    }
  }

  ~NoRefDiscovery() {
    if (_was_discovering_refs) {
      _rp->enable_discovery();
    }
  }
};


// A utility class to temporarily mutate the span of the
// given ReferenceProcessor in the scope that contains it.
class ReferenceProcessorSpanMutator: StackObj {
 private:
  ReferenceProcessor* _rp;
  MemRegion           _saved_span;

 public:
  ReferenceProcessorSpanMutator(ReferenceProcessor* rp,
                                MemRegion span):
    _rp(rp) {
    _saved_span = _rp->span();
    _rp->set_span(span);
  }

  ~ReferenceProcessorSpanMutator() {
    _rp->set_span(_saved_span);
  }
};

// A utility class to temporarily change the MT'ness of
// reference discovery for the given ReferenceProcessor
// in the scope that contains it.
class ReferenceProcessorMTMutator: StackObj {
 private:
  ReferenceProcessor* _rp;
  bool                _saved_mt;

 public:
  ReferenceProcessorMTMutator(ReferenceProcessor* rp,
                              bool mt):
    _rp(rp) {
    _saved_mt = _rp->discovery_is_mt();
    _rp->set_mt_discovery(mt);
  }

  ~ReferenceProcessorMTMutator() {
    _rp->set_mt_discovery(_saved_mt);
  }
};


// A utility class to temporarily change the disposition
// of the "is_alive_non_header" closure field of the
// given ReferenceProcessor in the scope that contains it.
class ReferenceProcessorIsAliveMutator: StackObj {
 private:
  ReferenceProcessor* _rp;
  BoolObjectClosure*  _saved_cl;

 public:
  ReferenceProcessorIsAliveMutator(ReferenceProcessor* rp,
                                   BoolObjectClosure*  cl):
    _rp(rp) {
    _saved_cl = _rp->is_alive_non_header();
    _rp->set_is_alive_non_header(cl);
  }

  ~ReferenceProcessorIsAliveMutator() {
    _rp->set_is_alive_non_header(_saved_cl);
  }
};
