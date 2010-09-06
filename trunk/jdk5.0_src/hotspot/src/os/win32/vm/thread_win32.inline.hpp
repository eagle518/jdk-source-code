#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_win32.inline.hpp	1.4 04/01/06 14:11:26 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Contains inlined functions for class Thread and ThreadLocalStorage

inline void ThreadLocalStorage::pd_invalidate_all()            { return; }
