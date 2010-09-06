/*
 * @(#)ByteToCharCp930.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM930;

/**
* Tables and data to convert Cp930 to Unicode.
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp930
	extends ByteToCharDBCS_EBCDIC

{
	private static IBM930 nioCoder = new IBM930(); 
	// Return the character set id

	public String getCharacterEncoding()
	{
		return "Cp930";
	}

	public ByteToCharCp930() {
		super();
		super.mask1 = 0xFFC0;
		super.mask2 = 0x003F;
		super.shift = 6;
		super.singleByteToChar = nioCoder.getDecoderSingleByteMappings();
		super.index1 = nioCoder.getDecoderIndex1();
		super.index2 = nioCoder.getDecoderIndex2();
	}
}
