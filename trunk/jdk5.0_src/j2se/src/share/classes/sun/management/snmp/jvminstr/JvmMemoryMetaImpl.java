/*
 * @(#)file      JvmMemoryMetaImpl.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.7
 * @(#)lastedit  04/02/25
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
import com.sun.jmx.snmp.SnmpOid;
import com.sun.jmx.snmp.SnmpStatusException;

// jdmk imports
//
import com.sun.jmx.snmp.agent.SnmpMib;
import com.sun.jmx.snmp.agent.SnmpStandardObjectServer;

import sun.management.snmp.jvmmib.JvmMemoryMeta;
import sun.management.snmp.jvmmib.JvmMemManagerTableMeta;
import sun.management.snmp.jvmmib.JvmMemGCTableMeta;
import sun.management.snmp.jvmmib.JvmMemPoolTableMeta;
import sun.management.snmp.jvmmib.JvmMemMgrPoolRelTableMeta;
import sun.management.snmp.util.MibLogger;

/**
 * The class is used for representing SNMP metadata for the "JvmMemory" group.
 */
public class JvmMemoryMetaImpl extends JvmMemoryMeta {
    /**
     * Constructor for the metadata associated to "JvmMemory".
     */
    public JvmMemoryMetaImpl(SnmpMib myMib, SnmpStandardObjectServer objserv) {
	super(myMib,objserv);
    }

    /**
     * Factory method for "JvmMemManagerTable" table metadata class.
     * 
     * You can redefine this method if you need to replace the default
     * generated metadata class with your own customized class.
     * 
     * @param tableName Name of the table object ("JvmMemManagerTable")
     * @param groupName Name of the group to which this table belong 
     *        ("JvmMemory")
     * @param mib The SnmpMib object in which this table is registered
     * @param server MBeanServer for this table entries (may be null)
     * 
     * @return An instance of the metadata class generated for the
     *         "JvmMemManagerTable" table (JvmMemManagerTableMeta)
     * 
     **/
    protected JvmMemManagerTableMeta createJvmMemManagerTableMetaNode(
        String tableName, String groupName, SnmpMib mib, MBeanServer server)  {
        return new JvmMemManagerTableMetaImpl(mib, objectserver);
    }


    /**
     * Factory method for "JvmMemGCTable" table metadata class.
     * 
     * You can redefine this method if you need to replace the default
     * generated metadata class with your own customized class.
     * 
     * @param tableName Name of the table object ("JvmMemGCTable")
     * @param groupName Name of the group to which this table belong 
     *        ("JvmMemory")
     * @param mib The SnmpMib object in which this table is registered
     * @param server MBeanServer for this table entries (may be null)
     * 
     * @return An instance of the metadata class generated for the
     *         "JvmMemGCTable" table (JvmMemGCTableMeta)
     * 
     **/
    protected JvmMemGCTableMeta createJvmMemGCTableMetaNode(String tableName,
		      String groupName, SnmpMib mib, MBeanServer server)  {
        return new JvmMemGCTableMetaImpl(mib, objectserver);
    }


    /**
     * Factory method for "JvmMemPoolTable" table metadata class.
     * 
     * You can redefine this method if you need to replace the default
     * generated metadata class with your own customized class.
     * 
     * @param tableName Name of the table object ("JvmMemPoolTable")
     * @param groupName Name of the group to which this table belong 
     *        ("JvmMemory")
     * @param mib The SnmpMib object in which this table is registered
     * @param server MBeanServer for this table entries (may be null)
     * 
     * @return An instance of the metadata class generated for the
     *         "JvmMemPoolTable" table (JvmMemPoolTableMeta)
     * 
     **/ 
    protected JvmMemPoolTableMeta 
	createJvmMemPoolTableMetaNode(String tableName, String groupName, 
				      SnmpMib mib, MBeanServer server)  {
        return new JvmMemPoolTableMetaImpl(mib, objectserver);
    }

    /**
     * Factory method for "JvmMemMgrPoolRelTable" table metadata class.
     * 
     * You can redefine this method if you need to replace the default
     * generated metadata class with your own customized class.
     * 
     * @param tableName Name of the table object ("JvmMemMgrPoolRelTable")
     * @param groupName Name of the group to which this table belong 
     *        ("JvmMemory")
     * @param mib The SnmpMib object in which this table is registered
     * @param server MBeanServer for this table entries (may be null)
     * 
     * @return An instance of the metadata class generated for the
     *         "JvmMemMgrPoolRelTable" table (JvmMemMgrPoolRelTableMeta)
     * 
     **/ 
    protected JvmMemMgrPoolRelTableMeta 
	createJvmMemMgrPoolRelTableMetaNode(String tableName, 
					    String groupName, 
					    SnmpMib mib, MBeanServer server) {
        return new JvmMemMgrPoolRelTableMetaImpl(mib, objectserver);
    }

}
