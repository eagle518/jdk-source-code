/*
 * @(#)ByteToCharMacUkraine.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacUkraine;

/**
 * A table to convert to MacUkraine to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacUkraine extends ByteToCharSingleByte {

    private final static MacUkraine nioCoder = new MacUkraine();

    public String getCharacterEncoding() {
        return "MacUkraine";
    }

    public ByteToCharMacUkraine() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
