/*
 * @(#)PerfStringCounter.java	1.5 04/02/13
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.counter.perf;

import sun.management.counter.*;
import java.nio.ByteBuffer;
import java.io.UnsupportedEncodingException;
import java.nio.ReadOnlyBufferException;

public class PerfStringCounter extends PerfByteArrayCounter
    implements StringCounter {

    PerfStringCounter(String name, Variability v, 
                      int flags, ByteBuffer bb) {
        this(name, v, flags, bb.limit(), bb);
    }

    PerfStringCounter(String name, Variability v, int flags,
                      int maxLength, ByteBuffer bb) {

        super(name, Units.STRING, v, flags, maxLength, bb);
    }

    // override isVector and getVectorLength in ByteArrayCounter
    public boolean isVector() {
        return false;
    }

    public int getVectorLength() {
        return 0;
    }

    public Object getValue() {
        return stringValue();
    }

    public String stringValue() {

        String str = "";
        byte[] b = byteArrayValue();

        if (b == null || b.length <= 1) {
            return str;
        }

        int i;
        for (i = 0; i < b.length && b[i] != (byte)0; i++);

        try {
            // convert byte array to string, using all bytes up to, but
            // not including the first null byte.
            str = new String(b,0,i,"UTF-8");
        }
        catch (UnsupportedEncodingException e) {
            // should never reach here -
            // "UTF-8" is always a known encoding
            str = "ERROR";
        }

        return str;
    }

    /**
     * Serialize as a snapshot object.
     */
    protected Object writeReplace() throws java.io.ObjectStreamException {
        return new StringCounterSnapshot(getName(),
                                         getUnits(),
                                         getVariability(),
                                         getFlags(),
                                         stringValue());
    }

}
