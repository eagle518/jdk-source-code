#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubRoutines_i486.cpp	1.60 03/12/23 16:36:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubRoutines_i486.cpp.incl"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.

address StubRoutines::i486::_handler_for_unsafe_access_entry                 = NULL;

address StubRoutines::i486::_get_previous_fp_entry                           = NULL;
