/*
 * @(#)CharToByteUnicodeBigUnmarked.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * Convert arrays containing Unicode characters into arrays of bytes, using
 * big-endian byte order; do not write a byte-order mark before the first
 * converted character.
 *
 * @version 	1.11, 03/12/19
 * @author	Mark Reinhold
 */

public class CharToByteUnicodeBigUnmarked extends CharToByteUnicode {

    public CharToByteUnicodeBigUnmarked () {
	byteOrder = BIG;
	usesMark = false;
    }

}
