#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compileBroker.hpp	1.42 04/03/12 13:06:08 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifdef COMPILER2
class BasicAdapter;
#endif

// CompileTask
//
// An entry in the compile queue.  It represents a pending or current
// compilation.
class CompileTask : public CHeapObj {
 private:
  Monitor*     _lock;
  uint         _compile_id;
  jobject      _method;
  int          _osr_bci;
  bool         _is_complete;
  bool         _is_success;
  bool         _is_blocking;
  int          _adapter_kind;
  int          _comp_level;
  int          _num_inlined_bytecodes;
  CompileTask* _next;

  // Fields used for logging why the compilation was initiated:
  jlong        _time_queued;  // in units of os::elapsed_counter()
  jobject      _hot_method;   // which method actually triggered this task
  int          _hot_count;    // information about its invocation counter
  const char*  _comment;      // more info about the task

 public:
  CompileTask() {
    _comp_level = CompLevel_full_optimization;
    _lock = new Monitor(Mutex::nonleaf+2, "CompileTaskLock");
  }

  void initialize(int compile_id, methodHandle method, int osr_bci,
                  methodHandle hot_method, int hot_count, const char* comment,
                  int adapter_kind, bool is_blocking);

  void free();

  int          compile_id() const                { return _compile_id; }
  jobject      method_handle() const             { return _method; }
  int          osr_bci() const                   { return _osr_bci; }
  bool         is_complete() const               { return _is_complete; }
  bool         is_blocking() const               { return _is_blocking; }
  bool         is_success() const                { return _is_success; }

  Monitor*     lock() const                      { return _lock; }

  void         mark_complete()                   { _is_complete = true; }
  void         mark_success()                    { _is_success = true; }

  int          adapter_kind() const              { return _adapter_kind; }
  bool         is_method_compile() const         { return _adapter_kind == ciEnv::not_adapter; }

  int          comp_level()                      { return _comp_level;}
  void         set_comp_level(int comp_level)    { _comp_level = comp_level;}

  int          num_inlined_bytecodes() const     { return _num_inlined_bytecodes; }
  void         set_num_inlined_bytecodes(int n)  { _num_inlined_bytecodes = n; }

  CompileTask* next() const                      { return _next; }
  void         set_next(CompileTask* next)       { _next = next; }

  void         print();
  void         print_line();
  void         print_line_on_error(outputStream* st, char* buf, int buflen);
  void         log_head(CompileLog* log);
  void         log_tail(CompileLog* log);
};

// CompilerCounters
//
// Per Compiler Performance Counters.
//
class CompilerCounters : public CHeapObj {

  public:
    enum {
      cmname_buffer_length = 160
    };

  private:

    char _current_method[cmname_buffer_length];
    PerfStringVariable* _perf_current_method;

    int  _compile_type;
    PerfVariable* _perf_compile_type;

    PerfCounter* _perf_time;
    PerfCounter* _perf_compiles;

  public:
    CompilerCounters(const char* name, int instance, TRAPS);

    // these methods should be called in a thread safe context

    void set_current_method(const char* method) {
      strncpy(_current_method, method, (size_t)cmname_buffer_length);
      if (UsePerfData) _perf_current_method->set_value(method);
    }

    char* current_method()                  { return _current_method; }

    void set_compile_type(int compile_type) {
      _compile_type = compile_type;
      if (UsePerfData) _perf_compile_type->set_value((jlong)compile_type);
    }

    int compile_type()                       { return _compile_type; }

    PerfCounter* time_counter()              { return _perf_time; }
    PerfCounter* compile_counter()           { return _perf_compiles; }
};


// CompileQueue
//
// A list of CompileTasks.
class CompileQueue : public CHeapObj {
 private:
  const char* _name;
  Monitor*    _lock;

  CompileTask* _first;
  CompileTask* _last;

 public:
  CompileQueue(const char* name, Monitor* lock) {
    _name = name;
    _lock = lock;
    _first = NULL;
    _last = NULL;
  }

  const char*  name() const                      { return _name; }
  Monitor*     lock() const                      { return _lock; }

  void         add(CompileTask* task);

  CompileTask* get();

  bool         is_empty() const                  { return _first == NULL; }

  void         print();
};


// Compilation
//
// The broker for all compilation requests.
class CompileBroker: AllStatic {
 friend class Threads;
  friend class CompileTaskWrapper;

 public:
  enum {
    name_buffer_length = 100
  };

  // Compile type Information for print_last_compile() and CompilerCounters
  enum { no_compile, normal_compile, osr_compile, native_compile };

 private:
  static bool _initialized;
  static volatile bool _should_block;

  // The installed compiler
  static AbstractCompiler* _compiler;

  // These counters are used for assigning id's to each compilation
  static uint _compilation_id;
  static uint _osr_compilation_id;
  static uint _native_compilation_id;

  static int  _last_compile_type;
  static char _last_method_compiled[name_buffer_length];

  static CompileQueue* _adapter_queue;
  static CompileQueue* _method_queue;
  static CompileTask* _task_free_list;

  static CompilerThread*                 _adapter_thread;
  static GrowableArray<CompilerThread*>* _method_threads;

  // performance counters
  static PerfCounter* _perf_total_compilation;
  static PerfCounter* _perf_native_compilation;
  static PerfCounter* _perf_osr_compilation;
  static PerfCounter* _perf_standard_compilation;

  static PerfCounter* _perf_total_bailout_count;
  static PerfCounter* _perf_total_invalidated_count;
  static PerfCounter* _perf_total_compile_count;
  static PerfCounter* _perf_total_native_compile_count;
  static PerfCounter* _perf_total_osr_compile_count;
  static PerfCounter* _perf_total_standard_compile_count;

  static PerfCounter* _perf_sum_osr_bytes_compiled;
  static PerfCounter* _perf_sum_standard_bytes_compiled;
  static PerfCounter* _perf_sum_nmethod_size;
  static PerfCounter* _perf_sum_nmethod_code_size;

  static PerfStringVariable* _perf_last_method;
  static PerfStringVariable* _perf_last_failed_method;
  static PerfStringVariable* _perf_last_invalidated_method;
  static PerfVariable*       _perf_last_compile_type;
  static PerfVariable*       _perf_last_compile_size;
  static PerfVariable*       _perf_last_failed_type;
  static PerfVariable*       _perf_last_invalidated_type;

  // Timers and counters for generating statistics
  static elapsedTimer _t_total_compilation;
  static elapsedTimer _t_native_compilation;
  static elapsedTimer _t_osr_compilation;
  static elapsedTimer _t_standard_compilation;

  static int _total_bailout_count;
  static int _total_invalidated_count;
  static int _total_compile_count;
  static int _total_native_compile_count;
  static int _total_osr_compile_count;
  static int _total_standard_compile_count;
  
  static int _sum_osr_bytes_compiled;
  static int _sum_standard_bytes_compiled;
  static int _sum_nmethod_size;
  static int _sum_nmethod_code_size;

  static int compiler_count() { 
    return CICompilerCountPerCPU
      // Example: if CICompilerCountPerCPU is true, then we get
      // max(log2(8)-1,1) = 2 compiler threads on an 8-way machine.  
      // May help big-app startup time.
      ? (MAX2(log2_intptr(os::active_processor_count())-1,1))
      : CICompilerCount;
  }

  static CompilerThread* make_compiler_thread(const char* name, CompileQueue* queue, CompilerCounters* counters, TRAPS);
  static void init_compiler_threads(int compiler_count);
  static bool check_compilation_result(methodHandle method, int osr_bci, int comp_level, nmethod** result);

#ifdef COMPILER2
  static bool check_adapter_result(methodHandle method, int adapter_kind, BasicAdapter** result);
  static BasicAdapter* compile_adapter_for(methodHandle method, int adapter_kind, bool blocking);
#endif // COMPILER2

  static bool compilation_is_in_queue  (methodHandle method, int osr_bci);
  static bool compilation_is_prohibited(methodHandle method, int osr_bci);
  static bool is_not_compile_only      (methodHandle method);
  static uint assign_compile_id        (methodHandle method, int osr_bci);
  static bool is_compile_blocking      (methodHandle method, int osr_bci);
  static void preload_classes          (methodHandle method, TRAPS);

  static CompileTask* create_compile_task(CompileQueue* queue,
                                          int           compile_id,
                                          methodHandle  method,
                                          int           osr_bci,
                                          methodHandle  hot_method,
                                          int           hot_count,
                                          const char*   comment,
                                          int           adapter_kind,
                                          bool          blocking);
  static CompileTask* allocate_task();
  static void free_task(CompileTask* task);
  static nmethod* wait_for_completion(CompileTask* task);

#ifdef COMPILER2
  static BasicAdapter* wait_for_adapter_completion(CompileTask* task);
  static void eager_compile_i2c_adapters(ciEnv* ci_env, ciMethod* target);
  static void eager_compile_c2i_adapters(ciEnv* ci_env, ciMethod* target);
  static void invoke_compiler_on_adapter(CompileTask* task);
#endif // COMPILER2

  static void invoke_compiler_on_method(CompileTask* task);
  static void set_last_compile(CompilerThread *thread, methodHandle method, bool is_osr, bool is_native);
  static void push_jni_handle_block();
  static void pop_jni_handle_block();
  static bool check_break_at(methodHandle method, int compile_id, bool is_osr, bool is_native);
  static void collect_statistics(CompilerThread* thread, elapsedTimer time, CompileTask* task);

  static nmethod* compile_method_base(methodHandle method, 
                                      int osr_bci,
                                      int comp_level,
                                      methodHandle hot_method, 
                                      int hot_count,
                                      const char* comment, 
                                      TRAPS);

 public:
  enum {
    // The entry bci used for non-OSR compilations.
    standard_entry_bci = InvocationEntryBci
  };

  static AbstractCompiler* compiler()            { return _compiler; }

  static void compilation_init(AbstractCompiler* compiler);
  static void init_compiler_thread_log();
  static nmethod* compile_method(methodHandle method, int osr_bci,
                                 methodHandle hot_method, int hot_count,
                                 const char* comment, TRAPS);

#ifdef COMPILER2
  static C2IAdapter* compile_c2i_adapter_for(methodHandle method, bool blocking) {
    return (C2IAdapter*)compile_adapter_for(method, ciEnv::c2i, blocking);
  }
  static I2CAdapter* compile_i2c_adapter_for(methodHandle method, bool blocking) {
    return (I2CAdapter*)compile_adapter_for(method, ciEnv::i2c, blocking);
  }
#endif // COMPILER2

  static void compiler_thread_loop();

  static bool is_idle();

  // Set _should_block.
  // Call this from the VM, with Threads_lock held and a safepoint requested.
  static void set_should_block();

  // Call this from the compiler at convenient points, to poll for _should_block.
  static void maybe_block();

  // Return total compilation ticks
  static jlong total_compilation_ticks() {
    return _perf_total_compilation->get_value();
  }

  // Print a detailed accounting of compilation time
  static void print_times();

  // Debugging output for failure
  static void print_last_compile();

  static void print_compiler_threads();
};
