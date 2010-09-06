/*
 * @(#)HotspotInternal.java	1.1 04/02/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import javax.management.MBeanRegistration;
import javax.management.MBeanServer;
import javax.management.ObjectName;

/**
 * Implementation class of HotspotInternalMBean interface.
 * 
 * <p> This is designed for internal customer use to create
 * this MBean dynamically from an agent which will then register
 * all internal MBeans to the platform MBeanServer.
 */
public class HotspotInternal 
    implements HotspotInternalMBean, MBeanRegistration {
  
    private MBeanServer server = null;

    /**
     * Default constructor that registers all hotspot internal MBeans
     * to the MBeanServer that creates this MBean.
     */
    public HotspotInternal() {
    }

    public ObjectName preRegister(MBeanServer server,
                                  ObjectName name) throws java.lang.Exception {
        // register all internal MBeans when this MBean is instantiated
        // and to be registered in a MBeanServer.
        ManagementFactory.registerInternalMBeans(server);
        this.server = server;
        return ManagementFactory.getHotspotInternalObjectName();
    }

    public void postRegister(Boolean registrationDone) {};

    public void preDeregister() throws java.lang.Exception {
        // unregister all internal MBeans when this MBean is unregistered.
        ManagementFactory.unregisterInternalMBeans(server);
    }

    public void postDeregister() {};

}
