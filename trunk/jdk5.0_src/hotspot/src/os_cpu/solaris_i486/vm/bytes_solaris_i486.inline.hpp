#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_solaris_i486.inline.hpp	1.7 03/12/23 16:38:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// in solaris_i486.il.
extern "C" u2 _raw_swap_u2(u2 x);
extern "C" u4 _raw_swap_u4(u4 x);
extern "C" u8 _raw_swap_u8(u4 x, u4 y);

// Efficient swapping of data bytes from Java byte
// ordering to native byte ordering and vice versa.
inline u2   Bytes::swap_u2(u2 x) {
  return _raw_swap_u2(x);
}

inline u4   Bytes::swap_u4(u4 x) {
  return _raw_swap_u4(x);
}

// Helper function for swap_u8
inline u8   Bytes::swap_u8_base(u4 x, u4 y) {
  return _raw_swap_u8(x, y);
}

