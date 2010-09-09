/*
 * @(#)CharToByteUnicodeBigUnmarked.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * Convert arrays containing Unicode characters into arrays of bytes, using
 * big-endian byte order; do not write a byte-order mark before the first
 * converted character.
 *
 * @version 	1.13, 10/03/23
 * @author	Mark Reinhold
 */

public class CharToByteUnicodeBigUnmarked extends CharToByteUnicode {

    public CharToByteUnicodeBigUnmarked () {
	byteOrder = BIG;
	usesMark = false;
    }

}
