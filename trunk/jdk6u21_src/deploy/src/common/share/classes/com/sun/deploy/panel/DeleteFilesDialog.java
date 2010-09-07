/*
 * @(#)DeleteFilesDialog.java	1.7 05/03/30
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import java.awt.FlowLayout;
import java.awt.BorderLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;
import javax.swing.BoxLayout;
import javax.swing.JPanel;
import javax.swing.JComponent;
import javax.swing.AbstractAction;
import javax.swing.KeyStroke;
import javax.swing.JLabel;
import javax.swing.JCheckBox;
import javax.swing.JDialog;
import javax.swing.BorderFactory;
import javax.swing.Box;
import javax.swing.JButton;
import com.sun.deploy.config.Config;
import com.sun.deploy.security.CredentialManager;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.DialogTemplate;


public class DeleteFilesDialog extends JDialog{    

    public DeleteFilesDialog(java.awt.Dialog parent) {
        super(parent, true);
        setTitle(getMessage("delete.files.dlg.title"));
        initComponents();
        setLocationRelativeTo(parent);

        setVisible(true);
    }
    
    private void initComponents(){
        /*
         * Create a panel with warning icon and description text area.
         */
        JPanel questionPanel = new JPanel();
        questionPanel.setLayout(new FlowLayout());
        questionPanel.setBorder(
                BorderFactory.createEmptyBorder(12, 12, 12, 12));
        
        // Create label with warning icon
        JLabel warningLbl = 
                new JLabel(getMessage("delete.files.dlg.temp_files"));
        warningLbl.setIcon(ResourceManager.getIcon("warning32.image"));
        warningLbl.setIconTextGap(12);
        
        questionPanel.add(warningLbl);
        
        JPanel checkBoxPanel = new JPanel();
        checkBoxPanel.setLayout(new BoxLayout(checkBoxPanel, BoxLayout.Y_AXIS));
        // Make sure that checkboxes are 12 pixels to the right from the text
        // are.  68 = 12 + 32 + 12 + 12
        checkBoxPanel.setBorder(BorderFactory.createEmptyBorder(0, 68, 24, 12));        
        
        applicationCheckBox = new JCheckBox(
		getMessage("delete.files.dlg.applications"));
        // See if caching is disabled in Config. If it is, do not select
        // checkbox for applets and applications, and show this checkbox
        // disabled, since java web start applications cannot be uninstalled
        // when caching is disabled.
        boolean enabled = Config.getBooleanProperty(Config.CACHE_ENABLED_KEY);        
        applicationCheckBox.setSelected(enabled);
        applicationCheckBox.setEnabled(enabled);
        String tooltipText = "";
        if (enabled){
            tooltipText = 
		getMessage("delete.files.dlg.applications.tooltip.enabled");
        }else{
            tooltipText = 
                getMessage("delete.files.dlg.applications.tooltip.disabled");
        }
        applicationCheckBox.setToolTipText(tooltipText);

        traceCheckBox = new JCheckBox(
		getMessage("delete.files.dlg.trace"));
        traceCheckBox.setSelected(true);
        traceCheckBox.setToolTipText(
		getMessage("delete.files.dlg.trace.tooltip"));

        checkBoxPanel.add(applicationCheckBox);
        checkBoxPanel.add(traceCheckBox);
        
        /*
         * Create panel with OK and Cancel buttons on it.
         */
        JPanel buttonsPanel = new JPanel();
        buttonsPanel.setLayout(new FlowLayout(FlowLayout.TRAILING));
        buttonsPanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 12, 0));
        
        JButton okBtn = new JButton(getMessage("common.ok_btn"));
        okBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okBtnActionPerformed(evt);
            }
        });
        
        JButton cancelBtn = new JButton(getMessage("common.cancel_btn"));
        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                cancelBtnActionPerformed(evt);
            }
        };
        cancelBtn.addActionListener(cancelAction);
        
        buttonsPanel.add(okBtn);
        buttonsPanel.add(Box.createHorizontalStrut(6));
        buttonsPanel.add(cancelBtn);
        buttonsPanel.add(Box.createHorizontalStrut(12));
        
        JButton [] btns = {okBtn, cancelBtn};
        DialogTemplate.resizeButtons( btns );     
        
        /*
         * Set cancel action and default button.
         */
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);
        getRootPane().setDefaultButton(okBtn);     
        
        getContentPane().setLayout(new BorderLayout());
        getContentPane().add(questionPanel, BorderLayout.NORTH);
        getContentPane().add(checkBoxPanel, BorderLayout.CENTER);
        getContentPane().add(buttonsPanel, BorderLayout.SOUTH);
        
        pack();
        setResizable(false);
    }
    
     
    /*
     * If user cancelled this dialog, just close it.
     */
    private void cancelBtnActionPerformed(ActionEvent evt) {
	setVisible(false);
    }
    
     
    /*
     * If user clicked "OK" button, remove temporary files.
     */
    private void okBtnActionPerformed(ActionEvent evt) {
	if (applicationCheckBox.isSelected()){
            // Delete all files in web start cache
            try {
            	ArrayList list = new ArrayList();
            	list.add(Config.getJavawsCommand());
            	list.add("-uninstall");
            	ProcessBuilder pb = new ProcessBuilder(list);
            	Process p = pb.start();
            	p.waitFor();
            } catch (IOException ioe) {
                //Trace.ignoredException(ioe);
            } catch (InterruptedException ie) {
            }
        }
        if (traceCheckBox.isSelected()) {
            // Delete all log and trace files.
            File dir = new File(Config.getLogDirectory());
            deleteFiles(dir);
	}
        
        /*
         * If iether one checkbox is checked, remove other temp files
         * stored on user's machine.
         */
        if (applicationCheckBox.isSelected() || traceCheckBox.isSelected()){        
            // Delete all other temporary stored files.
            // delete ext:
            File dir = new File(Config.getUserExtensionDirectory());
            deleteFiles(dir);
            // Delete cache/tmp:
            dir = new File(Config.getTempCacheDir());
            deleteFiles(dir);
            // Credential file is named auth.dat 
            // in {deployment.user.home}/security
            CredentialManager.removePersistantCredentials();
        }
        
        setVisible(false);
    }

    private void deleteFiles(File dir)
    {
	if (dir.exists() && dir.isDirectory()) {
	    File[] children = dir.listFiles();
	    for (int i = 0; i < children.length; i++) {
	    	if (children[i].isDirectory()) {
		    deleteFiles(children[i]);
	        }
	        children[i].delete();
	    }
	}
    }  
    
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }    

    private JCheckBox traceCheckBox, applicationCheckBox;
}
