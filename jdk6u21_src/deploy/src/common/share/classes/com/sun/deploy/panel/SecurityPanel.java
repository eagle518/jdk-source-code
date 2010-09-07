/*
 * @(#)SecurityPanel.java	1.14 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.JPanel;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;
import javax.swing.border.EtchedBorder;
import javax.swing.table.*;

import java.awt.Insets;
import java.awt.GridLayout;
import java.awt.BorderLayout;
import java.awt.FlowLayout;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import com.sun.deploy.resources.ResourceManager;

public class SecurityPanel extends JPanel{

    public SecurityPanel(){
        initComponents();
    }

    public void initComponents(){        
        //setLayout(new GridLayout(1, 1));
        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(new Insets(5, 5, 5, 5)));

        JPanel certificatesPanel = new JPanel();
        certificatesPanel.setLayout(new BorderLayout());
        certificatesPanel.setBorder(new TitledBorder(new TitledBorder(new EtchedBorder()),
                                                            getMessage("security.certificates.border.text"), 
                                                            TitledBorder.DEFAULT_JUSTIFICATION, 
                                                            TitledBorder.DEFAULT_POSITION));
        
        JPanel certsBtnPanel = new JPanel();
        certsBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        JButton certsBtn = new JButton(getMessage("security.certificates.button.text"));
        certsBtn.setMnemonic(ResourceManager.getVKCode("security.certificates.button.mnemonic"));
        certsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                certsBtnActionPerformed(evt);
            }
        });
        certsBtn.setToolTipText(getMessage("security.certs_btn.tooltip"));
        certsBtnPanel.add(certsBtn);

        JSmartTextArea certsPanelText = new JSmartTextArea(getMessage("security.certificates.desc.text"));
        certificatesPanel.add(certsPanelText, BorderLayout.NORTH);
        certificatesPanel.add(certsBtnPanel, BorderLayout.CENTER);

        add(certificatesPanel, BorderLayout.CENTER);

        /*
        JPanel policiesPanel = new JPanel();
        policiesPanel.setLayout(new BorderLayout());
        policiesPanel.setBorder(new TitledBorder(new TitledBorder(new EtchedBorder()), getMessage("security.policies.border.text"), TitledBorder.DEFAULT_JUSTIFICATION, TitledBorder.DEFAULT_POSITION));
        
        JPanel policiesBtnPanel = new JPanel();
        policiesBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        JButton advancedPolicyBtn = new JButton(getMessage("security.policies.advanced.text"));
        advancedPolicyBtn.setMnemonic(ResourceManager.getVKCode("security.policies.advanced.mnemonic"));
        advancedPolicyBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                advancedPolicyBtnActionPerformed(evt);
            }
        });
        policiesBtnPanel.add(advancedPolicyBtn);

        JSmartTextArea policiesPanelText = new JSmartTextArea(getMessage("security.policies.desc.text"));
        policiesPanel.add(policiesPanelText, BorderLayout.NORTH);
        policiesPanel.add(policiesBtnPanel, BorderLayout.CENTER);


        add(policiesPanel, BorderLayout.SOUTH);*/

    }

    private void certsBtnActionPerformed(ActionEvent evt) {
        //Open Certificates window.  Specify ControlPanel as a parent of the
        // CertificatesDialog to make it popup in front of ControlPanel when
        // it is maximized.
        CertificatesDialog certDialog = 
            new CertificatesDialog((JFrame) this.getTopLevelAncestor(), 
                                    true);
        certDialog.setLocationRelativeTo(this);
        certDialog.setVisible(true);
    }

   // private void advancedPolicyBtnActionPerformed(ActionEvent evt) {        
   // }

    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }
}

