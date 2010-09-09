/*
 * @(#)CharToByteUnicodeBig.java	1.12 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;


/**
 * Convert arrays containing Unicode characters into arrays of bytes, using
 * big-endian byte order.
 *
 * @version 	1.12, 10/03/23
 * @author	Mark Reinhold
 */

public class CharToByteUnicodeBig extends CharToByteUnicode {

    public CharToByteUnicodeBig () {
	byteOrder = BIG;
    }

}
