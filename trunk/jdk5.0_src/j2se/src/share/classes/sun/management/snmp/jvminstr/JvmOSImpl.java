/*
 * @(#)file      JvmOSImpl.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.9
 * @(#)lastedit  04/04/18
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.jvminstr;

// java imports
//
import java.io.Serializable;
import java.lang.management.ManagementFactory;
import java.lang.management.OperatingSystemMXBean;

// jmx imports
//
import javax.management.MBeanServer;
import com.sun.jmx.snmp.SnmpString;
import com.sun.jmx.snmp.SnmpStatusException;

// jdmk imports
//
import com.sun.jmx.snmp.agent.SnmpMib;

import sun.management.snmp.jvmmib.JvmOSMBean;

/**
 * The class is used for implementing the "JvmOS" group.
 */
public class JvmOSImpl implements JvmOSMBean, Serializable {

    /**
     * Constructor for the "JvmOS" group.
     * If the group contains a table, the entries created through an 
     * SNMP SET will not be registered in Java DMK.
     */
    public JvmOSImpl(SnmpMib myMib) {
    }


    /**
     * Constructor for the "JvmOS" group.
     * If the group contains a table, the entries created through an 
     * SNMP SET will be AUTOMATICALLY REGISTERED in Java DMK.
     */
    public JvmOSImpl(SnmpMib myMib, MBeanServer server) {
    }

    static OperatingSystemMXBean getOSMBean() {
	return ManagementFactory.getOperatingSystemMXBean();
    }
    
    private static String validDisplayStringTC(String str) {
	return JVM_MANAGEMENT_MIB_IMPL.validDisplayStringTC(str);
    }
    
    private static String validJavaObjectNameTC(String str) {
	return JVM_MANAGEMENT_MIB_IMPL.validJavaObjectNameTC(str);
    }
    
    /**
     * Getter for the "JvmRTProcessorCount" variable.
     */
    public Integer getJvmOSProcessorCount() throws SnmpStatusException {
        return new Integer(getOSMBean().getAvailableProcessors());

    }

    /**
     * Getter for the "JvmOSVersion" variable.
     */
    public String getJvmOSVersion() throws SnmpStatusException {
        return validDisplayStringTC(getOSMBean().getVersion());
    }

    /**
     * Getter for the "JvmOSArch" variable.
     */
    public String getJvmOSArch() throws SnmpStatusException {
        return validDisplayStringTC(getOSMBean().getArch());
    }

    /**
     * Getter for the "JvmOSName" variable.
     */
    public String getJvmOSName() throws SnmpStatusException {
        return validJavaObjectNameTC(getOSMBean().getName());
    }

}
