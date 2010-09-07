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

package sun.jvm.hotspot.runtime.ia64;

import sun.jvm.hotspot.asm.ia64.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;

public class IA64RegisterMap extends RegisterMap {

  /** This is the only public constructor */
  public IA64RegisterMap(JavaThread thread, boolean updateMap) {
    super(thread, updateMap);
  }

  protected IA64RegisterMap(RegisterMap map) {
    super(map);
  }

  public Object clone() {
    IA64RegisterMap retval = new IA64RegisterMap(this);
    return retval;
  }

  // no PD state to clear or copy:
  protected void clearPD() {}
  protected void initializePD() {}
  protected void initializeFromPD(RegisterMap map) {}
  protected Address getLocationPD(VMReg reg) { return null; }
}
