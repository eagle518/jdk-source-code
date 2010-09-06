#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vtune_linux.cpp	1.5 03/12/23 16:37:37 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_vtune_linux.cpp.incl"

// empty implementation

void VTune::start_GC() {}
void VTune::end_GC() {}
void VTune::start_class_load() {}
void VTune::end_class_load() {}
void VTune::exit() {}
void VTune::register_stub(const char* name, address start, address end) {}

#ifndef CORE
void VTune::create_nmethod(nmethod* nm) {}
void VTune::delete_nmethod(nmethod* nm) {}
#endif

void vtune_init() {}


// Reconciliation History
// vtune_solaris.cpp	1.8 99/07/12 23:54:21
// End
