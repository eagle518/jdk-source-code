/*
 * @(#)ByteToCharCp1381.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM1381;

/**
* Tables and data to convert Cp1381 to Unicode.
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp1381
	extends ByteToCharDBCS_ASCII

{

	private final static IBM1381 nioCoder = new IBM1381();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp1381";
	}


	private static final boolean leadByte[] = {
		false, false, false, false, false, false, false, false,  // 00 - 07
		false, false, false, false, false, false, false, false,  // 08 - 0F
		false, false, false, false, false, false, false, false,  // 10 - 17
		false, false, false, false, false, false, false, false,  // 18 - 1F
		false, false, false, false, false, false, false, false,  // 20 - 27
		false, false, false, false, false, false, false, false,  // 28 - 2F
		false, false, false, false, false, false, false, false,  // 30 - 37
		false, false, false, false, false, false, false, false,  // 38 - 3F
		false, false, false, false, false, false, false, false,  // 40 - 47
		false, false, false, false, false, false, false, false,  // 48 - 4F
		false, false, false, false, false, false, false, false,  // 50 - 57
		false, false, false, false, false, false, false, false,  // 58 - 5F
		false, false, false, false, false, false, false, false,  // 60 - 67
		false, false, false, false, false, false, false, false,  // 68 - 6F
		false, false, false, false, false, false, false, false,  // 70 - 77
		false, false, false, false, false, false, false, false,  // 78 - 7F
		false, false, false, false, false, false, false, false,  // 80 - 87
		false, false, false, false, true,  true,  true,  true,   // 88 - 8F
		true,  true,  true,  true,  true,  true,  true,  true,   // 90 - 97
		true,  true,  true,  true,  true,  true,  true,  true,   // 98 - 9F
		true,  true,  true,  true,  true,  true,  true,  true,   // A0 - A7
		true,  true,  false, false, false, false, false, false,  // A8 - AF
		true,  true,  true,  true,  true,  true,  true,  true,   // B0 - B7
		true,  true,  true,  true,  true,  true,  true,  true,   // B8 - BF
		true,  true,  true,  true,  true,  true,  true,  true,   // C0 - C7
		true,  true,  true,  true,  true,  true,  true,  true,   // C8 - CF
		true,  true,  true,  true,  true,  true,  true,  true,   // D0 - D7
		true,  true,  true,  true,  true,  true,  true,  true,   // D8 - DF
		true,  true,  true,  true,  true,  true,  true,  true,   // E0 - E7
		true,  true,  true,  true,  true,  true,  true,  true,   // E8 - EF
		true,  true,  true,  true,  true,  true,  true,  true,   // F0 - F7
		false, false, false, false, false, false, false, false,  // F8 - FF
	};

	public ByteToCharCp1381() {
		super();
		super.mask1 = 0xFFE0;
		super.mask2 = 0x001F;
		super.shift = 5;
		super.leadByte = this.leadByte;
		super.singleByteToChar = nioCoder.getDecoderSingleByteMappings();
		super.index1 = nioCoder.getDecoderIndex1();
		super.index2 = nioCoder.getDecoderIndex2();
	}
}
