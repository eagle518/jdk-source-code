/*
 * @(#)ByteToCharCp1383.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.io;

import sun.nio.cs.ext.IBM1383;

/**
* A table to convert Cp1383 to Unicode
*
* @author Malcolm Ayres, assisted by UniMap program
*/
public class ByteToCharCp1383
	extends ByteToCharEUC

{

        private final static IBM1383 nioCoder = new IBM1383();

	// Return the character set id
	public String getCharacterEncoding()
	{
		return "Cp1383";
	}

	public ByteToCharCp1383()
	{
	    // Set the correct mapping table
	    super();
	    super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
	    super.mappingTableG1 = nioCoder.getDecoderMappingTableG1();
	}
}
