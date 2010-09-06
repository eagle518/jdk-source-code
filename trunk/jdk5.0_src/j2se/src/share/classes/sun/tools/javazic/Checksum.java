/*
 * @(#)Checksum.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

import java.util.zip.CRC32;

/**
 * Checksum provides methods for calculating a CRC32 value for a
 * transitions table.
 *
 * @since 1.4
 */
public class Checksum extends CRC32
{
    /**
     * Updates the CRC32 value from each byte of the given int
     * value. The bytes are used in the big endian order.
     * @param val the int value
     */
    public void update(int val) {
	byte[] b = new byte[4];
	b[0] = (byte)((val >>> 24) & 0xff);
	b[1] = (byte)((val >>> 16) & 0xff);
	b[2] = (byte)((val >>> 8) & 0xff);
	b[3] = (byte)(val & 0xff);
	update(b);
    }

    /**
     * Updates the CRC32 value from each byte of the given long
     * value. The bytes are used in the big endian order.
     * @param val the long value
     */
    void update(long val) {
	byte[] b = new byte[8];
	b[0] = (byte)((val >>> 56) & 0xff);
	b[1] = (byte)((val >>> 48) & 0xff);
	b[2] = (byte)((val >>> 40) & 0xff);
	b[3] = (byte)((val >>> 32) & 0xff);
	b[4] = (byte)((val >>> 24) & 0xff);
	b[5] = (byte)((val >>> 16) & 0xff);
	b[6] = (byte)((val >>> 8) & 0xff);
	b[7] = (byte)(val & 0xff);
	update(b);
    }
}
