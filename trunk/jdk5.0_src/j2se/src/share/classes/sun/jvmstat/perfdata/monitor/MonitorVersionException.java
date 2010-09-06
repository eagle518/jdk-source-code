/*
 * @(#)MonitorVersionException.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.perfdata.monitor;

import sun.jvmstat.monitor.MonitorException;

/**
 * Exception thrown when version of the implementation does not
 * match the version of the instrumentation exported by a target
 * Java Virtual Machine.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class MonitorVersionException extends MonitorException {

    /**
     * Create a MonitorVersionException
     */
    public MonitorVersionException() {
        super();
    }

    /**
     * Create a MonitorVersionException with the given message.
     *
     * @param message the message to associate with the exception.
     */
    public MonitorVersionException(String message) {
        super(message);
    }
}
