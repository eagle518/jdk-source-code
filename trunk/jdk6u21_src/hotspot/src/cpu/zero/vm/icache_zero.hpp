/*
 * Copyright (c) 2003, 2004, Oracle and/or its affiliates. All rights reserved.
 * Copyright 2007, 2009 Red Hat, Inc.
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

// Interface for updating the instruction cache.  Whenever the VM
// modifies code, part of the processor instruction cache potentially
// has to be flushed.  This implementation is empty: Zero never deals
// with code, and LLVM handles cache flushing for Shark.

class ICache : public AbstractICache {
 public:
  static void initialize() {}
  static void invalidate_word(address addr) {}
  static void invalidate_range(address start, int nbytes) {}
};
