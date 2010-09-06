#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sharedRuntime.hpp	1.127 04/05/04 15:01:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Runtime is the base class for various runtime interfaces
// (InterpreterRuntime, CompilerRuntime, etc.). It provides
// shared functionality such as exception forwarding (C++ to
// Java exceptions), locking/unlocking mechanisms, statistical
// information, etc.

class SharedRuntime: AllStatic {
 private:
  // The following function pointers provide access to the
  // corresponding StrictMath functions - used for intrinsics
  // inlined by the compiler and special-cased by the interpreter
  // (must match compiled implementation).
  typedef jdouble (JNICALL *StrictMathFunction_DD_D)(JNIEnv* env_unused, jclass class_unused, jdouble x, jdouble y);
  static void lookup_function_DD_D(StrictMathFunction_DD_D& f, const char* fname);
  static StrictMathFunction_DD_D java_lang_strict_math_pow;

  static methodHandle resolve_sub_helper(JavaThread *thread,
                                     bool is_virtual,
                                     bool is_optimized, TRAPS);

 public:
  // The following arithmetic routines are used on platforms that do
  // not have machine instructions to implement their functionality.
  // Do not remove these.

  // long arithmetics
  static jlong   lmul(jlong y, jlong x);
  static jlong   ldiv(jlong y, jlong x);
  static jlong   lrem(jlong y, jlong x);

  // float and double remainder
  static jfloat  frem(jfloat  x, jfloat  y);
  static jdouble drem(jdouble x, jdouble y);

  // float conversion (needs to set appropriate rounding mode)
  static jint    f2i (jfloat  x);
  static jlong   f2l (jfloat  x);
  static jint    d2i (jdouble x);
  static jlong   d2l (jdouble x);
  static jfloat  d2f (jdouble x);
  static jfloat  l2f (jlong   x);
  static jdouble l2d (jlong   x);

  // double trigonometrics
  static jdouble dsin(jdouble x);
  static jdouble dcos(jdouble x);
  static jdouble dpow(jdouble x, jdouble y);
  
  // exception handling accross interpreter/compiler boundaries
  static address raw_exception_handler_for_return_address(address return_address);
  static address exception_handler_for_return_address(address return_address);

  // exception handling and implicit exceptions
  enum ImplicitExceptionKind {
    IMPLICIT_NULL,
    IMPLICIT_DIVIDE_BY_ZERO,
    STACK_OVERFLOW
  };
  static void    throw_AbstractMethodError(JavaThread* thread);
  static void    throw_ArithmeticException(JavaThread* thread);
  static void    throw_NullPointerException(JavaThread* thread);
  static void    throw_NullPointerException_at_call(JavaThread* thread);
  static void    throw_StackOverflowError(JavaThread* thread);
  static address continuation_for_implicit_exception(JavaThread* thread,
                                                     address faulting_pc,
                                                     ImplicitExceptionKind exception_kind);

  // Helper routine for full-speed JVMTI exception throwing support
  static void throw_and_post_jvmti_exception(JavaThread *thread, Handle h_exception);
  static void throw_and_post_jvmti_exception(JavaThread *thread, symbolOop name, const char *message = NULL);

  // To be used as the entry point for unresolved native methods.
  static address native_method_throw_unsatisfied_link_error_entry();

  // bytecode tracing is only used by the TraceBytecodes and
  // EnableJVMPIInstructionStartEvent options. When JVM/PI is retired,
  // this declaration can be made PRODUCT_RETURN0.
  static intptr_t trace_bytecode(JavaThread* thread, intptr_t preserve_this_value, intptr_t tos, intptr_t tos2);
  
  // Used to back off a spin lock that is under heavy contention
  static void yield_all(JavaThread* thread, int attempts = 0);

  static oop retrieve_receiver( symbolHandle sig, frame caller );  

  static void verify_caller_frame(frame caller_frame, methodHandle callee_method) PRODUCT_RETURN;
  static methodHandle find_callee_method_inside_interpreter(frame caller_frame, methodHandle caller_method, int bci) PRODUCT_RETURN_(return methodHandle(););

  // jvmpi
  static void jvmpi_method_entry_work(JavaThread* thread, methodOop method, oop receiver);
  static void jvmpi_method_entry(JavaThread* thread, methodOop method, oop receiver);
  static void jvmpi_method_exit(JavaThread* thread, methodOop method);

  // intialization
  static void initialize_StrictMath_entry_points();

  // used by native wrappers to reenable yellow if overflow happened in native code
  static void reguard_yellow_pages();

#ifndef CORE

  // Resolves a call site- may patch in the destination of the call into the
  // compiled code.
  static methodHandle resolve_helper(JavaThread *thread,
                                     bool is_virtual,
                                     bool is_optimized, TRAPS);

  // Create an OSR adapter of the correct size
  static OSRAdapter* generate_osr_blob(int frame_size);

  // deopt blob
  static void generate_deopt_blob(void);
  static DeoptimizationBlob* _deopt_blob;
  static DeoptimizationBlob* deopt_blob(void)      { return _deopt_blob; }

  // Resets a call-site in compiled code so it will get resolved again.
  static methodHandle reresolve_call_site(JavaThread *thread, TRAPS);

  // In the code prolog, if the klass comparison fails, the inline cache
  // misses and the call site is patched to megamorphic
  static methodHandle handle_ic_miss_helper(JavaThread* thread, TRAPS);

  // Find the method that called us.
  static methodHandle find_callee_method(JavaThread* thread, TRAPS);

 private:
  static Handle find_callee_info(JavaThread* thread,
                                 Bytecodes::Code& bc, 
                                 CallInfo& callinfo, TRAPS);
  static Handle find_callee_info_helper(JavaThread* thread,
                                        vframeStream& vfst,
                                        Bytecodes::Code& bc,
                                        CallInfo& callinfo, TRAPS);

  static address clean_virtual_call_entry();
  static address clean_opt_virtual_call_entry();
  static address clean_static_call_entry();
#ifndef PRODUCT

  // Collect and print inline cache miss statistics
 private:
  enum { maxICmiss_count = 100 };
  static int	 _ICmiss_index;                  // length of IC miss histogram
  static int	 _ICmiss_count[maxICmiss_count]; // miss counts
  static address _ICmiss_at[maxICmiss_count];    // miss addresses
  static void trace_ic_miss(address at);

 public:
  static int _ic_miss_ctr;                   // total # of IC misses
  static int _wrong_method_ctr;
  static int _resolve_static_ctr;
  static int _resolve_virtual_ctr;
  static int _resolve_opt_virtual_ctr;
  static int _implicit_null_throws;
  static int _implicit_div0_throws;

  static void print_ic_miss_histogram();

#endif // PRODUCT
#endif // CORE
};
