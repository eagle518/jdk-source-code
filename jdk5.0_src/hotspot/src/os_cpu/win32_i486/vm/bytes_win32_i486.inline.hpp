#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_win32_i486.inline.hpp	1.8 03/12/23 16:38:30 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#pragma warning(disable: 4035) // Disable warning 4035: no return value

// Efficient swapping of data bytes from Java byte
// ordering to native byte ordering and vice versa.
inline u2 Bytes::swap_u2(u2 x) {
  __asm {
    mov ax, x
    xchg al, ah
  }
  // no return statement needed, result is already in ax
  // compiler warning C4035 disabled via warning pragma
}

inline u4 Bytes::swap_u4(u4 x) {
  __asm {
    mov eax, x
    bswap eax
  }
  // no return statement needed, result is already in eax
  // compiler warning C4035 disabled via warning pragma
}

// Helper function for swap_u8
inline u8 Bytes::swap_u8_base(u4 x, u4 y) {
  __asm {
    mov eax, y
    mov edx, x
    bswap eax
    bswap edx
  }
  // no return statement needed, result is already in edx:eax
  // compiler warning C4035 disabled via warning pragma
}

#pragma warning(default: 4035) // Enable warning 4035: no return value
