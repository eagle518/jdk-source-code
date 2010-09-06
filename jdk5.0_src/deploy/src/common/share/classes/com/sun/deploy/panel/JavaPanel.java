/*
 * @(#)JavaPanel.java	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
        
        JPanel topPanel = new JPanel();        
	  topPanel.setLayout(new GridLayout(2, 1));

        JPanel jpiPanel = new JPanel();
        jpiPanel.setBorder(new TitledBorder(
	           new TitledBorder(new EtchedBorder()), 
	           getMessage("java.panel.plugin.border"), 
	           TitledBorder.DEFAULT_JUSTIFICATION, 
	           TitledBorder.DEFAULT_POSITION));

        jpiPanel.setLayout(new BorderLayout());

        jpiTextArea = new JSmartTextArea(getMessage("java.panel.plugin.text"));
       
        JPanel jpiBtnPanel = new JPanel();
        jpiBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        jpiSettingsBtn = new JButton(getMessage("java.panel.jpi_view_btn"));
        jpiSettingsBtn.setMnemonic(ResourceManager.getVKCode("java.panel.jpi_view_btn.mnemonic"));
        jpiSettingsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jpiSettingsBtnActionPerformed(evt);
            }
        });
        jpiSettingsBtn.setToolTipText(getMessage("java.panel.jpi_view_btn.tooltip"));
        jpiBtnPanel.add(jpiSettingsBtn);

        /*
         * Add textArea to the NORTH and button panel to the SOUTH.
         */
        jpiPanel.add(jpiTextArea, BorderLayout.NORTH);
        jpiPanel.add(jpiBtnPanel, BorderLayout.SOUTH);
        
        topPanel.add(jpiPanel);
        
        JPanel jwsPanel = new JPanel();
        jwsPanel.setBorder(new TitledBorder(
	           new TitledBorder(new EtchedBorder()), 
	           getMessage("java.panel.javaws.border"), 
	           TitledBorder.DEFAULT_JUSTIFICATION, 
	           TitledBorder.DEFAULT_POSITION));
        jwsPanel.setLayout(new BorderLayout());

        jwsTextArea = new JSmartTextArea(getMessage("java.panel.javaws.text"));
        
        JPanel jwsBtnPanel = new JPanel();
        jwsBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        jwsSettingsBtn = new JButton(getMessage("java.panel.javaws_view_btn"));
        jwsSettingsBtn.setMnemonic(ResourceManager.getVKCode("java.panel.javaws_view_btn.mnemonic"));
        jwsSettingsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jwsSettingsBtnActionPerformed(evt);
            }
        });
        jwsSettingsBtn.setToolTipText(getMessage("java.panel.javaws_view_btn.tooltip"));
        jwsBtnPanel.add(jwsSettingsBtn);
        
        /*
         * Add text area and button panel to jwsPanel
         */
        jwsPanel.add(jwsTextArea, BorderLayout.NORTH);
        jwsPanel.add(jwsBtnPanel, BorderLayout.SOUTH);
        
        topPanel.add(jwsPanel);
        
        add(topPanel, BorderLayout.CENTER);        
    }

    private void jpiSettingsBtnActionPerformed(ActionEvent evt) {
        PluginJreDialog jpiSettingsDialog = 
            new PluginJreDialog((JFrame) this.getTopLevelAncestor(), 
                               true);
        jpiSettingsDialog.setLocationRelativeTo(this);
        jpiSettingsDialog.setVisible(true);
    }

    private void jwsSettingsBtnActionPerformed(ActionEvent evt) {
        JavawsJreDialog jwsSettingsDialog = 
            new JavawsJreDialog((JFrame)this.getTopLevelAncestor(), 
                               true);
        jwsSettingsDialog.setLocationRelativeTo(this);
        jwsSettingsDialog.setVisible(true);
    }
        
    /*
     * Get String from resource file.
     */
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }        


    private JSmartTextArea jpiTextArea, jwsTextArea;
    private JButton jpiSettingsBtn, jwsSettingsBtn;

}
