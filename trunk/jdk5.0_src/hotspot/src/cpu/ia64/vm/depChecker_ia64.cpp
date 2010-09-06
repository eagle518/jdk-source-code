#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)depChecker_ia64.cpp	1.5 03/12/23 16:36:38 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_depChecker_ia64.cpp.incl"

#ifndef PRODUCT

void*    DepChecker::_library              = NULL;
DepChecker::check_func DepChecker::_check  = NULL;
bool DepChecker::load_failed = false;

bool DepChecker::load_library() {
  if (_library == NULL) {
    char buf[1024];
    char ebuf[1024];
    sprintf(buf, "depchecker%s", os::dll_file_extension());
    _library = hpi::dll_load(buf, ebuf, sizeof ebuf);
    if (_library != NULL) {
      tty->print_cr("Loaded Dependency Checker");
      _check = CAST_TO_FN_PTR(DepChecker::check_func, hpi::dll_lookup(_library, "check__FPci"));
    }
  }
  return (_library != NULL) && (_check != NULL);
}



void DepChecker::check(address inst, int length) {
  
  if (!load_library()) {
    // Only Warning load failure once
    if ( !load_failed ) {
      load_failed = true;
      tty->print_cr("Could not load dependency checker library");
    }
    return;
  }

  tty->print_cr("Checking depenency violations at address %p, length %d", inst, length);

  ((check_func)_check) (inst, length);

  tty->print_cr("Returned from dependency checker");
}

#endif // PRODUCT

