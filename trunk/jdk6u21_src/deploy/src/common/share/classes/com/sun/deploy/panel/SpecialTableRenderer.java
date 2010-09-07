/*
 * @(#)SpecialTableRenderer.java	1.5 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
            if (value != null && !value.toString().trim().equals("")){
                tooltip = value.toString();                                
            }
            
            // When cell's value is an empty string, we'll set the tooltip
            // to NULL effectively disabling it.
            ((JComponent)retValue).setToolTipText(tooltip);                     
        }
        
        return retValue;        
    }

}
