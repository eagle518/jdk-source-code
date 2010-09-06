/*
 * @(#)PerfStringMonitor.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.perfdata.monitor;

import sun.jvmstat.monitor.*;
import sun.management.counter.Units;
import sun.management.counter.Variability;
import java.nio.ByteBuffer;
import java.io.UnsupportedEncodingException;

/**
 * Class for monitoring a PerfData String instrument.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class PerfStringMonitor extends PerfByteArrayMonitor
       implements StringMonitor {

    /**
     * Constructor to create a StringMonitor object for the string instrument
     * represented by the data in the given buffer.
     *
     * @param name the name of the string instrument
     * @param v the variability attribute
     * @param supported support level indicator
     * @param bb the buffer containing the string instrument data.
     */
    public PerfStringMonitor(String name, Variability v, boolean supported,
                             ByteBuffer bb) {
        this(name, v, supported, bb, bb.limit());
    }

    /**
     * Constructor to create a StringMonitor object for the string instrument
     * represented by the data in the given buffer.
     *
     * @param name the name of the string instrument
     * @param v the variability attribute
     * @param supported support level indicator
     * @param bb the buffer containing the string instrument data.
     * @param maxLength the maximum length of the string data.
     */
    public PerfStringMonitor(String name, Variability v, boolean supported,
                             ByteBuffer bb, int maxLength) {
        super(name, Units.STRING, v, supported, bb, maxLength);
    }

    /**
     * {@inheritDoc}
     * The object returned contains a String with a copy of the current
     * value of the StringInstrument.
     *
     * @return Object - a copy of the current value of the StringInstrument.
     *                  The return value is guaranteed to be of type String.
     */
    public Object getValue() {
        return stringValue();
    }

    /**
     * Return the current value of the StringInstrument as a String.
     *
     * @return String - a copy of the current value of the StringInstrument.
     */
    public String stringValue() {
        String str = "";
        byte[] b = byteArrayValue();

        // catch null strings
        if ((b == null) || (b.length <= 1) || (b[0] == (byte)0)) {
            return str;
        }

        int i;
        for (i = 0; i < b.length && b[i] != (byte)0; i++);

        try {
            // convert byte array to string, using all bytes up to, but
            // not including the first null byte.
            str = new String(b, 0, i, "UTF-8");

        } catch (UnsupportedEncodingException e) {
            // ignore, "UTF-8" is always a known encoding
        }
        return str;
    }
}
