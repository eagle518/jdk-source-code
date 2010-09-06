/*
 * @(#)ByteToCharMS932DB.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

import sun.nio.cs.ext.MS932DB;

/**
 * Tables and data to convert MS932 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

abstract class ByteToCharMS932DB extends ByteToCharDoubleByte {

    public ByteToCharMS932DB() {
        super.index1 = MS932DB.Decoder.index1;
        super.index2 = MS932DB.Decoder.index2;
        start = 0x40;
        end = 0xFC;
    }
}
