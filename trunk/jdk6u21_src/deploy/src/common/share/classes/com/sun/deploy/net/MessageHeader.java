/*
 * @(#)MessageHeader.java	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* From sun.net.www.MessageHeader */

package com.sun.deploy.net;

import java.io.*;
import java.util.Collections;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.Iterator;
import java.util.NoSuchElementException;

/** An RFC 844 or MIME message header.  
    Key values of null are legal: they indicate lines in
    the header that don't have a valid key, but do have
    a value (this isn't legal according to the standard,
    but lines like this are everywhere). */
public
class MessageHeader {
    private String keys[];
    private String values[];
    private int nkeys;

    public MessageHeader () {
	grow();
    }
    
    /**
     * Find the value that corresponds to this key.
     * It finds only the first occurrence of the key.
     * @param k the key to find.
     * @return null if not found.
     */
    synchronized String findValue(String k) {
	if (k == null) {
	    for (int i = nkeys; --i >= 0;)
		if (keys[i] == null)
		    return values[i];
	} else
	    for (int i = nkeys; --i >= 0;) {
		if (k.equalsIgnoreCase(keys[i]))
		    return values[i];
	    }
	return null;
    }

    // return the location of the key
    public synchronized int getKey(String k) {
	for (int i = nkeys; --i >= 0;)
	    if ((keys[i] == k) ||
		(k != null && k.equalsIgnoreCase(keys[i])))
		return i;
	return -1;
    }

    public synchronized String getKey(int n) {
	if (n < 0 || n >= nkeys) return null;
	return keys[n];
    }

    public synchronized String getValue(int n) {
	if (n < 0 || n >= nkeys) return null;
	return values[n];
    }
	
    public synchronized Map getHeaders() {
	return getHeaders(null);
    }

    public synchronized Map getHeaders(String[] excludeList) {
        boolean skipIt = false;
	Map m = new HashMap();
	for (int i = nkeys; --i >= 0;) {
	    if (excludeList != null) {
		// check if the key is in the excludeList.
		// if so, don't include it in the Map.
		for (int j = 0; j < excludeList.length; j++) {
		    if ((excludeList[j] != null) && 
			(excludeList[j].equalsIgnoreCase(keys[i]))) {
			skipIt = true;
			break;
		    }
		}
	    }
	    if (!skipIt) {
		List l = (List)m.get(keys[i]);
		if (l == null) {
		    l = new ArrayList();
		    m.put(keys[i], l);
		}
		l.add(values[i]);
	    } else {
		// reset the flag
		skipIt = false;
	    }
	}

	Set keySet = m.keySet();
	for (Iterator i = keySet.iterator(); i.hasNext();) {
	    Object key = i.next();
	    List l = (List)m.get(key);
	    m.put(key, Collections.unmodifiableList(l));
	}
	
	return Collections.unmodifiableMap(m);
    }

    /** Adds a key value pair to the end of the
	header.  Duplicates are allowed */
    public synchronized void add(String k, String v) {
	grow();
	keys[nkeys] = k;
	values[nkeys] = v;
	nkeys++;
    }
	    
    /** grow the key/value arrays as needed */
    private void grow() {
	if (keys == null || nkeys >= keys.length) {
	    String[] nk = new String[nkeys + 4];
	    String[] nv = new String[nkeys + 4];
	    if (keys != null)
		System.arraycopy(keys, 0, nk, 0, nkeys);
	    if (values != null)
		System.arraycopy(values, 0, nv, 0, nkeys);
	    keys = nk;
	    values = nv;
	}
    }  

    public synchronized String toString() {
	String result = super.toString();
	for (int i = 0; i < keys.length; i++) {
	    result += "{"+keys[i]+": "+values[i]+"}";
	}
	return result;
    }
}
