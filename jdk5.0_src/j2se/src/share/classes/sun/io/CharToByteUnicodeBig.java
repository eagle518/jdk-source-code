/*
 * @(#)CharToByteUnicodeBig.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * Convert arrays containing Unicode characters into arrays of bytes, using
 * big-endian byte order.
 *
 * @version 	1.10, 03/12/19
 * @author	Mark Reinhold
 */

public class CharToByteUnicodeBig extends CharToByteUnicode {

    public CharToByteUnicodeBig () {
	byteOrder = BIG;
    }

}
