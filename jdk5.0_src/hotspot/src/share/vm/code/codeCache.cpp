#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)codeCache.cpp	1.111 04/05/21 12:30:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_codeCache.cpp.incl"

// Helper class for printing in CodeCache

class CodeBlob_sizes {
 private:
  int count;
  int total_size;
  int header_size;
  int code_size;
  int exception_size;
  int stub_size;
  int relocation_size;
  int scopes_oop_size;
  int scopes_data_size;
  int scopes_pcs_size;

 public:
  CodeBlob_sizes() {
    count            = 0;
    total_size       = 0;
    header_size      = 0;
    code_size        = 0;
    exception_size   = 0;
    stub_size        = 0;
    relocation_size  = 0;
    scopes_oop_size  = 0;
    scopes_data_size = 0;
    scopes_pcs_size  = 0;
  }

  int total()                                    { return total_size; }
  bool is_empty()                                { return count == 0; }

  void print(const char* title) {
    tty->print_cr(" #%d %s = %dK (hdr %d%%,  loc %d%%, code %d%%, except %d%%, stub %d%%, [oops %d%%, data %d%%, pcs %d%%])",
                  count,
                  title,
                  total() / K,
                  header_size             * 100 / total_size,
                  relocation_size         * 100 / total_size,
                  code_size               * 100 / total_size,
                  exception_size          * 100 / total_size,
                  stub_size               * 100 / total_size,                  
                  scopes_oop_size         * 100 / total_size,
                  scopes_data_size        * 100 / total_size,
                  scopes_pcs_size         * 100 / total_size);
  }

  void add(CodeBlob* cb) {
    count++;
    total_size       += cb->size();
    header_size      += cb->header_size();
    relocation_size  += cb->relocation_size();
    scopes_oop_size  += cb->oops_size();
    if (cb->is_nmethod()) { 
#ifndef CORE
      nmethod *nm = (nmethod*)cb;
      code_size        += nm->code_size();
      exception_size   += nm->exception_size();
      stub_size        += nm->stub_size();
    
      scopes_data_size += nm->scopes_data_size();
      scopes_pcs_size  += nm->scopes_pcs_size();
#endif
    } else {
      code_size        += cb->instructions_size();
    }
  }
};


// CodeCache implementation

CodeHeap * CodeCache::_heap = new CodeHeap();
int CodeCache::_number_of_blobs = 0;
int CodeCache::_number_of_nmethods_with_dependencies = 0;


CodeBlob* CodeCache::first() {
  assert_locked_or_safepoint(CodeCache_lock);
  return (CodeBlob*)_heap->first();
}


CodeBlob* CodeCache::next(CodeBlob* cb) {
  assert_locked_or_safepoint(CodeCache_lock);
  return (CodeBlob*)_heap->next(cb);
}


CodeBlob* CodeCache::alive(CodeBlob *cb) {
  assert_locked_or_safepoint(CodeCache_lock);
  while (cb != NULL && !cb->is_alive()) cb = next(cb);
  return cb;
}


nmethod* CodeCache::alive_nmethod(CodeBlob* cb) {
  assert_locked_or_safepoint(CodeCache_lock);
  while (cb != NULL && (!cb->is_alive() || !cb->is_nmethod())) cb = next(cb);
  return (nmethod*)cb;
}


CodeBlob* CodeCache::allocate(int size) {
  // Do not seize the CodeCache lock here--if the caller has not
  // already done so, we are going to lose bigtime, since the code
  // cache will contain a garbage CodeBlob until the caller can
  // run the constructor for the CodeBlob subclass he is busy
  // instantiating.
  guarantee(size >= 0, "allocation request must be reasonable");
  assert_locked_or_safepoint(CodeCache_lock);
  CodeBlob* cb = NULL;
  _number_of_blobs++;
  while (true) {
    cb = (CodeBlob*)_heap->allocate(size);
    if (cb != NULL) break;
    if (!_heap->expand_by(CodeCacheExpansionSize)) {
      // Expansion failed
      return NULL;
    }
    if (PrintCodeCacheExtension) {
      ResourceMark rm;
      tty->print_cr("code cache extended to [0x%lx, 0x%lx[ (%d bytes)",
		    (long)_heap->begin(), (long)_heap->end(),
		    (address)_heap->end() - (address)_heap->begin());
    }
  }
  verify_if_often();
  if (PrintCodeCache2) {	// Need to add a new flag
      ResourceMark rm;
      tty->print_cr("CodeCache allocation:  addr: " INTPTR_FORMAT ", size: 0x%x\n", cb, size);
  }
  return cb;
}

void CodeCache::free(CodeBlob* cb) {
  assert_locked_or_safepoint(CodeCache_lock);
  verify_if_often();

  if (PrintCodeCache2) {	// Need to add a new flag
      ResourceMark rm;
      tty->print_cr("CodeCache free:  addr: " INTPTR_FORMAT ", size: 0x%x\n", cb, cb->size());
  }
#ifndef CORE
  if (cb->is_nmethod() && ((nmethod *)cb)->number_of_dependents() > 0) {
    _number_of_nmethods_with_dependencies--;
  }  
#endif
  _number_of_blobs--;  

  _heap->deallocate(cb);

  verify_if_often();  
  assert(_number_of_blobs >= 0, "sanity check");
}


void CodeCache::commit(CodeBlob* cb) {
  // this is called by nmethod::nmethod, which must already own CodeCache_lock
  assert_locked_or_safepoint(CodeCache_lock);
#ifndef CORE
  if (cb->is_nmethod() && ((nmethod *)cb)->number_of_dependents() > 0) {
    _number_of_nmethods_with_dependencies++;
  }  
#endif
  // flush the hardware I-cache
  ICache::invalidate_range(cb->instructions_begin(), cb->instructions_size());
}


void CodeCache::flush() {
  assert_locked_or_safepoint(CodeCache_lock);
  Unimplemented();
}


// Iteration over CodeBlobs

#define FOR_ALL_BLOBS(var)       for (CodeBlob *var =       first() ; var != NULL; var =       next(var) )
#define FOR_ALL_ALIVE_BLOBS(var) for (CodeBlob *var = alive(first()); var != NULL; var = alive(next(var)))
#define FOR_ALL_ALIVE_NMETHODS(var) for (nmethod *var = alive_nmethod(first()); var != NULL; var = alive_nmethod(next(var)))


bool CodeCache::contains(void *p) {
  // It should be ok to call contains without holding a lock
  return _heap->contains(p);
}


// This method is safe to call without holding the CodeCache_lock, as long as a dead codeblob is not
// looked up (i.e., one that has been marked for deletion). It only dependes on the _segmap to contain
// valid indices, which it will always do, as long as the CodeBlob is not in the process of being recycled.
CodeBlob* CodeCache::find_blob(void* start) {  
  CodeBlob* result = find_blob_unsafe(start);
  if (result == NULL) return NULL;
  // We could potientially look up non_entrant methods
  guarantee(!result->is_zombie() || result->is_locked_by_vm() || is_error_reported(), "unsafe access to zombie method");
  return result;  
}

CodeBlob* CodeCache::find_blob_unsafe(void* start) {  
  CodeBlob* result = (CodeBlob*)_heap->find_start(start);
  if (result == NULL) return NULL;
  assert(result->blob_contains((address)start), "found wrong CodeBlob");     
  return result;  
}

nmethod* CodeCache::find_nmethod(void* start) {  
  CodeBlob *cb = find_blob(start);
  assert(cb == NULL || cb->is_nmethod(), "did not find an nmethod");  
  return (nmethod*)cb;
}


void CodeCache::blobs_do(void f(CodeBlob* nm)) {
  assert_locked_or_safepoint(CodeCache_lock);
  FOR_ALL_BLOBS(p) { 
    f(p); 
  }
}


void CodeCache::nmethods_do(void f(nmethod* nm)) {
  assert_locked_or_safepoint(CodeCache_lock);
  FOR_ALL_BLOBS(nm) { 
    if (nm->is_nmethod()) f((nmethod*)nm); 
  }
}


int CodeCache::alignment_unit() {
  return (int)_heap->alignment_unit();
}


int CodeCache::alignment_offset() {
  return (int)_heap->alignment_offset();
}


// Follow all roots in the compiled code, unless they are the only
// ones keeping a class alive. In that case, we NULL out the roots,
// mark the CodeBlob for unloading and set the boolean flag
// marked_for_unloading to true.
void CodeCache::do_unloading(BoolObjectClosure* is_alive,
                                     OopClosure* keep_alive,
                                     bool unloading_occurred,
                                     bool& marked_for_unloading) {
  assert_locked_or_safepoint(CodeCache_lock);
  marked_for_unloading = false;
  FOR_ALL_ALIVE_BLOBS(cb) {
    cb->follow_roots_or_mark_for_unloading(
          is_alive, keep_alive,
          unloading_occurred, marked_for_unloading);
  }
}

void CodeCache::oops_do(OopClosure* f) {
  assert_locked_or_safepoint(CodeCache_lock);
  FOR_ALL_ALIVE_BLOBS(cb) {
    cb->oops_do(f);
  }
}

void CodeCache::gc_prologue() {
}


void CodeCache::gc_epilogue() {
  assert_locked_or_safepoint(CodeCache_lock);
  bool needs_cache_clean = false;
  FOR_ALL_ALIVE_BLOBS(cb) {
#ifndef CORE
    if (cb->is_nmethod()) {
      nmethod *nm = (nmethod*)cb;
      if (nm->is_marked_for_unloading()) {
        if (nm->is_in_use()) {
          // transitioning directly from live to unloaded so we need
          // to force a cache cleanup.
          needs_cache_clean = true;
        }
        // the GC may have discovered nmethods which should be unloaded, so
        // make them unloaded, so the nmethod sweeper will eventually flush them
        nm->make_unloaded();
        // no need to fix oop relocations in unloaded nmethods, so continue;
        continue;
      } else if (nm->is_patched_for_deopt()) {
        // no need to fix oop relocations in deopted nmethods, so continue;
        continue;
      } else {
        debug_only(nm->verify();)
      }
    }
#endif // CORE
    cb->fix_oop_relocations();
  }
#ifndef CORE
  if (needs_cache_clean) {
    // Normally the sweeper would lazily clean all the inline caches
    // up but because unloaded nmethod aren't made not_entrant first,
    // doing it lazily isn't safe.  Visit every live nmethod and clean
    // any inline caches which refer to nmethod whose state != alive.
    FOR_ALL_ALIVE_NMETHODS(nm) {
      nm->cleanup_inline_caches();
    }
  }
#endif
}


address CodeCache::first_address() {
  assert_locked_or_safepoint(CodeCache_lock);
  return (address)_heap->begin();
}


address CodeCache::last_address() {
  assert_locked_or_safepoint(CodeCache_lock);
  return (address)_heap->end();
}


void CodeCache::initialize() {
  assert(CodeCacheSegmentSize >= CodeEntryAlignment, "CodeCacheSegmentSize must be large enough to align entry points");
#ifdef COMPILER2
  assert(CodeCacheSegmentSize >= OptoLoopAlignment,  "CodeCacheSegmentSize must be large enough to align inner loops");
#endif
  assert(CodeCacheSegmentSize >= sizeof(jdouble),    "CodeCacheSegmentSize must be large enough to align constants");
  // This was originally just a check of the alignment, causing failure, instead, round
  // the code cache to the page size.  In particular, Solaris is moving to a larger
  // default page size.
  CodeCacheExpansionSize = round_to(CodeCacheExpansionSize, os::vm_page_size());
  InitialCodeCacheSize = round_to(InitialCodeCacheSize, os::vm_page_size());
  ReservedCodeCacheSize = round_to(ReservedCodeCacheSize, os::vm_page_size());
  if (!_heap->reserve(ReservedCodeCacheSize, InitialCodeCacheSize, CodeCacheSegmentSize)) {
    vm_exit_during_initialization("Could not reserve enough space for code cache");
  }

  MemoryService::add_code_heap_memory_pool(_heap);
}


void codeCache_init() {
  CodeCache::initialize();
}

//------------------------------------------------------------------------------------------------
// Non-CORE stuff

#ifndef CORE
int CodeCache::number_of_nmethods_with_dependencies() { 
  return _number_of_nmethods_with_dependencies; 
}

void CodeCache::clear_inline_caches() {
  assert_locked_or_safepoint(CodeCache_lock);
  FOR_ALL_ALIVE_NMETHODS(nm) {
    nm->clear_inline_caches();
  }
}

#ifndef PRODUCT
// used to keep track of how much time is spent in mark_for_deoptimization
static elapsedTimer dependentCheckTime;
static int dependentCheckCount = 0;
#endif // PRODUCT


int CodeCache::mark_for_deoptimization(klassOop dependee) {
  MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
  
#ifndef PRODUCT
  dependentCheckTime.start();
  dependentCheckCount++;
#endif // PRODUCT
  
  int number_of_marked_CodeBlobs = 0;
  // Compute the dependent nmethods
  if (FastNMethodDependencies) {
    // search the hierarchy looking for nmethods which are affected by the loading of this class
    for (klassOop d = dependee; d != NULL; d = instanceKlass::cast(d)->super()) {
      number_of_marked_CodeBlobs += instanceKlass::cast(d)->mark_dependent_nmethods(dependee);
    }
    // then search the interfaces this class implements looking for nmethods
    // which might be dependent of the fact that an interface only had one
    // implementor.
    objArrayOop interfaces = instanceKlass::cast(dependee)->transitive_interfaces();
    int number_of_interfaces = interfaces->length();
    for (int interface_index = 0; interface_index < number_of_interfaces; interface_index += 1) {
      klassOop d = klassOop(interfaces->obj_at(interface_index));
      number_of_marked_CodeBlobs += instanceKlass::cast(d)->mark_dependent_nmethods(dependee);
    }
    
#ifdef ASSERT
    // do a brute force search for nmethods which are dependent.
    FOR_ALL_ALIVE_NMETHODS(nm) {
      if (nm->is_dependent_on(dependee)) {
        assert(nm->is_marked_for_deoptimization(), "should have been marked");
      }
    }
#endif // ASSERT
  } else {
    FOR_ALL_ALIVE_NMETHODS(nm) {
      if (nm->is_dependent_on(dependee)) {
        if (TraceDependencies) {
          ResourceMark rm;
          tty->print_cr("Marked for deoptimization");
          tty->print_cr("  dependee = %s", instanceKlass::cast(dependee)->external_name());
          nm->print();
          nm->print_dependencies();
        }
        nm->mark_for_deoptimization();
        number_of_marked_CodeBlobs++;
      }
    }
  }
  
#ifndef PRODUCT
  dependentCheckTime.stop();
#endif // PRODUCT
  
  return number_of_marked_CodeBlobs;
}


#ifdef HOTSWAP
int CodeCache::mark_for_evol_deoptimization(instanceKlassHandle dependee) {
  MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
  int number_of_marked_CodeBlobs = 0;
  
  // Deoptimize all methods of the evolving class itself
  objArrayOop old_methods = dependee->methods();
  for (int i = 0; i < old_methods->length(); i++) {
    ResourceMark rm;
    methodOop old_method = (methodOop) old_methods->obj_at(i);
    nmethod *nm = old_method->code();
    if (nm != NULL) {
      nm->mark_for_deoptimization();
      number_of_marked_CodeBlobs++;
    }
  }

  FOR_ALL_ALIVE_NMETHODS(nm) {
    if (nm->is_evol_dependent_on(dependee())) {
      ResourceMark rm;
      nm->mark_for_deoptimization();
      number_of_marked_CodeBlobs++;
    } else  {
      // flush caches in case they refer to a redefined methodOop
      nm->clear_inline_caches();
    } 
  }

  return number_of_marked_CodeBlobs;
}
#endif HOTSWAP


int CodeCache::mark_for_deoptimization(methodOop dependee) {
  MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
  int number_of_marked_CodeBlobs = 0;
  
  FOR_ALL_ALIVE_NMETHODS(nm) {
    if (nm->is_dependent_on_method(dependee)) {
      ResourceMark rm;
      nm->mark_for_deoptimization();
      number_of_marked_CodeBlobs++;
    }
  }

  return number_of_marked_CodeBlobs;
}

void CodeCache::make_marked_nmethods_zombies() {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");
  FOR_ALL_ALIVE_NMETHODS(nm) {
    if (nm->is_marked_for_deoptimization()) {
      // If the nmethod has not been seen on the stack we
      // can convert it. Lazy deopt is pickier here.
      if (!nm->is_not_entrant() || nm->can_not_entrant_be_converted() ) {
	nm->make_zombie();
      }
    }
  }
}

void CodeCache::make_marked_nmethods_not_entrant() {
  assert_locked_or_safepoint(CodeCache_lock);
  FOR_ALL_ALIVE_NMETHODS(nm) {
    if (nm->is_marked_for_deoptimization()) {
      nm->make_not_entrant();
    }
  }
}


#endif

//------------------------------------------------------------------------------------------------
// Non-product version

#ifndef PRODUCT

void CodeCache::verify() {
  _heap->verify();
  FOR_ALL_ALIVE_BLOBS(p) {
    p->verify();
  }
}

void CodeCache::verify_if_often() {
  if (VerifyCodeCacheOften) {
    _heap->verify();
  }
}

void CodeCache::print_internals() {
  int nmethodCount = 0;
  int runtimeStubCount = 0;
  int i2cAdapterCount = 0;
  int c2iAdapterCount = 0;
  int deoptimizationStubCount = 0;
  int uncommonTrapStubCount = 0;
  int total = 0;
  int nmethodAlive = 0;
  int nmethodNotEntrant = 0;
  int nmethodZombie = 0;
  int nmethodUnloaded = 0;
  int nmethodJava = 0;
  int nmethodNative = 0;
  int maxCodeSize = 0;
  ResourceMark rm;

  CodeBlob *cb;
  for (cb = first(); cb != NULL; cb = next(cb)) {
    total++;
    if (cb->is_nmethod()) {
#ifndef CORE
      nmethod* nm = (nmethod*)cb;

      if (Verbose && nm->method() != NULL) {
	ResourceMark rm;
	char *method_name = nm->method()->name_and_sig_as_C_string();
	tty->print("%s", method_name);
	if(nm->is_patched_for_deopt()) { tty->print_cr(" patched"); }
	if(nm->is_alive()) { tty->print_cr(" alive"); }
	if(nm->is_not_entrant()) { tty->print_cr(" not-entrant"); }
	if(nm->is_zombie()) { tty->print_cr(" zombie"); }
      }

      nmethodCount++;
 
      if(nm->is_alive()) { nmethodAlive++; }
      if(nm->is_not_entrant()) { nmethodNotEntrant++; }
      if(nm->is_zombie()) { nmethodZombie++; }
      if(nm->is_unloaded()) { nmethodUnloaded++; }
      if(nm->is_native_method()) { nmethodNative++; }

      if(nm->is_java_method()) { 
        nmethodJava++;
        if(nm->code_size() > maxCodeSize) {
          maxCodeSize = nm->code_size();
        }
      }
#endif
    } else if (cb->is_runtime_stub()) {
      runtimeStubCount++;
    } else if (cb->is_i2c_adapter()) {
      i2cAdapterCount++;
    } else if (cb->is_c2i_adapter()) {
      c2iAdapterCount++;
    } else if (cb->is_deoptimization_stub()) {
      deoptimizationStubCount++;
    } else if (cb->is_uncommon_trap_stub()) {
      uncommonTrapStubCount++;
    }
  }

  int bucketSize = 512;
  int bucketLimit = maxCodeSize / bucketSize + 1;
  int *buckets = NEW_C_HEAP_ARRAY(int, bucketLimit);
  memset(buckets,0,sizeof(int) * bucketLimit);

  for (cb = first(); cb != NULL; cb = next(cb)) {
    if (cb->is_nmethod()) {
#ifndef CORE
      nmethod* nm = (nmethod*)cb;
      if(nm->is_java_method()) { 
        buckets[nm->code_size() / bucketSize]++;
      }
#endif
    }
  }
  tty->print_cr("Code Cache Entries (total of %d)",total);
  tty->print_cr("-------------------------------------------------");
  tty->print_cr("nmethods: %d",nmethodCount);
  tty->print_cr("\talive: %d",nmethodAlive);
  tty->print_cr("\tnot_entrant: %d",nmethodNotEntrant);
  tty->print_cr("\tzombie: %d",nmethodZombie);
  tty->print_cr("\tunloaded: %d",nmethodUnloaded);
  tty->print_cr("\tjava: %d",nmethodJava);
  tty->print_cr("\tnative: %d",nmethodNative);
  tty->print_cr("runtime_stubs: %d",runtimeStubCount);
  tty->print_cr("i2c_adapters: %d",i2cAdapterCount);
  tty->print_cr("c2i_adapters: %d",c2iAdapterCount);
  tty->print_cr("deoptimization_stubs: %d",deoptimizationStubCount);
  tty->print_cr("uncommon_traps: %d",uncommonTrapStubCount);
  tty->print_cr("\nnmethod size distribution (non-zombie java)");
  tty->print_cr("-------------------------------------------------");

  for(int i=0; i<bucketLimit; i++) {
    if(buckets[i] != 0) {
      tty->print("%d - %d bytes",i*bucketSize,(i+1)*bucketSize);
      tty->fill_to(40);
      tty->print_cr("%d",buckets[i]);
    }
  }

  FREE_C_HEAP_ARRAY(int, buckets);
}

void CodeCache::print() {
  CodeBlob_sizes live;
  CodeBlob_sizes dead;

  FOR_ALL_BLOBS(p) {
    if (!p->is_alive()) {
      dead.add(p);
    } else {
      live.add(p);
    }
  }

  tty->print_cr("CodeCache:");

#ifndef CORE
  tty->print_cr("nmethod dependency checking time %f", dependentCheckTime.seconds(),
                dependentCheckTime.seconds() / dependentCheckCount);
#endif // !CORE

  if (!live.is_empty()) {
    live.print("live");
  }
  if (!dead.is_empty()) {
    dead.print("dead");
  }


  if (Verbose) {
     // print the oop_map usage
    int code_size = 0;
    int number_of_blobs = 0;
    int number_of_oop_maps = 0;
    int map_size = 0;
    FOR_ALL_BLOBS(p) {
      if (p->is_alive()) {
        number_of_blobs++;
        code_size += p->instructions_size();
        OopMapSet* set = p->oop_maps();
        if (set != NULL) {
          number_of_oop_maps += set->size();      
          map_size   += set->heap_size();
        }
      }
    }
    tty->print_cr("OopMaps");
    tty->print_cr("  #blobs    = %d", number_of_blobs);
    tty->print_cr("  code size = %d", code_size);
    tty->print_cr("  #oop_maps = %d", number_of_oop_maps);
    tty->print_cr("  map size  = %d", map_size);
  }

}

#endif // PRODUCT
