/*
 * @(#)AdvancedNetworkSettingsDialog.java	1.17 04/03/30
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;


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
        okButton.setMnemonic(ResourceManager.getVKCode("common.ok_btn.mnemonic"));
        okButton.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                okButtonActionPerformed(evt);
            }
        });
        
        cancelButton = new JButton(getMessage("common.cancel_btn"));
        cancelButton.setMnemonic(ResourceManager.getVKCode("common.cancel_btn.mnemonic"));

        AbstractAction cancelAction = new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                closeDialog();
            }
        };
        cancelButton.addActionListener(cancelAction);
        getRootPane().registerKeyboardAction(cancelAction, 
            KeyStroke.getKeyStroke((char)KeyEvent.VK_ESCAPE),
            JComponent.WHEN_IN_FOCUSED_WINDOW);

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
        String proxy, port;
        
        //
        // Get proxy address/port from parent if parent is NetworkSettingsDialog.
        // 
        if (getParent() != null && (getParent() instanceof NetworkSettingsDialog)){
            proxy = ((NetworkSettingsDialog)getParent()).getProxyAddressField();
            port = ((NetworkSettingsDialog)getParent()).getProxyPortField();
        }else{    
            //
            // If parent is null, or not an instance of NetworkSettingsDialog,
            // get the proxy address/port info from Config.
            //
            proxy = Config.getProperty(Config.PROX_HTTP_HOST_KEY);
            port = Config.getProperty(Config.PROX_HTTP_PORT_KEY);
        }
        
        //
        // Set text fields
        //
        httpTextField.setText(proxy);
	boolean enab = !Config.isLocked(Config.PROX_HTTP_HOST_KEY);
	httpTextField.setEditable(enab);
	httpTextField.setEnabled(enab);

        httpPortTextField.setText(port);
        enab = !Config.isLocked(Config.PROX_HTTP_PORT_KEY);
        httpPortTextField.setEditable(enab);
        httpPortTextField.setEnabled(enab);


        useForAllChBox.setSelected(Config.getBooleanProperty(Config.PROX_SAME_KEY));
        useForAllChBox.setEnabled(!Config.isLocked(Config.PROX_SAME_KEY));

        /*
         * If we should use the same proxy address/port for all protocols,
         * set the values for the corresponding text fields here.
         */
        setUseForAll(useForAllChBox.isSelected());

        proxy = Config.getProperty(Config.PROX_SOX_HOST_KEY);
        port =  Config.getProperty(Config.PROX_SOX_PORT_KEY);

        socksTextField.setText(proxy);
	enab = !Config.isLocked(Config.PROX_SOX_HOST_KEY);
        socksTextField.setEditable(enab);
        socksTextField.setEnabled(enab);

        socksPortTextField.setText(port);
	enab = !Config.isLocked(Config.PROX_SOX_PORT_KEY);
        socksPortTextField.setEditable(enab);
	socksPortTextField.setEnabled(enab);
        
        /*
         * See if there is a list of addresses for which proxy should
         * not be used:
         */
        String bypassList = Config.getProperty(Config.PROX_BYPASS_KEY);
        bypassTextArea.setText(bypassList);        
        bypassTextArea.setEditable(!Config.isLocked(Config.PROX_BYPASS_KEY));        
        
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
        // Save values set - pass them to the Config;
        // Close this dialog
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
        
        setVisible(false);
        dispose();
    }

    private void useForAllChBoxItemStateChanged(ItemEvent evt){
        setUseForAll(evt.getStateChange() == ItemEvent.SELECTED);
    }

    private void setUseForAll(boolean useAll) {
	if (useAll) {
            setTextFields(false);

	    String httpHost = httpTextField.getText();
	    String httpPort = httpPortTextField.getText();

	    /*
	     * If we should use the same proxy address/port for all protocols,
	     * set the values for the corresponding text fields here.
	     */
            secureTextField.setText(httpHost);
            securePortTextField.setText(httpPort);
            ftpTextField.setText(httpHost);
            ftpPortTextField.setText(httpPort);
	}
        else{
            secureTextField.setText(
		Config.getProperty(Config.PROX_HTTPS_HOST_KEY));
            securePortTextField.setText(
		Config.getProperty(Config.PROX_HTTPS_PORT_KEY));
            ftpTextField.setText(
		Config.getProperty(Config.PROX_FTP_HOST_KEY));
            ftpPortTextField.setText(
		Config.getProperty(Config.PROX_FTP_PORT_KEY));
            setTextFields(true);
        }
    }

    private void setTextFields(boolean state){
	    boolean enab = state && !Config.isLocked(Config.PROX_HTTPS_HOST_KEY);
            secureTextField.setEnabled(enab);
            secureTextField.setEditable(enab);

	    enab = state && !Config.isLocked(Config.PROX_FTP_HOST_KEY);
            ftpTextField.setEnabled(enab); 
            ftpTextField.setEditable(enab);

	    enab = state && !Config.isLocked(Config.PROX_HTTPS_PORT_KEY);
            securePortTextField.setEnabled(enab); 
            securePortTextField.setEditable(enab);

	    enab = state && !Config.isLocked(Config.PROX_FTP_PORT_KEY);
            ftpPortTextField.setEnabled(enab);
            ftpPortTextField.setEditable(enab);
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
    
}
