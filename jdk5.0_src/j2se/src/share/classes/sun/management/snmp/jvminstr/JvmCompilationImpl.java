/*
 * @(#)file      JvmCompilationImpl.java
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
import java.lang.management.CompilationMXBean;

// jmx imports
//
import javax.management.MBeanServer;
import com.sun.jmx.snmp.SnmpString;
import com.sun.jmx.snmp.SnmpStatusException;

// jdmk imports
//
import com.sun.jmx.snmp.agent.SnmpMib;

import sun.management.snmp.jvmmib.JvmCompilationMBean;
import sun.management.snmp.jvmmib.EnumJvmJITCompilerTimeMonitoring;
import sun.management.snmp.util.MibLogger;

/**
 * The class is used for implementing the "JvmCompilation" group.
 */
public class JvmCompilationImpl implements JvmCompilationMBean {

    /**
     * Variable for storing the value of "JvmJITCompilerTimeMonitoring".
     *
     * "Indicates whether the Java virtual machine supports
     * compilation time monitoring.
     * 
     * See java.management.CompilationMXBean.
     * isCompilationTimeMonitoringSupported()
     * "
     *
     */
    static final EnumJvmJITCompilerTimeMonitoring 
	JvmJITCompilerTimeMonitoringSupported = 
	new EnumJvmJITCompilerTimeMonitoring("supported");
    static final EnumJvmJITCompilerTimeMonitoring 
	JvmJITCompilerTimeMonitoringUnsupported = 
	new EnumJvmJITCompilerTimeMonitoring("unsupported");


    /**
     * Constructor for the "JvmCompilation" group.
     * If the group contains a table, the entries created through an SNMP SET
     * will not be registered in Java DMK.
     */
    public JvmCompilationImpl(SnmpMib myMib) {
    }


    /**
     * Constructor for the "JvmCompilation" group.
     * If the group contains a table, the entries created through an SNMP 
     * SET will be AUTOMATICALLY REGISTERED in Java DMK.
     */
    public JvmCompilationImpl(SnmpMib myMib, MBeanServer server) {
    }
    
    private static CompilationMXBean getCompilationMXBean() {
	return ManagementFactory.getCompilationMXBean();
    }

    /**
     * Getter for the "JvmJITCompilerTimeMonitoring" variable.
     */
    public EnumJvmJITCompilerTimeMonitoring getJvmJITCompilerTimeMonitoring() 
	throws SnmpStatusException {
	
	// If we reach this point, then we can safely assume that
	// getCompilationMXBean() will not return null, because this 
	// object will not be instantiated when there is no compilation
	// system (see JVM_MANAGEMENT_MIB_IMPL).
	//
        if(getCompilationMXBean().isCompilationTimeMonitoringSupported())
	    return JvmJITCompilerTimeMonitoringSupported;
	else
	    return JvmJITCompilerTimeMonitoringUnsupported;
    }

    /**
     * Getter for the "JvmJITCompilerTimeMs" variable.
     */
    public Long getJvmJITCompilerTimeMs() throws SnmpStatusException {
	final long t;
	if(getCompilationMXBean().isCompilationTimeMonitoringSupported())
	    t = getCompilationMXBean().getTotalCompilationTime();
	else 
	    t = 0;
	return new Long(t);
    }

    /**
     * Getter for the "JvmJITCompilerName" variable.
     */
    public String getJvmJITCompilerName() throws SnmpStatusException {
        return JVM_MANAGEMENT_MIB_IMPL.
	    validJavaObjectNameTC(getCompilationMXBean().getName());
    }

}
