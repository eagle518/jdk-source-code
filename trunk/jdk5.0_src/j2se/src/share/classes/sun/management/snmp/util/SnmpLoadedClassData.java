/*
 * @(#)file      SnmpLoadedClassData.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.6
 * @(#)lastedit  03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.util;

import com.sun.jmx.snmp.SnmpOid;
import com.sun.jmx.snmp.SnmpStatusException;

import java.io.Serializable;

import java.util.Comparator;
import java.util.Arrays;
import java.util.TreeMap;
import java.util.List;
import java.util.Iterator;

import java.lang.ref.WeakReference;

/**
 * This class is used to cache LoadedClass table data.
 * WARNING : MUST IMPLEMENT THE SnmpTableHandler directly. Some changes in daniel classes.
 **/
public final class SnmpLoadedClassData extends SnmpCachedData {
    
    /**
     * Constructs a new instance of SnmpLoadedClassData. Instances are
     * immutable.
     * @param lastUpdated Time stamp as returned by 
     *        {@link System#currentTimeMillis System.currentTimeMillis()}
     * @param indexMap The table indexed table data, sorted in ascending 
     *                 order by {@link #oidComparator}. The keys must be
     *                 instances of {@link SnmpOid}.
     **/
    public SnmpLoadedClassData(long lastUpdated, TreeMap indexMap) {
	super(lastUpdated, indexMap, false);
    }
    
    
    // SnmpTableHandler.getData()
    public final Object getData(SnmpOid index) {
	int pos = 0;
	
	try {
	    pos = (int) index.getOidArc(0);
	}catch(SnmpStatusException e) {
	    return null;
	}
	
	if (pos >= datas.length) return null;
	return datas[pos];
    }

    // SnmpTableHandler.getNext()
    public final SnmpOid getNext(SnmpOid index) {
	int pos = 0;
	if (index == null) {
	    if( (datas!= null) && (datas.length >= 1) )
		return new SnmpOid(0);
	}
	try {
	    pos = (int) index.getOidArc(0);
	}catch(SnmpStatusException e) {
	    return null;
	}
	
	if(pos < (datas.length - 1))
	    return new SnmpOid(pos+1);
	else 
	    return null;
    }

    // SnmpTableHandler.contains()
    public final boolean contains(SnmpOid index) {
	int pos = 0;
	
	try {
	    pos = (int) index.getOidArc(0);
	}catch(SnmpStatusException e) {
	    return false;
	}

	return (pos < datas.length);
    }

}
 
