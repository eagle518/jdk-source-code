/*
 * @(#)XCreateWindowParams.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class XCreateWindowParams extends HashMap {
    public XCreateWindowParams() {
    }
    public XCreateWindowParams(Object[] map) {
        init(map);
    }
    private void init(Object[] map) {
        if (map.length % 2 != 0) {
            throw new IllegalArgumentException("Map size should be devisible by two");
        }
        for (int i = 0; i < map.length; i += 2) {
            put(map[i], map[i+1]);
        }
    }

    public XCreateWindowParams putIfNull(Object key, Object value) {
        if (!containsKey(key)) {
            put(key, value);
        }
        return this;
    }   
    public XCreateWindowParams putIfNull(Object key, int value) {        
        if (!containsKey(key)) {
            put(key, new Integer(value));
        }
        return this;
    }
    public XCreateWindowParams putIfNull(Object key, long value) {
        if (!containsKey(key)) {
            put(key, new Long(value));
        }
        return this;
    }

    public XCreateWindowParams add(Object key, Object value) {
        put(key, value);
        return this;
    }
    public XCreateWindowParams add(Object key, int value) {
        put(key, new Integer(value));
        return this;
    }
    public XCreateWindowParams add(Object key, long value) {
        put(key, new Long(value));
        return this;
    }
    public XCreateWindowParams delete(Object key) {
        remove(key);
        return this;
    }
    public String toString() {
        StringBuffer buf = new StringBuffer();
        Iterator eIter = entrySet().iterator();
        while (eIter.hasNext()) {
            Map.Entry entry = (Map.Entry)eIter.next();
            buf.append(entry.getKey() + ": " + entry.getValue() + "\n");
        }
        return buf.toString();
    }

}
