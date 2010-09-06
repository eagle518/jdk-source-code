/*
 * @(#)ByteToCharJohab.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.Johab;

/**
 * Tables and data to convert Johab to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharJohab extends ByteToCharDoubleByte {

    private final static Johab nioCoder = new Johab();

    public String getCharacterEncoding() {
        return "Johab";
    }

    public ByteToCharJohab() {
        super.index1 = nioCoder.getDecoderIndex1();
        super.index2 = nioCoder.getDecoderIndex2();
        start = 0x20;
        end = 0xFE;
    }
}
