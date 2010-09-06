/*
 * @(#)file      NotificationTarget.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.4
 * @(#)lastedit  03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.jvminstr;

import java.net.InetAddress;

/**
 * Target notification.
 */
public interface NotificationTarget {
    public InetAddress getAddress();
    public int getPort();
    public String getCommunity();
}
