#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)nmethod_sparc.cpp	1.3 03/12/23 16:37:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_nmethod_sparc.cpp.incl"

int nmethod::size_of_exception_handler() {
    return 2 * ( MacroAssembler::worst_case_size_of_set() + 1  + 1) * BytesPerInstWord;
}

void nmethod::deoptimize_nmethod(address active_return) {
#ifndef CORE
  ResourceMark rm;
  //
  // NOTE: only called at a safepoint. No thread is positioned in the
  // nmethod being deoptimized. We can do whatever we like with the
  // code body.
  //
  // We can be called to deoptimize a nmethod for as many active activations
  // that exist for the nmethod (at various active_return instruction). 
  // We take all of the normal code and nop from just after the verified entry
  // (which has the not-entrant patch) and all but the last two instructions
  // the last two instructions are a branch to the trampoline we place in
  // the exception handler and a nop.
  // Note: we can't use a normal nop because c1 runtime patching stubs examine the
  // code stream to determine if it has been deoptimized while in the vm to
  // resolve the patch. So we use an intruction the compiler would never
  // generate that has no effect on the state we need to carry.

  if (!is_patched_for_deopt()) {
    DeoptimizationBlob* deopt = SharedRuntime::deopt_blob();

    address trampoline;

    //
    // No one will ever exeucte anything in this nmethod but the deopt patches
    // from this point forward. Clean all the inline caches now. No one will
    // touch them from this point on.

    clear_inline_caches();

    // Once we patch an nmethod it must be non-entrant because we have modified the
    // code in ways that make it unexecutable except for current activations.
    make_not_entrant();


    {
    // First patch exception handler so we know where the trampoline is.
    // Note on this path O0 has the exception and O1 has the original
    // call return pc (the exception pc)

    // The exception handler is used for two purposes in deoptimization
    // First it dispatches exceptions to the deopt blob with the
    // pending exception. Second after that dispatch code is a trampoline
    // used by normal deopt returns to make up for the fact we can't
    // do a direct branch because of distance limitations.

    // Exception handler area gets two batches of instructions each identical
    // in size:  1st is a jump to the deopt blob to unpack with exception
    // and second is a trampoline for the normal deopt jump. We must always
    // know where trampoline is so we generate it first.

    address exception_return = exception_begin();

    CodeBuffer* ecb = new CodeBuffer(exception_return, size_of_exception_handler() + 1);
    MacroAssembler* e = new MacroAssembler(ecb);

    // Worst case 8 instructions,  will never relocate
    // DESTROY G2 so we keep all other allocatable registers live.
    e->set((intptr_t) deopt->unpack_with_exception(), G2);
    // 1 instruction
    e->jmp(G2, G0);
    // 1 instruction
    // On entry to the exception handler Oissuing_pc (O1) (
    // will have the address we were returning to. We went the
    // pc to look like it would if we were still in the call
    e->delayed()->sub(Oissuing_pc, frame::pc_return_offset, O7);

    // remember where the trampoline is located.

    trampoline = e->pc();

    // Now the trampoline for normal deoptimizations

    // Worst case 8 instructions will never relocate
    // DESTROY G2 so we keep all other allocatable registers live.
    e->set((intptr_t) deopt->unpack(), G2);
    // 1 instruction
    e->jmp(G2, G0);
    // 1 instruction
    e->delayed()->nop();


    }


    // Figure size of nmethod from just after verified entry patch to
    // the end of the nmethod.

    int code_size = (code_end() - verified_entry_point())  - 1*BytesPerInstWord;
    CodeBuffer* cb = new CodeBuffer(verified_entry_point() + BytesPerInstWord, code_size + 1);
    MacroAssembler* a = new MacroAssembler(cb);

    for (int i = 0 ; i < code_size - 2 * BytesPerInstWord ; i+=BytesPerInstWord) {
      a->sub(G0, G0, G0);
    }

    // Trampoline to the code we patch into the exception handler
    // that will do a normal deopt (i.e. no exception present).

    a->br(Assembler::always, false, Assembler::pt, trampoline );
    a->delayed()->nop();



    // Invalidate pretty much all of the nmethod

    ICache::invalidate_range(verified_entry_point() + BytesPerInstWord,
			     exception_end() - verified_entry_point() - BytesPerInstWord);
  }

  // Notify the world that this nmethod is toast

  set_patched_for_deopt();

#endif /* CORE */

}

