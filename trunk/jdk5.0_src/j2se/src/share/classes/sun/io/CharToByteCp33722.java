/*
 * @(#)CharToByteCp33722.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM33722;

/**
* Tables and data to convert Unicode to Cp33722
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class CharToByteCp33722
	extends CharToByteEUC

{
	private final static IBM33722 nioCoder = new IBM33722();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp33722";
	}

	public int getMaxBytesPerChar() { 
		return 3;
	}

	public CharToByteCp33722()
	{
		super();
		super.mask1 = 0xFFE0;
		super.mask2 = 0x001F;
		super.shift = 5;
		super.index1 = nioCoder.getEncoderIndex1();
		super.index2 = nioCoder.getEncoderIndex2();
		super.index2a = nioCoder.getEncoderIndex2a();
		super.index2b = nioCoder.getEncoderIndex2b();
	}
}
