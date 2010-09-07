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

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;

public class ParNewGeneration extends DefNewGeneration {
  public ParNewGeneration(Address addr) {
    super(addr);
  }

  public Generation.Name kind() {
    return Generation.Name.PAR_NEW;
  }
}
