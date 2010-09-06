/*
 * @(#)SpecialTableEditor.java	1.2 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.panel;

import javax.swing.DefaultCellEditor;
import javax.swing.JTable;
import javax.swing.JTextField;
import java.awt.Component;


/*
 * The purpose of this editor, is to prohibit column resize
 * during cell editing. 
 */
public class SpecialTableEditor extends DefaultCellEditor {
    JTable t;

    SpecialTableEditor() {
        super(new JTextField());
    }
    public Component getTableCellEditorComponent(JTable table,
        	Object value, boolean isSelected, int row, int column) {
        t = table;   
        
        // Make columns non-resizable.
        table.getTableHeader().setResizingAllowed(false);     
        
        return super.getTableCellEditorComponent(table, value, isSelected,
                                          row, column);
    }
    
    public boolean stopCellEditing(){ 
        // Allow columns to be resizable once editing stopped.
        t.getTableHeader().setResizingAllowed(true);
        return super.stopCellEditing();
    }
}
