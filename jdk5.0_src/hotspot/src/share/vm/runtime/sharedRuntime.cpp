#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)sharedRuntime.cpp	1.328 04/05/21 12:32:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_sharedRuntime.cpp.incl"
#include <math.h>


// Implementation of SharedRuntime

SharedRuntime::StrictMathFunction_DD_D SharedRuntime::java_lang_strict_math_pow = NULL;

#ifndef PRODUCT
#ifndef CORE
// For statistics
int SharedRuntime::_ic_miss_ctr = 0;
int SharedRuntime::_wrong_method_ctr = 0;
int SharedRuntime::_resolve_static_ctr = 0;
int SharedRuntime::_resolve_virtual_ctr = 0;
int SharedRuntime::_resolve_opt_virtual_ctr = 0;
int SharedRuntime::_implicit_null_throws = 0;
int SharedRuntime::_implicit_div0_throws = 0;

int     SharedRuntime::_ICmiss_index                    = 0;
int     SharedRuntime::_ICmiss_count[SharedRuntime::maxICmiss_count];
address SharedRuntime::_ICmiss_at[SharedRuntime::maxICmiss_count];

void SharedRuntime::trace_ic_miss(address at) {
  for (int i = 0; i < _ICmiss_index; i++) {
    if (_ICmiss_at[i] == at) {
      _ICmiss_count[i]++; 
      return;
    }
  }
  int index = _ICmiss_index++; 
  if (_ICmiss_index >= maxICmiss_count) _ICmiss_index = maxICmiss_count - 1;
  _ICmiss_at[index] = at; 
  _ICmiss_count[index] = 1;
}

void SharedRuntime::print_ic_miss_histogram() {
  if (ICMissHistogram) {
    tty->print_cr ("IC Miss Histogram:");
    int tot_misses = 0;
    for (int i = 0; i < _ICmiss_index; i++) {
      tty->print_cr("  at: " INTPTR_FORMAT "  nof: %d", _ICmiss_at[i], _ICmiss_count[i]);
      tot_misses += _ICmiss_count[i];
    }
    tty->print_cr ("Total IC misses: %7d", tot_misses);
  }
}
#endif // CORE
#endif // PRODUCT


void SharedRuntime::lookup_function_DD_D(StrictMathFunction_DD_D& f, const char* fname) {
  if (f == NULL) f = CAST_TO_FN_PTR(StrictMathFunction_DD_D, NativeLookup::base_library_lookup("java/lang/StrictMath", fname, "(DD)D"));
  assert(f != NULL, "lookup failed");
}

void SharedRuntime::initialize_StrictMath_entry_points() {
  assert(!Universe::is_jdk12x_version(), "entry points do not exist for JDK 1.2");
  lookup_function_DD_D(java_lang_strict_math_pow, "pow");
}

JRT_LEAF(jdouble, SharedRuntime::dpow(jdouble x, jdouble y))
  assert(java_lang_strict_math_pow != NULL, "pow entry point must exist");
  return (*java_lang_strict_math_pow)(NULL, NULL, x, y);
JRT_END


JRT_LEAF(jlong, SharedRuntime::lmul(jlong y, jlong x))
  return x * y;
JRT_END


JRT_LEAF(jlong, SharedRuntime::ldiv(jlong y, jlong x))
  if (x == min_jlong && y == CONST64(-1)) {
    return x;
  } else {
    return x / y;
  }
JRT_END


JRT_LEAF(jlong, SharedRuntime::lrem(jlong y, jlong x))
  if (x == min_jlong && y == CONST64(-1)) {
    return 0;
  } else {
    return x % y;
  }
JRT_END


const juint  float_sign_mask  = 0x7FFFFFFF;
const juint  float_infinity   = 0x7F800000;
const julong double_sign_mask = CONST64(0x7FFFFFFFFFFFFFFF);
const julong double_infinity  = CONST64(0x7FF0000000000000);

JRT_LEAF(jfloat, SharedRuntime::frem(jfloat  x, jfloat  y))
#ifdef _WIN64
  // 64-bit Windows on amd64 returns the wrong values for
  // infinity operands.
  union { jfloat f; juint i; } xbits, ybits;
  xbits.f = x;
  ybits.f = y;
  // x Mod Infinity == x unless x is infinity
  if ( ((xbits.i & float_sign_mask) != float_infinity) &&
       ((ybits.i & float_sign_mask) == float_infinity) ) {
    return x;
  }
#endif
  return ((jfloat)fmod((double)x,(double)y));
JRT_END


JRT_LEAF(jdouble, SharedRuntime::drem(jdouble x, jdouble y))
#ifdef _WIN64
  union { jdouble d; julong l; } xbits, ybits;
  xbits.d = x;
  ybits.d = y;
  // x Mod Infinity == x unless x is infinity
  if ( ((xbits.l & double_sign_mask) != double_infinity) &&
       ((ybits.l & double_sign_mask) == double_infinity) ) {
    return x;
  }
#endif
  return ((jdouble)fmod((double)x,(double)y));
JRT_END


JRT_LEAF(jint, SharedRuntime::f2i(jfloat  x))
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  jint ltmp   = (jint)lltmp;
  if (ltmp == lltmp) {
    return ltmp;
  } else {
    if (x < 0) {
      return min_jint;
    } else {
      return max_jint;
    }
  }
JRT_END


JRT_LEAF(jlong, SharedRuntime::f2l(jfloat  x))  
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  if (lltmp != min_jlong) {
    return lltmp;
  } else {
    if (x < 0) {
      return min_jlong;
    } else {
      return max_jlong;
    }
  }
JRT_END


JRT_LEAF(jint, SharedRuntime::d2i(jdouble x))
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  jint ltmp   = (jint)lltmp;
  if (ltmp == lltmp) {
    return ltmp;
  } else {
    if (x < 0) {
      return min_jint;
    } else {
      return max_jint;
    }
  }
JRT_END


JRT_LEAF(jlong, SharedRuntime::d2l(jdouble x))
  if (g_isnan(x)) {return 0;}
  jlong lltmp = (jlong)x;
  if (lltmp != min_jlong) {
    return lltmp;
  } else {
    if (x < 0) {
      return min_jlong;
    } else {
      return max_jlong;
    }
  }
JRT_END


JRT_LEAF(jfloat, SharedRuntime::d2f(jdouble x))
  return (jfloat)x;
JRT_END


JRT_LEAF(jfloat, SharedRuntime::l2f(jlong x))
  return (jfloat)x;
JRT_END


JRT_LEAF(jdouble, SharedRuntime::l2d(jlong x))
  return (jdouble)x;
JRT_END

// Exception handling accross interpreter/compiler boundaries
//
// exception_handler_for_return_address(...) returns the continuation address.
// The continuation address is the entry point of the exception handler of the
// previous frame depending on the return address.

address SharedRuntime::raw_exception_handler_for_return_address(address return_address) {


  assert(frame::verify_return_pc(return_address), "must be a return pc");


#ifdef COMPILER1
  // the fastest case first
  CodeBlob* blob = CodeCache::find_blob(return_address);
  if (blob != NULL && blob->is_nmethod()) {
    nmethod* code = (nmethod*)blob;
    assert(code != NULL, "nmethod must be present");
    assert(code->header_begin() != code->exception_begin(), "no exception handler");
    return code->exception_begin();
  } 
#endif // COMPILER1

  
  // Entry code
  if (StubRoutines::returns_to_call_stub(return_address)) {
    return StubRoutines::catch_exception_entry();
  }
  // Interpreted code
  if (Interpreter::contains(return_address)) {
    return Interpreter::rethrow_exception_entry();
  }
#ifdef COMPILER1
  // Handle runtime stubs before compiled methods

  // OSR adapter
  if (blob != NULL && blob->is_osr_adapter()) {
    return Runtime1::entry_for(Runtime1::osr_unwind_exception_id);
  }

  // alignment adapter
  if (Runtime1::entry_for(Runtime1::alignment_frame_return_id) == return_address) {
    return Runtime1::entry_for(Runtime1::unwind_exception_id);
  }

#endif // COMPILER1
#ifndef CORE
  // Compiled code
  if (CodeCache::contains(return_address)) {
    CodeBlob* blob = CodeCache::find_blob(return_address);
    if (blob->is_nmethod()) {
      nmethod* code = (nmethod*)blob;
      assert(code != NULL, "nmethod must be present");
      assert(code->header_begin() != code->exception_begin(), "no exception handler");
      return code->exception_begin();
    } 
    if (blob->is_runtime_stub()) {
      ShouldNotReachHere();   // callers are responsible for skipping runtime stub frames
    }
#ifdef COMPILER2
    if (blob->is_c2i_adapter()) {
      return OptoRuntime::exception_blob()->instructions_begin();
    } 
    if (blob->is_i2c_adapter()) {
      return OptoRuntime::exception_blob()->instructions_begin();
    } 
    if (blob->is_osr_adapter()) {
      return OptoRuntime::exception_blob()->instructions_begin();
    }
#endif // COMPILER2
  }
  guarantee(!VtableStubs::contains(return_address), "NULL exceptions in vtables should have been handled already!");
#endif // CORE
#ifndef PRODUCT
  { ResourceMark rm;
    tty->print_cr("No exception handler found for exception at " INTPTR_FORMAT " - potential problems:", return_address);
    tty->print_cr("a) exception happened in (new?) code stubs/buffers that is not handled here");
    tty->print_cr("b) other problem");
  }
#endif // PRODUCT
  ShouldNotReachHere();
  return NULL;
}


JRT_LEAF(address, SharedRuntime::exception_handler_for_return_address(address return_address))
  JavaThread* thread = JavaThread::current();
#ifndef CORE
  address ra = thread->safepoint_state()->compute_adjusted_pc(return_address);  
#else
  address ra = return_address;  
#endif
  return raw_exception_handler_for_return_address(ra);
JRT_END


oop SharedRuntime::retrieve_receiver( symbolHandle sig, frame caller ) {
  assert(caller.is_interpreted_frame(), "");
  int args_size = ArgumentSizeComputer(sig).size() + 1;
  assert(args_size <= caller.interpreter_frame_expression_stack_size(), "receiver must be on interpreter stack");
  oop result = (oop) caller.interpreter_frame_tos_at(args_size - 1);
  assert(Universe::heap()->is_in(result) && result->is_oop(), "receiver must be an oop");
  return result;
}


void SharedRuntime::throw_and_post_jvmti_exception(JavaThread *thread, Handle h_exception) {
  if (JvmtiExport::can_post_exceptions()) {
    vframeStream vfst(thread, true);
    methodHandle method = methodHandle(thread, vfst.method());
    address bcp = method()->bcp_from(vfst.bci());
    JvmtiExport::post_exception_throw(thread, method(), bcp, h_exception());
  }
  Exceptions::_throw(thread, __FILE__, __LINE__, h_exception);
}

void SharedRuntime::throw_and_post_jvmti_exception(JavaThread *thread, symbolOop name, const char *message) {
  Handle h_exception = Exceptions::new_exception(thread, name, message);
  throw_and_post_jvmti_exception(thread, h_exception);
}

JRT_ENTRY(void, SharedRuntime::throw_AbstractMethodError(JavaThread* thread))
  // These errors occur only at call sites
  throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_AbstractMethodError());
JRT_END

JRT_ENTRY(void, SharedRuntime::throw_ArithmeticException(JavaThread* thread))
  throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_ArithmeticException(), "/ by zero");
JRT_END

JRT_ENTRY(void, SharedRuntime::throw_NullPointerException(JavaThread* thread))
  throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_NullPointerException());
JRT_END

JRT_ENTRY(void, SharedRuntime::throw_NullPointerException_at_call(JavaThread* thread))
  // This entry point is effectively only used for NullPointerExceptions which occur at inline
  // cache sites (when the callee activation is not yet set up) so we are at a call site
  throw_and_post_jvmti_exception(thread, vmSymbols::java_lang_NullPointerException());
JRT_END

JRT_ENTRY(void, SharedRuntime::throw_StackOverflowError(JavaThread* thread))
  // We avoid using the normal exception construction in this case because
  // it performs an upcall to Java, and we're already out of stack space.
  klassOop k = SystemDictionary::StackOverflowError_klass();
  oop exception_oop = instanceKlass::cast(k)->allocate_instance(CHECK);
  Handle exception (thread, exception_oop);
  if (StackTraceInThrowable) {
    java_lang_Throwable::fill_in_stack_trace(exception);
  }
  throw_and_post_jvmti_exception(thread, exception);
JRT_END

address SharedRuntime::continuation_for_implicit_exception(JavaThread* thread,
                                                           address faulting_pc,
                                                           SharedRuntime::ImplicitExceptionKind exception_kind)
{
  // compute the corrected pc (if the exception happened in a temporary code
  // buffer used for safepoint synchronization, we need to adjust it first)
#ifdef CORE
  address pc = faulting_pc;
#else
  address pc = thread->safepoint_state()->compute_adjusted_pc(faulting_pc);
#endif
  address target_pc = NULL;

  if (Interpreter::contains(pc)) {
#ifdef CC_INTERP
    // C++ interpreter doesn't throw implicit exceptions
    ShouldNotReachHere();
#else
    switch (exception_kind) {
      case IMPLICIT_NULL:           return Interpreter::throw_NullPointerException_entry();
      case IMPLICIT_DIVIDE_BY_ZERO: return Interpreter::throw_ArithmeticException_entry();
      case STACK_OVERFLOW:          return Interpreter::throw_StackOverflowError_entry();
      default:                      ShouldNotReachHere();
    }
#endif // !CC_INTERP
  } else {
    switch (exception_kind) {
      case STACK_OVERFLOW: {
        // Stack overflow only occurs upon frame setup; the callee is
        // going to be unwound. Dispatch to a shared runtime stub
        // which will cause the StackOverflowError to be fabricated
        // and processed.
        return StubRoutines::throw_StackOverflowError_entry();
      }

      case IMPLICIT_NULL: {
#ifndef CORE
        if (VtableStubs::contains(pc)) {
          // We haven't yet entered the callee frame. Fabricate an
          // exception and begin dispatching it in the caller. Since
          // the caller was at a call site, it's safe to destroy all
          // caller-saved registers, as these entry points do.
          VtableStub* vt_stub = VtableStubs::stub_containing(pc);
          guarantee(vt_stub != NULL, "unable to find SEGVing vtable stub");
          if (vt_stub->is_abstract_method_error(pc)) {
            assert(!vt_stub->is_vtable_stub(), "should never see AbstractMethodErrors from vtable-type VtableStubs");
            return StubRoutines::throw_AbstractMethodError_entry();
          } else {
            return StubRoutines::throw_NullPointerException_at_call_entry();
          }
        } else {
          CodeBlob* cb = CodeCache::find_blob(pc);
          guarantee(cb != NULL, "exception happened outside interpreter, nmethods and vtable stubs (1)");

          // Exception happened in CodeCache. Must be either:
          // 1. Inline-cache check in C2I adapter,
          // 2. Inline-cache check in nmethod, or
          // 3. Implict null exception in nmethod
          
#ifdef COMPILER2
          if (!cb->is_nmethod()) {
            if (OptoRuntime::uncommon_trap_blob() != NULL && OptoRuntime::uncommon_trap_blob()->contains(pc)) {
              ShouldNotReachHere();
            }
            guarantee((cb->is_c2i_adapter() && ((C2IAdapter*)cb)->inlinecache_check_contains(pc)),
                      "exception happened outside interpreter, nmethods and vtable stubs (2)");

            // There is no handler here, so we will simply unwind.
            return StubRoutines::throw_NullPointerException_at_call_entry();
          }
#endif  // COMPILER2

          // Otherwise, it's an nmethod.  Consult its exception handlers.
          nmethod* nm = (nmethod*)cb;
          if (nm->inlinecache_check_contains(pc)) {
            // exception happened inside inline-cache check code
            // => the nmethod is not yet active (i.e., the frame
            // is not set up yet) => use return address pushed by
            // caller => don't push another return address
            return StubRoutines::throw_NullPointerException_at_call_entry();
          }

#ifndef PRODUCT
          _implicit_null_throws++;
#endif
          target_pc = nm->continuation_for_implicit_exception(pc);
          guarantee(target_pc != 0, "must have a continuation point");
        }
#endif /* !CORE */

        break; // fall through
      }


      case IMPLICIT_DIVIDE_BY_ZERO: {
#ifndef CORE
        nmethod* nm = CodeCache::find_nmethod(pc);
        guarantee(nm != NULL, "must have containing nmethod for implicit division-by-zero exceptions");
#ifndef PRODUCT
        _implicit_div0_throws++;
#endif
        target_pc = nm->continuation_for_implicit_exception(pc);
        guarantee(target_pc != 0, "must have a continuation point");
#endif /* !CORE */
        break; // fall through
      }

      default: ShouldNotReachHere();
    }

#ifndef CORE
    // If a safepoint was in progress, we need to redirect
    // the target pc into the ThreadCodeBuffer.
    guarantee(target_pc != NULL, "must have computed destination PC for implicit exception");
    target_pc = thread->safepoint_state()->maybe_capture_pc(target_pc);
    assert(thread->safepoint_state()->code_buffer() != NULL || CodeCache::contains(target_pc), "bad target_pc (1)");
    assert(thread->safepoint_state()->code_buffer() == NULL || thread->safepoint_state()->code_buffer()->contains(target_pc), "bad target_pc (2)");
    assert(exception_kind == IMPLICIT_NULL || exception_kind == IMPLICIT_DIVIDE_BY_ZERO, "wrong implicit exception kind");
    // for AbortVMOnException flag
    NOT_PRODUCT(Exceptions::debug_check_abort("java.lang.NullPointerException"));
    if (exception_kind == IMPLICIT_NULL) {
      Events::log("Implicit null exception at " INTPTR_FORMAT " to " INTPTR_FORMAT, faulting_pc, target_pc);
    } else {
      Events::log("Implicit division by zero exception at " INTPTR_FORMAT " to " INTPTR_FORMAT, faulting_pc, target_pc);
    }
    return target_pc;
#endif /* !CORE */
  }

  ShouldNotReachHere();
  return NULL;
}


JNI_ENTRY(void, throw_unsatisfied_link_error(JNIEnv* env, ...))
{
  THROW(vmSymbols::java_lang_UnsatisfiedLinkError());
}
JNI_END


address SharedRuntime::native_method_throw_unsatisfied_link_error_entry() {
  return CAST_FROM_FN_PTR(address, &throw_unsatisfied_link_error);
}


// SharedRuntime::trace_bytecode is only used by the TraceBytecodes and
// EnableJVMPIInstructionStartEvent options. When JVM/PI is retired,
// this entire routine can be made '#ifndef PRODUCT'.
JRT_ENTRY(intptr_t, SharedRuntime::trace_bytecode(JavaThread* thread, intptr_t preserve_this_value, intptr_t tos, intptr_t tos2))
  const frame f = thread->last_frame();
  assert(f.is_interpreted_frame(), "must be an interpreted frame");
#ifndef PRODUCT
  methodHandle mh(THREAD, f.interpreter_frame_method());
  BytecodeTracer::trace(mh, f.interpreter_frame_bcp(), tos, tos2);
#endif // !PRODUCT
  if (EnableJVMPIInstructionStartEvent && jvmpi::is_event_enabled(JVMPI_EVENT_INSTRUCTION_START)) {
    jvmpi::post_instruction_start_event(f);
  }
  return preserve_this_value;
JRT_END


JRT_ENTRY(void, SharedRuntime::yield_all(JavaThread* thread, int attempts))
  os::yield_all(attempts);
JRT_END


// ---------------------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void SharedRuntime::verify_caller_frame(frame caller_frame, methodHandle callee_method) {
  ResourceMark rm;  
  assert (caller_frame.is_interpreted_frame(), "sanity check");
  assert (callee_method->has_compiled_code(), "callee must be compiled");  
  methodHandle caller_method (Thread::current(), caller_frame.interpreter_frame_method());
  jint bci = caller_frame.interpreter_frame_bci();
  methodHandle method = find_callee_method_inside_interpreter(caller_frame, caller_method, bci);
  assert (callee_method == method, "incorrect method");
}

methodHandle SharedRuntime::find_callee_method_inside_interpreter(frame caller_frame, methodHandle caller_method, int bci) {
  EXCEPTION_MARK;
  Bytecode_invoke* bytecode = Bytecode_invoke_at(caller_method, bci);
  methodHandle staticCallee = bytecode->static_target(CATCH); // Non-product code
  
  bytecode = Bytecode_invoke_at(caller_method, bci);
  int bytecode_index = bytecode->index();
  Bytecodes::Code bc = bytecode->adjusted_invoke_code();      

  Handle receiver;
  if (bc == Bytecodes::_invokeinterface ||
      bc == Bytecodes::_invokevirtual ||
      bc == Bytecodes::_invokespecial) {
    symbolHandle signature (THREAD, staticCallee->signature());
    receiver = Handle(THREAD, retrieve_receiver(signature, caller_frame));
  } else {
    receiver = Handle();
  }
  CallInfo result;
  constantPoolHandle constants (THREAD, caller_method->constants());
  LinkResolver::resolve_invoke(result, receiver, constants, bytecode_index, bc, CATCH); // Non-product code
  methodHandle calleeMethod = result.selected_method();
  return calleeMethod;
}

#endif 

// This is factored because it is also called from Runtime1.
void SharedRuntime::jvmpi_method_entry_work(JavaThread* thread, methodOop method, oop receiver) {
  GC_locker::lock();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY2)) {
    jvmpi::post_method_entry2_event(method, receiver);
  } 
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_ENTRY)) {
    jvmpi::post_method_entry_event(method);
  } 
  GC_locker::unlock();
}


// Must be entry as it may lock when acquring the jmethodID of the method
JRT_ENTRY (void,  SharedRuntime::jvmpi_method_entry(JavaThread* thread, methodOop method, oop receiver))
  jvmpi_method_entry_work(thread, method, receiver);
JRT_END


// Must be entry as it may lock when acquring the jmethodID of the method
JRT_ENTRY (void, SharedRuntime::jvmpi_method_exit(JavaThread* thread, methodOop method))
#ifdef COMPILER1
#ifdef ASSERT
  frame stub_fr = thread->last_frame();
  CodeBlob* stub_cb = CodeCache::find_blob(stub_fr.pc());  
  bool stub_is_jvmpi_method_exit = stub_cb != NULL &&
                                   stub_cb->instructions_begin() ==
                                   Runtime1::entry_for(Runtime1::jvmpi_method_exit_id);
  if (stub_is_jvmpi_method_exit) {
    CodeBlob* caller = CodeCache::find_blob(stub_fr.frameless_stub_return_addr());
    assert(caller != NULL && caller->is_nmethod(), "should only be called from an nmethod");
  }
#endif
#endif
  GC_locker::lock();
  if (jvmpi::is_event_enabled(JVMPI_EVENT_METHOD_EXIT)) {
    jvmpi::post_method_exit_event(method);
  }
  GC_locker::unlock();
JRT_END


#ifndef CORE

// Finds receiver, CallInfo (i.e. receiver method), and calling bytecode)
// for a call current in progress, i.e., arguments has been pushed on stack
// put callee has not been invoked yet.  Used by: resolve virtual/static,
// vtable updates, etc.  Caller frame must be compiled.
Handle SharedRuntime::find_callee_info(JavaThread* thread, Bytecodes::Code& bc, CallInfo& callinfo, TRAPS) {
  ResourceMark rm(THREAD);

  // last java frame on stack (which includes native call frames)
  vframeStream vfst(thread, true);  // Do not skip and javaCalls

  return find_callee_info_helper(thread, vfst, bc, callinfo, CHECK_(Handle()));
}


// Finds receiver, CallInfo (i.e. receiver method), and calling bytecode
// for a call current in progress, i.e., arguments has been pushed on stack
// but callee has not been invoked yet.  Caller frame must be compiled.
Handle SharedRuntime::find_callee_info_helper(JavaThread* thread,
                                              vframeStream& vfst,
                                              Bytecodes::Code& bc,
                                              CallInfo& callinfo, TRAPS) {
  Handle receiver;
  Handle nullHandle;  //create a handy null handle for exception returns

  assert(!vfst.at_end(), "Java frame must exist");
  
  // Find caller and bci from vframe
  methodHandle caller (THREAD, vfst.method());
  int          bci    = vfst.bci();
  
  // Find bytecode
  Bytecode_invoke* bytecode = Bytecode_invoke_at(caller, bci);
  bc = bytecode->adjusted_invoke_code();
  int bytecode_index = bytecode->index();

  // Find receiver for non-static call    
  if (bc != Bytecodes::_invokestatic) {
    // This register map must be update since we need to find the receiver for
    // compiled frames. The receiver might be in a register.
    RegisterMap reg_map2(thread); 
    frame stubFrame   = thread->last_frame();
    // Caller-frame is either a compiled frame or an I2C adapter
    frame callerFrame = stubFrame.sender(&reg_map2); 

    methodHandle callee = bytecode->static_target(CHECK_(nullHandle));
    if (callee.is_null()) {
      THROW_(vmSymbols::java_lang_NoSuchMethodException(), nullHandle);
    }  
#ifdef COMPILER2
    // Retrieve from either a compiled frame or an I2C adapter
    receiver = Handle(THREAD, callerFrame.retrieve_receiver(&reg_map2));
#else // COMPILER2
#ifdef SPARC
    receiver = Handle(THREAD, callerFrame.saved_receiver(&reg_map2));
#else // SPARC
    symbolHandle signature(THREAD, callee->signature());
    receiver = callerFrame.interpreter_callee_receiver(signature);
#endif // SPARC
#endif // COMPILER2

    if (receiver.is_null()) {
      THROW_(vmSymbols::java_lang_NullPointerException(), nullHandle);
    }
  }      

  // Resolve method. This is parameterized by bytecode.  
  constantPoolHandle constants (THREAD, caller->constants());
  assert (receiver.is_null() || receiver->is_oop(), "wrong receiver");
  LinkResolver::resolve_invoke(callinfo, receiver, constants, bytecode_index, bc, CHECK_(nullHandle));

#ifdef ASSERT
  // Check that the receiver klass is of the right subtype and that it is initialized for virtual calls  
  if (bc != Bytecodes::_invokestatic) {    
    assert(receiver.not_null(), "should have thrown exception");
    KlassHandle receiver_klass (THREAD, receiver->klass());
    klassOop rk = constants->klass_ref_at(bytecode_index, CHECK_(nullHandle));
                            // klass is already loaded
    KlassHandle static_receiver_klass (THREAD, rk);
    assert(receiver_klass->is_subtype_of(static_receiver_klass()), "actual receiver must be subclass of static receiver klass");
    if (receiver_klass->oop_is_instance()) {
      if (instanceKlass::cast(receiver_klass())->is_not_initialized()) {
        tty->print_cr("ERROR: Klass not yet initialized!!");
        receiver_klass.print();
      }
      assert (!instanceKlass::cast(receiver_klass())->is_not_initialized(), "receiver_klass must be initialized");
    }
  }
#endif

  return receiver;
}

methodHandle SharedRuntime::find_callee_method(JavaThread* thread, TRAPS) {
  ResourceMark rm(THREAD);
  // We need first to check if any Java activations (compiled, interpreted)
  // exist on the stack since last JavaCall.  If not, we need
  // to get the target method from the JavaCall wrapper.    
  vframeStream vfst(thread, true);  // Do not skip any javaCalls
  methodHandle callee_method;
  if (vfst.at_end()) {    
    // No Java frames were found on stack since we did the JavaCall.
    // Hence the stack can only contain an entry_frame and possibly an 
    // i2c_adapter.  We need to find the target method from the stub frame.
    RegisterMap reg_map(thread, false);
    frame fr = thread->last_frame();
    assert(fr.is_runtime_frame(), "must be a runtimeStub");
    fr = fr.sender(&reg_map);
    while(!fr.is_entry_frame())  {
      assert(fr.is_c2i_frame() || fr.is_i2c_frame(), "sanity check");
      fr = fr.sender(&reg_map);
    }
    // fr is now pointing to the entry frame.    
    callee_method = methodHandle(THREAD, fr.entry_frame_call_wrapper()->callee_method());        
    assert(fr.entry_frame_call_wrapper()->receiver() == NULL || !callee_method->is_static(), "non-null receiver for static call??");
  } else {
    Bytecodes::Code bc;
    CallInfo callinfo;
    find_callee_info_helper(thread, vfst, bc, callinfo, CHECK_(methodHandle()));
    callee_method = callinfo.selected_method();
  }
  // Update to either c2i or nmethod entry (i.e., replace lazy stub with real
  // stub)  
  callee_method->update_compiled_code_entry_point(false);
  return callee_method;
}

// Resolves a call.  
methodHandle SharedRuntime::resolve_helper(JavaThread *thread,
                                           bool is_virtual,
                                           bool is_optimized, TRAPS) {
  methodHandle callee_method;
  callee_method = resolve_sub_helper(thread, is_virtual, is_optimized, THREAD);
  if (JvmtiExport::can_hotswap_or_post_breakpoint()) {
    int retry_count = 0;
    while (!HAS_PENDING_EXCEPTION && callee_method()->is_old_version()) {
      // If has a pending exception then there is no need to re-try to 
      // resolve this method.

      // It is very unlikely that method is redefined more than 100 times
      // in the middle of resolve. If it is looping here more than 100 times 
      // means then there could be a bug here.
      guarantee((retry_count++ < 100), 
    	        "Could not resolve to latest version of redefined method");
      // method is redefined in the middle of resolve so re-try.
      callee_method = resolve_sub_helper(thread, is_virtual, is_optimized, THREAD);
    }
  }
  return callee_method;
}

// Resolves a call.  The compilers generate code for calls that go here
// and are patched with the real destination of the call.
methodHandle SharedRuntime::resolve_sub_helper(JavaThread *thread,
                                           bool is_virtual,
                                           bool is_optimized, TRAPS) {

  ResourceMark rm(thread);
  RegisterMap cbl_map(thread, false);
  frame caller_frame = thread->last_frame().sender(&cbl_map);

  CodeBlob* cb = CodeCache::find_blob(caller_frame.pc());
  guarantee(cb != NULL && cb->is_nmethod(), "must be called from nmethod");
  // make sure caller is not getting deoptimized
  // and removed before we are done with it.
  // CLEANUP - with lazy deopt shouldn't need this lock
  nmethodLocker caller_lock((nmethod*)cb);

  // determine call info & receiver
  // note: a) receiver is NULL for static calls
  //       b) an exception is thrown if receiver is NULL for non-static calls
  CallInfo call_info;
  Bytecodes::Code invoke_code = Bytecodes::_illegal;
  Handle receiver = find_callee_info(thread, invoke_code,
                                     call_info, CHECK_(methodHandle()));
  methodHandle callee_method = call_info.selected_method();

  assert((!is_virtual && invoke_code == Bytecodes::_invokestatic) ||
         ( is_virtual && invoke_code != Bytecodes::_invokestatic), "inconsistent bytecode");

#ifndef PRODUCT
  // tracing/debugging/statistics
  int *addr = (is_optimized) ? (&_resolve_opt_virtual_ctr) :
                (is_virtual) ? (&_resolve_virtual_ctr) : 
                               (&_resolve_static_ctr);
  Atomic::inc(addr);

  if (TraceCallFixup) {
    ResourceMark rm(thread);
    tty->print("resolving %s%s (%s) call to", 
      (is_optimized) ? "optimized " : "", (is_virtual) ? "virtual" : "static",
      Bytecodes::name(invoke_code));
    callee_method->print_short_name(tty);
    tty->print_cr(" code: " INTPTR_FORMAT, callee_method->code());
  }
#endif

  // Compute entry points. This might require generation of C2I converter
  // frames, so we cannot be holding any locks here. Furthermore, the
  // computation of the entry points is independent of patching the call.  We
  // always return the entry-point, but we only patch the stub if the call has
  // not been deoptimized.  Return values: For a virtual call this is an
  // (cached_oop, destination address) pair. For a static call/optimized
  // virtual this is just a destination address.

  StaticCallInfo static_call_info;
  CompiledICInfo virtual_call_info;


  // Make sure the callee nmethod does not get deoptimized and removed before
  // we are done patching the code.
  nmethod* nm = callee_method->code();
  nmethodLocker nl_callee(nm);
#ifdef ASSERT
  address dest_entry_point = nm == NULL ? 0 : nm->entry_point(); // used below
#endif

  if (is_virtual) {
    assert(receiver.not_null(), "sanity check");
    bool static_bound = 
                 Klass::can_be_statically_bound(call_info.resolved_method()());
    KlassHandle h_klass(THREAD, receiver->klass());
    CompiledIC::compute_monomorphic_entry(callee_method, h_klass,
                     is_optimized, static_bound, virtual_call_info,
                     CHECK_(methodHandle()));
  } else {
    // static call 
    CompiledStaticCall::compute_entry(callee_method, static_call_info);
  }

  // grab lock, check for deoptimization and potentially patch caller
  { 
    MutexLocker ml_patch(CompiledIC_lock);
    // Now that we are ready to patch we must be certain that the
    // nmethod hasn't been deoptimized while we figured out what
    // to do and waited for the CompiledIC_lock. If the nmethod
    // has been deoptimized we can not patch it because we could
    // overwrite a deopt patch.  If the methodOop was redefined 
    // then don't update call site and let the caller retry.

    if (!((nmethod*)cb)->is_patched_for_deopt()  && 
        !callee_method->is_old_version()) {
#ifdef ASSERT
      // We must not try to patch to jump to an already unloaded method.
      if (dest_entry_point != 0) {
	assert(CodeCache::find_blob(dest_entry_point) != NULL,
	       "should not unload nmethod while locked");
      }
#endif
      if (is_virtual) {
	CompiledIC* inline_cache = CompiledIC_before(caller_frame.pc());
	if (inline_cache->is_clean()) {
	  inline_cache->set_to_monomorphic(virtual_call_info);
	}
      } else {
	CompiledStaticCall* ssc = compiledStaticCall_before(caller_frame.pc());
	if (ssc->is_clean()) ssc->set(static_call_info);
      }
    }

  } // unlock CompiledIC_lock

  // Force this method to have a compiled code entry point.
  callee_method->update_compiled_code_entry_point(false);
  return callee_method;
}


methodHandle SharedRuntime::handle_ic_miss_helper(JavaThread *thread, TRAPS) {
  ResourceMark rm(thread);  
  CallInfo call_info;
  Bytecodes::Code bc;  

  // receiver is NULL for static calls. An exception is thrown for NULL
  // receivers for non-static calls
  Handle receiver = find_callee_info(thread, bc, call_info,
                                     CHECK_(methodHandle()));
  methodHandle callee_method = call_info.selected_method();

#ifndef PRODUCT
  Atomic::inc(&_ic_miss_ctr);

  // Statistics & Tracing
  if (TraceCallFixup) {
    ResourceMark rm(thread);
    tty->print("IC miss (%s) call to", Bytecodes::name(bc));
    callee_method->print_short_name(tty);
    tty->print_cr(" code: " INTPTR_FORMAT, callee_method->code());
  }

  if (ICMissHistogram) {
    MutexLocker m(VMStatistic_lock);
    RegisterMap reg_map(thread, false);
    frame f = thread->last_frame().real_sender(&reg_map);// skip runtime stub
    // produce statistics under the lock
    trace_ic_miss(f.pc());
  }
#endif

  // install an event collector so that when a vtable stub is created the 
  // profiler can be notified via a DYNAMIC_CODE_GENERATED event. The
  // event can't be posted when the stub is created as locks are held
  // - instead the event will be deferred until the event collector goes
  // out of scope.
  JvmtiDynamicCodeEventCollector event_collector;

  // Update inline cache to megamorphic. Skip update if caller has been 
  // deoptimized or we are called from interpreted.
  { MutexLocker ml_patch (CompiledIC_lock);
    RegisterMap reg_map(thread, false);
    frame caller_frame = thread->last_frame().sender(&reg_map);    
    CodeBlob* cb = CodeCache::find_blob(caller_frame.pc());
    if (cb->is_nmethod() && !((nmethod*)cb)->is_patched_for_deopt()) {
      // Not a deoptimized nmethod, so find inline_cache 
      CompiledIC* inline_cache = CompiledIC_before(caller_frame.pc());    
      if (!inline_cache->is_megamorphic() && !inline_cache->is_clean()) {
	// Change to megamorphic
	inline_cache->set_to_megamorphic(&call_info, bc, CHECK_(methodHandle()));
      } else {
	// Either clean or megamorphic
      }
    }  
  } // Release CompiledIC_lock

  // Force this method to have a compiled code entry point.
  callee_method->update_compiled_code_entry_point(false);
  return callee_method;
}

//
// Resets a call-site in compiled code so it will get resolved again.
// This routines handles both virtual call sites, optimized virtual call
// sites, and static call sites. Typically used to change a call sites
// destination from compiled to interpreted.
//
methodHandle SharedRuntime::reresolve_call_site(JavaThread *thread, TRAPS) {
  ResourceMark rm(thread);
  RegisterMap reg_map(thread, false);
  frame stub_frame = thread->last_frame();  
  assert(stub_frame.is_runtime_frame(), "must be a runtimeStub");
  frame caller = stub_frame.sender(&reg_map);

  // Do nothing if the frame isn't a live compiled frame.
  // nmethod could be deoptimized by the time we get here
  // so no update to the caller is needed.
  
  bool is_deopted;
  if (caller.is_compiled_frame(&is_deopted) && !is_deopted) {

    address pc = caller.pc();
    Events::log("update call-site at pc " INTPTR_FORMAT, pc);
    
    // Default call_addr is the location of the "basic" call.
    // Determine the address of the call we a reresolving. With
    // Inline Caches we will always find a recognizable call.
    // With Inline Caches disabled we may or may not find a
    // recognizable call. We will always find a call for static
    // calls and for optimized virtual calls. For vanilla virtual
    // calls it depends on the state of the UseInlineCaches switch.
    //
    // With Inline Caches disabled we can get here for a virtual call
    // for two reasons: 
    //   1 - calling an abstract method. The vtable for abstract methods
    //       will run us thru handle_wrong_method and we will eventually
    //       end up in the interpreter to throw the ame.
    //   2 - a racing deoptimization. We could be doing a vanilla vtable
    //       call and between the time we fetch the entry address and
    //       we jump to it the target gets deoptimized. Similar to 1
    //       we will wind up in the interprter (thru a c2i with c2).
    //
    address call_addr = NULL;
    {
      // Get call instruction under lock because another thread may be
      // busy patching it.
      MutexLockerEx ml_patch(Patching_lock, Mutex::_no_safepoint_check_flag);
      // Location of call instruction
      if (NativeCall::is_call_before(pc)) {
	NativeCall *ncall = nativeCall_before(pc);
	call_addr = ncall->instruction_address();
      }
    }

    // Check for static or virtual call
    bool is_static_call = false;
    nmethod* caller_nm = CodeCache::find_nmethod(pc);
    // Make sure nmethod doesn't get deoptimized and removed until
    // this is done with it.
    // CLEANUP - with lazy deopt shouldn't need this lock
    nmethodLocker nmlock(caller_nm);

    if (call_addr != NULL) {
      RelocIterator iter(caller_nm, call_addr, call_addr+1);
      int ret = iter.next(); // Get item
      if (ret) {
	assert(iter.addr() == call_addr, "must find call");
	if (iter.type() == relocInfo::static_call_type) {
	  is_static_call = true;
	} else {
	  assert(iter.type() == relocInfo::virtual_call_type ||
		 iter.type() == relocInfo::opt_virtual_call_type
		, "unexpected relocInfo. type");
	}
      } else {
	assert(!UseInlineCaches, "relocation info. must exist for this address");
      }

      // Cleaning the inline cache will force a new resolve. This is more robust
      // than directly setting it to the new destination, since resolving of calls
      // is always done through the same code path. (experience shows that it
      // leads to very hard to track down bugs, if an inline cache gets updated
      // to a wrong method). It should not be performance critical, since the
      // resolve is only done once.
      
      MutexLocker ml(CompiledIC_lock);    
      //
      // We can not patch the call site if the nmethod has been deoptimized
      // because we could overwrite a deopt patch
      //
      if (!caller_nm->is_patched_for_deopt()) {
	if (is_static_call) {         
	  CompiledStaticCall* ssc= compiledStaticCall_at(call_addr);
	  ssc->set_to_clean();
	} else {
	  // compiled, dispatched call (which used to call an interpreted method)
	  CompiledIC* inline_cache = CompiledIC_at(call_addr);
	  inline_cache->set_to_clean();
	}
      }
    }
  
  }

  methodHandle callee_method = find_callee_method(thread, CHECK_(methodHandle()));

#ifndef PRODUCT
  Atomic::inc(&_wrong_method_ctr);

  if (TraceCallFixup) { 
    ResourceMark rm(thread);
    tty->print("handle_wrong_method reresolving call to");
    callee_method->print_short_name(tty);
    tty->print_cr(" code: " INTPTR_FORMAT, callee_method->code());
  }
#endif

  return callee_method;
}
#endif // !CORE


JRT_LEAF(void, SharedRuntime::reguard_yellow_pages())
  (void) JavaThread::current()->reguard_stack();
JRT_END


