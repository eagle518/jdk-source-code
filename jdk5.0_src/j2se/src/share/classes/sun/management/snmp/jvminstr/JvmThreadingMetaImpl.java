/*
 * @(#)file      JvmThreadingMetaImpl.java
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
import javax.management.MBeanServer;
import com.sun.jmx.snmp.SnmpCounter;
import com.sun.jmx.snmp.SnmpCounter64;
import com.sun.jmx.snmp.SnmpGauge;
import com.sun.jmx.snmp.SnmpInt;
import com.sun.jmx.snmp.SnmpUnsignedInt;
import com.sun.jmx.snmp.SnmpIpAddress;
import com.sun.jmx.snmp.SnmpTimeticks;
import com.sun.jmx.snmp.SnmpOpaque;
import com.sun.jmx.snmp.SnmpString;
import com.sun.jmx.snmp.SnmpStringFixed;
import com.sun.jmx.snmp.SnmpOid;
import com.sun.jmx.snmp.SnmpNull;
import com.sun.jmx.snmp.SnmpValue;
import com.sun.jmx.snmp.SnmpVarBind;
import com.sun.jmx.snmp.SnmpStatusException;

// jdmk imports
//
import com.sun.jmx.snmp.agent.SnmpMib;
import com.sun.jmx.snmp.agent.SnmpMibGroup;
import com.sun.jmx.snmp.agent.SnmpStandardObjectServer;
import com.sun.jmx.snmp.agent.SnmpStandardMetaServer;
import com.sun.jmx.snmp.agent.SnmpMibSubRequest;
import com.sun.jmx.snmp.agent.SnmpMibTable;
import com.sun.jmx.snmp.EnumRowStatus;

import sun.management.snmp.jvmmib.JvmThreadingMeta;
import sun.management.snmp.jvmmib.JvmThreadInstanceTableMeta;

/**
 * The class is used for representing SNMP metadata for the "JvmThreading" 
 * group.
 */
public class JvmThreadingMetaImpl extends JvmThreadingMeta {

    /**
     * Constructor for the metadata associated to "JvmThreading".
     */
    public JvmThreadingMetaImpl(SnmpMib myMib, 
				SnmpStandardObjectServer objserv) {
        super(myMib, objserv);
    }

    /**
     * Factory method for "JvmThreadInstanceTable" table metadata class.
     * 
     * You can redefine this method if you need to replace the default
     * generated metadata class with your own customized class.
     * 
     * @param tableName Name of the table object ("JvmThreadInstanceTable")
     * @param groupName Name of the group to which this table belong 
     *        ("JvmThreading")
     * @param mib The SnmpMib object in which this table is registered
     * @param server MBeanServer for this table entries (may be null)
     * 
     * @return An instance of the metadata class generated for the
     *         "JvmThreadInstanceTable" table (JvmThreadInstanceTableMeta)
     * 
     **/
    protected JvmThreadInstanceTableMeta 
	createJvmThreadInstanceTableMetaNode(String tableName, 
					     String groupName, 
					     SnmpMib mib, 
					     MBeanServer server)  {
        return new JvmThreadInstanceTableMetaImpl(mib, objectserver);
    }
}
