/*
 * @(#)MalformedInputException.java	1.14 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
* The input string or input byte array to a character conversion
* contains a malformed sequence of characters or bytes.
*
* @author Asmus Freytag
*
* @deprecated Replaced by {@link java.nio.charset}.  THIS API WILL BE
* REMOVED IN J2SE 1.6.
*/
@Deprecated
public class MalformedInputException
    extends java.io.CharConversionException
{
    /**
     * Constructs a MalformedInputException with no detail message.
     * A detail message is a String that describes this particular exception.
     */
    public MalformedInputException() {
	super();
    }

    /**
     * Constructs a MalformedInputException with the specified detail message.
     * A detail message is a String that describes this particular exception.
     * @param s the String containing a detail message
     */
    public MalformedInputException(String s) {
	super(s);
    }
}
