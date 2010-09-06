/*
 * @(#)SpecialTableRenderer.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.panel;

import java.awt.Component;
import javax.swing.JTable;
import javax.swing.JComponent;
import javax.swing.table.DefaultTableCellRenderer;


/**
 * The only purpose of this renderer is to set the tooltip on the
 * cell, which is a string typed into the cell.  This is to make
 * long strings easy viewable.
 */
public class SpecialTableRenderer extends DefaultTableCellRenderer {

    public Component getTableCellRendererComponent (JTable table, 
                Object value, boolean isSelected, 
                boolean hasFocus, int row, int column) {                      
        Component retValue = super.getTableCellRendererComponent (
			table, value, isSelected, hasFocus, row, column);   
        
        // Set the tooltip for the cell.
        if (retValue instanceof JComponent){
            String tooltip = null;
            if (value != null){
                tooltip = value.toString();
            }
            
            ((JComponent)retValue).setToolTipText(tooltip);
        }
        
        return retValue;        
    }

}
