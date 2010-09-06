#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreterRuntime.hpp	1.130 04/03/22 19:27:52 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The InterpreterRuntime is called by the interpreter for everything
// that cannot/should not be dealt with in assembly and needs C support.

class InterpreterRuntime: AllStatic {
  friend class BytecodeClosure; // for method and bcp
  friend class PrintingClosure; // for method and bcp

 private:
  // Helper functions to access current interpreter state
  static frame     last_frame(JavaThread *thread)    { return thread->last_frame(); }  
  static methodOop method(JavaThread *thread)        { return last_frame(thread).interpreter_frame_method(); }
  static address   bcp(JavaThread *thread)           { return last_frame(thread).interpreter_frame_bcp(); }
  static void      set_bcp_and_mdp(address bcp, JavaThread*thread);
  static Bytecodes::Code code(JavaThread *thread)       { return Bytecodes::code_at(bcp(thread)); }
  static bool      already_resolved(JavaThread *thread) { return cache_entry(thread)->is_resolved(code(thread)); }
  static int       one_byte_index(JavaThread *thread)   { return bcp(thread)[1]; }
  static int       two_byte_index(JavaThread *thread)   { return Bytes::get_Java_u2(bcp(thread) + 1); }
  static int       number_of_dimensions(JavaThread *thread)  { return bcp(thread)[3]; }
  static ConstantPoolCacheEntry* cache_entry(JavaThread *thread)  { return method(thread)->constants()->cache()->entry_at(Bytes::get_native_u2(bcp(thread) + 1)); }
  static void      note_trap(JavaThread *thread, int reason, TRAPS);

 public:
  // Constants
  static void    ldc           (JavaThread* thread, bool wide);

  // Allocation
  static void    _new          (JavaThread* thread, constantPoolOop pool, int index);
  static void    newarray      (JavaThread* thread, BasicType type, jint size);
  static void    anewarray     (JavaThread* thread, constantPoolOop pool, int index, jint size);
  static void    multianewarray(JavaThread* thread, jint* first_size_address);

  // Quicken instance-of and check-cast bytecodes
  static void    quicken_io_cc(JavaThread* thread);

  // Exceptions thrown by the interpreter
  static void    throw_AbstractMethodError(JavaThread* thread);
  static void    throw_IncompatibleClassChangeError(JavaThread* thread);
  static void    throw_StackOverflowError(JavaThread* thread);
  static void    throw_ArrayIndexOutOfBoundsException(JavaThread* thread, char* name, jint index);
  static void    create_exception(JavaThread* thread, char* name, char* message);
  static void    create_klass_exception(JavaThread* thread, char* name, oop obj);
  static address exception_handler_for_exception(JavaThread* thread, oop exception);
  static void    throw_pending_exception(JavaThread* thread);

  // Statics & fields
  static void    resolve_get_put(JavaThread* thread, Bytecodes::Code bytecode);  
  
  // Synchronization
  static void    monitorenter(JavaThread* thread, BasicObjectLock* elem);
  static void    monitorexit (JavaThread* thread, BasicObjectLock* elem);
  
  static void    throw_illegal_monitor_state_exception(JavaThread* thread);
  static void    new_illegal_monitor_state_exception(JavaThread* thread);

  // Calls
  static void    resolve_invoke     (JavaThread* thread, Bytecodes::Code bytecode);

  // Breakpoints
  static void _breakpoint(JavaThread* thread, methodOop method, address bcp);
  static Bytecodes::Code get_original_bytecode_at(JavaThread* thread, methodOop method, address bcp);
  static void            set_original_bytecode_at(JavaThread* thread, methodOop method, address bcp, Bytecodes::Code new_code);
  static bool is_breakpoint(JavaThread *thread) { return Bytecodes::code_or_bp_at(bcp(thread)) == Bytecodes::_breakpoint; }

  // Safepoints
  static void    at_safepoint(JavaThread* thread);

  // Debugger support
  static void post_field_access(JavaThread *thread, oop obj,
    ConstantPoolCacheEntry *cp_entry);
  static void post_field_modification(JavaThread *thread, oop obj,
    ConstantPoolCacheEntry *cp_entry, jvalue *value);
  static void post_method_entry(JavaThread *thread);
  static void post_method_exit (JavaThread *thread);
  static int  interpreter_contains(address pc);

  // Native signature handlers
  static void prepare_native_call(JavaThread* thread, methodOop method);

  // Platform dependent stuff
  #include "incls/_interpreterRT_pd.hpp.incl"

#ifndef CORE
  static address nmethod_entry_point(JavaThread* thread, methodOop m, nmethod* nm);

  // Interpreter's frequency counter overflow
  static IcoResult frequency_counter_overflow(JavaThread* thread, address branch_bcp);
  
  // Interpreter profiling support
  static jint    bcp_to_di(methodOop method, address cur_bcp);
  static jint    profile_method(JavaThread* thread, address cur_bcp);
  static void    update_mdp_for_ret(JavaThread* thread, int bci);
#ifdef ASSERT
  static void    verify_mdp(methodOop method, address bcp, address mdp);
#endif // ASSERT
#endif // !CORE
};


class SignatureHandlerLibrary: public AllStatic {
 public:
  enum { buffer_size =  1*K }; // the size of the temporary code buffer
  enum { blob_size   = 32*K }; // the size of a handler code blob.

 private:
  static BufferBlob*              _handler_blob; // the current buffer blob containing the generated handlers
  static address                  _handler;      // next available address within _handler_blob;
  static GrowableArray<uint64_t>* _fingerprints; // the fingerprint collection
  static GrowableArray<address>*  _handlers;     // the corresponding handlers
  static u_char                   _buffer[SignatureHandlerLibrary::buffer_size]; 
                                                 // the temporary code buffer

  static address set_handler_blob();
  static void initialize();
  static address set_handler(CodeBuffer* buffer);
  static void pd_set_handler(address handler);

 public:
  static void add(methodHandle method);
};
