#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_ia64.hpp	1.8 03/12/23 16:36:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

const int BytesPerInstWord = 16;

// Kludge to fix bug in gcc for ia64 in converting float to double for denorms
extern double ia64_double_zero;
