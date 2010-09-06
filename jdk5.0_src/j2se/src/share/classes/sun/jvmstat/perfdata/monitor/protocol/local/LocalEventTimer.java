/*
 * @(#)LocalEventTimer.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.perfdata.monitor.protocol.local;

import sun.jvmstat.monitor.*;
import sun.jvmstat.perfdata.monitor.MonitorStatus;
import java.util.*;
import java.util.regex.*;
import java.io.*;

/**
 * Singleton Timer subclass to run polling tasks that generate events
 * for local Java Virtual Machines..
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class LocalEventTimer extends Timer {
    private static LocalEventTimer instance;   // singleton instance

    /**
     * Creates the singleton LocalEventTimer instance.
     */
    private LocalEventTimer() {
        super(true);
    }

    /**
     * Get the singleton LocalEventTimer instance
     *
     * @return LocalEventTimer - the singleton LocalEventTimer instance
     */
    public static synchronized LocalEventTimer getInstance() {
        if (instance == null) {
            instance = new LocalEventTimer();
        }
        return instance;
    }
}
