/*
 * @(#)CharToByteJIS0212_Solaris.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */


package sun.io;

import sun.nio.cs.ext.JIS_X_0212_Solaris_Encoder;

/**
 * Tables and data to convert Unicode to JIS0212_Solaris
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteJIS0212_Solaris extends CharToByteDoubleByte {

    public String getCharacterEncoding() {
        return "JIS0212_Solaris";
    }

    public CharToByteJIS0212_Solaris() {
        super.index1 = JIS_X_0212_Solaris_Encoder.getIndex1();
        super.index2 = JIS_X_0212_Solaris_Encoder.getIndex2();
    }
}
