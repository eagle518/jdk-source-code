/*
 * @(#)file      JvmMemMgrPoolRelEntryImpl.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.7
 * @(#)lastedit  04/02/25
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.jvminstr;

// jmx imports
//
import com.sun.jmx.snmp.SnmpStatusException;

// jdmk imports
//


import sun.management.snmp.jvmmib.JvmMemMgrPoolRelEntryMBean;

/**
 * The class is used for implementing the "JvmMemMgrPoolRelEntry" group.
 */
public class JvmMemMgrPoolRelEntryImpl 
    implements JvmMemMgrPoolRelEntryMBean {

    /**
     * Variable for storing the value of "JvmMemManagerIndex".
     *
     * "An index opaquely computed by the agent and which uniquely
     * identifies a Memory Manager."
     *
     */
    final protected int JvmMemManagerIndex;

    /**
     * Variable for storing the value of "JvmMemPoolIndex".
     *
     * "An index value opaquely computed by the agent which uniquely
     * identifies a row in the jvmMemPoolTable.
     * "
     *
     */
    final protected int JvmMemPoolIndex;
    final protected String mmmName;
    final protected String mpmName;

    /**
     * Constructor for the "JvmMemMgrPoolRelEntry" group.
     */
    public JvmMemMgrPoolRelEntryImpl(String mmmName, 
				     String mpmName, 
				     int mmarc, int mparc) {
	JvmMemManagerIndex = mmarc;
	JvmMemPoolIndex    = mparc;

	this.mmmName = mmmName;
	this.mpmName = mpmName;
    }

    /**
     * Getter for the "JvmMemMgrRelPoolName" variable.
     */
    public String getJvmMemMgrRelPoolName() throws SnmpStatusException {
        return JVM_MANAGEMENT_MIB_IMPL.validJavaObjectNameTC(mpmName);
    }

    /**
     * Getter for the "JvmMemMgrRelManagerName" variable.
     */
    public String getJvmMemMgrRelManagerName() throws SnmpStatusException {
        return JVM_MANAGEMENT_MIB_IMPL.validJavaObjectNameTC(mmmName);
    }

    /**
     * Getter for the "JvmMemManagerIndex" variable.
     */
    public Integer getJvmMemManagerIndex() throws SnmpStatusException {
        return new Integer(JvmMemManagerIndex);
    }

    /**
     * Getter for the "JvmMemPoolIndex" variable.
     */
    public Integer getJvmMemPoolIndex() throws SnmpStatusException {
        return new Integer(JvmMemPoolIndex);
    }

}
