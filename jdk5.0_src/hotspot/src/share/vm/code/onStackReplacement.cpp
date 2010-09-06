#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)onStackReplacement.cpp	1.44 04/04/05 13:05:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_onStackReplacement.cpp.incl"


// Implementation of OSRAdapter

OSRAdapter::OSRAdapter(CodeBuffer *cb, OopMapSet *oop_maps, int size,
                       int frame_size, int returning_fp_entry_offset)
  :CodeBlob("OSRAdapter",
            cb,                  // Code buffer
            sizeof(OSRAdapter),  // Header size            
            size,                // Size
            frame_size,          // Frame size
            oop_maps)
, _returning_fp_entry_offset(returning_fp_entry_offset) {
}


OSRAdapter* OSRAdapter::new_osr_adapter(CodeBuffer* cb,	OopMapSet *oop_maps,
                                        int frame_size, int returning_fp_entry_offset) {
  unsigned int size = allocation_size(cb, sizeof(OSRAdapter));
  OSRAdapter* osr_adapter = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    osr_adapter = new (size) OSRAdapter(cb, oop_maps, size, frame_size, returning_fp_entry_offset);
  }

  // We do not need to hold the CodeCache lock during name formatting.
  if (osr_adapter != NULL) {
    char blob_id[256];
    jio_snprintf(blob_id, sizeof(blob_id), "OSRAdapter@" PTR_FORMAT, osr_adapter->instructions_begin());
    VTune::register_stub(blob_id, osr_adapter->instructions_begin(), osr_adapter->instructions_end());
    Forte::register_stub(blob_id, osr_adapter->instructions_begin(), osr_adapter->instructions_end());

    // notify JVMTI profiler about this OSR
    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated("OSRAdapter", osr_adapter->instructions_begin(),
					       osr_adapter->instructions_end());
    }
  }

  return osr_adapter;
}


void* OSRAdapter::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);  
  return p;
}


#ifndef PRODUCT
void OSRAdapter::verify() {
  // Nothing to verify for now
}


void OSRAdapter::print() const {
  tty->print_cr("[OSRAdapter. Framesize: %d]", frame_size());
}


void OSRAdapter::print_value_on(outputStream* st) const {
  tty->print_cr("[OSRAdapter]");
}
#endif // PRODUCT


// Implementation of OnStackReplacement

GrowableArray<OSRAdapter*>* OnStackReplacement::_osr_adapters;

void OnStackReplacement::initialize() {
  if (!UseOnStackReplacement) return;
  // Initialize vector of OSRAdapter. The common used sizes gets precomputed
  // to limit fragmentation of the code cache.
  _osr_adapters = new(ResourceObj::C_HEAP)GrowableArray<OSRAdapter*>(InitialAdapterVectorSize, true);
  // Precompute common cases
  assert(MaxTypicalAdapterSize < InitialAdapterVectorSize, "wrong setup parameters");
  for(int framesize = MaxTypicalAdapterSize; framesize >= MinTypicalAdapterSize; framesize--) {
    OSRAdapter* osr = SharedRuntime::generate_osr_blob(framesize);
    assert(osr != NULL, "must generate an adapter blob");
    _osr_adapters->at_put_grow(framesize, osr);
  }
}


// The generator method is implemented in sharedRuntime_<cpu>.cpp
OSRAdapter* OnStackReplacement::get_osr_adapter(frame fr, methodHandle method) {
  assert(UseOnStackReplacement, "on-stack replacement not used");

  // compute osr_adapter_frame_return_address
  int framesize = fr.frame_size() + method->size_of_parameters();
#if defined(IA32) || defined(AMD64)
  {
    // the osr adapter code in the interpreter aligns the size of the
    // adapter frame so that the compiled code is properly aligned.
    // This calculation duplicates that logic so that the framesize of
    // the osr adapter matches that size.
    intptr_t* sp = (intptr_t*)
      ((intptr_t) (fr.sp() - method->size_of_parameters()) & -(2*wordSize));
    frame aligned_frame(sp, fr.fp(), fr.pc());
    framesize = aligned_frame.frame_size();
  }
#endif


  OSRAdapter* osr = NULL;  
  { MutexLocker mu(AdapterCache_lock);
    // This will potentially grow the array. 
    osr = _osr_adapters->at_grow(framesize);
  }

  // Need to create OSR adapter and update list.
  if (osr == NULL) {
    osr = SharedRuntime::generate_osr_blob(framesize);
    if (osr == NULL) {
       //CodeCache is probably full
       return NULL;
    }

    OSRAdapter* check = NULL;
    { MutexLocker mu(AdapterCache_lock);
      // Grap lock and update cache. Check if someone else already updated the list
      check = _osr_adapters->at(framesize);
      if (check == NULL) {
        _osr_adapters->at_put(framesize, osr);    
      }
    }

    // The adapter already existed
    if (check != NULL) {
      MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
      CodeCache::free(osr);     
      osr = check;                
    }      
  }
  return osr;
}


void onStackReplacement_init() {
  // Note: C1 doesn't need any of the osr adapter machinery
  //       in this file as there is only one osr adapter and
  //       the corresponding code is generated via c1_Runtime1
  //       => no initialization code needed for now
  OnStackReplacement::initialize();
}
