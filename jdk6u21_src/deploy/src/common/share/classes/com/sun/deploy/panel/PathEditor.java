/*
 * @(#)PathEditor.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.DefaultCellEditor;
import javax.swing.JTable;
import javax.swing.table.TableModel;
import javax.swing.JTextField;
import javax.swing.JFileChooser;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.border.Border;
import javax.swing.border.LineBorder;

import java.lang.Object;
import java.lang.String;

import java.io.File;
import java.io.IOException;

import java.awt.Component;
import java.awt.Color;
import com.sun.deploy.config.JREInfo;

/**
 * Editor used for the path to the JRE. Changes the border based on
 * the validity of the path being edited.
 */  
public class PathEditor extends DefaultCellEditor implements DocumentListener {

    private int row;
    private Border badBorder = new LineBorder(Color.red);
    private Border goodBorder = new LineBorder(Color.black);
    private Border currentBorder = badBorder;
    private JTable table;

    
    PathEditor() {
        super(new JTextField());        
        ((JTextField)editorComponent).getDocument().addDocumentListener(this);        
    }
         
    public Component getTableCellEditorComponent(JTable table,
        	Object value, boolean isSelected, int row, int column) {
                    
        table.getTableHeader().setResizingAllowed(false);                                                            
        this.row = row;
        this.table = table;
        super.getTableCellEditorComponent(table, value, isSelected,
                                          row, column);
        currentBorder = ( ((JreTableModel)table.getModel()).isPathValid(row) ) ?
                        goodBorder : badBorder;
        
        editorComponent.setBorder(currentBorder);
        
        return editorComponent;
    }
        
    public void insertUpdate(DocumentEvent e) {
        updateBorderFromEditor();
        
    }
    public void removeUpdate(DocumentEvent e) {
        updateBorderFromEditor();
    }
    public void changedUpdate(DocumentEvent e) {}
    
    private void updateBorderFromEditor() {
        Object value = getCellEditorValue();        
        boolean valid;
        if (value instanceof String && 
            table.getModel() instanceof JreTableModel) {            
            valid = JREInfo.isValidJREPath((String)value);
        } else {
            valid = false;
        }
        if (valid) {
            editorComponent.setBorder(goodBorder);
        } else {
            editorComponent.setBorder(badBorder);
        }
    }
    
    public boolean stopCellEditing(){        
        this.table.getTableHeader().setResizingAllowed(true);
        return super.stopCellEditing();
    }
}

