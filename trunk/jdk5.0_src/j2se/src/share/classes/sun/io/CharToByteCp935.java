/*
 * @(#)CharToByteCp935.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM935;

/**
* Tables and data to convert Unicode to Cp935
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class CharToByteCp935
        extends CharToByteDBCS_EBCDIC

{
	private final static IBM935 nioCoder = new IBM935();

        // Return the character set id
        public String getCharacterEncoding()
        {
                return "Cp935";
        }

        public CharToByteCp935()
        {
                super();
                super.mask1 = 0xFFE0;
                super.mask2 = 0x001F;
                super.shift = 5;
                super.index1 = nioCoder.getEncoderIndex1();
                super.index2 = nioCoder.getEncoderIndex2();
                super.index2a = nioCoder.getEncoderIndex2a();
                subBytes = new byte[1];
                subBytes[0] = 0x6f;
        }
}
