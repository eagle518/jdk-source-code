/*
 * Copyright (c) 2003, 2007, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2008, 2009, 2010 Red Hat, Inc.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime_zero.cpp.incl"

DeoptimizationBlob *SharedRuntime::_deopt_blob;
SafepointBlob      *SharedRuntime::_polling_page_safepoint_handler_blob;
SafepointBlob      *SharedRuntime::_polling_page_return_handler_blob;
RuntimeStub        *SharedRuntime::_wrong_method_blob;
RuntimeStub        *SharedRuntime::_ic_miss_blob;
RuntimeStub        *SharedRuntime::_resolve_opt_virtual_call_blob;
RuntimeStub        *SharedRuntime::_resolve_virtual_call_blob;
RuntimeStub        *SharedRuntime::_resolve_static_call_blob;

int SharedRuntime::java_calling_convention(const BasicType *sig_bt,
                                           VMRegPair *regs,
                                           int total_args_passed,
                                           int is_outgoing) {
  return 0;
}

AdapterHandlerEntry* SharedRuntime::generate_i2c2i_adapters(
                        MacroAssembler *masm,
                        int total_args_passed,
                        int comp_args_on_stack,
                        const BasicType *sig_bt,
                        const VMRegPair *regs,
                        AdapterFingerPrint *fingerprint) {
  return AdapterHandlerLibrary::new_entry(
    fingerprint,
    ShouldNotCallThisStub(),
    ShouldNotCallThisStub(),
    ShouldNotCallThisStub());
}

nmethod *SharedRuntime::generate_native_wrapper(MacroAssembler *masm,
                                                methodHandle method,
                                                int total_in_args,
                                                int comp_args_on_stack,
                                                BasicType *in_sig_bt,
                                                VMRegPair *in_regs,
                                                BasicType ret_type) {
#ifdef SHARK
  return SharkCompiler::compiler()->generate_native_wrapper(masm,
                                                            method,
                                                            in_sig_bt,
                                                            ret_type);
#else
  ShouldNotCallThis();
#endif // SHARK
}

int Deoptimization::last_frame_adjust(int callee_parameters,
                                      int callee_locals) {
  return 0;
}

uint SharedRuntime::out_preserve_stack_slots() {
  ShouldNotCallThis();
}

static RuntimeStub* generate_empty_runtime_stub(const char* name) {
  CodeBuffer buffer(name, 0, 0);
  return RuntimeStub::new_runtime_stub(name, &buffer, 0, 0, NULL, false);
}

static SafepointBlob* generate_empty_safepoint_blob() {
  CodeBuffer buffer("handler_blob", 0, 0);
  return SafepointBlob::create(&buffer, NULL, 0);
}

void SharedRuntime::generate_stubs() {
  _wrong_method_blob =
    generate_empty_runtime_stub("wrong_method_stub");
  _ic_miss_blob =
    generate_empty_runtime_stub("ic_miss_stub");
  _resolve_opt_virtual_call_blob =
    generate_empty_runtime_stub("resolve_opt_virtual_call");
  _resolve_virtual_call_blob =
    generate_empty_runtime_stub("resolve_virtual_call");
  _resolve_static_call_blob =
    generate_empty_runtime_stub("resolve_static_call");

  _polling_page_safepoint_handler_blob =
    generate_empty_safepoint_blob();
  _polling_page_return_handler_blob =
    generate_empty_safepoint_blob();
}

int SharedRuntime::c_calling_convention(const BasicType *sig_bt,
                                         VMRegPair *regs,
                                         int total_args_passed) {
  ShouldNotCallThis();
}
