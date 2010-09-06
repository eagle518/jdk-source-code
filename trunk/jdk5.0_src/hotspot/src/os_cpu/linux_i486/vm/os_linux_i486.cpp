#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_linux_i486.cpp	1.76 04/05/05 16:15:12 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_linux_i486.cpp.incl"
# include "incls/_os_pd.hpp.incl"

// put OS-includes here
# include <sys/types.h>
# include <sys/mman.h>
# include <pthread.h>
# include <signal.h>
# include <errno.h>
# include <dlfcn.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/resource.h>
# include <pthread.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/utsname.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <pwd.h>
# include <poll.h>
# include <ucontext.h>
# include <fpu_control.h>

address os::current_stack_pointer() {
  char dummy;
  return (address) &dummy;
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).

  return (char*) -1;
}

void os::initialize_thread() {
// Nothing to do.
}

address os::Linux::ucontext_get_pc(ucontext_t * uc) {
  return (address)uc->uc_mcontext.gregs[REG_EIP];
}

jint* os::Linux::ucontext_get_sp(ucontext_t * uc) {
  return (jint*)uc->uc_mcontext.gregs[REG_UESP];
}

jint* os::Linux::ucontext_get_fp(ucontext_t * uc) {
  return (jint*)uc->uc_mcontext.gregs[REG_EBP];
}

// For Forte Analyzer AsyncGetCallTrace profiling support - thread
// is currently interrupted by SIGPROF.
// os::Solaris::fetch_frame_from_ucontext() tries to skip nested signal
// frames. Currently we don't do that on Linux, so it's the same as 
// os::fetch_frame_from_context().
ExtendedPC os::Linux::fetch_frame_from_ucontext(Thread* thread,
  ucontext_t* uc, intptr_t** ret_sp, intptr_t** ret_fp) {

  assert(thread != NULL, "just checking");
  assert(ret_sp != NULL, "just checking");
  assert(ret_fp != NULL, "just checking");

  return os::fetch_frame_from_context(uc, ret_sp, ret_fp);
}

ExtendedPC os::fetch_frame_from_context(void* ucVoid,
                    intptr_t** ret_sp, intptr_t** ret_fp) {

  ExtendedPC  epc;
  ucontext_t* uc = (ucontext_t*)ucVoid;

  if (uc != NULL) {
    epc = ExtendedPC(os::Linux::ucontext_get_pc(uc));
    if (ret_sp) *ret_sp = os::Linux::ucontext_get_sp(uc);
    if (ret_fp) *ret_fp = os::Linux::ucontext_get_fp(uc);
  } else {
    // construct empty ExtendedPC for return value checking
    epc = ExtendedPC(NULL);
    if (ret_sp) *ret_sp = (intptr_t *)NULL;
    if (ret_fp) *ret_fp = (intptr_t *)NULL;
  }

  return epc;
}

frame os::fetch_frame_from_context(void* ucVoid) {
  intptr_t* sp;
  intptr_t* fp;
  ExtendedPC epc = fetch_frame_from_context(ucVoid, &sp, &fp);
  return frame(sp, fp, epc.pc());
}

// By default, gcc always save frame pointer (%ebp) on stack. It may get
// turned off by -fomit-frame-pointer,
frame os::get_sender_for_C_frame(frame* fr) {
  return frame(fr->sender_sp(), fr->link(), fr->sender_pc());
}

frame os::current_frame() {
  jint* fp = (*CAST_TO_FN_PTR( jint* (*)(void), StubRoutines::i486::get_previous_fp_entry()))();
  frame myframe((intptr_t*)os::current_stack_pointer(), 
                (intptr_t*)fp,
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
bool os::set_thread_pc_and_resume(JavaThread* thread, ExtendedPC old_addr, ExtendedPC new_addr) {
  assert(thread->is_in_compiled_safepoint(), "only for compiled safepoint");

  OSThread* osthread = thread->osthread();
  assert(thread->is_vm_suspended(), "must be suspended");

  // Note: self-suspended thread or thread suspended when waiting for
  // a mutex can not be repositioned, because it is not suspended
  // inside SR_handler. set_thread_pc() is only called when fetch_top_frame()
  // returns address in nmethod. That cannot be self-suspension nor
  // the mutex case.
  assert(!(thread->is_Java_thread()&&((JavaThread*)thread)->is_ext_suspended())
       &&!thread->osthread()->sr.is_try_mutex_enter(),
         "cannot be self-suspended or suspended while waiting for mutex");

  ucontext_t* context = (ucontext*) osthread->ucontext();
  bool rslt = false;
  if (context->uc_mcontext.gregs[REG_EIP] == (int)old_addr.pc()) {
    context->uc_mcontext.gregs[REG_EIP] = (int)new_addr.pc();
    rslt = true;
  }

  thread->safepoint_state()->notify_set_thread_pc_result(rslt);
  thread->vm_resume();
  return rslt;
}
#endif

// Utility functions

julong os::allocatable_physical_memory(julong size) {
  julong result = MIN2(size, (julong)3800*M);
   if (!is_allocatable(result)) {
     // See comments under solaris for alignment considerations
     julong reasonable_size = (julong)2*G - 2 * os::vm_page_size();
     result =  MIN2(size, reasonable_size);
   }
   return result;
}

// From IA32 System Programming Guide
enum {
  trap_page_fault = 0xE
};

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
/*
  NOTE: does not seem to work on linux.
  if (info == NULL || info->si_code <= 0 || info->si_code == SI_NOINFO) {
    // can't decode this kind of signal
    info = NULL;
  } else {
    assert(sig == info->si_signo, "bad siginfo");
  }
*/
  // decide if this trap can be handled by a stub
  address stub = NULL;

  address pc          = NULL;
  address adjusted_pc = NULL;

  //%note os_trap_1
  if (info != NULL && thread != NULL) {
    // factor me: getPCfromContext
    pc = (address) uc->uc_mcontext.gregs[REG_EIP];
    #ifndef CORE
    adjusted_pc = SafepointPolling ? pc : thread->safepoint_state()->compute_adjusted_pc(pc);
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
            stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::STACK_OVERFLOW);
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
       stub =  Runtime1::entry_for(Runtime1::illegal_instruction_handler_id);
#else
        // debugging trap, or a safepoint
        assert(OptoRuntime::illegal_exception_handler_blob() != NULL, "stub not created yet");
        stub = OptoRuntime::illegal_exception_handler_blob()->instructions_begin();
#endif	// COMPILER1
        CompiledCodeSafepointHandler *handler = (CompiledCodeSafepointHandler *) thread->safepoint_state()->handle();
        if (handler != NULL && thread->is_in_compiled_safepoint()) {
          nmethod* nm = handler->get_nmethod();
          assert(nm != NULL, "safepoint handler is not setup correctly");

          relocInfo::relocType type = nm->reloc_type_for_address(adjusted_pc);
          assert( ((NativeInstruction*)adjusted_pc)->is_call() || 
        	  (type == relocInfo::return_type) || (type == relocInfo::safepoint_type), 
        	  "Only calls, returns, and backward branches are patched at safepoint");

          if(type == relocInfo::return_type) {
            // The retl case: restore has already happened. The safepoint blob frame will appear 
            // immediately below caller of the orignal method that we have patched. The stack could 
            // be walked properly (the frame of the patched method is already popped). 
            // Will need to be revisited if and when we put restore in the delay slot of ret.
          }
          else {
            // This is the case when either a call or a branch was patched with an illegal instruction.
            // At this point we are done with the patched method and would like things to
            // appear as if the orignal method simply called the safepoint blob at a "safe point".
            // To achieve this, we set up the correct linkage by placing the adjusted trap pc in O7
            // and then "returning" from this trap handler to the safepoint blob.

            // QQQ TODO
            // uc->uc_mcontext.gregs[REG_O7] = (greg_t) adjusted_pc;
          }
        }
      }
      else if (sig == SIGSEGV && SafepointPolling && os::is_poll_address((address)uc->uc_mcontext.cr2)) {

        // Look up the code blob
        CodeBlob *cb = CodeCache::find_blob(pc);

        // Should be an nmethod
        assert( cb && cb->is_nmethod(), "safepoint polling: pc must refer to an nmethod" );

        // Look up the relocation information
        assert( ((nmethod*)cb)->is_at_poll_or_poll_return(pc),
          "safepoint polling: type must be poll" );

        assert( ((NativeInstruction*)pc)->is_safepoint_poll(),
          "Only polling locations are used for safepoint");

        // safepoint
        if (((nmethod*)cb)->is_at_poll_return(pc)) {
#ifdef COMPILER1
          stub =  Runtime1::entry_for(Runtime1::polling_page_return_handler_id);
#else
          assert(OptoRuntime::polling_page_return_handler_blob() != NULL, "stub not created yet");
          stub = OptoRuntime::polling_page_return_handler_blob()->instructions_begin();
#endif

#ifndef PRODUCT
          if( TraceSafepoint )
            tty->print("... found polling page return exception at pc = " INTPTR_FORMAT ", stub = " INTPTR_FORMAT "\n",
              (intptr_t)pc, (intptr_t)stub);
#endif
        }
        else {
#ifdef COMPILER1
          stub =  Runtime1::entry_for(Runtime1::polling_page_safepoint_handler_id);
#else
          assert(OptoRuntime::polling_page_safepoint_handler_blob() != NULL, "stub not created yet");
          stub = OptoRuntime::polling_page_safepoint_handler_blob()->instructions_begin();
#endif

#ifndef PRODUCT
          if( TraceSafepoint )
            tty->print("... found polling page safepoint exception at pc = " INTPTR_FORMAT ", stub = " INTPTR_FORMAT "\n",
              (intptr_t)pc, (intptr_t)stub);
#endif
        }
      }
      else
#endif // NOT CORE 

      if (sig == SIGFPE /* && info->si_code == FPE_INTDIV */) {
        // HACK: si_code does not work on linux 2.2.12-20!!!
        int op = pc[0];
        if (op == 0xDB) {
          // FIST
          // TODO: The encoding of D2I in i486.ad can cause an exception
          // prior to the fist instruction if there was an invalid operation
          // pending. We want to dismiss that exception. From the win_32
          // side it also seems that if it really was the fist causing
          // the exception that we do the d2i by hand with different
          // rounding. Seems kind of weird.
          // NOTE: that we take the exception at the NEXT floating point instruction.
          assert(pc[0] == 0xDB, "not a FIST opcode");
          assert(pc[1] == 0x14, "not a FIST opcode");
          assert(pc[2] == 0x24, "not a FIST opcode");
          return true;
        } else if (op == 0xF7) {
          // IDIV
          stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_DIVIDE_BY_ZERO);
        } else {
          // TODO: handle more cases if we are using other x86 instructions
          //   that can generate SIGFPE signal on linux.
          tty->print_cr("unknown opcode 0x%X with SIGFPE.", op);
          fatal("please update this code.");
        }
      }

#ifdef COMPILER2
      else if (sig == SIGSEGV && OptoRuntime::illegal_exception_handler_blob() != NULL &&
               OptoRuntime::illegal_exception_handler_blob()->contains(pc)) {
        // This is the case when we use a trap to restore the context at the end of safepoint
        // taken in compiled code. The "return" address is passed in O7
        // QQQ TODO
        ShouldNotReachHere();

#if 0
        greg_t ret_pc = uc->uc_mcontext.gregs[REG_EIP];
        // Recover global regs saved at the original illegal instruction trap. 
        // (%o are handled "automatically" and are possibly modified by the gc - don't touch them)
        thread->restore_global_regs_from_saved_context(uc);
        uc->uc_mcontext.gregs[REG_EIP] =  ret_pc;
        return true;
#endif	// 0
      }
#endif	// COMPILER2

      else if (sig == SIGSEGV &&
               !MacroAssembler::needs_explicit_null_check(uc->uc_mcontext.cr2)) {
        // Determination of interpreter/vtable stub/compiled code null exception
        stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_NULL);
      }
    }
  }

  // Execution protection violation
  //
  // This should be kept as the last step in the triage.  We don't
  // have a dedicated trap number for a no-execute fault, so be
  // conservative and allow other handlers the first shot.
  //
  // Note: We don't test that info->si_code == SEGV_ACCERR here.
  // this si_code is so generic that it is almost meaningless; and
  // the si_code for this condition may change in the future.
  // Furthermore, a false-positive should be harmless.
  if (UnguardOnExecutionViolation > 0 &&
      (sig == SIGSEGV || sig == SIGBUS) &&
      uc->uc_mcontext.gregs[REG_TRAPNO] == trap_page_fault) {
    int page_size = os::vm_page_size();
    address addr = (address) info->si_addr;
    address pc = (address) uc->uc_mcontext.gregs[REG_EIP];
    // Make sure the pc and the faulting address are sane.
    //
    // If an instruction spans a page boundary, and the page containing
    // the beginning of the instruction is executable but the following
    // page is not, the pc and the faulting address might be slightly
    // different - we still want to unguard the 2nd page in this case.
    //
    // 15 bytes seems to be a (very) safe value for max instruction size.
    bool pc_is_near_addr = 
      (pointer_delta((void*) addr, (void*) pc, sizeof(char)) < 15);
    bool instr_spans_page_boundary =
      (align_size_down((intptr_t) pc ^ (intptr_t) addr,
                       (intptr_t) page_size) > 0);
    
    if (pc == addr || (pc_is_near_addr && instr_spans_page_boundary)) {
      static volatile address last_addr =
        (address) os::non_memory_address_word();
      
      // In conservative mode, don't unguard unless the address is in the VM
      if (addr != last_addr &&
          (UnguardOnExecutionViolation > 1 || os::address_is_in_vm(addr))) {
        
        // Unguard and retry
        address page_start =
          (address) align_size_down((intptr_t) addr, (intptr_t) page_size);
        bool res = os::unguard_memory((char*) page_start, page_size);

        if (PrintMiscellaneous && Verbose) {
          char buf[256];
          jio_snprintf(buf, sizeof(buf), "Execution protection violation "
                       "at " INTPTR_FORMAT
                       ", unguarding " INTPTR_FORMAT ": %s, errno=%d", addr,
                       page_start, (res ? "success" : "failed"), errno);
          tty->print_raw_cr(buf);
        }
        stub = pc;

	// Set last_addr so if we fault again at the same address, we don't end
	// up in an endless loop.
	// 
	// There are two potential complications here.  Two threads trapping at
	// the same address at the same time could cause one of the threads to
	// think it already unguarded, and abort the VM.  Likely very rare.
	// 
	// The other race involves two threads alternately trapping at
	// different addresses and failing to unguard the page, resulting in
	// an endless loop.  This condition is probably even more unlikely than
	// the first.
	//
	// Although both cases could be avoided by using locks or thread local
	// last_addr, these solutions are unnecessary complication: this
	// handler is a best-effort safety net, not a complete solution.  It is
	// disabled by default and should only be used as a workaround in case
	// we missed any no-execute-unsafe VM code.

        last_addr = addr;
      }
    }
  }

  if (stub != NULL) {
    // save all thread context in case we need to restore it

    if (thread != NULL) thread->set_saved_exception_pc(pc);
    // TODO: 
    // Fastlane -- is this all? Do we need to save off any context?
    // Currently, no, but might need to if we'll be returning to
    // the context where the trap occurred. Will need to revisit
    // this for compiled safepoint stubs.  factor me: setPC
    uc->uc_mcontext.gregs[REG_EIP] = (greg_t)stub;
    return true;
  }

  // jni_fast_Get<Primitive>Field can trap at certain pc's if a GC kicks in
  // and the heap gets shrunk before the field access.
  if (sig == SIGSEGV) {
    address addr = JNI_FastGetField::find_slowcase_pc(pc);
    if (addr != (address)-1) {
      uc->uc_mcontext.gregs[REG_EIP] = (greg_t)addr;
      return true;
    }
  }

  // signal-chaining
  if (UseSignalChaining) {
    bool chained = false;
    struct sigaction *actp = os::Linux::get_chained_signal_action(sig);
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
    pc = (address) uc->uc_mcontext.gregs[REG_EIP];
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

void os::Linux::init_thread_fpu_state(void) {
  // set fpu to 53 bit precision
  unsigned int fpu_control = 0x27f;
  _FPU_SETCW(fpu_control);
}

// Check that the linux kernel version is 2.4 or higher since earlier
// versions do not support SSE without patches.
bool os::supports_sse() {
  struct utsname uts;
  if( uname(&uts) != 0 ) return false; // uname fails?
  char *minor_string;
  int major = strtol(uts.release,&minor_string,10);
  int minor = strtol(minor_string+1,NULL,10);
  bool result = (major > 2 || (major==2 && minor >= 4));
#ifndef PRODUCT
  if (PrintMiscellaneous && Verbose) {
    tty->print("OS version is %d.%d, which %s support SSE/SSE2\n",
               major,minor, result ? "DOES" : "does NOT");
  }
#endif
  return result;
}

bool os::is_allocatable(size_t bytes) {

  if (bytes < 2 * G) {
    return true;
  }

  char* addr = reserve_memory(bytes, NULL);

  if (addr != NULL) {
    release_memory(addr, bytes);
  }

  return addr != NULL;
}

////////////////////////////////////////////////////////////////////////////////
// thread stack

size_t os::Linux::min_stack_allowed  =  (48 DEBUG_ONLY(+4))*K;

#define GET_GS() ({int gs; __asm__ volatile("movw %%gs, %w0":"=q"(gs)); gs&0xffff;})

// Test if pthread library can support variable thread stack size. LinuxThreads
// in fixed stack mode allocates 2M fixed slot for each thread. LinuxThreads 
// in floating stack mode and NPTL support variable stack size.
bool os::Linux::supports_variable_stack_size() {
  if (os::Linux::is_NPTL()) {
     // NPTL, yes
     return true;

  } else {
    // Note: We can't control default stack size when creating a thread.
    // If we use non-default stack size (pthread_attr_setstacksize), both 
    // floating stack and non-floating stack LinuxThreads will return the 
    // same value. This makes it impossible to implement this function by
    // detecting thread stack size directly.
    // 
    // An alternative approach is to check %gs. Fixed-stack LinuxThreads
    // do not use %gs, so its value is 0. Floating-stack LinuxThreads use
    // %gs (either as LDT selector or GDT selector, depending on kernel)
    // to access thread specific data. 
    //
    // Note that %gs is a reserved glibc register since early 2001, so 
    // applications are not allowed to change its value (Ulrich Drepper from 
    // Redhat confirmed that all known offenders have been modified to use
    // either %fs or TSD). In the worst case scenario, when VM is embedded in 
    // a native application that plays with %gs, we might see non-zero %gs 
    // even LinuxThreads is running in fixed stack mode. As the result, we'll 
    // return true and skip _thread_safety_check(), so we may not be able to 
    // detect stack-heap collisions. But otherwise it's harmless.
    //
    return (GET_GS() != 0);
  }
}

// return default stack size for thr_type
size_t os::Linux::default_stack_size(os::ThreadType thr_type) {
  // default stack size (compiler thread needs larger stack)
  size_t s = (thr_type == os::compiler_thread ? 2 * M : 512 * K);
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

static void current_stack_region(address * bottom, size_t * size) {
  if (os::Linux::is_initial_thread()) {
     // initial thread needs special handling because pthread_getattr_np()
     // may return bogus value.
     *bottom = os::Linux::initial_thread_stack_bottom();
     *size   = os::Linux::initial_thread_stack_size();
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

address os::current_stack_base() {
  address bottom;
  size_t size;
  current_stack_region(&bottom, &size);
  return (bottom + size);
}

size_t os::current_stack_size() {
  // stack size includes normal stack and HotSpot guard pages
  address bottom;
  size_t size;
  current_stack_region(&bottom, &size);
  return size;
}

/////////////////////////////////////////////////////////////////////////////
// helper functions for fatal error handler

void os::print_context(outputStream *st, void *context) {
  if (context == NULL) return;

  ucontext_t *uc = (ucontext_t*)context;
  st->print_cr("Registers:");
  st->print(  "EAX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EAX]);
  st->print(", EBX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EBX]);
  st->print(", ECX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_ECX]);
  st->print(", EDX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EDX]);
  st->cr();
  st->print(  "ESP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_UESP]);
  st->print(", EBP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EBP]);
  st->print(", ESI=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_ESI]);
  st->print(", EDI=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EDI]);
  st->cr();
  st->print(  "EIP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EIP]);
  st->print(", CR2=" INTPTR_FORMAT, uc->uc_mcontext.cr2);
  st->print(", EFLAGS=" INTPTR_FORMAT, uc->uc_mcontext.gregs[REG_EFL]);
  st->cr();
  st->cr();

  intptr_t *sp = (intptr_t *)os::Linux::ucontext_get_sp(uc);
  st->print_cr("Top of Stack: (sp=" PTR_FORMAT ")", sp);
  print_hex_dump(st, (address)sp, (address)(sp + 32), sizeof(intptr_t));
  st->cr();

  // Note: it may be unsafe to inspect memory near pc. For example, pc may
  // point to garbage if entry point in an nmethod is corrupted. Leave
  // this at the end, and hope for the best.
  address pc = os::Linux::ucontext_get_pc(uc);
  st->print_cr("Instructions: (pc=" PTR_FORMAT ")", pc);
  print_hex_dump(st, pc - 16, pc + 16, sizeof(char));
}
