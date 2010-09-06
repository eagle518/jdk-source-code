/*
 * @(#)JSmartTextArea.java	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.awt.Graphics;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Insets;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.border.EmptyBorder;
import com.sun.deploy.util.Trace;


public class JSmartTextArea extends JTextArea 
{
    /** Creates a new instance of JSmartLabel */
    public JSmartTextArea(String text) {
	this();
	setText(text);
    }   
    
    /** Creates a new instance of JSmartLabel */
    public JSmartTextArea() {
        
	JLabel label = new JLabel();

	// Set empty border
        setBorder(new EmptyBorder(new Insets(0, 5, 0, 0)));
        setEditable(false);
        setLineWrap(true);
        setWrapStyleWord(true); 
	setFont(label.getFont());
	setFocusable(false);
	setRows(0);
        invalidate();
    }   

    public void setText(String text)
    {
        super.setText(text);
        invalidate();
    }    

    // use parents background color, not the UIResource, but the real color
    public void paintComponent(Graphics g) {
        setBackground(new Color(
	    getParent().getBackground().getRGB()));
        super.paintComponent(g);
    }

    public Dimension getPreferredSize() {
	Dimension d;
	// if user hasn't set any preferred rows or columns, 
	// prefer a size that is 360 pixels wide
	if ((getRows() == 0) && (getColumns() == 0)) {
	    int columns = (360 / getColumnWidth());
	    setColumns(columns);
	    d = super.getPreferredSize();
	    setColumns(0);
	} else {
	    d = super.getPreferredSize();
	}
	return d;
    }

}
