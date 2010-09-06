/*
 * @(#)ByteToCharHKSCS.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)ByteToCharHKSCS.java	1.6    03/12/19
 *
 * DISTRIBUTING Copyright 1995-2001 Sun Microsystems, Inc. All Rights Reserved. 
 *
 * DISTRIBUTING This software is the proprietary information of Sun Microsystems, Inc. 
 * DISTRIBUTING Use is subject to license terms. 
 *
 * CopyrightVersion 1.1_beta
 *
 */

package sun.io;

import sun.nio.cs.ext.HKSCS;

/**
 * Tables and data to convert HKSCS to Unicode
 *
 * @author  ConverterGenerator tool
 * @version >= JDK1.1.6
 */

public class ByteToCharHKSCS extends ByteToCharDoubleByte {

    public String getCharacterEncoding() {
        return "HKSCS";
    }

    public ByteToCharHKSCS() {
        super.index1 = HKSCS.getDecoderIndex1();
        super.index2= HKSCS.getDecoderIndex2();
        start = 0x40;
        end = 0xFE;
    }
}
