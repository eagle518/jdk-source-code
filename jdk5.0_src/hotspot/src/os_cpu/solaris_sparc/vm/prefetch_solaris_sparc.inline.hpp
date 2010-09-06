#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)prefetch_solaris_sparc.inline.hpp	1.3 03/12/23 16:38:22 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#if defined(COMPILER2) || defined(_LP64)

// In solaris_sparc.il
extern "C" void _Prefetch_read (void *loc, intx interval);
extern "C" void _Prefetch_write(void *loc, intx interval);

inline void Prefetch::read(void *loc, intx interval) {
  _Prefetch_read(loc, interval);
}

inline void Prefetch::write(void *loc, intx interval) {
  _Prefetch_write(loc, interval);
}

#else

inline void Prefetch::read (void *loc, intx interval) {}
inline void Prefetch::write(void *loc, intx interval) {}

#endif
