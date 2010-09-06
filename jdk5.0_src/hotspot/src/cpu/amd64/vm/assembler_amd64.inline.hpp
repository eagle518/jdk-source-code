#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)assembler_amd64.inline.hpp	1.3 03/12/23 16:35:41 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

inline void Assembler::emit_long64(jlong x)
{
  *(jlong*) _code_pos = x;
  _code_pos += sizeof(jlong);
  code()->set_code_end(_code_pos);
}
