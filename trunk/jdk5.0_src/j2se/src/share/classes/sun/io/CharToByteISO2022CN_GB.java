/*
 * @(#)CharToByteISO2022CN_GB.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

/**
* @author Tom Zhou
*/

public class CharToByteISO2022CN_GB extends CharToByteISO2022
{
    public CharToByteISO2022CN_GB()
    {
	SODesignator = "$)A";
	try {
	    codeConverter = CharToByteConverter.getConverter("GB2312");
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
        return "ISO2022CN_GB";
    }

}
