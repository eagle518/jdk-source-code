/*
 * @(#)file      SnmpTableHandler.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.6
 * @(#)lastedit  03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.util;

import com.sun.jmx.snmp.SnmpOid;

/**
 * Defines the interface implemented by an object that holds
 * table data.
 **/
public interface SnmpTableHandler {

    /**
     * Returns the data associated with the given index.
     * If the given index is not found, null is returned.
     * Note that returning null does not necessarily means that
     * the index was not found.
     **/
    public Object  getData(SnmpOid index);

    /**
     * Returns the index that immediately follows the given 
     * <var>index</var>. The returned index is strictly greater
     * than the given <var>index</var>, and is contained in the table.
     * <br>If the given <var>index</var> is null, returns the first
     * index in the table. 
     * <br>If there are no index after the given <var>index</var>,
     * returns null.
     **/
    public SnmpOid getNext(SnmpOid index);
    
    /**
     * Returns true if the given <var>index</var> is present.
     **/
    public boolean contains(SnmpOid index);

}

