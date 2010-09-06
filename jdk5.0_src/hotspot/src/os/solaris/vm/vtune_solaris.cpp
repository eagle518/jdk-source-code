#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vtune_solaris.cpp	1.17 04/04/05 13:05:28 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_vtune_solaris.cpp.incl"

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


//Reconciliation History
// 1.7 98/05/15 09:52:12 vtune_win32.cpp
// 1.8 98/11/11 13:22:55 vtune_win32.cpp
// 1.9 98/12/04 17:37:54 vtune_win32.cpp
// 1.12 99/06/28 11:01:49 vtune_win32.cpp
//End
