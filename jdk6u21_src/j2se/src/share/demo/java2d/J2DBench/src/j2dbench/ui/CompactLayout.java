/*
 * @(#)CompactLayout.java	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench.ui;

import java.awt.Dimension;
import java.awt.Insets;
import java.awt.Component;
import java.awt.Container;
import java.awt.LayoutManager;

public class CompactLayout implements LayoutManager {
    boolean horizontal;

    public CompactLayout(boolean horizontal) {
	this.horizontal = horizontal;
    }

    /**
     * If the layout manager uses a per-component string,
     * adds the component <code>comp</code> to the layout,
     * associating it 
     * with the string specified by <code>name</code>.
     * 
     * @param name the string to be associated with the component
     * @param comp the component to be added
     */
    public void addLayoutComponent(String name, Component comp) {
    }

    /**
     * Removes the specified component from the layout.
     * @param comp the component to be removed
     */
    public void removeLayoutComponent(Component comp) {
    }

    /**
     * Calculates the preferred size dimensions for the specified 
     * container, given the components it contains.
     * @param parent the container to be laid out
     *  
     * @see #minimumLayoutSize
     */
    public Dimension preferredLayoutSize(Container parent) {
	return getSize(parent, false);
    }

    /** 
     * Calculates the minimum size dimensions for the specified 
     * container, given the components it contains.
     * @param parent the component to be laid out
     * @see #preferredLayoutSize
     */
    public Dimension minimumLayoutSize(Container parent) {
	return getSize(parent, true);
    }

    public Dimension getSize(Container parent, boolean minimum) {
	int n = parent.getComponentCount();
	Insets insets = parent.getInsets();
	Dimension d = new Dimension();
	for (int i = 0; i < n; i++) {
	    Component comp = parent.getComponent(i);
	    if (comp instanceof EnableButton) {
		continue;
	    }
	    Dimension p = (minimum
			   ? comp.getMinimumSize()
			   : comp.getPreferredSize());
	    if (horizontal) {
		d.width += p.width;
		if (d.height < p.height) {
		    d.height = p.height;
		}
	    } else {
		if (d.width < p.width) {
		    d.width = p.width;
		}
		d.height += p.height;
	    }
	}
	d.width += (insets.left + insets.right);
	d.height += (insets.top + insets.bottom);
	return d;
    }

    /** 
     * Lays out the specified container.
     * @param parent the container to be laid out 
     */
    public void layoutContainer(Container parent) {
	int n = parent.getComponentCount();
	Insets insets = parent.getInsets();
	Dimension size = parent.getSize();
	int c = horizontal ? insets.left : insets.top;
	int x, y;
	int ebx = size.width - insets.right;
	size.width -= (insets.left + insets.right);
	size.height -= (insets.top + insets.bottom);
	for (int i = 0; i < n; i++) {
	    Component comp = parent.getComponent(i);
	    Dimension pref = comp.getPreferredSize();
	    if (comp instanceof EnableButton) {
		ebx -= 4;
		ebx -= pref.width;
		x = ebx;
		y = (insets.top - pref.height) / 2;
	    } else if (horizontal) {
		x = c;
		c += pref.width;
		y = insets.top;
		pref.height = size.height;
	    } else {
		x = insets.left;
		pref.width = size.width;
		y = c;
		c += pref.height;
	    }
	    comp.setBounds(x, y, pref.width, pref.height);
	}
    }
}
