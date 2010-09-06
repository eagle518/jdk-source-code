#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)nativeInst_ia64.cpp	1.14 04/02/11 19:01:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_nativeInst_ia64.cpp.incl"


// Similar to replace_mt_safe, but just changes the destination.  The
// important thing is that free-running threads are able to execute this
// call instruction at all times.  Thus, the displacement field must be
// instruction-word-aligned.
//
// Used in the runtime linkage of calls; see class CompiledIC.
void NativeCall::set_destination_mt_safe(address dest) {

  assert(Patching_lock->is_locked() ||
         SafepointSynchronize::is_at_safepoint(), "concurrent code patching"); 
  // Get the address of the bundle containing the call
  IPF_Bundle *bundle = (IPF_Bundle*)addr_at(0);

  // Generate the bits for a "chk.a.nc GR0, .+0", which always branches to self
  M22 check(4 | Assembler::keep, GR0, 0, PR0);

  // Loop until the change is accomplished
  while (true) {
    uint41_t new_X, new_L;

    // verify that this is a movl
    guarantee( Assembler::is_movl( bundle->get_template(), bundle->get_slot2() ), "not a movl instruction");
  
    // Save the old bundle, and make an image that is updated
    IPF_Bundle old_bundle = *bundle;
    IPF_Bundle mid_bundle = old_bundle;
    IPF_Bundle new_bundle = old_bundle;
  
    // Change the middle bundle so that the 0 slot instruction branchs to self
    mid_bundle.set_slot0( check.bits() );
  
    // Update the new image
    X2::set_imm((uint64_t)dest, new_bundle.get_slot2(), new_X, new_L);
    new_bundle.set_slot1( new_L );
    new_bundle.set_slot2( new_X );
  
    // Now the synchronous work begins: get the halves
    uint64_t old_half0 = old_bundle.get_half0();
  
    // Exchange the low order half, verify it was unchanged, and retry if it was different
    int64_t cur_half0 = Atomic::cmpxchg((jlong)mid_bundle.get_half0(),
					(volatile jlong*)bundle->addr_half0(),
					(jlong)old_half0);

    if( cur_half0 == old_half0 ) {
  
      // Force a memory barrier
      OrderAccess::fence();
  
      // Write the upper half with the changed bits
      bundle->set_half1(new_bundle.get_half1());
  
      // Force a memory barrier
      OrderAccess::fence();
  
      // Write the lower half
      bundle->set_half0(new_bundle.get_half0());
  
      // Final memory barries
      OrderAccess::fence();
  
      break;
    }
  }

  ICache::invalidate_range((address)bundle, sizeof(bundle));
}

void NativeCall::verify() const {
  assert( is_movl(), "not a movl instruction");
}


void NativeInstruction::verify() const {
  // make sure code pattern is actually an instruction address
  address addr = addr_at(0);
  if (addr == 0 || ((intptr_t)addr & 7) != 0) {
    fatal("not an instruction address");
  }
}

bool NativeInstruction::is_call() const {

  // Must start with a "movl" to load the address
  IPF_Bundle *bundle0 = (IPF_Bundle*)addr_at(0);
  if( !Assembler::is_movl( bundle0->get_template(), bundle0->get_slot2() ) )
    return false;

// More accurate test required later
#if 0
  IPF_Bundle *bundle1 = (IPF_Bundle*)addr_at(sizeof(IPF_Bundle));
  if( !Assembler::is_call_indirect( bundle0->get_template(), bundle0->get_slot2() ) )
    return false;
#endif

  return true;
}

bool NativeInstruction::is_illegal() const {
  // The VM does not use illegal instructions on ia64 so return false;
  // The signal handler will identify the problem and abort the VM
  return false;
}

bool NativeInstruction::is_movl() const {

  IPF_Bundle *bundle0 = (IPF_Bundle*)addr_at(0);
  if( !Assembler::is_movl( bundle0->get_template(), bundle0->get_slot2() ) )
    return false;

  return true;
}

bool NativeInstruction::is_nop() const {
  Unimplemented();
  return true;
}

//-------------------------------------------------------------------

void NativeMovConstReg::verify() const {
  assert( is_movl(), "not a movl instruction");
}

//--------------------------------------------------------------------------------

void NativeJump::verify() const {
  assert( is_movl(), "not a movl instruction");
}

void NativeMovConstReg::set_data(int64_t src)  {
  verify();
  uint41_t new_X;
  uint41_t new_L;
  IPF_Bundle *bundle = (IPF_Bundle *)addr_at(0);
  X2::set_imm((uint64_t)src, bundle->get_slot2(), new_X, new_L);
  bundle->set_slot1( new_L );
  bundle->set_slot2( new_X );

  ICache::invalidate_range((address)bundle, sizeof(bundle));

#ifndef CORE
  // also store the value into an oop_Relocation cell, if any
  CodeBlob* nm = CodeCache::find_blob(instruction_address());
  if (nm != NULL) {
    RelocIterator iter(nm, instruction_address(), next_instruction_address());
    oop* oop_addr = NULL;
    while (iter.next()) {
      if (iter.type() == relocInfo::oop_type) {
        oop_Relocation *r = iter.oop_reloc();
        if (oop_addr == NULL) {
          oop_addr = r->oop_addr();
          *oop_addr = (oop)src;
        } else {
          assert(oop_addr == r->oop_addr(), "must be only one set-oop here") ;
        }
      }
    }
  }
#endif
}

void NativeJump::patch_verified_entry(address entry, address verified_entry, address dest) {
  uint41_t new_X;
  uint41_t new_L;

  // Not even remotely MT safe
  IPF_Bundle *bundle = (IPF_Bundle *)(nativeInstruction_at(verified_entry)->addr_at(0));

  M37 nopfill(1, 0, PR0);
  X3 branch(0, (uint) Assembler::sptk, (uint) Assembler::few, (uint) Assembler::keep, 0, PR0);

  branch.set_target((uint64_t)(dest - verified_entry), branch.bits(), new_X, new_L);

  bundle->set(IPF_Bundle::MLX, nopfill.bits(), new_L, new_X);

  ICache::invalidate_range((address)bundle, sizeof(bundle));
}
