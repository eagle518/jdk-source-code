#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_linux.cpp	1.177 04/06/18 11:31:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// do not include  precompiled  header file
# include "incls/_os_linux.cpp.incl"

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
# include <pthread.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/utsname.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <pwd.h>
# include <poll.h>
# include <semaphore.h>
# include <fcntl.h>
# include <string.h>
# include <syscall.h>
# include <sys/sysinfo.h>
# include <gnu/libc-version.h>

#define MAX_PATH    (2 * K)

// for timer info max values which include all bits
#define ALL_64_BITS CONST64(0xFFFFFFFFFFFFFFFF)

////////////////////////////////////////////////////////////////////////////////
// global variables

int    os::Linux::_processor_count = 0;
julong os::Linux::_physical_memory = 0;

address   os::Linux::_initial_thread_stack_bottom = NULL;
uintptr_t os::Linux::_initial_thread_stack_size   = 0;

Mutex* os::Linux::_createThread_lock = NULL;
pthread_t os::Linux::_main_thread;
int os::Linux::_page_size = -1;
bool os::Linux::_is_floating_stack = false;
bool os::Linux::_is_NPTL = false;
char * os::Linux::_glibc_version = NULL;
char * os::Linux::_libpthread_version = NULL;

static jlong initial_time_count=0;

static int clock_tics_per_sec = 100;

static pid_t _initial_pid = 0;

/* Signal number used to suspend/resume a thread */

/* do not use any signal number less than SIGSEGV, see 4355769 */
static int SR_signum = SIGUSR2;
sigset_t SR_sigset;

////////////////////////////////////////////////////////////////////////////////
// utility functions

static int SR_initialize();
static int SR_finalize();

julong os::physical_memory() {
  return Linux::physical_memory();
}

////////////////////////////////////////////////////////////////////////////////
// environment support

bool os::getenv(const char* name, char* buf, int len) {
  const char* val = ::getenv(name);
  if (val != NULL && strlen(val) < len) {
    strcpy(buf, val);
    return true;
  }
  if (len > 0) buf[0] = 0;  // return a null string
  return false;
}


// Return true if user is running as root.

bool os::have_special_privileges() {
  static bool init = false;
  static bool privileges = false;
  if (!init) {
    privileges = (getuid() != geteuid()) || (getgid() != getegid());
    init = true;
  }
  return privileges;
}


#ifndef SYS_gettid
// i386: 224, IA64: 1105, amd64: 186
#ifdef __ia64__
#define SYS_gettid 1105     
#elif __i386__
#define SYS_gettid 224     
#elif __amd64__
#define SYS_gettid 186
#endif
#endif

// pid_t gettid()
//
// Returns the kernel thread id of the currently running thread. Kernel
// thread id is used to access /proc.
//
// (Note that getpid() on LinuxThreads returns kernel thread id too; but
// on NPTL, it returns the same pid for all threads, as required by POSIX.)
//
pid_t os::Linux::gettid() {
  int rslt = syscall(SYS_gettid);
  if (rslt == -1) {        
     // old kernel, no NPTL support
     return getpid();
  } else {
     return (pid_t)rslt;
  }
}

void os::Linux::initialize_system_info() {
  _processor_count = sysconf(_SC_NPROCESSORS_CONF);
  _physical_memory = (julong)sysconf(_SC_PHYS_PAGES) * (julong)sysconf(_SC_PAGESIZE);
  assert(_processor_count > 0, "linux error");
}

void os::init_system_properties_values() {
//  char arch[12];
//  sysinfo(SI_ARCHITECTURE, arch, sizeof(arch));

  // The next steps are taken in the product version:
  //
  // Obtain the JAVA_HOME value from the location of libjvm[_g].so.
  // This library should be located at:
  // <JAVA_HOME>/jre/lib/<arch>/{client|server}/libjvm[_g].so.
  //
  // If "/jre/lib/" appears at the right place in the path, then we 
  // assume libjvm[_g].so is installed in a JDK and we use this path. 
  //
  // Otherwise exit with message: "Could not create the Java virtual machine."
  //
  // The following extra steps are taken in the debugging version:
  //
  // If "/jre/lib/" does NOT appear at the right place in the path
  // instead of exit check for $JAVA_HOME environment variable.
  //
  // If it is defined and we are able to locate $JAVA_HOME/jre/lib/<arch>,
  // then we append a fake suffix "hotspot/libjvm[_g].so" to this path so
  // it looks like libjvm[_g].so is installed there
  // <JAVA_HOME>/jre/lib/<arch>/hotspot/libjvm[_g].so.
  //
  // Otherwise exit. 
  //
  // Important note: if the location of libjvm.so changes this 
  // code needs to be changed accordingly.

  // The next few definitions allow the code to be verbatim:
#define malloc(n) (char*)NEW_C_HEAP_ARRAY(char, (n))
#define getenv(n) ::getenv(n)

#define EXTENSIONS_DIR "/lib/ext"
#define ENDORSED_DIR "/lib/endorsed"

  {
    /* sysclasspath, java_home, dll_dir */
    {
        char *home_path;
	char *dll_path;
	char *pslash;
        char buf[MAXPATHLEN];
	os::jvm_path(buf, sizeof(buf));

	// Found the full path to libjvm.so. 
	// Now cut the path to <java_home>/jre if we can. 
	*(strrchr(buf, '/')) = '\0';  /* get rid of /libjvm.so */
	pslash = strrchr(buf, '/');
	if (pslash != NULL)
	    *pslash = '\0';           /* get rid of /{client|server|hotspot} */
	dll_path = malloc(strlen(buf) + 1);
	if (dll_path == NULL)
	    return;
	strcpy(dll_path, buf);
        Arguments::set_dll_dir(dll_path);

	if (pslash != NULL) {
	    pslash = strrchr(buf, '/');
	    if (pslash != NULL) {
		*pslash = '\0';       /* get rid of /<arch> */ 
		pslash = strrchr(buf, '/');
		if (pslash != NULL)
		    *pslash = '\0';   /* get rid of /lib */
	    }
	}

	home_path = malloc(strlen(buf) + 1);
	if (home_path == NULL)
	    return;
	strcpy(home_path, buf);
        Arguments::set_java_home(home_path);

	if (!set_boot_path('/', ':'))
	    return;
    }

    /* Where to look for native libraries */
    {
	/*
	 * Get the user setting of LD_LIBRARY_PATH
	 */
	char *v = getenv("LD_LIBRARY_PATH");

	if (v == NULL) {
            v = "";
	}

        Arguments::set_library_path(v);
    }

    /* Extensions directories */
    {
        char * buf;
        buf = malloc(strlen(Arguments::get_java_home()) + sizeof(EXTENSIONS_DIR));
        sprintf(buf, "%s" EXTENSIONS_DIR, Arguments::get_java_home());
        Arguments::set_ext_dirs(buf);
    }

    /* Endorsed standards default directory. */
    {
	char * buf;
	buf = malloc(strlen(Arguments::get_java_home()) + sizeof(ENDORSED_DIR));
	sprintf(buf, "%s" ENDORSED_DIR, Arguments::get_java_home());
        Arguments::set_endorsed_dirs(buf);
    }
  }

#undef malloc
#undef getenv
#undef EXTENSIONS_DIR
#undef ENDORSED_DIR

  // Done
  return;
}

////////////////////////////////////////////////////////////////////////////////
// breakpoint support

void os::breakpoint() {
  BREAKPOINT;
}

extern "C" void breakpoint() {
  // use debugger to set breakpoint here
}

////////////////////////////////////////////////////////////////////////////////
// signal support

// These are signals that are unblocked while a thread is running Java.
// (For some reason, they get blocked by default.)
sigset_t* os::Linux::unblocked_signals() {
  static sigset_t sigs;
  static volatile bool done = false;
  if (!done) {
    sigemptyset(&sigs);
    sigaddset  (&sigs, SIGINT);
    sigaddset  (&sigs, SIGCHLD);
    sigaddset  (&sigs, SIGTERM);
    /* these are normally unblocked but someone embedding a vm could block them */
    sigaddset(&sigs, SIGILL);
    sigaddset(&sigs, SIGSEGV);
    sigaddset(&sigs, SIGBUS);
    sigaddset(&sigs, SIGFPE);
    OrderAccess::release_store(&done, true); // it's OK to re-enter this
  }
  return &sigs;
}

void os::Linux::hotspot_sigmask(Thread* thread) {
  sigset_t sigset;

  sigemptyset(&sigset);
  if (!ReduceSignalUsage) {
    // Do not change the blocked/unblocked status of these signals if
    // -Xrs is specified. See signal_sets_init, os_solaris.cpp.
    if (!os::Linux::is_sig_ignored(SHUTDOWN1_SIGNAL)) {
       sigaddset  (&sigset, SHUTDOWN1_SIGNAL);
    }
    if (!os::Linux::is_sig_ignored(SHUTDOWN2_SIGNAL)) {
       sigaddset  (&sigset, SHUTDOWN2_SIGNAL);
    }
    if (!os::Linux::is_sig_ignored(SHUTDOWN3_SIGNAL)) {
       sigaddset  (&sigset, SHUTDOWN3_SIGNAL);
    }
    sigaddset  (&sigset, BREAK_SIGNAL);
  }
  // These are needed for correctness of the VM.
  sigaddset  (&sigset, SIGCHLD);
  sigaddset  (&sigset, SR_signum);

  //Save caller's signal mask before setting VM signal mask
  sigset_t caller_sigmask;
  pthread_sigmask(SIG_BLOCK, NULL, &caller_sigmask);
 
  OSThread* osthread = thread->osthread();
  osthread->set_caller_sigmask(caller_sigmask);

  pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);

  if (!ReduceSignalUsage) {
    sigemptyset(&sigset);
    sigaddset  (&sigset, BREAK_SIGNAL);
    if (thread->is_VM_thread()) {
      // Only the VM thread handles BREAK_SIGNAL ...
      pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);
    } else {
      // ... all other threads block BREAK_SIGNAL
      pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////
// detecting pthread library

void os::Linux::libpthread_init() {
  // Save glibc and pthread version strings. Note that _CS_GNU_LIBC_VERSION
  // and _CS_GNU_LIBPTHREAD_VERSION are supported in glibc >= 2.3.2. Use a 
  // generic name for earlier versions.
  // Define macros here so we can build HotSpot on old systems.
# ifndef _CS_GNU_LIBC_VERSION
# define _CS_GNU_LIBC_VERSION 2
# endif
# ifndef _CS_GNU_LIBPTHREAD_VERSION
# define _CS_GNU_LIBPTHREAD_VERSION 3
# endif

  size_t n = confstr(_CS_GNU_LIBC_VERSION, NULL, 0);
  if (n > 0) {
     char *str = (char *)malloc(n);
     confstr(_CS_GNU_LIBC_VERSION, str, n);
     os::Linux::set_glibc_version(str);
  } else {
     // _CS_GNU_LIBC_VERSION is not supported, try gnu_get_libc_version()
     static char _gnu_libc_version[32];
     jio_snprintf(_gnu_libc_version, sizeof(_gnu_libc_version), 
              "glibc %s %s", gnu_get_libc_version(), gnu_get_libc_release());
     os::Linux::set_glibc_version(_gnu_libc_version);
  }

  n = confstr(_CS_GNU_LIBPTHREAD_VERSION, NULL, 0);
  if (n > 0) {
     char *str = (char *)malloc(n);
     confstr(_CS_GNU_LIBPTHREAD_VERSION, str, n);

     // Vanilla RH-9 (glibc 2.3.2) has a bug that confstr() always tells
     // us "NPTL-0.29" even we are running with LinuxThreads. Check if this
     // is the case:
     if (strcmp(os::Linux::glibc_version(), "glibc 2.3.2") == 0 &&
         strstr(str, "NPTL")) {
        // LinuxThreads has a hard limit on max number of threads. So
        // sysconf(_SC_THREAD_THREADS_MAX) will return a positive value.
        // On the other hand, NPTL does not have such a limit, sysconf()
        // will return -1 and errno is not changed. Check if it is really
        // NPTL:
        if (sysconf(_SC_THREAD_THREADS_MAX) > 0) {
           free(str);
           str = "linuxthreads";
        }
     }
     os::Linux::set_libpthread_version(str);
  } else {
     // glibc before 2.3.2 only has LinuxThreads.
     os::Linux::set_libpthread_version("linuxthreads");
  }

  if (strstr(libpthread_version(), "NPTL")) {
     os::Linux::set_is_NPTL();
  } else {
     os::Linux::set_is_LinuxThreads();
  }

  // LinuxThreads have two flavors: floating-stack mode, which allows variable 
  // stack size; and fixed-stack mode. NPTL is always floating-stack.
  if (os::Linux::is_NPTL() || os::Linux::supports_variable_stack_size()) {
     os::Linux::set_is_floating_stack();
  }
}

/////////////////////////////////////////////////////////////////////////////
// thread stack

// Force Linux kernel to expand current thread stack. If "bottom" is close
// to the stack guard, caller should block all signals.
//
// MAP_GROWSDOWN:
//   A special mmap() flag that is used to implement thread stacks. It tells 
//   kernel that the memory region should extend downwards when needed. This 
//   allows early versions of LinuxThreads to only mmap the first few pages 
//   when creating a new thread. Linux kernel will automatically expand thread
//   stack as needed (on page faults). 
//
//   However, because the memory region of a MAP_GROWSDOWN stack can grow on
//   demand, if a page fault happens outside an already mapped MAP_GROWSDOWN
//   region, it's hard to tell if the fault is due to a legitimate stack 
//   access or because of reading/writing non-exist memory (e.g. buffer 
//   overrun). As a rule, if the fault happens below current stack pointer, 
//   Linux kernel does not expand stack, instead a SIGSEGV is sent to the 
//   application (see Linux kernel fault.c).
//
//   This Linux feature can cause SIGSEGV when VM bangs thread stack for
//   stack overflow detection.
//
//   Newer version of LinuxThreads (since glibc-2.2, or, RH-7.x) and NPTL do 
//   not use this flag. However, the stack of initial thread is not created
//   by pthread, it is still MAP_GROWSDOWN. Also it's possible (though 
//   unlikely) that user code can create a thread with MAP_GROWSDOWN stack
//   and then attach the thread to JVM.
//
// To get around the problem and allow stack banging on Linux, we need to 
// manually expand thread stack after receiving the SIGSEGV.
//
// There are two ways to expand thread stack to address "bottom", we used
// both of them in JVM before 1.5:
//   1. adjust stack pointer first so that it is below "bottom", and then
//      touch "bottom"
//   2. mmap() the page in question
//
// Now alternate signal stack is gone, it's harder to use 2. For instance,
// if current sp is already near the lower end of page 101, and we need to
// call mmap() to map page 100, it is possible that part of the mmap() frame
// will be placed in page 100. When page 100 is mapped, it is zero-filled.
// That will destroy the mmap() frame and cause VM to crash.
//
// The following code works by adjusting sp first, then accessing the "bottom"
// page to force a page fault. Linux kernel will then automatically expand the 
// stack mapping. 
//
// _expand_stack_to() assumes its frame size is less than page size, which 
// should always be true if the function is not inlined.

static void _expand_stack_to(address bottom) __attribute__ ((noinline));

static void _expand_stack_to(address bottom) {
  address sp;
  size_t size;
  volatile char *p;

  // Adjust bottom to point to the largest address within the same page, it
  // gives us a one-page buffer if alloca() allocates slightly more memory.
  bottom = (address)align_size_down((uintptr_t)bottom, os::Linux::page_size());
  bottom += os::Linux::page_size() - 1;

  // sp might be slightly above current stack pointer; if that's the case, we
  // will alloca() a little more space than necessary, which is OK. Don't use 
  // os::current_stack_pointer(), as its result can be slightly below current
  // stack pointer, causing us to not alloca enough to reach "bottom".
  sp = (address)&sp;

  if (sp > bottom) {
    size = sp - bottom;
    p = (volatile char *)alloca(size);
    assert(p != NULL && p <= (volatile char *)bottom, "alloca problem?");
    p[0] = '\0';
  }
}

bool os::Linux::manually_expand_stack(JavaThread * t, address addr) {
  assert(t!=NULL, "just checking");
  assert(t->osthread()->expanding_stack(), "expand should be set");
  assert(t->stack_base() != NULL, "stack_base was not initialized");

  if (addr <  t->stack_base() && addr >= t->stack_yellow_zone_base()) {
    sigset_t mask_all, old_sigset;
    sigfillset(&mask_all);
    pthread_sigmask(SIG_SETMASK, &mask_all, &old_sigset);
    _expand_stack_to(addr);
    pthread_sigmask(SIG_SETMASK, &old_sigset, NULL);
    return true;
  }
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// create new thread

static address highest_vm_reserved_address();

// check if it's safe to start a new thread
static bool _thread_safety_check(Thread* thread) {
  if (os::Linux::is_LinuxThreads() && !os::Linux::is_floating_stack()) {
    // Fixed stack LinuxThreads (SuSE Linux/x86, and some versions of Redhat)
    //   Heap is mmap'ed at lower end of memory space. Thread stacks are
    //   allocated (MAP_FIXED) from high address space. Every thread stack
    //   occupies a fixed size slot (usually 2Mbytes, but user can change
    //   it to other values if they rebuild LinuxThreads). 
    //
    // Problem with MAP_FIXED is that mmap() can still succeed even part of
    // the memory region has already been mmap'ed. That means if we have too 
    // many threads and/or very large heap, eventually thread stack will 
    // collide with heap.
    //
    // Here we try to prevent heap/stack collision by comparing current
    // stack bottom with the highest address that has been mmap'ed by JVM
    // plus a safety margin for memory maps created by native code.
    //
    // This feature can be disabled by setting ThreadSafetyMargin to 0
    //
    if (ThreadSafetyMargin > 0) {
      address stack_bottom = os::current_stack_base() - os::current_stack_size();

      // not safe if our stack extends below the safety margin
      return stack_bottom - ThreadSafetyMargin >= highest_vm_reserved_address();
    } else {
      return true;
    }
  } else {
    // Floating stack LinuxThreads or NPTL:
    //   Unlike fixed stack LinuxThreads, thread stacks are not MAP_FIXED. When
    //   there's not enough space left, pthread_create() will fail. If we come
    //   here, that means enough space has been reserved for stack.
    return true;
  }
}

// Thread start routine for all newly created threads
static void *_start(Thread *thread) {
  // Try to randomize the cache line index of hot stack frames.
  // This helps when threads of the same stack traces evict each other's
  // cache lines. The threads can be either from the same JVM instance, or
  // from different JVM instances. The benefit is especially true for
  // processors with hyperthreading technology.
  static int counter = 0;
  int pid = os::current_process_id();
  alloca(((pid ^ counter++) & 7) * 128);

  ThreadLocalStorage::set_thread(thread);

  OSThread* osthread = thread->osthread();
  Monitor* sync = osthread->startThread_lock();

  // non floating stack LinuxThreads needs extra check, see above
  if (!_thread_safety_check(thread)) {
    // notify parent thread
    MutexLockerEx ml(sync, Mutex::_no_safepoint_check_flag);
    osthread->set_state(ZOMBIE);
    sync->notify_all();
    return NULL;
  }

  // thread_id is kernel thread id (similar to Solaris LWP id)
  osthread->set_thread_id(os::Linux::gettid());

  // initialize signal mask for this thread
  os::Linux::hotspot_sigmask(thread);

  // initialize floating point control register
  os::Linux::init_thread_fpu_state();

  // handshaking with parent thread
  {
    MutexLockerEx ml(sync, Mutex::_no_safepoint_check_flag);

    // notify parent thread
    osthread->set_state(INITIALIZED);
    sync->notify_all();

    // wait until os::start_thread()
    while (osthread->get_state() == INITIALIZED) {
      sync->wait(Mutex::_no_safepoint_check_flag);
    }
  }

  // call one more level start routine
  thread->run();

  return 0;
}

bool os::create_thread(Thread* thread, ThreadType thr_type, size_t stack_size) {
  assert(thread->osthread() == NULL, "caller responsible");

  // Allocate the OSThread object
  OSThread* osthread = new OSThread(NULL, NULL);
  if (osthread == NULL) {
    return false;
  }

  // set the correct thread state
  osthread->set_thread_type(thr_type);

  // Initial state is ALLOCATED but not INITIALIZED
  osthread->set_state(ALLOCATED);

  thread->set_osthread(osthread);

  // init thread attributes
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  // stack size
  if (os::Linux::supports_variable_stack_size()) {
    // calculate stack size if it's not specified by caller
    if (stack_size == 0) {
      stack_size = os::Linux::default_stack_size(thr_type);

      switch (thr_type) {
      case os::java_thread:
        // Java threads use ThreadStackSize which default value can be changed with the flag -Xss
        if (JavaThread::stack_size_at_create() > 0) stack_size = JavaThread::stack_size_at_create();
        break;
      case os::compiler_thread:
        if (CompilerThreadStackSize > 0) {
          stack_size = (size_t)(CompilerThreadStackSize * K);
          break;
        } // else fall through:
          // use VMThreadStackSize if CompilerThreadStackSize is not defined
      case os::vm_thread: 
      case os::gc_thread:
      case os::pgc_thread: 
      case os::cms_thread: 
      case os::watcher_thread: 
        if (VMThreadStackSize > 0) stack_size = (size_t)(VMThreadStackSize * K);
        break;
      }
    }

    stack_size = MAX2(stack_size, os::Linux::min_stack_allowed);
    pthread_attr_setstacksize(&attr, stack_size);
  } else {
    // let pthread_create() pick the default value.
  }

  // glibc guard page
  pthread_attr_setguardsize(&attr, os::Linux::default_guard_size(thr_type));

  ThreadState state;

  {
    // Serialize thread creation if we are running with fixed stack LinuxThreads
    bool lock = os::Linux::is_LinuxThreads() && !os::Linux::is_floating_stack();
    if (lock) {
      os::Linux::createThread_lock()->lock_without_safepoint_check();
    }

    pthread_t tid;
    int ret = pthread_create(&tid, &attr, (void* (*)(void*)) _start, thread);

    pthread_attr_destroy(&attr);

    if (ret != 0) {
      if (PrintMiscellaneous && (Verbose || WizardMode)) {
        perror("pthread_create()");
      }
      // Need to clean up stuff we've allocated so far
      thread->set_osthread(NULL);
      delete osthread;
      if (lock) os::Linux::createThread_lock()->unlock();
      return false;
    }

    // Store pthread info into the OSThread
    osthread->set_pthread_id(tid);

    // Wait until child thread is either initialized or aborted
    {
      Monitor* sync_with_child = osthread->startThread_lock();
      MutexLockerEx ml(sync_with_child, Mutex::_no_safepoint_check_flag);
      while ((state = osthread->get_state()) == ALLOCATED) {
        sync_with_child->wait(Mutex::_no_safepoint_check_flag);
      }
    }

    if (lock) {
      os::Linux::createThread_lock()->unlock();
    }
  }

  // Aborted due to thread limit being reached
  if (state == ZOMBIE) {
      thread->set_osthread(NULL);
      delete osthread;
      return false;
  }

  // The thread is returned suspended (in state INITIALIZED),
  // and is started higher up in the call chain
  assert(state == INITIALIZED, "race condition");
  return true;
}

/////////////////////////////////////////////////////////////////////////////
// attach existing thread

// bootstrap the main thread
bool os::create_main_thread(Thread* thread) {
  assert(os::Linux::_main_thread == pthread_self(), "should be called inside main thread");
  return create_attached_thread(thread);
}

bool os::create_attached_thread(Thread* thread) {
  // TODO: we should really change the interface to "JavaThread"
  assert(thread->is_Java_thread(), "sanity check");

  // Allocate the OSThread object
  OSThread* osthread = new OSThread(NULL, NULL);

  if (osthread == NULL) {
    return false;
  }

  // Store pthread info into the OSThread
  osthread->set_thread_id(os::Linux::gettid());
  osthread->set_pthread_id(::pthread_self());

  // initialize floating point control register
  os::Linux::init_thread_fpu_state();

  // Initial thread state is RUNNABLE
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    thread->clear_is_baby_thread();
    osthread->set_state(RUNNABLE);
  }

  thread->set_osthread(osthread);
  
  if (os::Linux::is_initial_thread()) {
    // If current thread is initial thread, its stack is mapped on demand,
    // see notes about MAP_GROWSDOWN. Here we try to force kernel to map
    // the entire stack region to avoid SEGV in stack banging.
    // It is also useful to get around the heap-stack-gap problem on SuSE
    // kernel (see 4821821 for details). We first expand stack to the top
    // of yellow zone, then enable stack yellow zone (order is significant,
    // enabling yellow zone first will crash JVM on SuSE Linux), so there 
    // is no gap between the last two virtual memory regions.

    JavaThread *jt = (JavaThread *)thread;
    address addr = jt->stack_yellow_zone_base();
    assert(addr != NULL, "initialization problem?");
    assert(jt->stack_available(addr) > 0, "stack guard should not be enabled");

    osthread->set_expanding_stack();
    os::Linux::manually_expand_stack(jt, addr);
    osthread->clear_expanding_stack();
  }

  // initialize signal mask for this thread
  // and save the caller's signal mask
  os::Linux::hotspot_sigmask(thread);

  return true;
}

void os::pd_start_thread(Thread* thread) {
  OSThread * osthread = thread->osthread();
  assert(osthread->get_state() != INITIALIZED, "just checking");
  Monitor* sync_with_child = osthread->startThread_lock();
  MutexLockerEx ml(sync_with_child, Mutex::_no_safepoint_check_flag);
  sync_with_child->notify();
}

// Free Linux resources related to the OSThread
void os::free_thread(OSThread* osthread) {
  assert(osthread != NULL, "osthread not set");
 
  if (Thread::current()->osthread() == osthread) {
    // Restore caller's signal mask
    sigset_t sigmask = osthread->caller_sigmask();
    pthread_sigmask(SIG_SETMASK, &sigmask, NULL);
   }
 
  delete osthread;
}

//////////////////////////////////////////////////////////////////////////////
// thread local storage

int os::allocate_thread_local_storage() {
  pthread_key_t key;
  int rslt = pthread_key_create(&key, NULL);
  assert(rslt == 0, "cannot allocate thread local storage");
  return (int)key;
}

// Note: This is currently not used by VM, as we don't destroy TLS key
// on VM exit.
void os::free_thread_local_storage(int index) {
  int rslt = pthread_key_delete((pthread_key_t)index);
  assert(rslt == 0, "invalid index");
}

void os::thread_local_storage_at_put(int index, void* value) {
  int rslt = pthread_setspecific((pthread_key_t)index, value);
  assert(rslt == 0, "pthread_setspecific failed");
}

extern "C" Thread* get_thread() {
  return ThreadLocalStorage::thread();
}

//////////////////////////////////////////////////////////////////////////////
// initial thread

// Check if current thread is the initial thread, similar to Solaris thr_main.
bool os::Linux::is_initial_thread(void) {
  char dummy;
  assert(initial_thread_stack_bottom() != NULL &&
         initial_thread_stack_size()   != 0,
         "os::init did not locate initial thread's stack region");
  if ((address)&dummy >= initial_thread_stack_bottom() &&
      (address)&dummy < initial_thread_stack_bottom() + initial_thread_stack_size())
       return true;
  else return false;
}

// Find the virtual memory area that contains addr
static bool find_vma(address addr, address* vma_low, address* vma_high) {
  FILE *fp = fopen("/proc/self/maps", "r");
  if (fp) {
    address low, high;
    while (!feof(fp)) {
      if (fscanf(fp, "%p-%p", &low, &high) == 2) {
        if (low <= addr && addr < high) {
           if (vma_low)  *vma_low  = low;
           if (vma_high) *vma_high = high;
           fclose (fp);
           return true;
        }
      }
      for (;;) {
        int ch = fgetc(fp);
        if (ch == EOF || ch == (int)'\n') break;
      }
    }
    fclose(fp);
  }
  return false;
}

// Locate initial thread stack. This special handling of initial thread stack
// is needed because pthread_getattr_np() on most (all?) Linux distros returns 
// bogus value for initial thread.
void os::Linux::capture_initial_stack(size_t max_size) {
  // stack size is the easy part, get it from RLIMIT_STACK
  size_t stack_size;
  struct rlimit rlim;
  getrlimit(RLIMIT_STACK, &rlim);
  stack_size = rlim.rlim_cur;

  // 4441425: avoid crash with "unlimited" stack size on SuSE 7.1 or Redhat
  //   7.1, in both cases we will get 2G in return value.
  // 4466587: glibc 2.2.x compiled w/o "--enable-kernel=2.4.0" (RH 7.0,
  //   SuSE 7.2, Debian) can not handle alternate signal stack correctly
  //   for initial thread if its stack size exceeds 6M. Cap it at 2M,
  //   in case other parts in glibc still assumes 2M max stack size.
  // FIXME: alt signal stack is gone, maybe we can relax this constraint?
#ifndef IA64
  if (stack_size > 2 * K * K) stack_size = 2 * K * K;
#else
  // Problem still exists RH7.2 (IA64 anyway) but 2MB is a little small
  if (stack_size > 4 * K * K) stack_size = 4 * K * K;
#endif

  // Try to figure out where the stack base (top) is. This is harder.
  //
  // When an application is started, glibc saves the initial stack pointer in
  // a global variable "__libc_stack_end", which is then used by system 
  // libraries. __libc_stack_end should be pretty close to stack top. The
  // variable is available since the very early days. However, because it is
  // a private interface, it could disappear in the future.
  //
  // Linux kernel saves start_stack information in /proc/<pid>/stat. Similar
  // to __libc_stack_end, it is very close to stack top, but isn't the real
  // stack top. Note that /proc may not exist if VM is running as a chroot 
  // program, so reading /proc/<pid>/stat could fail. Also the contents of
  // /proc/<pid>/stat could change in the future (though unlikely).
  //
  // We try __libc_stack_end first. If that doesn't work, look for
  // /proc/<pid>/stat. If neither of them works, we use current stack pointer
  // as a hint, which should work well in most cases.
  
  uintptr_t stack_start;

  // try __libc_stack_end first
  uintptr_t *p = (uintptr_t *)dlsym(RTLD_DEFAULT, "__libc_stack_end");
  if (p && *p) {
    stack_start = *p;
  } else {
    // see if we can get the start_stack field from /proc/self/stat
    FILE *fp;
    int pid;
    char state;
    int ppid;
    int pgrp;
    int session;
    int nr;
    int tpgrp;
    unsigned long flags;
    unsigned long minflt;
    unsigned long cminflt;
    unsigned long majflt;
    unsigned long cmajflt;
    unsigned long utime;
    unsigned long stime;
    long cutime;
    long cstime;
    long prio;
    long nice;
    long junk;
    long it_real;
    uintptr_t start;
    uintptr_t vsize;
    uintptr_t rss;
    unsigned long rsslim;
    uintptr_t scodes;
    uintptr_t ecode;
    int i;

    // Figure what the primordial thread stack base is. Code is inspired
    // by email from Hans Boehm. /proc/self/stat begins with current pid,
    // followed by command name surrounded by parentheses, state, etc.
    char stat[2048];
    int statlen;

    fp = fopen("/proc/self/stat", "r");
    if (fp) {
      statlen = fread(stat, 1, 2047, fp);
      stat[statlen] = '\0';
      fclose(fp);

      // Skip pid and the command string. Note that we could be dealing with
      // weird command names, e.g. user could decide to rename java launcher
      // to "java 1.4.2 :)", then the stat file would look like
      //                1234 (java 1.4.2 :)) R ... ... 
      // We don't really need to know the command string, just find the last 
      // occurrence of ")" and then start parsing from there. See bug 4726580.
      char * s = strrchr(stat, ')');

      i = 0;
      if (s) {
        // Skip blank chars
        do s++; while (isspace(*s));

        /*                                     1   1   1   1   1   1   1   1   1   1   2   2   2   2   2   2   2   2   2 */
        /*              3  4  5  6  7  8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8 */
        i = sscanf(s, "%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu", 
	     &state,          /* 3  %c  */
	     &ppid,           /* 4  %d  */
	     &pgrp,           /* 5  %d  */
	     &session,        /* 6  %d  */
	     &nr,             /* 7  %d  */
	     &tpgrp,          /* 8  %d  */
	     &flags,          /* 9  %lu  */
	     &minflt,         /* 10 %lu  */
	     &cminflt,        /* 11 %lu  */
	     &majflt,         /* 12 %lu  */
	     &cmajflt,        /* 13 %lu  */
	     &utime,          /* 14 %lu  */
	     &stime,          /* 15 %lu  */
	     &cutime,         /* 16 %ld  */
	     &cstime,         /* 17 %ld  */
	     &prio,           /* 18 %ld  */
	     &nice,           /* 19 %ld  */
	     &junk,           /* 20 %ld  */
	     &it_real,        /* 21 %ld  */
	     &start,          /* 22 %lu  */
	     &vsize,          /* 23 %lu  */
	     &rss,            /* 24 %ld  */
	     &rsslim,         /* 25 %lu  */
	     &scodes,         /* 26 %lu  */
	     &ecode,          /* 27 %lu  */
	     &stack_start);   /* 28 %lu  */
      }

      if (i != 28 - 2) {
         assert(false, "Bad conversion from /proc/self/stat");
         // product mode - assume we are the initial thread, good luck in the
         // embedded case.
         warning("Can't detect initial thread stack location - bad conversion");
         stack_start = (uintptr_t) &rlim;
      }
    } else {
      // For some reason we can't open /proc/self/stat (for example, running on
      // FreeBSD with a Linux emulator, or inside chroot), this should work for 
      // most cases, so don't abort:
      warning("Can't detect initial thread stack location - no /proc/self/stat");
      stack_start = (uintptr_t) &rlim;
    }
  }

  // Now we have a pointer (stack_start) very close to the stack top, the 
  // next thing to do is to figure out the exact location of stack top. We
  // can find out the virtual memory area that contains stack_start by 
  // reading /proc/self/maps, it should be the last vma in /proc/self/maps,
  // and its upper limit is the real stack top. (again, this would fail if 
  // running inside chroot, because /proc may not exist.)
  
  uintptr_t stack_top;
  address low, high;
  if (find_vma((address)stack_start, &low, &high)) {
    // success, "high" is the true stack top. (ignore "low", because initial
    // thread stack grows on demand, its real bottom is high - RLIMIT_STACK.)
    stack_top = (uintptr_t)high;
  } else {
    // failed, likely because /proc/self/maps does not exist
    warning("Can't detect initial thread stack location - find_vma failed");
    // best effort: stack_start is normally within a few pages below the real 
    // stack top, use it as stack top, and reduce stack size so we won't put
    // guard page outside stack.
    stack_top = stack_start;
    stack_size -= 16 * page_size();
  }

  // stack_top could be partially down the page so align it
  stack_top = align_size_up(stack_top, page_size());

  if (max_size && stack_size > max_size) {
     _initial_thread_stack_size = max_size;
  } else {
     _initial_thread_stack_size = stack_size;
  }

  _initial_thread_stack_size = align_size_down(_initial_thread_stack_size, page_size());
  _initial_thread_stack_bottom = (address)stack_top - _initial_thread_stack_size;
}


// ISM not avaliable for linux
char* os::reserve_memory_special(size_t bytes) {
  ShouldNotReachHere();
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// time support

// Time since start-up in seconds to a fine granularity.
// Used by VMSelfDestructTimer and the MemProfiler.
double os::elapsedTime() {

  return (double)(os::elapsed_counter()) * 0.000001;
}

jlong os::elapsed_counter() {
  timeval time;
  int status = gettimeofday(&time, NULL);
  return jlong(time.tv_sec) * 1000 * 1000 + jlong(time.tv_usec) - initial_time_count;
}

jlong os::elapsed_frequency() {
  return (1000 * 1000);
}

jlong os::javaTimeMillis() {
  timeval time;
  int status = gettimeofday(&time, NULL);
  assert(status != -1, "linux error");
  return jlong(time.tv_sec) * 1000  +  jlong(time.tv_usec / 1000);
}

#define JAVA_TIME_NANOS_USES_CLOCK_GETTIME 0

jlong os::javaTimeNanos() {
#if JAVA_TIME_NANOS_USES_CLOCK_GETTIME
  // Recent linux libraries support clock_gettime, which is preferable
  // here, but only if CLOCK_MONOTONIC is supported, which it is not in
  // most distributions as of this writing.  When it is, the following
  // code that uses can be used. For now, it just uses gettimeofday.
  // Also, some sort of auto-detection should be used to trigger it. - dl
  struct timespec tp;
  int status = clock_gettime(CLOCK_MONOTONIC, &tp);
  assert(status == 0, "gettime error");
  jlong result = jlong(tp.tv_sec) * (1000 * 1000 * 1000) + jlong(tp.tv_nsec);
  return result;
#else
  timeval time;
  int status = gettimeofday(&time, NULL);
  assert(status != -1, "linux error");
  jlong usecs = jlong(time.tv_sec) * (1000 * 1000) + jlong(time.tv_usec);
  return 1000 * usecs;
#endif
}

void os::javaTimeNanos_info(jvmtiTimerInfo *info_ptr) {
#if JAVA_TIME_NANOS_USES_CLOCK_GETTIME
  info_ptr->max_value = ALL_64_BITS; 

  // CLOCK_MONOTONIC - amount of time since some arbitrary point in the past
  info_ptr->may_skip_backward = false;      // not subject to resetting or drifting
  info_ptr->may_skip_forward = false;       // not subject to resetting or drifting
#else
  // gettimeofday - based on time in seconds since the Epoch thus does not wrap
  info_ptr->max_value = ALL_64_BITS;  

  // gettimeofday is a real time clock so it skips
  info_ptr->may_skip_backward = true;  
  info_ptr->may_skip_forward = true; 
#endif

  info_ptr->kind = JVMTI_TIMER_ELAPSED;                // elapsed not CPU time
}



////////////////////////////////////////////////////////////////////////////////
// runtime exit support

// Note: os::shutdown() might be called very early during initialization, or
// called from signal handler. Before adding something to os::shutdown(), make
// sure it is async-safe and can handle partially initialized VM.
void os::shutdown() {

  // allow PerfMemory to attempt cleanup of any persistent resources
  perfMemory_exit();

  // flush buffered output, finish log files
  ostream_abort();

  // Check for abort hook
  abort_hook_t abort_hook = Arguments::abort_hook();
  if (abort_hook != NULL) {
    abort_hook();
  }

}

// Note: os::abort() might be called very early during initialization, or
// called from signal handler. Before adding something to os::abort(), make
// sure it is async-safe and can handle partially initialized VM.
void os::abort(bool dump_core) {
  os::shutdown(); 
  if (dump_core) {
#ifndef PRODUCT
    fdStream out(defaultStream::output_fd());
    out.print_raw("Current thread is ");
    char buf[16];
    jio_snprintf(buf, sizeof(buf), UINTX_FORMAT, os::current_thread_id());
    out.print_raw_cr(buf);
    out.print_raw_cr("Dumping core ...");
#endif
    ::abort(); // dump core
  }

  ::exit(1);
}

// Die immediately, no exit hook, no abort hook, no cleanup.
void os::die() {
  // _exit() on LinuxThreads only kills current thread 
  ::abort();
}

intx os::current_thread_id() { return (intx)pthread_self(); }
int os::current_process_id() {

  // Under the old linux thread library, linux gives each thread 
  // its own process id. Because of this each thread will return 
  // a differend pid if this method were to return the result 
  // of getpid(2). Linux provides no api that returns the pid 
  // of the primordial thread for the vm. This implementation 
  // returns a unique pid, the pid for the thread that invokes 
  // os::init(), for the vm 'process'.

  // Under the NPTL, getpid() returns the same pid as the 
  // primordial thread rather than a unique pid per thread. 
  // Use gettid() if you want the old pre NPTL behaviour.

  // if you are looking for the result of a call to getpid() that
  // returns a unique pid for the calling thread, then look at the
  // OSThread::thread_id() method in osThread_linux.hpp file

  return (int)(_initial_pid ? _initial_pid : getpid());
}

// DLL functions

const char* os::dll_file_extension() { return ".so"; }

const char* os::get_temp_directory() { return "/tmp/"; }

// check if addr is inside libjvm[_g].so
bool os::address_is_in_vm(address addr) {
  static address libjvm_base_addr;
  Dl_info dlinfo;

  if (libjvm_base_addr == NULL) {
    dladdr(CAST_FROM_FN_PTR(void *, os::address_is_in_vm), &dlinfo);
    libjvm_base_addr = (address)dlinfo.dli_fbase;
    assert(libjvm_base_addr !=NULL, "Cannot obtain base address for libjvm");
  }

  if (dladdr((void *)addr, &dlinfo)) {
    if (libjvm_base_addr == (address)dlinfo.dli_fbase) return true;
  }

  return false;
}

bool os::dll_address_to_function_name(address addr, char *buf,
                                      int buflen, int *offset) {
  Dl_info dlinfo;

  if (dladdr((void*)addr, &dlinfo) && dlinfo.dli_sname != NULL) {
    if (buf) jio_snprintf(buf, buflen, "%s", dlinfo.dli_sname);
    if (offset) *offset = addr - (address)dlinfo.dli_saddr;
    return true;
  } else {
    if (buf) buf[0] = '\0';
    if (offset) *offset = -1;
    return false;
  }
}

bool os::dll_address_to_library_name(address addr, char* buf,
                                     int buflen, int* offset) {
  Dl_info dlinfo;

  if (dladdr((void*)addr, &dlinfo)){
     if (buf) jio_snprintf(buf, buflen, "%s", dlinfo.dli_fname);
     if (offset) *offset = addr - (address)dlinfo.dli_fbase;
     return true;
  } else {
     if (buf) buf[0] = '\0';
     if (offset) *offset = -1;
     return false;
  }
}

bool _print_ascii_file(const char* filename, outputStream* st) {
  int fd = open(filename, O_RDONLY);
  if (fd == -1) {
     return false;
  }

  char buf[32];
  int bytes;
  while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
    st->print_raw(buf, bytes);
  }

  close(fd);

  return true;
}

void os::print_dll_info(outputStream *st) {
   st->print_cr("Dynamic libraries:");

   char fname[32];
   pid_t pid = os::Linux::gettid();

   jio_snprintf(fname, sizeof(fname), "/proc/%d/maps", pid);

   if (!_print_ascii_file(fname, st)) {
     st->print("Can not get library information for pid = %d\n", pid);
   }
}

void os::print_os_info(outputStream* st) {
  st->print("OS:");

  // Try to identify popular distros.
  // Most Linux distributions have /etc/XXX-release file, which contains
  // the OS version string. Some have more than one /etc/XXX-release file 
  // (e.g. Mandrake has both /etc/mandrake-release and /etc/redhat-release.),
  // so the order is important.
  if (!_print_ascii_file("/etc/mandrake-release", st) &&
      !_print_ascii_file("/etc/sun-release", st) &&
      !_print_ascii_file("/etc/redhat-release", st) &&
      !_print_ascii_file("/etc/SuSE-release", st) &&
      !_print_ascii_file("/etc/turbolinux-release", st) &&
      !_print_ascii_file("/etc/gentoo-release", st) &&
      !_print_ascii_file("/etc/debian_version", st)) {
      st->print("Linux");
  }
  st->cr();

  // kernel
  st->print("uname:");
  struct utsname name;
  uname(&name);
  st->print(name.sysname); st->print(" ");
  st->print(name.release); st->print(" ");
  st->print(name.version); st->print(" ");
  st->print(name.machine);
  st->cr();

  // libc, pthread
  st->print("libc:");
  st->print(os::Linux::glibc_version()); st->print(" ");
  st->print(os::Linux::libpthread_version()); st->print(" ");
  if (os::Linux::is_LinuxThreads()) {
     st->print("(%s stack)", os::Linux::is_floating_stack() ? "floating" : "fixed");
  }
  st->cr();

  // rlimit
  st->print("rlimit:");
  struct rlimit rlim;

  st->print(" STACK ");
  getrlimit(RLIMIT_STACK, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%uk", rlim.rlim_cur >> 10);

  st->print(", CORE ");
  getrlimit(RLIMIT_CORE, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%uk", rlim.rlim_cur >> 10);

  st->print(", NPROC ");
  getrlimit(RLIMIT_NPROC, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%d", rlim.rlim_cur);

  st->print(", NOFILE ");
  getrlimit(RLIMIT_NOFILE, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%d", rlim.rlim_cur);

  st->print(", AS ");
  getrlimit(RLIMIT_AS, &rlim);
  if (rlim.rlim_cur == RLIM_INFINITY) st->print("infinity");
  else st->print("%uk", rlim.rlim_cur >> 10);
  st->cr();

  // load average
  st->print("load average:");
  double loadavg[3];
  getloadavg(loadavg, 3);
  st->print("%0.02f %0.02f %0.02f", loadavg[0], loadavg[1], loadavg[2]);
  st->cr();
}

void os::print_memory_info(outputStream* st) {
  st->print("Memory:");
  st->print(" %dk page", os::vm_page_size()>>10);

  // values in struct sysinfo are "unsigned long"
  struct sysinfo si;
  sysinfo(&si);
  st->print(", physical " UINTX_FORMAT "k", si.totalram >> 10);
  st->print("(" UINTX_FORMAT "k free)", si.freeram >> 10);
  st->print(", swap " UINTX_FORMAT "k", si.totalswap >> 10);
  st->print("(" UINTX_FORMAT "k free)", si.freeswap >> 10);
  st->cr();
}

void os::print_siginfo(outputStream* st, void* siginfo) {
  st->print("siginfo:");

  siginfo_t *si = (siginfo_t*)siginfo;
  st->print("si_signo=%d", si->si_signo);
  st->print(", si_errno=%d", si->si_errno);
  st->print(", si_code=%d", si->si_code);
  switch (si->si_signo) {
  case SIGILL:
  case SIGFPE:
  case SIGSEGV:
  case SIGBUS:
    st->print(", si_addr=" PTR_FORMAT, si->si_addr);
    break;
  }
  st->cr();
}

static char saved_jvm_path[MAXPATHLEN] = {0};

// Find the full path to the current module, libjvm.so or libjvm_g.so
void os::jvm_path(char *buf, jint len) {
  // Error checking.
  if (len < MAXPATHLEN) {
    assert(false, "must use a large-enough buffer");
    buf[0] = '\0';
    return;
  }
  // Lazy resolve the path to current module.
  if (saved_jvm_path[0] != 0) {
    strcpy(buf, saved_jvm_path);
    return;
  }

  Dl_info dlinfo;
  int ret = dladdr(CAST_FROM_FN_PTR(void *, os::jvm_path), &dlinfo);
  assert(ret != 0, "cannot locate libjvm");
  realpath((char *)dlinfo.dli_fname, buf);

#ifndef PRODUCT
  // Support for the gamma launcher.  Typical value for buf is
  // "<JAVA_HOME>/jre/lib/<arch>/<vmtype>/libjvm.so".  If "/jre/lib/" appears at
  // the right place in the string, then assume we are installed in a JDK and
  // we're done.  Otherwise, check for a JAVA_HOME environment variable and fix
  // up the path so it looks like libjvm.so is installed there (append a
  // fake suffix hotspot/libjvm.so).
  const char *p = buf + strlen(buf) - 1;
  for (int count = 0; p > buf && count < 5; ++count) {
    for (--p; p > buf && *p != '/'; --p)
      /* empty */ ;
  }

  if (strncmp(p, "/jre/lib/", 9) != 0) {
    // Look for JAVA_HOME in the environment.
    char* java_home_var = ::getenv("JAVA_HOME");
    if (java_home_var != NULL && java_home_var[0] != 0) {
#if   defined(IA64)
      char cpu_arch[] = "ia64";
#elif defined(IA32)
      char cpu_arch[] = "i386";
#elif defined(AMD64)
      char cpu_arch[] = "amd64";
#else
#  error Add appropriate cpu_arch setting
#endif
      // Check the current module name "libjvm.so" or "libjvm_g.so".
      p = strrchr(buf, '/');
      assert(strstr(p, "/libjvm") == p, "invalid library name");
      p = strstr(p, "_g") ? "_g" : "";

      realpath(java_home_var, buf);
      sprintf(buf + strlen(buf), "/jre/lib/%s", cpu_arch);
      if (0 == access(buf, F_OK)) {
	// Use current module name "libjvm[_g].so" instead of 
	// "libjvm"debug_only("_g")".so" since for fastdebug version
	// we should have "libjvm.so" but debug_only("_g") adds "_g"!
	// It is used when we are choosing the HPI library's name 
	// "libhpi[_g].so" in hpi::initialize_get_interface().
	sprintf(buf + strlen(buf), "/hotspot/libjvm%s.so", p);
      } else {
        // Go back to path of .so
        realpath((char *)dlinfo.dli_fname, buf);
      }
    }
  } 
#endif // #ifndef PRODUCT

  strcpy(saved_jvm_path, buf);
}

void os::print_jni_name_prefix_on(outputStream* st, int args_size) {
  // no prefix required, not even "_"
}

void os::print_jni_name_suffix_on(outputStream* st, int args_size) {
  // no suffix required
}

////////////////////////////////////////////////////////////////////////////////
// sun.misc.Signal support

static volatile jint sigint_count = 0;

static void
UserHandler(int sig, void *siginfo, void *context) {
  // 4511530 - sem_post is serialized and handled by the manager thread. When
  // the program is interrupted by Ctrl-C, SIGINT is sent to every thread. We
  // don't want to flood the manager thread with sem_post requests.
  if (sig == SIGINT && Atomic::add(1, &sigint_count) > 1) 
      return;

  // Ctrl-C is pressed during error reporting, likely because the error 
  // handler fails to abort. Let VM die immediately.
  if (sig == SIGINT && is_error_reported()) {
     os::die();
  }

  os::signal_notify(sig);
}

void* os::user_handler() {
  return CAST_FROM_FN_PTR(void*, UserHandler);
}

extern "C" {
  typedef void (*sa_handler_t)(int);
  typedef void (*sa_sigaction_t)(int, siginfo_t *, void *);
}

void* os::signal(int signal_number, void* handler) {
  struct sigaction sigAct, oldSigAct;

  sigfillset(&(sigAct.sa_mask));
  sigAct.sa_flags   = SA_RESTART|SA_SIGINFO;
  sigAct.sa_handler = CAST_TO_FN_PTR(sa_handler_t, handler);

  if (sigaction(signal_number, &sigAct, &oldSigAct)) {
    // -1 means registration failed
    return (void *)-1;
  }

  return CAST_FROM_FN_PTR(void*, oldSigAct.sa_handler);
}

void os::signal_raise(int signal_number) {
  ::raise(signal_number);
}

/*
 * The following code is moved from os.cpp for making this
 * code platform specific, which it is by its very nature.
 */

// Will be modified when max signal is changed to be dynamic
int os::sigexitnum_pd() {
  return NSIG;
}

// a counter for each possible signal value
static volatile jint pending_signals[NSIG+1] = { 0 };

// Linux(POSIX) specific hand shaking semaphore.
static sem_t sig_sem;

void os::signal_init_pd() {
  // Initialize signal structures
  ::memset((void*)pending_signals, 0, sizeof(pending_signals));

  // Initialize signal semaphore
  ::sem_init(&sig_sem, 0, 0);
}

void os::signal_notify(int sig) {
  Atomic::inc(&pending_signals[sig]);
  ::sem_post(&sig_sem);
}

static int check_pending_signals(bool wait) {
  Atomic::store(0, &sigint_count);
  for (;;) {
    for (int i = 0; i < NSIG + 1; i++) {
      jint n = pending_signals[i];
      if (n > 0 && n == Atomic::cmpxchg(n - 1, &pending_signals[i], n)) {
        return i;
      }
    }
    if (!wait) {
      return -1;
    }
    JavaThread *thread = JavaThread::current();
    ThreadBlockInVM tbivm(thread);

    bool threadIsSuspended;
    do {
      thread->set_suspend_equivalent();
      // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
      ::sem_wait(&sig_sem);

      // were we externally suspended while we were waiting?
      threadIsSuspended = thread->handle_special_suspend_equivalent_condition();
      if (threadIsSuspended) {
        //
        // The semaphore has been incremented, but while we were waiting
        // another thread suspended us. We don't want to continue running
        // while suspended because that would surprise the thread that
        // suspended us.
        //
        ::sem_post(&sig_sem);

        thread->java_suspend_self();
      }
    } while (threadIsSuspended);
  }
}

int os::signal_lookup() {
  return check_pending_signals(false);
}

int os::signal_wait() {
  return check_pending_signals(true);
}

////////////////////////////////////////////////////////////////////////////////
// Virtual Memory


int os::vm_page_size() {
  // Seems redundant as all get out
  assert(os::Linux::page_size() != -1, "must call os::init");
  return os::Linux::page_size();
}

// Solaris allocates memory by pages.
int os::vm_allocation_granularity() {
  assert(os::Linux::page_size() != -1, "must call os::init");
  return os::Linux::page_size();
}

// NOTE: Linux kernel does not really reserve the pages for us. 
//       All it does is to check if there are enough free pages 
//       left at the time of mmap(). This could be a potential  
//       problem.                                               
bool os::commit_memory(char* addr, size_t size) {
// return ::mmap(addr, size,
//               PROT_READ|PROT_WRITE|PROT_EXEC,
//               MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0)
//   != MAP_FAILED;

  uintptr_t res = (uintptr_t) ::mmap(addr, size,
		                     PROT_READ|PROT_WRITE|PROT_EXEC,
                                     MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);
  int err = errno;
  return res != (uintptr_t) MAP_FAILED;
}

bool os::commit_memory(char* addr, size_t size, size_t alignment_hint) {
  return commit_memory(addr, size);
}

bool os::uncommit_memory(char* addr, size_t size) {
  return ::mmap(addr, size,
		PROT_READ|PROT_WRITE|PROT_EXEC,
		MAP_PRIVATE|MAP_FIXED|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0)
    != MAP_FAILED;
}

static address _highest_vm_reserved_address = NULL;

char* os::reserve_memory(size_t bytes, char* requested_addr) {
  char * addr;
  int flags;

  if (requested_addr != NULL) {
      flags = MAP_FIXED | MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;
      addr = requested_addr;
  } else {
      flags = MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS;
      addr = NULL;
  }

  addr = (char*)::mmap(addr, bytes, PROT_READ|PROT_WRITE|PROT_EXEC,
                             flags, -1, 0);

  guarantee(requested_addr == NULL || requested_addr == addr,
	    "OS failed to return requested mmap address.");

  if (addr != MAP_FAILED) {
    // os::reserve_memory() should only get called during VM initialization,
    // don't need lock (actually we can skip locking even it can be called
    // from multiple threads, because _highest_vm_reserved_address is just a
    // hint about the upper limit of non-stack memory regions.)
    if ((address)addr + bytes > _highest_vm_reserved_address) {
      _highest_vm_reserved_address = (address)addr + bytes;
    }
  }

  return addr == MAP_FAILED ? NULL : addr;
}

bool os::release_memory(char* addr, size_t size) {
  // Don't update _highest_vm_reserved_address, because there might be memory
  // regions above addr + size. If so, releasing a memory region only creates
  // a hole in the address space, it doesn't help prevent heap-stack collision.
  // 
  return ::munmap(addr, size) == 0;
}

static address highest_vm_reserved_address() {
  return _highest_vm_reserved_address;
}

static bool linux_mprotect(char* addr, size_t size, int prot) {
  // Linux wants the mprotect address argument to be page aligned.
  char* bottom = (char*)align_size_down((intptr_t)addr, os::Linux::page_size());
  size = align_size_up(pointer_delta(addr, bottom, 1) + size, os::Linux::page_size());
  return ::mprotect(bottom, size, prot) == 0;
}

bool os::protect_memory(char* addr, size_t size) {
  return linux_mprotect(addr, size, PROT_READ);
}

bool os::guard_memory(char* addr, size_t size) {
  return linux_mprotect(addr, size, PROT_NONE);
}

bool os::unguard_memory(char* addr, size_t size) {
  return linux_mprotect(addr, size, PROT_READ|PROT_WRITE|PROT_EXEC);
}


// Reserve memory at an arbitrary address, only if that area is
// available (and not reserved for something else).

char* os::attempt_reserve_memory_at(size_t bytes, char* requested_addr) {
  const int max_tries = 5;
  char* base[max_tries];
  size_t size[max_tries];
  const size_t gap = 0x000000;

  // Assert that this block is a multiple of 4MB (which is the minimum 
  // size increment for the Java heap).  A multiple of the page size 
  // would probably do. 
  assert(bytes % (4*1024*1024) == 0, "reserving unexpected size block"); 

  // Repeatedly allocate blocks until the block is allocated at the
  // right spot. Give up after max_tries. Note that reserve_memory() will 
  // automatically update _highest_vm_reserved_address if the call is 
  // successful. The variable tracks the highest memory address every reserved
  // by JVM. It is used to detect heap-stack collision if running with
  // fixed-stack LinuxThreads. Because here we may attempt to reserve more
  // space than needed, it could confuse the collision detecting code. To 
  // solve the problem, save current _highest_vm_reserved_address and 
  // calculate the correct value before return.
  address old_highest = _highest_vm_reserved_address;

  int i;
  for (i = 0; i < max_tries; ++i) {
    base[i] = reserve_memory(bytes);

    if (base[i] != NULL) {
      // Is this the block we wanted?
      if (base[i] == requested_addr) {
        size[i] = bytes;
        break;
      }

      // Does this overlap the block we wanted? Give back the overlapped
      // parts and try again.

      size_t top_overlap = requested_addr + (bytes + gap) - base[i];
      if (top_overlap >= 0 && top_overlap < bytes) {
        unmap_memory(base[i], top_overlap);
        base[i] += top_overlap;
        size[i] = bytes - top_overlap;
      } else {
        size_t bottom_overlap = base[i] + bytes - requested_addr;
        if (bottom_overlap >= 0 && bottom_overlap < bytes) {
          unmap_memory(requested_addr, bottom_overlap);
          size[i] = bytes - bottom_overlap;
        } else {
          size[i] = bytes;
        }
      }
    }
  }

  // Give back the unused reserved pieces.

  for (int j = 0; j < i; ++j) {
    if (base[j] != NULL) {
      unmap_memory(base[j], size[j]);
    }
  }

  if (i < max_tries) {
    _highest_vm_reserved_address = MAX2(old_highest, (address)requested_addr + bytes);
    return requested_addr;
  } else {
    _highest_vm_reserved_address = old_highest;
    return NULL;
  }
}


size_t os::read(int fd, void *buf, unsigned int nBytes) {
  return ::read(fd, buf, nBytes);
}  

// TODO-FIXME: reconcile Solaris' os::sleep with the linux variation.
// Solaris uses poll(), linux uses park().  
// Poll() is likely a better choice, assuming that Thread.interrupt()
// generates a SIGUSRx signal. 

int os::sleep(Thread* thread, jlong millis, bool interruptible) {
  assert(thread == Thread::current(),  "thread consistency check");

  if (interruptible) {
    OSThread* osthread = thread->osthread();
    Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
    event->reset() ; 
    OrderAccess::fence() ; 

    jlong prevtime = javaTimeMillis();

    for (;;) {
      if (os::is_interrupted(thread, true)) {
        return OS_INTRPT;
      }

      jlong newtime = javaTimeMillis();
      assert(newtime >= prevtime, "time moving backwards");
      /* Doing prevtime and newtime in microseconds doesn't help precision,
         and trying to round up to avoid lost milliseconds can result in a
         too-short delay. */
      millis -= newtime - prevtime;
      if(millis <= 0) {
        return OS_OK;
      }

      prevtime = newtime;

      {
        ThreadBlockInVM tbivm((JavaThread*) thread);
        OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
        event->park(millis);

#if 0
        // XXX - This code was not exercised during the Merlin
        // pre-integration test cycle so it has been removed to
        // reduce risk.
        //
        // were we externally suspended while we were waiting?
        if (((JavaThread *) thread)->is_external_suspend_with_lock()) {
          //
          // While we were waiting in park() another thread suspended us.
          // We don't want to continue running while suspended because
          // that would surprise the thread that suspended us.
          //
          ((JavaThread *) thread)->java_suspend_self();
        }
#endif
      }
    }
  } else {
    OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
    Linux::Event event;
    event.lock();
    int rslt = event.timedwait(millis);
    event.unlock();
    return rslt;
  }
}

// Sleep forever; naked call to OS-specific sleep; use with CAUTION
void os::infinite_sleep() {
  while (true) {    // sleep forever ...
    ::sleep(100);   // ... 100 seconds at a time
  }
}

// Used to convert frequent JVM_Yield() to nops
bool os::dont_yield() {
  return DontYieldALot;
}

void os::yield() {
  sched_yield();
}

void os::yield_all(int attempts) {
  // Yields to all threads, including threads with lower priorities
  // Threads on Linux are all with same priority. The Solaris style
  // os::yield_all() with nanosleep(1ms) is not necessary.
  sched_yield();
}

// Called from the tight loops to possibly influence time-sharing heuristics
void os::loop_breaker(int attempts) {
  os::yield_all(attempts);
}

////////////////////////////////////////////////////////////////////////////////
// thread priority support

// TODO: the entire priority issue needs to be overhauled.
// [jk]: Linux doesn't support priorities with SCHED_OTHER

#define T_PRI_MIN   1
#define T_PRI_NORM  1
#define T_PRI_MAX   1

int os::java_to_os_priority[MaxPriority + 1] = {
  0,              // 0 Entry should never be used

  1,              // 1 MinPriority
  2,              // 2
  3,              // 3

  4,              // 4
  5,              // 5 NormPriority
  6,              // 6

  7,              // 7
  8,              // 8
  9,              // 9 NearMaxPriority

  10              // 10 MaxPriority
};

// Note: LinuxThreads only honor thread priority for real time threads.
// sched_priority is ignored if policy is SCHED_OTHER. This function is
// equivalent to a "noop" on current Linux platforms.
OSReturn os::set_native_priority(Thread* thread, int newpri) {
  if ( !UseThreadPriorities ) return OS_OK;
  pthread_t thr = thread->osthread()->pthread_id();
  int policy = SCHED_OTHER;
  struct sched_param param;
  param.sched_priority = newpri;
  int ret = pthread_setschedparam(thr, policy, &param);
  return (ret == 0) ? OS_OK : OS_ERR;
}

OSReturn os::get_native_priority(const Thread* const thread, int *priority_ptr) {
  if ( !UseThreadPriorities ) {
    *priority_ptr = java_to_os_priority[NormPriority];
    return OS_OK;
  }
  pthread_t thr = thread->osthread()->pthread_id();
  int policy = SCHED_OTHER;
  struct sched_param param;
  int ret = pthread_getschedparam(thr, &policy, &param);
  if (param.sched_priority < MinPriority) {
    *priority_ptr = MinPriority;
  } else if (param.sched_priority > MaxPriority) {
    *priority_ptr = MaxPriority;
  } else {
    *priority_ptr = param.sched_priority;
  }
  return (ret == 0) ? OS_OK : OS_ERR;
}

// Hint to the underlying OS that a task switch would not be good.
// Void return because it's a hint and can fail.
void os::hint_no_preempt() {}

////////////////////////////////////////////////////////////////////////////////
// suspend/resume/interrupt support

static void resume_clear_context(OSThread *osthread) {
  osthread->set_ucontext(NULL);
  osthread->set_siginfo(NULL);

  // notify the suspend action is completed, we have now resumed
  osthread->sr.clear_suspended();
}

static void suspend_save_context(OSThread *osthread, siginfo_t* siginfo, ucontext_t* context) {
  osthread->set_ucontext(context);
  osthread->set_siginfo(siginfo);
}

//
// Handler function invoked when a thread's execution is suspended or
// resumed. We have to be careful that only async-safe functions are
// called here (Note: most pthread functions are not async safe and 
// should be avoided.)
//
// Note: sigwait() is a more natural fit than sigsuspend() from an
// interface point of view, but sigwait() prevents the signal hander
// from being run. libpthread would get very confused by not having
// its signal handlers run and prevents sigwait()'s use with the
// mutex granting granting signal.
//
static void SR_handler(int sig, siginfo_t* siginfo, ucontext_t* context) {
  // Save and restore errno to avoid confusing native code with EINTR
  // after sigsuspend.
  int old_errno = errno;

  Thread* thread = Thread::current();
  OSThread* osthread = thread->osthread();

  // read current suspend action
  int action = osthread->sr.suspend_action();
  if (action == SR_SUSPEND) {

    assert(thread->is_Java_thread(), "Must be Java thread");
    JavaThread* jt = (JavaThread*) thread;

    // 
    // We are only "logically" blocked.
    // NOTE: this will make the stack walkable (i.e. flush windows as needed) but may not
    // mark anchor as flushed unless we have a last_Java_sp. This allows profiling to have
    // a readable window but doesn't subvert the stack walking invariants.

    jt->frame_anchor()->make_walkable(jt);

    suspend_save_context(osthread, siginfo, context);

    // Notify the suspend action is about to be completed. do_suspend()
    // waits until SR_SUSPENDED is set and then returns. We will wait
    // here for a resume signal and that completes the suspend-other
    // action. do_suspend() will return to eventually release the
    // SR_lock and return to the caller. At that point it is
    // possible for a racing resume request to get the SR_lock
    // and send a resume signal before we call sigsuspend() below.
    // Fortunately, the resume signal is blocked until we call
    // sigsuspend().
    //
    // A special case for suspend-other is when we come here
    // while waiting for a mutex, we don't want to get suspended
    // in this handler. Otherwise, VM would hang when Linux decides 
    // to grant that mutex to us. - We will return immediately and 
    // let safe_mutex_lock() handle this.

    // notify the caller
    osthread->sr.set_suspended();

    if (osthread->sr.is_try_mutex_enter()) {
      // thread suspension while waiting for a mutex is implemented as a 
      // spin inside safe_mutex_lock().

      // This ucontext/siginfo is no longer valid but we are still logically suspended
      osthread->set_ucontext(NULL);
      osthread->set_siginfo(NULL);
      errno = old_errno;
      return;
    }

    sigset_t suspend_set;  // signals for sigsuspend()

    // get current set of blocked signals and unblock resume signal
    pthread_sigmask(SIG_BLOCK, NULL, &suspend_set);
    sigdelset(&suspend_set, SR_signum);

    // wait here until we are resumed
    do {
      sigsuspend(&suspend_set);
      // ignore all returns until we get a resume signal
    } while (osthread->sr.suspend_action() != SR_CONTINUE);

    resume_clear_context(osthread);

  } else if (action == SR_CONTINUE && osthread->sr.is_try_mutex_enter()) {

    // Normally, we receive a resume signal in the sigsuspend()
    // call above and the SR_handler() call resulting from the resume
    // signal does no work because is_try_mutex_enter() is false.
    // However, if we were suspended while trying to enter a mutex,
    // then we are not suspended in the above sigsuspend() loop,
    // instead, we are doing a spin inside safe_mutex_lock(), we 
    // need to change suspend_action here.
    resume_clear_context(osthread);
  }

  errno = old_errno;
}


static int SR_initialize() {
  struct sigaction act;
  char *s;
  /* Get signal number to use for suspend/resume */
  if ((s = ::getenv("_JAVA_SR_SIGNUM")) != 0) {
    int sig = ::strtol(s, 0, 10);
    if (sig > 0 || sig < _NSIG) {
	SR_signum = sig;
    }
  }

  assert(SR_signum > SIGSEGV && SR_signum > SIGBUS,
        "SR_signum must be greater than max(SIGSEGV, SIGBUS), see 4355769");

  sigemptyset(&SR_sigset);
  sigaddset(&SR_sigset, SR_signum);

  /* Set up signal handler for suspend/resume */
  act.sa_flags = SA_RESTART|SA_SIGINFO;
  act.sa_handler = (void (*)(int)) SR_handler;
  // SR_signum is blocked by default.
  // 4528190 - We also need to block pthread restart signal (32 on all
  // supported Linux platforms). Note that LinuxThreads need to block
  // this signal for all threads to work properly. So we don't have
  // to use hard-coded signal number when setting up the mask.
  pthread_sigmask(SIG_BLOCK, NULL, &act.sa_mask);

  if (sigaction(SR_signum, &act, 0) == -1) {
    return -1;
  }
  return 0;
}

static int SR_finalize() {
  return 0;
}


int do_suspend(OSThread* osthread, int action) {
  int ret;

  // set suspend action and send signal
  osthread->sr.set_suspend_action(action);

  ret = pthread_kill(osthread->pthread_id(), SR_signum);
  // check return code and wait for notification

  if (ret == 0) {
    for (int i = 0; !osthread->sr.is_suspended(); i++) {
      os::yield_all(i);
    }
  }

  osthread->sr.set_suspend_action(SR_NONE);
  return ret;
}

// Suspend another thread by one level. The VM code tracks suspend
// nesting and handles self-suspension via wait on SR_lock.
// Returns zero on success.
// Solaris found that non-compiler threads also needed ThreadCritical
// May also need to free malloc, etc. locks here. fence currently always true.
int os::pd_suspend_thread(Thread* thread, bool fence) {
    int ret;
    OSThread *osthread = thread->osthread();

    if (fence) {
      ThreadCritical tc;
      ret = do_suspend(osthread, SR_SUSPEND);
    } else {
      ret = do_suspend(osthread, SR_SUSPEND);
    }
    return ret;
}

// Resume a thread by one level.  This method assumes that consecutive
// suspends nest and require matching resumes to fully resume.  Note that
// this is different from Java's Thread.resume, which always resumes any
// number of nested suspensions.  The ability to nest suspensions is used
// by other facilities like safe points.  
// Resuming a thread that is not suspended is a no-op.
// Returns zero on success.

int os::pd_resume_thread(Thread* thread) {
  OSThread *osthread = thread->osthread();
  int ret;
  int i;

  // higher layers should catch resume of a thread that is not suspended
  assert(osthread->sr.is_suspended(), "thread should be suspended");
  if (!osthread->sr.is_suspended()) {
    return -1;  // robustness
  }

  osthread->sr.set_suspend_action(SR_CONTINUE);

  ret = pthread_kill(osthread->pthread_id(), SR_signum);

  if (ret == 0) {
    for (i = 0; osthread->sr.is_suspended(); i++) {
      os::yield_all(i);
    }
  }

  osthread->sr.set_suspend_action(SR_NONE);
  return ret;
}

void os::interrupt(Thread* thread) {
  assert(Thread::current() == thread || Threads_lock->owned_by_self(),
    "possibility of dangling Thread pointer");

  OSThread* osthread = thread->osthread();

  if (!osthread->interrupted()) {
    Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
    osthread->set_interrupted(true);
    // More than one thread can get here with the same value of osthread,
    // resulting in multiple notifications.  We do, however, want the store
    // to interrupted() to be visible to other threads before we execute unpark().
    OrderAccess::release();
    event->unpark();
  }

  // For JSR166. Unpark even if interrupt status already was set
  if (thread->is_Java_thread()) 
    ((JavaThread*)thread)->parker()->unpark();

}

bool os::is_interrupted(Thread* thread, bool clear_interrupted) {
  assert(Thread::current() == thread || Threads_lock->owned_by_self(),
    "possibility of dangling Thread pointer");

  OSThread* osthread = thread->osthread();

  bool interrupted = osthread->interrupted();

  if (interrupted && clear_interrupted) {
    Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
    osthread->set_interrupted(false);
    event->reset();
  }

  return interrupted;
}

///////////////////////////////////////////////////////////////////////////////////
// signal handling (expect suspend/resume/interrupt)

// This routine may be used by user applications as a "hook" to catch signals.
// The user-defined signal handler must pass unrecognized signals to this
// routine, and if it returns true (non-zero), then the signal handler must
// return immediately.  If the flag "abort_if_unrecognized" is true, then this
// routine will never retun false (zero), but instead will execute a VM panic
// routine kill the process.
//
// If this routine returns false, it is OK to call it again.  This allows
// the user-defined signal handler to perform checks either before or after
// the VM performs its own checks.  Naturally, the user code would be making
// a serious error if it tried to handle an exception (such as a null check
// or breakpoint) that the VM was generating for its own correct operation.
//
// This routine may recognize any of the following kinds of signals:
//    SIGBUS, SIGSEGV, SIGILL, SIGFPE, SIGQUIT, SIGPIPE, SIGUSR1.
// It should be consulted by handlers for any of those signals.
//
// The caller of this routine must pass in the three arguments supplied
// to the function referred to in the "sa_sigaction" (not the "sa_handler")
// field of the structure passed to sigaction().  This routine assumes that
// the sa_flags field passed to sigaction() includes SA_SIGINFO and SA_RESTART.
//
// Note that the VM will print warnings if it detects conflicting signal
// handlers, unless invoked with the option "-XX:+AllowUserSignalHandlers".
//
extern "C" int
JVM_handle_linux_signal(int signo, siginfo_t* siginfo,
                        void* ucontext, int abort_if_unrecognized);

void signalHandler(int sig, siginfo_t* info, void* uc) {
  assert(info != NULL && uc != NULL, "it must be old kernel");
  JVM_handle_linux_signal(sig, info, uc, true);
}


// This boolean allows users to forward their own non-matching signals
// to JVM_handle_linux_signal, harmlessly.
bool os::Linux::signal_handlers_are_installed = false;

bool os::Linux::is_sig_ignored(int sig) {
      struct sigaction oact;
      sigaction(sig, (struct sigaction*)NULL, &oact);
      void* ohlr = oact.sa_sigaction ? CAST_FROM_FN_PTR(void*,  oact.sa_sigaction)
                                     : CAST_FROM_FN_PTR(void*,  oact.sa_handler);
      if (ohlr == CAST_FROM_FN_PTR(void*, SIG_IGN))
           return true;
      else 
           return false;
}

// For signal-chaining
struct sigaction os::Linux::sigact[MAXSIGNUM];
unsigned int os::Linux::sigs = 0;
bool os::Linux::libjsig_is_loaded = false;
typedef struct sigaction *(*get_signal_t)(int);
get_signal_t os::Linux::get_signal_action = NULL;

struct sigaction* os::Linux::get_chained_signal_action(int sig) {
  struct sigaction *actp = NULL;
 
  if (libjsig_is_loaded) {
    // Retrieve the old signal handler from libjsig
    actp = (*get_signal_action)(sig);
  }
  if (actp == NULL) {
    // Retrieve the preinstalled signal handler from jvm
    actp = get_preinstalled_handler(sig);
  }

  return actp;
}

bool os::Linux::chained_handler(struct sigaction *actp, int sig,
                                  siginfo_t *siginfo, void *context) {
  // Call the old signal handler
  if (actp->sa_handler == SIG_DFL) {
    // It's more reasonable to let jvm treat it as an unexpected exception
    // instead of taking the default action.
    return false;
  } else if (actp->sa_handler != SIG_IGN) {
    if ((actp->sa_flags & SA_NODEFER) == 0) {
      // automaticlly block the signal
      sigaddset(&(actp->sa_mask), sig);
    }

    sa_handler_t hand;
    sa_sigaction_t sa;
    bool siginfo_flag_set = (actp->sa_flags & SA_SIGINFO) != 0;
    // retrieve the chained handler
    if (siginfo_flag_set) {
      sa = actp->sa_sigaction;
    } else {
      hand = actp->sa_handler;
    }

    if ((actp->sa_flags & SA_RESETHAND) != 0) {
      actp->sa_handler = SIG_DFL;
    }

    // try to honor the signal mask
    sigset_t oset;
    pthread_sigmask(SIG_SETMASK, &(actp->sa_mask), &oset);

    // call into the chained handler
    if (siginfo_flag_set) {
      (*sa)(sig, siginfo, context);
    } else {
      (*hand)(sig);
    }

    // restore the signal mask
    pthread_sigmask(SIG_SETMASK, &oset, 0);
  }
  // Tell jvm's signal handler the signal is taken care of.
  return true;
}

struct sigaction* os::Linux::get_preinstalled_handler(int sig) {
  if ((( (unsigned int)1 << sig ) & sigs) != 0) {
    return &sigact[sig];
  }
  return NULL;
}

void os::Linux::save_preinstalled_handler(int sig, struct sigaction& oldAct) {
  assert(sig > 0 && sig < MAXSIGNUM, "vm signal out of expected range");
  sigact[sig] = oldAct;
  sigs |= (unsigned int)1 << sig;
}

void os::Linux::set_signal_handler(int sig, bool set_installed) {
  // Check for overwrite.
  struct sigaction oldAct;
  sigaction(sig, (struct sigaction*)NULL, &oldAct);

  void* oldhand = oldAct.sa_sigaction
                ? CAST_FROM_FN_PTR(void*,  oldAct.sa_sigaction)
		: CAST_FROM_FN_PTR(void*,  oldAct.sa_handler);
  if (oldhand != CAST_FROM_FN_PTR(void*, SIG_DFL) &&
      oldhand != CAST_FROM_FN_PTR(void*, SIG_IGN) &&
      oldhand != CAST_FROM_FN_PTR(void*, (sa_sigaction_t)signalHandler)) {
    if (AllowUserSignalHandlers || !set_installed) {
      // Do not overwrite; user takes responsibility to forward to us.
      return;
    } else if (UseSignalChaining) {
      // save the old handler in jvm
      save_preinstalled_handler(sig, oldAct);
      // libjsig also interposes the sigaction() call below and saves the
      // old sigaction on it own.
    } else {
      fatal2("Encountered unexpected pre-existing sigaction handler %#lx for signal %d.", (long)oldhand, sig);
    }
  }

  struct sigaction sigAct;
  sigfillset(&(sigAct.sa_mask));
  sigAct.sa_handler = SIG_DFL;
  if (!set_installed) {
    sigAct.sa_flags = SA_SIGINFO|SA_RESTART;
  } else {
    sigAct.sa_sigaction = signalHandler;
    sigAct.sa_flags = SA_SIGINFO|SA_RESTART;
  }

  int ret = sigaction(sig, &sigAct, &oldAct);
  assert(ret == 0, "check");

  void* oldhand2  = oldAct.sa_sigaction
                  ? CAST_FROM_FN_PTR(void*, oldAct.sa_sigaction)
                  : CAST_FROM_FN_PTR(void*, oldAct.sa_handler);
  assert(oldhand2 == oldhand, "no concurrent signal handler installation");
}

// install signal handlers for signals that HotSpot needs to
// handle in order to support Java-level exception handling.

void os::Linux::install_signal_handlers() {
  if (!signal_handlers_are_installed) {
    signal_handlers_are_installed = true;

    // signal-chaining
    typedef void (*signal_setting_t)();
    signal_setting_t begin_signal_setting = NULL;
    signal_setting_t end_signal_setting = NULL;
    begin_signal_setting = CAST_TO_FN_PTR(signal_setting_t,
                             dlsym(RTLD_DEFAULT, "JVM_begin_signal_setting"));
    if (begin_signal_setting != NULL) {
      end_signal_setting = CAST_TO_FN_PTR(signal_setting_t,
                             dlsym(RTLD_DEFAULT, "JVM_end_signal_setting"));
      get_signal_action = CAST_TO_FN_PTR(get_signal_t,
                            dlsym(RTLD_DEFAULT, "JVM_get_signal_action"));
      libjsig_is_loaded = true;
      assert(UseSignalChaining, "should enable signal-chaining");
    }
    if (libjsig_is_loaded) {
      // Tell libjsig jvm is setting signal handlers
      (*begin_signal_setting)();
    }

    set_signal_handler(SIGSEGV, true);
    set_signal_handler(SIGPIPE, true);
    set_signal_handler(SIGBUS, true);
    set_signal_handler(SIGILL, true);
    set_signal_handler(SIGFPE, true);

    if (libjsig_is_loaded) {
      // Tell libjsig jvm finishes setting signal handlers
      (*end_signal_setting)();
    }
  }
}

extern void report_error(char* file_name, int line_no, char* title, char* format, ...);

const char * signames[] = {
  "SIG0",
  "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "SIGTRAP",
  "SIGABRT", "SIGBUS", "SIGFPE", "SIGKILL", "SIGUSR1",
  "SIGSEGV", "SIGUSR2", "SIGPIPE", "SIGALRM", "SIGTERM",
  "SIGSTKFLT", "SIGCHLD", "SIGCONT", "SIGSTOP", "SIGTSTP",
  "SIGTTIN", "SIGTTOU", "SIGURG", "SIGXCPU", "SIGXFSZ",
  "SIGVTALRM", "SIGPROF", "SIGWINCH", "SIGPOLL", "SIGPWR"
  "SIGSYS"
};

const char * os::exception_name(int exception_code, char* buf, int size) {
  if (0 < exception_code && exception_code <= SIGRTMAX) {
     // signal
     if (exception_code < sizeof(signames)/sizeof(const char*)) {
        jio_snprintf(buf, size, "%s", signames[exception_code]);
     } else {
        jio_snprintf(buf, size, "SIG%d", exception_code);
     }
     return buf;
  } else {
     return NULL;
  }
}

// this is called _before_ the global arguments have been parsed
void os::init(void)
{
  char dummy;	/* used to get a guess on initial stack address */
//  first_hrtime = gethrtime();

  _initial_pid = getpid();

  clock_tics_per_sec = sysconf(_SC_CLK_TCK);

  init_random(1234567);

  ThreadCritical::initialize();

  Linux::set_page_size(sysconf(_SC_PAGESIZE));

  if (Linux::page_size() == -1) {
    fatal1("os_linux.cpp: os::init: sysconf failed (%s)", strerror(errno));
  }

  Linux::initialize_system_info();

  // main_thread points to the aboriginal thread
  Linux::_main_thread = pthread_self();

  initial_time_count = os::elapsed_counter();
}

// To install functions for atexit system call
extern "C" {
  static void perfMemory_exit_helper() {
    perfMemory_exit();
  }
}

// this is called _after_ the global arguments have been parsed
jint os::init_2(void)
{
  // Allocate a single page and mark it as readable for safepoint polling
  if( SafepointPolling ) {
    address polling_page = (address) ::mmap(NULL, Linux::page_size(), PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    guarantee( polling_page != MAP_FAILED, "os::init_2: failed to allocate polling page" );

    os::set_polling_page( polling_page );

#ifndef PRODUCT
    if( Verbose )
      tty->print("[SafePoint Polling address: " INTPTR_FORMAT "]\n", (intptr_t)polling_page);
#endif
  }

  if (UseTopLevelExceptionFilter) {
    Linux::install_signal_handlers();
  }

  size_t threadStackSizeInBytes = ThreadStackSize * K;
  if (threadStackSizeInBytes != 0 &&
      threadStackSizeInBytes < Linux::min_stack_allowed) {
        tty->print_cr("\nThe stack size specified is too small, "
                      "Specify at least %dk",
                      Linux::min_stack_allowed / K);
        return JNI_ERR;
  }

  // Make the stack size a multiple of the page size so that
  // the yellow/red zones can be guarded.
  JavaThread::set_stack_size_at_create(round_to(threadStackSizeInBytes,
        vm_page_size()));

  Linux::capture_initial_stack(JavaThread::stack_size_at_create());

  Linux::libpthread_init();
  if (PrintMiscellaneous && (Verbose || WizardMode)) {
     tty->print_cr("[HotSpot is running with %s, %s(%s)]\n", 
          Linux::glibc_version(), Linux::libpthread_version(), 
          Linux::is_floating_stack() ? "floating stack" : "fixed stack");
  }

  // initialize suspend/resume support
  if (SR_initialize() != 0) {
    perror("SR_initialize failed");
    return JNI_ERR;
  }

  if (MaxFDLimit) {
    // set the number of file descriptors to max. print out error
    // if getrlimit/setrlimit fails but continue regardless.
    struct rlimit nbr_files;
    int status = getrlimit(RLIMIT_NOFILE, &nbr_files);
    if (status != 0) {
      if (PrintMiscellaneous && (Verbose || WizardMode))
        perror("os::init_2 getrlimit failed");
    } else {
      nbr_files.rlim_cur = nbr_files.rlim_max;
      status = setrlimit(RLIMIT_NOFILE, &nbr_files);
      if (status != 0) {
        if (PrintMiscellaneous && (Verbose || WizardMode))
          perror("os::init_2 setrlimit failed");
      }
    }
  }

  // Initialize lock used to serialize thread creation (see os::create_thread)
  Linux::set_createThread_lock(new Mutex(Mutex::leaf, "createThread_lock", false));

  // Initialize HPI.
  jint hpi_result = hpi::initialize();
  if (hpi_result != JNI_OK) {
    tty->print_cr("There was an error trying to initialize the HPI library.");
    tty->print_cr("Please check your installation, HotSpot does not work correctly");
    tty->print_cr("when installed in the JDK 1.2 Linux Production Release, or");
    tty->print_cr("with any JDK 1.1.x release.");
    return hpi_result;
  }

  // at-exit methods are called in the reverse order of their registration.
  // atexit functions are called on return from main or as a result of a
  // call to exit(3C). There can be only 32 of these functions registered
  // and atexit() does not set errno.

  if (PerfAllowAtExitRegistration) {
    // only register atexit functions if PerfAllowAtExitRegistration is set.
    // atexit functions can be delayed until process exit time, which
    // can be problematic for embedded VM situations. Embedded VMs should
    // call DestroyJavaVM() to assure that VM resources are released.

    // note: perfMemory_exit_helper atexit function may be removed in
    // the future if the appropriate cleanup code can be added to the
    // VM_Exit VMOperation's doit method.
    if (atexit(perfMemory_exit_helper) != 0) {
      warning("os::init2 atexit(perfMemory_exit_helper) failed");
    }
  }

  return JNI_OK;
}

// Mark the polling page as unreadable
void os::make_polling_page_unreadable(void) {
  if( !SafepointPolling )
    return;

  if( !guard_memory((char*)_polling_page, Linux::page_size()) )
    fatal("Could not disable polling page");
};

// Mark the polling page as readable
void os::make_polling_page_readable(void) {
  if( !SafepointPolling )
    return;

  if( !protect_memory((char *)_polling_page, Linux::page_size()) )
    fatal("Could not enable polling page");
};

int os::processor_count() {
  return Linux::processor_count();
}

int os::active_processor_count() {
  // Linux doesn't yet have a (official) notion of processor sets,
  // so just return the number of online processors.
  int online_cpus = ::sysconf(_SC_NPROCESSORS_ONLN);
  assert(online_cpus > 0 && online_cpus <= Linux::processor_count(), "sanity check");
  return online_cpus;
}

bool os::distribute_processes(uint length, uint* distribution) {
  // Not yet implemented.
  return false;
}

bool os::bind_to_processor(uint processor_id) {
  // Not yet implemented.
  return false;
}

/// 

// A lightweight implementation that does not suspend the target thread and
// thus returns only a hint. Used for profiling only!
ExtendedPC os::get_thread_pc(Thread* thread) {
  // Make sure that it is called by the watcher and the Threads lock is owned.
  assert(Thread::current()->is_Watcher_thread(), "Must be watcher and own Threads_lock");
  // For now, is only used to profile the VM Thread
  assert(thread->is_VM_thread(), "Can only be called for VMThread");

  ExtendedPC epc;

  thread->vm_suspend();
  OSThread* osthread = thread->osthread();
  if (osthread->ucontext() != NULL) {
    epc = os::Linux::ucontext_get_pc(osthread->ucontext());
  } else {
    assert(thread->is_Java_thread(), "Must be Java thread");
    JavaThread* jt = (JavaThread*)thread;
    // Does not need to be walkable sicne we just want pc for sampling
    assert(jt->has_last_Java_frame(), "must have last Java frame");
    epc = jt->frame_anchor()->last_Java_pc();
  }
  thread->vm_resume();

  return epc;
}

ExtendedPC os::fetch_top_frame(Thread* thread, intptr_t** sp, intptr_t** fp) {
  OSThread* osthread = thread->osthread();
  assert(thread->is_vm_suspended(), "must be suspended");
  ucontext_t* uc = osthread->ucontext();
  if (uc != NULL) {
    *sp = os::Linux::ucontext_get_sp(uc);
    *fp = os::Linux::ucontext_get_fp(uc);
    ExtendedPC pc(os::Linux::ucontext_get_pc(uc));
    return pc;
  }
  assert(thread->is_Java_thread(), "Must be Java thread");
  JavaThread* jt = (JavaThread*)thread;

  // We used to have the following:
  //   assert(jt->has_last_Java_frame(), "must have last Java frame");
  //
  // When a new thread is attached via jni_AttachCurrentThread(), it
  // is possible for it to self-suspend in the JavaCallWrapper ctr
  // after it has changed state to _thread_in_Java. If the VMThread
  // is trying to safepoint at the same time as the thread is trying
  // to self-suspend, then we have a race. If the VMThread makes it
  // past its is_any_suspended_with_lock() check before the thread
  // self-suspends, then it will vm_suspend() the thread and try to
  // fetch the top frame. However, the thread hasn't run any Java
  // code yet so _last_Java_sp is NULL. Returning a NULL ExtendedPC
  // will cause the safepoint to retry this thread.
  if (!jt->has_last_Java_frame()) {
    *sp = NULL;
    *fp = NULL;
    ExtendedPC pc;  // create a NULL ExtendedPC
    return pc;
  }
  *sp = jt->last_Java_sp();
  *fp = jt->frame_anchor()->last_Java_fp();
  ExtendedPC pc(jt->frame_anchor()->last_Java_pc());
  return pc;
}

// Hotspot uses signal to implement suspend/resume on Linux. However,
// because LinuxThreads knows nothing about thread suspension in Hotspot,
// it can grant mutex to an already suspended thread. That may cause
// VM hang when it involves locks like SR_lock or ThreadCritical, 
// because the thread that can resume us may not be able to obtain the 
// mutex (it is now owned by us) to complete the task.
// To avoid such hangs, SR_handler() returns immediately if it discovers 
// the thread is waiting for a mutex. We will spin here - to let other 
// threads grab the mutex - as long as we are suspended.
// Note we cannot use any lock in this function nor use higher level self
// suspension logic - that will invalidate the whole idea.
//
// TODO: this function does not provide any help for native code that 
//       calls pthread_mutex_lock() directly, or for glibc function,
//       e.g. malloc(), that needs to lock mutex. A better implementation 
//       might let SR_handler() to walk stack and discover if it's safe
//       or not to suspend. Of course, polling would always be an attractive 
//       alternative to solve suspend/resume problems. Also, this 
//       function could be implemented with CAS, that probably can save a 
//       couple of suspend/resume signals flying around.

extern sigset_t SR_sigset;
#define BLOCK_SR_SIGNAL   pthread_sigmask(SIG_BLOCK, &SR_sigset, NULL)
#define UNBLOCK_SR_SIGNAL pthread_sigmask(SIG_UNBLOCK, &SR_sigset, NULL)

static inline int spin_if_suspended(OSThread* osthread, pthread_mutex_t*_mutex,
                                    int status)
{
  // It is possible that we are interrupted by a suspend signal when we
  // test flag SR_SUSPENDED and clear SR_TRY_MUTEX_ENTER. Fortunately this
  // function is implemented by CAS, so the "test" and "clear" are done
  // atomically.
  while(!osthread->sr.clear_try_mutex_enter_if_not_suspended()) {
    // LinuxThreads just granted the mutex to us, but we are still
    // suspended. We don't want to enter the mutex because otherwise
    // it would surprise the thread that suspended us.
    // Give the mutex to other thread(s) by unlocking it.
    status = pthread_mutex_unlock(_mutex);
    assert(status == 0, "pthread_mutex_unlock");

    // Don't contend the mutex as long as we are still suspended.
    // sigsuspend() will be interrupted by the resume signal.
    // It is necessary to mask suspend/resume signal because we
    // don't want to lose track of resume signal if it is delivered
    // right after we have read suspend_action but before sigsuspend().
    sigset_t suspend_set;
    pthread_sigmask(SIG_BLOCK, NULL, &suspend_set);
    BLOCK_SR_SIGNAL;
    while(osthread->sr.is_suspended()) {
       sigsuspend(&suspend_set);
    }
    UNBLOCK_SR_SIGNAL;

    // try to grab the mutex again
    status = pthread_mutex_lock(_mutex);
    assert(status == 0, "pthread_mutex_lock");
  }

  // if a suspend signal is delivered after this, the thread will be
  // suspended inside SR_handler, because try_mutex_enter is now cleared

  return status;
}

#undef BLOCK_SR_SIGNAL
#undef UNBLOCK_SR_SIGNAL

int os::Linux::safe_mutex_lock(pthread_mutex_t *_mutex)
{
   // get current thread
   Thread *t = ThreadLocalStorage::thread();
   OSThread * osthread = t ? t->osthread() : NULL;
   // Ok for non java threads??? QQQ
   if (!osthread || !t->is_Java_thread() ) {
      return pthread_mutex_lock(_mutex);
   } else {
      JavaThread* jt = (JavaThread*) t;
      // We are only "logically" blocked.
      jt->frame_anchor()->make_walkable(jt);
      // try_mutex_enter is cleared in spin_if_suspended
      osthread->sr.set_try_mutex_enter();

      // normal mutex locking
      int status = pthread_mutex_lock(_mutex);
      assert(status == 0, "pthread_mutex_lock");

      status = spin_if_suspended(osthread, _mutex, status);

      return status;
   }
}

int os::Linux::safe_cond_wait(pthread_cond_t *_cond, pthread_mutex_t *_mutex)
{
   // get current thread
   Thread *t = ThreadLocalStorage::thread();
   OSThread * osthread = t ? t->osthread() : NULL;
   if (!osthread || !t->is_Java_thread()) {
      return pthread_cond_wait(_cond, _mutex);
   } else {
      JavaThread* jt = (JavaThread*) t;
      // We are only "logically" blocked.
      jt->frame_anchor()->make_walkable(jt);
      // try_mutex_enter is cleared in spin_if_suspended
      osthread->sr.set_try_mutex_enter();

      int status = pthread_cond_wait(_cond, _mutex);

      spin_if_suspended(osthread, _mutex, status);

      // return the result from pthread_cond_wait().
      return status;
   }
}

int os::Linux::safe_cond_timedwait(pthread_cond_t *_cond, pthread_mutex_t *_mutex, const struct timespec *_abstime)
{
   // get current thread
   Thread *t = ThreadLocalStorage::thread();
   OSThread * osthread = t ? t->osthread() : NULL;

   if (!osthread || !t->is_Java_thread()) {
      return pthread_cond_timedwait(_cond, _mutex, _abstime);
   } else {
      JavaThread* jt = (JavaThread*) t;
      // We are only "logically" blocked.
      jt->frame_anchor()->make_walkable(jt);
      // try_mutex_enter is cleared in spin_if_suspended
      osthread->sr.set_try_mutex_enter();

      int status = pthread_cond_timedwait(_cond, _mutex, _abstime);

      spin_if_suspended(osthread, _mutex, status);

      // return the result from pthread_cond_timedwait();
      return status;
   }
}

// we need to be able to suspend ourself while at the same (atomic) time
// giving up the SR_lock -- we do this by using the
// SR_lock to implement a suspend_self
int os::pd_self_suspend_thread(Thread* thread) {
    assert(thread->is_Java_thread(), "must be Java thread");
    // We are only "logically" blocked.
    thread->SR_lock()->wait(Mutex::_no_safepoint_check_flag);
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// debug support

#ifndef PRODUCT
static address same_page(address x, address y) {
  int page_bits = -os::vm_page_size();
  if ((jint(x) & page_bits) == (jint(y) & page_bits))
    return x;
  else if (x > y)
    return (address)(jint(y) | ~page_bits) + 1;
  else
    return (address)(jint(y) & page_bits);
}

bool os::find(address addr) {
  Dl_info dlinfo;
  memset(&dlinfo, 0, sizeof(dlinfo));
  if (dladdr(addr, &dlinfo)) {
    tty->print(PTR_FORMAT ": ", addr);
    if (dlinfo.dli_sname != NULL) {
      tty->print("%s+%#x", dlinfo.dli_sname,
                 addr - (intptr_t)dlinfo.dli_saddr);
    } else if (dlinfo.dli_fname) {
      tty->print("<offset %#x>", addr - (intptr_t)dlinfo.dli_fbase);
    } else {
      tty->print("<absolute address>");
    }
    if (dlinfo.dli_fname) {
      tty->print(" in %s", dlinfo.dli_fname);
    }
    if (dlinfo.dli_fbase) {
      tty->print(" at " PTR_FORMAT, dlinfo.dli_fbase);
    }
    tty->cr();

    if (Verbose) {
      // decode some bytes around the PC
      address begin = same_page(addr-40, addr);
      address end   = same_page(addr+40, addr);
      address       lowest = (address) dlinfo.dli_sname;
      if (!lowest)  lowest = (address) dlinfo.dli_fbase;
      if (begin < lowest)  begin = lowest;
      Dl_info dlinfo2;
      if (dladdr(end, &dlinfo2) && dlinfo2.dli_saddr != dlinfo.dli_saddr
	  && end > dlinfo2.dli_saddr && dlinfo2.dli_saddr > begin)
        end = (address) dlinfo2.dli_saddr;
      Disassembler::decode(begin, end);
    }
    return true;
  }
  return false;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// misc

// This does not do anything on Linux. This is basically a hook for being
// able to use structured exception handling (thread-local exception filters)
// on, e.g., Win32.
void
os::os_exception_wrapper(java_call_t f, JavaValue* value, methodHandle* method,
                         JavaCallArguments* args, Thread* thread) {
  f(value, method, args, thread);
}

void os::print_statistics() {
}

int os::message_box(const char* title, const char* message) {
  int i;
  fdStream err(defaultStream::error_fd());
  for (i = 0; i < 78; i++) err.print_raw("=");
  err.cr();
  err.print_raw_cr(title);
  for (i = 0; i < 78; i++) err.print_raw("-");
  err.cr();
  err.print_raw_cr(message);
  for (i = 0; i < 78; i++) err.print_raw("=");
  err.cr();

  char buf[16];
  // Prevent process from exiting upon "read error" without consuming all CPU
  while (::read(0, buf, sizeof(buf)) <= 0) { ::sleep(100); }

  return buf[0] == 'y' || buf[0] == 'Y';
}

int os::stat(const char *path, struct stat *sbuf) {
  char pathbuf[MAX_PATH];
  if (strlen(path) > MAX_PATH - 1) {
    errno = ENAMETOOLONG;
    return -1;
  }
  hpi::native_path(strcpy(pathbuf, path));
  return ::stat(pathbuf, sbuf);
}

bool os::check_heap(bool force) {
  return true;
}

int local_vsnprintf(char* buf, size_t count, const char* format, va_list args) {
  return ::vsnprintf(buf, count, format, args);
}

// Is a (classpath) directory empty?
bool os::dir_is_empty(const char* path) {
  DIR *dir = NULL;
  struct dirent *ptr;

  dir = opendir(path);
  if (dir == NULL) return true;

  /* Scan the directory */
  bool result = true;
  char buf[sizeof(struct dirent) + MAX_PATH];
  while (result && (ptr = ::readdir(dir)) != NULL) {
    if (strcmp(ptr->d_name, ".") != 0 && strcmp(ptr->d_name, "..") != 0) {
      result = false;
    }
  }
  closedir(dir);
  return result;
}


// Map a block of memory.
char* os::map_memory(int fd, const char* file_name, size_t file_offset,
                     char *addr, size_t bytes, bool read_only,
                     bool allow_exec) {
  int prot;
  int flags;

  if (read_only) {
    prot = PROT_READ;
    flags = MAP_SHARED;
  } else {
    prot = PROT_READ | PROT_WRITE;
    flags = MAP_PRIVATE;
  }

  if (allow_exec) {
    prot |= PROT_EXEC;
  }

  if (addr != NULL) {
    flags |= MAP_FIXED;
  }

  char* mapped_address = (char*)mmap(addr, (size_t)bytes, prot, flags,
                                     fd, file_offset);
  if (mapped_address == MAP_FAILED) {
    return NULL;
  }
  return mapped_address;
}


// Unmap a block of memory.
bool os::unmap_memory(char* addr, size_t bytes) {
  return munmap(addr, bytes) == 0;
}

// current_thread_cpu_time(bool) and thread_cpu_time(Thread*, bool) 
// are used by JVM M&M and JVMTI to get user+sys or user CPU time
// of a thread.
//
// current_thread_cpu_time() and thread_cpu_time(Thread*) returns
// the fast estimate available on the platform.

// current_thread_cpu_time() is not optimized for Linux yet
jlong os::current_thread_cpu_time() {
  // return user + sys since the cost is the same
  return os::thread_cpu_time(Thread::current(), true /* user + sys */);
}

jlong os::thread_cpu_time(Thread* thread) {
  // consistent with what current_thread_cpu_time() returns
  return os::thread_cpu_time(thread, true /* user + sys */);
}

jlong os::current_thread_cpu_time(bool user_sys_cpu_time) {
  return os::thread_cpu_time(Thread::current(), user_sys_cpu_time);
}

//
//  -1 on error.
// 
// Note: A future version of Linux will allow pthread_getcpuclockid() to 
//       support reading the clock of any thread in our process.
// 

jlong os::thread_cpu_time(Thread *thread, bool user_sys_cpu_time) {
  static bool proc_pid_cpu_avail = true;
  int i;
  char *s;
  char stat[2048];
  int statlen;
  char proc_name[64];
  int count;
  long sys_time, user_time;
  char string[64];
  int idummy;
  long ldummy;
  FILE *fp;

  // We first try accessing /proc/<pid>/cpu since this is faster to
  // process.  If this file is not present (linux kernels 2.5 and above)
  // then we open /proc/<pid>/stat.
  if ( proc_pid_cpu_avail ) {
    sprintf(proc_name, "/proc/%d/cpu", thread->osthread()->thread_id());
    fp =  fopen(proc_name, "r");
    if ( fp != NULL ) {
      count = fscanf( fp, "%s %lu %lu\n", string, &user_time, &sys_time);
      fclose(fp);
      if ( count != 3 ) return -1;

      if (user_sys_cpu_time) {
        return ((jlong)sys_time + (jlong)user_time) * (1000000000 / clock_tics_per_sec);
      } else {
        return (jlong)user_time * (1000000000 / clock_tics_per_sec);
      }
    }
    else proc_pid_cpu_avail = false;
  }

  sprintf(proc_name, "/proc/%d/stat", thread->osthread()->thread_id());
  fp =  fopen(proc_name, "r");
  if ( fp == NULL ) return -1;
  statlen = fread(stat, 1, 2047, fp);
  stat[statlen] = '\0';
  fclose(fp);

  // Skip pid and the command string. Note that we could be dealing with
  // weird command names, e.g. user could decide to rename java launcher
  // to "java 1.4.2 :)", then the stat file would look like
  //                1234 (java 1.4.2 :)) R ... ...
  // We don't really need to know the command string, just find the last
  // occurrence of ")" and then start parsing from there. See bug 4726580.
  s = strrchr(stat, ')');
  i = 0;
  if (s == NULL ) return -1;

  // Skip blank chars
  do s++; while (isspace(*s));

  count = sscanf(s,"%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu", 
		 &idummy, &idummy, &idummy, &idummy, &idummy, &idummy, 
                 &ldummy, &ldummy, &ldummy, &ldummy, &ldummy, 
                 &user_time, &sys_time);
  if ( count != 13 ) return -1;
  if (user_sys_cpu_time) {
    return ((jlong)sys_time + (jlong)user_time) * (1000000000 / clock_tics_per_sec);
  } else {
    return (jlong)user_time * (1000000000 / clock_tics_per_sec);
  }
}

void os::current_thread_cpu_time_info(jvmtiTimerInfo *info_ptr) {
  info_ptr->max_value = ALL_64_BITS;       // will not wrap in less than 64 bits
  info_ptr->may_skip_backward = false;     // elapsed time not wall time
  info_ptr->may_skip_forward = false;      // elapsed time not wall time
  info_ptr->kind = JVMTI_TIMER_TOTAL_CPU;  // user+system time is returned
}

void os::thread_cpu_time_info(jvmtiTimerInfo *info_ptr) {
  info_ptr->max_value = ALL_64_BITS;       // will not wrap in less than 64 bits
  info_ptr->may_skip_backward = false;     // elapsed time not wall time
  info_ptr->may_skip_forward = false;      // elapsed time not wall time
  info_ptr->kind = JVMTI_TIMER_TOTAL_CPU;  // user+system time is returned
}

bool os::is_thread_cpu_time_supported() {
  return true;
}

bool os::thread_is_running(JavaThread* tp) {
  Unimplemented();
  return false;
}

#ifndef PRODUCT
void os::Linux::Event::verify() {
  guarantee(!Universe::is_fully_initialized() ||
            !Universe::heap()->is_in_reserved((oop)this),
            "Mutex must be in C heap only.");
}

void os::Linux::OSMutex::verify() {
  guarantee(!Universe::is_fully_initialized() || 
    	    !Universe::heap()->is_in_reserved((oop)this), 
    	    "OSMutex must be in C heap only.");
}

void os::Linux::OSMutex::verify_locked() {
  pthread_t my_id = pthread_self();
  assert(_is_owned, "OSMutex should be locked");
  assert(pthread_equal(_owner, my_id), "OSMutex should be locked by me");
}
#endif

extern "C" {

/**
 * NOTE: the following code is to keep the green threads code
 * in the libjava.so happy. Once the green threads is removed,
 * these code will no longer be needed.
 */
int
jdk_waitpid(pid_t pid, int* status, int options) {
    return waitpid(pid, status, options);
}

int
fork1() {
    return fork();
}

int
jdk_sem_init(sem_t *sem, int pshared, unsigned int value) {
    return sem_init(sem, pshared, value);
}

int
jdk_sem_post(sem_t *sem) {
    return sem_post(sem);
}

int
jdk_sem_wait(sem_t *sem) {
    return sem_wait(sem);
}

int
jdk_pthread_sigmask(int how , const sigset_t* newmask, sigset_t* oldmask) {
    return pthread_sigmask(how , newmask, oldmask);
}

}
