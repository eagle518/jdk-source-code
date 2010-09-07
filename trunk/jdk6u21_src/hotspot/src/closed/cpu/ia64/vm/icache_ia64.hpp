/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface for updating the instruction cache.  Whenever the VM modifies
// code, part of the processor instruction cache potentially has to be flushed.

class ICache : public AbstractICache {
 public:
  enum {
    stub_size      = 200, // Size of the icache flush stub in bytes
    line_size      = 32,  // Icache line size in bytes
    log2_line_size = 5    // log2(line_size)
  };

  // Use default implementation
};
