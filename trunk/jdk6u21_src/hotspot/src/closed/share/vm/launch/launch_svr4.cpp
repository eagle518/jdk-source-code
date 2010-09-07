/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <windows.h>
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
    fprintf(stderr, "Could not find %s in the VM shared object %s: %s\n",
            func, name, dlerror());
    exit(-1);
  }

  return result;
}


dll_func vm_entry_point() {
  // 1. Check environment variable.
  char* environment_contents = getenv( environment_key);
  if ( environment_contents != NULL) {
    return load_dll_and_lookup(environment_contents);
  // 2. Check path.
  char *suffix = ".so.1";
  char *buffer = new char[strlen(default_name) + strlen(suffix) + 1];
  sprintf( buffer, "%s%s", default_name, suffix);
  return load_dll_and_lookup(buffer);
}
