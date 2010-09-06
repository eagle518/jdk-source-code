#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)icache_sparc.hpp	1.11 04/02/12 13:00:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface for updating the instruction cache.  Whenever the VM modifies
// code, part of the processor instruction cache potentially has to be flushed.


class ICache : public AbstractICache {
 public:
  enum {
    stub_size      = 160, // Size of the icache flush stub in bytes
    line_size      = 8,   // flush instruction affects a dword
    log2_line_size = 3    // log2(line_size)
  };

  // Use default implementation
};
