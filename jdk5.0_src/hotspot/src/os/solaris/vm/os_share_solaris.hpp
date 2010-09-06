#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_share_solaris.hpp	1.19 03/12/23 16:37:47 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Defines the interfaces to Solaris operating systems that vary across platforms

#ifndef CORE
// Synchronous: signal sender waits for the processing to see if update succeeded.
class SetThreadPC_Callback : public OSThread::Sync_Interrupt_Callback {
 private:
  ExtendedPC _old_addr;
  ExtendedPC _new_addr;
  volatile bool _result;

 public:

  SetThreadPC_Callback(ExtendedPC old_addr, ExtendedPC new_addr, Monitor *sync) : 
    OSThread::Sync_Interrupt_Callback(sync) {
    _result = false;
    _old_addr = old_addr;
    _new_addr = new_addr;
  }

  void execute(OSThread::InterruptArguments *args);
  bool result() { return _result; }
};
#endif

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

#define PROCFILE_LENGTH 128

