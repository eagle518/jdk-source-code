#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)orderAccess.cpp	1.2 03/12/23 16:43:58 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_orderAccess.cpp.incl"

volatile intptr_t OrderAccess::dummy = 0;
