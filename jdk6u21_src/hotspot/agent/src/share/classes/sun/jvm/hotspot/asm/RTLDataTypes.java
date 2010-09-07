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

public interface RTLDataTypes {

   // HALF = 16 bits, WORD = 32 bits, DWORD = 64 bits and QWORD = 128 bits.

   public static final int RTLDT_SIGNED_BYTE    = 0;
   public static final int RTLDT_UNSIGNED_BYTE  = 1;
   public static final int RTLDT_SIGNED_HALF    = 2;
   public static final int RTLDT_UNSIGNED_HALF  = 3;
   public static final int RTLDT_SIGNED_WORD    = 4;
   public static final int RTLDT_UNSIGNED_WORD  = 5;
   public static final int RTLDT_SIGNED_DWORD   = 6;
   public static final int RTLDT_UNSIGNED_DWORD = 7;
   public static final int RTLDT_SIGNED_QWORD   = 8;
   public static final int RTLDT_UNSIGNED_QWORD = 9;

   // float is 4 bytes, double is 8 bytes, extended double is 10 bytes
   // and quad is 16 bytes.

   public static final int RTLDT_FL_SINGLE     = 10;
   public static final int RTLDT_FL_DOUBLE     = 11;
   public static final int RTLDT_FL_EXT_DOUBLE = 12;
   public static final int RTLDT_FL_QUAD       = 13;

   public static final int RTLDT_STRING        = 14;

   public static final int RTLDT_UNKNOWN       = Integer.MAX_VALUE;
}
