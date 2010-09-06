/*
 * @(#)XArrayDataViewer.java	1.5 04/04/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import

import java.awt.Component;
import java.util.Collection;
import java.util.Iterator;

import javax.swing.JList;
import javax.swing.JScrollPane;

import java.lang.reflect.Array;

class XArrayDataViewer {
    private XArrayDataViewer() {}

    public static boolean isViewableValue(Object value) {
	return Utils.isSupportedDataStructure(value);
    }

    public static Component loadArray(Object value) {
	Component comp = null;
	if(isViewableValue(value)) {
	    Object[] arr = null;
	    if(value instanceof Collection) {
		Collection collec = (Collection) value;
		Iterator it = collec.iterator();
		arr = new Object[collec.size()];
		int i = 0;
		while(it.hasNext()) {
		    arr[i] = it.next();
		    i++;
		}
	    } else {
		int length = Array.getLength(value);
		arr = new Object[length];
		int colWidth = 0;
		for(int i = 0; i < length; i++) {
		    arr[i] = Array.get(value, i);
		}
	    }
	    JList list = new JList(arr);
	    list.setVisibleRowCount(10);
	    list.setEnabled(false);
	    JScrollPane scrollp = new JScrollPane(list);
	    comp = scrollp;
	}
	return comp;
    }
}
