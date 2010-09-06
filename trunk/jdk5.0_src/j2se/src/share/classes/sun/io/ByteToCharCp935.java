/*
 * @(#)ByteToCharCp935.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM935;

/**
* Tables and data to convert Cp935 to Unicode.
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp935
	extends ByteToCharDBCS_EBCDIC

{
	private static IBM935 nioCoder = new IBM935();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp935";
	}


	public ByteToCharCp935() {
		super();
		super.mask1 = 0xFFC0;
		super.mask2 = 0x003F;
		super.shift = 6;
		super.singleByteToChar = nioCoder.getDecoderByteToCharMappings();
		super.index1 = nioCoder.getDecoderIndex1();
		super.index2 = nioCoder.getDecoderIndex2();
	}
}
