#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nmethod_sparc.hpp	1.11 03/12/23 16:37:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class nmethod
 public:
  static int size_of_exception_handler();

  // Since we effectively nop entire nmethod at deopt one deopt
  // patch is plenty
  static bool evict_all_threads_at_deopt() { return false; }
