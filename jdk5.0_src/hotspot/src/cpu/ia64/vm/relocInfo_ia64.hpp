#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocInfo_ia64.hpp	1.4 03/12/23 16:36:49 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class relocInfo
 private:
  enum {
    // IA64 instructions are 128bit-aligned.
    offset_unit        =  16,

    // Encodes Instruction slot (at least, may need nore bits)
    format_width       =  2
  };
