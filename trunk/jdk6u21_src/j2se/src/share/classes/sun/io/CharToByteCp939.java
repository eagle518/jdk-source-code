/*
 * @(#)CharToByteCp939.java	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM939;

/**
* Tables and data to convert Unicode to Cp939
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class CharToByteCp939
        extends CharToByteDBCS_EBCDIC

{

	private final static IBM939 nioCoder = new IBM939();

        // Return the character set id
        public String getCharacterEncoding()
        {
                return "Cp939";
        }

        public CharToByteCp939()
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
