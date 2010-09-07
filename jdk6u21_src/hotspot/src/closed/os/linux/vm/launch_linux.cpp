/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <dlfcn.h>

#include "launch.hpp"

dll_func load_dll_and_lookup(char* name) {
  // Load the dll file
  void* handle = dlopen(name,  RTLD_LAZY | RTLD_GLOBAL);
  if (handle == NULL) {
    fprintf(stderr, "Could not load the VM shared object %s: %s\n",
            name, dlerror());
    exit(-1);
  }

  // Find the entry point
  char* func = "vm_main";
  dll_func result = (dll_func) dlsym(handle, func);
  if (result == NULL) {
    // Perhaps the entry point has been statically linked with this executable.
    result = (dll_func) dlsym(dlopen(0, RTLD_LAZY), func);
  }
  if (result == NULL) {
    fprintf(stderr, "Could not find %s in the VM shared object %s: %s\n",
            func, name, dlerror());
    exit(-1);
  }

  return result;
}


dll_func vm_entry_point() {
  char *suffix = ".so.1";
  char *buffer = new char[3 + strlen(vm_library_name) + strlen(suffix) + 1];
  sprintf( buffer, "lib%s%s", vm_library_name, suffix);
  return load_dll_and_lookup(buffer);
}

// Reconciliation History
// launch_solaris.cpp   1.8 99/06/22 16:38:48
// End
