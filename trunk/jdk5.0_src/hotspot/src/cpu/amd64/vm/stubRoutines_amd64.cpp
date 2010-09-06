#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubRoutines_amd64.cpp	1.4 04/03/15 15:24:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubRoutines_amd64.cpp.incl"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.

address StubRoutines::amd64::_handler_for_unsafe_access_entry  = NULL;

address StubRoutines::amd64::_get_previous_fp_entry = NULL;

address StubRoutines::amd64::_f2i_fixup = NULL;
address StubRoutines::amd64::_f2l_fixup = NULL;
address StubRoutines::amd64::_d2i_fixup = NULL;
address StubRoutines::amd64::_d2l_fixup = NULL;
address StubRoutines::amd64::_float_sign_mask = NULL;
address StubRoutines::amd64::_float_sign_flip = NULL;
address StubRoutines::amd64::_double_sign_mask = NULL;
address StubRoutines::amd64::_double_sign_flip = NULL;
address StubRoutines::amd64::_mxcsr_std = NULL;

