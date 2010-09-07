//
// Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
// ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

// do not include  precompiled  header file
# include "incls/_os_linux_ia64.cpp.incl"

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

// On Itanium the SP is in r12
#ifndef REG_SP
#define REG_SP 12
#endif


address os::current_stack_pointer() {
  register void* sp __asm__ ("r12");
  return (address)sp;
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).

  return (char*) -1;
}


address os::Linux::ucontext_get_pc(ucontext_t * uc) {
  return (address)uc->uc_mcontext.sc_ip;
}

intptr_t* os::Linux::ucontext_get_sp(ucontext_t * uc) {
  return (intptr_t*)uc->uc_mcontext.sc_gr[REG_SP];
}

intptr_t* os::Linux::ucontext_get_fp(ucontext_t * uc) {
  return (intptr_t*)uc->uc_mcontext.sc_ar_bsp;
}

// For Forte Analyzer AsyncGetCallTrace profiling support - thread
// is currently interrupted by SIGPROF.
ExtendedPC os::Linux::fetch_frame_from_ucontext(Thread* thread,
  ucontext_t* uc, intptr_t** ret_sp, intptr_t** ret_fp) {

  // Forte does not yet support Linux ia64
  Unimplemented();

  ExtendedPC epc;
  return epc;
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
  frame fr(sp, fp);
  fr.set_pc(epc.pc());
  return fr;
}

// IA64 does not support C stack walking.
frame os::get_sender_for_C_frame(frame* fr) {
  frame f(NULL, NULL);
  f.set_pc(NULL);
  return f;
}

frame os::current_frame() {
  frame fr(NULL, NULL);
  fr.set_pc(NULL);
  return fr;
}

// Utility functions

extern "C" int
JVM_handle_linux_signal(int sig,
                        siginfo_t* info,
                        void* ucVoid,
                        int abort_if_unrecognized) {
  ucontext_t* uc = (ucontext_t*) ucVoid;

  Thread* t = ThreadLocalStorage::get_thread_slow();   // slow & steady

  SignalHandlerMark shm(t);

  // Note: it's not uncommon that JNI code uses signal/sigset to install
  // then restore certain signal handler (e.g. to temporarily block SIGPIPE,
  // or have a SIGILL handler when detecting CPU type). When that happens,
  // JVM_handle_linux_signal() might be invoked with junk info/ucVoid. To
  // avoid unnecessary crash when libjsig is not preloaded, try handle signals
  // that do not require siginfo/ucontext first.

  if (sig == SIGPIPE || sig == SIGXFSZ) {
    // allow chained handler to go first
    if (os::Linux::chained_handler(sig, info, ucVoid)) {
      return true;
    } else {
      if (PrintMiscellaneous && (WizardMode || Verbose)) {
        char buf[64];
        warning("Ignoring %s - see bugs 4229104 or 646499219",
                os::exception_name(sig, buf, sizeof(buf)));
      }
      return true;
    }
  }

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

  //%note os_trap_1
  if (info != NULL && uc != NULL && thread != NULL) {
    pc = os::Linux::ucontext_get_pc(uc);

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
      } else if ( addr == NULL ) {
        // Check for a register stack overflow in compiled code.
        // The generated compiled code will write to 0 to signal
        // a register stack overflow.  Verify that this is indeed the
        // condition and start a stack overflow exception in motion.
        if (thread->thread_state() == _thread_in_Java) {
          if ((address)uc->uc_mcontext.sc_ar_bsp > thread->register_stack_limit() ) {
            // 4826555: nsk test stack016 fails.  See stubGenerator_ia64.cpp.
            // Linux kernel puts zero in GR4-7, so this assert always fires.
            //            assert((address)uc->uc_mcontext.sc_gr[7] == thread->register_stack_limit(),
            //                   "GR7 not set to the register stack limit");
            if (thread->stack_yellow_zone_enabled()) {
              thread->disable_stack_yellow_zone();
            }
            thread->disable_register_stack_guard();
            // Give the compiled code the new register stack limit so we
            // don't come back here again.
            uc->uc_mcontext.sc_gr[7] = (unsigned long)thread->register_stack_limit();
            // Throw a stack overflow exception.  Guard pages will be reenabled
            // while unwinding the stack.
            stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::STACK_OVERFLOW);
          }
        }
      }
    }

    if (thread->thread_state() == _thread_in_Java) {
      // Java thread running in Java code => find exception handler if any
      // a fault inside compiled code, the interpreter, or a stub

      if (sig == SIGSEGV && (address)info->si_addr == os::get_polling_page() ) {
        stub = SharedRuntime::get_poll_stub(pc);
      }
      else

      if (sig == SIGFPE /* && info->si_code == FPE_INTDIV */) {
#if 0
        // HACK: si_code does not work on linux 2.2.12-20!!!
        int op = pc[0];
        if (op == 0xDB) {
          // FIST
          // TODO: The encoding of D2I in ia64.ad can cause an exception
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
#else
          tty->print_raw_cr("Fix SIGFPE handler, trying divide by zero handler.");
          stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_DIVIDE_BY_ZERO);
#endif
      }
      else if (sig == SIGSEGV ) {
        //
        // We only expect null pointers in the stubs (vtable only)
        // and c2i adapters
        //
        CodeBlob* cb = CodeCache::find_blob(pc);
        if (cb != NULL) {
          if (VtableStubs::stub_containing(pc) != NULL ||
               cb->is_native_method() || cb->is_adapter_blob() ) {
            // We have to assume it is a NULL fault because info->si_addr
            // is incorrect here
            if (true || ((uintptr_t)info->si_addr) < (uintptr_t)os::vm_page_size() ) {
              // an access to the first page of VM--assume it is a null pointer
              stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_NULL);
            }
          }
        }
      }
    }

    // Check to see if we caught the safepoint code in the
    // process of write protecting the memory serialization page.
    // It write enables the page immediately after protecting it
    // so we can just return to retry the write.
    if ((sig == SIGSEGV) &&
        os::is_memory_serialize_page(thread, (address)info->si_addr)) {
      // Block current thread until the memory serialize page permission restored.
      os::block_on_serialize_page_trap();
      return true;
    }
  }

  if (stub != NULL) {
    // save all thread context in case we need to restore it
    thread->set_saved_exception_pc(pc);
    uc->uc_mcontext.sc_ip = (unsigned long)stub;
    return true;
  }

  // signal-chaining
  if (os::Linux::chained_handler(sig, info, ucVoid)) {
    return true;
  }

  if (!abort_if_unrecognized) {
    // caller wants another chance, so give it to him
    return false;
  }

  if (pc == NULL && uc != NULL) {
    pc = os::Linux::ucontext_get_pc(uc);
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

  // nothing to do on ia64
}

/////////////////////////////////////////////////////////////////////////////
// thread stack

size_t os::Linux::min_stack_allowed  = 768*K;

// ia64: pthread on ia64 is always in floating stack mode
bool os::Linux::supports_variable_stack_size()      { return true; }

// return default stack size for thr_type
size_t os::Linux::default_stack_size(os::ThreadType thr_type) {
  // default stack size (compiler thread needs larger stack)
  size_t s = (thr_type == os::compiler_thread ? 4 * M : 1024 * K);
  return s;
}

size_t os::Linux::default_guard_size(os::ThreadType thr_type) {
  // IA64 put 2 guard pages right in the middle of thread stack. This value
  // should be consistent with the value used by register stack handling code.
  return 2 * page_size();
}

// On IA64 the register windows reside at the bottom of the stack.
// The pthread library divided this stack size in two. The top half
// is dedicated to the memory stack the bottom half is dedicated to
// the register window stack. The pthread library places a guard
// page in the middle of these two regions. The way we deal with this
// situation is basically to make the register window area invisible
// to the VM as far a stack allocation is concerned.
//
// Java thread:
//
//   Low memory addresses
// P1 +------------------------+
//    |                        |\
//    |Register Window Stack   | -
//    |                        |/
//    +------------------------+
//    |                        |\
//    |    Glibc Guard Pages   | - Right in the middle of stack, 2 pages
//    |                        |/
// JJ +------------------------+ Thread::stack_base() - Thread::stack_size()
//    |                        |\
//    |  HotSpot Guard Pages   | -
//    |                        |/
//    +------------------------+ JavaThread::stack_yellow_zone_base()
//    |                        |\
//    |      Memory Stack      | -
//    |                        |/
// P2 +------------------------+ Thread::stack_base()
//
// Non-Java thread on IA64:
//
//   Low memory addresses
// P1 +------------------------+
//    |                        |\
//    |Register Window Stack   | -
//    |                        |/
//    +------------------------+
//    |                        |\
//    |    Glibc Guard Pages   | - Right in the middle of stack, 2 pages
//    |                        |/
// JJ +------------------------+ Thread::stack_base() - Thread::stack_size()
//    |                        |\
//    |      Memory Stack      | -
//    |                        |/
// P2 +------------------------+ Thread::stack_base()
//
// ** P1 (aka bottom) and size ( P2 = P1 - size) are the address and stack size returned from
//    pthread_attr_getstack()
//    As far as VM is concerned, the "bottom" of memory stack is JJ, stack size is P2 - JJ.

static void current_stack_region(address * bottom, size_t * size) {
  if ( os::Linux::is_initial_thread()) {
     /*
        There is a hazard here since we don't know where the "middle"
        guard page is located.  Experience has shown that we miss
        by one page when executing a java command with a lot
        of arguments.  We reserve three extra pages in order to
        avoid missing.
      */
      #define MIDDLE_BUFFER_PAGE_COUNT 3

     *bottom = os::Linux::initial_thread_stack_bottom() +
               (os::Linux::initial_thread_stack_size() / 2) +
               (MIDDLE_BUFFER_PAGE_COUNT * os::Linux::page_size());
     *size   = (os::Linux::initial_thread_stack_size() / 2) -
               (MIDDLE_BUFFER_PAGE_COUNT * os::Linux::page_size());
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

    size_t guard_size;
    if (pthread_attr_getstack(&attr, (void **)bottom, size) != 0 ||
        pthread_attr_getguardsize(&attr, &guard_size) != 0 ) {
       fatal("Can not locate current stack attributes!");
    }

    pthread_attr_destroy(&attr);

    // The size returned from glibc includes the requested stack size
    // plus any guard pages we requested.  We want to only report usable
    // stack space so we reduce the stack size by the guard page amount.

    address top = (address) ((uintptr_t)(*bottom) + *size);

    uintptr_t false_bottom =  (uintptr_t)top - ((*size - guard_size)/ 2);
    *bottom = (address) align_size_up(false_bottom, os::Linux::page_size());
    *size   = top - *bottom;
  }
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

// OS specific thread initialization
//
// For Itanium, we calculate and store the limits of the
// register and memory stacks.
//
void os::initialize_thread() {
  address mem_stk_limit;
  uintptr_t current_bsp;
  uintptr_t mask_bsp;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // 4900635: We now use the current BSP as the basis
  // for our register stack limit.  This works for both
  // the primary thread as well as linuxthread created
  // threads.  We used to assume that the register stack
  // was in the same region of memory as the memory
  // stack.  This is true for all threads other than
  // the primary stack. We now only assume that the
  // register stack is the same size as memory stack
  // and use the currently running register stack pointer
  // (bsp) to find the base which we use to calculate
  // our limit.

  __asm__ volatile ("mov %0=ar.bsp" : "=r" (current_bsp) );
  mask_bsp = ~(uintptr_t)(os::vm_page_size()-1);
  thread->set_register_stack_base((address)(current_bsp & mask_bsp));

  // Initialize our register stack limit.  This is our guard.
  JavaThread::enable_register_stack_guard();

  // Initialize the memory stack limit
  mem_stk_limit = thread->stack_base() - thread->stack_size() +
                  ((StackShadowPages + StackYellowPages + StackRedPages)
                  * os::vm_page_size());

  thread->set_memory_stack_limit( mem_stk_limit );
}



// Check to see if the current BSP is within our guard
// page area.
bool JavaThread::register_stack_overflow()  {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return false;

  reg_stk_limit = thread->register_stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackYellowPages + StackRedPages)
                  * os::vm_page_size());

  if ( StubRoutines::ia64::get_backing_store_pointer() > reg_stk_limit )
    return true;
  else
    return false;
}

// Set the guard page address to it's normal guarded position.
// Compiled code and interpreter entry compares the current
// BSP to this address to check for overflow.

void JavaThread::enable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // We assume that the register stack is the same size as the
  // memory stack.

  reg_stk_limit = thread->register_stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackYellowPages + StackRedPages)
                  * os::vm_page_size());

  thread->set_register_stack_limit( reg_stk_limit );
}

// Reduce the guard page by StackShadowPages to allow for the processing
// of register stack overflow exceptions.
void JavaThread::disable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  reg_stk_limit = thread->register_stack_base() + thread->stack_size() -
                  ((StackYellowPages + StackRedPages) * os::vm_page_size());

  thread->set_register_stack_limit( reg_stk_limit );
}

void os::print_context(outputStream *st, void *context) {
  if (context == NULL) return;

  st->print_cr("Registers:");

  ucontext_t* uc = (ucontext_t*)context;

#define _R_(reg) uc->uc_mcontext.sc_##reg

  st->print   ("unat=" INTPTR_FORMAT "  ", _R_(ar_unat));  // user NaT
  st->print   ("lc=" INTPTR_FORMAT "  ", _R_(ar_lc));      // loop counter
  st->print_cr("ccv=" INTPTR_FORMAT "  ", _R_(ar_ccv));    // cmpxchg register

  st->print   ("pfs=" INTPTR_FORMAT "  ", _R_(ar_pfs));    // previous function state
  st->print   ("bsp=" INTPTR_FORMAT "  ", _R_(ar_bsp));    // backing store pointer
  st->print_cr("cfm=" INTPTR_FORMAT "  ", _R_(cfm));       // current frame marker
  st->print   ("rsc=" INTPTR_FORMAT "  ", _R_(ar_rsc));    // RSE config
  st->print_cr("rnat=" INTPTR_FORMAT "  ", _R_(ar_rnat));  // RSE Nat

  st->print   ("ip=" INTPTR_FORMAT "  ", _R_(ip));  // ip
  st->print_cr("pr=" INTPTR_FORMAT "  ", _R_(pr));  // predicates

  int i;

  // b0-b7
  for (i = 0; i < 8; i++) {
    st->print("b%d=" INTPTR_FORMAT "  ", i, uc->uc_mcontext.sc_br[i]);
    if (i < 10) st->print(" ");
    if (i % 3 == 2) st->cr();
  }
  st->cr();

  // r0-r31 (sp is r12)
  for (i = 0; i < 32; i++) {
    st->print("r%d=" INTPTR_FORMAT "  ", i, uc->uc_mcontext.sc_gr[i]);
    if (i < 10) st->print(" ");
    if (i % 3 == 2) st->cr();
  }
  st->cr();
  st->cr();

  intptr_t *sp = (intptr_t *)os::Linux::ucontext_get_sp(uc);
  st->print_cr("Top of Stack: (sp=" PTR_FORMAT ")", sp);
  print_hex_dump(st, (address)sp, (address)(sp + 32), sizeof(intptr_t));
  st->cr();

  intptr_t *fp = (intptr_t *)os::Linux::ucontext_get_fp(uc);
  st->print_cr("Top of Register Stack: (bsp=" PTR_FORMAT ")", fp);
  print_hex_dump(st, (address)(fp - 31), (address)(fp + 1), sizeof(intptr_t));
  st->cr();

  // Note: it may be unsafe to inspect memory near pc. For example, pc may
  // point to garbage if entry point in an nmethod is corrupted. Leave
  // this at the end, and hope for the best.
  address pc = os::Linux::ucontext_get_pc(uc);
  st->print_cr("Instructions: (pc=" PTR_FORMAT ")", pc);
  print_hex_dump(st, pc - 16, pc + 16, sizeof(char));
}
