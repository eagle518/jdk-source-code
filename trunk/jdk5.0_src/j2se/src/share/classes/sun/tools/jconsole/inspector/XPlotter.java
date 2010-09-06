/*
 * @(#)XPlotter.java	1.1 04/04/30
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;
import sun.tools.jconsole.Plotter;
import javax.swing.JTable;
import java.awt.Graphics;

public class XPlotter extends Plotter {
    JTable table;
    public XPlotter(JTable table, 
		    boolean b) {
	super(b);
	this.table = table;
    }
    public void addValue(String key, long value) {
	super.addValue(key, value);
	table.repaint();	
    }
}
