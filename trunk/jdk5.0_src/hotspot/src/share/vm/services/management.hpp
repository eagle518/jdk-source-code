#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)management.hpp	1.15 04/06/15 12:17:38 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

class OopClosure;
class ThreadSnapshot;

class Management : public AllStatic {
private:
  static PerfVariable*      _begin_vm_creation_time;
  static PerfVariable*      _end_vm_creation_time;
  static PerfVariable*      _vm_init_done_time;
  static jmmOptionalSupport _optional_support;
  static TimeStamp          _stamp; // Timestamp since vm init done time

  // Management klasses
  static klassOop           _sensor_klass;
  static klassOop           _threadInfo_klass;
  static klassOop           _memoryUsage_klass;
  static klassOop           _memoryPoolMXBean_klass;
  static klassOop           _memoryManagerMXBean_klass;
  static klassOop           _garbageCollectorMXBean_klass;
  static klassOop           _managementFactory_klass;

  static klassOop load_and_initialize_klass(symbolHandle sh, TRAPS);

public:
  static void init();
  static void initialize(TRAPS);
 
  static jlong ticks_to_ms(jlong ticks);
  static jlong timestamp();

  static void  oops_do(OopClosure* f);
  static void* get_jmm_interface(int version);
  static void  get_optional_support(jmmOptionalSupport* support);

  static instanceOop create_thread_info_instance(instanceHandle snapshot_thread, ThreadSnapshot* snapshot, Handle stacktraces, TRAPS);
  static void get_loaded_classes(JavaThread* cur_thread, GrowableArray<KlassHandle>* klass_handle_array);

  static void  record_vm_startup_time(jlong begin, jlong duration);
  static void  record_vm_init_completed() {
    // Initialize the timestamp to get the current time
    _vm_init_done_time->set_value(os::javaTimeMillis());

    // Update the timestamp to the vm init done time
    _stamp.update();
  }

  static jlong vm_init_done_time() {
    return _vm_init_done_time->get_value();
  }
 
  // methods to return a klassOop.
  static klassOop java_lang_management_ThreadInfo_klass(TRAPS);
  static klassOop java_lang_management_MemoryUsage_klass(TRAPS);
  static klassOop java_lang_management_MemoryPoolMXBean_klass(TRAPS);
  static klassOop java_lang_management_MemoryManagerMXBean_klass(TRAPS);
  static klassOop java_lang_management_GarbageCollectorMXBean_klass(TRAPS);
  static klassOop sun_management_Sensor_klass(TRAPS);
  static klassOop sun_management_ManagementFactory_klass(TRAPS);
};

class TraceVmCreationTime : public StackObj {
private:
  TimeStamp _timer;
  jlong     _begin_time;

public:
  TraceVmCreationTime() {}
  ~TraceVmCreationTime() {}

  void start() 
  { _timer.update_to(0); _begin_time = os::javaTimeMillis(); }

  /**
   * Only call this if initialization completes successfully; it will   
   * crash if PerfMemory_exit() has already been called (usually by
   * os::shutdown() when there was an initialization failure).
   */
  void end()  
  { Management::record_vm_startup_time(_begin_time, _timer.milliseconds()); }

};
