/*
 * @(#)AuthCacheImpl.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import java.io.IOException;
import java.net.URL;
import java.util.Hashtable;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Enumeration;
import java.util.HashMap;

/**
 * @author Michael McMahon
 * @version 1.2, 12/19/03
 */

public class AuthCacheImpl implements AuthCache {
    HashMap hashtable;

    public AuthCacheImpl () {
	hashtable = new HashMap ();
    }

    public void setMap (HashMap map) {
	hashtable = map;
    }

    // put a value in map according to primary key + secondary key which
    // is the path field of AuthenticationInfo

    public synchronized void put (String pkey, AuthCacheValue value) {
	LinkedList list = (LinkedList) hashtable.get (pkey);
	String skey = value.getPath();
	if (list == null) {
	    list = new LinkedList ();
	    hashtable.put (pkey, list);
	}
	// Check if the path already exists or a super-set of it exists
	ListIterator iter = list.listIterator();
    	while (iter.hasNext()) {
    	    AuthenticationInfo inf = (AuthenticationInfo)iter.next();
    	    if (inf.path == null || inf.path.startsWith (skey)) {
	    	iter.remove ();
    	    }
	}
	iter.add (value);
    }

    // get a value from map checking both primary
    // and secondary (urlpath) key

    public synchronized AuthCacheValue get (String pkey, String skey) {
	AuthenticationInfo result = null;
	LinkedList list = (LinkedList) hashtable.get (pkey);
	if (list == null || list.size() == 0) {
	    return null;
	}
	if (skey == null) { 
	    // list should contain only one element
	    return (AuthenticationInfo)list.get (0);
	}
	ListIterator iter = list.listIterator();
    	while (iter.hasNext()) {
    	    AuthenticationInfo inf = (AuthenticationInfo)iter.next();
    	    if (skey.startsWith (inf.path)) {
	    	return inf;
    	    }
	}
	return null;
    }

    public synchronized void remove (String pkey, AuthCacheValue entry) {
	LinkedList list = (LinkedList) hashtable.get (pkey);
	if (list == null) {
	    return;
	}
	if (entry == null) {
	    list.clear();
	    return;
	}
	ListIterator iter = list.listIterator ();
	while (iter.hasNext()) {
	    AuthenticationInfo inf = (AuthenticationInfo)iter.next();
	    if (entry.equals(inf)) {
	    	iter.remove ();
	    }
	}
    }
}
