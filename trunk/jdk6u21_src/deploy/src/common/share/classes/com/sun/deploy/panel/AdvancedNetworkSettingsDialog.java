/*
 * @(#)AdvancedNetworkSettingsDialog.java	1.24 10/03/24
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
import com.sun.deploy.config.Config;
import com.sun.deploy.panel.NetworkSettingsDialog;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.panel.NumberDocument;

public class AdvancedNetworkSettingsDialog extends JDialog {

    /*
     * Creates new form AdvancedNetworkSettingsDialog 
     */
    public AdvancedNetworkSettingsDialog(Dialog parent, boolean modal) {
        super(parent, modal);
        initComponents();
    }

    /*
     * This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {

        setTitle(getMessage("advanced.network.dlg.title"));

        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                closeDialog();
            }
        });
        
        /*
         * topPanel contains panel with "Servers" panel and
         * "Exceptions" panel.
         */
        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BorderLayout());        
        
        /*
         * upperPanel contains allTypesPanel and panel
         * with checkbox.
         */
        JPanel upperPanel = new JPanel();
        upperPanel.setLayout(new BorderLayout());
        upperPanel.setBorder(BorderFactory.createTitledBorder(
                             new EtchedBorder(),
	             getMessage("advanced.network.dlg.servers"),
	             TitledBorder.DEFAULT_JUSTIFICATION,
	             TitledBorder.DEFAULT_POSITION));

        /*
         * allTypesPanel has all proxy/port related info and 
         * consists of 
         *     -typesPanel
         *     -addressPanel
         *     -sepsPanel
         *     -portPanel
         */
        JPanel allTypesPanel = new JPanel();
        allTypesPanel.setLayout(new BoxLayout(allTypesPanel, BoxLayout.X_AXIS));

        /*
         * typesPanel
         */
        JPanel typesPanel = new JPanel();
        typesPanel.setLayout(new GridLayout(5, 1, 0, 5));

        JLabel typeLabel = new JLabel(getMessage("advanced.network.dlg.type"));
        typeLabel.setHorizontalAlignment(SwingConstants.CENTER);       
        JLabel httpLabel = new JLabel(getMessage("advanced.network.dlg.http"));
        JLabel secureLabel = new JLabel(getMessage("advanced.network.dlg.secure"));
        JLabel ftpLabel = new JLabel(getMessage("advanced.network.dlg.ftp"));
        JLabel socksLabel = new JLabel(getMessage("advanced.network.dlg.socks"));

        typesPanel.add(typeLabel);
        typesPanel.add(httpLabel);
        typesPanel.add(secureLabel);
        typesPanel.add(ftpLabel);
        typesPanel.add(socksLabel);

        allTypesPanel.add(Box.createHorizontalStrut(5));
        allTypesPanel.add(typesPanel);
        allTypesPanel.add(Box.createHorizontalStrut(5));


        /* addressPanel */
        JPanel addressPanel = new JPanel();
        addressPanel.setLayout(new GridLayout(5, 1, 0, 5));

        JLabel addressLabel = new JLabel(getMessage("advanced.network.dlg.proxy_address"));
        addressLabel.setHorizontalAlignment(SwingConstants.CENTER);       
        httpTextField = new JTextField(20);
        secureTextField = new JTextField(20);
        ftpTextField = new JTextField(20);
        socksTextField = new JTextField(20);

        addressPanel.add(addressLabel);
        addressPanel.add(httpTextField);
        addressPanel.add(secureTextField);
        addressPanel.add(ftpTextField);
        addressPanel.add(socksTextField);

        allTypesPanel.add(addressPanel);
        allTypesPanel.add(Box.createHorizontalStrut(5));

        /* sepsPanel  */
        JPanel sepsPanel = new JPanel();
        sepsPanel.setLayout(new GridLayout(5,1, 0, 5));
        JLabel sepsLabel1 = new JLabel(":");
        sepsLabel1.setHorizontalAlignment(SwingConstants.CENTER);
        JLabel sepsLabel2 = new JLabel(":");
        sepsLabel2.setHorizontalAlignment(SwingConstants.CENTER);
        JLabel sepsLabel3 = new JLabel(":");
        sepsLabel3.setHorizontalAlignment(SwingConstants.CENTER);
        JLabel sepsLabel4 = new JLabel(":");
        sepsLabel4.setHorizontalAlignment(SwingConstants.CENTER);
        sepsPanel.add(Box.createGlue());        
        sepsPanel.add(sepsLabel1);
        sepsPanel.add(sepsLabel2);
        sepsPanel.add(sepsLabel3);
        sepsPanel.add(sepsLabel4);

        allTypesPanel.add(sepsPanel);
        allTypesPanel.add(Box.createHorizontalStrut(5));

        /* portPanel */
        JPanel portPanel = new JPanel();
        portPanel.setLayout(new GridLayout(5, 1, 0, 5));
        JLabel portLabel = new JLabel(getMessage("advanced.network.dlg.port"));
        portLabel.setHorizontalAlignment(SwingConstants.CENTER);        
        httpPortTextField = new JTextField(6);
        httpPortTextField.setDocument(new NumberDocument());
        securePortTextField = new JTextField(6);
        securePortTextField.setDocument(new NumberDocument());
        ftpPortTextField = new JTextField(6);
        ftpPortTextField.setDocument(new NumberDocument());
        socksPortTextField = new JTextField(6);
        socksPortTextField.setDocument(new NumberDocument());
        portPanel.add(portLabel);
        portPanel.add(httpPortTextField);
        portPanel.add(securePortTextField);
        portPanel.add(ftpPortTextField);
        portPanel.add(socksPortTextField);

        allTypesPanel.add(portPanel);
        allTypesPanel.add(Box.createHorizontalStrut(5));

        /*
         * checkboxPanel
         */
        JPanel checkboxPanel = new JPanel();
        checkboxPanel.setLayout(new FlowLayout(FlowLayout.CENTER));
        useForAllChBox = new JCheckBox(getMessage("advanced.network.dlg.same_proxy"));
        useForAllChBox.setSelected(false);
        useForAllChBox.setMnemonic(ResourceManager.getVKCode("advanced.network.dlg.same_proxy.mnemonic"));
        useForAllChBox.addItemListener(new ItemListener() {
            public void itemStateChanged(ItemEvent ie){
		useForAllChBoxItemStateChanged(ie);
            }
        });
        checkboxPanel.add(useForAllChBox);

        upperPanel.add(allTypesPanel, BorderLayout.CENTER);
        upperPanel.add(checkboxPanel, BorderLayout.SOUTH);
        
        topPanel.add(upperPanel, BorderLayout.NORTH);
        

        
        /* 
         * Construct "Exceptions" subpanel.
         */
        JPanel exceptionsPanel = new JPanel();
        exceptionsPanel.setLayout(new BorderLayout());
        exceptionsPanel.setBorder(BorderFactory.createTitledBorder(
                                  new EtchedBorder(),
		  getMessage("advanced.network.dlg.exceptions"),
		  TitledBorder.DEFAULT_JUSTIFICATION,
		  TitledBorder.DEFAULT_POSITION));

        JLabel topLineLabel = new JLabel(getMessage("advanced.network.dlg.no_proxy"));

        bypassTextArea = new JTextArea(3, 1);
 	  bypassTextArea.setFont(ResourceManager.getUIFont());
        bypassTextArea.setLineWrap(true);
        
        JScrollPane jsp = new JScrollPane();
        jsp.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        jsp.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        jsp.setAutoscrolls(true);
        jsp.setViewportView(bypassTextArea);
        
        
        JLabel bottomLabel = new JLabel(getMessage("advanced.network.dlg.no_proxy_note"));

        /*
         * construct componentsPanel with 
         *    - topLineLabel
         *    - jsp
         *    - bottomLabel
         */
        JPanel componentsPanel = new JPanel();
        componentsPanel.setLayout(new BorderLayout());
        componentsPanel.add(topLineLabel, BorderLayout.NORTH);
        componentsPanel.add(jsp, BorderLayout.CENTER);
        componentsPanel.add(bottomLabel, BorderLayout.SOUTH);
        
        exceptionsPanel.add(Box.createHorizontalStrut(10), BorderLayout.WEST);
        exceptionsPanel.add(componentsPanel, BorderLayout.CENTER);
        exceptionsPanel.add(Box.createHorizontalStrut(10), BorderLayout.EAST);
        exceptionsPanel.add(Box.createVerticalStrut(5),  BorderLayout.SOUTH);

        topPanel.add(exceptionsPanel, BorderLayout.CENTER);
        
        /*
         * decisionPanel contains "OK" and "Cancel" buttons.
         */
        JPanel decisionPanel = new JPanel();
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        
        okButton = new JButton(getMessage("common.ok_btn"));
        okButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okButtonActionPerformed(evt);
            }
        });
        
        cancelButton = new JButton(getMessage("common.cancel_btn"));

        JButton [] btns = {okButton, cancelButton};
        com.sun.deploy.ui.DialogTemplate.resizeButtons( btns );     
        
        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                closeDialog();
            }
        };
        cancelButton.addActionListener(cancelAction);
        
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);
        
        decisionPanel.add(okButton);
        decisionPanel.add(cancelButton);
        
        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(okButton);        
        
        /*
         * Add decisionPanel to the bottom of topPanel.
         */
        topPanel.add(decisionPanel, BorderLayout.SOUTH);

        getContentPane().add(topPanel);
        
        pack();
        setValues();
	
	// Make the dialog not resizable
	setResizable(false);
        
        // Set focus sequence for the elements.
        setFocusSequence();
    }

    private void setValues(){                
        boolean use_for_all;
        //
        // Get proxy address/port from parent if parent is NetworkSettingsDialog.
        // or if parent is null, or not an instance of NetworkSettingsDialog,
        // get the proxy address/port info from Config.
        //        
        if (getParent() != null && (getParent() instanceof NetworkSettingsDialog)){
            getValuesFromParent();
            use_for_all = ((NetworkSettingsDialog)getParent()).getUseSameProxy();
            useForAllChBox.setSelected(use_for_all);
        }else{                
            getValuesFromConfig();
            use_for_all = Config.getBooleanProperty(Config.PROX_SAME_KEY);
            useForAllChBox.setSelected(use_for_all);            
        }
        
        useForAllChBox.setEnabled(!Config.isLocked(Config.PROX_SAME_KEY));  
        bypassTextArea.setEditable(!Config.isLocked(Config.PROX_BYPASS_KEY));
        
        setUseForAll(use_for_all);        
    }
    
    private void getValuesFromParent(){
        NetworkSettingsDialog parent = (NetworkSettingsDialog)getParent(); 
        
        //
        // Get values only if they have not been set yet.  We are going through this
        // every time "Same proxy for all" checkbox is cheched/unchecked.  In order 
        // not to overwrite new values in this dialog with values from parent, retrieve
        // values from parent only if they have not been set yet.
        //
        if (httpTextField.getText().trim().equals("")){
            httpTextField.setText(parent.getProxyAddressField(Config.PROX_HTTP_HOST_KEY));
        }
        if (httpPortTextField.getText().trim().equals("")){
            httpPortTextField.setText(parent.getProxyPortField(Config.PROX_HTTP_PORT_KEY));
        }                
        if (secureTextField.getText().trim().equals("")){
            secureTextField.setText(parent.getProxyAddressField(Config.PROX_HTTPS_HOST_KEY));
        }
        if (securePortTextField.getText().trim().equals("")){
            securePortTextField.setText(parent.getProxyPortField(Config.PROX_HTTPS_PORT_KEY));
        }
        if (ftpTextField.getText().trim().equals("")){
            ftpTextField.setText(parent.getProxyAddressField(Config.PROX_FTP_HOST_KEY));
        }
        if (ftpPortTextField.getText().trim().equals("")){
            ftpPortTextField.setText(parent.getProxyPortField(Config.PROX_FTP_PORT_KEY));            
        }                
        if (socksTextField.getText().trim().equals("")){
            socksTextField.setText(parent.getProxyAddressField(Config.PROX_SOX_HOST_KEY));
        }
        if (socksPortTextField.getText().trim().equals("")){
            socksPortTextField.setText(parent.getProxyPortField(Config.PROX_SOX_PORT_KEY));
        }
        if (bypassTextArea.getText().trim().equals("")){
            bypassTextArea.setText(parent.getBypassString());
        }
    }
    
    private void getValuesFromConfig(){
        //
        // Get values only if they have not been set yet.  We are going through this
        // every time "Same proxy for all" checkbox is cheched/unchecked.  In order 
        // not to overwrite new values in this dialog with values from config, retrieve
        // values from config only if they have not been set yet.
        //
        if (httpTextField.getText().trim().equals("")){
            httpTextField.setText(Config.getProperty(Config.PROX_HTTP_HOST_KEY));
        }
        if (httpPortTextField.getText().trim().equals("")){
            httpPortTextField.setText(Config.getProperty(Config.PROX_HTTP_PORT_KEY));
        }
        if (secureTextField.getText().trim().equals("")){
            secureTextField.setText(Config.getProperty(Config.PROX_HTTPS_HOST_KEY));
        }
        if (securePortTextField.getText().trim().equals("")){
            securePortTextField.setText(Config.getProperty(Config.PROX_HTTPS_PORT_KEY));
        }
        if (ftpTextField.getText().trim().equals("")){
            ftpTextField.setText(Config.getProperty(Config.PROX_FTP_HOST_KEY));
        }
        if (ftpPortTextField.getText().trim().equals("")){
            ftpPortTextField.setText(Config.getProperty(Config.PROX_FTP_PORT_KEY));
        }
        if (socksTextField.getText().trim().equals("")){
            socksTextField.setText(Config.getProperty(Config.PROX_SOX_HOST_KEY));
        }
        if (socksPortTextField.getText().trim().equals("")){
            socksPortTextField.setText(Config.getProperty(Config.PROX_SOX_PORT_KEY));
        }
        if (bypassTextArea.getText().trim().equals("")){
            bypassTextArea.setText(Config.getProperty(Config.PROX_BYPASS_KEY));  
        }
    }
    
    
    private void setFocusSequence(){
        httpTextField.setNextFocusableComponent(httpPortTextField);
        secureTextField.setNextFocusableComponent(securePortTextField);
        ftpTextField.setNextFocusableComponent(ftpPortTextField);
        socksTextField.setNextFocusableComponent(socksPortTextField);
        
        httpPortTextField.setNextFocusableComponent(secureTextField);
        securePortTextField.setNextFocusableComponent(ftpTextField);
        ftpPortTextField.setNextFocusableComponent(socksTextField); 
        
        bypassTextArea.setNextFocusableComponent(okButton);
    }        
    
    private void setComponent(JComponent cmp, boolean value){
        if (cmp instanceof JLabel || cmp instanceof JCheckBox || cmp instanceof JButton){
            cmp.setEnabled(value);
        }
        else if (cmp instanceof JTextField){
            cmp.setEnabled(value);
            ((JTextField)cmp).setEditable(value);            
        }
    }    


    private void okButtonActionPerformed(ActionEvent evt) {
        
        /*
         * If user made any changes to properties in this dialog, make sure
         * to update NetworkSettingsDialog.  NetworkSettingsDialog will save
         * properties in Config.
         *
         * If parent of this dialog is NOT NetworkSettingsDialog, then
         * save changes to Config.
         */
        if (this.getParent() instanceof NetworkSettingsDialog){
            ((NetworkSettingsDialog)this.getParent()).updateProxyInfo(            
                httpTextField.getText(), httpPortTextField.getText(),
                secureTextField.getText(), securePortTextField.getText(),
                ftpTextField.getText(), ftpPortTextField.getText(),
                socksTextField.getText(), socksPortTextField.getText(), 
                useForAllChBox.isSelected(), bypassTextArea.getText());
        }else{
            savePropertiesInConfig();
        }
        
        // Close this dialog.
        setVisible(false);
        dispose();
    }
    
    private void savePropertiesInConfig(){
        // Save values set - pass them to the Config;
        Config.setProperty(Config.PROX_HTTP_HOST_KEY, httpTextField.getText());
        Config.setProperty(Config.PROX_HTTP_PORT_KEY, httpPortTextField.getText());
        Config.setProperty(Config.PROX_HTTPS_HOST_KEY, secureTextField.getText());
        Config.setProperty(Config.PROX_HTTPS_PORT_KEY, securePortTextField.getText());
        Config.setProperty(Config.PROX_FTP_HOST_KEY, ftpTextField.getText());
        Config.setProperty(Config.PROX_FTP_PORT_KEY, ftpPortTextField.getText());
        Config.setProperty(Config.PROX_SOX_HOST_KEY, socksTextField.getText());
        Config.setProperty(Config.PROX_SOX_PORT_KEY, socksPortTextField.getText());  
        Config.setBooleanProperty(Config.PROX_SAME_KEY, useForAllChBox.isSelected());
        Config.setProperty(Config.PROX_BYPASS_KEY, bypassTextArea.getText());    
    }

    private void useForAllChBoxItemStateChanged(ItemEvent evt){
        // Preserve current values in the text field in case if user unchecks
        // the checkbox again:
        if (useForAllChBox.isSelected()){
            lastFtpProxy    = ftpTextField.getText();
            lastFtpPort     = ftpPortTextField.getText();
            lastSecureProxy = secureTextField.getText();
            lastSecurePort  = securePortTextField.getText();
        }else{
            // Restore preserved values:
            secureTextField.setText(lastSecureProxy);
            securePortTextField.setText(lastSecurePort);
            ftpTextField.setText(lastFtpProxy);
            ftpPortTextField.setText(lastFtpPort); 
        }
        setUseForAll(useForAllChBox.isSelected());        
    }

    private void setUseForAll(boolean use_for_all) {   
        //
        // Get proxy address/port from parent if parent is NetworkSettingsDialog.
        // or if parent is null, or not an instance of NetworkSettingsDialog,
        // get the proxy address/port info from Config.
        //
        if (getParent() != null && (getParent() instanceof NetworkSettingsDialog)){
            getValuesFromParent();
        }else{                
            getValuesFromConfig();
        }
        
        setTextFields(!use_for_all);
	if (use_for_all) {            
	    /*
	     * If we should use the same proxy address/port for all protocols,
	     * set the values for https/ftp to be the same as for http.
	     */            
            secureTextField.setText(httpTextField.getText());
            securePortTextField.setText(httpPortTextField.getText());
            ftpTextField.setText(httpTextField.getText());
            ftpPortTextField.setText(httpPortTextField.getText());           
	}
    }

    private void setTextFields(boolean state){
        // http proxy/port
	boolean enab = !Config.isLocked(Config.PROX_HTTP_HOST_KEY);
        httpTextField.setEnabled(enab);
        httpTextField.setEditable(enab);
            
        enab = !Config.isLocked(Config.PROX_HTTP_PORT_KEY);
        httpPortTextField.setEnabled(enab);
        httpPortTextField.setEditable(enab);        
        
        // https proxy/port
        enab = state && !Config.isLocked(Config.PROX_HTTPS_HOST_KEY);
        secureTextField.setEnabled(enab);
        secureTextField.setEditable(enab);
	
	enab = state && !Config.isLocked(Config.PROX_HTTPS_PORT_KEY);
        securePortTextField.setEnabled(enab); 
        securePortTextField.setEditable(enab);

        // ftp proxy/port
	enab = state && !Config.isLocked(Config.PROX_FTP_HOST_KEY);
        ftpTextField.setEnabled(enab); 
        ftpTextField.setEditable(enab);

        enab = state && !Config.isLocked(Config.PROX_FTP_PORT_KEY);
        ftpPortTextField.setEnabled(enab);
        ftpPortTextField.setEditable(enab);
        
        // socks proxy/port
        enab = !Config.isLocked(Config.PROX_SOX_HOST_KEY);
        socksTextField.setEnabled(enab);
        socksTextField.setEditable(enab);
        
        enab = !Config.isLocked(Config.PROX_SOX_PORT_KEY);
        socksPortTextField.setEnabled(enab);
        socksPortTextField.setEditable(enab);
    }


    /** Closes the dialog */
    private void closeDialog() {
        setVisible(false);
        dispose();
    }
    
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }     

    private JTextArea bypassTextArea;
    private JTextField httpTextField, secureTextField, ftpTextField, 
            socksTextField, httpPortTextField, securePortTextField, 
            ftpPortTextField, socksPortTextField;
    private JButton okButton, cancelButton;
    private JCheckBox  useForAllChBox;
    
    private String lastFtpProxy, lastFtpPort, lastSecureProxy, lastSecurePort;    
}
