/*
 * @(#)file      SnmpListTableCache.java
 * @(#)author    Sun Microsystems, Inc.
 * @(#)version   1.7
 * @(#)lastedit  03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.management.snmp.util;

import com.sun.jmx.snmp.SnmpOid;

import java.io.Serializable;

import java.util.Comparator;
import java.util.Arrays;
import java.util.TreeMap;
import java.util.List;
import java.util.Iterator;

import java.lang.ref.WeakReference;

/**
 * This abstract class implements a weak cache for a table whose data
 * is obtained from a {@link  List}.
 *
 * <p><b>NOTE: This class is not synchronized, subclasses must implement
 *          the appropriate synchronization whwn needed.</b></p>
 **/
public abstract class SnmpListTableCache extends SnmpTableCache {


    /**
     * The index of the entry corresponding to the given <var>item</var>.
     * <br>This method is called by {@link #updateCachedDatas(Object,List)}. 
     * The given <var>item</var> is expected to be always associated with
     * the same index.
     * @param context The context passed to 
     *        {@link #updateCachedDatas(Object,List)}.
     * @param rawDatas Raw table datas passed to 
     *        {@link #updateCachedDatas(Object,List)}.
     * @param rank Rank of the given <var>item</item> in the 
     *        <var>rawDatas</var> list iterator.
     * @param item The raw data object for which an index must be determined.
     **/
    protected abstract SnmpOid getIndex(Object context, List rawDatas, 
					int rank, Object item);

    /**
     * The data for the entry corresponding to the given <var>item</var>.
     * <br>This method is called by {@link #updateCachedDatas(Object,List)}. 
     * @param context The context passed to 
     *        {@link #updateCachedDatas(Object,List)}.
     * @param rawDatas Raw table datas passed to 
     *        {@link #updateCachedDatas(Object,List)}.
     * @param rank Rank of the given <var>item</item> in the 
     *        <var>rawDatas</var> list iterator.
     * @param item The raw data object from which the entry data must be 
     *        extracted.
     * @return By default <var>item</var> is returned.
     **/
    protected Object getData(Object context, List rawDatas, 
			     int rank, Object item) {
	return item;
    }
    
    /**
     * Recompute cached data.
     * @param context A context object, valid during the duration of
     *        of the call to this method, and that will be passed to
     *        {@link #getIndex} and {@link #getData}. <br>
     *        This method is intended to be called by 
     *        {@link #updateCachedDatas(Object)}. It is assumed that
     *        the context is be allocated by  before this method is called,
     *        and released just after this method has returned.<br>
     *        This class does not use the context object: it is a simple
     *        hook for subclassed.
     * @param rawDatas The table datas from which the cached data will be
     *        computed.
     * @return the computed cached data.
     **/
    protected SnmpCachedData updateCachedDatas(Object context, List rawDatas) {
	final int size = ((rawDatas == null)?0:rawDatas.size());
	if (size == 0) return  null;

	final long time = System.currentTimeMillis();
	final Iterator it  = rawDatas.iterator();
	final TreeMap  map = new TreeMap(SnmpCachedData.oidComparator);
	for (int rank=0; it.hasNext() ; rank++) {
	    final Object  item  = it.next();
	    final SnmpOid index = getIndex(context, rawDatas, rank, item);
	    final Object  data  = getData(context, rawDatas, rank, item);
	    if (index == null) continue;
	    map.put(index,data);
	}

	return new SnmpCachedData(time,map);
    }

}
