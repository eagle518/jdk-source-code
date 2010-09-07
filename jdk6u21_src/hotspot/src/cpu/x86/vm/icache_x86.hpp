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

// On the x86, this is a no-op -- the I-cache is guaranteed to be consistent
// after the next jump, and the VM never modifies instructions directly ahead
// of the instruction fetch path.

// [phh] It's not clear that the above comment is correct, because on an MP
// system where the dcaches are not snooped, only the thread doing the invalidate
// will see the update.  Even in the snooped case, a memory fence would be
// necessary if stores weren't ordered.  Fortunately, they are on all known
// x86 implementations.

class ICache : public AbstractICache {
 public:
#ifdef AMD64
  enum {
    stub_size      = 64, // Size of the icache flush stub in bytes
    line_size      = 32, // Icache line size in bytes
    log2_line_size = 5   // log2(line_size)
  };

  // Use default implementation
#else
  enum {
    stub_size      = 16,                 // Size of the icache flush stub in bytes
    line_size      = BytesPerWord,      // conservative
    log2_line_size = LogBytesPerWord    // log2(line_size)
  };
#endif // AMD64
};
