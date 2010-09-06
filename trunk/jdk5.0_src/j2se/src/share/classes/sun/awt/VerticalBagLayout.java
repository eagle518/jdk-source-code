/*
 * @(#)VerticalBagLayout.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.*;


/**
 * A vertical 'bag' of Components.  Allocates space for each Component from
 * top to bottom.
 *
 * @version 	1.10 12/19/03
 * @author 	Herb Jellinek
 */
public class VerticalBagLayout implements LayoutManager {

    int vgap;

    /**
     * Constructs a new VerticalBagLayout.
     */
    public VerticalBagLayout() {
	this(0);
    }

    /**
     * Constructs a VerticalBagLayout with the specified gaps.
     * @param vgap the vertical gap
     */
    public VerticalBagLayout(int vgap) {
	this.vgap = vgap;
    }

    /**
     * Adds the specified named component to the layout.
     * @param name the String name
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
     * Returns the minimum dimensions needed to lay out the components
     * contained in the specified target container. 
     * @param target the Container on which to do the layout
     * @see Container
     * @see #preferredLayoutSize
     */
    public Dimension minimumLayoutSize(Container target) {
	Dimension dim = new Dimension();
	int nmembers = target.countComponents();

	for (int i = 0; i < nmembers; i++) {
	    Component comp = target.getComponent(i);
	    if (comp.isVisible()) {
		Dimension d = comp.minimumSize();
		dim.width = Math.max(d.width, dim.width);
		dim.height += d.height + vgap;
	    }
	}

	Insets insets = target.insets();
	dim.width += insets.left + insets.right;
	dim.height += insets.top + insets.bottom;

	return dim;
    }
    
    /**
     * Returns the preferred dimensions for this layout given the components
     * in the specified target container.
     * @param target the component which needs to be laid out
     * @see Container
     * @see #minimumLayoutSize
     */
    public Dimension preferredLayoutSize(Container target) {
	Dimension dim = new Dimension();
	int nmembers = target.countComponents();

	for (int i = 0; i < nmembers; i++) {
	    Component comp = target.getComponent(i);
	    if (true || comp.isVisible()) {
		Dimension d = comp.preferredSize();
		dim.width = Math.max(d.width, dim.width);
		dim.height += d.height + vgap;
	    }
	}

	Insets insets = target.insets();
	dim.width += insets.left + insets.right;
	dim.height += insets.top + insets.bottom;

	return dim;
    }

    /**
     * Lays out the specified container. This method will actually reshape the
     * components in the specified target container in order to satisfy the 
     * constraints of the VerticalBagLayout object. 
     * @param target the component being laid out
     * @see Container
     */
    public void layoutContainer(Container target) {
	Insets insets = target.insets();
	int top = insets.top;
	int bottom = target.size().height - insets.bottom;
	int left = insets.left;
	int right = target.size().width - insets.right;
	int nmembers = target.countComponents();
	
	for (int i = 0; i < nmembers; i++) {
	    Component comp = target.getComponent(i);
	    if (comp.isVisible()) {
		int compHeight = comp.size().height;
		comp.resize(right - left, compHeight);
		Dimension d = comp.preferredSize();
		comp.reshape(left, top, right - left, d.height);
		top += d.height + vgap;
	    }
	}
    }
    
    /**
     * Returns the String representation of this VerticalBagLayout's values.
     */
    public String toString() {
	return getClass().getName() + "[vgap=" + vgap + "]";
    }
}
