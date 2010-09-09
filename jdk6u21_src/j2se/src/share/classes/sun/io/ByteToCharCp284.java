/*
 * @(#)ByteToCharCp284.java	1.18	10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM284;

/**
 * A table to convert to Cp284 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp284 extends ByteToCharSingleByte {

    private final static IBM284 nioCoder = new IBM284();

    public String getCharacterEncoding() {
        return "Cp284";
    }

    public ByteToCharCp284() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
