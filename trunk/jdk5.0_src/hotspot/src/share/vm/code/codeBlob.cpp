#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)codeBlob.cpp	1.107 04/04/05 13:05:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_codeBlob.cpp.incl"

unsigned int align_code_offset(int offset) {
  // align the size to CodeEntryAlignment
  return
    ((offset + (int)CodeHeap::header_size() + (CodeEntryAlignment-1)) & ~(CodeEntryAlignment-1))
    - (int)CodeHeap::header_size();
}


unsigned int CodeBlob::allocation_size(CodeBuffer* cb, int header_size, int oop_size) {
  unsigned int size = header_size + round_to(cb->reloc_size(), oopSize);
  // align the size to CodeEntryAlignment
  size = align_code_offset(size);
  size += round_to(cb->code_size(), oopSize);
  size += oop_size;
  return size;
}


// Creates a simple CodeBlob. Sets up the size of the different regions.
CodeBlob::CodeBlob(const char* name, int header_size, int size) {
  assert(size == round_to(size, oopSize), "unaligned size");
  assert(header_size == round_to(header_size, oopSize), "unaligned size");
  assert(!UseRelocIndex, "no space allocated for reloc index yet");

  // Note: If UseRelocIndex is enabled, there needs to be (at least) one
  //       extra word for the relocation information, containing the reloc
  //       index table length. Unfortunately, the reloc index table imple-
  //       mentation is not easily understandable and thus it is not clear
  //       what exactly the format is supposed to be. For now, we just turn
  //       off the use of this table (gri 7/6/2000).

  _name                  = name;
  _size                  = size;
  _header_size           = header_size;
  _relocation_size       =  0;
  _instructions_offset   = align_code_offset(header_size);
  _data_offset           = size;
  _oops_offset           = size;
  _oops_length           =  0;
  _frame_size            =  0;
  set_oop_maps(NULL);
}


// Creates a CodeBlob from a CodeBuffer. Sets up the size of the different regions,
// and copy code and relocation info.
CodeBlob::CodeBlob(
  const char* name,
  CodeBuffer* cb,
  int         header_size,
  int         size,
  int         frame_size,
  OopMapSet*  oop_maps,
  int         oop_size
) {
  assert(size == round_to(size, oopSize), "unaligned size");
  assert(header_size == round_to(header_size, oopSize), "unaligned size");

  _name                = name;
  _size                = size;
  _header_size         = header_size;  
  _relocation_size     = round_to(cb->reloc_size(), oopSize);
  _instructions_offset = align_code_offset(header_size + _relocation_size);
  _data_offset         = _instructions_offset + round_to(cb->code_size(), oopSize);
  _oops_offset         = _size - round_to(oop_size, oopSize);
  _oops_length         = 0;
  assert(_oops_offset >= _data_offset, "codeBlob is too small");
  assert(_data_offset <= size, "codeBlob is too small");

  cb->copy_relocation(this);	// copy relocation info
  cb->copy_code(this);		// copy code
  set_oop_maps(oop_maps);
  _frame_size = frame_size;
  COMPILER2_ONLY(_link_offset = not_yet_computed;) // Set to Uncomputed value
#ifdef COMPILER1
#ifdef SPARC
#endif // SPARC
  assert(_frame_size >= -1, "must use frame size or -1 for runtime stubs");
#endif // COMPILER1
}


void CodeBlob::set_oop_maps(OopMapSet* p) {
  // Danger Will Robinson! This method allocates a big
  // chunk of memory, its your job to free it.
  if (p != NULL) {
    // We need to allocate a chunk big enough to hold the OopMapSet and all of its OopMaps
    _oop_maps = (OopMapSet* )NEW_C_HEAP_ARRAY(unsigned char, p->heap_size());
    p->copy_to((address)_oop_maps);
  } else {
    _oop_maps = NULL;
  }
}


void CodeBlob::flush() {
  if (_oop_maps) {
    FREE_C_HEAP_ARRAY(unsigned char, _oop_maps);
    _oop_maps = NULL;
  }
}


// relocation indexes are biased by 1 (because 0 is reserved)
oop* CodeBlob::oop_addr_at(int index) const {  
  assert(index > 0 && index <= _oops_length, "must be a valid non-zero index");
  return &oops_begin()[index-1];  
}


void CodeBlob::copy_oops(jobject* array, int length) {
  for (int index = 0 ; index < length; index++) {
    // As a special case, IC oops are initialized to 1 or -1.
    if (array[index] == (jobject)Universe::non_oop_word()) {
      oops_begin()[index] = (oop)Universe::non_oop_word();
      continue;
    }
    oops_begin()[index] = JNIHandles::resolve(array[index]);
  }
  _oops_length = length;
}


relocInfo::relocType CodeBlob::reloc_type_for_address(address pc) {
  RelocIterator iter(this, pc, pc+1);
  while (iter.next()) {
    return (relocInfo::relocType) iter.type();  
  }

  if (SafepointPolling) {
    // Not relocation info found for pc 
    ShouldNotReachHere();
  }
  // lazy deopt speculatively calls this when !SafepointPolling
  // so we must return something and none is fine
  return relocInfo::none; // dummy return value
}


bool CodeBlob::is_at_poll_return(address pc) {
  RelocIterator iter(this, pc, pc+1);
  while (iter.next()) {
    if (iter.type() == relocInfo::poll_return_type)
      return true;
  }
  return false;
}


bool CodeBlob::is_at_poll_or_poll_return(address pc) {
  RelocIterator iter(this, pc, pc+1);
  while (iter.next()) {
    relocInfo::relocType t = iter.type();
    if (t == relocInfo::poll_return_type || t == relocInfo::poll_type)
      return true;
  }
  return false;
}


void CodeBlob::fix_relocation_at_move(intptr_t delta) {
  PatchingRelocIterator iter(this);
  while (iter.next()) {
    iter.reloc()->fix_relocation_at_move(delta);
  }
}


void CodeBlob::fix_oop_relocations(address begin, address end) {
  // re-patch all oop-bearing instructions, just in case some oops moved
#ifndef CORE
  bool skip_embedded =  is_nmethod() && ((nmethod*)this)->is_patched_for_deopt();
#else
  bool skip_embedded =  false;
#endif /* CORE */
  RelocIterator iter(this, begin, end);
  while (iter.next()) {
    if (!skip_embedded || iter.oop_reloc()->oop_index() != 0) {
      iter.reloc()->fix_oop_relocation();
    }
  }
}


void CodeBlob::fix_oop_relocations() {
  fix_oop_relocations(NULL, NULL);
}


void CodeBlob::follow_roots_or_mark_for_unloading(BoolObjectClosure* is_alive,
                      OopClosure* keep_alive,
                      bool unloading_occurred,
                      bool& marked_for_unloading) {
   ShouldNotReachHere();   //MAY CAUSE A PROBLEM FOR CMS??!!! XXX YSR
}

OopMap* CodeBlob::oop_map_for_return_address(address return_address, bool at_call) {
  address pc = return_address ;
  assert (oop_maps() != NULL, "nope");
  return oop_maps()->find_map_at_offset ((intptr_t) pc - (intptr_t) instructions_begin(), at_call);
}


//----------------------------------------------------------------------------------------------------
// Implementation of BufferBlob


BufferBlob::BufferBlob(const char* name, int size)
: CodeBlob(name, sizeof(BufferBlob), size)
{}

BufferBlob* BufferBlob::create(const char* name, int buffer_size) {
  ThreadInVMfromUnknown __tiv;	// get to VM state in case we block on CodeCache_lock

  BufferBlob* blob = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    unsigned int size = align_code_offset(sizeof(BufferBlob)) + round_to(buffer_size, oopSize);
    assert(name != NULL, "must provide a name");
    blob = new (size) BufferBlob(name, size);
  }
  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();

  return blob;
}


void* BufferBlob::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  return p;
}


void BufferBlob::free( BufferBlob *blob ) {
  ThreadInVMfromUnknown __tiv;	// get to VM state in case we block on CodeCache_lock
  { 
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    CodeCache::free((CodeBlob*)blob);
  }
  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();
}



//----------------------------------------------------------------------------------------------------
// Implementation of RuntimeStub

RuntimeStub::RuntimeStub(
  const char* name,
  CodeBuffer* cb,
  int         size,
  int         frame_size,
  OopMapSet*  oop_maps,
  bool        caller_must_gc_arguments
) 
: CodeBlob(name, cb, sizeof(RuntimeStub), size, frame_size, oop_maps)
{  
  _caller_must_gc_arguments = caller_must_gc_arguments;
}


RuntimeStub* RuntimeStub::new_runtime_stub(const char* stub_name,
                                           CodeBuffer* cb,
                                           int frame_size,
                                           OopMapSet* oop_maps,
                                           bool caller_must_gc_arguments)
{
  RuntimeStub* stub = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    unsigned int size = allocation_size(cb, sizeof(RuntimeStub));
    stub = new (size) RuntimeStub(stub_name, cb, size, frame_size, oop_maps, caller_must_gc_arguments);
  }

  // Do not hold the CodeCache lock during name formatting.
  if (stub != NULL) {
    char stub_id[256];
    jio_snprintf(stub_id, sizeof(stub_id), "RuntimeStub - %s", stub_name);
    VTune::register_stub(stub_id, stub->instructions_begin(), stub->instructions_end());   
    Forte::register_stub(stub_id, stub->instructions_begin(), stub->instructions_end());   

    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated(stub_name, stub->instructions_begin(), stub->instructions_end());      
    }
  }

  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();

  return stub;
}


void* RuntimeStub::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("Initial size of CodeCache is too small");
  return p;
}


//----------------------------------------------------------------------------------------------------
// Implementation of DeoptimizationBlob

DeoptimizationBlob::DeoptimizationBlob(
  CodeBuffer* cb,
  int         size,
  OopMapSet*  oop_maps, 
  int         unpack_offset,
  int         unpack_with_exception_offset,
  int         unpack_with_reexecution_offset,
  int         frame_size
)
: SingletonBlob("DeoptimizationBlob", cb, sizeof(DeoptimizationBlob), size, frame_size, oop_maps)
{
  _unpack_offset           = unpack_offset;  
  _unpack_with_exception   = unpack_with_exception_offset;
  _unpack_with_reexecution = unpack_with_reexecution_offset;
}


DeoptimizationBlob* DeoptimizationBlob::create(
  CodeBuffer* cb,
  OopMapSet*  oop_maps, 
  int        unpack_offset,
  int        unpack_with_exception_offset,
  int        unpack_with_reexecution_offset,
  int        frame_size)
{
  DeoptimizationBlob* blob = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    unsigned int size = allocation_size(cb, sizeof(DeoptimizationBlob));
    blob = new (size) DeoptimizationBlob(cb,
                                         size,
                                         oop_maps,
                                         unpack_offset,
                                         unpack_with_exception_offset,
                                         unpack_with_reexecution_offset,
                                         frame_size);
  }

  // Do not hold the CodeCache lock during name formatting.
  if (blob != NULL) {
    char blob_id[256];
    jio_snprintf(blob_id, sizeof(blob_id), "DeoptimizationBlob@" PTR_FORMAT, blob->instructions_begin());
    VTune::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());
    Forte::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());

    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated("DeoptimizationBlob",
					       blob->instructions_begin(),
					       blob->instructions_end());
    }    
  }

  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();

  return blob;
}


void* DeoptimizationBlob::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("Initial size of CodeCache is too small");
  return p;
}

//----------------------------------------------------------------------------------------------------
// Implementation of UncommonTrapBlob

#ifdef COMPILER2
UncommonTrapBlob::UncommonTrapBlob(
  CodeBuffer* cb,
  int         size,
  OopMapSet*  oop_maps,
  int         frame_size
)
: SingletonBlob("UncommonTrapBlob", cb, sizeof(UncommonTrapBlob), size, frame_size, oop_maps)
{}


UncommonTrapBlob* UncommonTrapBlob::create(
  CodeBuffer* cb,
  OopMapSet*  oop_maps,
  int        frame_size)
{
  UncommonTrapBlob* blob = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    unsigned int size = allocation_size(cb, sizeof(UncommonTrapBlob)); 
    blob = new (size) UncommonTrapBlob(cb, size, oop_maps, frame_size);
  }

  // Do not hold the CodeCache lock during name formatting.
  if (blob != NULL) {
    char blob_id[256];
    jio_snprintf(blob_id, sizeof(blob_id), "UncommonTrapBlob@" PTR_FORMAT, blob->instructions_begin());
    VTune::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());
    Forte::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());
   
    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated("UncommonTrapBlob",
					       blob->instructions_begin(),
					       blob->instructions_end());
    }    
  }

  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();

  return blob;
}


void* UncommonTrapBlob::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("Initial size of CodeCache is too small");
  return p;
}
#endif // COMPILER2


//----------------------------------------------------------------------------------------------------
// Implementation of ExceptionBlob

#ifdef COMPILER2
ExceptionBlob::ExceptionBlob(
  CodeBuffer* cb,
  int         size,
  OopMapSet*  oop_maps,
  int         frame_size
) 
: SingletonBlob("ExceptionBlob", cb, sizeof(ExceptionBlob), size, frame_size, oop_maps)
{}


ExceptionBlob* ExceptionBlob::create(
  CodeBuffer* cb,
  OopMapSet*  oop_maps,
  int         frame_size)
{
  ExceptionBlob* blob = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    unsigned int size = allocation_size(cb, sizeof(ExceptionBlob)); 
    blob = new (size) ExceptionBlob(cb, size, oop_maps, frame_size);
  }

  // We do not need to hold the CodeCache lock during name formatting
  if (blob != NULL) {
    char blob_id[256];
    jio_snprintf(blob_id, sizeof(blob_id), "ExceptionBlob@" PTR_FORMAT, blob->instructions_begin());
    VTune::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());
    Forte::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());

    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated("ExceptionBlob",
					       blob->instructions_begin(),
					       blob->instructions_end());
    }    
  }
  
  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();

  return blob;
}


void* ExceptionBlob::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("Initial size of CodeCache is too small");
  return p;
}
#endif // COMPILER2


//----------------------------------------------------------------------------------------------------
// Implementation of SafepointBlob

SafepointBlob::SafepointBlob(
  CodeBuffer* cb,
  int         size,
  OopMapSet*  oop_maps,
  int         frame_size
) 
: SingletonBlob("SafepointBlob", cb, sizeof(SafepointBlob), size, frame_size, oop_maps)
{}


SafepointBlob* SafepointBlob::create(
  CodeBuffer* cb,
  OopMapSet*  oop_maps,
  int         frame_size)
{
  SafepointBlob* blob = NULL;
  {
    MutexLockerEx mu(CodeCache_lock, Mutex::_no_safepoint_check_flag);
    unsigned int size = allocation_size(cb, sizeof(SafepointBlob)); 
    blob = new (size) SafepointBlob(cb, size, oop_maps, frame_size);
  }

  // We do not need to hold the CodeCache lock during name formatting.
  if (blob != NULL) {
    char blob_id[256];
    jio_snprintf(blob_id, sizeof(blob_id), "SafepointBlob@" PTR_FORMAT, blob->instructions_begin());
    VTune::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());
    Forte::register_stub(blob_id, blob->instructions_begin(), blob->instructions_end());

    if (JvmtiExport::should_post_dynamic_code_generated()) {
      JvmtiExport::post_dynamic_code_generated("SafepointBlob",
					       blob->instructions_begin(),
					       blob->instructions_end());
    }
  }

  // Track memory usage statistic after releasing CodeCache_lock
  MemoryService::track_code_cache_memory_usage();

  return blob;
}


void* SafepointBlob::operator new(size_t s, unsigned size) {
  void* p = CodeCache::allocate(size);
  if (!p) fatal("Initial size of CodeCache is too small");
  return p;
}


bool SafepointBlob::caller_must_gc_arguments(JavaThread* thread) const {
  return thread->safepoint_state()->caller_must_gc_arguments();
}


//----------------------------------------------------------------------------------------------------
// Non-product code

#ifndef PRODUCT

void CodeBlob::verify() {
  ShouldNotReachHere(); 
}


void CodeBlob::print() const {
  tty->print_cr("[CodeBlob]");
  tty->print_cr("Framesize: %d", _frame_size);
}


void CodeBlob::print_value_on(outputStream* st) const {
  st->print_cr("[CodeBlob]");
}


void BufferBlob::verify() {
  // unimplemented
}


void BufferBlob::print() const {
  CodeBlob::print();
  print_value_on(tty);
}


void BufferBlob::print_value_on(outputStream* st) const {
  st->print_cr("BufferBlob used for %s", name());
}


void RuntimeStub::verify() {  
  // unimplemented
}


void RuntimeStub::print() const {
  CodeBlob::print();
  tty->print("Runtime Stub: ");
  tty->print_cr(name());
  Disassembler::decode((CodeBlob*)this);
}


void RuntimeStub::print_value_on(outputStream* st) const {
  st->print("RuntimeStub: "); st->print(name());
}


void SingletonBlob::verify() {  
  // unimplemented
}


void SingletonBlob::print() const {
  CodeBlob::print();  
  tty->print_cr(name());
  Disassembler::decode((CodeBlob*)this);
}


void SingletonBlob::print_value_on(outputStream* st) const {
  st->print_cr(name());
}

void DeoptimizationBlob::print_value_on(outputStream* st) const {
  st->print_cr("Deoptimization (frame not available)");
}

#endif // PRODUCT
