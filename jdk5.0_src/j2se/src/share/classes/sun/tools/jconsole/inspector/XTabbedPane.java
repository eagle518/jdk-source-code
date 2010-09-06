/*
 * @(#)XTabbedPane.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

// java import
import javax.swing.JTabbedPane;
import java.awt.Component;
import java.awt.Container;
import java.io.*;
//

public class XTabbedPane extends JTabbedPane {

    public XTabbedPane() {
    }

    public XTabbedPane(int p) {
	super(p);
    }
    
    public Component findComponentAt(int x, int y) {
	if (!contains(x, y)) {
	    return null;
	}
	int ncomponents = getComponentCount();
	for (int i = 0; i < ncomponents; i++) {
	    Component comp = getComponentAt(i);
	    if (comp != null) {
		if (comp instanceof Container) {
		    if(comp.isVisible()) 
			comp = ((Container)comp).
			    findComponentAt(x - comp.getX(),
					    y - comp.getY());
		} else {
		    comp = comp.getComponentAt(x - comp.getX(), 
					       y - comp.getY());
		}
		if (comp != null && comp.isVisible()) {
		    return comp;
		}
	    }
	}
	return this;
    }
}
