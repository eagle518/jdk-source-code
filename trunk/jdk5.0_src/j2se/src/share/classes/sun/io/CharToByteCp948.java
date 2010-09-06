/*
 * @(#)CharToByteCp948.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM948;

/**
* Tables and data to convert Unicode to Cp948
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class CharToByteCp948
        extends CharToByteDBCS_ASCII

{
        private static IBM948 nioCoder = new IBM948();

        // Return the character set id
        public String getCharacterEncoding()
        {
                return "Cp948";
        }


        public CharToByteCp948()
        {
                super();
                super.mask1 = 0xFFC0;
                super.mask2 = 0x003F;
                super.shift = 6;
                super.index1 = nioCoder.getEncoderIndex1();
                super.index2 = nioCoder.getEncoderIndex2();
                super.index2a = nioCoder.getEncoderIndex2a();
        }
}
