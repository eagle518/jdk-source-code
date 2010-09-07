/*
 * Copyright (c) 2003, 2006, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007 Red Hat, Inc.
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
#include "incls/_vtableStubs_zero.cpp.incl"

VtableStub* VtableStubs::create_vtable_stub(int vtable_index) {
  ShouldNotCallThis();
}

VtableStub* VtableStubs::create_itable_stub(int vtable_index) {
  ShouldNotCallThis();
}

int VtableStub::pd_code_size_limit(bool is_vtable_stub) {
  ShouldNotCallThis();
}

int VtableStub::pd_code_alignment() {
  ShouldNotCallThis();
}
