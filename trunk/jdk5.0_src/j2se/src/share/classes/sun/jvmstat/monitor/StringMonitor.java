/*
 * @(#)StringMonitor.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.monitor;

/**
 * Interface for Monitoring StringInstrument objects.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 * @see sun.jvmstat.instrument.StringInstrument
 */
public interface StringMonitor extends Monitor {

    /**
     * Get a copy of the current value of the StringInstrument object.
     *
     * @return String - a String object containing a copy of the value of
     *                  the associated StringInstrument.
     */
    public String stringValue();
}
