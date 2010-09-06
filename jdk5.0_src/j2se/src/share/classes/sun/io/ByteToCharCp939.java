/*
 * @(#)ByteToCharCp939.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM939;

/**
* Tables and data to convert Cp939 to Unicode.
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp939
	extends ByteToCharDBCS_EBCDIC

{
        private final static IBM939 nioCoder = new IBM939();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp939";
	}

	public ByteToCharCp939() {
		super();
		super.mask1 = 0xFFC0;
		super.mask2 = 0x003F;
		super.shift = 6;
		super.singleByteToChar = nioCoder.getDecoderByteToCharMappings();
		super.index1 = nioCoder.getDecoderIndex1();
		super.index2 = nioCoder.getDecoderIndex2();
	}
}
