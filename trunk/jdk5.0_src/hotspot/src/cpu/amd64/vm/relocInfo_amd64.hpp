#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocInfo_amd64.hpp	1.2 03/12/23 16:35:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class relocInfo
 private:
  enum 
  {
    // Instructions are byte-aligned.
    offset_unit        =  1,

    // Encodes Assembler::disp32_operand vs. Assembler::imm64_operand.
    format_width       =  1
  };
