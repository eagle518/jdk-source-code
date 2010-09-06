/*
 * @(#)ByteToCharCp970.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM970;

/**
* A table to convert Cp970 to Unicode
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp970
	extends ByteToCharEUC

{
	private final static IBM970 nioCoder = new IBM970();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp970";
	}

	public ByteToCharCp970()
	{
	    // Set the correct mapping table
	    super();
	    super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
	    super.mappingTableG1 = nioCoder.getDecoderMappingTableG1();
	}
}
