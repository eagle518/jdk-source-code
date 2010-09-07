/*
 * Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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
#include "incls/_stubRoutines_x86_64.cpp.incl"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.

address StubRoutines::x86::_get_previous_fp_entry = NULL;

address StubRoutines::x86::_verify_mxcsr_entry = NULL;

address StubRoutines::x86::_f2i_fixup = NULL;
address StubRoutines::x86::_f2l_fixup = NULL;
address StubRoutines::x86::_d2i_fixup = NULL;
address StubRoutines::x86::_d2l_fixup = NULL;
address StubRoutines::x86::_float_sign_mask = NULL;
address StubRoutines::x86::_float_sign_flip = NULL;
address StubRoutines::x86::_double_sign_mask = NULL;
address StubRoutines::x86::_double_sign_flip = NULL;
address StubRoutines::x86::_mxcsr_std = NULL;
