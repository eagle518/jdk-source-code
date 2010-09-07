/*
 * Copyright (c) 1997, 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
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
