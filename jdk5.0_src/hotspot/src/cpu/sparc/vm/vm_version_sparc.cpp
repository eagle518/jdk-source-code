#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vm_version_sparc.cpp	1.40 03/12/23 16:37:26 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_vm_version_sparc.cpp.incl"

# include <sys/systeminfo.h>

int VM_Version::_features = VM_Version::unknown_m;
const char* VM_Version::_features_str = "";

void VM_Version_init() {
  VM_Version::initialize();
}

void VM_Version::initialize() {
  _features = determine_features();
  PrefetchCopyIntervalInBytes = prefetch_copy_interval_in_bytes(); 
  PrefetchScanIntervalInBytes = prefetch_scan_interval_in_bytes();
  PrefetchFieldsAhead         = prefetch_fields_ahead();
  _supports_cx8               = has_v9();

  char buf[256];
  jio_snprintf(buf, sizeof(buf), "%s%s%s%s%s%s%s",
               (has_v8() ? ", has_v8" : ""),
               (has_v9() ? ", has_v9" : ""),
               (has_vis1() ? ", has_vis1" : ""),
               (has_vis2() ? ", has_vis2" : ""),
               (is_ultra3() ? ", is_ultra3" : ""),
               (!has_hardware_int_muldiv ? ", no-muldiv" : ""),
               (!has_hardware_fsmuld() ? ", no-fsmuld" : ""));

  // buf is started with ", " or is empty
  _features_str = strdup(strlen(buf) > 2 ? buf + 2 : buf);

  NOT_PRODUCT( if (PrintMiscellaneous && Verbose) print_features(); );
}

void VM_Version::print_features() {
  tty->print_cr("Version:%s", cpu_features());
}

int VM_Version::determine_features() {

  if (UseV8InstrsOnly) {
    NOT_PRODUCT(if (PrintMiscellaneous && Verbose) tty->print_cr("Version is Forced-V8");)
    return generic_v8_m;
  } 

  // We determine what sort of hardware we have via sysinfo(SI_ISALIST, ...).
  // This isn't the best of all possible ways because there's not enough
  // detail in the isa list it returns, but it's a bit less arcane than
  // generating assembly code and an illegal instruction handler.  We used
  // to generate a getpsr trap, but that's even more arcane.
  //
  // Another possibility would be to use sysinfo(SI_PLATFORM, ...), but
  // that would require more knowledge here than is wise.

  char   tmp;
  size_t bufsize = sysinfo(SI_ISALIST, &tmp, 1);
  char  *buf     = (char *)malloc(bufsize);
  int    features= unknown_m;

  // isalist spec via 'man isalist' as of 01-Aug-2001

  if (buf != NULL) {
    if (sysinfo(SI_ISALIST, buf, bufsize) == bufsize) {

      // Figure out what kind of sparc we have
      char *sparc = strstr(buf, "sparc");
      if (sparc != NULL) {            features |= v8_instructions_m;
        if (sparc[5] == 'v') {
	  if (sparc[6] == '8') {
            if (sparc[7] == '-')      features |= hardware_int_muldiv_m;
            else if (sparc[7] == 'p') features |= generic_v9_m;
            else                      features |= generic_v8_m;
          } else if (sparc[6] == '9') features |= generic_v9_m;
        }
      }

      // Check for visualization instructions
      char *vis = strstr(buf, "vis");
      if (vis != NULL) {              features |= vis1_instructions_m;
        if (vis[3] == '2')            features |= vis2_instructions_m;
      }
    }
    free(buf);
  }

  if (features == unknown_m) {
    features = generic_v9_m;
    warning("Cannot recognize SPARC version. Default to V9");
  }

  return features;
}

static int saved_features = 0;

void VM_Version::allow_all() {
  saved_features = _features;
  _features     = all_features_m;
}

void VM_Version::revert() {
  _features = saved_features;
}
