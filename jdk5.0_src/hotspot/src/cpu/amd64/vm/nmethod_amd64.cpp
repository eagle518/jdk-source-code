#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nmethod_amd64.cpp	1.3 03/12/23 16:35:52 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_nmethod_amd64.cpp.incl"

void nmethod::deoptimize_nmethod(address active_return)
{
#ifndef CORE
  ResourceMark rm;

  // NOTE: only called at a safepoint. No thread is positioned in the
  // nmethod being deoptimized. We can do whatever we like with the
  // code body.
  //
  //
  // We can be called to deoptimize a nmethod for as many active activations
  // that exist for the nmethod (at various active_return instruction). Some
  // of the mork must be done only the first time we apply the deoptimization
  // patches.

  if (!is_patched_for_deopt()) {
    // No one will ever execute anything in this nmethod but the deopt patches
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

  int code_size = NativeCall::instruction_size;
  CodeBuffer* cb = new CodeBuffer(active_return, code_size + 1);
  MacroAssembler* a = new MacroAssembler(cb);

  // Call the deopt blob, will never relocate.

  a->call(deopt->unpack(), relocInfo::none);

  ICache::invalidate_range(active_return, code_size);

  // Patch exception handler

  // frame/register state at exception handler
  // (return address removed)
  // rax: exception
  // rbx: exception handler
  // rdx: throwing pc

  // Only need to patch the exception handler once
  if (!is_patched_for_deopt()) {
    address exception_return = exception_begin();
    int code_size = NativeCall::instruction_size;
    CodeBuffer* cb = new CodeBuffer(exception_return, code_size + 1);
    MacroAssembler* a = new MacroAssembler(cb);
    a->jmp(deopt->unpack_with_exception(), relocInfo::none);

    ICache::invalidate_range(exception_return, code_size);
  }

  // Notify the world that this nmethod is toast
  set_patched_for_deopt();
#endif /* CORE */
}
