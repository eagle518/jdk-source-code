#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)extendedPC_amd64.hpp	1.2 03/12/23 16:35:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 public:
  ExtendedPC(address pc)
  {
    _pc  = pc;
  }

  ExtendedPC()
  {
    _pc  = NULL;
  }
