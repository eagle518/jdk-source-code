/*
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.asm;

// address is calculated as (base + (index * scale) + displacement)
// optionally index is auto incremented or decremented

public abstract class BaseIndexScaleDispAddress extends IndirectAddress {
   private final Register base, index;
   private final int      scale;
   private final long     disp;
   private boolean  isAutoIncr;
   private boolean  isAutoDecr;

   public BaseIndexScaleDispAddress(Register base, Register index, long disp, int scale) {
      this.base = base;
      this.index = index;
      this.disp = disp;
      this.scale = scale;
   }

   public BaseIndexScaleDispAddress(Register base, Register index, long disp) {
      this(base, index, disp, 1);
   }

   public BaseIndexScaleDispAddress(Register base, Register index) {
      this(base, index, 0L, 1);
   }

   public BaseIndexScaleDispAddress(Register base, long disp) {
      this(base, null, disp, 1);
   }

   public Register getBase() {
      return base;
   }

   public Register getIndex() {
      return index;
   }

   public int      getScale() {
      return scale;
   }

   public long     getDisplacement() {
      return disp;
   }

   // is the index auto decremented or incremented?
   public boolean  isAutoIncrement() {
      return isAutoIncr;
   }

   public void setAutoIncrement(boolean value) {
      isAutoIncr = value;
   }

   public boolean  isAutoDecrement() {
      return isAutoDecr;
   }

   public void setAutoDecrement(boolean value) {
      isAutoDecr = value;
   }
}
