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

import sun.jvm.hotspot.asm.Address;
import sun.jvm.hotspot.asm.BaseIndexScaleDispAddress;

public class X86RegisterIndirectAddress extends BaseIndexScaleDispAddress {

   final private X86SegmentRegister segReg;

   public X86RegisterIndirectAddress(X86SegmentRegister segReg, X86Register base, X86Register index, long disp, int scale) {
      super(base, index, disp, scale);
      this.segReg = segReg;
   }

   public X86RegisterIndirectAddress(X86SegmentRegister segReg, X86Register base, X86Register index, long disp) {
      super(base, index, disp, -1);
      this.segReg = segReg;
   }

   public String toString() {
      StringBuffer buf = new StringBuffer();
      if(segReg != null) {
         buf.append(segReg.toString());
         buf.append(":");
      }

      long disp = getDisplacement();
      if(disp != 0)
          buf.append(disp);

      sun.jvm.hotspot.asm.Register base = getBase();
      sun.jvm.hotspot.asm.Register index = getIndex();
      int scaleVal = getScale();
      scaleVal = 1 << scaleVal;

      if( (base != null) || (index != null) || (scaleVal > 1) )
         buf.append('[');

      if(base != null) {
         buf.append(base.toString());
         if(index != null) {
            buf.append("+");
            buf.append(index.toString());
         }
      }
      else {
         if(index != null) {
            buf.append(index.toString());
         }
      }

      if (scaleVal > 1) {
         buf.append(" * ");
         buf.append(Integer.toString(scaleVal));
      }

      if( (base != null) || (index != null) || (scaleVal > 1) )
         buf.append(']');

      return buf.toString();
   }
}
