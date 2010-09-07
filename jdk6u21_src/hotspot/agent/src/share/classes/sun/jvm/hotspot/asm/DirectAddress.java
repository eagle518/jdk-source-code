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

public class DirectAddress extends Address {
   private long value;
   public DirectAddress(long value) {
      this.value = value;
   }

   public long getValue() {
      return value;
   }

   public String toString() {
      return Long.toHexString(value);
   }
}
