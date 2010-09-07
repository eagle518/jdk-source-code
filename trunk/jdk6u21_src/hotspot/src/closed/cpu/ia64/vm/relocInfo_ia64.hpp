/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class relocInfo
 private:
  enum {
    // IA64 instructions are 128bit-aligned.
    offset_unit        =  16,

    // Encodes Instruction slot (at least, may need nore bits)
    format_width       =  2
  };
