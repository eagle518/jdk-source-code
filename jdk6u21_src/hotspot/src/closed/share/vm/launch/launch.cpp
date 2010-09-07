/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>

# ifdef _WIN32
# include <windows.h>
# endif

#include "launch.hpp"

#if defined(_DEBUG) || defined(ASSERT)
  char* vm_library_name = "jvm";
#else
  #ifdef PRODUCT
    char* vm_library_name = "jvm_o";
  #else
    char* vm_library_name = "jvm";
  #endif
#endif

void main(int argc, char* argv[]) {
  dll_func result = vm_entry_point();
  result(argc, argv);
}
