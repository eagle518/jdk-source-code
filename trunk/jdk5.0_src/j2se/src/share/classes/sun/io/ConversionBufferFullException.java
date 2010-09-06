/*
 * @(#)ConversionBufferFullException.java	1.13 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * The output buffer for a character conversion is full, but additional
 * input remains to be converted
 *
 * @author Asmus Freytag
 *
 * @deprecated Replaced by {@link java.nio.charset}.  THIS API WILL BE
 * REMOVED IN J2SE 1.6.
 */
@Deprecated
public class ConversionBufferFullException
    extends java.io.CharConversionException
{
    /**
     * Constructs a BufferFullException with no detail message.
     * A detail message is a String that describes this particular exception.
     */
    public ConversionBufferFullException() {
	super();
    }

    /**
     * Constructs a BufferFullException with the specified detail message.
     * A detail message is a String that describes this particular exception.
     * @param s the String containing a detail message
     */
    public ConversionBufferFullException(String s) {
	super(s);
    }
}
