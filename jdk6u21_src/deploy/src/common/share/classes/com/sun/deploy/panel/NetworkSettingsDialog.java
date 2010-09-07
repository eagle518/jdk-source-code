/*
 * @(#)NetworkSettingsDialog.java	1.27 10/03/24
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
import com.sun.deploy.panel.AdvancedNetworkSettingsDialog;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UITextArea;

public class NetworkSettingsDialog extends JDialog 
                implements ActionListener{

    NetworkSettingsDialog (JFrame parent, boolean modal) {
        super(parent, modal);
        initComponents();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     */
    private void initComponents() {
        proxySettingsButtonGroup = new ButtonGroup();     
        browserRbutton = new JRadioButton();           
        manualRbutton = new JRadioButton();                
        bypassProxyChbox = new JCheckBox();                
        autoConfigRbutton = new JRadioButton();  
	directRbutton = new JRadioButton();      
        
        setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        setTitle(getMessage("network.settings.dlg.title"));
        setModal(true);
        addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent evt) {
                closeDialog(evt);
            }
        });
        
        JPanel proxySettingsPanel = new JPanel();
        proxySettingsPanel.setLayout(new BorderLayout());
        
        proxySettingsPanel.setBorder(new TitledBorder(new TitledBorder(new EtchedBorder()), 
                                     getMessage("network.settings.dlg.border_title"), 
                                     TitledBorder.DEFAULT_JUSTIFICATION, 
                                     TitledBorder.DEFAULT_POSITION));

        descriptionTextArea = new UITextArea(); 
        JPanel descriptionPanel = new JPanel();
        descriptionPanel.setLayout(new BorderLayout());
        descriptionPanel.add(descriptionTextArea, BorderLayout.NORTH);
        descriptionPanel.add(new JLabel(" "), BorderLayout.CENTER);
        proxySettingsPanel.add(descriptionPanel, BorderLayout.NORTH);
        
        JPanel rbuttonsPanel = new JPanel();
        rbuttonsPanel.setLayout(new BoxLayout(rbuttonsPanel, BoxLayout.Y_AXIS));
        
        JPanel browserSettingsPanel = new JPanel();
        browserSettingsPanel.setLayout(new BorderLayout());
        
        browserRbutton.setText(getMessage("network.settings.dlg.browser_rbtn"));
        browserRbutton.setMnemonic(ResourceManager.getVKCode("browser_rbtn.mnemonic"));
        proxySettingsButtonGroup.add(browserRbutton);
        browserSettingsPanel.add(browserRbutton, BorderLayout.NORTH);
        
        rbuttonsPanel.add(browserSettingsPanel);
        
        /*
         * manualSettingsPanel contains:
         *    - radio button (NORTH)
         *    - addressPortPanel with         
         *        * address label
         *        * address text field
         *        * port label
         *        * port text field 
         *    - advancedBtnPanel with
         *        * advanced button.
         *    - proxyChboxPanel with
         *        * checkbox
         */  
        JPanel manualSettingsPanel = new JPanel();
        manualSettingsPanel.setLayout(new BorderLayout());

        manualRbutton.setText(getMessage("network.settings.dlg.manual_rbtn"));
        manualRbutton.setMnemonic(ResourceManager.getVKCode("manual_rbtn.mnemonic"));
        proxySettingsButtonGroup.add(manualRbutton);
        manualSettingsPanel.add(manualRbutton, BorderLayout.NORTH);

        JPanel addressPortPanel = new JPanel();
        addressPortPanel.add(Box.createRigidArea(new Dimension(20, 1)));

        addressLabel = new JLabel (getMessage("network.settings.dlg.address_lbl"));
        addressPortPanel.add(addressLabel); 

        addressTextField = new JTextField("");
        addressTextField.setColumns(10);   
     
        addressPortPanel.add(addressTextField);

        addressPortPanel.add(Box.createGlue());

        portLabel = new JLabel(getMessage("network.settings.dlg.port_lbl"));
        addressPortPanel.add(portLabel);

        portTextField = new JTextField("");
        portTextField.setColumns(3);
        portTextField.setDocument(new NumberDocument());
        addressPortPanel.add(portTextField);

        addressPortPanel.add(Box.createGlue());  

        manualSettingsPanel.add(addressPortPanel, BorderLayout.WEST);

        JPanel advancedBtnPanel = new JPanel();
        
        advancedBtn = makeButton("network.settings.dlg.advanced_btn");  
        advancedBtn.setMnemonic(ResourceManager.getVKCode(
                "network.settings.dlg.advanced_btn.mnemonic"));
        advancedBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                advancedBtnActionPerformed(evt);
            }
        });
        advancedBtn.setToolTipText(getMessage("network.settings.advanced_btn.tooltip"));
        advancedBtnPanel.add(advancedBtn);

        manualSettingsPanel.add(advancedBtnPanel, BorderLayout.EAST);
        
        JPanel proxyChboxPanel = new JPanel();
        proxyChboxPanel.setLayout(new BoxLayout(proxyChboxPanel, BoxLayout.X_AXIS));
        proxyChboxPanel.add(Box.createRigidArea(new Dimension(20, 1)));

        bypassProxyChbox.setText(getMessage("network.settings.dlg.bypass_text"));
        bypassProxyChbox.setMnemonic(ResourceManager.getVKCode("network.settings.dlg.bypass.mnemonic"));
        proxyChboxPanel.add(bypassProxyChbox);
        proxyChboxPanel.add(Box.createGlue());

        manualSettingsPanel.add(proxyChboxPanel, BorderLayout.SOUTH);
        /*
         **** end of manualSettingsPanel construction.****
         */
        
        rbuttonsPanel.add(manualSettingsPanel);
        
        /*
         * this is the panel with Automatic proxy settings.
         * autoSettingsPanel consists of: 
         *    - radio button 
         *    - autoScriptLocationPanel with 
         *          * label 
         *          * text field
         */
        JPanel autoSettingsPanel = new JPanel();
        autoSettingsPanel.setLayout(new BorderLayout());
        
        autoConfigRbutton.setText(getMessage("network.settings.dlg.autoconfig_rbtn"));
        autoConfigRbutton.setMnemonic(ResourceManager.getVKCode("autoconfig_rbtn.mnemonic"));
        proxySettingsButtonGroup.add(autoConfigRbutton);
        autoSettingsPanel.add(autoConfigRbutton, BorderLayout.NORTH);
        
        JPanel autoScriptLocationPanel = new JPanel();
        autoScriptLocationPanel.setLayout(new FlowLayout(FlowLayout.LEFT));
        
        locationLabel = new JLabel(getMessage("network.settings.dlg.location_lbl"));
        autoScriptLocationPanel.add(Box.createHorizontalStrut(20));
        autoScriptLocationPanel.add(locationLabel);
        
        locationTextField = new JTextField("");
        locationTextField.setColumns(20);
	locationTextField.setNextFocusableComponent(directRbutton);
        autoScriptLocationPanel.add(locationTextField);
        
        autoSettingsPanel.add(autoScriptLocationPanel, BorderLayout.CENTER);
        
        rbuttonsPanel.add(autoSettingsPanel);

        /*
         * Construct panel with "No proxy" radio button.
         */
        JPanel directConnectionPanel = new JPanel();
        directConnectionPanel.setLayout(new BorderLayout());
        
        directRbutton.setText(getMessage("network.settings.dlg.direct_rbtn"));
        directRbutton.setMnemonic(ResourceManager.getVKCode("direct_rbtn.mnemonic"));
        proxySettingsButtonGroup.add(directRbutton);
        directConnectionPanel.add(directRbutton, BorderLayout.NORTH);

        rbuttonsPanel.add(directConnectionPanel);
        
        proxySettingsPanel.add(rbuttonsPanel, BorderLayout.CENTER);
        
        getContentPane().add(proxySettingsPanel, BorderLayout.CENTER);
        
        /* 
         * Construct decisionPanel with "OK" and "Cancel" buttons.
         */
        JPanel decisionPanel = new JPanel();
        decisionPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        
        okButton = makeButton("common.ok_btn");
        okButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okBtnActionPerformed(evt);
            }
        });
        decisionPanel.add(okButton);

        cancelButton = makeButton("common.cancel_btn");
        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                cancelBtnActionPerformed(evt);
            }
        };
        cancelButton.addActionListener(cancelAction);
        getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
            KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0), "cancel");
        getRootPane().getActionMap().put("cancel", cancelAction);
        
        JButton [] btns = {okButton, cancelButton};
        com.sun.deploy.ui.DialogTemplate.resizeButtons( btns );     

        decisionPanel.add(cancelButton);
        
        getContentPane().add(decisionPanel, BorderLayout.SOUTH);
        
        /* Add listener to radion buttons. */
        browserRbutton.addActionListener(this);
        browserRbutton.setActionCommand("useBrowser");
        manualRbutton.addActionListener(this);
        manualRbutton.setActionCommand("useProxy");
        autoConfigRbutton.addActionListener(this);
        autoConfigRbutton.setActionCommand("useScript");
        directRbutton.addActionListener(this);
        directRbutton.setActionCommand("noProxy");

        // Set "OK" to be default button.  This means when user
        // presses "Enter" on the keyboard, "OK" button will be pressed.
        getRootPane().setDefaultButton(okButton);
        
        setValues();        
        pack();
        
        // Dialog should not be resizable.
        setResizable(false);
    }

    private void setValues(){
        addressTextField.setText(Config.getProperty(Config.PROX_HTTP_HOST_KEY));
        portTextField.setText(Config.getProperty(Config.PROX_HTTP_PORT_KEY));
        locationTextField.setText(Config.getProperty(Config.PROX_AUTOCFG_KEY));
        bypassProxyChbox.setSelected(Config.getBooleanProperty(Config.PROX_LOCAL_KEY));
        same_for_all = Config.getBooleanProperty(Config.PROX_SAME_KEY);
        
        /*
         * Get all the strings for description text area and find the longest
         * string. Use that string to set text for the text area and find out
         * preferred size of the text area with longest string.  Set Preferred
         * size of the text area to preferred size of the text area with
         * longest string.
         */
        String [] description = { 
            getMessage("network.settings.dlg.browser_text"),
            getMessage("network.settings.dlg.proxy_text"),
            getMessage("network.settings.dlg.auto_text"),
            getMessage("network.settings.dlg.none_text") };
            
        String longest = "";
        for ( int i = 0; i < description.length; i++ ) {
            if (description[ i ].length() > longest.length()){
                longest = description[ i ];
            }
        }
        descriptionTextArea.setText(longest);
        descriptionTextArea.setSize(
                descriptionTextArea.getPreferredSize());
        //fix textarea preferred size, it wont calulate again.
        descriptionTextArea.setPreferredSize(
                descriptionTextArea.getPreferredSize());
        
        /*
         * Get connection type and set description text:  
         */
        int connection = Config.getIntProperty(Config.PROX_TYPE_KEY);
	boolean enab = !Config.isLocked(Config.PROX_TYPE_KEY);
        switch ( connection ){
            case Config.PROX_TYPE_MANUAL:
                manualRbutton.setSelected(true);
                useProxy();  
                break;
            case Config.PROX_TYPE_AUTO:
                autoConfigRbutton.setSelected(true);
                useScript();        
                break;
            case Config.PROX_TYPE_NONE:
                directRbutton.setSelected(true);
                noProxy();
                break;
            default:
                browserRbutton.setSelected(true);
                useBrowser();                        
                break;
        }
	manualRbutton.setEnabled(enab);
	autoConfigRbutton.setEnabled(enab);
	directRbutton.setEnabled(enab);
	browserRbutton.setEnabled(enab);
        
        /*
         * Get values from Config for https/ftp/sox protocols and
         * bypass proxy list.  
         */
        https_host = Config.getProperty(Config.PROX_HTTPS_HOST_KEY); 
        https_port = Config.getProperty(Config.PROX_HTTPS_PORT_KEY);
        ftp_host = Config.getProperty(Config.PROX_FTP_HOST_KEY);
        ftp_port = Config.getProperty(Config.PROX_FTP_PORT_KEY);           
        socks_host = Config.getProperty(Config.PROX_SOX_HOST_KEY);
        socks_port = Config.getProperty(Config.PROX_SOX_PORT_KEY);
        bypass = Config.getProperty(Config.PROX_BYPASS_KEY);        
    }
    
    
    /*
     * This method is called when child of this dialog - AdvancedNetworkSettingsDialog -
     * is closed and http address/port information fields in this dialog should be updated
     * if they have been changed through the advanced settings dialog.
     */
    public void updateProxyInfo(String http_host, String http_port,
                                String https_host, String https_port,
                                String ftp_host, String ftp_port,
                                String socks_host, String socks_port,
                                boolean same_for_all, String bypass){
        addressTextField.setText(http_host);
        portTextField.setText(http_port);        
        
        // Cache settings from AdvancedNetworkSettingsDialog locally:
        this.http_host = http_host;
        this.http_port = http_port;
        this.https_host = https_host;
        this.https_port = https_port;
        this.ftp_host = ftp_host;
        this.ftp_port = ftp_port;
        
        this.socks_host = socks_host;
        this.socks_port = socks_port;
        this.same_for_all = same_for_all;
        this.bypass = bypass;
    }
    
    /*
     * This method is accessed by AdvancedNetworkSettingsDailog in order to get the 
     * current address for the proxy.
     */
    public String getProxyAddressField(String protocol){
        if (protocol.equals(Config.PROX_SOX_HOST_KEY)){
            return socks_host;
        }else if (protocol.equals(Config.PROX_HTTPS_HOST_KEY)){
            return https_host;
        }else if (protocol.equals(Config.PROX_FTP_HOST_KEY)){
            return ftp_host;
        }
        return addressTextField.getText();                
    }
    
    /*
     * This method is accessed by AdvancedNetworkSettingsDailog in order to get the
     * current port for the proxy.
     */
    public String getProxyPortField(String protocol){
        if (protocol.equals(Config.PROX_SOX_PORT_KEY)){
            return socks_port;
        }else if (protocol.equals(Config.PROX_HTTPS_PORT_KEY)){
            return https_port;
        }else if (protocol.equals(Config.PROX_FTP_PORT_KEY)){
            return ftp_port;
        }
        return portTextField.getText();
    }
    
    public boolean getUseSameProxy(){
        return same_for_all;
    }
    
    public String getBypassString(){
        return bypass;
    }


    private void disableAll() {
        addressLabel.setEnabled(false);
        addressTextField.setEnabled(false);
        portLabel.setEnabled(false);
        portTextField.setEnabled(false);
        advancedBtn.setEnabled(false);
        bypassProxyChbox.setEnabled(false);
        locationLabel.setEnabled(false);
        locationTextField.setEnabled(false);

        addressTextField.setEditable(false);
        locationTextField.setEditable(false);
        portTextField.setEditable(false);
    }

    private void useBrowser(){
        descriptionTextArea.setText(getMessage("network.settings.dlg.browser_text"));
	pack();
	disableAll();
    }

    private void useProxy(){
        descriptionTextArea.setText(getMessage("network.settings.dlg.proxy_text"));
	disableAll();
	if (!Config.isLocked(Config.PROX_HTTP_HOST_KEY)) {
	    addressLabel.setEnabled(true);
	    addressTextField.setEnabled(true);
	    addressTextField.setEditable(true);
	}
	if (!Config.isLocked(Config.PROX_HTTP_PORT_KEY)) {
	    portLabel.setEnabled(true);
	    portTextField.setEnabled(true);
	    portTextField.setEditable(true);
	}
        advancedBtn.setEnabled(true);
	if (!Config.isLocked(Config.PROX_LOCAL_KEY)) {
            bypassProxyChbox.setEnabled(true);
	}
    }

    private void useScript(){
        descriptionTextArea.setText(getMessage("network.settings.dlg.auto_text"));
	disableAll();
	if (!Config.isLocked(Config.PROX_AUTOCFG_KEY)) {
	    locationLabel.setEnabled(true);
	    locationTextField.setEnabled(true);
	    locationTextField.setEditable(true);
	}
    }

    private void noProxy(){
        descriptionTextArea.setText(getMessage("network.settings.dlg.none_text"));
	disableAll();
    }

    public void actionPerformed(ActionEvent e){
        String event = e.getActionCommand();
        if (event.equalsIgnoreCase("useBrowser")){
            useBrowser();
        }else if(event.equalsIgnoreCase("useProxy")){
            useProxy();
        }else if(event.equalsIgnoreCase("useScript")){
            useScript();
        }else if(event.equalsIgnoreCase("noProxy")){
            noProxy();
        }
    }

    private void advancedBtnActionPerformed(ActionEvent evt) {
        //Open Proxy setup dialog
        AdvancedNetworkSettingsDialog proxyDialog = new AdvancedNetworkSettingsDialog((Dialog)this, true);
        proxyDialog.setLocationRelativeTo(this);

        proxyDialog.setVisible(true);
        
    }

    private void okBtnActionPerformed(ActionEvent evt) {
        // Save values set - pass them to the Config;
        // Close this dialog
        /* Config.save(....); */
        if (browserRbutton.isSelected()){
            Config.setIntProperty(Config.PROX_TYPE_KEY, Config.PROX_TYPE_BROWSER);
        } else if (manualRbutton.isSelected()){
            Config.setIntProperty(Config.PROX_TYPE_KEY, Config.PROX_TYPE_MANUAL);
        } else if (autoConfigRbutton.isSelected()){
            Config.setIntProperty(Config.PROX_TYPE_KEY, Config.PROX_TYPE_AUTO);
        } else if (directRbutton.isSelected()){
            Config.setIntProperty(Config.PROX_TYPE_KEY, Config.PROX_TYPE_NONE);
        }
        Config.setProperty( Config.PROX_AUTOCFG_KEY, locationTextField.getText() );
        
        /*
         * If "Use same proxy for all protocols" is checked, save https and ftp  
         * proxy host/port combinations to be the same as http proxy host/port.
         * If it is not checked, save all addresses/ports separately.
         */
        if (same_for_all){
            Config.setProperty(Config.PROX_HTTP_HOST_KEY, addressTextField.getText());
            Config.setProperty(Config.PROX_HTTP_PORT_KEY, portTextField.getText());
            Config.setProperty(Config.PROX_HTTPS_HOST_KEY, addressTextField.getText());
            Config.setProperty(Config.PROX_HTTPS_PORT_KEY, portTextField.getText());
            Config.setProperty(Config.PROX_FTP_HOST_KEY, addressTextField.getText());
            Config.setProperty(Config.PROX_FTP_PORT_KEY, portTextField.getText());
        }else{   
            Config.setProperty(Config.PROX_HTTP_HOST_KEY, addressTextField.getText());
            Config.setProperty(Config.PROX_HTTP_PORT_KEY, portTextField.getText());
            Config.setProperty(Config.PROX_HTTPS_HOST_KEY, https_host);
            Config.setProperty(Config.PROX_HTTPS_PORT_KEY, https_port);
            Config.setProperty(Config.PROX_FTP_HOST_KEY, ftp_host);
            Config.setProperty(Config.PROX_FTP_PORT_KEY, ftp_port);
        }
          
        /*
         * Save SOCKS address/port separately in either case.
         */
        Config.setProperty(Config.PROX_SOX_HOST_KEY, socks_host);        
        Config.setProperty(Config.PROX_SOX_PORT_KEY, socks_port);                     

        Config.setBooleanProperty(Config.PROX_SAME_KEY, same_for_all);
        Config.setProperty(Config.PROX_BYPASS_KEY, bypass);                 
	 
        Config.setBooleanProperty( Config.PROX_LOCAL_KEY, bypassProxyChbox.isSelected() );

        setVisible(false);
        dispose();
    }

    private void cancelBtnActionPerformed(ActionEvent evt) {
        // Clear all values to the last saved state and close dialog.
        //setValues(); - not needed, we are disposind of the dialog anyway.
        setVisible(false);
        dispose();
    }


    /** Closes the dialog */
    private void closeDialog(WindowEvent evt) {
        setVisible(false);
        dispose();
    }
    
    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }  
    
    public JButton makeButton(String key) {
	JButton b = new JButton(getMessage(key));
	return b;
    }     

    private ButtonGroup proxySettingsButtonGroup;
    private UITextArea descriptionTextArea;
    private JLabel addressLabel, portLabel, locationLabel;
    private JRadioButton browserRbutton, manualRbutton, autoConfigRbutton, directRbutton;
    private JTextField addressTextField, portTextField, locationTextField;
    private JCheckBox bypassProxyChbox;
    private JButton advancedBtn, okButton, cancelButton;
    private String http_host, http_port, https_host, https_port, ftp_host, ftp_port,
            socks_host, socks_port, bypass;
    private boolean same_for_all=false;
}
