#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)hpi.cpp	1.10 03/12/23 16:43:47 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_hpi.cpp.incl"

extern "C" {
  static void unimplemented_panic(const char *fmt, ...) {
    Unimplemented();
  }

  static void unimplemented_monitorRegister(sys_mon_t *mid, char *info_str) {
    Unimplemented();
  }
}

static vm_calls_t callbacks = {
  jio_fprintf,
  unimplemented_panic,
  unimplemented_monitorRegister,
  
  NULL, // jvmpi_monitor_contended_enter,
  NULL, // jvmpi_monitor_contended_entered,
  NULL  // jvmpi_monitor_contended_exit,
};

GetInterfaceFunc        hpi::_get_interface = NULL;
HPI_FileInterface*      hpi::_file          = NULL;
HPI_SocketInterface*    hpi::_socket        = NULL;
HPI_LibraryInterface*   hpi::_library       = NULL;
HPI_SystemInterface*    hpi::_system        = NULL;

jint hpi::initialize()
{
  initialize_get_interface(&callbacks);
  if (_get_interface == NULL)
    return JNI_ERR;
  
  jint result;
  
  result = (*_get_interface)((void **)&_file, "File", 1);
  if (result != 0) {
    if (TraceHPI) tty->print_cr("Can't find HPI_FileInterface");
    return JNI_ERR;
  }
  
  
  result = (*_get_interface)((void **)&_library, "Library", 1);
  if (result != 0) {
    if (TraceHPI) tty->print_cr("Can't find HPI_LibraryInterface");
    return JNI_ERR;
  }
  
  result = (*_get_interface)((void **)&_system, "System", 1);
  if (result != 0) {
    if (TraceHPI) tty->print_cr("Can't find HPI_SystemInterface");
    return JNI_ERR;
  }
  
  return JNI_OK;
}

jint hpi::initialize_socket_library()
{
  if (_get_interface == NULL) {
    if (TraceHPI) {
      tty->print_cr("Fatal HPI error: reached initialize_socket_library with NULL _get_interface");
    }
    return JNI_ERR;
  }
  
  jint result;
  result = (*_get_interface)((void **)&_socket, "Socket", 1);
  if (result != 0) {
    if (TraceHPI) tty->print_cr("Can't find HPI_SocketInterface");
    return JNI_ERR;
  }
  
  return JNI_OK;
}
