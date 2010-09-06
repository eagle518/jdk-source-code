/*
 * @(#)PluginJreDialog.java	1.12 04/03/30
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.*;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.border.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Enumeration;
import com.sun.deploy.config.*;
import java.util.ArrayList;
import com.sun.deploy.resources.ResourceManager;

public class PluginJreDialog extends JDialog 
                             implements ListSelectionListener{
    
    public PluginJreDialog(Frame parent, boolean modal) {
        super(parent, modal);
        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {        
        setTitle(getMessage("jpi.jres.dialog.title"));
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                closeDialog(evt);
            }
        });
        
        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BorderLayout());
        topPanel.setBorder(new TitledBorder(getMessage("jpi.jres.dialog.border")));

        /*
         * Create table with PluginTableModel.
         */        
        jresTable = new JTable(new PluginTableModel());
        jresTable.setBorder(new LineBorder(Color.black));
        jresTable.setPreferredScrollableViewportSize(new Dimension(500, 70));
        jresTable.getSelectionModel().addListSelectionListener(this);
        
        // Set the special table editor for all columns.  This is to prohibit
        // header/column resize during editing.
        Enumeration en = jresTable.getColumnModel().getColumns();
        while( en.hasMoreElements() ){
            TableColumn next = (TableColumn)en.nextElement();
            next.setCellEditor(new SpecialTableEditor());
            next.setCellRenderer(new SpecialTableRenderer());
        }
        
        // Set a renderer that will show badBorder if the path isn't valid.
        jresTable.getColumnModel().getColumn(2).setCellRenderer(new PathRenderer());        
               
        // Set an editor that will show badBorder if the path isn't valid.
        jresTable.getColumnModel().getColumn(2).setCellEditor(new PathEditor());  
                
        // With this line uncommented out, the first column has a JComboBox
        // with two options: JRE, JDK.  
        //setProductTableColumn(jresTable.getColumnModel().getColumn(0));
        
        /*
         * Put table on a JScrollPane - this way the headers show up.
         * If I put table on a JPanel, headers don't show up.
         */
        JScrollPane tablePane = new JScrollPane(jresTable);
 
        // Add scroll pane to the top panel.
        topPanel.add(tablePane, BorderLayout.CENTER);

        JPanel addRemovePanel = new JPanel(new FlowLayout(FlowLayout.LEFT));
        addBtn = new JButton(getMessage("common.add_btn"));
        addBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                addBtnActionPerformed(evt);
            }
        });
        addBtn.setToolTipText(getMessage("vm.options.add_btn.tooltip"));
        addBtn.setMnemonic(ResourceManager.getVKCode("common.add_btn.mnemonic"));

        removeBtn = new JButton(getMessage("common.remove_btn"));
        removeBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                removeBtnActionPerformed(evt);
            }
        });
        removeBtn.setMnemonic(ResourceManager.getVKCode("common.remove_btn.mnemonic"));
        removeBtn.setEnabled(false);
        removeBtn.setToolTipText(getMessage("vm.options.remove_btn.tooltip"));
        addRemovePanel.add(addBtn);
        addRemovePanel.add(removeBtn);
        
        if (System.getProperty("os.name").indexOf("Windows") != -1){
            // Add an empty space to the dialog.
            topPanel.add(Box.createGlue(), BorderLayout.SOUTH);        
        }else{
            // Add add/remove panel to the dialog.   
            topPanel.add(addRemovePanel, BorderLayout.SOUTH);
        }
                        
        getContentPane().add(topPanel, BorderLayout.CENTER);
        
        JPanel decisionPanel = new JPanel();
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT, 10, 5));
        
        okBtn = new JButton(getMessage("common.ok_btn"));
        okBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okBtnActionPerformed(evt);
            }
        });
        okBtn.setMnemonic(ResourceManager.getVKCode("common.ok_btn.mnemonic"));
        decisionPanel.add(okBtn);

        cancelBtn = new JButton(getMessage("common.cancel_btn"));
        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                cancelBtnActionPerformed(evt);
            }
        };
        cancelBtn.addActionListener(cancelAction);
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke((char)KeyEvent.VK_ESCAPE),"cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);

        cancelBtn.setMnemonic(ResourceManager.getVKCode("common.cancel_btn.mnemonic"));
        decisionPanel.add(cancelBtn);
        
        getContentPane().add(decisionPanel, BorderLayout.SOUTH);

        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(okBtn);
        
        pack();
    }

    private void addBtnActionPerformed(ActionEvent evt) {
        // Add line to the table.
        PluginTableModel model = (PluginTableModel)jresTable.getModel();
        model.add(new PluginJavaInfo( null, null, false, ""), false, true);
        int row = model.getRowCount() - 1;
        jresTable.requestFocus();
        jresTable.setRowSelectionInterval(row, row);        
    } 
    
    private void removeBtnActionPerformed(ActionEvent evt) {
        // Remove currently highlighted line(s) if selected JREs are non-system JRE(s).  
        //If none of the lines selected, don't do anything.
        ((PluginTableModel)jresTable.getModel()).remove(jresTable.getSelectedRows());
    }  
    
    private void okBtnActionPerformed(ActionEvent evt) {
        //Apply changes and hide the dialog.
        // flushes any changes in the table.
	if (jresTable.isEditing()) {
	    jresTable.getCellEditor().stopCellEditing();
	}
        
        // Get table model.
        PluginTableModel model = (PluginTableModel)jresTable.getModel();
        
        if (model.getRowCount() >= 0 ) {
            // Clear list of jres.
	    PluginJavaInfo.clear();
            
	    // Add Plugin JREs from PluginTableModel:
	    for (int n = 0; n < model.getRowCount(); n++) {
                //Add JRE from table model to PluginJavaInfo.
	        PluginJavaInfo.addJRE((PluginJavaInfo)model.getJRE(n));
	    }
	}
        
        /*
         * This is to enable "Apply" button in Control Panel
         */
        ControlPanel.propertyHasChanged();
        setVisible(false);
        dispose();        
    }

    private void cancelBtnActionPerformed(ActionEvent evt) {
        //Close dialog, no changes are saved.
        setVisible(false);
        dispose();
    }


    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
        setVisible(false);
        dispose();
    }
    
    /*
     * Product Name column should have a combo box with two options:
     * JRE or J2SDK
     * to choose from.
     */
    private void setProductTableColumn (TableColumn c){
        //Set up the editor
        JComboBox comboBox = new JComboBox();
        comboBox.addItem(getMessage("jpi.jre.string"));
        comboBox.addItem(getMessage("jpi.jdk.string"));
        c.setCellEditor(new DefaultCellEditor(comboBox));

        //Set up tool tip
        DefaultTableCellRenderer renderer =
                new DefaultTableCellRenderer();
        renderer.setToolTipText(getMessage("jpi.jres.dialog.product.tooltip"));
        c.setCellRenderer(renderer);   
    }
    
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }    

    public void valueChanged(ListSelectionEvent listSelectionEvent) {
        enableButtons();
    }
    
    private void enableButtons(){
        int [] rows = jresTable.getSelectedRows();
        PluginTableModel model = (PluginTableModel)jresTable.getModel();
        
        for (int i = 0; i < rows.length; i++ ){
            // If any one of the selected JREs is system JRE, "Remove" button
            // should be disabled.
            if (model.getJRE(rows[i]).isSystemJRE()){
                removeBtn.setEnabled(false);
                break;
            }
            removeBtn.setEnabled(true);
        }        
    }
       
    private JTable jresTable;
    private JButton okBtn, cancelBtn, addBtn, removeBtn;

}

