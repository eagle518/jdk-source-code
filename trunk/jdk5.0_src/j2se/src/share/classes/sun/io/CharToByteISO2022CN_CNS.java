/*
 * @(#)CharToByteISO2022CN_CNS.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
* @author Tom Zhou
*/

public class CharToByteISO2022CN_CNS extends CharToByteISO2022
{
    public CharToByteISO2022CN_CNS()
    {
	SODesignator = "$)G";
	SS2Designator = "$*H";
	SS3Designator = "$+I";
	try {
	    codeConverter = CharToByteConverter.getConverter("CNS11643");
	} catch (Exception e) {};
    }
	
    /**
     * returns the maximum number of bytes needed to convert a char
     */
    public int getMaxBytesPerChar()
    {
	return maximumDesignatorLength+4;
    }

    /**
     * Return the character set ID
     */
    public String getCharacterEncoding() 
    {
        return "ISO2022CN_CNS";
    }
}

