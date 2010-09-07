/*
 * @(#)JSmartTextArea.java	1.12 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
    //
    // Control Panel uses 360-pixels wide JTextArea.
    //
    int preferred_width = 360;
    
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
    
    /** Creates a new instance of JSmartTextArea with font size <fz>,
     *  and specified preferred width. 
     *  This is used by the dialog UI template.
     */
    public JSmartTextArea(int fz, int my_width, boolean bold) {
        preferred_width = my_width;
        JLabel label = new JLabel();
        Font system_font = label.getFont();
	Font new_font;
	if (bold){
		new_font = system_font.deriveFont(Font.BOLD, (float)fz);
	}else{
		new_font = system_font.deriveFont(system_font.getStyle(), (float)fz);
	}
        // Set empty border
        setBorder(new EmptyBorder(new Insets(0, 0, 0, 0)));
        setEditable(false);
        setLineWrap(true);
        setWrapStyleWord(true); 
        setFont(new_font);
        setRows(0);
        setFocusable(false);
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
	    int columns = (preferred_width / getColumnWidth());
	    setColumns(columns);
	    d = super.getPreferredSize();
	    setColumns(0);
	} else {
	    d = super.getPreferredSize();
	}
	return d;
    }

}
