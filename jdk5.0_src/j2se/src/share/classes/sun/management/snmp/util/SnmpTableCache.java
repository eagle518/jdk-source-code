/*
 * @(#)file      SnmpTableCache.java
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
 * This abstract class implements a weak cache that holds table data.
 * <p>The table data is stored in an instance of 
 * {@link SnmpCachedData}, which is kept in a {@link WeakReference}.
 * If the WeakReference is null or empty, the cached data is recomputed.</p>
 *
 * <p><b>NOTE: This class is not synchronized, subclasses must implement
 *          the appropriate synchronization when needed.</b></p>
 **/
public abstract class SnmpTableCache implements Serializable {

    /**
     * Interval of time in ms during which the cached table data
     * is considered valid.
     **/
    protected long validity;

    /**
     * A weak refernce holding cached table data.
     **/
    protected transient WeakReference datas;

    /**
     * true if the given cached table data is obsolete.
     **/
    protected boolean isObsolete(SnmpCachedData cached) {
	if (cached   == null) return true;
	if (validity < 0)     return false;
	return ((System.currentTimeMillis() - cached.lastUpdated) > validity);
    }

    /**
     * Returns the cached table data.
     * Returns null if the cached data is obsolete, or if there is no
     * cached data, or if the cached data was garbage collected.
     * @return a still valid cached data or null. 
     **/
    protected SnmpCachedData getCachedDatas() {
	if (datas == null) return null;
	final SnmpCachedData cached = (SnmpCachedData) datas.get();
	if ((cached == null) || isObsolete(cached)) return null;
	return cached;
    }

    /**
     * Returns the cached table data, if it is still valid,
     * or recompute it if it is obsolete. 
     * <p>
     * When cache data is recomputed, store it in the weak reference,
     * unless {@link #validity} is 0: then the data will not be stored
     * at all.<br>
     * This method calls {@link #isObsolete(SnmpCachedData)} to determine 
     * whether the cached data is obsolete, and {
     * {@link #updateCachedDatas(Object)} to recompute it.
     * </p>
     * @param context A context object.
     * @return the valid cached data, or the recomputed table data.
     **/
    protected synchronized SnmpCachedData getTableDatas(Object context) {
	final SnmpCachedData cached   = getCachedDatas();
	if (cached != null) return cached;
	final SnmpCachedData computedDatas = updateCachedDatas(context);
	if (validity != 0) datas = new WeakReference(computedDatas);
	return computedDatas;
    }

    /**
     * Recompute cached data.
     * @param context A context object, as passed to 
     *        {@link #getTableDatas(Object)}
     **/
    protected abstract SnmpCachedData updateCachedDatas(Object context);

    /**
     * Return a table handler that holds the table data.
     * This method should return the cached table data if it is still
     * valid, recompute it and cache the new value if it's not.
     **/
    public abstract SnmpTableHandler getTableHandler();

}
