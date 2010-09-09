/*
 * @(#)CharToByteJIS0208_Solaris.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */


package sun.io;

import sun.nio.cs.ext.JIS_X_0208_Solaris_Encoder;

/**
 * Tables and data to convert Unicode to JIS0208_Solaris
 *
 *
 * Vendor defined chars added for benefit of vendor defined character
 * supplemented mappings for EUC-JP-Solaris/PCK Solaris variants of EUC-JP
 * and SJIS/Shift_JIS (4765370)
 *
 * @author  ConverterGenerator tool
 */

public class CharToByteJIS0208_Solaris extends CharToByteDoubleByte {

    public String getCharacterEncoding() {
        return "JIS0208_Solaris";
    }

    public CharToByteJIS0208_Solaris() {
        super.index1 = JIS_X_0208_Solaris_Encoder.getIndex1();
        super.index2 = JIS_X_0208_Solaris_Encoder.getIndex2();
    }
}
