#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memoryPool.cpp	1.25 04/05/18 11:39:14 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_memoryPool.cpp.incl"

MemoryPool::MemoryPool(const char* name,
                       PoolType type,
                       size_t init_size,
                       size_t max_size,
                       bool support_usage_threshold,
                       bool support_gc_threshold) {
  _name = name;
  _initial_size = init_size;
  _max_size = max_size;
  _memory_pool_obj = NULL;
  _available_for_allocation = true;
  _num_managers = 0;
  _type = type;

  // initialize the max and init size of collection usage
  _after_gc_usage = MemoryUsage(_initial_size, 0, 0, _max_size);

  _usage_sensor = NULL;
  _gc_usage_sensor = NULL;
  // usage threshold supports both high and low threshold
  _usage_threshold = new ThresholdSupport(support_usage_threshold, support_usage_threshold);
  // gc usage threshold supports only high threshold
  _gc_usage_threshold = new ThresholdSupport(support_gc_threshold, support_gc_threshold);
}

void MemoryPool::add_manager(MemoryManager* mgr) {
  assert(_num_managers < MemoryPool::max_num_managers, "_num_managers exceeds the max");
  if (_num_managers < MemoryPool::max_num_managers) {
    _managers[_num_managers] = mgr;
    _num_managers++;
  }
}


// Returns an instanceHandle of a MemoryPool object.
// It creates a MemoryPool instance when the first time
// this function is called.
instanceOop MemoryPool::get_memory_pool_instance(TRAPS) { 
  // Must do an acquire so as to force ordering of subsequent
  // loads from anything _memory_pool_obj points to or implies.
  instanceOop pool_obj = (instanceOop)OrderAccess::load_ptr_acquire(&_memory_pool_obj);
  if (pool_obj == NULL) {
    // It's ok for more than one thread to execute the code up to the locked region.
    // Extra pool instances will just be gc'ed.
    klassOop k = Management::sun_management_ManagementFactory_klass(CHECK_0);
    instanceKlassHandle ik(THREAD, k);
    
    Handle pool_name = java_lang_String::create_from_str(_name, CHECK_0);
    jlong usage_threshold_value = (_usage_threshold->is_high_threshold_supported() ? 0 : -1L);
    jlong gc_usage_threshold_value = (_gc_usage_threshold->is_high_threshold_supported() ? 0 : -1L);
  
    JavaValue result(T_OBJECT);
    JavaCallArguments args; 
    args.push_oop(pool_name);           // Argument 1
    args.push_int((int) is_heap());     // Argument 2
  
    symbolHandle method_name = vmSymbolHandles::createMemoryPool_name();
    symbolHandle signature = vmSymbolHandles::createMemoryPool_signature();

    // ---------------------------------------------------------------------
    // Transitioning: TO BE REMOVED after JDK support for MXBean is putback
    // ---------------------------------------------------------------------
    if (ik->find_method(method_name(), signature()) == NULL) {
      method_name = vmSymbolHandles::createMemoryPoolMBean_name();
      signature = vmSymbolHandles::createMemoryPoolMBean_signature();
    }

    args.push_long(usage_threshold_value);    // Argument 3
    args.push_long(gc_usage_threshold_value); // Argument 4

    JavaCalls::call_static(&result,
                           ik,
                           method_name,
                           signature,
                           &args,
                           CHECK_0);

    instanceOop p = (instanceOop) result.get_jobject();
    instanceHandle pool(THREAD, p);

    {
      // Get lock since another thread may have create the instance 
      MutexLocker ml(Management_lock);

      // Check if another thread has created the pool.  We reload
      // _memory_pool_obj here because some other thread may have
      // initialized it while we were executing the code before the lock.
      //
      // The lock has done an acquire, so the load can't float above it,
      // but we need to do a load_acquire as above.
      pool_obj = (instanceOop)OrderAccess::load_ptr_acquire(&_memory_pool_obj);
      if (pool_obj != NULL) {
         return pool_obj;
      }

      // Get the address of the object we created via call_special.
      pool_obj = pool();

      // Use store barrier to make sure the memory accesses associated
      // with creating the pool are visible before publishing its address.
      // The unlock will publish the store to _memory_pool_obj because
      // it does a release first.
      OrderAccess::release_store_ptr(&_memory_pool_obj, pool_obj);
    }
  }

  return pool_obj;
}

inline static size_t get_max_value(size_t val1, size_t val2) {
    return (val1 > val2 ? val1 : val2);
}

void MemoryPool::record_peak_memory_usage() {
  // Caller in JDK is responsible for synchronization -
  // acquire the lock for this memory pool before calling VM
  MemoryUsage usage = get_memory_usage();
  size_t peak_used = get_max_value(usage.used(), _peak_usage.used());
  size_t peak_committed = get_max_value(usage.committed(), _peak_usage.committed());
  size_t peak_max_size = get_max_value(usage.max_size(), _peak_usage.max_size());

  _peak_usage = MemoryUsage(initial_size(), peak_used, peak_committed, peak_max_size);
}

static void set_sensor_obj_at(SensorInfo** sensor_ptr, instanceHandle sh) {
  assert(*sensor_ptr == NULL, "Should be called only once");
  SensorInfo* sensor = new SensorInfo();
  sensor->set_sensor(sh());
  *sensor_ptr = sensor;
}

void MemoryPool::set_usage_sensor_obj(instanceHandle sh) {
  set_sensor_obj_at(&_usage_sensor, sh);
}

void MemoryPool::set_gc_usage_sensor_obj(instanceHandle sh) {
  set_sensor_obj_at(&_gc_usage_sensor, sh);
}

void MemoryPool::oops_do(OopClosure* f) {
  f->do_oop((oop*) &_memory_pool_obj);
  if (_usage_sensor != NULL) {
    _usage_sensor->oops_do(f);
  }
  if (_gc_usage_sensor != NULL) {
    _gc_usage_sensor->oops_do(f);
  }
}

ContiguousSpacePool::ContiguousSpacePool(ContiguousSpace* space, 
                                         const char* name, 
                                         PoolType type, 
                                         size_t max_size,
                                         bool support_usage_threshold) :
  CollectedMemoryPool(name, type, space->capacity(), max_size, 
                      support_usage_threshold), _space(space) {
}

MemoryUsage ContiguousSpacePool::get_memory_usage() {
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);
  size_t used      = used_in_bytes();
  size_t committed = _space->capacity();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

SurvivorContiguousSpacePool::SurvivorContiguousSpacePool(DefNewGeneration* gen,
                                                         const char* name, 
                                                         PoolType type, 
                                                         size_t max_size,
                                                         bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->from()->capacity(), max_size, 
                      support_usage_threshold), _gen(gen) {
}

MemoryUsage SurvivorContiguousSpacePool::get_memory_usage() {
  size_t maxSize = (available_for_allocation() ? max_size() : 0);
  size_t used    = used_in_bytes();
  size_t committed = committed_in_bytes();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

CompactibleFreeListSpacePool::CompactibleFreeListSpacePool(CompactibleFreeListSpace* space, 
                                                           const char* name,
                                                           PoolType type, 
                                                           size_t max_size,
                                                           bool support_usage_threshold) :
  CollectedMemoryPool(name, type, space->capacity(), max_size, 
                      support_usage_threshold), _space(space) {
}

MemoryUsage CompactibleFreeListSpacePool::get_memory_usage() {
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);
  size_t used      = used_in_bytes();
  size_t committed = _space->capacity();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

PSGenerationPool::PSGenerationPool(PSOldGen* gen, 
                                   const char* name, 
                                   PoolType type,
                                   bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->capacity_in_bytes(), 
                      gen->reserved().byte_size(), support_usage_threshold), _gen(gen) {
}

PSGenerationPool::PSGenerationPool(PSPermGen* gen, 
                                   const char* name, 
                                   PoolType type,
                                   bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->capacity_in_bytes(), 
                      gen->reserved().byte_size(), support_usage_threshold), _gen(gen) {
}

MemoryUsage PSGenerationPool::get_memory_usage() {
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);
  size_t used      = used_in_bytes();
  size_t committed = _gen->capacity_in_bytes();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

// The max size of EdenMutableSpacePool =
//     max size of the PSYoungGen - capacity of two survivor spaces
//
// Max size of PS eden space is changing due to ergonomic.
// PSYoungGen, PSOldGen, Eden, Survivor spaces are all resizable.
//
EdenMutableSpacePool::EdenMutableSpacePool(PSYoungGen* gen,
                                           MutableSpace* space,
                                           const char* name, 
                                           PoolType type,
                                           bool support_usage_threshold) :
  CollectedMemoryPool(name, type, space->capacity_in_bytes(), 
                      (gen->max_size() - gen->from_space()->capacity_in_bytes() - gen->to_space()->capacity_in_bytes()), 
                       support_usage_threshold), 
  _gen(gen), _space(space) {
}

MemoryUsage EdenMutableSpacePool::get_memory_usage() {
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);
  size_t used = used_in_bytes();
  size_t committed = _space->capacity_in_bytes();

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

// The max size of SurvivorMutableSpacePool =
//     current capacity of the from-space
//
// PS from and to survivor spaces could have different sizes.
//
SurvivorMutableSpacePool::SurvivorMutableSpacePool(PSYoungGen* gen,
                                                   const char* name, 
                                                   PoolType type,
                                                   bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->from_space()->capacity_in_bytes(),
                      gen->from_space()->capacity_in_bytes(),
                      support_usage_threshold), _gen(gen) {
}

MemoryUsage SurvivorMutableSpacePool::get_memory_usage() {
  size_t maxSize = (available_for_allocation() ? max_size() : 0);
  size_t used    = used_in_bytes();
  size_t committed = committed_in_bytes();
  return MemoryUsage(initial_size(), used, committed, maxSize);
}

GenerationPool::GenerationPool(Generation* gen,
                               const char* name, 
                               PoolType type,
                               bool support_usage_threshold) :
  CollectedMemoryPool(name, type, gen->capacity(), gen->max_capacity(), 
                      support_usage_threshold), _gen(gen) {
}

MemoryUsage GenerationPool::get_memory_usage() {
  size_t used      = used_in_bytes();
  size_t committed = _gen->capacity();
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);

  return MemoryUsage(initial_size(), used, committed, maxSize);
}

CodeHeapPool::CodeHeapPool(CodeHeap* codeHeap, const char* name, bool support_usage_threshold) :
  MemoryPool(name, NonHeap, codeHeap->capacity(), codeHeap->max_capacity(), 
             support_usage_threshold, false), _codeHeap(codeHeap) {
}

MemoryUsage CodeHeapPool::get_memory_usage() {
  size_t used      = used_in_bytes();
  size_t committed = _codeHeap->capacity();
  size_t maxSize   = (available_for_allocation() ? max_size() : 0);

  return MemoryUsage(initial_size(), used, committed, maxSize);
}
