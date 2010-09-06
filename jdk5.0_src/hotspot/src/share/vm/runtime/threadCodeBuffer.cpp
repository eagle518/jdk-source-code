#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadCodeBuffer.cpp	1.37 04/03/05 12:25:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_threadCodeBuffer.cpp.incl"

// -------------------------------------------------------------------------------------------------------
// Implementation of ThreadCodeBuffer

ThreadCodeBuffer::ThreadCodeBuffer(int size_in_bytes, nmethod *nm, address real_pc) {
   _code = NEW_C_HEAP_ARRAY(u_char, size_in_bytes);
  assert(_code != NULL, "out of memory");
  os::unguard_memory((char*) _code, size_in_bytes);
  _size = size_in_bytes;
  _method = nm;
  _real_pc = real_pc;

  debug_only(
    // Initialize area 
    for(int i=0; i<size_in_bytes; i++) {
      _code[i] = 0xEE;  
    }
  )
}

ThreadCodeBuffer::~ThreadCodeBuffer() {
  assert(_code != NULL, "buffer must be allocated");
  FREE_C_HEAP_ARRAY(char, _code);
  debug_only(
    _code = NULL;
    _size = 0;  
  )
}

// returns true if successful, false otherwise
bool ThreadCodeBuffer::reposition_and_resume_thread(JavaThread* thread, ExtendedPC addr) {
  return os::set_thread_pc_and_resume(thread, addr, addr.adjust(method()->instructions_begin(), method()->instructions_end(), code_begin()));
}

void ThreadCodeBuffer::copy(int offset, address start, int length) {
  assert(length>=0, "invalid parameters to copy_code");
  assert(offset+length<=size(), "code buffer to small");
  assert(_method->instructions_begin()<=start, "start not right nmethod");
  assert(start+length <= _method->instructions_end(), "end not right nmethod");
  
  // copy raw bytes
  memcpy(_code+offset, start, length);
}

void ThreadCodeBuffer::copy_code(int offset, address start, int length) {
  // Initialize buffer    
  assert(length>=0, "invalid parameters to copy_code");
  assert(offset+length<=size(), "code buffer to small");
  assert(_method->instructions_begin() <= start, "start not right nmethod");
  assert(start+length <= _method->instructions_end(), "end not right nmethod");

  // Acquire Patching_lock in case compiled code is being patched.  The
  // copied code buffer may be in an unstable state when c1 is patching the
  // code for calls to unloaded classes in Runtime1::patch_code.
  MutexLockerEx pl(Patching_lock, Mutex::_no_safepoint_check_flag);

  address copy_start = _code + offset;

  // copy raw bytes
  memcpy(copy_start, start, length);

  // reallocate all position dependent code in range  
  intptr_t delta = copy_start - start;
  
  RelocIterator iter(delta, _method, copy_start, copy_start + length);
  while(iter.next()) {
    NEEDS_CLEANUP
#ifdef _LP64
#if defined(SPARC) || defined(AMD64)
    // We can't relocate call types because they will typically go
    // across the address space. However, we patch all of these either
    // with illegal instructions or trampolines (for runtime calls) so
    // we merely skip the relocation step here.
    // TODO: we patch all but runtime_call_type on the other platforms too, so
    // the relocation step below should be skipped for all platforms too.
    if (iter.type() == relocInfo::virtual_call_type     ||
        iter.type() == relocInfo::opt_virtual_call_type ||
        iter.type() == relocInfo::static_call_type      ||
        iter.type() == relocInfo::runtime_call_type) {
      continue;
    }
#endif // SPARC || AMD64
#endif // _LP64

    // The internal word is used to form an address inside CodeCache
    // On SPARC (and other RISC processors), the internal word is composed
    // with two instructions. If an nmethod is suspended between those two
    // instructions we are not allowed to relocate the internal word.
   if (iter.type() != relocInfo::internal_word_type &&
       iter.type() != relocInfo::external_word_type) {
      iter.reloc()->fix_relocation_at_move(delta);
    }
  }
}
