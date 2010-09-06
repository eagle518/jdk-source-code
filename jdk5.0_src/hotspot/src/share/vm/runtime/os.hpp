#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os.hpp	1.184 04/06/15 12:17:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// os defines the interface to operating system; this includes traditional
// OS services (time, I/O) as well as other functionality with system-
// dependent code.

typedef void (*dll_func)(...);

class Thread;
class JavaThread;
class Event;
class DLL;
class FileHandle;

// %%%%% Moved ThreadState, START_FN, OSThread to new osThread.hpp. -- Rose

// Platform-independent error return values from OS functions
enum OSReturn {
  OS_OK         =  0,        // Operation was successful
  OS_ERR        = -1,        // Operation failed 
  OS_INTRPT     = -2,        // Operation was interrupted
  OS_TIMEOUT    = -3,        // Operation timed out
  OS_NOMEM      = -5,        // Operation failed for lack of memory
  OS_NORESOURCE = -6         // Operation failed for lack of nonmemory resource
};

enum ThreadPriority {        // JLS 20.20.1-3
  NoPriority       = -1,     // Initial non-priority value
  MinPriority      =  1,     // Minimum priority
  NormPriority     =  5,     // Normal (non-daemon) priority
  NearMaxPriority  =  9,     // High priority, used for VMThread
  MaxPriority      = 10      // Highest priority, used for WatcherThread
                             // ensures that VMThread doesn't starve profiler
};

// Typedef for structured exception handling support 
typedef void (*java_call_t)(JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread);

class os: AllStatic {
 private:
  static OSThread*          _starting_thread;
  static address            _polling_page;

 public:

  static void init(void);			// Called before command line parsing
  static jint init_2(void);                    // Called after command line parsing

  // File names are case-insensitive on windows only
  // Override me as needed
  static inline int    file_name_strcmp(const char* s1, const char* s2);

  static bool getenv(const char* name, char* buffer, int len);
  static bool have_special_privileges();

  static jlong  javaTimeMillis();
  static jlong  javaTimeNanos();
  static void   javaTimeNanos_info(jvmtiTimerInfo *info_ptr);

  // Returns the elapsed time in seconds since the vm started.
  static double elapsedTime();
    
  // Interface to the performance counter
  static jlong elapsed_counter();
  static jlong elapsed_frequency();

  // Interface for detecting multiprocessor system
  static bool is_MP();
  static julong physical_memory();
  static julong allocatable_physical_memory(julong size);
  static bool is_server_class_machine();

  // number of CPUs
  static int processor_count();

  // Returns the number of CPUs this process is currently allowed to run on.
  // Note that on some OSes this can change dynamically.
  static int active_processor_count();

  // Bind processes to processors.
  //     This is a two step procedure:
  //     first you generate a distribution of processes to processors,
  //     then you bind processes according to that distribution.
  // Compute a distribution for number of processes to processors.
  //    Stores the processor id's into the distribution array argument. 
  //    Returns true if it worked, false if it didn't.
  static bool distribute_processes(uint length, uint* distribution);
  // Binds the current process to a processor.
  //    Returns true if it worked, false if it didn't.
  static bool bind_to_processor(uint processor_id);

  // Interface for stack banging (predetect possible stack overflow for
  // exception processing)  There are guard pages, and above that shadow
  // pages for stack overflow checking.
  static inline bool uses_stack_guard_pages();
  static inline bool allocate_stack_guard_pages();
  static inline void bang_stack_shadow_pages();
  static bool stack_shadow_pages_available(Thread *thread, methodHandle method);

  // OS interface to Virtual Memory
  static int    vm_page_size();
  static int    vm_allocation_granularity();
  static char*  reserve_memory(size_t bytes, char* addr = 0);
  static char*  attempt_reserve_memory_at(size_t bytes, char* addr);
  static void   split_reserved_memory(char *base, size_t size,
                                      size_t split, bool realloc);
  static bool   commit_memory(char* addr, size_t bytes);
  static bool   commit_memory(char* addr, size_t size, size_t alignment_hint);
  static bool   uncommit_memory(char* addr, size_t bytes);
  static bool   release_memory(char* addr, size_t bytes);
  static bool   protect_memory(char* addr, size_t bytes);
  static bool   guard_memory(char* addr, size_t bytes);
  static bool   unguard_memory(char* addr, size_t bytes);
  static char*  map_memory(int fd, const char* file_name, size_t file_offset,
                           char *addr, size_t bytes, bool read_only = false,
                           bool allow_exec = false);
  static bool   unmap_memory(char *addr, size_t bytes);
  static char*  non_memory_address_word();
  static char*  reserve_memory_special(size_t size);

  // OS interface to polling page
  static address get_polling_page()             { return _polling_page; }
  static void    set_polling_page(address page) { _polling_page = page; }
  static bool    is_poll_address(address addr)  { return addr >= _polling_page && addr < (_polling_page + os::vm_page_size()); }
  static void    make_polling_page_unreadable();
  static void    make_polling_page_readable();

  // threads

  enum ThreadType {
    vm_thread,
    cms_thread,
    pgc_thread,
    java_thread,
    compiler_thread,
    watcher_thread,
    gc_thread
  };

  static bool create_thread(Thread* thread,
                            ThreadType thr_type,
                            size_t stack_size = 0);
  static bool create_main_thread(Thread* thread);
  static bool create_attached_thread(Thread* thread);
  static void pd_start_thread(Thread* thread);
  static void start_thread(Thread* thread);

  static void initialize_thread();
  static void free_thread(OSThread* osthread);

  // thread id on Linux/64bit is 64bit, on Windows and Solaris, it's 32bit
  static intx current_thread_id();
  static int current_process_id();
  // hpi::read for calls from non native state
  // For performance, hpi::read is only callable from _thread_in_native
  static size_t read(int fd, void *buf, unsigned int nBytes);
  static int sleep(Thread* thread, jlong ms, bool interruptable);
  static void infinite_sleep(); // never returns, use with CAUTION
  static void yield();        // Yields to all threads with same priority
  static void yield_all(int attempts = 0); // Yields to all other threads including lower priority
  static void loop_breaker(int attempts);  // called from within tight loops to possibly influence time-sharing
  static OSReturn set_priority(Thread* thread, ThreadPriority priority);
  static OSReturn get_priority(const Thread* const thread, ThreadPriority& priority);

  static void interrupt(Thread* thread);
  static bool is_interrupted(Thread* thread, bool clear_interrupted);

  static int pd_suspend_thread(Thread* thread, bool _fence);
  static int pd_resume_thread(Thread* thread);
  static int pd_self_suspend_thread(Thread* thread);
  
  static ExtendedPC fetch_top_frame(Thread* thread, intptr_t** sp, intptr_t** fp);
  static ExtendedPC fetch_frame_from_context(void* ucVoid, intptr_t** sp, intptr_t** fp);
  static frame      fetch_frame_from_context(void* ucVoid);

  NOT_CORE(static bool set_thread_pc_and_resume(JavaThread* thread, ExtendedPC old_addr, ExtendedPC new_addr);)
  static ExtendedPC get_thread_pc(Thread *thread);
  static void breakpoint();

  static address current_stack_pointer();
  static address current_stack_base();
  static size_t current_stack_size();

  static int message_box(const char* title, const char* message);
  static char* do_you_want_to_debug(const char* message);

  // os::exit() is merged with vm_exit()
  // static void exit(int num);

  // Terminate the VM, but don't exit the process
  static void shutdown();

  // Terminate with an error.  Default is to generate a core file on platforms
  // that support such things.  This calls shutdown() and then aborts.
  static void abort(bool dump_core = true);

  // Die immediately, no exit hook, no abort hook, no cleanup.
  static void die();

  // Reading directories.
  static DIR*           opendir(const char* dirname);
  static int            readdir_buf_size(const char *path);
  static struct dirent* readdir(DIR* dirp, dirent* dbuf);
  static int            closedir(DIR* dirp);
  
  // Dynamic library extension
  static const char*    dll_file_extension();

  static const char*    get_temp_directory();

  // Symbol lookup, find nearest function name; basically it implements
  // dladdr() for all platforms. Name of the nearest function is copied
  // to buf. Distance from its base address is returned as offset.
  // If function name is not found, buf[0] is set to '\0' and offset is
  // set to -1.
  static bool dll_address_to_function_name(address addr, char* buf,
                                           int buflen, int* offset);

  // Locate DLL/DSO. On success, full path of the library is copied to
  // buf, and offset is set to be the distance between addr and the 
  // library's base address. On failure, buf[0] is set to '\0' and
  // offset is set to -1.
  static bool dll_address_to_library_name(address addr, char* buf,
                                          int buflen, int* offset);

  // Find out whether the pc is in the static code for jvm.dll/libjvm.so.
  static bool address_is_in_vm(address addr);

  // Print out system information; they are called by fatal error handler.
  // Output format may be different on different platforms.
  static void print_os_info(outputStream* st);
  static void print_cpu_info(outputStream* st);
  static void print_memory_info(outputStream* st);
  static void print_dll_info(outputStream* st);
  static void print_environment_variables(outputStream* st, const char** env_list, char* buffer, int len);
  static void print_context(outputStream* st, void* context);
  static void print_siginfo(outputStream* st, void* siginfo);

  // The following two functions are used by fatal error handler to trace 
  // native (C) frames. They are not part of frame.hpp/frame.cpp because 
  // frame.hpp/cpp assume thread is JavaThread, and also because different
  // OS/compiler may have different convention or provide different API to 
  // walk C frames.
  //
  // We don't attempt to become a debugger, so we only follow frames if that 
  // does not require a lookup in the unwind table, which is part of the binary
  // file but may be unsafe to read after a fatal error. So on x86, we can 
  // only walk stack if %ebp is used as frame pointer; on ia64, it's not
  // possible to walk C stack without having the unwind table.
  static bool is_first_C_frame(frame *fr);
  static frame get_sender_for_C_frame(frame *fr);

  // return current frame. pc() and sp() are set to NULL on failure.
  static frame      current_frame();

  static void print_hex_dump(outputStream* st, address start, address end, int unitsize);

  // returns a string to describe the exception/signal; 
  // returns NULL if exception_code is not an OS exception/signal.
  static const char * os::exception_name(int exception_code, char* buf, int buflen);

  // Returns native Java library, loads if necessary
  static void*    native_java_library();

  // Fills in path to jvm.dll/libjvm.so (this info used to find hpi).
  static void     jvm_path(char *buf, jint buflen);

  // JNI names
  static void     print_jni_name_prefix_on(outputStream* st, int args_size);
  static void     print_jni_name_suffix_on(outputStream* st, int args_size);

  // File conventions
  static inline const char* file_separator();
  static inline const char* line_separator();
  static inline const char* path_separator();

  // Init os specific system properties values
  static void init_system_properties_values();

  // IO operations, non-JVM_ version.
  static int stat(const char* path, struct stat* sbuf);
  static bool dir_is_empty(const char* path);

  // Thread Local Storage
  static int   allocate_thread_local_storage();
  static void  thread_local_storage_at_put(int index, void* value);
  static void* thread_local_storage_at(int index);
  static void  free_thread_local_storage(int index);

  // General allocation (must be MT-safe)
  static void* malloc  (size_t size);
  static void* realloc (void *memblock, size_t size);
  static void  free    (void *memblock);
  static bool  check_heap(bool force = false);      // verify C heap integrity
  static char* strdup(const char *);  // Like strdup

#ifndef PRODUCT
  static int  num_mallocs;            // # of calls to malloc/realloc
  static size_t  alloc_bytes;         // # of bytes allocated
  static int  num_frees;              // # of calls to free
#endif

  // Printing 64 bit integers
  static inline const char* jlong_format_specifier();
  static inline const char* julong_format_specifier();

  // Support for signals (see JVM_RaiseSignal, JVM_RegisterSignal)
  static void  signal_init();
  static void  signal_init_pd();
  static void  signal_notify(int signal_number);
  static void* signal(int signal_number, void* handler);
  static void  signal_raise(int signal_number);
  static int   signal_wait();
  static int   signal_lookup();
  static void* user_handler();
  static void  terminate_signal_thread();
  static int   sigexitnum_pd();

  // random number generation
  static long random();                    // return 32bit pseudorandom number
  static void init_random(long initval);   // initialize random sequence

  // Structured OS Exception support
  static void os_exception_wrapper(java_call_t f, JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread);

  // jvmpi
  static bool thread_is_running(JavaThread* tp);

  // JVMTI & JVM monitoring and management support
  // The thread_cpu_time() and current_thread_cpu_time() are only
  // supported if is_thread_cpu_time_supported() returns true.
  // They are not supported on Solaris T1.

  // Thread CPU Time - return the fast estimate on a platform
  // On Solaris - call gethrvtime (fast) - user time only
  // On Linux   - /proc (until a faster way is identified) - user+sys
  // On Windows - GetThreadTimes - user+sys
  static jlong current_thread_cpu_time();
  static jlong thread_cpu_time(Thread* t);

  // Thread CPU Time with user_sys_cpu_time parameter.
  //
  // If user_sys_cpu_time is true, user+sys time is returned.
  // Otherwise, only user time is returned
  static jlong current_thread_cpu_time(bool user_sys_cpu_time);
  static jlong thread_cpu_time(Thread* t, bool user_sys_cpu_time);

  // Return a bunch of info about the timers.
  // Note that the returned info for these two functions may be different
  // on some platforms
  static void current_thread_cpu_time_info(jvmtiTimerInfo *info_ptr);
  static void thread_cpu_time_info(jvmtiTimerInfo *info_ptr);

  static bool is_thread_cpu_time_supported();

  // Hook for os specific jvm options that we don't want to abort on seeing
  static bool obsolete_option(const JavaVMOption *option);

  // Platform dependent stuff
  #include "incls/_os_pd.hpp.incl"

  // debugging support (mostly used by debug.cpp)
  static bool find(address pc) PRODUCT_RETURN0; // OS specific function to make sense out of an address
  
  static bool dont_yield();                     // when true, JVM_Yield() is nop
  static void print_statistics();

  // Thread priority helpers (implemented in OS-specific part)
  static OSReturn set_native_priority(Thread* thread, int native_prio);
  static OSReturn get_native_priority(const Thread* const thread, int* priority_ptr);
  static int java_to_os_priority[MaxPriority + 1];
  // Hint to the underlying OS that a task switch would not be good.
  // Void return because it's a hint and can fail.
  static void hint_no_preempt();

 protected:
  static long _rand_seed;                   // seed for random number generator

  static bool set_boot_path(char fileSep, char pathSep);
};

