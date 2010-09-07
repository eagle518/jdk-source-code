/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.debugger.dbx;

import sun.jvm.hotspot.debugger.*;

class DbxOopHandle extends DbxAddress implements OopHandle {
  DbxOopHandle(DbxDebugger debugger, long addr) {
    super(debugger, addr);
  }

  public Address    addOffsetTo       (long offset) throws UnsupportedOperationException {
    throw new UnsupportedOperationException("addOffsetTo not applicable to OopHandles (interior object pointers not allowed)");
  }

  public Address    andWithMask(long mask) throws UnsupportedOperationException {
    throw new UnsupportedOperationException("andWithMask not applicable to OopHandles (i.e., anything but C addresses)");
  }

  public Address    orWithMask(long mask) throws UnsupportedOperationException {
    throw new UnsupportedOperationException("orWithMask not applicable to OopHandles (i.e., anything but C addresses)");
  }

  public Address    xorWithMask(long mask) throws UnsupportedOperationException {
    throw new UnsupportedOperationException("xorWithMask not applicable to OopHandles (i.e., anything but C addresses)");
  }
}
