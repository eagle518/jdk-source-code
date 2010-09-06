#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_win32.hpp	1.41 04/04/20 16:27:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Win32_OS defines the interface to windows operating systems

class win32 {

 protected:
  static int    _vm_page_size;
  static int    _vm_allocation_granularity;
  static int    _processor_type;
  static int    _processor_level;
  static int    _processor_count;
  static julong _physical_memory;
  static size_t _default_stack_size;
  static bool   _is_nt;  

 public:
  // Windows-specific interface:
  static void   initialize_system_info();
  static void   setmode_streams();  

  // Processor info as provided by NT
  static int processor_type()  { return _processor_type;  }
  // Processor level may not be accurate on non-NT systems
  static int processor_level() {
    assert(is_nt(), "use vm_version instead");
    return _processor_level;
  }
  static int processor_count() { return _processor_count; }
  static julong physical_memory() { return _physical_memory; }

 public:
  // Generic interface:

  // Trace number of created threads
  static          intx  _os_thread_limit;
  static volatile intx  _os_thread_count;

  // Tells whether the platform is NT or Windown95
  static bool is_nt() { return _is_nt; }

  // Tells whether we're running on an MP machine
  static bool is_MP() {
    return processor_count() > 1;
  }

  // Returns the byte size of a virtual memory page
  static int vm_page_size() { return _vm_page_size; }

  // Returns the size in bytes of memory blocks which can be allocated.
  static int vm_allocation_granularity() { return _vm_allocation_granularity; }

  // Read the headers for the executable that started the current process into
  // the structure passed in (see winnt.h).
  static void read_executable_headers(PIMAGE_NT_HEADERS);

  // Default stack size for the current process.
  static size_t default_stack_size() { return _default_stack_size; }

#ifndef _WIN64
  // A wrapper to install a structured exception handler for fast JNI accesors.
  static address fast_jni_accessor_wrapper(BasicType);
#endif
};
