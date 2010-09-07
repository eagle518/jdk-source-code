/*
 * Copyright (c) 2001, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include "toolHelp.hpp"
#include <assert.h>

namespace ToolHelp {

static HMODULE kernelDLL = NULL;

HMODULE loadDLL() {
  if (kernelDLL == NULL) {
    kernelDLL = LoadLibrary("KERNEL32.DLL");
  }

  assert(kernelDLL != NULL);
  return kernelDLL;
}

void unloadDLL() {
  if (kernelDLL != NULL) {
    FreeLibrary(kernelDLL);
    kernelDLL = NULL;
  }
}

} // namespace ToolHelp
