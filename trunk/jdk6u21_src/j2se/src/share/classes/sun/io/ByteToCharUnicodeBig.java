/*
 * @(#)ByteToCharUnicodeBig.java	1.13 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;
import java.io.*;


/**
 * Convert byte arrays containing Unicode characters into arrays of actual
 * Unicode characters, assuming a big-endian byte order.
 *
 * @version 	1.13, 10/03/23
 * @author	Mark Reinhold
 */

public class ByteToCharUnicodeBig extends ByteToCharUnicode {

    public ByteToCharUnicodeBig() {
	super(BIG, true);
    }

}
