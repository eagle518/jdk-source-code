/*
 * @(#)PathRenderer.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.JTextField;
import javax.swing.JTable;
import javax.swing.JComponent;
import javax.swing.border.Border;
import javax.swing.border.LineBorder;
import java.awt.Color;
import java.awt.Component;


/**
 *
 * PathRenderer paints the cell border in red if the path to
 * JRE is incorrect.  Once user types in correct path to JRE,
 * cell' border will be painted in black.
 *
 * Also, this renderer sets the tooltip for the cell.  The tooltip
 * is the value of the cell itself.  Some paths can be very long
 * and might not fit in the cell, so we display it in a tooltip.
 */
public class PathRenderer extends DefaultTableCellRenderer {

    private Border badBorder = new LineBorder(Color.red);
    private Border goodBorder = new LineBorder(Color.black);
    
    public Component getTableCellRendererComponent (JTable table, 
                Object value, boolean isSelected, 
                boolean hasFocus, int row, int column) {                      
        Component retValue = super.getTableCellRendererComponent (
			table, value, isSelected, hasFocus, row, column);

        if (retValue instanceof JComponent && 
            table.getModel() instanceof JreTableModel &&
            !(((JreTableModel)table.getModel()).isPathValid(row)) ) {
                    ((JComponent)retValue).setBorder(badBorder);
                }
        else if (retValue instanceof JComponent){
            ((JComponent)retValue).setBorder(goodBorder);
        }
        
        if (retValue instanceof JComponent){
            String tooltip = null;
            if (value != null){
                tooltip = value.toString();
            }
            
            // Set the tooltip for the cell.
            ((JComponent)retValue).setToolTipText(tooltip);
            
        }
        
        return retValue;
    }    
}
