/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.debugger.cdbg.basic.amd64;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;
import sun.jvm.hotspot.debugger.cdbg.basic.*;

/** Basic AMD64 frame functionality providing sender() functionality. */

public class AMD64CFrame extends BasicCFrame {
  private Address rbp;
  private Address pc;

  private static final int ADDRESS_SIZE = 8;

  /** Constructor for topmost frame */
  public AMD64CFrame(CDebugger dbg, Address rbp, Address pc) {
    super(dbg);
    this.rbp = rbp;
    this.pc  = pc;
  }

  public CFrame sender() {
    if (rbp == null) {
      return null;
    }

    Address nextRBP = rbp.getAddressAt( 0 * ADDRESS_SIZE);
    if (nextRBP == null) {
      return null;
    }
    Address nextPC  = rbp.getAddressAt( 1 * ADDRESS_SIZE);
    if (nextPC == null) {
      return null;
    }
    return new AMD64CFrame(dbg(), nextRBP, nextPC);
  }

  public Address pc() {
    return pc;
  }

  public Address localVariableBase() {
    return rbp;
  }
}
