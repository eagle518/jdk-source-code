/*
 * @(#)CharToByteISO2022CN_CNS.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

