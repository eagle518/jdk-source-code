/*
 * @(#)WeakClassHashMap.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.server;

import java.lang.ref.SoftReference;
import java.util.Map;
import java.util.WeakHashMap;

public abstract class WeakClassHashMap {

    private WeakHashMap internalMap = new WeakHashMap();
	
    public WeakClassHashMap() {}
	
    public Map getMap(Class remoteClass) {
	SoftReference[] tableRef;

	/*
	 * The maps for remote classes are cached in a hash table using
	 * weak references to hold the Class object keys, so that the cache
	 * does not prevent the class from being unloaded, and using soft
	 * references to hold the values, so that the computed maps will
	 * generally persist while no objects of the remote class are
	 * exported, but their storage may be reclaimed if necessary, and
	 * accidental reachability of the remote class through its
	 * interfaces is avoided.
	 */
	synchronized (this) {
	    /*
	     * Look up class in cache; add entry if not found.
	     */
	    tableRef = (SoftReference[]) internalMap.get(remoteClass);
	    if (tableRef == null) {
		tableRef = new SoftReference[] { null };
		internalMap.put(remoteClass, tableRef);
	    }
	}

	/*
	 * Check cached reference to map corresponding to this class;
	 * if it is null, go and create a new map.
	 */
	synchronized (tableRef) {
	    Map table = null;
	    if (tableRef[0] != null) {
		table = (Map) tableRef[0].get();
	    }
	    if (table == null) {
		table = createMap(remoteClass);
		tableRef[0] = new SoftReference(table);
	    }
	    return table;
	}
    }

    protected abstract Map createMap(Class remoteClass);
    
}
