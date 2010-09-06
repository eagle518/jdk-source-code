/*
 * @(#)LongMonitor.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.monitor;

/**
 * Interface for Monitoring LongInstrument objects.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 * @see sun.jvmstat.instrument.LongInstrument
 */
public interface LongMonitor extends Monitor {

    /**
     * Get the current value of this LongInstrument object.
     *
     * @return long - the current value of the associated LongInstrument object.
     */
    public long longValue();
}
