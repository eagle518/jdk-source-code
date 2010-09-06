/*
 * @(#)UnknownCharacterException.java	1.14 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
* A character conversion encountered a character
* that does not have an assigned mapping, and automatic
* substitution is not selected
*
* @author Asmus Freytag
* @deprecated Replaced by {@link java.nio.charset}.  THIS API WILL BE
* REMOVED IN J2SE 1.6.
*/
@Deprecated
public class UnknownCharacterException
    extends java.io.CharConversionException
{
    /**
     * Constructs a UnknownCharacterException with no detail message.
     * A detail message is a String that describes this particular exception.
     */
    public UnknownCharacterException() {
	super();
    }

    /**
     * Constructs a UnknownCharacterException with the specified detail message.
     * A detail message is a String that describes this particular exception.
     * @param s the String containing a detail message
     */
    public UnknownCharacterException(String s) {
	super(s);
    }
}
