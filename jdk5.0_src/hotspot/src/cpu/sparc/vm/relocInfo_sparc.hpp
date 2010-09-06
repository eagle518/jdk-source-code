#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocInfo_sparc.hpp	1.16 03/12/23 16:37:21 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class relocInfo
 private:
  enum {
    // Since SPARC instructions are whole words,
    // the two low-order offset bits can always be discarded.
    offset_unit        =  4,

    // There is no need for format bits; the instructions are
    // sufficiently self-identifying.
    format_width       =  0
  };


//Reconciliation History
// 1.3 97/10/15 15:38:36 relocInfo_i486.hpp
// 1.4 97/12/08 16:01:06 relocInfo_i486.hpp
// 1.5 98/01/23 01:34:55 relocInfo_i486.hpp
// 1.6 98/02/27 15:44:53 relocInfo_i486.hpp
// 1.6 98/03/12 14:47:13 relocInfo_i486.hpp
// 1.8 99/06/22 16:37:50 relocInfo_i486.hpp
// 1.9 99/07/16 11:12:11 relocInfo_i486.hpp
//End
