#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os.cpp	1.150 04/06/09 19:46:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_os.cpp.incl"

# include <signal.h>


OSThread*   os::_starting_thread    = NULL;
address     os::_polling_page       = NULL;
long        os::_rand_seed          = 1;
#ifndef PRODUCT
int os::num_mallocs = 0;            // # of calls to malloc/realloc
size_t os::alloc_bytes = 0;         // # of bytes allocated
int os::num_frees = 0;              // # of calls to free
#endif


OSReturn os::set_priority(Thread* thread, ThreadPriority p) {
#ifdef ASSERT
  if (!(!thread->is_Java_thread() ||
         Thread::current() == thread  ||
         Threads_lock->owned_by_self()
#ifndef CORE
         || thread->is_Compiler_thread()
#endif
	)) {
    assert(false, "possibility of dangling Thread pointer");
  }
#endif

  if (p >= MinPriority && p <= MaxPriority) {
    int priority = java_to_os_priority[p];
    return set_native_priority(thread, priority);
  } else {
    assert(false, "Should not happen");
    return OS_ERR;
  }
}


OSReturn os::get_priority(const Thread* const thread, ThreadPriority& priority) {
  int p;
  int os_prio;
  OSReturn ret = get_native_priority(thread, &os_prio);
  if (ret != OS_OK) return ret;

  for (p = MaxPriority; p > MinPriority && java_to_os_priority[p] > os_prio; p--) ;
  priority = (ThreadPriority)p;
  return OS_OK;
}


// --------------------- sun.misc.Signal (optional) ---------------------


// SIGBREAK is sent by the keyboard to query the VM state
#ifndef SIGBREAK
#define SIGBREAK SIGQUIT
#endif

// sigexitnum_pd is a platform-specific special signal used for terminating the Signal thread.


static void signal_thread_entry(JavaThread* thread, TRAPS) {
  os::set_priority(thread, MaxPriority);  
  while (true) {
    int sig;
    {
      // FIXME : Currently we have not decieded what should be the status
      //         for this java thread blocked here. Once we decide about
      //         that we should fix this.
      sig = os::signal_wait();
    }
    if (sig == os::sigexitnum_pd()) {
       // Terminate the signal thread
       return;
    }

    switch (sig) {
      case SIGBREAK: {
        // Print stack traces
	// Any SIGBREAK operations added here should make sure to flush
	// the output stream (e.g. tty->flush()) after output.  See 4803766.
        VM_PrintThreads op;
        VMThread::execute(&op);
        VM_FindDeadlocks op1;
        VMThread::execute(&op1);
        if (PrintClassHistogram) {
          VM_GC_HeapInspection op1;
          VMThread::execute(&op1);
        }
        if (JvmtiExport::should_post_data_dump()) {
          JvmtiExport::post_data_dump();
        }
        jvmpi::post_dump_event();
        break;
      }
      default: {        
        // Dispatch the signal to java
        HandleMark hm(THREAD);
        klassOop k = SystemDictionary::resolve_or_null(vmSymbolHandles::sun_misc_Signal(), THREAD);
        KlassHandle klass (THREAD, k);
        if (klass.not_null()) {
          JavaValue result(T_VOID);
          JavaCallArguments args;
          args.push_int(sig);
          JavaCalls::call_static(
            &result,
            klass, 
            vmSymbolHandles::dispatch_name(), 
            vmSymbolHandles::int_void_signature(),
            &args,
            THREAD
          );
        }
        CLEAR_PENDING_EXCEPTION;
      }
    }
  }
}


void os::signal_init() {
  if (!ReduceSignalUsage) {
    // Setup JavaThread for processing signals
    EXCEPTION_MARK;
    klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_Thread(), true, CHECK);
    instanceKlassHandle klass (THREAD, k);
    instanceHandle thread_oop = klass->allocate_instance_handle(CHECK);

    const char thread_name[] = "Signal Dispatcher";
    Handle string = java_lang_String::create_from_str(thread_name, CHECK);    

    // Initialize thread_oop to put it into the system threadGroup
    Handle thread_group (THREAD, Universe::system_thread_group());
    JavaValue result(T_VOID);
    JavaCalls::call_special(&result, thread_oop, 
                           klass, 
                           vmSymbolHandles::object_initializer_name(), 
                           vmSymbolHandles::threadgroup_string_void_signature(), 
                           thread_group, 
                           string, 
                           CHECK);  
    
    KlassHandle group(THREAD, SystemDictionary::threadGroup_klass());
    JavaCalls::call_special(&result,
                            thread_group,
                            group,
                            vmSymbolHandles::add_method_name(),
                            vmSymbolHandles::thread_void_signature(),
			    thread_oop,		// ARG 1
                            CHECK);

    os::signal_init_pd();

    { MutexLocker mu(Threads_lock);
      JavaThread* signal_thread = new JavaThread(&signal_thread_entry);
                                                                                                                              
      // At this point it may be possible that no osthread was created for the
      // JavaThread due to lack of memory. We would have to throw an exception
      // in that case. However, since this must work and we do not allow
      // exceptions anyway, check and abort if this fails.
      if (signal_thread == NULL || signal_thread->osthread() == NULL) {
        vm_exit_during_initialization("java.lang.OutOfMemoryError",
                                      "unable to create new native thread");
      }

      java_lang_Thread::set_thread(thread_oop(), signal_thread);
      java_lang_Thread::set_priority(thread_oop(), MaxPriority);
      java_lang_Thread::set_daemon(thread_oop());
         
      signal_thread->set_threadObj(thread_oop());
      Threads::add(signal_thread);
      Thread::start(signal_thread);
    }
    // Handle ^BREAK
    os::signal(SIGBREAK, os::user_handler());
  }
}


void os::terminate_signal_thread() {
  if (!ReduceSignalUsage)
    signal_notify(sigexitnum_pd());
}


// --------------------- loading libraries ---------------------

typedef jint (JNICALL *JNI_OnLoad_t)(JavaVM *, void *);
extern struct JavaVM_ main_vm;

static void* _native_java_library = NULL;

void* os::native_java_library() {
  if (_native_java_library == NULL) {
    char buffer[JVM_MAXPATHLEN];
    char ebuf[1024];

    // Try to load verify dll first. In 1.3 java dll depends on it and is not always
    // able to find it when the loading executable is outside the JDK. 
    // In order to keep working with 1.2 we ignore any loading errors.
    hpi::dll_build_name(buffer, sizeof(buffer), Arguments::get_dll_dir(), "verify");
    hpi::dll_load(buffer, ebuf, sizeof(ebuf));

    // Load java dll
    hpi::dll_build_name(buffer, sizeof(buffer), Arguments::get_dll_dir(), "java");
    _native_java_library = hpi::dll_load(buffer, ebuf, sizeof(ebuf));
    if (_native_java_library == NULL) {
      vm_exit_during_initialization("Unable to load native library", ebuf);
    }
    // The JNI_OnLoad handling is normally done by method load in java.lang.ClassLoader$NativeLibrary,
    // but the VM loads the base library explicitly so we have to check for JNI_OnLoad as well
    const char *onLoadSymbols[] = JNI_ONLOAD_SYMBOLS;
    JNI_OnLoad_t JNI_OnLoad = CAST_TO_FN_PTR(JNI_OnLoad_t, hpi::dll_lookup(_native_java_library, onLoadSymbols[0]));
    if (JNI_OnLoad != NULL) {
      JavaThread* thread = JavaThread::current();
      ThreadToNativeFromVM ttn(thread);
      HandleMark hm(thread);
      jint ver = (*JNI_OnLoad)(&main_vm, NULL);
      if (!Threads::is_supported_jni_version_including_1_1(ver)) {
        vm_exit_during_initialization("Unsupported JNI version");
      }
    }
  }
  return _native_java_library;
}

// --------------------- heap allocation utilities ---------------------

char *os::strdup(const char *str) {
  size_t size = strlen(str);
  char *dup_str = (char *)malloc(size + 1);
  if (dup_str == NULL) return NULL;
  strcpy(dup_str, str);
  return dup_str;
}



#ifdef ASSERT
#define space_before             (MallocCushion + sizeof(double))
#define space_after              MallocCushion
#define size_addr_from_base(p)   (size_t*)(p + space_before - sizeof(size_t))
#define size_addr_from_obj(p)    ((size_t*)p - 1)
// MallocCushion: size of extra cushion allocated around objects with +UseMallocOnly
// NB: cannot be debug variable, because these aren't set from the command line until
// *after* the first few allocs already happened
#define MallocCushion            16 
#else
#define space_before             0
#define space_after              0
#define size_addr_from_base(p)   should not use w/o ASSERT
#define size_addr_from_obj(p)    should not use w/o ASSERT
#define MallocCushion            0 
#endif
#define paranoid                 0  /* only set to 1 if you suspect checking code has bug */

#ifdef ASSERT
inline size_t get_size(void* obj) {
  size_t size = *size_addr_from_obj(obj);
  if (size < 0 )
    fatal2("free: size field of object #%p was overwritten (%lu)", obj, size);
  return size;
}

u_char* find_cushion_backwards(u_char* start) {
  u_char* p = start; 
  while (p[ 0] != badResourceValue || p[-1] != badResourceValue ||
         p[-2] != badResourceValue || p[-3] != badResourceValue) p--;
  // ok, we have four consecutive marker bytes; find start
  u_char* q = p - 4;
  while (*q == badResourceValue) q--;
  return q + 1;
}

u_char* find_cushion_forwards(u_char* start) {
  u_char* p = start; 
  while (p[0] != badResourceValue || p[1] != badResourceValue ||
         p[2] != badResourceValue || p[3] != badResourceValue) p++;
  // ok, we have four consecutive marker bytes; find end of cushion
  u_char* q = p + 4;
  while (*q == badResourceValue) q++;
  return q - MallocCushion;
}

void print_neighbor_blocks(void* ptr) {
  // find block allocated before ptr (not entirely crash-proof)
  if (MallocCushion < 4) {
    tty->print_cr("### cannot find previous block (MallocCushion < 4)");
    return;
  }
  u_char* start_of_this_block = (u_char*)ptr - space_before;
  u_char* end_of_prev_block_data = start_of_this_block - space_after -1;
  // look for cushion in front of prev. block
  u_char* start_of_prev_block = find_cushion_backwards(end_of_prev_block_data);
  ptrdiff_t size = *size_addr_from_base(start_of_prev_block);
  u_char* obj = start_of_prev_block + space_before;
  if (size <= 0 ) {
    // start is bad; mayhave been confused by OS data inbetween objects
    // search one more backwards
    start_of_prev_block = find_cushion_backwards(start_of_prev_block);
    size = *size_addr_from_base(start_of_prev_block);
    obj = start_of_prev_block + space_before;  
  }

  if (start_of_prev_block + space_before + size + space_after == start_of_this_block) {
    tty->print_cr("### previous object: %p (%ld bytes)", obj, size);
  } else {
    tty->print_cr("### previous object (not sure if correct): %p (%ld bytes)", obj, size);
  }

  // now find successor block
  u_char* start_of_next_block = (u_char*)ptr + *size_addr_from_obj(ptr) + space_after;
  start_of_next_block = find_cushion_forwards(start_of_next_block);
  u_char* next_obj = start_of_next_block + space_before;
  ptrdiff_t next_size = *size_addr_from_base(start_of_next_block);
  if (start_of_next_block[0] == badResourceValue && 
      start_of_next_block[1] == badResourceValue && 
      start_of_next_block[2] == badResourceValue && 
      start_of_next_block[3] == badResourceValue) {
    tty->print_cr("### next object: %p (%ld bytes)", next_obj, next_size);
  } else {
    tty->print_cr("### next object (not sure if correct): %p (%ld bytes)", next_obj, next_size);
  }
}


void report_heap_error(void* memblock, void* bad, const char* where) {
  tty->print_cr("## nof_mallocs = %d, nof_frees = %d", os::num_mallocs, os::num_frees);
  tty->print_cr("## memory stomp: byte at %p %s object %p", bad, where, memblock);
  print_neighbor_blocks(memblock);
  fatal("memory stomping error");
}

void verify_block(void* memblock) {  
  size_t size = get_size(memblock);
  if (MallocCushion) {
    u_char* ptr = (u_char*)memblock - space_before;
    for (int i = 0; i < MallocCushion; i++) {
      if (ptr[i] != badResourceValue) {
        report_heap_error(memblock, ptr+i, "in front of");
      }
    }
    u_char* end = (u_char*)memblock + size + space_after;
    for (int j = -MallocCushion; j < 0; j++) {
      if (end[j] != badResourceValue) {
        report_heap_error(memblock, end+j, "after");
      }
    }
  }
}
#endif

void* os::malloc(size_t size) {
  NOT_PRODUCT(num_mallocs++);
  NOT_PRODUCT(alloc_bytes += size);

  if (size == 0) {
    // return a valid pointer if size is zero
    // if NULL is returned the calling functions assume out of memory.
    size = 1;
  }

  if (MallocVerifyInterval > 0) check_heap();
  u_char* ptr = (u_char*)::malloc(size + space_before + space_after);
#ifdef ASSERT
  if (ptr == NULL) return NULL;
  if (MallocCushion) {
    for (u_char* p = ptr; p < ptr + MallocCushion; p++) *p = (u_char)badResourceValue;
    u_char* end = ptr + space_before + size;
    for (u_char* q = end; q < end + MallocCushion; q++) *q = (u_char)badResourceValue;
  }
  // put size just before data
  *size_addr_from_base(ptr) = size;
#endif
  u_char* memblock = ptr + space_before;
  if ((intptr_t)memblock == (intptr_t)MallocCatchPtr) {
    tty->print_cr("os::malloc caught, %lu bytes --> %p", size, memblock);
    breakpoint();
  }
  debug_only(if (paranoid) verify_block(memblock));
  if (PrintMalloc && tty != NULL) tty->print_cr("os::malloc %lu bytes --> %p", size, memblock);
  return memblock;
}


void* os::realloc(void *memblock, size_t size) {
  NOT_PRODUCT(num_mallocs++);
  NOT_PRODUCT(alloc_bytes += size);
#ifndef ASSERT
  return ::realloc(memblock, size);
#else
  if (memblock == NULL) {
    return os::malloc(size);
  }
  if ((intptr_t)memblock == (intptr_t)MallocCatchPtr) {
    tty->print_cr("os::realloc caught %p", memblock);
    breakpoint();
  }
  verify_block(memblock);
  if (MallocVerifyInterval > 0) check_heap();
  if (size == 0) return NULL;
  // always move the block
  void* ptr = malloc(size);
  if (PrintMalloc) tty->print_cr("os::remalloc %lu bytes, %p --> %p", size, memblock, ptr);
  // Copy to new memory if malloc didn't fail
  if ( ptr != NULL ) {
    memcpy(ptr, memblock, MIN2(size, get_size(memblock)));
    if (paranoid) verify_block(ptr);
    if ((intptr_t)ptr == (intptr_t)MallocCatchPtr) {
      tty->print_cr("os::realloc caught, %lu bytes --> %p", size, ptr);
      breakpoint();
    }
    free(memblock);
  }
  return ptr;
#endif
}


void  os::free(void *memblock) {
  NOT_PRODUCT(num_frees++);
#ifdef ASSERT
  if (memblock == NULL) return;
  if ((intptr_t)memblock == (intptr_t)MallocCatchPtr) {
    if (tty != NULL) tty->print_cr("os::free caught %p", memblock);
    breakpoint();
  }
  verify_block(memblock);
  if (PrintMalloc && tty != NULL) tty->print_cr("os::free %p", memblock);
  if (MallocVerifyInterval > 0) check_heap();
#endif
  ::free((char*)memblock - space_before);
}

void os::init_random(long initval) {
  _rand_seed = initval;
}


long os::random() {
  /* standard, well-known linear congruential random generator with
   * next_rand = (16807*seed) mod (2**31-1)
   * see
   * (1) "Random Number Generators: Good Ones Are Hard to Find",
   *      S.K. Park and K.W. Miller, Communications of the ACM 31:10 (Oct 1988),
   * (2) "Two Fast Implementations of the 'Minimal Standard' Random 
   *     Number Generator", David G. Carta, Comm. ACM 33, 1 (Jan 1990), pp. 87-88. 
  */
  const long a = 16807;
  const long m = 2147483647;
  const long q = m / a;        assert(q == 127773, "weird math");
  const long r = m % a;        assert(r == 2836, "weird math");

  // compute az=2^31p+q
  unsigned long lo = a * (long)(_rand_seed & 0xFFFF);
  unsigned long hi = a * (long)((unsigned long)_rand_seed >> 16);
  lo += (hi & 0x7FFF) << 16;

  // if q overflowed, ignore the overflow and increment q
  if (lo > m) {
    lo &= m;
    ++lo;
  }
  lo += hi >> 15;

  // if (p+q) overflowed, ignore the overflow and increment (p+q)
  if (lo > m) {
    lo &= m;
    ++lo;
  }
  return (_rand_seed = lo);
}

// The INITIALIZED state is distinguished from the SUSPENDED state because the
// conditions in which a thread is first started are different from those in which
// a suspension is resumed.  These differences make it hard for us to apply the
// tougher checks when starting threads that we want to do when resuming them.
// However, when start_thread is called as a result of Thread.start, on a Java
// thread, the operation is synchronized on the Java Thread object.  So there
// cannot be a race to start the thread and hence for the thread to exit while
// we are working on it.  Non-Java threads that start Java threads either have
// to do so in a context in which races are impossible, or should do appropriate
// locking.

void os::start_thread(Thread* thread) {
  // guard suspend/resume
  MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);

  assert(thread->is_baby_thread(), "thread has started");
  thread->clear_is_baby_thread();

  OSThread* osthread = thread->osthread();

  // A thread may be suspended in the presence of the profiler.
  // Only start thread when it's not suspended. 
  osthread->set_state(RUNNABLE);
  if (!thread->is_vm_suspended()) {
    pd_start_thread(thread);
  }
}

//---------------------------------------------------------------------------
// Helper functions for fatal error handler

void os::print_hex_dump(outputStream* st, address start, address end, int unitsize) {
  assert(unitsize == 1 || unitsize == 2 || unitsize == 4 || unitsize == 8, "just checking");

  int cols = 0;
  int cols_per_line = 0;
  switch (unitsize) {
    case 1: cols_per_line = 16; break;
    case 2: cols_per_line = 8;  break;
    case 4: cols_per_line = 4;  break;
    case 8: cols_per_line = 2;  break;
    default: return;
  }

  address p = start;
  st->print(PTR_FORMAT ":   ", start);
  while (p < end) {
    switch (unitsize) {
      case 1: st->print("%02x", *(u1*)p); break;
      case 2: st->print("%04x", *(u2*)p); break;
      case 4: st->print("%08x", *(u4*)p); break;
      case 8: st->print("%016" FORMAT64_MODIFIER "x", *(u8*)p); break;
    }
    p += unitsize;
    cols++;
    if (cols >= cols_per_line && p < end) {
       cols = 0;
       st->cr();
       st->print(PTR_FORMAT ":   ", p);
    } else {
       st->print(" ");
    }
  }
  st->cr();
}

void os::print_environment_variables(outputStream* st, const char** env_list,
                                     char* buffer, int len) {
  if (env_list) {
    st->print_cr("Environment Variables:");

    for (int i = 0; env_list[i] != NULL; i++) {
      if (getenv(env_list[i], buffer, len)) {
        st->print(env_list[i]);
        st->print("=");
        st->print_cr(buffer);
      }
    }
  }
}

void os::print_cpu_info(outputStream* st) {
  // cpu
  st->print("CPU:");
  st->print("total %d", os::processor_count());
  // It's not safe to query number of active processors after crash
  // st->print("(active %d)", os::active_processor_count());
  st->print(" %s", VM_Version::cpu_features());
  st->cr();
}

// Looks like all platforms except IA64 can use the same function to check
// if C stack is walkable beyond current frame. The check for fp() is not
// necessary on Sparc, but it's harmless.
bool os::is_first_C_frame(frame* fr) {
#ifdef IA64
  // In order to walk native frames on Itanium, we need to access the unwind
  // table, which is inside ELF. We don't want to parse ELF after fatal error,
  // so return true for IA64. If we need to support C stack walking on IA64, 
  // this function needs to be moved to CPU specific files, as fp() on IA64 
  // is register stack, which grows towards higher memory address.
  return true;
#endif

  uintptr_t ufp    = (uintptr_t)fr->fp();
  uintptr_t usp    = (uintptr_t)fr->sp();
  uintptr_t old_sp = (uintptr_t)fr->sender_sp();
  uintptr_t old_fp = (uintptr_t)fr->link();

  // common case
  if (old_sp == 0 || old_sp == (uintptr_t)-1) return true;
  if (old_fp == 0 || old_fp == (uintptr_t)-1 || old_fp == ufp) return true;

  // stack grows downwards; if old_fp is below current fp or if the stack
  // frame is too large, either the stack is corrupted or fp is not saved
  // on stack (i.e. on x86, ebp may be used as general register). The stack 
  // is not walkable beyond current frame.
  if (old_fp < ufp) return true;
  if (old_fp - ufp > 64 * K) return true;

  // see if old_fp and old_sp are correctly aligned
  if ((old_fp & (sizeof(address) - 1)) != 0) return true;
  if ((old_sp & (sizeof(address) - 1)) != 0) return true;

  return false;
}

#ifdef ASSERT
extern "C" void test_random() {
  const double m = 2147483647;
  double mean = 0.0, variance = 0.0, t;
  long reps = 10000;
  unsigned long seed = 1;

  tty->print_cr("seed %ld for %ld repeats...", seed, reps);
  os::init_random(seed);
  long num; 
  for (int k = 0; k < reps; k++) {
    num = os::random();
    double u = (double)num / m;
    assert(u >= 0.0 && u <= 1.0, "bad random number!");

    // calculate mean and variance of the random sequence 
    mean += u;
    variance += (u*u);
  }
  mean /= reps;
  variance /= (reps - 1);

  assert(num == 1043618065, "bad seed");
  tty->print_cr("mean of the 1st 10000 numbers: %f", mean);
  tty->print_cr("variance of the 1st 10000 numbers: %f", variance);
  const double eps = 0.0001;
  // fabs() not always available
  t = (mean - 0.5018) < 0.0 ? -(mean - 0.5018) : mean - 0.5018;
  assert(t < eps, "bad mean");
  t = (variance - 0.3355) < 0.0 ? -(variance - 0.3355) : variance - 0.3355;
  assert(t < eps, "bad variance");
}
#endif


// Set up the boot classpath.

bool os::set_boot_path(char fileSep, char pathSep) {

    assert((fileSep == '/' && pathSep == ':') ||
	   (fileSep == '\\' && pathSep == ';'), "unexpected seperator chars");

    const char* home = Arguments::get_java_home();
    int home_len = (int)strlen(home);

    static const char classpathFormat[] =
	"%/lib/rt.jar:"
	"%/lib/i18n.jar:"
	"%/lib/sunrsasign.jar:"
	"%/lib/jsse.jar:"
	"%/lib/jce.jar:"
        "%/lib/charsets.jar:"
	"%/classes";

    // Scan the format string to determine the length of the actual
    // boot classpath, and handle platform dependencies as well.
    int sysclasspathLen = 0;
    const char* p;
    for (p = classpathFormat; *p != 0; ++p) {
	if (*p == '%') sysclasspathLen += home_len - 1;
	++sysclasspathLen;
    }

    char* sysclasspath = NEW_C_HEAP_ARRAY(char, sysclasspathLen + 1);
    if (sysclasspath == NULL) {
	return false;
    }

    // Create boot classpath from format, substituting separator chars and
    // java home directory.
    char* q = sysclasspath;
    for (p = classpathFormat; *p != 0; ++p) {
	switch (*p) {
	case '%':
	    strcpy(q, home);
	    q += home_len;
	    break;
	case '/':
	    *q++ = fileSep;
	    break;
	case ':':
	    *q++ = pathSep;
	    break;
	default:
	    *q++ = *p;
	}
    }
    *q = '\0';

    assert((q - sysclasspath) == sysclasspathLen, "sysclasspath size botched");
    Arguments::set_sysclasspath(sysclasspath);

    return true;
}

// Returns true if the current stack pointer is above the stack shadow
// pages, false otherwise.

bool os::stack_shadow_pages_available(Thread *thread, methodHandle method) {
  assert(StackRedPages > 0 && StackYellowPages > 0,"Sanity check");
  address sp = current_stack_pointer();
  // Check if we have StackShadowPages above the yellow zone.  This parameter
  // is dependant on the depth of the maximum VM call stack possible from
  // the handler for stack overflow.  'instanceof' in the stack overflow
  // handler or a println uses at least 8k stack of VM and native code
  // respectively.
  const int framesize_in_bytes =
    AbstractInterpreter::size_top_interpreter_activation(method()) * wordSize;
  int reserved_area = ((StackShadowPages + StackRedPages + StackYellowPages) 
                      * vm_page_size()) + framesize_in_bytes;
  // The very lower end of the stack
  address stack_limit = thread->stack_base() - thread->stack_size();
  return (sp > (stack_limit + reserved_area));
}

// This is the working definition of a server class machine:
// >= 2 physical CPU's and >=2GB of memory, with some fuzz 
// because the graphics memory (?) sometimes masks physical memory.
// If you want to change the definition of a server class machine 
// on some OS or platform, e.g., >=4GB on Windohs platforms, 
// then you'll have to parameterize this method based on that state, 
// as was done for logical processors here, or replicate and 
// specialize this method for each platform.  (Or fix os to have 
// some inheritance structure and use subclassing.  Sigh.)
// If you want some platform to always or never behave as a server 
// class machine, change the setting of AlwaysActAsServerClassMachine 
// and NeverActAsServerClassMachine in globals*.hpp.
bool os::is_server_class_machine() {
  // First check for the early returns
  if (NeverActAsServerClassMachine) {
    return false;
  }
  if (AlwaysActAsServerClassMachine) {
    return true;
  }
  // Then actually look at the machine
  bool         result            = false;
  const int    server_processors = 2;
  const julong server_memory     = 2UL * G;
  // We seem not to get our full complement of memory.
  //     We allow some part (1/8?) of the memory to be "missing", 
  //     based on the sizes of DIMMs, and maybe graphics cards.
  const julong missing_memory   = 256UL * M;

  /* Is this a server class machine? */
  if ((os::active_processor_count() >= server_processors) && 
      (os::physical_memory() >= (server_memory - missing_memory))) {
    const unsigned int logical_processors =
      VM_Version::logical_processors_per_package();
    if (logical_processors > 1) {
      const unsigned int physical_packages =
        os::active_processor_count() / logical_processors;
      if (physical_packages > server_processors) {
        result = true;
      }
    } else {
      result = true;
    }
  }
  return result;
}
