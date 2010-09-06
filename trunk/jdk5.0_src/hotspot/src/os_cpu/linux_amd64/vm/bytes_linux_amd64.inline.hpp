#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_linux_amd64.inline.hpp	1.2 03/12/23 16:38:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Efficient swapping of data bytes from Java byte
// ordering to native byte ordering and vice versa.

#include <byteswap.h>

inline u2 Bytes::swap_u2(u2 x) 
{
  return bswap_16(x);
}

inline u4 Bytes::swap_u4(u4 x) 
{
  return bswap_32(x);
}

inline u8 Bytes::swap_u8(u8 x) 
{
  return bswap_64(x);
}
