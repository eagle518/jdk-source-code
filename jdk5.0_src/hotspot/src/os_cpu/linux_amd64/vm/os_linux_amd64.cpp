#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_linux_amd64.cpp	1.11 04/04/20 16:27:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// do not include  precompiled  header file
#include "incls/_os_linux_amd64.cpp.incl"
#include "incls/_os_pd.hpp.incl"

// put OS-includes here
#include <sys/types.h>
#include <sys/mman.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pwd.h>
#include <poll.h>
#include <ucontext.h>

address os::current_stack_pointer()
{
  register void *rsp __asm__ ("rsp");
  return (address) rsp;
}

char* os::non_memory_address_word()
{
  // Must never look like an address returned by reserve_memory, even
  // in its subfields (as defined by the CPU immediate fields, if the
  // CPU splits constants across multiple instructions).

  return (char*) -1;
}

void os::initialize_thread()
{
// Nothing to do.
}

address os::Linux::ucontext_get_pc(ucontext_t* uc)
{
  return (address) uc->uc_mcontext.gregs[REG_RIP];
}

intptr_t* os::Linux::ucontext_get_sp(ucontext_t* uc)
{
  return (intptr_t*) uc->uc_mcontext.gregs[REG_RSP];
}

intptr_t* os::Linux::ucontext_get_fp(ucontext_t* uc)
{
  return (intptr_t*) uc->uc_mcontext.gregs[REG_RBP];
}

// For Forte Analyzer AsyncGetCallTrace profiling support - thread is
// currently interrupted by SIGPROF.
// os::Solaris::fetch_frame_from_ucontext() tries to skip nested
// signal frames. Currently we don't do that on Linux, so it's the
// same as os::fetch_frame_from_context().
ExtendedPC os::Linux::fetch_frame_from_ucontext(Thread* thread,
                                                ucontext_t* uc,
                                                intptr_t** ret_sp,
                                                intptr_t** ret_fp)
{
  assert(thread != NULL, "just checking");
  assert(ret_sp != NULL, "just checking");
  assert(ret_fp != NULL, "just checking");

  return os::fetch_frame_from_context(uc, ret_sp, ret_fp);
}

ExtendedPC os::fetch_frame_from_context(void* ucVoid,
                                        intptr_t** ret_sp,
                                        intptr_t** ret_fp)
{
  ucontext_t* uc = (ucontext_t*) ucVoid;
  ExtendedPC  epc;

  if (uc != NULL) {
    epc = ExtendedPC(os::Linux::ucontext_get_pc(uc));
    if (ret_sp) {
      *ret_sp = os::Linux::ucontext_get_sp(uc);
    }
    if (ret_fp) {
      *ret_fp = os::Linux::ucontext_get_fp(uc);
    }
  } else {
    // construct empty ExtendedPC for return value checking
    epc = ExtendedPC(NULL);
    if (ret_sp) {
      *ret_sp = (intptr_t*) NULL;
    }
    if (ret_fp) {
      *ret_fp = (intptr_t*) NULL;
    }
  }

  return epc;
}

frame os::fetch_frame_from_context(void* ucVoid)
{
  intptr_t* sp;
  intptr_t* fp;
  ExtendedPC epc = fetch_frame_from_context(ucVoid, &sp, &fp);
  return frame(sp, fp, epc.pc());
}

// XXX
// By default, gcc always saves frame pointer (%rbp) on stack. It may get
// turned off by -fomit-frame-pointer,
frame os::get_sender_for_C_frame(frame* fr) {
  return frame(fr->sender_sp(), fr->link(), fr->sender_pc());
}

frame os::current_frame()
{
  intptr_t* fp = 
    (*CAST_TO_FN_PTR(intptr_t* (*)(void),
                     StubRoutines::amd64::get_previous_fp_entry()))();
  frame myframe((intptr_t*) os::current_stack_pointer(),
                (intptr_t*) fp,
                CAST_FROM_FN_PTR(address, os::current_frame));
  if (os::is_first_C_frame(&myframe)) {
    // stack is not walkable
    return frame(NULL, NULL, NULL);
  } else {
    return os::get_sender_for_C_frame(&myframe);
  }
}

#ifndef CORE
// return true on success, false otherwise
bool os::set_thread_pc_and_resume(JavaThread* thread,
                                  ExtendedPC old_addr,
                                  ExtendedPC new_addr)
{
  assert(thread->is_in_compiled_safepoint(), "only for compiled safepoint");

  OSThread* osthread = thread->osthread();
  assert(thread->is_vm_suspended(), "must be suspended");

  // Note: self-suspended thread or thread suspended when waiting for
  // a mutex can not be repositioned, because it is not suspended
  // inside SR_handler. set_thread_pc() is only called when
  // fetch_top_frame() returns address in nmethod. That cannot be
  // self-suspension nor the mutex case.
  assert(!(thread->is_Java_thread() &&
           ((JavaThread*) thread)->is_ext_suspended()) &&
         !thread->osthread()->sr.is_try_mutex_enter(),
         "cannot be self-suspended or suspended while waiting for mutex");

  ucontext_t* context = osthread->ucontext();
  bool res = false;
  if (context->uc_mcontext.gregs[REG_RIP] == (intptr_t) old_addr.pc()) {
    context->uc_mcontext.gregs[REG_RIP] = (intptr_t) new_addr.pc();
    res = true;
  }

  thread->safepoint_state()->notify_set_thread_pc_result(res);
  thread->vm_resume();
  return res;
}
#endif

// Utility functions

julong os::allocatable_physical_memory(julong size)
{
  return size;
}


extern "C" int 
JVM_handle_linux_signal(int sig,
                        siginfo_t* info,
                        void* ucVoid,
                        int abort_if_unrecognized)
{
  ucontext_t* uc = (ucontext_t*) ucVoid;

  Thread* t = ThreadLocalStorage::get_thread_slow();

  SignalHandlerMark shm(t);

  JavaThread* thread = NULL;
  VMThread* vmthread = NULL;
  if (os::Linux::signal_handlers_are_installed) {
    if (t != NULL ){
      if(t->is_Java_thread()) {
        thread = (JavaThread*)t;
      }
      else if(t->is_VM_thread()){
        vmthread = (VMThread *)t;
      }
    }
  }

  // decide if this trap can be handled by a stub
  address stub = NULL;

  address pc = NULL;
  address adjusted_pc = NULL;

  //%note os_trap_1
  if (info != NULL && thread != NULL) {
    // factor me: getPCfromContext
    pc = (address) uc->uc_mcontext.gregs[REG_RIP];
#ifndef CORE
    adjusted_pc = 
      SafepointPolling 
      ? pc
      : thread->safepoint_state()->compute_adjusted_pc(pc);
#else
    adjusted_pc = pc;
#endif

    // Handle ALL stack overflow variations here
    if (sig == SIGSEGV) {
      address addr = (address) info->si_addr;

      // check if fault address is within thread stack
      if (addr < thread->stack_base() &&
          addr >= thread->stack_base() - thread->stack_size()) {
        // stack overflow
        if (thread->in_stack_yellow_zone(addr)) {
          thread->disable_stack_yellow_zone();
          if (thread->thread_state() == _thread_in_Java) {
            // Throw a stack overflow exception.  Guard pages will be reenabled
            // while unwinding the stack.
            stub =
              SharedRuntime::
              continuation_for_implicit_exception(thread,
                                                  pc,
                                                  SharedRuntime::STACK_OVERFLOW);
          } else {
            // Thread was in the vm or native code.  Return and try to finish.
            return 1;
          }
        } else if (thread->in_stack_red_zone(addr)) {
          // Fatal red zone violation.  Disable the guard pages and fall through
          // to handle_unexpected_exception way down below.
          thread->disable_stack_red_zone();
          tty->print_raw_cr("An irrecoverable stack overflow has occurred.");
        } else {
          // Accessing stack address below sp may cause SEGV if current
          // thread has MAP_GROWSDOWN stack. This should only happen when
          // current thread was created by user code with MAP_GROWSDOWN flag
          // and then attached to VM. See notes in os_linux.cpp.
          if (thread->osthread()->expanding_stack() == 0) {
             thread->osthread()->set_expanding_stack();
             if (os::Linux::manually_expand_stack(thread, addr)) {
               thread->osthread()->clear_expanding_stack();
               return 1;
             }
             thread->osthread()->clear_expanding_stack();
          } else {
             fatal("recursive segv. expanding stack.");
          }
        } 
      }
    }

    if (thread->thread_state() == _thread_in_Java) {
      // Java thread running in Java code => find exception handler if any
      // a fault inside compiled code, the interpreter, or a stub

#ifndef CORE
      if (sig == SIGILL && nativeInstruction_at(pc)->is_illegal()) {
#ifdef COMPILER1
        stub = Runtime1::entry_for(Runtime1::illegal_instruction_handler_id);
#else
        // debugging trap, or a safepoint
        assert(OptoRuntime::illegal_exception_handler_blob() != NULL,
               "stub not created yet");
        stub = 
          OptoRuntime::illegal_exception_handler_blob()->instructions_begin();
#endif	// COMPILER1
        CompiledCodeSafepointHandler* handler = 
          (CompiledCodeSafepointHandler *) thread->safepoint_state()->handle();
        if (handler != NULL && thread->is_in_compiled_safepoint()) {
          nmethod* nm = handler->get_nmethod();
          assert(nm != NULL, "safepoint handler is not setup correctly");

          relocInfo::relocType type = nm->reloc_type_for_address(adjusted_pc);
          assert(((NativeInstruction*) adjusted_pc)->is_call() || 
                 type == relocInfo::return_type ||
                 type == relocInfo::safepoint_type, 
                 "Only calls, returns, and backward branches are patched "
                 "at safepoint");

          if(type == relocInfo::return_type) {
            // The retl case: restore has already happened. The
            // safepoint blob frame will appear immediately below
            // caller of the orignal method that we have patched. The
            // stack could be walked properly (the frame of the
            // patched method is already popped). Will need to be
            // revisited if and when we put restore in the delay slot
            // of ret.
          } else {
            // This is the case when either a call or a branch was
            // patched with an illegal instruction.  At this point we
            // are done with the patched method and would like things
            // to appear as if the orignal method simply called the
            // safepoint blob at a "safe point".  To achieve this, we
            // set up the correct linkage by placing the adjusted trap
            // pc in O7 and then "returning" from this trap handler to
            // the safepoint blob.

            // QQQ TODO
            // uc->uc_mcontext.gregs[REG_O7] = (greg_t) adjusted_pc;
          }
        }
      } else if (sig == SIGSEGV && SafepointPolling && 
                 (address) info->si_addr == os::get_polling_page()) {
        // Look up the code blob
        CodeBlob* cb = CodeCache::find_blob(pc);

        // Should be an nmethod
        assert(cb && cb->is_nmethod(),
               "safepoint polling: pc must refer to an nmethod");

        // Look up the relocation information
        assert( ((nmethod*) cb)->is_at_poll_or_poll_return(pc),
                "safepoint polling: type must be poll" );

        assert(((NativeInstruction*) pc)->is_safepoint_poll(),
               "Only polling locations are used for safepoint");

        // return
        if (((nmethod*)cb)->is_at_poll_return(pc)) {
#ifdef COMPILER1
          stub = Runtime1::entry_for(Runtime1::polling_page_return_handler_id);
#else
          assert(OptoRuntime::polling_page_return_handler_blob() != NULL,
                 "polling page return stub not created yet");
          stub = OptoRuntime::polling_page_return_handler_blob()->instructions_begin();
#endif

#ifndef PRODUCT
          if (TraceSafepoint)
            tty->print("... found polling page return exception at pc = "
                       INTPTR_FORMAT ", stub = " INTPTR_FORMAT "\n",
                       (intptr_t) pc, (intptr_t) stub);
#endif
        }
        else {
#ifdef COMPILER1
          stub = Runtime1::entry_for(Runtime1::polling_page_safepoint_handler_id);
#else
          assert(OptoRuntime::polling_page_safepoint_handler_blob() != NULL,
                 "polling page safepoint stub not created yet");
          stub = OptoRuntime::polling_page_safepoint_handler_blob()->instructions_begin();
#endif

#ifndef PRODUCT
          if (TraceSafepoint)
            tty->print("... found polling page safepoint exception at pc = "
                       INTPTR_FORMAT ", stub = " INTPTR_FORMAT "\n",
                       (intptr_t) pc, (intptr_t) stub);
#endif
        }
      } else 
#endif // NOT CORE 

      if (sig == SIGFPE  && 
          (info->si_code == FPE_INTDIV || info->si_code == FPE_FLTDIV)) {
        stub = 
          SharedRuntime::
          continuation_for_implicit_exception(thread,
                                              pc, 
                                              SharedRuntime::
                                              IMPLICIT_DIVIDE_BY_ZERO);
      }

#ifdef COMPILER2
      else if (sig == SIGSEGV && 
               OptoRuntime::illegal_exception_handler_blob() != NULL &&
               OptoRuntime::illegal_exception_handler_blob()->contains(pc)) {
        // This is the case when we use a trap to restore the context
        // at the end of safepoint taken in compiled code. The
        // "return" address is passed in O7 QQQ TODO
        ShouldNotReachHere();

#if 0
        greg_t ret_pc = uc->uc_mcontext.gregs[REG_RIP];
        // Recover global regs saved at the original illegal
        // instruction trap. (%o are handled "automatically" and are
        // possibly modified by the gc - don't touch them)
        thread->restore_global_regs_from_saved_context(uc);
        uc->uc_mcontext.gregs[REG_RIP] =  ret_pc;
        return true;
#endif	// 0
      }
#endif	// COMPILER2

      else if (sig == SIGSEGV &&
               !MacroAssembler::needs_explicit_null_check((intptr_t)
                                                          info->si_addr)) {
        // Determination of interpreter/vtable stub/compiled code null
        // exception
        stub = 
          SharedRuntime::
          continuation_for_implicit_exception(thread, pc, 
                                              SharedRuntime::
                                              IMPLICIT_NULL);
      }
    }
  }

  if (stub != NULL) {
    // save all thread context in case we need to restore it

    thread->set_saved_exception_pc(pc);
    // TODO: 
    // Fastlane -- is this all? Do we need to save off any context?
    // Currently, no, but might need to if we'll be returning to the
    // context where the trap occurred. Will need to revisit this for
    // compiled safepoint stubs.  factor me: setPC
    uc->uc_mcontext.gregs[REG_RIP] = (greg_t) stub;
    return true;
  }

  // jni_fast_Get<Primitive>Field can trap at certain pc's if a GC kicks in
  // and the heap gets shrunk before the field access.
  if ((sig == SIGSEGV) || (sig == SIGBUS)) {
    address addr = JNI_FastGetField::find_slowcase_pc(pc);
    if (addr != (address)-1) {
      uc->uc_mcontext.gregs[REG_RIP] = (greg_t)addr;
      return true;
    }
  }

  // signal-chaining
  if (UseSignalChaining) {
    bool chained = false;
    struct sigaction* actp = os::Linux::get_chained_signal_action(sig);
    if (actp != NULL) {
      chained = os::Linux::chained_handler(actp, sig, info, ucVoid);
    }
    if (chained) {
      // signal-chaining in effect. Continue.
      return true;
    }
  }

  if (sig == SIGPIPE) {
    if (PrintMiscellaneous && (WizardMode || Verbose)) {
      warning("Ignoring SIGPIPE - see bug 4229104");
    }
    return 1;
  }
  
  if (!abort_if_unrecognized) {
    // caller wants another chance, so give it to him
    return false;
  }

  if (pc == NULL && uc != NULL) {
    pc = (address) uc->uc_mcontext.gregs[REG_RIP];
  }

  // unmask current signal
  sigset_t newset;
  sigemptyset(&newset);
  sigaddset(&newset, sig);
  sigprocmask(SIG_UNBLOCK, &newset, NULL);

  VMError err(t, sig, pc, info, ucVoid);
  err.report_and_die();

  ShouldNotReachHere();
}

void os::Linux::init_thread_fpu_state(void)
{
  // Nothing to do
}

///////////////////////////////////////////////////////////////////////////////
// thread stack

size_t os::Linux::min_stack_allowed  = 64 * K;

// amd64: pthread on amd64 is always in floating stack mode
bool os::Linux::supports_variable_stack_size() {  return true; }

// return default stack size for thr_type
size_t os::Linux::default_stack_size(os::ThreadType thr_type) {
  // default stack size (compiler thread needs larger stack)
  size_t s = (thr_type == os::compiler_thread ? 4 * M : 1 * M);
  return s;
} 
  
size_t os::Linux::default_guard_size(os::ThreadType thr_type) {
  // Creating guard page is very expensive. Java thread has HotSpot
  // guard page, only enable glibc guard page for non-Java threads.
  return (thr_type == java_thread ? 0 : page_size());
}

// Java thread:
//
//   Low memory addresses
//    +------------------------+
//    |                        |\  JavaThread created by VM does not have glibc
//    |    glibc guard page    | - guard, attached Java thread usually has
//    |                        |/  1 page glibc guard.
// P1 +------------------------+ Thread::stack_base() - Thread::stack_size()
//    |                        |\
//    |  HotSpot Guard Pages   | - red and yellow pages
//    |                        |/
//    +------------------------+ JavaThread::stack_yellow_zone_base()
//    |                        |\
//    |      Normal Stack      | -
//    |                        |/
// P2 +------------------------+ Thread::stack_base()
//
// Non-Java thread:
//
//   Low memory addresses
//    +------------------------+
//    |                        |\
//    |  glibc guard page      | - usually 1 page
//    |                        |/
// P1 +------------------------+ Thread::stack_base() - Thread::stack_size()
//    |                        |\
//    |      Normal Stack      | -
//    |                        |/
// P2 +------------------------+ Thread::stack_base()
//
// ** P2 is the address returned from pthread_attr_getstackaddr(), P2 - P1
//    is the stack size returned by pthread_attr_getstacksize().

static void current_stack_region(address* bottom, size_t* size) {
  if (os::Linux::is_initial_thread()) {
     // initial thread needs special handling because pthread_getattr_np()
     // may return bogus value.
     *bottom = os::Linux::initial_thread_stack_bottom();
     *size = os::Linux::initial_thread_stack_size();
  } else {
     pthread_attr_t attr;

     int rslt = pthread_getattr_np(pthread_self(), &attr);

     // JVM needs to know exact stack location, abort if it fails
     if (rslt != 0) {
       if (rslt == ENOMEM) {
         vm_exit_out_of_memory(0, "pthread_getattr_np");
       } else {
         fatal1("pthread_getattr_np failed with errno = %d", rslt);
       }
     }

     void * top;
     if (pthread_attr_getstackaddr(&attr, &top) != 0 || 
         pthread_attr_getstacksize(&attr, size) != 0) {
         fatal("Can not locate current stack attributes!");
     }

     pthread_attr_destroy(&attr);

     *bottom = (address) align_size_up((uintptr_t)top - *size, os::Linux::page_size());
     *size   = (address)top - *bottom;
  }
  assert(os::current_stack_pointer() >= *bottom &&
         os::current_stack_pointer() < *bottom + *size, "just checking");
}

address os::current_stack_base()
{
  address bottom;
  size_t size;
  current_stack_region(&bottom, &size);
  return bottom + size;
}

size_t os::current_stack_size()
{
  // stack size includes normal stack and HotSpot guard pages
  address bottom;
  size_t size;
  current_stack_region(&bottom, &size);
  return size;
}

/////////////////////////////////////////////////////////////////////////////
// helper functions for fatal error handler

void os::print_context(outputStream* st, void* context)
{
  if (context == NULL) {
    return;
  }

  ucontext_t* uc = (ucontext_t*) context;
  st->print_cr("Registers:");
  st->print(  "RAX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RAX]);
  st->print(", RBX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RBX]);
  st->print(", RCX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RCX]);
  st->print(", RDX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RDX]);
  st->cr();
  st->print(  "RSP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RSP]);
  st->print(", RBP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RBP]);
  st->print(", RSI=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RSI]);
  st->print(", RDI=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RDI]);
  st->cr();
  st->print(  "R8 =" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R8]);
  st->print(", R9 =" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R9]);
  st->print(", R10=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R10]);
  st->print(", R11=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R11]);
  st->cr();
  st->print(  "R12=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R12]);
  st->print(", R13=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R13]);
  st->print(", R14=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R14]);
  st->print(", R15=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_R15]);
  st->cr();
  st->print(  "RIP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_RIP]);
  st->print(", EFL=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EFL]);
  st->print(", CSGSFS=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_CSGSFS]);
  st->print(", ERR=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_ERR]);
  st->cr();
  st->print("  TRAPNO=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_TRAPNO]);
  st->cr();
  st->cr();

  intptr_t* sp = (intptr_t*) os::Linux::ucontext_get_sp(uc);
  st->print_cr("Top of Stack: (sp=" PTR_FORMAT ")", sp);
  print_hex_dump(st, (address) sp, (address) (sp + 8*sizeof(intptr_t)), sizeof(intptr_t));
  st->cr();

  // Note: it may be unsafe to inspect memory near pc. For example, pc
  // may point to garbage if entry point in an nmethod is
  // corrupted. Leave this at the end, and hope for the best.
  address pc = os::Linux::ucontext_get_pc(uc);
  st->print_cr("Instructions: (pc=" PTR_FORMAT ")", pc);
  print_hex_dump(st, pc - 16, pc + 16, sizeof(char));
}
