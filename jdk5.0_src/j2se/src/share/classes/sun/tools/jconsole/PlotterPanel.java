/*
 * @(#)PlotterPanel.java	1.6 04/04/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

public class PlotterPanel extends BorderedComponent {
    Plotter plotter;

    public PlotterPanel(String labelStr, boolean unitsBytes, boolean collapsible) {
	super(labelStr, new Plotter(unitsBytes), collapsible);
	plotter = (Plotter)comp;
    }
}
