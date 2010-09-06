#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_solaris_i486.cpp	1.95 04/05/05 16:15:12 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_solaris_i486.cpp.incl"
# include "incls/_os_pd.hpp.incl"

// put OS-includes here
# include <sys/types.h>
# include <sys/mman.h>
# include <pthread.h>
# include <signal.h>
# include <errno.h>
# include <dlfcn.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/resource.h>
# include <thread.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/filio.h>
# include <sys/utsname.h>
# include <sys/systeminfo.h>
# include <sys/socket.h>
# include <sys/trap.h>
# include <sys/lwp.h>
# include <pwd.h>
# include <poll.h>
# include <sys/lwp.h>

# define _STRUCTURED_PROC 1  //  this gets us the new structured proc interfaces of 5.6 & later
# include <sys/procfs.h>     //  see comment in <sys/procfs.h>


#define MAX_PATH (2 * K)

// Minimum stack size for the VM.  It's easier to document a constant value
// but it's different for x86 and sparc because the page sizes are different.
size_t os::Solaris::min_stack_allowed = 64*K;

static char*  reserved_addr = 0;
static size_t reserved_increment = 0x1000000;

// 4900493 counter to prevent runaway LDTR refresh attempt

static volatile int ldtr_refresh = 0;

// the libthread instruction that faults because of the stale LDTR

static const unsigned char movlfs[] = { 0x8e, 0xe0    // movl %eax,%fs
                       };

// Thanks, Mario Wolzcko for the code below: (Ungar 5/97)

char* os::Solaris::mmap_chunk(char *addr, size_t size, int flags, int prot) {
  char *b = mmap(addr, size, prot, flags, os::Solaris::_dev_zero_fd, 0);

  if (b == MAP_FAILED) {
    return NULL;
  }

  // QQQ TODO this could be moved back to os_solaris.cpp if high_half_mask was factored instead
  // of this entire routine and reserve_memory was also moved backed to os_solaris.cpp

  // Ensure MP-correctness when we patch instructions containing addresses.
  const int high_half_mask = -1 << 24;   // misalignment can separate high byte
  guarantee(((long)b & high_half_mask) != ((long)os::non_memory_address_word() & high_half_mask), "high half of address must not be all-zero");
  
  return b;
}
char* os::reserve_memory(size_t bytes, char* requested_addr) {
  // On Sparc, addr is not critical,
  // for Intel Solaris, though, must ensure heap is at address with
  // sentinel bit set -- assuming stack is up there, too.
  // - Ungar 11/97

  char* addr;
  int   flags;

  if (requested_addr != NULL) {
      flags = MAP_FIXED | MAP_PRIVATE | MAP_NORESERVE;
      addr = requested_addr;
  } else {
      flags = MAP_PRIVATE | MAP_NORESERVE;
      addr = reserved_addr + 0x40000000; // A hint to mmap
      reserved_addr = (char*)(
		((intptr_t)reserved_addr + bytes + reserved_increment - 1)
		     & ~(reserved_increment - 1));
  }

  // Map uncommitted pages PROT_NONE so we fail early if we touch an
  // uncommitted page. Otherwise, the read/write might succeed if we
  // have enough swap space to back the physical page.
  addr = Solaris::mmap_chunk(addr, bytes, flags, PROT_NONE);

  guarantee(requested_addr == NULL || requested_addr == addr,
	    "OS failed to return requested mmap address.");

  return addr;
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).

  // On Intel, virtual addresses never have the sign bit set.
  return (char*) -1;
}

//
// Validate a ucontext retrieved from walking a uc_link of a ucontext.
// There are issues with libthread giving out uc_links for different threads
// on the same uc_link chain and bad or circular links. 
//
bool os::Solaris::valid_ucontext(Thread* thread, ucontext_t* valid, ucontext_t* suspect) {
  if (valid >= suspect || 
      valid->uc_stack.ss_flags != suspect->uc_stack.ss_flags ||
      valid->uc_stack.ss_sp    != suspect->uc_stack.ss_sp    ||
      valid->uc_stack.ss_size  != suspect->uc_stack.ss_size) {
    DEBUG_ONLY(tty->print_cr("valid_ucontext: failed test 1");)
    return false;
  }

  if (thread->is_Java_thread()) {
    if (!valid_stack_address(thread, (address)suspect)) {
      DEBUG_ONLY(tty->print_cr("valid_ucontext: uc_link not in thread stack");)
      return false;
    }
    if (!valid_stack_address(thread,  (address) suspect->uc_mcontext.gregs[UESP])) {
      DEBUG_ONLY(tty->print_cr("valid_ucontext: stackpointer not in thread stack");)
      return false;
    }
  }
  return true;
}

// We will only follow one level of uc_link since there are libthread
// issues with ucontext linking and it is better to be safe and just
// let caller retry later.
ucontext_t* os::Solaris::get_valid_uc_in_signal_handler(Thread *thread,
  ucontext_t *uc) {

  ucontext_t *retuc = NULL;

  if (uc != NULL) {
    if (uc->uc_link == NULL) {
      // cannot validate without uc_link so accept current ucontext
      retuc = uc;
    } else if (os::Solaris::valid_ucontext(thread, uc, uc->uc_link)) {
      // first ucontext is valid so try the next one
      uc = uc->uc_link;
      if (uc->uc_link == NULL) {
        // cannot validate without uc_link so accept current ucontext
        retuc = uc;
      } else if (os::Solaris::valid_ucontext(thread, uc, uc->uc_link)) {
        // the ucontext one level down is also valid so return it
        retuc = uc;
      }
    }
  }
  return retuc;
}

// Assumes ucontext is valid
ExtendedPC os::Solaris::ucontext_get_ExtendedPC(ucontext_t *uc) {
  return ExtendedPC((address)uc->uc_mcontext.gregs[EIP]);
}

// Assumes ucontext is valid
intptr_t* os::Solaris::ucontext_get_sp(ucontext_t *uc) {
  return (intptr_t*)uc->uc_mcontext.gregs[UESP];
}

// Assumes ucontext is valid
intptr_t* os::Solaris::ucontext_get_fp(ucontext_t *uc) {
  return (intptr_t*)uc->uc_mcontext.gregs[EBP];
}

// For Forte Analyzer AsyncGetCallTrace profiling support - thread
// is currently interrupted by SIGPROF.
//
// The difference between this and os::fetch_frame_from_context() is that
// here we try to skip nested signal frames.
ExtendedPC os::Solaris::fetch_frame_from_ucontext(Thread* thread,
  ucontext_t* uc, intptr_t** ret_sp, intptr_t** ret_fp) {

  assert(thread != NULL, "just checking");
  assert(ret_sp != NULL, "just checking");
  assert(ret_fp != NULL, "just checking");

  ucontext_t *luc = os::Solaris::get_valid_uc_in_signal_handler(thread, uc);
  return os::fetch_frame_from_context(luc, ret_sp, ret_fp);
}

ExtendedPC os::fetch_frame_from_context(void* ucVoid, 
                    intptr_t** ret_sp, intptr_t** ret_fp) {

  ExtendedPC  epc;
  ucontext_t *uc = (ucontext_t*)ucVoid;

  if (uc != NULL) {
    epc = os::Solaris::ucontext_get_ExtendedPC(uc);
    if (ret_sp) *ret_sp = os::Solaris::ucontext_get_sp(uc);
    if (ret_fp) *ret_fp = os::Solaris::ucontext_get_fp(uc);
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
// synchronous: cancelled by the sender thread
void SetThreadPC_Callback::execute(OSThread::InterruptArguments *args) {
  assert(args->thread()->is_Java_thread(), "must be a java thread");

  JavaThread* thread = (JavaThread*)args->thread();
  ucontext_t* uc     = args->ucontext();

  // In some instances with some versions of libthread we get this callback while
  // we are executing libthread signal handling code (during preemption/resumption) 
  // and we used to blindly try update the pc with a java code pc. This seriously
  // messes up the state of the world. Now that we have made the operation synchronous
  // we can examine the pc and either find the expected pc or not. We are willing to
  // examine at most a single nest ucontext. There are libthread issues with the
  // sanity of uc_link (cycles, cross thread stacks, ...) so the less we use uc_link
  // the better. If we find the expected pc we patch it and return (to VM thread typically)
  // success, if not we return failure and the caller can retry.
  // 
  _result = false;
  if (uc->uc_mcontext.gregs[EIP] == (greg_t)_old_addr.pc() ) {
    uc->uc_mcontext.gregs[EIP] = (greg_t)_new_addr.pc();
    _result = true;
  } else if (uc->uc_link != NULL) {
    // Check (and validate) one level of stacked ucontext
    ucontext_t* linked_uc = uc->uc_link;
    if (os::Solaris::valid_ucontext(thread, uc, linked_uc) && linked_uc->uc_mcontext.gregs[EIP] == (greg_t)_old_addr.pc()) {
      linked_uc->uc_mcontext.gregs[EIP] = (greg_t)_new_addr.pc();
      _result = true;
    }
  }

  thread->safepoint_state()->notify_set_thread_pc_result(_result);

#ifdef ASSERT
  if (!_result) { 
    if (uc->uc_link != NULL) { 
      tty->print_cr("set_thread_pc:(nested) failed to set pc " INTPTR_FORMAT " -> " INTPTR_FORMAT, _old_addr.pc(), _new_addr.pc()); 
    } else { 
      tty->print_cr("set_thread_pc: failed to set pc " INTPTR_FORMAT " -> " INTPTR_FORMAT, _old_addr.pc(), _new_addr.pc()); 
    } 
  } 
#endif
}
#endif

// This is a simple callback that just fetches a PC for an interrupted thread.
// The thread need not be suspended and the fetched PC is just a hint.
// Returned PC and nPC are not necessarily consecutive.
// This one is currently used for profiling the VMThread ONLY!

// Must be synchronous
void GetThreadPC_Callback::execute(OSThread::InterruptArguments *args) {
  Thread*     thread = args->thread();
  ucontext_t* uc     = args->ucontext();
  jint* sp;

  assert(ProfileVM && thread->is_VM_thread(), "just checking");
    
  ExtendedPC new_addr((address)uc->uc_mcontext.gregs[EIP]);
  _addr = new_addr;
}


static int threadgetstate(thread_t tid, int *flags, lwpid_t *lwp, stack_t *ss, gregset_t rs, lwpstatus_t *lwpstatus) {
  char lwpstatusfile[PROCFILE_LENGTH];
  int lwpfd, err;

  if (err = os::Solaris::thr_getstate(tid, flags, lwp, ss, rs))
    return (err);
  if (*flags == TRS_LWPID) {
    sprintf(lwpstatusfile, "/proc/%d/lwp/%d/lwpstatus", getpid(),
	    *lwp);
    if ((lwpfd = open(lwpstatusfile, O_RDONLY)) < 0) {
      perror("thr_mutator_status: open lwpstatus");
      return (EINVAL);
    }
    if (pread(lwpfd, lwpstatus, sizeof (lwpstatus_t), (off_t)0) !=
	sizeof (lwpstatus_t)) {
      perror("thr_mutator_status: read lwpstatus");
      (void) close(lwpfd);
      return (EINVAL);
    }
    (void) close(lwpfd);
  }
  return (0);
}


// Returns a pointer to ucontext from SIGLWP handler by walking up the stack
// NULL indicates failure
static ucontext_t* get_ucontext_from_siglwp_handler(Thread* thread, address s_pc, jint* s_sp, jint* s_fp) {

// This routine is relatively fragile.
// If we wander back into a c2 compiled frame, we can't trust our frame pointer
// any more, as the compiler can use ebp as a general register.
// As a result we try very hard to ensure that no references will cause
// an accidental segv.
//
   ucontext_t *uc = NULL;
   address stackStart = (address) thread->stack_base();
   address stackEnd   = stackStart - thread->stack_size();
   int i = 0;
   jint* s_oldfp = s_fp - 1;  // initialize to smaller value
   intptr_t page_mask = (intptr_t) ~(os::vm_page_size() - 1);

   // walk back on stack until the signal handler is found
   // only follow legitimate frame pointers. Note: this can
   // still be fooled but as long as we don't segv we are ok.
   while (1) {

     // Check for bad pc's and bad fp's. We don't need sp and since
     // all but the initial sp are simply derived from fp it is a
     // waste to check 2..n. We have also seen a problem where the
     // sp that is delivered is stale (see bug 4335248 on sparc)
     // so we just completely ignore sp.

     if ( s_pc == (address) -1 || s_oldfp >= s_fp ||
	 (intptr_t) s_fp & 0x3 || (address) s_fp > stackStart - wordSize || (address) s_fp <= stackEnd ) {
         // The real stackStart seems to be one word less that what is returned by thr_stksegment.
       return NULL;
     }

     // The siglwp handler is known to be 4-7 frames back
     if (++i > 10) {
       return NULL;
     }
     if (s_pc >= os::Solaris::handler_start && s_pc < os::Solaris::handler_end) {
       frame sighandler_frame(s_sp, s_fp, s_pc);
       return *(ucontext_t **)sighandler_frame.native_param_addr(2);
     }

     frame fr(s_sp, s_fp, s_pc);
     s_sp = fr.sender_sp();
     s_oldfp = s_fp;

     s_fp = fr.link();
     s_pc = fr.sender_pc();
  }
  ShouldNotReachHere();

  return NULL;
}

// This is a "fast" implementation of fetch_top_frame that relies on the new libthread
// APIs to get pc for a suspended thread
ExtendedPC os::Solaris::fetch_top_frame_fast(Thread* thread, jint** sp, jint** fp) {
  int flag;
  lwpid_t lwpid;
  gregset_t rs;
  lwpstatus_t lwpstatus;
  int res;
  ExtendedPC addr;
  
  #ifdef ASSERT
  memset(&rs[0], 0xff, sizeof(gregset_t));
  #endif

  thread_t tid = thread->osthread()->thread_id();
  res = threadgetstate(tid, &flag, &lwpid, NULL, rs, &lwpstatus);
  // Let caller know about failure 
  // thread self-suspension uses wait on SR_lock, so
  // thread is not t_stop or t_stopallmutators
//  assert(res == 0, "threadgetstate() failure"); // guarantee
  if(res) {
    ExtendedPC new_addr(NULL);
    *sp   = (jint*)0;
    *fp   = (jint*)0;
    addr = new_addr;
    return addr;  // bail out
  }
  
  switch(flag) {
  case TRS_NONVOLATILE:   
      // called when thread voluntarily gave up control via thr_suspend().
    // Even worse, the register values that libthread returns are for the libthread's
    // own SIGLWP handler - a suspended thread sleeps on a synchronization object
    // inside this handler... 
    if (Arguments::has_profile() || UseStrictCompilerSafepoints || jvmpi::enabled()) {
	ucontext_t *uc = NULL;

	// get_ucontext_from_siglwp_handler is too dangerous to call unless thread state
	// is _thread_in_Java. It is dangerous then too but we must use it if we expect
	// to be able to reach a safepoint with a long (infinite) loop in Java code.
	// 
	if (thread->is_Java_thread()) {
	  JavaThread* jthread = (JavaThread *)thread;
	  if (jthread->thread_state() == _thread_in_Java) {
	    uc = get_ucontext_from_siglwp_handler(thread, (address)rs[EIP], (jint*)rs[UESP], (jint*)rs[EBP]);
	  }
	}
        // if we can not find the signal handler on the stack,
        // set new_addr to null, which get_top_frame will use
        // to return false, which causes examine_state_of_thread to
        // roll_forward and SafepointSynchronize::begin() will
        // retry this thread again.
        if (uc == NULL) {
            ExtendedPC new_addr(NULL);
            *sp   = (jint*)0;
            *fp   = (jint*)0;
            addr = new_addr;
        }
        else {
            ExtendedPC new_addr((address)uc->uc_mcontext.gregs[EIP]);
            addr = new_addr;
	    assert(uc->uc_mcontext.gregs[EIP] != NULL, "cannot be null");
	    *sp = (jint *)uc->uc_mcontext.gregs[UESP];
	    *fp = (jint *)uc->uc_mcontext.gregs[EBP];
        }
        break;
    }
    else
      ;// fallthrough to invalid case
  case TRS_INVALID:
    {
      ExtendedPC new_addr(NULL);
      *sp   = (jint*)0;
      *fp   = (jint*)0;
      addr = new_addr;
    }
  break;


  case TRS_VALID:         // the entire register set is cached for us by libthread
    {
      assert(rs[UESP] != NULL, "stack point shouldn't be null in TRS_VALID case");
      ExtendedPC new_addr((address)rs[EIP]);
      *sp   = (jint*)rs[UESP];
      *fp   = (jint*)rs[EBP];
      assert(rs[EIP] != NULL, "cannot be null");
      addr = new_addr;
    }
  break;

  case TRS_LWPID:        // got a full register set from the /proc interface
    // Fastlane?
    {
      assert(lwpstatus.pr_reg[UESP] != 0, "stack point shouldn't be null in TRS_LWPID case");
      ExtendedPC new_addr((address)lwpstatus.pr_reg[EIP]);
      assert(lwpstatus.pr_reg[EIP] != NULL, "cannot be null");
      *sp   = (jint*)lwpstatus.pr_reg[UESP];
      *fp   = (jint*)lwpstatus.pr_reg[EBP];
      addr = new_addr;
    }
  break;
  default:
    break;
  }
  
  return addr;
}

// Detecting SSE support by OS
// From solaris_i486.s
extern "C" bool sse_check();
extern "C" bool sse_unavailable();

enum { SSE_UNKNOWN, SSE_NOT_SUPPORTED, SSE_SUPPORTED};
static int sse_status = SSE_UNKNOWN;


static void  check_for_sse_support() {
  if (!VM_Version::supports_sse()) {
    sse_status = SSE_NOT_SUPPORTED;
    return;
  }
  // looking for _sse_hw in libc.so, if it does not exist or
  // the value (int) is 0, OS has no support for SSE 
  int *sse_hwp;
  void *h;

  if ((h=dlopen("/usr/lib/libc.so", RTLD_LAZY)) == NULL) {
    //open failed, presume no support for SSE
    sse_status = SSE_NOT_SUPPORTED;
    return;
  }
  if ((sse_hwp = (int *)dlsym(h, "_sse_hw")) == NULL) {
    sse_status = SSE_NOT_SUPPORTED;
  } else if (*sse_hwp == 0) {
    sse_status = SSE_NOT_SUPPORTED;
  }
  dlclose(h);

  if (sse_status == SSE_UNKNOWN) {
    bool (*try_sse)() = (bool (*)())sse_check;
    sse_status = (*try_sse)() ? SSE_SUPPORTED : SSE_NOT_SUPPORTED;
  }

}

bool os::supports_sse() {
  if (sse_status == SSE_UNKNOWN)
    check_for_sse_support();
  return sse_status == SSE_SUPPORTED;
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

extern "C" int JVM_handle_solaris_signal(int signo, siginfo_t* siginfo, void* ucontext, int abort_if_unrecognized);

int JVM_handle_solaris_signal(int sig, siginfo_t* info, void* ucVoid, int abort_if_unrecognized) {
  ucontext_t* uc = (ucontext_t*) ucVoid;

  if (sig == SIGILL && info->si_addr == (caddr_t)sse_check) {
    // the SSE instruction faulted. supports_sse() need return false.
    uc->uc_mcontext.gregs[EIP] = (greg_t)sse_unavailable;
    return true;
  }

  Thread* t = ThreadLocalStorage::get_thread_slow();  // slow & steady

  SignalHandlerMark shm(t);

  JavaThread* thread = NULL;
  VMThread* vmthread = NULL;

  if (os::Solaris::signal_handlers_are_installed) {
    if (t != NULL ){
      if(t->is_Java_thread()) {
	thread = (JavaThread*)t;
      }
      else if(t->is_VM_thread()){
	vmthread = (VMThread *)t;
      }
    }
  }

  guarantee(sig != os::Solaris::SIGinterrupt(), "Can not chain VM interrupt signal, try -XX:+UseAltSigs");

  if (sig == os::Solaris::SIGasync()) {
    if(thread){
      OSThread::InterruptArguments args(thread, uc);
      thread->osthread()->do_interrupt_callbacks_at_interrupt(&args);
      return true; 
    } 
    else if(vmthread){
      OSThread::InterruptArguments args(vmthread, uc);
      vmthread->osthread()->do_interrupt_callbacks_at_interrupt(&args);
      return true;
    }
  }

  if (info == NULL || info->si_code <= 0 || info->si_code == SI_NOINFO) {
    // can't decode this kind of signal
    info = NULL;
  } else {
    assert(sig == info->si_signo, "bad siginfo");
  }

  // decide if this trap can be handled by a stub
  address stub = NULL;

  address pc          = NULL;
  address adjusted_pc = NULL;

  //%note os_trap_1
  if (info != NULL && thread != NULL) {
    // factor me: getPCfromContext
    pc = (address) uc->uc_mcontext.gregs[EIP];
    #ifndef CORE
    adjusted_pc = SafepointPolling ? pc : thread->safepoint_state()->compute_adjusted_pc(pc);
    #else
    adjusted_pc = pc;
    #endif

    // Handle ALL stack overflow variations here
    if (sig == SIGSEGV && info->si_code == SEGV_ACCERR) {
      address addr = (address) info->si_addr;
      if (thread->in_stack_yellow_zone(addr)) {
	thread->disable_stack_yellow_zone();
	if (thread->thread_state() == _thread_in_Java) {
	  // Throw a stack overflow exception.  Guard pages will be reenabled
	  // while unwinding the stack.
	  stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::STACK_OVERFLOW);
	} else {
	  // Thread was in the vm or native code.  Return and try to finish.
	  return true;
	}
      } else if (thread->in_stack_red_zone(addr)) {
	// Fatal red zone violation.  Disable the guard pages and fall through
	// to handle_unexpected_exception way down below.
	thread->disable_stack_red_zone();
	tty->print_raw_cr("An irrecoverable stack overflow has occurred.");
      }
    }

    if (thread->thread_state() == _thread_in_vm) {
      if (sig == SIGBUS && info->si_code == BUS_OBJERR && thread->doing_unsafe_access()) {
	stub = StubRoutines::i486::handler_for_unsafe_access();
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

	    // if(type == relocInfo::return_type) {
	    // The retl case: restore has already happened. The safepoint blob frame will appear 
	    // immediately below caller of the orignal method that we have patched. The stack could 
	    // be walked properly (the frame of the patched method is already popped). 
	    // Sparc: Will need to be revisited if and when we put restore in the delay slot of ret.
	    // }
	    // else {
	    // This is the case when either a call or a branch was patched with an illegal instruction.
	    // At this point we are done with the patched method and would like things to
	    // appear as if the orignal method simply called the safepoint blob at a "safe point".
	    // Sparc: To achieve this, we set up the correct linkage by placing the adjusted trap pc in O7
	    // and then "returning" from this trap handler to the safepoint blob.
            // Intel: to achieve this the handle_illegal_instruction_exception code 
            // called by the stub will place saved_exception_pc into %sp and return there

	    // }
	}
      }

      // Support Safepoint Polling
      else if ( sig == SIGSEGV && SafepointPolling && os::is_poll_address((address)info->si_addr)) {

        // Look up the code blob
        CodeBlob *cb = CodeCache::find_blob(pc);

        // Should be an nmethod
        if( cb && cb->is_nmethod() ) {

          // Look up the relocation information
          assert( ((nmethod*)cb)->is_at_poll_or_poll_return(pc),
            "not poll or poll_return" );
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

          // return
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
      }
      else if (sig == SIGFPE && info->si_code == FPE_INTDIV) {
#else
      if (sig == SIGFPE && info->si_code == FPE_INTDIV) {
#endif	// ! CORE
// TODO -- needs work - fastlane
// is_zombie needs factoring?
	// integer divide by zero
// TODO -- needs work - fastlane
        stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_DIVIDE_BY_ZERO);
      }
      else if (sig == SIGFPE && info->si_code == FPE_FLTDIV) {
	// floating-point divide by zero
// TODO -- needs work - fastlane
        stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_DIVIDE_BY_ZERO);
      }
      else if (sig == SIGFPE && info->si_code == FPE_FLTINV) {
	// The encoding of D2I in i486.ad can cause an exception prior
	// to the fist instruction if there was an invalid operation 
	// pending. We want to dismiss that exception. From the win_32
	// side it also seems that if it really was the fist causing
	// the exception that we do the d2i by hand with different
	// rounding. Seems kind of weird. QQQ TODO
	// Note that we take the exception at the NEXT floating point instruction.
	if (pc[0] == 0xDB) {
	    assert(pc[0] == 0xDB, "not a FIST opcode");
	    assert(pc[1] == 0x14, "not a FIST opcode");
	    assert(pc[2] == 0x24, "not a FIST opcode");
	    return true;
	} else {
	    assert(pc[-3] == 0xDB, "not an flt invalid opcode");
	    assert(pc[-2] == 0x14, "not an flt invalid opcode");
	    assert(pc[-1] == 0x24, "not an flt invalid opcode");
	}
      }
      else if (sig == SIGFPE ) {
        tty->print_cr("caught SIGFPE, info 0x%x.", info->si_code);
      }
	// QQQ It doesn't seem that we need to do this on x86 because we should be able
	// to return properly from the handler without this extra stuff on the back side.

#ifdef COMPILER2
      else if (sig == SIGSEGV && OptoRuntime::illegal_exception_handler_blob() != NULL &&
               OptoRuntime::illegal_exception_handler_blob()->contains(pc)) {
	// This is the case when we use a trap to restore the context at the end of safepoint
	// taken in compiled code. The "return" address is passed in O7
	// QQQ TODO
        ShouldNotReachHere();

      }
#endif	// COMPILER2

      else if (sig == SIGSEGV && info->si_code > 0 && !MacroAssembler::needs_explicit_null_check((int)info->si_addr)) {
        // Determination of interpreter/vtable stub/compiled code null exception
        stub = SharedRuntime::continuation_for_implicit_exception(thread, pc, SharedRuntime::IMPLICIT_NULL);
      }

#ifndef CORE
      else if (sig == SIGBUS && info->si_code == BUS_OBJERR) {
	// BugId 4454115: A read from a MappedByteBuffer can fault
	// here if the underlying file has been truncated.
	// Do not crash the VM in such a case.
        CodeBlob* cb = CodeCache::find_blob_unsafe(adjusted_pc);
        nmethod* nm = cb->is_nmethod() ? (nmethod*)cb : NULL;
        if (nm != NULL && nm->has_unsafe_access()) {
	  stub = StubRoutines::i486::handler_for_unsafe_access();
	}
      }
#endif //CORE
    }
  }

  // Execution protection violation
  //
  // Preventative code for future versions of Solaris which may
  // enable execution protection when running the 32-bit VM on AMD64.
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
      uc->uc_mcontext.gregs[TRAPNO] == T_PGFLT) {  // page fault
    int page_size = os::vm_page_size();
    address addr = (address) info->si_addr;
    address pc = (address) uc->uc_mcontext.gregs[EIP];
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
    // 12/02/99: On Sparc it appears that the full context is alsosaved
    // but as yet, no one looks at or restores that saved context
    // factor me: setPC
    uc->uc_mcontext.gregs[EIP] = (greg_t)stub;
    return true;
  }

  // jni_fast_Get<Primitive>Field can trap at certain pc's if a GC kicks in
  // and the heap gets shrunk before the field access.
  if (sig == SIGSEGV) {
    address addr = JNI_FastGetField::find_slowcase_pc(pc);
    if (addr != (address)-1) {
      uc->uc_mcontext.gregs[EIP] = (greg_t)addr;
      return true;
    }
  }

  // signal-chaining
  if (UseSignalChaining) {
    bool chained = false;
    struct sigaction *actp = os::Solaris::get_chained_signal_action(sig);
    if (actp != NULL) {
      chained = os::Solaris::chained_handler(actp, sig, info, ucVoid);
    }
    if (chained) {
      // signal-chaining in effect. Continue.
      return true;
    }
  }


// If os::Solaris::SIGasync not chained, and this is a non-vm and non-java thread
  if (sig == os::Solaris::SIGasync()) {
    return true;
  }

  if(sig == SIGPIPE) {
    if (PrintMiscellaneous && (WizardMode || Verbose)) {
      warning("Ignoring SIGPIPE - see bug 4229104");
    }
    return true;
  }

  // Workaround (bug 4900493) for Solaris kernel bug 4966651.
  // Handle an undefined selector caused by an attempt to assign
  // fs in libthread getipriptr(). With the current libthread design every 512
  // thread creations the LDT for a private thread data structure is extended
  // and thre is a hazard that and another thread attempting a thread creation
  // will use a stale LDTR that doesn't reflect the structure's growth,
  // causing a GP fault.
  // Enforce the probable limit of passes through here to guard against an
  // infinite loop if some other move to fs caused the GP fault. Note that
  // this loop counter is ultimately a heuristic as it is possible for
  // more than one thread to generate this fault at a time in an MP system.
  // In the case of the loop count being exceeded or if the poll fails 
  // just fall through to a fatal error. 
  // If there is some other source of T_GPFLT traps and the text at EIP is
  // unreadable this code will loop infinitely until the stack is exausted.
  // The key to diagnosis in this case is to look for the bottom signal handler
  // frame.

  if(! IgnoreLibthreadGPFault) {
    if (sig == SIGSEGV && uc->uc_mcontext.gregs[TRAPNO] == T_GPFLT) {
      const unsigned char *p = 
			(unsigned const char *) uc->uc_mcontext.gregs[EIP];

      // Expected instruction?

      if(p[0] == movlfs[0] && p[1] == movlfs[1]) {

	Atomic::inc(&ldtr_refresh);

	// Infinite loop?

	if(ldtr_refresh < ((2 << 16) / PAGESIZE)) {

	  // No, force scheduling to get a fresh view of the LDTR

	  if(poll(NULL, 0, 10) == 0) {

	    // Retry the move

	    return false;
	  }
	}      
      }
    }
  }

  if (!abort_if_unrecognized) {
    // caller wants another chance, so give it to him
    return false;
  }

  if (!os::Solaris::libjsig_is_loaded) {
    struct sigaction oldAct;
    sigaction(sig, (struct sigaction *)0, &oldAct);
    if (oldAct.sa_sigaction != signalHandler) {
      void* sighand = oldAct.sa_sigaction ? CAST_FROM_FN_PTR(void*,  oldAct.sa_sigaction)
					  : CAST_FROM_FN_PTR(void*, oldAct.sa_handler);
      warning("Unexpected Signal %d occured under user-defined signal handler %#lx", sig, (long)sighand);
    }
  }

  if (pc == NULL && uc != NULL) {
    pc = (address) uc->uc_mcontext.gregs[EIP];
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

void os::print_context(outputStream *st, void *context) {
  if (context == NULL) return;

  ucontext_t *uc = (ucontext_t*)context;
  st->print_cr("Registers:");
  st->print(  "EAX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EAX]);
  st->print(", EBX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EBX]);
  st->print(", ECX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[ECX]);
  st->print(", EDX=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EDX]);
  st->cr();
  st->print(  "ESP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[UESP]);
  st->print(", EBP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EBP]);
  st->print(", ESI=" INTPTR_FORMAT, uc->uc_mcontext.gregs[ESI]);
  st->print(", EDI=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EDI]);
  st->cr();
  st->print(  "EIP=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EIP]);
  st->print(", EFLAGS=" INTPTR_FORMAT, uc->uc_mcontext.gregs[EFL]);
  st->cr();
  st->cr();

  intptr_t *sp = (intptr_t *)os::Solaris::ucontext_get_sp(uc);
  st->print_cr("Top of Stack: (sp=" PTR_FORMAT ")", sp);
  print_hex_dump(st, (address)sp, (address)(sp + 32), sizeof(intptr_t));
  st->cr();

  // Note: it may be unsafe to inspect memory near pc. For example, pc may
  // point to garbage if entry point in an nmethod is corrupted. Leave
  // this at the end, and hope for the best.
  ExtendedPC epc = os::Solaris::ucontext_get_ExtendedPC(uc);
  address pc = epc.pc();
  st->print_cr("Instructions: (pc=" PTR_FORMAT ")", pc);
  print_hex_dump(st, pc - 16, pc + 16, sizeof(char));
}

// From solaris_i486.s
extern "C" void fixcw();

void os::Solaris::init_thread_fpu_state(void) {
  // Set fpu to 53 bit precision. This happens too early to use a stub.
  fixcw();
}

// JVMPI code
bool os::thread_is_running(JavaThread* tp) {
  int         flag;
  lwpid_t     lwpid;
  gregset_t   reg;
  lwpstatus_t lwpstatus;
  int         res;
  thread_t    tid = tp->osthread()->thread_id();
  res = threadgetstate(tid, &flag, &lwpid, NULL, reg, &lwpstatus);
  assert(res == 0, "threadgetstate() failure");
  if(res != 0) return false; // Safe return value

  unsigned int sum = 0;
  // give TRS_NONVOLATILE doesn't trust any other registers, just use these
  sum += reg[UESP];  sum += reg[EIP];   sum += reg[EBP];

  if (tp->last_sum() == sum) {
    return false;
  } else {
    tp->set_last_sum(sum);
    return true;
  }
}

// These routines are the initial value of atomic_xchg_entry(),
// atomic_cmpxchg_entry(), atomic_inc_entry() and fence_entry()
// until initialization is complete.
// TODO - replace with .il implementation when compiler supports it.

typedef jint  xchg_func_t        (jint,  volatile jint*);
typedef jint  cmpxchg_func_t     (jint,  volatile jint*,  jint);
typedef jlong cmpxchg_long_func_t(jlong, volatile jlong*, jlong);
typedef jint  add_func_t         (jint,  volatile jint*);
typedef void  fence_func_t       ();

jint os::atomic_xchg_bootstrap(jint exchange_value, volatile jint* dest) {
  // try to use the stub:
  xchg_func_t* func = CAST_TO_FN_PTR(xchg_func_t*, StubRoutines::atomic_xchg_entry());

  if (func != NULL) {
    os::atomic_xchg_func = func;
    return (*func)(exchange_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jint old_value = *dest;
  *dest = exchange_value;
  return old_value;
}

jint os::atomic_cmpxchg_bootstrap(jint exchange_value, volatile jint* dest, jint compare_value) {
  // try to use the stub:
  cmpxchg_func_t* func = CAST_TO_FN_PTR(cmpxchg_func_t*, StubRoutines::atomic_cmpxchg_entry());

  if (func != NULL) {
    os::atomic_cmpxchg_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jint old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

jlong os::atomic_cmpxchg_long_bootstrap(jlong exchange_value, volatile jlong* dest, jlong compare_value) {
  // try to use the stub:
  cmpxchg_long_func_t* func = CAST_TO_FN_PTR(cmpxchg_long_func_t*, StubRoutines::atomic_cmpxchg_long_entry());

  if (func != NULL) {
    os::atomic_cmpxchg_long_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jlong old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

jint os::atomic_add_bootstrap(jint add_value, volatile jint* dest) {
  // try to use the stub:
  add_func_t* func = CAST_TO_FN_PTR(add_func_t*, StubRoutines::atomic_add_entry());

  if (func != NULL) {
    os::atomic_add_func = func;
    return (*func)(add_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  return (*dest) += add_value;
}

void os::fence_bootstrap() {
  // try to use the stub:
  fence_func_t* func = CAST_TO_FN_PTR(fence_func_t*, StubRoutines::fence_entry());

  if (func != NULL) {
    os::fence_func = func;
    (*func)();
    return;
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  // don't have to do anything for a single thread
}

xchg_func_t*         os::atomic_xchg_func         = os::atomic_xchg_bootstrap;
cmpxchg_func_t*      os::atomic_cmpxchg_func      = os::atomic_cmpxchg_bootstrap;
cmpxchg_long_func_t* os::atomic_cmpxchg_long_func = os::atomic_cmpxchg_long_bootstrap;
add_func_t*          os::atomic_add_func          = os::atomic_add_bootstrap;
fence_func_t*        os::fence_func               = os::fence_bootstrap;
