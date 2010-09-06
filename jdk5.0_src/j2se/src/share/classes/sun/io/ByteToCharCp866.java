/*
 * @(#)ByteToCharCp866.java	1.15	03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.io;

import sun.nio.cs.ext.IBM866;

/**
 * A table to convert to Cp866 to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharCp866 extends ByteToCharSingleByte {

    private final static IBM866 nioCoder = new IBM866();

    public String getCharacterEncoding() {
        return "Cp866";
    }

    public ByteToCharCp866() {
        super.byteToCharTable = nioCoder.getDecoderSingleByteMappings();
    }
}
