/*
 * @(#)ByteToCharCp933.java	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM933;

/**
* Tables and data to convert Cp933 to Unicode.
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp933
	extends ByteToCharDBCS_EBCDIC

{
	private final static IBM933 nioCoder = new IBM933();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp933";
	}


	public ByteToCharCp933() {
		super();
		super.mask1 = 0xFFF0;
		super.mask2 = 0x000F;
		super.shift = 4;
		super.singleByteToChar = nioCoder.getDecoderSingleByteMappings();
		super.index1 = nioCoder.getDecoderIndex1();
		super.index2 = nioCoder.getDecoderIndex2();
	}
}
