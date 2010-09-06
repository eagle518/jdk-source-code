#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)icache_i486.hpp	1.12 04/02/11 18:41:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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
  static void initialize() {}
  static void invalidate_word (address addr) {} // for writes up to 4 bytes
  static void invalidate_range(address start, int nbytes) {}
};
