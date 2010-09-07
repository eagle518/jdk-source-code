/*
 * Copyright (c) 2006, 2008, Oracle and/or its affiliates. All rights reserved.
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

# include "incls/_precompiled.incl"
# include "incls/_vm_version_linux_sparc.cpp.incl"

static bool detect_niagara() {
  char cpu[128];
  bool rv = false;

  FILE* fp = fopen("/proc/cpuinfo", "r");
  if (fp == NULL) {
    return rv;
  }

  while (!feof(fp)) {
    if (fscanf(fp, "cpu\t\t: %100[^\n]", &cpu) == 1) {
      if (strstr(cpu, "Niagara") != NULL) {
        rv = true;
      }
      break;
    }
  }

  fclose(fp);

  return rv;
}

int VM_Version::platform_features(int features) {
  // Default to generic v9
  features = generic_v9_m;

  if (detect_niagara()) {
    NOT_PRODUCT(if (PrintMiscellaneous && Verbose) tty->print_cr("Detected Linux on Niagara");)
    features = niagara1_m;
  }

  return features;
}
