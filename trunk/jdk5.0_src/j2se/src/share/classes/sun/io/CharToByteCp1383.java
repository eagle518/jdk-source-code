/*
 * @(#)CharToByteCp1383.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM1383;

/**
* Tables and data to convert Unicode to Cp1383
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class CharToByteCp1383
        extends CharToByteDBCS_ASCII

{

	private final static IBM1383 nioCoder = new IBM1383();

        // Return the character set id
        public String getCharacterEncoding()
        {
                return "Cp1383";
        }

        public CharToByteCp1383()
        {
                super();
                super.mask1 = 0xFFE0;
                super.mask2 = 0x001F;
                super.shift = 5;
                super.index1 = nioCoder.getEncoderIndex1();
                super.index2 = nioCoder.getEncoderIndex2();
                super.index2a = nioCoder.getEncoderIndex2a();
        }
}
