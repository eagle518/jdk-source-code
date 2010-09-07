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

package sun.jvm.hotspot.asm.x86;

import sun.jvm.hotspot.asm.*;
import sun.jvm.hotspot.asm.x86.*;

public class X86MemoryIndirectAddress extends IndirectAddress {

   private long value;

   public X86MemoryIndirectAddress(long value) {
      this.value = value;
   }

   public String toString() {
      StringBuffer buf = new StringBuffer();
      buf.append("*");
      buf.append("[");
      buf.append(Long.toHexString(value));
      buf.append(']');
      return buf.toString();
   }
}
