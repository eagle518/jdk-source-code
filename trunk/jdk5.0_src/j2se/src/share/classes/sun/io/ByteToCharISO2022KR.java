/*
 * @(#)ByteToCharISO2022KR.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

public class ByteToCharISO2022KR extends ByteToCharISO2022 
{
    public ByteToCharISO2022KR()
    {
	SODesignator = new String[1];
	SODesignator[0] = "$)C";

	SOConverter = new ByteToCharConverter[1];

	try {
	    SOConverter[0] = ByteToCharConverter.getConverter("KSC5601");
	} catch (Exception e) {};
    }

    // Return the character set id
    public String getCharacterEncoding()
    {
        return "ISO2022KR";
    }
}
