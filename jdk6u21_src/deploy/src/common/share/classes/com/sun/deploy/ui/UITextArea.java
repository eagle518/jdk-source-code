/*
 * @(#)UITextArea.java	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

import java.awt.Graphics;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Insets;
import java.awt.Image;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.border.EmptyBorder;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;

public class UITextArea extends JTextArea 
{
    int preferred_width = 360;
    Image backgroundImage = null;
    
    /** Creates a new instance of UITextArea */
    public UITextArea() {
        
        JLabel label = new JLabel();

        // Set empty border
        setBorder(new EmptyBorder(new Insets(0, 5, 0, 0)));
        setEditable(false);
        setLineWrap(true);
        setWrapStyleWord(true); 
        setFont(label.getFont());
        setRows(0);
	setHighlighter(null);
        invalidate();
    }   
    
    /** Creates a new instance of UITextArea 
     */
    public UITextArea(String text) {
        this();
        setText(text);
    }   
    
    /** Creates a new instance of UITextArea with font size <fz>,
     *  and specified preferred width. 
     *  This is used by the dialog UI template.
     */
    public UITextArea(int fz, int my_width, boolean bold) {
	this();
        preferred_width = my_width;
        JLabel label = new JLabel();
        Font system_font = label.getFont();
        Font new_font;
        if (bold) {   
	      new_font= system_font.deriveFont(Font.BOLD,(float)fz);
        } else {
	      new_font= system_font.deriveFont((float)fz);
        }
        // Set empty border
        setBorder(new EmptyBorder(new Insets(0, 0, 0, 0)));
        setFont(new_font);
        invalidate();
    }   

    public boolean isFocusTraversable() {
	return false;
    }

    public void setText(String text)
    {
        super.setText(text);
        invalidate();
    }    

    public void setBackgroundImage(Image image) {
        backgroundImage = image;
        boolean isOpaque = (backgroundImage == null);
        setOpaque(isOpaque);
    }

    // use parents background color, not the UIResource, but the real color
    public void paintComponent(Graphics g) {
        if (backgroundImage != null) {
            g.drawImage(backgroundImage, 0, 0, this);
        } else {
            setBackground(new Color(
                getParent().getBackground().getRGB()));
	}
        super.paintComponent(g);
    }

    public Dimension getPreferredSize() {
        Dimension d;
	if (backgroundImage != null) {
	    d = new Dimension(backgroundImage.getWidth(this),
			      backgroundImage.getHeight(this));
	} else {
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
	}
        return d;
    }

}
