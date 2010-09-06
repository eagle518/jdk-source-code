#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)referenceProcessorMT.hpp	1.4 03/12/23 16:41:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ReferenceProcessorMT class is a subclass of ReferenceProcessor,
// providing multi-threaded reference processing capabilities.
// It is currently only used by the CMS collector. The plan is
// to allow ParNewGC to also use this in the near future.
// For ParallelGC, a parallel (sibling) class will be needed.
//
// The basic idea here is that each of several threads may
// both discover references in parallel as well as process
// the discovered references. The parallel discovery capability
// already exists in the basic ReferenceProcessor class currently;
// this class augments that with parallel processing and enqueueing
// capabilities by (essentially) providing alternate implementations
// of the following virtual methods:
// 
//

// fwd decl
class AbstractRefProcTask;

class ReferenceProcessorMT: public ReferenceProcessor {
  bool  _processing_is_mt; // true during phases when
                           // reference processing is MT.
  int   _next_id;          // round-robin counter in
                           // support of work distribution

 protected:
  friend class ReferenceProcessorInitializer;
  friend class ReferenceProcessorSerial;
  friend class ReferenceProcessorParallel;

  AbstractRefProcTask*       _par_task;

 public:
  // Override
  virtual void process_discovered_reflist(oop* refs_list_addr,
                                  ReferencePolicy *policy,
                                  bool clear_referent);

  // Override XXX this may be avoidable? FIX ME !!!
  virtual void process_phaseJNI();

  // "Preclean" the given discovered reference list
  // by removing references with strongly reachable referents.
  // Currently used in support of CMS only.
  void preclean_discovered_reflist(oop* refs_list_addr,
                                   BoolObjectClosure* is_alive,
                                   YieldClosure*      yield);

  virtual void enqueue_discovered_reflist(oop refs_list,
                                  oop* pending_list_addr);
 protected:
  // Override with MT implementation
  virtual oop* get_discovered_list(ReferenceType rt);
  virtual void add_to_discovered_list_mt(oop* list, oop obj,
                                         oop* discovered_addr);

 private:
  virtual void enqueue_discovered_reflists(oop* pending_list_addr);

  int  next_id() {
    int id = _next_id;
    if (++_next_id == _num_q) {
      _next_id = 0;
    }
    return id;
  }

 public:
  // constructor
  ReferenceProcessorMT():
    ReferenceProcessor(), 
    _processing_is_mt(false),
    _par_task(NULL),
    _next_id(0)
  {}

  ReferenceProcessorMT(MemRegion span, bool atomic_discovery,
                       bool mt_discovery, int mt_degree);

  // Whether we are in a phase when _processing_ is MT.
  bool processing_is_mt() const { return _processing_is_mt; }
  void set_mt_processing(bool mt) { _processing_is_mt = mt; }

 public:
  // "Preclean" all the discovered reference lists
  // by removing references with strongly reachable referents.
  // The first argument is a predicate on an oop that indicates
  // its (strong) reachability and the second is a closure that
  // may be used to incrementalize or abort the precleaning process.
  // The caller is responsible for taking care of potential
  // interference with concurrent operations on these lists
  // (or predicates involved) by other threads. Currently
  // only used by the CMS collector.
  void preclean_discovered_references(BoolObjectClosure* is_alive,
                                      YieldClosure*      yield);
};

// A utility class used to "seed" the given ReferenceProcessorMT
// instance with appropriate "input" values, and used subsequerntly
// for reference prcoessing via a call to
// ReferenceProcessor::process_discovered_references().
class ReferenceProcessorParallel: public ReferenceProcessorInitializer {
 protected:
  virtual void is_clean() const {
    ReferenceProcessorInitializer::is_clean();
    assert(((ReferenceProcessorMT*)_rp)->_par_task == NULL, "dirty decks");
  }
 public:
  ReferenceProcessorParallel(ReferenceProcessorMT*  rp,
                             AbstractRefProcTask* par_task):
    ReferenceProcessorInitializer(rp) {
    assert(rp->processing_is_mt(), "Use ReferenceProcessorSerial");
    rp->_par_task = par_task;
  }
  ~ReferenceProcessorParallel() {
    ((ReferenceProcessorMT*)_rp)->_par_task = NULL;
  }
};

// A utility class to temporarily change the MT processing
// disposition of the given ReferenceProcessorMT instance
// in the scope that contains it.
class ReferenceProcessorMTProcMutator: StackObj {
 private:
  ReferenceProcessorMT* _rp;
  bool  _saved_mt;

 public:
  ReferenceProcessorMTProcMutator(ReferenceProcessorMT* rp,
                                 bool  mt):
    _rp(rp) {
    _saved_mt = _rp->processing_is_mt();
    _rp->set_mt_processing(mt);
  }

  ~ReferenceProcessorMTProcMutator() {
    _rp->set_mt_processing(_saved_mt);
  }
};

////////////////////////////////////////////////////
// Parallel Reference Processing Task
////////////////////////////////////////////////////////
class AbstractRefProcTask: public AbstractGangTask {
 public:
  enum RefProcPhase {
    Phase1,
    Phase2,
    Phase3,
    PhaseJNI
  };

 protected:
  ReferenceProcessorMT*  _rp;
  int                  _n_workers;
  WorkGang*            _workers;
  ReferencePolicy*     _policy;
  oop*                 _ref_list;
  bool                 _clear_ref;
  RefProcPhase         _phase;

 public:
  AbstractRefProcTask(char* name,
                      ReferenceProcessorMT* rp,
                      int                 n_workers,
                      WorkGang*           workers):
    AbstractGangTask(name),
    _rp(rp), _n_workers(n_workers), _workers(workers) {
    assert(_rp->num_q() == _n_workers, "worker/queue mismatch");
    assert(_n_workers == 1 || _workers != NULL, "no workers?");
  }
  virtual void reset() = 0;
  void set_phase(RefProcPhase phase)  { _phase = phase; }
  void set_policy(ReferencePolicy* p) { _policy = p; }
  void set_ref_list(oop* list)        { _ref_list = list; }
  void set_clear_ref(bool clear)      { _clear_ref = clear; }
};
