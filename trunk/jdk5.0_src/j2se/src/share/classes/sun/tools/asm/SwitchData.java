/*
 * @(#)SwitchData.java	1.27 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;
import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Arrays;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public final
class SwitchData {
    int minValue, maxValue;
    Label defaultLabel = new Label();
    Hashtable tab = new Hashtable();
// JCOV
    Hashtable whereCaseTab = null;
// end JCOV

    /**
     * Get a label
     */
    public Label get(int n) {
	return (Label)tab.get(new Integer(n));
    }
    
    /**
     * Get a label
     */
    public Label get(Integer n) {
	return (Label)tab.get(n);
    }

    /**
     * Add a label
     */
    public void add(int n, Label lbl) {
	if (tab.size() == 0) {
	    minValue = n;
	    maxValue = n;
	} else {
	    if (n < minValue) {
		minValue = n;
	    }
	    if (n > maxValue) {
		maxValue = n;
	    }
	}
	tab.put(new Integer(n), lbl);
    }

    /**
     * Get the default label
     */
    public Label getDefaultLabel() {
	return defaultLabel;
    }

    /**
     * Return the keys of this enumaration sorted in ascending order 
     */
    public synchronized Enumeration sortedKeys() {
	return new SwitchDataEnumeration(tab);
    }

// JCOV
    public void initTableCase() {
	whereCaseTab = new Hashtable();
    }
    public void addTableCase(int index, long where) {
	if (whereCaseTab != null)
            whereCaseTab.put(new Integer(index), new Long(where));
    }
    public void addTableDefault(long where) {
	if (whereCaseTab != null)
	    whereCaseTab.put("default", new Long(where));
    }
    public long whereCase(Object key) {
	Long i = (Long) whereCaseTab.get(key);
	return (i == null) ? 0 : i.longValue();
    }
    public boolean getDefault() {
         return (whereCase("default") != 0);
    }
// end JCOV
}

class SwitchDataEnumeration implements Enumeration { 
    private Integer table[];
    private int current_index = 0;

    /**
     * Create a new enumeration from the hashtable.  Each key in the
     * hash table will be an Integer, with the value being a label.  The
     * enumeration returns the keys in sorted order.
     */
    SwitchDataEnumeration(Hashtable tab) { 
	table = new Integer[tab.size()];
	int i = 0;
	for (Enumeration e = tab.keys() ; e.hasMoreElements() ; ) {
	    table[i++] = (Integer)e.nextElement();
	}
	Arrays.sort(table);
	current_index = 0;
    }

    /**
     * Are there more keys to return?
     */
    public boolean hasMoreElements() { 
	return current_index < table.length;
    }

    /**
     * Return the next key.
     */
    public Object nextElement() { 
	return table[current_index++];
    }
}


