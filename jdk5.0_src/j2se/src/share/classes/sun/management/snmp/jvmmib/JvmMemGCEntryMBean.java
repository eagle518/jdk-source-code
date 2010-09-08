/*
 * @(#)JvmMemGCEntryMBean.java	1.6 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management.snmp.jvmmib;

//
// Generated by mibgen version 5.0 (06/02/03) when compiling JVM-MANAGEMENT-MIB in standard metadata mode.
//


// jmx imports
//
import com.sun.jmx.snmp.SnmpStatusException;

/**
 * This interface is used for representing the remote management interface for the "JvmMemGCEntry" MBean.
 */
public interface JvmMemGCEntryMBean {

    /**
     * Getter for the "JvmMemGCTimeMs" variable.
     */
    public Long getJvmMemGCTimeMs() throws SnmpStatusException;

    /**
     * Getter for the "JvmMemGCCount" variable.
     */
    public Long getJvmMemGCCount() throws SnmpStatusException;

    /**
     * Getter for the "JvmMemManagerIndex" variable.
     */
    public Integer getJvmMemManagerIndex() throws SnmpStatusException;

}