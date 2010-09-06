#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_win32_ia64.inline.hpp	1.3 03/12/23 16:38:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#pragma warning(disable: 4035) // Disable warning 4035: no return value

// Efficient swapping of data bytes from Java byte
// ordering to native byte ordering and vice versa.
inline u2 Bytes::swap_u2(u2 x) {
#if _M_IA64
  assert(0, "fix swap_u2");
#else
  __asm {
    mov ax, x
    xchg al, ah
  }
  // no return statement needed, result is already in ax
  // compiler warning C4035 disabled via warning pragma
#endif
}

inline u4 Bytes::swap_u4(u4 x) {
#if _M_IA64
  assert(0, "fix swap_u4");
#else
  __asm {
    mov eax, x
    bswap eax
  }
  // no return statement needed, result is already in eax
  // compiler warning C4035 disabled via warning pragma
#endif
}

// Helper function for swap_u8
inline u8 Bytes::swap_u8_base(u4 x, u4 y) {
#if _M_IA64
  assert(0, "fix swap_u8_base");
#else
  __asm {
    mov eax, y
    mov edx, x
    bswap eax
    bswap edx
  }
  // no return statement needed, result is already in edx:eax
  // compiler warning C4035 disabled via warning pragma
#endif
}

#pragma warning(default: 4035) // Enable warning 4035: no return value
