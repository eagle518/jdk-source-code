/*
 * Copyright (c) 2003, 2005, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.runtime.amd64;

import sun.jvm.hotspot.asm.amd64.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;

public class AMD64RegisterMap extends RegisterMap {

  /** This is the only public constructor */
  public AMD64RegisterMap(JavaThread thread, boolean updateMap) {
    super(thread, updateMap);
  }

  protected AMD64RegisterMap(RegisterMap map) {
    super(map);
  }

  public Object clone() {
    AMD64RegisterMap retval = new AMD64RegisterMap(this);
    return retval;
  }

  // no PD state to clear or copy:
  protected void clearPD() {}
  protected void initializePD() {}
  protected void initializeFromPD(RegisterMap map) {}
  protected Address getLocationPD(VMReg reg) { return null; }
}
