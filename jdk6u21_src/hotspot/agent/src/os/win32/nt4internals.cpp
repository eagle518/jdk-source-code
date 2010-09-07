/*
 * Copyright (c) 2000, 2001, Oracle and/or its affiliates. All rights reserved.
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

#include "nt4internals.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

namespace NT4 {

static HMODULE ntDLL = NULL;

HMODULE loadNTDLL() {
  if (ntDLL == NULL) {
    ntDLL = LoadLibrary("NTDLL.DLL");
  }

  assert(ntDLL != NULL);
  return ntDLL;
}

void unloadNTDLL() {
  if (ntDLL != NULL) {
    FreeLibrary(ntDLL);
    ntDLL = NULL;
  }
}

} // namespace NT4

static HMODULE psapiDLL = NULL;

HMODULE
loadPSAPIDLL() {
  if (psapiDLL == NULL) {
    psapiDLL = LoadLibrary("PSAPI.DLL");
  }

  if (psapiDLL == NULL) {
    fprintf(stderr, "Simple Windows Debug Server requires PSAPI.DLL on Windows NT 4.0.\n");
    fprintf(stderr, "Please install this DLL from the SDK and restart the server.\n");
    exit(1);
  }

  return psapiDLL;
}

void
unloadPSAPIDLL() {
  if (psapiDLL != NULL) {
    FreeLibrary(psapiDLL);
    psapiDLL = NULL;
  }
}
