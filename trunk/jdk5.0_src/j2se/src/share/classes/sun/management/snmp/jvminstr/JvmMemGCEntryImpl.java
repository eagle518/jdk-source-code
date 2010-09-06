/*
 * @(#)file      JvmMemGCEntryImpl.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.8
 * @(#)lastedit  04/04/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.jvminstr;

// java imports
//
import java.io.Serializable;

// jmx imports
//
import com.sun.jmx.snmp.SnmpStatusException;

// jdmk imports
//
import com.sun.jmx.snmp.agent.SnmpMib;

import java.lang.management.GarbageCollectorMXBean;

import sun.management.snmp.jvmmib.JvmMemGCEntryMBean;
import sun.management.snmp.util.MibLogger;

/**
 * The class is used for implementing the "JvmMemGCEntry" group.
 */
public class JvmMemGCEntryImpl implements JvmMemGCEntryMBean {

    /**
     * Variable for storing the value of "JvmMemManagerIndex".
     *
     * "An index opaquely computed by the agent and which uniquely
     * identifies a Memory Manager."
     *
     */
    protected final int JvmMemManagerIndex;

    protected final GarbageCollectorMXBean gcm;

    /**
     * Constructor for the "JvmMemGCEntry" group.
     */
    public JvmMemGCEntryImpl(GarbageCollectorMXBean gcm, int index) {
	this.gcm=gcm;
	this.JvmMemManagerIndex = index;
    }

    /**
     * Getter for the "JvmMemGCTimeMs" variable.
     */
    // Don't bother to uses the request contextual cache for this.
    public Long getJvmMemGCTimeMs() throws SnmpStatusException {
        return new Long(gcm.getCollectionTime());
    }

    /**
     * Getter for the "JvmMemGCCount" variable.
     */
    // Don't bother to uses the request contextual cache for this.
    public Long getJvmMemGCCount() throws SnmpStatusException {
        return new Long(gcm.getCollectionCount());
    }

    /**
     * Getter for the "JvmMemManagerIndex" variable.
     */
    public Integer getJvmMemManagerIndex() throws SnmpStatusException {
        return new Integer(JvmMemManagerIndex);
    }

}
