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

// Defines the interfaces to Solaris operating systems that vary across platforms


// This is a simple callback that just fetches a PC for an interrupted thread.
// The thread need not be suspended and the fetched PC is just a hint.
// Returned PC and nPC are not necessarily consecutive.
// This one is currently used for profiling the VMThread ONLY!

// Must be synchronous
class GetThreadPC_Callback : public OSThread::Sync_Interrupt_Callback {
 private:
  ExtendedPC _addr;

 public:

  GetThreadPC_Callback(Monitor *sync) :
    OSThread::Sync_Interrupt_Callback(sync) { }
  ExtendedPC addr() const { return _addr; }

  void set_addr(ExtendedPC addr) { _addr = addr; }

  void execute(OSThread::InterruptArguments *args);
};

// misc
extern "C" {
  void signalHandler(int, siginfo_t*, void*);
}
void resolve_lwp_exit_calls(void);
void handle_unexpected_exception(Thread* thread, int sig, siginfo_t* info, address pc, address adjusted_pc);
#ifndef PRODUCT
void continue_with_dump(void);
#endif

#if defined(__sparc) && defined(COMPILER2)
// For Sun Studio compiler implementation is in  file
// src/os_cpu/solaris_sparc/vm/solaris_sparc.il
// For gcc implementation is in  file
// src/os_cpu/solaris_sparc/vm/os_solaris_sparc.cpp
extern "C" void _mark_fpu_nosave() ;
#endif

#define PROCFILE_LENGTH 128
