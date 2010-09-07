/*
 * @(#)JavaPanel.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.*;
import javax.swing.border.*;
import javax.swing.table.*;
import java.awt.*;
import java.awt.event.*;
import com.sun.deploy.resources.ResourceManager;

public class JavaPanel extends JPanel{

    /** Creates new form JavaPanel */
    public JavaPanel() {
        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {
        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(new Insets(5, 5, 5, 5)));  
        
        JPanel jrePanel = new JPanel();
        jrePanel.setBorder(new TitledBorder(
	           new TitledBorder(new EtchedBorder()), 
	           getMessage("java.panel.jre.border"), 
	           TitledBorder.DEFAULT_JUSTIFICATION, 
	           TitledBorder.DEFAULT_POSITION));
        jrePanel.setLayout(new BorderLayout());

        jreTextArea = new JSmartTextArea(getMessage("java.panel.jre.text"));
        
        JPanel jreBtnPanel = new JPanel();
        jreBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        jreSettingsBtn = new JButton(getMessage("java.panel.jre_view_btn"));
        jreSettingsBtn.setMnemonic(ResourceManager.getVKCode("java.panel.jre_view_btn.mnemonic"));
        jreSettingsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jreSettingsBtnActionPerformed(evt);
            }
        });
        jreSettingsBtn.setToolTipText(getMessage("java.panel.jre_view_btn.tooltip"));
        jreBtnPanel.add(jreSettingsBtn);
        
        /*
         * Add text area and button panel to jrePanel
         */
        jrePanel.add(jreTextArea, BorderLayout.NORTH);
        jrePanel.add(jreBtnPanel, BorderLayout.CENTER);
        
        add(jrePanel, BorderLayout.CENTER);        
    }

    private void jreSettingsBtnActionPerformed(ActionEvent evt) {
        JreDialog jreSettingsDialog = 
            new JreDialog((JFrame)this.getTopLevelAncestor(), 
                               true);
        jreSettingsDialog.setLocationRelativeTo(this);
        jreSettingsDialog.setVisible(true);
    }
        
    /*
     * Get String from resource file.
     */
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }        


    private JSmartTextArea jreTextArea;
    private JButton jreSettingsBtn;

}
