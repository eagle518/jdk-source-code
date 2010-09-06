#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)perfMemory.cpp	1.18 03/12/23 16:44:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_perfMemory.cpp.incl"

char*                    PerfMemory::_start = NULL;
char*                    PerfMemory::_end = NULL;
char*                    PerfMemory::_top = NULL;
size_t                   PerfMemory::_capacity = 0;
jint                     PerfMemory::_initialized = false;
PerfDataPrologue*        PerfMemory::_prologue = NULL;

void perfMemory_init() {

  if (!UsePerfData) return;

  PerfMemory::initialize();
}

void perfMemory_exit() {

  if (!UsePerfData) return;
  if (!PerfMemory::is_initialized()) return;

  // if the StatSampler is active, then we don't want to remove
  // resources it may be dependent on. Typically, the StatSampler
  // is disengaged from the watcher thread when this method is called,
  // but it is not disengaged if this method is invoked during a
  // VM abort. 
  //
  if (!StatSampler::is_active())
    PerfDataManager::destroy();

  // remove the persistent external resources, if any. this method
  // does not unmap or invalidate any virtual memory allocated during
  // initialization.
  //
  PerfMemory::destroy();
}

void PerfMemory::initialize() {

  if (_prologue != NULL)
    // initialization already performed
    return;

  size_t capacity = align_size_up(PerfDataMemorySize,
                                  os::vm_allocation_granularity());

  if (PerfTraceMemOps) {
    tty->print("PerfDataMemorySize = " SIZE_FORMAT ","
	       " os::vm_allocation_granularity = " SIZE_FORMAT ","
	       " adjusted size = " SIZE_FORMAT "\n",
	       PerfDataMemorySize,
	       os::vm_allocation_granularity(),
	       capacity);
  }

  // allocate PerfData memory region
  create_memory_region(capacity);

  if (_start == NULL) {

    // the PerfMemory region could not be created as desired. Rather
    // than terminating the JVM, we revert to creating the instrumentation
    // on the C heap. When running in this mode, external monitoring
    // clients cannot attach to and monitor this JVM.
    //
    // the warning is issued only in debug mode in order to avoid
    // additional output to the stdout or stderr output streams.
    //
    if (PrintMiscellaneous && Verbose) {
      warning("Could not create PerfData Memory region, reverting to malloc");
    }

    _prologue = NEW_C_HEAP_OBJ(PerfDataPrologue);
  }
  else {

    // the PerfMemory region was created as expected.

    if (PerfTraceMemOps) {
      tty->print("PerfMemory created: address = " INTPTR_FORMAT ","
		 " size = " SIZE_FORMAT "\n",
		 (void*)_start,
		 _capacity);
    }

    _prologue = (PerfDataPrologue *)_start;
    _end = _start + _capacity;
    _top = _start + sizeof(PerfDataPrologue);
  }

  assert(_prologue != NULL, "prologue pointer must be initialized");

#ifdef VM_LITTLE_ENDIAN
  _prologue->magic = (jint)0xc0c0feca;
  _prologue->byte_order = PERFDATA_LITTLE_ENDIAN;
#else
  _prologue->magic = (jint)0xcafec0c0;
  _prologue->byte_order = PERFDATA_BIG_ENDIAN;
#endif

  _prologue->major_version = PERFDATA_MAJOR_VERSION;
  _prologue->minor_version = PERFDATA_MINOR_VERSION;
  _prologue->accessible = 0;

  _prologue->entry_offset = sizeof(PerfDataPrologue);
  _prologue->num_entries = 0;
  _prologue->used = 0;
  _prologue->overflow = 0;
  _prologue->mod_time_stamp = 0;

  OrderAccess::release_store(&_initialized, 1);
}

void PerfMemory::destroy() {

  assert(_prologue != NULL, "prologue pointer must be initialized");

  if (_start != NULL && _prologue->overflow != 0) {

    // This state indicates that the contiguous memory region exists and
    // that it wasn't large enough to hold all the counters. In this case,
    // we output a warning message to the user on exit if the -XX:+Verbose
    // flag is set (a debug only flag). External monitoring tools can detect
    // this condition by monitoring the _prologue->overflow word.
    //
    // There are two tunables that can help resolve this issue:
    //   - increase the size of the PerfMemory with -XX:PerfDataMemorySize=<n>
    //   - decrease the maximum string constant length with
    //     -XX:PerfMaxStringConstLength=<n>
    //
    if (PrintMiscellaneous && Verbose) {
      warning("PerfMemory Overflow Occurred.\n"
              "\tCapacity = " SIZE_FORMAT " bytes"
              "  Used = " SIZE_FORMAT " bytes"
              "  Overflow = " INT32_FORMAT " bytes"
              "\n\tUse -XX:PerfDataMemorySize=<size> to specify larger size.",
              PerfMemory::capacity(),
              PerfMemory::used(),
              _prologue->overflow);
    }
  }

  if (_start != NULL) {

    // this state indicates that the contiguous memory region was successfully
    // and that persistent resources may need to be cleaned up. This is
    // expected to be the typical condition.
    //
    delete_memory_region();
  }

  _start = NULL;
  _end = NULL;
  _top = NULL;
  _prologue = NULL;
  _capacity = 0;
}

// allocate an aligned block of memory from the PerfData memory
// region. This method assumes that the PerfData memory region
// was aligned on a double word boundary when created.
//
char* PerfMemory::alloc(size_t size) {

  if (!UsePerfData) return NULL;

  MutexLocker ml(PerfDataMemAlloc_lock);

  assert(_prologue != NULL, "called before initialization");

  // check that there is enough memory for this request
  if ((_top + size) >= _end) {

    _prologue->overflow += (jint)size;

    return NULL;
  }

  char* result = _top;

  _top += size;

  assert(contains(result), "PerfData memory pointer out of range");

  _prologue->used = (jint)used();
  _prologue->num_entries = _prologue->num_entries + 1;

  return result;
}

void PerfMemory::mark_updated() {
  if (!UsePerfData) return;

  _prologue->mod_time_stamp = os::elapsedTime();
}
