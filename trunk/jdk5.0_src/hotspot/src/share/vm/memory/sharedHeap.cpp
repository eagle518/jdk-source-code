#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)sharedHeap.cpp	1.44 04/03/28 16:18:06 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_sharedHeap.cpp.incl"

SharedHeap* SharedHeap::_sh;

SharedHeap::SharedHeap(CollectorPolicy* policy_) :
  CollectedHeap(),
  _collector_policy(policy_),
  _perm_gen(NULL), _rem_set(NULL),
  _strong_roots_parity(0),
  _workers(NULL), _n_par_threads(0)
{
  _sh = this;  // ch is static, should be set only once.
  if ((UseParNewGC ||
      (UseConcMarkSweepGC && CMSParallelRemarkEnabled)) &&
      ParallelGCThreads > 0) {
    _workers = new WorkGang("Parallel GC Threads", ParallelGCThreads, true);
    if (_workers == NULL) {
      vm_exit_during_initialization("Failed necessary allocation.");
    }
  }
}


void SharedHeap::set_par_threads(int t) {
  _n_par_threads = t;
}

class AssertIsPermClosure: public OopClosure {
public:
  void do_oop(oop* p) {
    assert((*p) == NULL || (*p)->is_perm(), "Referent should be perm.");
  }
};
static AssertIsPermClosure assert_is_perm_closure;

void SharedHeap::change_strong_roots_parity() {
  // Also set the new collection parity.
  assert(_strong_roots_parity >= 0 && _strong_roots_parity <= 2,
	 "Not in range.");
  _strong_roots_parity++;
  if (_strong_roots_parity == 3) _strong_roots_parity = 1;
  assert(_strong_roots_parity >= 1 && _strong_roots_parity <= 2,
	 "Not in range.");
}

void SharedHeap::set_barrier_set(BarrierSet* bs) {
  _barrier_set = bs;
  // Cached barrier set for fast access in oops
  oopDesc::set_bs(bs);
}

void SharedHeap::post_initialize() {
  ref_processing_init();
}

void SharedHeap::ref_processing_init() {
  perm_gen()->ref_processor_init();
}

class JVMPIAllocEventDisabler: public StackObj {
  bool _enabled;
public:
  JVMPIAllocEventDisabler() {
    _enabled = Universe::jvmpi_alloc_event_enabled();
    if (_enabled)
      Universe::set_jvmpi_alloc_event_enabled(Universe::_jvmpi_disabled);
  }
  ~JVMPIAllocEventDisabler() {
    if (_enabled)
      Universe::set_jvmpi_alloc_event_enabled(Universe::_jvmpi_enabled);
  }
};

void SharedHeap::fill_region_with_object(MemRegion mr) {
  // Disable allocation events, since this isn't a "real" allocation.
  JVMPIAllocEventDisabler dis;  
  
  // Disable the posting of JVMTI VMObjectAlloc events as we
  // don't want the filling of tlabs with filler arrays to be
  // reported to the profiler.
  NoJvmtiVMObjectAllocMark njm;    

  size_t word_size = mr.word_size();
  size_t aligned_array_header_size =
    align_object_size(typeArrayOopDesc::header_size(T_INT));

  if (word_size >= aligned_array_header_size) {
    const size_t array_length =
      pointer_delta(mr.end(), mr.start()) -
      typeArrayOopDesc::header_size(T_INT);
    const size_t array_length_words =
      array_length * (HeapWordSize/sizeof(jint));
    post_allocation_setup_array(Universe::intArrayKlassObj(),
				mr.start(),
				mr.word_size(),
				(int)array_length_words);
#ifdef ASSERT
    HeapWord* elt_words = (mr.start() + typeArrayOopDesc::header_size(T_INT));
    Copy::fill_to_aligned_words(elt_words, array_length, 0xDEAFBABE);
#endif
  } else {
    assert(word_size == (size_t)oopDesc::header_size(), "Unaligned?");
    post_allocation_setup_obj(SystemDictionary::object_klass(),
			      mr.start(),
			      mr.word_size());
  }
}
