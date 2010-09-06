/*
 * @(#)ByteToCharCp949C.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.io.*;

public class ByteToCharCp949C extends ByteToCharDBCS_ASCII {
   protected static final String singleByteToChar;
   protected static final boolean leadByte[];
   protected static final short   index1[];
   protected static final String  index2;
   protected static final int     mask1;
   protected static final int     mask2;
   protected static final int     shift;

   static {
      ByteToCharDBCS_ASCII y = new ByteToCharCp949();
      mask1 = y.mask1;
      mask2 = y.mask2;
      shift = y.shift;
      leadByte = y.leadByte;
      index1 = y.index1;
      index2 = y.index2;

      /* Fix converter to pass through 0x00 to 0x7f unchanged to U+0000 to U+007F */
      String indexs = "";
      for (char c = '\0'; c < '\u0080'; ++c) indexs += c;
      singleByteToChar = indexs + y.singleByteToChar.substring(indexs.length());
   }

   public String getCharacterEncoding() {
      return "Cp949C";
   }

   ByteToCharCp949C() {
      super();
      super.mask1 = mask1;
      super.mask2 = mask2;
      super.shift = shift;
      super.leadByte = leadByte;
      super.singleByteToChar = singleByteToChar;
      super.index1 = index1;
      super.index2 = index2;
   }
}
