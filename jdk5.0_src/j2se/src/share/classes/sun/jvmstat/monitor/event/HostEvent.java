/*
 * @(#)HostEvent.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.jvmstat.monitor.event;

import java.util.EventObject;
import sun.jvmstat.monitor.MonitoredHost;

/**
 * Base class for events emitted by a {@link MonitoredHost}.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class HostEvent extends EventObject {

    /**
     * Construct a new HostEvent instance.
     *
     * @param host the MonitoredHost source of the event.
     */
    public HostEvent(MonitoredHost host) {
        super(host);
    }

    /**
     * Return the MonitoredHost source of this event.
     *
     * @return MonitoredHost - the source of this event.
     */
    public MonitoredHost getMonitoredHost() {
        return (MonitoredHost)source;
    }
}
