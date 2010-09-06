/*
 * @(#)ByteToCharMacCentralEurope.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.MacCentralEurope;

/**
 * A table to convert to MacCentralEurope to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharMacCentralEurope extends ByteToCharSingleByte {

    private final static MacCentralEurope nioCoder = new MacCentralEurope();

    public String getCharacterEncoding() {
        return "MacCentralEurope";
    }

    public ByteToCharMacCentralEurope() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
