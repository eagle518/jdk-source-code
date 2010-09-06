/*
 * @(#)ByteToCharMacDingbat.java	1.14	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacDingbat;

/**
 * A table to convert to MacDingbat to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacDingbat extends ByteToCharSingleByte {

    private final static MacDingbat nioCoder = new MacDingbat();

    public String getCharacterEncoding() {
        return "MacDingbat";
    }

    public ByteToCharMacDingbat() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
