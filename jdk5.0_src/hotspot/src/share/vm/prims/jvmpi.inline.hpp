#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmpi.inline.hpp	1.9 03/12/23 16:43:14 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

inline bool jvmpi::is_event_enabled(jint event_type) {
  return (event_type >= 31
	  ? (enabled() && (_event_flags_array[event_type] == JVMPI_EVENT_ENABLED)) 
	  : (_event_flags & (1 << event_type)));
}


inline void* jvmpi::calloc(size_t size) {
  void* p = os::malloc(size);
  if (p == NULL) {
    vm_exit_out_of_memory(size, NULL);
  }
  memset(p, 0, size);
  return p;
}


inline void jvmpi::free(void* ptr) {
  os::free(ptr);
}


inline void jvmpi::enable_event(jint event_type) {
  if (event_type < 31) {
    _event_flags |= 1 << event_type;
  }
  _event_flags_array[event_type] = JVMPI_EVENT_ENABLED;
}


inline void jvmpi::disable_event(jint event_type) {
  if (event_type < 31) {
    _event_flags &= ~(1 << event_type);
  }
  _event_flags_array[event_type] = JVMPI_EVENT_DISABLED;
}


inline bool jvmpi::enabled() {
  return !!(_event_flags & JVMPI_PROFILING_ON);
}


inline bool jvmpi::is_event_supported(jint event_type) {
  return ((event_type <= JVMPI_MAX_EVENT_TYPE_VAL) &&
	  (_event_flags_array[event_type] != JVMPI_EVENT_NOT_SUPPORTED));
}


inline unsigned int* jvmpi::event_flags_array_at_addr(jint event_type) {
  return &_event_flags_array[event_type];
}
