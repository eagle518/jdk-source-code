/*
 * Copyright (c) 1999, 2007, Oracle and/or its affiliates. All rights reserved.
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

// The following routines and definitions are used internally in the
// compiler interface.


// Add a ci native entry wrapper?

// Bring the compilation thread into the VM state.
#define VM_ENTRY_MARK                       \
  CompilerThread* thread=CompilerThread::current(); \
  ThreadInVMfromNative __tiv(thread);       \
  ResetNoHandleMark rnhm;                   \
  HandleMarkCleaner __hm(thread);           \
  Thread* THREAD = thread;                  \
  debug_only(VMNativeEntryWrapper __vew;)



// Bring the compilation thread into the VM state.  No handle mark.
#define VM_QUICK_ENTRY_MARK                 \
  CompilerThread* thread=CompilerThread::current(); \
  ThreadInVMfromNative __tiv(thread);       \
/*                                          \
 * [TODO] The NoHandleMark line does nothing but declare a function prototype \
 * The NoHandkeMark constructor is NOT executed. If the ()'s are   \
 * removed, causes the NoHandleMark assert to trigger. \
 * debug_only(NoHandleMark __hm();)         \
 */                                         \
  Thread* THREAD = thread;                  \
  debug_only(VMNativeEntryWrapper __vew;)


#define EXCEPTION_CONTEXT \
  CompilerThread* thread=CompilerThread::current(); \
  Thread* THREAD = thread;


#define CURRENT_ENV                         \
  ciEnv::current()

// where current thread is THREAD
#define CURRENT_THREAD_ENV                  \
  ciEnv::current(thread)

#define IS_IN_VM                            \
  ciEnv::is_in_vm()

#define ASSERT_IN_VM                        \
  assert(IS_IN_VM, "must be in vm state");

#define GUARDED_VM_ENTRY(action)            \
  {if (IS_IN_VM) { action } else { VM_ENTRY_MARK; { action }}}

// Redefine this later.
#define KILL_COMPILE_ON_FATAL_(result)           \
  THREAD);                                       \
  if (HAS_PENDING_EXCEPTION) {                   \
    if (PENDING_EXCEPTION->klass() ==            \
        SystemDictionary::ThreadDeath_klass()) { \
      /* Kill the compilation. */                \
      fatal("unhandled ci exception");           \
      return (result);                           \
    }                                            \
    CLEAR_PENDING_EXCEPTION;                     \
    return (result);                             \
  }                                              \
  (0

#define KILL_COMPILE_ON_ANY                      \
  THREAD);                                       \
  if (HAS_PENDING_EXCEPTION) {                   \
    fatal("unhandled ci exception");             \
    CLEAR_PENDING_EXCEPTION;                     \
  }                                              \
(0


inline const char* bool_to_str(bool b) {
  return ((b) ? "true" : "false");
}

const char* basictype_to_str(BasicType t);
const char  basictype_to_char(BasicType t);
