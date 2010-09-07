/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
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

#include "incls/_precompiled.incl"
#include "incls/_vtune_solaris.cpp.incl"

// empty implementation

void VTune::start_GC() {}
void VTune::end_GC() {}
void VTune::start_class_load() {}
void VTune::end_class_load() {}
void VTune::exit() {}
void VTune::register_stub(const char* name, address start, address end) {}

void VTune::create_nmethod(nmethod* nm) {}
void VTune::delete_nmethod(nmethod* nm) {}

void vtune_init() {}
