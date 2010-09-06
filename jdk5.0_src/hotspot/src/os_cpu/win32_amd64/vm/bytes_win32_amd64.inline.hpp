#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_win32_amd64.inline.hpp	1.3 03/12/23 16:38:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


// Efficient swapping of data bytes from Java byte
// ordering to native byte ordering and vice versa.
inline u2 Bytes::swap_u2(u2 x) {
  address p = (address) &x;
  return  ( (u2(p[0]) << 8 ) | ( u2(p[1])) );
}

inline u4 Bytes::swap_u4(u4 x) {
  address p = (address) &x;
  return ( (u4(p[0]) << 24) | (u4(p[1]) << 16) | (u4(p[2]) << 8) | u4(p[3])) ;
}

// Helper function for swap_u8
inline u8 Bytes::swap_u8(u8 x) {
  address p = (address) &x;
  return ( (u8(p[0]) << 56) | (u8(p[1]) << 48) | (u8(p[2]) << 40) | (u8(p[3]) << 32) |
           (u8(p[4]) << 24) | (u8(p[5]) << 16) | (u8(p[6]) << 8)  | u8(p[7])) ;
}

