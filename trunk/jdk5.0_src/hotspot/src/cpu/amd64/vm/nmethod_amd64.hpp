#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nmethod_amd64.hpp	1.4 03/12/23 16:35:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class nmethod
  // Since we only patch a call at the return point of a frame
  // we must find all live activations and evict them.
  static bool evict_all_threads_at_deopt() { return true; }

