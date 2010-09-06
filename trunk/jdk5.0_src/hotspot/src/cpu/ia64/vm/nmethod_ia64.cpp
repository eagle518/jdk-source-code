#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)nmethod_ia64.cpp	1.4 03/12/23 16:36:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_nmethod_ia64.cpp.incl"

void nmethod::deoptimize_nmethod(address active_return) {
#ifndef CORE
  ResourceMark rm;
  //
  // NOTE: only called at a safepoint. No thread is positioned in the
  // nmethod being deoptimized. We can do whatever we like with the
  // code body.
  //
  //
  //
  // We can be called to deoptimize a nmethod for as many active activations
  // that exist for the nmethod (at various active_return instruction). Some
  // of the mork must be done only the first time we apply the deoptimization
  // patches.

  if (!is_patched_for_deopt()) {
    //
    // No one will ever exeucte anything in this nmethod but the deopt patches
    // from this point forward. Clean all the inline caches now. No one will
    // touch them from this point on.

    clear_inline_caches();

    // Once we patch an nmethod it must be non-entrant because we have modified the
    // code in ways that make it unexecutable except for current activations.
    make_not_entrant();
  }

  // Apply a patch to the nmethod to take us to the deopt blob

  DeoptimizationBlob* deopt = SharedRuntime::deopt_blob();

  // First patch the active return address we have been given.
  // It is safe to do this multiple times.
  // We do it for each return address we find. Because return
  // address are always unique bundles and we have no delay
  // slot issues like sparc we can just patch each individual
  // return address like x86 and we're fine.

  int code_size = 1 * BytesPerInstWord;
  CodeBuffer* cb = new CodeBuffer(active_return, code_size + 1);
  MacroAssembler* a = new MacroAssembler(cb);

  // We call the trampoline hiding out at the end of the exception handler

  a->call(exception_begin() + 2 * BytesPerInstWord, relocInfo::none);
  a->flush();

  // Patch exception handler
  // Note on this path GR8 has the exception and GR9 has the original
  // call return pc (the exception pc)

  // Only need to patch the exception handler once
  if (!is_patched_for_deopt()) {

    // The exception handler is used for two purposes in deoptimization
    // First it dispatches exceptions to the deopt blob with the
    // pending exception. Second after that dispatch code is a trampoline
    // used by normal deopt returns to make up for the fact we can't
    // do a direct branch because of distance limitations.

    address exception_return = exception_begin();
    int code_size = 6 * BytesPerInstWord;
    CodeBuffer* cb = new CodeBuffer(exception_return, code_size + 1);
    MacroAssembler* a = new MacroAssembler(cb);
    // Must be a branch so that we stay in the deoptee register window
    a->mova(GR2_SCRATCH, deopt->unpack_with_exception(), relocInfo::runtime_call_type);
    a->eog();

    a->movi(BR7_SCRATCH, GR2_SCRATCH);
    a->br  (BR7_SCRATCH);

    a->flush_bundle();

    // Trampoline
    a->mova(GR2_SCRATCH, deopt->unpack(), relocInfo::runtime_call_type);
    a->eog();

    a->movi(BR7_SCRATCH, GR2_SCRATCH);
    a->br  (BR7_SCRATCH);
    a->flush();

  }

  // Notify the world that this nmethod is toast

  set_patched_for_deopt();

#endif /* CORE */

}


