/*
 * @(#)JVMDINode.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdwpgen;

import java.util.*;
import java.io.*;
import java.text.Collator;

/**
 * Will get converted to Constant nodes during prune.
 */
class JVMDINode extends AbstractNamedNode {
            
    void addConstants(List addons) {
	List addList = new ArrayList();
        int prefixLength = name.length();

	// Place the name map entries that match into addList
        for (Iterator it = Main.nameMap.entrySet().iterator(); it.hasNext();) {
            Map.Entry entry = (Map.Entry)it.next();
            String nm = (String)entry.getKey();
            if (nm.startsWith(name)) {
                NameValueNode nv = (NameValueNode)entry.getValue();
                nv.name = nm.substring(prefixLength);
		addList.add(nv);
	    }
	}

	// Sort JVMDI defs to be added, by numeric value (if possible)
	Collections.sort(addList, new Comparator() {
	    public int compare(Object o1, Object o2) {
		String s1 = ((NameValueNode)o1).val;
		String s2 = ((NameValueNode)o2).val;
		try {
		    return Integer.parseInt(s1) - Integer.parseInt(s2);
		} catch (NumberFormatException exc) {
		    return Collator.getInstance().compare(s1, s2);
		}
	    }
	});
	
	// Wrap in constant nodes and add to addons
        for (Iterator it = addList.iterator(); it.hasNext();) {
	    NameValueNode nv = (NameValueNode)it.next();
	    List constName = new ArrayList();
	    constName.add(nv);
	    ConstantNode cn = new ConstantNode(constName);
	    addons.add(cn);
        }
    }        
}
