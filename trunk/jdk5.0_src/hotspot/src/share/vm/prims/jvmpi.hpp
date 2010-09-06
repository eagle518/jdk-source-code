#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmpi.hpp	1.42 03/12/23 16:43:13 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#define JVMPI_EVENT_DISABLED        0
#define JVMPI_EVENT_NOT_SUPPORTED  -1
#define JVMPI_EVENT_ENABLED        -2

#define JVMPI_PROFILING_OFF        0x00000000
#define JVMPI_PROFILING_ON         0x80000000

#define JVMPI_INVALID_CLASS ((oop)(-1))

typedef struct {
    methodOop method;               /* method being compiled */
    void *code_addr;                /* virtual address of the the method */
    jint code_size;                 /* size of compiled method in memory */
    jint lineno_table_len;          /* number of lineno table entries */
    JVMPI_Lineno *lineno_table;     /* pointer to beginning of line table */
} compiled_method_t;


class jvmpi : public AllStatic {
 private:
  // JVMPI interface data structure
  static JVMPI_Interface jvmpi_interface;
  static bool slow_allocation;

  static void reset_jvmpi_allocation();

  // To track if notification for a particular event type is enabled/disabled.
  static unsigned int _event_flags_array[JVMPI_MAX_EVENT_TYPE_VAL+1];
  static unsigned int _event_flags;

  // initialization
  static void initialize(int version);

  // enable/disable event notification
  static inline void enable_event(jint event_type);
  static inline void disable_event(jint event_type);

  static void post_event(JVMPI_Event* event);

  static void post_event_common(JVMPI_Event* event);

  static void post_event_vm_mode(JVMPI_Event* event, JavaThread* calling_thread);

  // C heap memory allocation/free
  static inline void* calloc(size_t size);
  static inline void free(void* ptr);

  // functions exported through the JVMPI
  static void get_call_trace(JVMPI_CallTrace *trace, jint depth);
  static jlong get_current_thread_cpu_time();
  static JVMPI_RawMonitor raw_monitor_create(char *lock_name);
  static void raw_monitor_enter(JVMPI_RawMonitor lock_id);
  static void raw_monitor_exit(JVMPI_RawMonitor lock_id);
  static void raw_monitor_destroy(JVMPI_RawMonitor lock_id);
  static void raw_monitor_wait(JVMPI_RawMonitor lock_id, jlong ms);
  static void raw_monitor_notify_all(JVMPI_RawMonitor lock_id);
  static void suspend_thread(JNIEnv *env);
  static void suspend_thread_list(jint reqCnt, JNIEnv **reqList, jint *results);
  static void resume_thread(JNIEnv *env);
  static void resume_thread_list(jint reqCnt, JNIEnv **reqList, jint *results);
  static jint get_thread_status(JNIEnv *env);
  static jboolean thread_has_run(JNIEnv *env);
  static void run_gc();
  static void profiler_exit(jint exit_code);
  static jint create_system_thread(char *name, jint priority, jvmpi_void_function_of_void f);
  static jint enable_event(jint event_type, void *arg);
  static jint disable_event(jint event_type, void *arg);
  static jint request_event(jint event_type, void *arg);
  static void set_thread_local_storage(JNIEnv *env, void *ptr);
  static void* get_thread_local_storage(JNIEnv *env);
  static void disable_gc();
  static void enable_gc();
  static jobjectID get_thread_object(JNIEnv *env);
  static jobjectID get_method_class(jmethodID mid);
  static jobject   jobjectID_2_jobject(jobjectID);
  static jobjectID jobject_2_jobjectID(jobject);
  
 public:
  // called from JNI to get the JVMPI interface function table
  static JVMPI_Interface* GetInterface_1(int version);

  // called before VM shutdown
  static void disengage();

  // test if jvmpi is enabled
  static inline bool enabled();
  
  // per event tests
  static inline bool is_event_enabled(jint event_type);
  static inline bool is_event_supported(jint event_type);
  
  // support for (interpreter) code generation
  static inline unsigned int* event_flags_array_at_addr(jint event_type);

  // functions called by other parts of the VM to notify events
  static void post_vm_initialization_events();
  static void post_vm_initialized_event();
  static void post_vm_death_event      ();

  static void post_instruction_start_event(const frame& f);

  static void post_thread_start_event  (JavaThread* thread, jint flag);
  static void post_thread_start_event  (JavaThread* thread);
  static void post_thread_end_event    (JavaThread* thread);

  static void fillin_array_class_load_event  (oop k, JVMPI_Event *eventp);
  static void fillin_class_load_event  (oop k, JVMPI_Event *eventp, bool fillin_jni_ids);
  static void post_class_load_event    (oop k, jint flag);
  static void post_class_load_event    (oop k);
  // ptr to a function that takes an unsigned int param and returns a void *
  typedef void * (*jvmpi_alloc_func_t)(unsigned int bytecnt);
  static void post_class_load_hook_event(unsigned char **ptrP,
    unsigned char **end_ptrP, jvmpi_alloc_func_t malloc_f);
  static void *jvmpi_alloc(unsigned int bytecnt);
  static void post_class_unload_events();
  static void save_class_unload_event_info(oop k);

  static void post_dump_event();

  static void post_new_globalref_event(jobject ref, oop obj, bool post_jvmpi_event);
  static void post_delete_globalref_event(jobject ref, bool post_jvmpi_event);
  static void post_new_weakref_event(jobject ref, oop obj);
  static void post_delete_weakref_event(jobject ref);

  static void post_arena_new_event(int arena_id, const char* arena_name);
  static void post_arena_delete_event(int arena_id);
  static void post_object_alloc_event(oop obj, size_t bytesize, jint arena_id, jint flag);
  static void post_object_free_event(oop obj);
  static void post_object_move_event(oop oldobj, int old_arena, oop newobj, int new_arena);

  static void post_method_entry2_event(methodOop m, oop receiver);
  static void post_method_entry_event(methodOop m);
  static void post_method_exit_event(methodOop m);

  static void post_compiled_method_load_event(compiled_method_t *compiled_method_info);
  static void post_compiled_method_unload_event(methodOop method);

  static void post_monitor_contended_enter_event(void *mid);
  static void post_monitor_contended_entered_event(void *mid);
  static void post_monitor_contended_exit_event(void *mid);

  static void post_monitor_wait_event(oop obj, jlong millis);
  static void post_monitor_waited_event(oop obj, jlong millis);

  static void post_raw_monitor_contended_enter_event(RawMonitor* o);
  static void post_raw_monitor_contended_entered_event(RawMonitor* o);
  static void post_raw_monitor_contended_exit_event(RawMonitor* o);

  static void post_gc_start_event();
  static void post_gc_finish_event(jlong used_obj_space, jlong total_obj_space);

  static void post_trace_instr_event(unsigned char *pc, unsigned char opcode);
  static void post_trace_if_event(unsigned char *pc, int is_true);
  static void post_trace_tableswitch_event(unsigned char *pc, int key, int low, int hi);
  static void post_trace_lookupswitch_event(unsigned char *pc,
                                            int chosen_pair_index,
                                            int pairs_total);

  static void post_object_dump_event(oop obj, int flag);
  static void post_heap_dump_event_in_safepoint(int level, int flag);
  static void post_monitor_dump_event_in_safepoint(int flag);
};
