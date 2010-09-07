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

public class X86SegmentRegisterAddress extends IndirectAddress {
   private final X86SegmentRegister segment;
   private final X86Register offset;

   public X86SegmentRegisterAddress(X86SegmentRegister segment, X86Register offset) {
      this.segment = segment;
      this.offset = offset;
   }

   public String toString() {
        StringBuffer buf = new StringBuffer();
        buf.append(getSegment().toString());
        buf.append(":");
        buf.append(getOffset().toString());
        return buf.toString();
   }

   public X86SegmentRegister getSegment() {
      return segment;
   }

   public X86Register getOffset() {
      return offset;
   }
}
