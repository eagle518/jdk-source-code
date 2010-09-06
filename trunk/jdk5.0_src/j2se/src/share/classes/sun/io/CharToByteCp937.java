/*
 * @(#)CharToByteCp937.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM937;

/**
* Tables and data to convert Unicode to Cp937
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class CharToByteCp937
        extends CharToByteDBCS_EBCDIC

{

        // Return the character set id
        public String getCharacterEncoding()
        {
                return "Cp937";
        }

        private short index1[];
        private String index2;
        private String index2a;
	private static final IBM937 nioCoder = new IBM937();

        public CharToByteCp937()
        {
                super();
                super.mask1 = 0xFFC0;
                super.mask2 = 0x003F;
                super.shift = 6;
                super.index1 = nioCoder.getEncoderIndex1();
                super.index2 = nioCoder.getEncoderIndex2();
                super.index2a = nioCoder.getEncoderIndex2a();
                subBytes = new byte[1];
                subBytes[0] = 0x6f;
        }
}
