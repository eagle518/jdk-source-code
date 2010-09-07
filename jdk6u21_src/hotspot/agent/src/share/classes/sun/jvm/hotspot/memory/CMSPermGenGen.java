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

public class CMSPermGenGen extends ConcurrentMarkSweepGeneration {
   public CMSPermGenGen(Address addr) {
      super(addr);
   }

   public String name() {
      return "concurrent-mark-sweep perm gen";
   }
}
