#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_MacroAssembler_sparc.cpp	1.45 03/12/23 16:37:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_MacroAssembler_sparc.cpp.incl"

void C1_MacroAssembler::inline_cache_check(Register receiver, Register iCache) {
  Label L;
  const Register temp_reg = G3_scratch;
  // Note: needs more testing of out-of-line vs. inline slow case
  Address ic_miss(temp_reg, Runtime1::entry_for(Runtime1::handle_ic_miss_id));
  verify_oop(receiver);
  ld_ptr(receiver, oopDesc::klass_offset_in_bytes(), temp_reg);
  cmp(temp_reg, iCache);
  brx(Assembler::equal, true, Assembler::pt, L);
  delayed()->nop();
  jump_to(ic_miss, 0);
  delayed()->nop();
  align(CodeEntryAlignment);
  bind(L);
}


void C1_MacroAssembler::method_exit(bool restore_frame) {
  // this code must be structured this way so that the return
  // instruction can be a safepoint.
  if (restore_frame) {
    restore();
  }
  relocate(relocInfo::return_type);
  retl();
  delayed()->nop();
}


void C1_MacroAssembler::explicit_null_check(Register base) {
  Unimplemented();
}


void C1_MacroAssembler::build_frame(int frame_size_in_bytes) {

  generate_stack_overflow_check(frame_size_in_bytes);
  // Create the frame.
  save_frame_c1(frame_size_in_bytes);
}


void C1_MacroAssembler::exception_handler(bool has_Java_exception_handler, int frame_size_in_bytes) {
  verify_oop(Oexception);
  if (has_Java_exception_handler) {
    // O0: exception
    // O1: issuing pc
    call(Runtime1::entry_for(Runtime1::handle_exception_id), relocInfo::runtime_call_type);
    delayed()->nop();
    // exception handler for particular exception doesn't exist
    // => unwind activation and forward exception to caller
    // O0: exception
  } else {
    // There is no handler in this method, we continue in caller
  }
  // unlock the receiver/klass if necessary
  // O0: exception
  verify_oop(Oexception);
}


void C1_MacroAssembler::fast_ObjectHashCode(Register receiver, Register result) {
  Label SlowerPath;
  Register header               = Gtemp;
  Register hash                 = Gtemp;  // overwrite header value with hash value
  Register mask                 = G1;  // to get hash field from header

  // Get the receiver's register.  Use outgoing_reg_location() because no SAVE has been issued.
  if (receiver == noreg) return;   // Don't bother with this optimization when the receiver is passed in memory.

  // Read the header and build a mask to get its hash field.  Give up if the object is not unlocked.
  // We depend on hash_mask being at most 32 bits and avoid the use of
  // hash_mask_in_place because it could be larger than 32 bits in a 64-bit
  // vm: see markOop.hpp.
  ld_ptr(receiver, oopDesc::mark_offset_in_bytes(), header);
  sethi(markOopDesc::hash_mask, mask);
  btst(markOopDesc::unlocked_value, header);
  br(Assembler::zero, false, Assembler::pn, SlowerPath);
  delayed()->or3(mask, low10(markOopDesc::hash_mask), mask);

  // Check for a valid (non-zero) hash code and get its value.
#ifdef _LP64
  srlx(header, markOopDesc::hash_shift, hash);
#else
  srl(header, markOopDesc::hash_shift, hash);
#endif
  andcc(hash, mask, hash);
  br(Assembler::equal, false, Assembler::pn, SlowerPath);
  delayed()->nop();

  // Safepoint leaf return.
  mov(hash, result);
  method_exit(false);
  bind(SlowerPath);
}


void C1_MacroAssembler::unverified_entry(Register receiver, Register ic_klass) {
  if (C1Breakpoint) breakpoint_trap();
  inline_cache_check(receiver, ic_klass);
}


void C1_MacroAssembler::verified_entry() {
  if (C1Breakpoint) breakpoint_trap();
  // build frame
  verify_FPU(0, "method_entry");
}


void C1_MacroAssembler::lock_object(Register Rmark, Register Roop, Register Rbox, Register Rscratch, Label& slow_case) {
  assert_different_registers(Rmark, Roop, Rbox, Rscratch);

  Label done;

  Address mark_addr(Roop, 0, oopDesc::mark_offset_in_bytes());

  // The following move must be the first instruction of emitted since debug
  // information may be generated for it.
  // Load object header
  ld_ptr(mark_addr, Rmark);

  verify_oop(Roop);

#ifdef ASSERT
  {
    Label ok;

    ld_ptr(mark_addr, Rmark);
    cmp(Rbox, Rmark);
    brx(Assembler::notEqual, false, Assembler::pt, ok);
    delayed()->nop();
    stop("bad recursive lock");
    should_not_reach_here();
    bind(ok);
  }
#endif

  // Save Rbox in Rscratch to be used for the cas operation
  mov(Rbox, Rscratch);
 
  // and mark it unlocked
  or3(Rmark, markOopDesc::unlocked_value, Rmark);

  // save unlocked object header into the displaced header location on the stack
  st_ptr(Rmark, Rbox, BasicLock::displaced_header_offset_in_bytes());
  // save object being locked into the BasicObjectLock
  st_ptr(Roop, Rbox, BasicObjectLock::obj_offset_in_bytes());

  // compare object markOop with Rmark and if equal exchange Rscratch with object markOop
  assert(mark_addr.disp() == 0, "cas must take a zero displacement");
  casx_under_lock(mark_addr.base(), Rmark, Rscratch, (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());
  // if compare/exchange succeeded we found an unlocked object and we now have locked it
  // hence we are done
  cmp(Rmark, Rscratch);
#ifdef _LP64
  sub(Rscratch, STACK_BIAS, Rscratch);
#endif
  brx(Assembler::equal, false, Assembler::pt, done);
  delayed()->sub(Rscratch, SP, Rscratch);  //pull next instruction into delay slot
  // we did not find an unlocked object so see if this is a recursive case
  // sub(Rscratch, SP, Rscratch);
  assert(os::vm_page_size() > 0xfff, "page size too small - change the constant");
  andcc(Rscratch, 0xfffff003, Rscratch);
  brx(Assembler::notZero, false, Assembler::pn, slow_case);
  delayed()->st_ptr(Rscratch, Rbox, BasicLock::displaced_header_offset_in_bytes());
  bind(done);
}


void C1_MacroAssembler::unlock_object(Register Rmark, Register Roop, Register Rbox, Label& slow_case) {
  assert_different_registers(Rmark, Roop, Rbox);

  Label done;
                         
  // Test first it it is a fast recursive unlock
  ld_ptr(Rbox, BasicLock::displaced_header_offset_in_bytes(), Rmark);
  br_null(Rmark, false, Assembler::pt, done);
  delayed()->nop();
  // load object
  ld_ptr(Rbox, BasicObjectLock::obj_offset_in_bytes(), Roop);
  verify_oop(Roop);
                         
  // Check if it is still a light weight lock, this is is true if we see
  // the stack address of the basicLock in the markOop of the object
  Address mark_addr(Roop, 0, oopDesc::mark_offset_in_bytes());
  assert(mark_addr.disp() == 0, "cas must take a zero displacement");
  casx_under_lock(mark_addr.base(), Rbox, Rmark, (address)StubRoutines::sparc::atomic_memory_operation_lock_addr());
  cmp(Rbox, Rmark);

  brx(Assembler::notEqual, false, Assembler::pn, slow_case);
  delayed()->nop();
  // Done
  bind(done);       
}


void C1_MacroAssembler::try_allocate(
  Register obj,                        // result: pointer to object after successful allocation
  Register var_size_in_bytes,          // object size in bytes if unknown at compile time; invalid otherwise
  int      con_size_in_bytes,          // object size in bytes if   known at compile time
  Register t1,                         // temp register
  Register t2,                         // temp register
  Label&   slow_case                   // continuation point if fast allocation fails
) {
  if (UseTLAB) {
    tlab_allocate(obj, var_size_in_bytes, con_size_in_bytes, t1, slow_case);
  } else {
    eden_allocate(obj, var_size_in_bytes, con_size_in_bytes, t1, t2, slow_case);
  }
}


void C1_MacroAssembler::initialize_header(Register obj, Register klass, Register len, Register tmp) {
  assert_different_registers(obj, klass, len, tmp);
  set((intx)markOopDesc::prototype(), tmp);
  st_ptr(tmp  , obj, oopDesc::mark_offset_in_bytes       ());
  st_ptr(klass, obj, oopDesc::klass_offset_in_bytes      ());
  if (len->is_valid()) st(len  , obj, arrayOopDesc::length_offset_in_bytes());
}


void C1_MacroAssembler::initialize_body(Register base, Register index) {
  assert_different_registers(base, index);
  Label loop;
  bind(loop);
  subcc(index, HeapWordSize, index);
  brx(Assembler::greaterEqual, true, Assembler::pt, loop);
  delayed()->st_ptr(G0, base, index);
}


void C1_MacroAssembler::allocate_object(
  Register obj,                        // result: pointer to object after successful allocation
  Register t1,                         // temp register
  Register t2,                         // temp register
  Register t3,                         // temp register
  int      hdr_size,                   // object header size in words
  int      obj_size,                   // object size in words 
  Register klass,                      // object klass
  Label&   slow_case                   // continuation point if fast allocation fails
) {
  assert_different_registers(obj, t1, t2, t3, klass);
  assert(klass == G5, "must be G5");
  
  // allocate space & initialize header
  if (!is_simm13(obj_size * wordSize)) {
    // would need to use extra register to load
    // object size => go the slow case for now
    br(Assembler::always, false, Assembler::pt, slow_case);
    delayed()->nop();
    return;
  }
  try_allocate(obj, noreg, obj_size * wordSize, t2, t3, slow_case);
  
  initialize_object(obj, klass, noreg, obj_size * HeapWordSize, t1, t2);
}

void C1_MacroAssembler::initialize_object(
  Register obj,                        // result: pointer to object after successful allocation
  Register klass,                      // object klass
  Register var_size_in_bytes,          // object size in bytes if unknown at compile time; invalid otherwise
  int      con_size_in_bytes,          // object size in bytes if   known at compile time
  Register t1,                         // temp register
  Register t2                          // temp register
  ) {
  const int hdr_size_in_bytes = oopDesc::header_size_in_bytes();

  initialize_header(obj, klass, noreg, t2);

#ifdef ASSERT
  {
    Label ok;
    ld(klass, klassOopDesc::header_size() * HeapWordSize + Klass::size_helper_offset_in_bytes(), t1);
    sll(t1, LogHeapWordSize, t1);
    if (var_size_in_bytes != noreg) {
      cmp(t1, var_size_in_bytes);
    } else {
      cmp(t1, con_size_in_bytes);
    }
    brx(Assembler::equal, false, Assembler::pt, ok);
    delayed()->nop();
    stop("bad size in initialize_object");
    should_not_reach_here();
    
    bind(ok);
  }
  
#endif
  
  // initialize body
  const int threshold = 5 * HeapWordSize;              // approximate break even point for code size
  if (var_size_in_bytes != noreg) {
    // use a loop
    add(obj, hdr_size_in_bytes, t1);               // compute address of first element
    sub(var_size_in_bytes, hdr_size_in_bytes, t2); // compute size of body
    initialize_body(t1, t2);
  } else if (VM_Version::v9_instructions_work() && con_size_in_bytes < threshold * 2) {
    // on v9 we can do double word stores to fill twice as much space.
#ifndef _LP64
    assert(hdr_size_in_bytes % 8 == 0, "double word aligned");
    assert(con_size_in_bytes % 8 == 0, "double word aligned");
#endif
    for (int i = hdr_size_in_bytes; i < con_size_in_bytes; i += 2 * HeapWordSize) stx(G0, obj, i);
  } else if (con_size_in_bytes <= threshold) {
    // use explicit NULL stores
    for (int i = hdr_size_in_bytes; i < con_size_in_bytes; i += HeapWordSize)     st_ptr(G0, obj, i);
  } else if (con_size_in_bytes > hdr_size_in_bytes) {
    // use a loop
    const Register base  = t1;
    const Register index = t2;
    add(obj, hdr_size_in_bytes, base);               // compute address of first element
    // compute index = number of words to clear
    set(con_size_in_bytes - hdr_size_in_bytes, index);
    initialize_body(base, index);
  }
  verify_oop(obj);
}


void C1_MacroAssembler::allocate_array(
  Register obj,                        // result: pointer to array after successful allocation
  Register len,                        // array length
  Register t1,                         // temp register
  Register t2,                         // temp register
  Register t3,                         // temp register
  int      hdr_size,                   // object header size in words
  int      elt_size,                   // element size in bytes 
  Register klass,                      // object klass
  Label&   slow_case                   // continuation point if fast allocation fails
) {
  assert_different_registers(obj, len, t1, t2, t3, klass);
  assert(klass == G5, "must be G5");
  assert(t1 == G1, "must be G1");

  // determine alignment mask
  assert(!(BytesPerWord & 1), "must be a multiple of 2 for masking code to work");

  // check for negative or excessive length
  // note: the maximum length allowed is chosen so that arrays of any
  //       element size with this length are always smaller or equal
  //       to the largest integer (i.e., array size computation will
  //       not overflow)
  const int max_length = 0x01000000; // sparc friendly value, requires sethi only
  set(max_length, t1);
  cmp(len, t1);
  br(Assembler::greaterUnsigned, false, Assembler::pn, slow_case);
   
  // compute array size
  // note: if 0 <= len <= max_length, len*elt_size + header + alignment is
  //       smaller or equal to the largest integer; also, since top is always
  //       aligned, we can do the alignment here instead of at the end address
  //       computation
  const Register arr_size = t1;
  switch (elt_size) {
    case  1: delayed()->mov(len,    arr_size); break;
    case  2: delayed()->sll(len, 1, arr_size); break;
    case  4: delayed()->sll(len, 2, arr_size); break;
    case  8: delayed()->sll(len, 3, arr_size); break;
    default: ShouldNotReachHere();
  }
  add(arr_size, hdr_size * wordSize + MinObjAlignmentInBytesMask, arr_size); // add space for header & alignment
  and3(arr_size, ~MinObjAlignmentInBytesMask, arr_size);                     // align array size
 
  // allocate space & initialize header
  if (UseTLAB) {
    tlab_allocate(obj, arr_size, 0, t2, slow_case);
  } else {
    eden_allocate(obj, arr_size, 0, t2, t3, slow_case);
  }
  initialize_header(obj, klass, len, t2);

  // initialize body
  const Register base  = t2;
  const Register index = t3;
  add(obj, hdr_size * wordSize, base);               // compute address of first element
  sub(arr_size, hdr_size * wordSize, index);         // compute index = number of words to clear
  initialize_body(base, index);
  verify_oop(obj);
}
