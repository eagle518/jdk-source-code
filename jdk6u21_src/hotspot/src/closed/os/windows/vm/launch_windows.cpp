/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <windows.h>
#include <stdio.h>

#include "launch.hpp"


static HINSTANCE load_dll_library(char* name) {
  HINSTANCE lib = LoadLibrary(name);
  if (lib == NULL) {
    int error = GetLastError();
    if (error == ERROR_MOD_NOT_FOUND) return NULL;
    printf("Error code %d\n", error);
    exit(-1);
  }
  return lib;
}

dll_func load_dll_and_lookup(char* name) {
  char buffer[MAX_PATH+1];
  HINSTANCE lib = NULL;

  // Get location of <java-home>\bin directory
  if (GetModuleFileName(NULL, buffer, sizeof(buffer)) != NULL) {
    char* p = strrchr(buffer, '\\');
    if (p != NULL) *(p+1) = '\0';
    strcat(buffer, name);
    // First try bin directory
    lib = load_dll_library(buffer);
    // Then try standard OS path
    if (lib == NULL) {
      lib = load_dll_library(name);
    }
  }
  if (lib == NULL) {
    fprintf(stderr, "Could not load VM library %s\n", name);
    exit(-1);
  }
  // Find the entry point
  dll_func result = (dll_func) GetProcAddress(lib, "vm_main");
  if (result == NULL) {
    fprintf(stderr, "Could not find %s in %s\n", "vm_main", name);
    exit(-1);
  }
  return result;
}


dll_func vm_entry_point() {
  const int buffer_length = MAX_PATH;
  char buffer[MAX_PATH];
  sprintf(buffer, "%s.dll", vm_library_name);
  return load_dll_and_lookup(buffer);
}
