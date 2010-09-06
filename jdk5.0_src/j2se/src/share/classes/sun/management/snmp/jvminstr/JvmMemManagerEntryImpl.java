/*
 * @(#)file      JvmMemManagerEntryImpl.java
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

import java.lang.management.MemoryManagerMXBean;

import sun.management.snmp.jvmmib.JvmMemManagerEntryMBean;
import sun.management.snmp.jvmmib.EnumJvmMemManagerState;


/**
 * The class is used for implementing the "JvmMemManagerEntry" group.
 * The group is defined with the following 
 */
public class JvmMemManagerEntryImpl implements JvmMemManagerEntryMBean {
    
    /**
     * Variable for storing the value of "JvmMemManagerIndex".
     *
     * "An index opaquely computed by the agent and which uniquely
     * identifies a Memory Manager."
     *
     */
    protected final int JvmMemManagerIndex;
    
    protected MemoryManagerMXBean manager;
    
    /**
     * Constructor for the "JvmMemManagerEntry" group.
     */
    public JvmMemManagerEntryImpl(MemoryManagerMXBean m, int myindex) {
	manager = m;
	JvmMemManagerIndex = myindex;
    }
    
    /**
     * Getter for the "JvmMemManagerName" variable.
     */
    public String getJvmMemManagerName() throws SnmpStatusException {
        return JVM_MANAGEMENT_MIB_IMPL.
	    validJavaObjectNameTC(manager.getName());
    }

    /**
     * Getter for the "JvmMemManagerIndex" variable.
     */
    public Integer getJvmMemManagerIndex() throws SnmpStatusException {
        return new Integer(JvmMemManagerIndex);
    }

    /**
     * Getter for the "JvmMemManagerState" variable.
     */
    public EnumJvmMemManagerState getJvmMemManagerState() 
	throws SnmpStatusException {
	if (manager.isValid()) 
	    return JvmMemManagerStateValid;
	else
	    return JvmMemManagerStateInvalid;
    }

    private final static EnumJvmMemManagerState JvmMemManagerStateValid = 
	new EnumJvmMemManagerState("valid");
    private final static EnumJvmMemManagerState JvmMemManagerStateInvalid = 
	new EnumJvmMemManagerState("invalid");

}

