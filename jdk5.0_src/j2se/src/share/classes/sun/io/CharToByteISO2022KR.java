/*
 * @(#)CharToByteISO2022KR.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

public class CharToByteISO2022KR extends CharToByteISO2022
{
    public CharToByteISO2022KR()
    {
	SODesignator = "$)C";
	try {
	    codeConverter = CharToByteConverter.getConverter("KSC5601");
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
        return "ISO2022KR";
    }

}
