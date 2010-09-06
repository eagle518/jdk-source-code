/*
 * @(#)file      JvmRTInputArgsEntryImpl.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.8
 * @(#)lastedit  04/02/27
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

import sun.management.snmp.jvmmib.JvmRTInputArgsEntryMBean;

/**
 * The class is used for implementing the "JvmRTInputArgsEntry" group.
 */
public class JvmRTInputArgsEntryImpl implements JvmRTInputArgsEntryMBean, 
						Serializable {

    private final String item;
    private final int index;

    /**
     * Constructor for the "JvmRTInputArgsEntry" group.
     */
    public JvmRTInputArgsEntryImpl(String item, int index) {
	this.item = validArgValueTC(item);
	this.index = index;
    }
    
    private String validArgValueTC(String str) {
	return JVM_MANAGEMENT_MIB_IMPL.validArgValueTC(str);
    }
    
    /**
     * Getter for the "JvmRTInputArgsItem" variable.
     */
    public String getJvmRTInputArgsItem() throws SnmpStatusException {
        return item;
    }

    /**
     * Getter for the "JvmRTInputArgsIndex" variable.
     */
    public Integer getJvmRTInputArgsIndex() throws SnmpStatusException {
        return new Integer(index);
    }

}
