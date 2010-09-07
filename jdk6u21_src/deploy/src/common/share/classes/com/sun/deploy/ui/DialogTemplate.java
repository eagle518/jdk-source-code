/*
 * @(#)DialogTemplate.java	1.58 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.ui;


import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JSeparator;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JButton;
import javax.swing.JProgressBar;
import javax.swing.BoxLayout;
import javax.swing.Box;
import javax.swing.SwingConstants;
import javax.swing.ImageIcon;
import javax.swing.border.Border;
import javax.swing.BorderFactory;
import javax.swing.JComponent;
import javax.swing.AbstractAction;
import javax.swing.KeyStroke;
import javax.swing.JTextField;
import javax.swing.JPasswordField;
import javax.swing.JList;
import javax.swing.SwingUtilities;
import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Font;
import java.awt.Insets;
import java.awt.Image;
import java.awt.Dimension;
import java.awt.Component;
import java.awt.Frame;
import java.awt.Dialog;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.KeyEvent;
import java.awt.GridBagLayout;
import java.awt.GridBagConstraints;
import java.lang.NumberFormatException;
import java.text.MessageFormat;
import java.io.File;
import java.net.URL;
import java.security.cert.Certificate;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogListener;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import java.security.cert.X509Certificate;
import com.sun.deploy.security.CertificateDialog;
import com.sun.deploy.security.CredentialManager;
import com.sun.deploy.services.ServiceManager;

/*
 * Notes:  The following fields in the template can be NULL:
 *          owner - parent Component to use for this dialog
 *          okBtnStr - can be NULL to create dialog with only CANCEL action.
 *          cancelBtnStr - can be NULL to create dialog with only OK action.
 *          alerts - can be NULL if there are  no alert messages in security
 *                   dialog
 *          info -  can be NULL if there are no informational messages in
 *                  security dialog.
 *
 *          certs - can be NULL if there is no certificate chain (for non-
 *                  security related dialogs)
 *          start - will be needed only by CertificateDialog to show certificate
 *                  details
 *          end   - will be needed only by CertificateDialog to show certificate
 *                  details
 *
 */

public class DialogTemplate implements ActionListener, ImageLoaderCallback {
    
    /*
     * This is the costructor for all forms of this dialog
     *
     * @param  ainfo        - AppInfo object
     * @param  owner        - Component for deriving parent of dialog
     * @param  title        - title string for the dialog,
     * @param  topText      - short description of the cause for this dialog 
     *                        to appear
     */
    DialogTemplate( AppInfo ainfo, 
                    Component owner,
                    String title, 
                    String topText)
    {
        Component parent = deriveParent(owner, title);
        if (parent instanceof Dialog) {
            this.dialog = new JDialog((Dialog) parent, true);
        } else {
            this.dialog = new JDialog((Frame) parent, true);
        }
        this.dialog.setTitle(title);
        this.ainfo = ainfo;
        this.topText = topText;

        this.appTitle = ainfo.getTitle();
        this.appPublisher = ainfo.getVendor();
        this.appURL = ainfo.getFrom();
    }


    void setSecurityContent(boolean showAlways, 
                            boolean checkAlways,
                            String okBtnStr,
                            String cancelBtnStr,
                            String [] alerts,
                            String [] info,
                            int securityInfoCount,
                            boolean showMoreInfo,
                            Certificate[] certs,
                            int start, int end,
                            boolean majorWarning) 
    {
        this.certs = certs;
        this.start = start;
        this.end = end;
        this.alertStrs = alerts;
        this.infoStrs = info;
        this.securityInfoCount = securityInfoCount;
        this.majorWarning = majorWarning;

        if (alerts != null && alerts.length > 0) {
            useWarningIcon = true;
        }
        
        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                createTopPanel(false), BorderLayout.NORTH);
            
            // Set security warning dialogs type to TOOLKIT_MODAL
            if (Config.isJavaVersionAtLeast16()) {
                dialog.setModalityType(Dialog.ModalityType.TOOLKIT_MODAL);
            }

            if (showAlways) {
                this.alwaysString = 
                    ResourceManager.getString("security.dialog.always");
            }
            dialog.getContentPane().add(createCenterPanel(
                checkAlways, okBtnStr, cancelBtnStr, -1), BorderLayout.CENTER);

            dialog.getContentPane().add(
                createBottomPanel(showMoreInfo), BorderLayout.SOUTH);

            dialog.pack();
            dialog.setResizable(false);
        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }


    void setSimpleContent(String contentString,
                          boolean contentScroll, 
                          String infoString, 
                          String okBtnStr, 
                          String cancelBtnStr, 
                          boolean includeTop,
                          boolean useWarningIcon)
    {
        this.contentString = contentString;
        this.contentScroll = contentScroll;
        this.throwable = throwable;
        this.detailPanel = detailPanel;

        this.includeMasthead = includeTop;
        this.includeAppInfo = includeTop;
        this.largeScroll = !includeTop;

        this.useWarningIcon = useWarningIcon;
	if (infoString != null) {
	    String[] strs = { infoString };
	    if (useWarningIcon) {
		this.alertStrs = strs;
	    } else {
                this.infoStrs = strs;
            }
	}
        
        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                createTopPanel(false), BorderLayout.NORTH);

            dialog.getContentPane().add(createCenterPanel(false, 
                okBtnStr, cancelBtnStr, -1), BorderLayout.CENTER);

            dialog.getContentPane().add(
                createBottomPanel(false), BorderLayout.SOUTH);

            dialog.pack();
            boolean isResizable = contentScroll;
            dialog.setResizable(isResizable);

        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }
    
    void setMixedCodeContent(String contentString,
                          boolean contentScroll, 
                          String infoString, 
                          String bottomString, 
                          String okBtnStr, 
                          String cancelBtnStr, 
                          boolean includeTop,
                          boolean useWarningIcon)
    {
        this.contentString = contentString;
        this.contentScroll = contentScroll;
        this.throwable = throwable;
        this.detailPanel = detailPanel;

        this.includeMasthead = includeTop;
        this.includeAppInfo = includeTop;
        this.largeScroll = !includeTop;

        this.useMixcodeIcon = true;

        this.alertStrs = new String[1];
        String[] botStr = { bottomString};
        this.alertStrs = botStr;

        this.infoStrs = new String[3];
        String aStr = ResourceManager.getString("security.dialog.mixcode.info1");
        String bStr = ResourceManager.getString("security.dialog.mixcode.info2");
        String cStr = ResourceManager.getString("security.dialog.mixcode.info3");
        
        String[] strs = { aStr, bStr, cStr};
        this.infoStrs = strs;
        
        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                createTopPanel(false), BorderLayout.NORTH);

            this.mixedCodeString = infoString;
            dialog.getContentPane().add(createCenterPanel(false, 
                okBtnStr, cancelBtnStr, -1), BorderLayout.CENTER);

            dialog.getContentPane().add(
                createBottomPanel(false), BorderLayout.SOUTH);

            dialog.pack();

	    // Set default button to Yes.
	    okBtn.requestFocusInWindow();
            boolean isResizable = contentScroll;
            dialog.setResizable(isResizable);

        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }
       
        
    void setListContent( String label, 
                         JList scrollList, 
                         boolean showDetails,
                         String okBtnStr, 
                         String cancelBtnStr,
                         HashMap clientAuthCertsMap){
        
        // There is only one case of this dialog, which requires warning
        // icon.  Should this change in the future, add one more parameter
        // to this content - to specify icon type.
        this.useWarningIcon = true;
        
        // No need to display AppInfo
        this.includeAppInfo = false;
        
        // Cache certificates - this is needed to handle "Certificate
        // Details..." FancyButton click
        this.clientAuthCertsMap = clientAuthCertsMap;
        
        // show lable above optionPane if needed
        this.contentLabel = label;
        this.contentScroll = true;
        this.scrollList = scrollList;
        this.showDetails = showDetails;
        
        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                createTopPanel(false), BorderLayout.NORTH);

            dialog.getContentPane().add(createCenterPanel(
                false, okBtnStr, cancelBtnStr, -1), BorderLayout.CENTER);

            dialog.getContentPane().add(
                createBottomPanel(false), BorderLayout.SOUTH);

            dialog.pack();
            //dialog.setResizable(false);
        } catch (Throwable t) {
            Trace.ignored(t);
        }        
        
    }
    
    void setApiContent(String contentString,
                       String contentLabel, 
                       String alwaysString,
                       boolean alwaysChecked,
                       String okBtnStr, 
                       String cancelBtnStr)
    {
        this.contentString = contentString;
        this.contentLabel = contentLabel;
        this.contentScroll = (contentString != null);
        this.alwaysString = alwaysString;

        if (contentLabel == null && contentString != null) {
            this.infoStrs = new String[1];
            infoStrs[0] = contentString;
            this.contentString = null;
        }

        this.includeMasthead = true;
        this.includeAppInfo = (this.contentString == null);
        this.largeScroll = false;

        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                createTopPanel(false), BorderLayout.NORTH);

            dialog.getContentPane().add(createCenterPanel(
                false, okBtnStr, cancelBtnStr, -1), BorderLayout.CENTER);

            dialog.getContentPane().add(
                createBottomPanel(false), BorderLayout.SOUTH);

            dialog.pack();
            boolean isResizable = contentScroll;
            dialog.setResizable(isResizable);

        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }
    

    void setErrorContent(String contentString,
                         String okBtnStr,
                         String cancelBtnStr,
                         Throwable throwable,
                         JPanel detailPanel,
                         Certificate[] certs,
			 boolean hideLabel) {
    
        this.contentString = contentString;
        this.throwable = throwable;
        this.detailPanel = detailPanel;
        this.certs = certs;

	if (hideLabel) {
	    this.includeAppInfo = false;
	}

        this.useErrorIcon = true;

        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                        createTopPanel(false), BorderLayout.NORTH);

            dialog.getContentPane().add(createCenterPanel(false,
                        okBtnStr, cancelBtnStr, -1), BorderLayout.CENTER);

            dialog.getContentPane().add(
                        createBottomPanel(false), BorderLayout.SOUTH);

            dialog.pack();
            dialog.setResizable(false);

        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }

    void setMultiButtonErrorContent(String message, 
            String btnOneKey, String btnTwoKey, String btnThreeKey){
        this.useErrorIcon = true;
                        
        try {
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                        createTopPanel(false), BorderLayout.NORTH);
            
            dialog.getContentPane().add(
                    createInfoPanel(message), BorderLayout.CENTER);

            dialog.getContentPane().add(createThreeButtonsPanel(
                    btnOneKey, btnTwoKey, btnThreeKey, false), 
                    BorderLayout.SOUTH);

            dialog.pack();
            dialog.setResizable(false);

        } catch (Throwable t) {
            Trace.ignored(t);
        }
        
    }
    
    void setInfoContent(String contentString, String okBtnStr){
        this.useInfoIcon = true;
        this.contentString = contentString;
        
        try{
            dialog.getContentPane().setLayout(new BorderLayout());
            
            dialog.getContentPane().add(
                    createTopPanel(false), BorderLayout.NORTH);
            
            dialog.getContentPane().add(createCenterPanel(false, 
                    okBtnStr, null, -1), BorderLayout.CENTER);
            
            dialog.pack();
            dialog.setResizable(false);
                    
        }catch (Throwable t){
            Trace.ignored(t);
        }
    }

    void setPasswordContent(String details, 
                            boolean showUserName, boolean showDomain,  
                            String userName, String domain,  
                            boolean save, char[] password, String scheme){ 
        try{
            dialog.getContentPane().setLayout(new BorderLayout());
            
            dialog.getContentPane().add(
                    createPasswordPanel(details, showUserName, showDomain,  
                    userName, domain, save, password, scheme ), 
                    BorderLayout.CENTER);
            
            // Set password dialog type to TOOLKIT_MODAL
            if (Config.isJavaVersionAtLeast16()) {
                dialog.setModalityType(Dialog.ModalityType.TOOLKIT_MODAL);
            }

            dialog.pack();
            dialog.setResizable(false);
            
        }catch (Throwable t){
            Trace.ignored(t);
        }
        
    }
    
    void setUpdateCheckContent( String infoStr,
					  String yesBtnKey, 
                                String noBtnKey,
                                String askLaterBtnKey ){
        try{
            dialog.getContentPane().setLayout(new BorderLayout());
            
            dialog.getContentPane().add(
                    createTopPanel(false), BorderLayout.NORTH);

            dialog.getContentPane().add(
                    createInfoPanel(infoStr), BorderLayout.CENTER);
            
            dialog.getContentPane().add(
                    createThreeButtonsPanel(
                    yesBtnKey, noBtnKey, askLaterBtnKey, true),
                    BorderLayout.SOUTH);
            
            dialog.pack();
            dialog.setResizable(false);
        }catch (Throwable t){
            Trace.ignored(t);
        }
    }


    void setProgressContent(String okBtnStr,
                            String detailBtnStr,
                            String contentStr,
                            boolean showOkBtn, 
                            int percent)
    {
        try {
            this.cacheUpgradeContentString = contentStr;
            dialog.getContentPane().setLayout(new BorderLayout());

            dialog.getContentPane().add(
                createTopPanel(true), BorderLayout.NORTH);

            dialog.getContentPane().add(createCenterPanel(false, 
                okBtnStr, detailBtnStr, percent), BorderLayout.CENTER);

            if (cacheUpgradeContentString == null) {
                dialog.getContentPane().add(createBottomPanel(false),
                        BorderLayout.SOUTH);
            }

            dialog.pack();
            dialog.setResizable(false);
        } catch (Throwable t) {
            Trace.ignored(t);
        }
    }

    /*
     * This is to be used to create an informational message set alone
     * on a panel.  Currently this is used by update check dialog, but
     * can be used by any dialog that needs to display a message.
     *
     * infoStr is translated string.
     */
    private JPanel createInfoPanel( String infoStr ){
        JPanel infoPanel = new JPanel();
        infoPanel.setLayout(new FlowLayout(FlowLayout.LEADING, 0, 0));
        infoPanel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 12));
         
        UITextArea infoText = new UITextArea(
                ResourceManager.getUIFont().getSize(),  // get font size
                DIALOG_WIDTH - 20 - 12,         // subtract left/right border
                false);                         // plain font.
        
        infoText.setText(infoStr);
        infoText.setSize(infoText.getPreferredSize());

        infoPanel.add(infoText);
        return infoPanel;
    }
    
    // This method creates two or three buttons panel - third button
    // key can be null.
    private JPanel createThreeButtonsPanel( String btnOneKey, 
                                    String btnTwoKey, 
                                    String btnThreeKey,
                                    boolean isUpdateSpecific){
        JPanel buttonsPanel = new JPanel();
        buttonsPanel.setLayout(new FlowLayout(FlowLayout.TRAILING, 6, 0)); 
        buttonsPanel.setBorder(BorderFactory.createEmptyBorder(12, 12, 12, 12));         

        // First button 
        JButton oneBtn = new JButton(getMessage(btnOneKey));
        oneBtn.setMnemonic(ResourceManager.getVKCode(btnOneKey + ".mnemonic"));
        oneBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                setUserAnswer(UIFactory.OK);
                setVisible(false);
            }
        });
        buttonsPanel.add(oneBtn); 
        
        // Second button
        JButton twoBtn = new JButton(getMessage(btnTwoKey));
        twoBtn.setMnemonic(ResourceManager.getVKCode(btnTwoKey + ".mnemonic"));
        twoBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                setUserAnswer(UIFactory.CANCEL);
                setVisible(false);
            }
        });
        buttonsPanel.add(twoBtn);
        
        // Third button
        JButton threeBtn = null;
        if (btnThreeKey != null){
            threeBtn = new JButton(getMessage(btnThreeKey));
            threeBtn.setMnemonic(
                    ResourceManager.getVKCode(btnThreeKey + ".mnemonic"));
            threeBtn.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent evt) {
                    setUserAnswer(UIFactory.ASK_ME_LATER);
                    setVisible(false);
                }
            });
            buttonsPanel.add(threeBtn);
        }


        if (isUpdateSpecific){
            threeBtn.setToolTipText(ResourceManager.getMessage(
                        "autoupdatecheck.masthead"));
        }
                         
        if (threeBtn != null){
            JButton [] btns = {oneBtn, twoBtn, threeBtn};
            resizeButtons( btns );     
        } else {
            JButton [] btns = {oneBtn, twoBtn};
            resizeButtons( btns );   
        }
   
        return buttonsPanel;
    }

    
    /*
     * top part of the dialog contains short informative message, and either
     * an icon, or the text is displayed over a watermark image
     *
     * @param   useMastheadImage  - weather to use a masthead Image, or an icon.
     *
     */
    private JPanel createTopPanel(boolean useMastheadImage) {

        topPanel = new JPanel();
        topPanel.setBackground(BG);

        GridBagLayout gb = new GridBagLayout(); 
        GridBagConstraints c = new GridBagConstraints();          

        topPanel.setLayout(gb);
        
        // Create panel with text area and icon or just a background image:
        // Create topPanel's components.  UITextArea determines
        // the size of the dialog by defining the number of columns
        // based on font size.
        int mastheadFontSize = ResourceManager.getUIFont().getSize() + 4;
        masthead = new UITextArea(mastheadFontSize, MAIN_TEXT_WIDTH, true);
        
        // Set text to textStr - this is already translated string.
        masthead.setText(topText);

        // This is to workaround JTextArea bug 4924163.
        Dimension pref = masthead.getPreferredSize();
        masthead.setSize(pref.width, pref.height);

        masthead.setBorder(BorderFactory.createEmptyBorder(6, 12, 6, 6));
        
        if (useMastheadImage) {
            if (includeMasthead){ 
                c.fill = GridBagConstraints.HORIZONTAL;  
                c.gridwidth = GridBagConstraints.RELATIVE;  
                c.anchor = GridBagConstraints.WEST; 
                gb.setConstraints(masthead, c); 
                topPanel.add(masthead); 

                ImageIcon icon = 
                    ResourceManager.getIcon("progress.background.image");
                masthead.setBackgroundImage(icon.getImage());
		}
        } else {
            if (includeMasthead){ 
                c.fill = GridBagConstraints.HORIZONTAL;  
                c.gridwidth = GridBagConstraints.RELATIVE; 
                c.anchor = GridBagConstraints.WEST; 
                gb.setConstraints(masthead, c); 
                topPanel.add(masthead);      

                topPanel.setBackground(Color.white); 
                topPanel.setForeground(Color.white); 
                topPanel.setOpaque(true); 

	          // Create label with icon.
                topIcon = new JLabel();
                topIcon.setHorizontalAlignment(SwingConstants.CENTER);
                topIcon.setBorder(BorderFactory.createEmptyBorder(6, 6, 6, 12));
        
                // use default icon untill user icon is loaded 
                ImageIcon icon = ResourceManager.getIcon("java48.image");
                if (useErrorIcon) {
                    icon = ResourceManager.getIcon("error48.image");
                }
                if (useInfoIcon) {
                    icon = ResourceManager.getIcon("info48.image");
                }
                if (useMixcodeIcon) {
                    icon = ResourceManager.getIcon("mixcode.image");
		}
                topIcon.setIcon(icon);

                if (useWarningIcon) {
                    if (majorWarning){
                        icon = ResourceManager.getIcon("major-warning48.image");
                    }else{
                        icon = ResourceManager.getIcon("warning48.image");
                    }
                    topIcon.setIcon(icon);
                } else if (ainfo.getIconRef() != null) {
                    // setup ImageLoader here; using icon in ainfo
                    ImageLoader.getInstance().loadImage(ainfo.getIconRef(), 
                                            ainfo.getIconVersion(), this);
                }
                c.fill = GridBagConstraints.NONE; 
                c.gridwidth = GridBagConstraints.REMAINDER; 
                c.anchor = GridBagConstraints.EAST; 
                gb.setConstraints(topIcon, c); 
                topPanel.add(topIcon); 

            }// end if includeMasthead             
        } 
         
        
        // Create separator to be 510 pixels wide - this is to set the size
        // of the whole dialog.
        JSeparator sep = new JSeparator();
        sep.setPreferredSize(new Dimension(DIALOG_WIDTH, 1));
        c.gridy =1; 
        c.gridwidth = GridBagConstraints.REMAINDER; 
        gb.setConstraints(sep, c); 

        topPanel.add(sep);
        
        return topPanel;
    }
    
    /*
     * createCenterPanel - has app's name, publisher and from labels.
     *
     * @param - checkAlways
     * @param - okBtnStr
     * @param - cancelBtnStr
     * @param - progress
     */

    private JPanel createCenterPanel(boolean checkAlways,
                                     String okBtnStr,
                                     String cancelBtnStr,
                                     int progress ){

        Font f = ResourceManager.getUIFont();
        int newFontSize = getSubpanelFontSize();
        Font smallerFont = f.deriveFont(f.getStyle(), (float)newFontSize);

        centerPanel = new JPanel();
        centerPanel.setLayout(new BoxLayout(centerPanel, BoxLayout.Y_AXIS));
        centerPanel.setBorder(BorderFactory.createEmptyBorder(24, 24, 12, 12));
        
        GridBagLayout gb = new GridBagLayout(); 
        GridBagConstraints c = new GridBagConstraints();  
        JPanel labelsPanel = new JPanel();
        labelsPanel.setLayout(gb);
        labelsPanel.setBorder(BorderFactory.createEmptyBorder());

        JLabel nameLbl = new JLabel(getMessage("dialog.template.name"));
        
        // Set labels font to plain/bold according to UE spec.  In different
        // L&F(s) default font for labels is different.  To make all dialogs
        // appear according to spec, setting fonts manually.        
        Font bold = nameLbl.getFont().deriveFont(Font.BOLD);
        Font plain = nameLbl.getFont().deriveFont(Font.PLAIN);
        
        nameLbl.setFont(bold);

        // Create publisher label only if it is not NULL.
        JLabel publisherLbl = new JLabel(
            getMessage("dialog.template.publisher"));
        publisherLbl.setFont(bold);

        JLabel fromLbl = new JLabel(getMessage("dialog.template.from"));
        fromLbl.setFont(bold);

        nameInfo = new JLabel();
        publisherInfo = new JLabel();
        urlInfo = new JLabel();  
                
        nameInfo.setFont(plain);
        publisherInfo.setFont(plain);
        urlInfo.setFont(plain);

	// Disable HTML rendering for nameInfo, publisherInfo and urlInfo
	nameInfo.putClientProperty("html.disable", Boolean.TRUE);
	publisherInfo.putClientProperty("html.disable", Boolean.TRUE);
	urlInfo.putClientProperty("html.disable", Boolean.TRUE);
        
        // Set constraints
        c.fill = GridBagConstraints.HORIZONTAL; 
        c.gridwidth = 1;
        c.anchor = GridBagConstraints.WEST;
        c.insets = new Insets(0, 0, 0, 12);
        c.weightx = 0;

        // Only neccessary labels should be added...
        if (appTitle != null) {
            gb.setConstraints(nameLbl, c); 
            labelsPanel.add(nameLbl);
            
            c.insets = new Insets(0, 0, 0, 12);
            c.gridx = GridBagConstraints.RELATIVE;
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.weightx = 1.0;
            gb.setConstraints(nameInfo, c);
            labelsPanel.add(nameInfo);
        }
        if (appPublisher != null){
            c.fill = GridBagConstraints.HORIZONTAL; 
            c.insets = new Insets(12, 0, 0, 12);
            c.gridwidth = 1;  
            c.weightx = 0;
            c.anchor = GridBagConstraints.WEST;         
            gb.setConstraints(publisherLbl, c);
            labelsPanel.add(publisherLbl);
            
            c.insets = new Insets(12, 0, 0, 12);
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.weightx = 1.0;
            gb.setConstraints(publisherInfo, c);
            labelsPanel.add(publisherInfo);
        }
        // if there is a title, show from also
        if (appTitle != null  && appURL != null) {
            c.fill = GridBagConstraints.HORIZONTAL; 
            c.weightx = 0;
            c.gridwidth = 1;  
            c.anchor = GridBagConstraints.WEST;  
            c.insets = new Insets(12, 0, 0, 12);
            gb.setConstraints(fromLbl, c);
            labelsPanel.add(fromLbl);
            
            c.insets = new Insets(12, 0, 0, 12);
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.weightx = 1.0;
            gb.setConstraints(urlInfo, c);
            labelsPanel.add(urlInfo);
        }

        setInfo(appTitle, appPublisher, appURL);

        JPanel checkboxPanel = new JPanel();
        checkboxPanel.setLayout(new FlowLayout(FlowLayout.LEADING, 0, 0));

        JPanel mixedcodePanel = new JPanel();
        mixedcodePanel.setLayout(new BorderLayout());

        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new BorderLayout());

        // If checkbox is needed, create it and set border
        // to the infoHolderPanel above with left inset equal to
        // the left inset of the checkbox border.
        if (alwaysString != null){
            String key = "security.dialog.always";
            always = new JCheckBox(alwaysString);
            always.setMnemonic(ResourceManager.getVKCode(key+".mnemonic"));
            always.setSelected(checkAlways);
            
            // We want the checkbox to appear in plain font through all l&f(s)
            always.setFont(plain);

            // This is to align left side of the checkbox with first
            // letter of labels in the above panel.
            int left = always.getBorder().getBorderInsets(always).left;
            labelsPanel.setBorder(
                BorderFactory.createEmptyBorder(0,left, 0, 0));
            checkboxPanel.add(always);
            checkboxPanel.setBorder(
                BorderFactory.createEmptyBorder(0, 0, 12, 0));
        }

        // We might show text for mixed code warning dialog box here
        if (mixedCodeString != null) {
            mixedCodeLabel = new JLabel(mixedCodeString);
            mixedCodeLabel.setFont(plain);

            // Create a panel and add more info link
            JPanel moreinfoPanel = new JPanel();
            moreinfoPanel.setLayout(new BorderLayout());

            FancyButton moreInfoButton = null;
            moreInfoButton = new FancyButton("dialog.template.more.info");
                                
            Font newFont = f.deriveFont((float)newFontSize);
            moreInfoButton.setFont(newFont);
            moreInfoButton.addMouseListener(new MouseListener(){
                public void mouseClicked(MouseEvent me){
                    if (me.getComponent() instanceof FancyButton){
                        showMixedcodeMoreInfo();
                    }
                }
                public void mousePressed(MouseEvent me) {}
                public void mouseReleased(MouseEvent me) {}
                public void mouseEntered(MouseEvent me) {}
                public void mouseExited(MouseEvent me) {}
            });
            moreInfoButton.addActionListener(new ActionListener(){
                public void actionPerformed(ActionEvent ae){
                    showMixedcodeMoreInfo();
                }
            });

            moreinfoPanel.add(moreInfoButton, BorderLayout.WEST);
            mixedcodePanel.add(mixedCodeLabel, BorderLayout.NORTH);
            mixedcodePanel.add(moreinfoPanel, BorderLayout.SOUTH);
        }

        boolean showProgress = (progress >= 0);
        if (showProgress) {
            // A progress from 0 to 100 means show that progress.
            // less than 0 means don't show any progress bar
            // greater than 100 means create it invisible
            progressBar = new JProgressBar(0, 100);
            progressBar.setVisible(progress <= 100);
        }
        // We might show a scrollable text area here...
        if (contentString != null) {
            if (contentLabel != null) {
                JPanel top = new JPanel(new BorderLayout());
                top.add(new JLabel(contentLabel), BorderLayout.WEST);
                contentPanel.add(top, BorderLayout.NORTH);
            }
            if (contentScroll) {
                JTextArea text;
                final boolean limitWidth = largeScroll;
                if (largeScroll) {
                    text = new JTextArea(contentString, 20, 80);
                } else {
                    text = new JTextArea(contentString, 4, 40);
                }
                text.setEditable(false);
                JScrollPane sp = new JScrollPane(text,
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED) {

                    public Dimension getPreferredSize() {
                        Dimension ret = super.getPreferredSize();
                        if (limitWidth) {
                           ret.width = Math.min(ret.width, 
                                MAX_LARGE_SCROLL_WIDTH);
                        }
                        return ret;
                    }
                };

                contentPanel.add(sp, BorderLayout.CENTER);
            } else {
		UITextArea ta = new UITextArea(contentString);

        	// This is to workaround JTextArea bug 4924163.
		ta.setSize(ta.getPreferredSize());

                contentPanel.add(ta, BorderLayout.CENTER);
            }
            contentPanel.setBorder(
                    BorderFactory.createEmptyBorder(0, 0, 12, 0));
        }
        // ... we might show a scrollable list
        if (scrollList != null){
            if (contentLabel != null) {
                JPanel top = new JPanel(new BorderLayout());
                top.add(new JLabel(contentLabel), BorderLayout.WEST);
                contentPanel.add(top, BorderLayout.NORTH);
            }
            if (contentScroll) {
                JScrollPane sp = new JScrollPane(scrollList,
                    JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED,
                    JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
                contentPanel.add(sp, BorderLayout.CENTER);
            }
        
            // Create "Certificate Details..." link label if needed, and set its
            // alignment to the right.  Adding it here, after scroll panel.
            if (showDetails){    
                FancyButton certDetails = 
                        new FancyButton("security.more.info.details");
                
                certDetails.setFont(smallerFont);
                certDetails.addMouseListener(new MouseListener(){
                    public void mouseClicked(MouseEvent me){
                        if (me.getComponent() instanceof FancyButton){
                            showCertificateDetails();
                        }
                    }
                    public void mousePressed(MouseEvent me) {}
                    public void mouseReleased(MouseEvent me) {}
                    public void mouseEntered(MouseEvent me) {}
                    public void mouseExited(MouseEvent me) {}
                });
            
                JPanel certDetailsPanel = new JPanel();
                certDetailsPanel.setBorder(
                        BorderFactory.createEmptyBorder(12, 0, 12,0));
                certDetailsPanel.setLayout(
                        new FlowLayout(FlowLayout.TRAILING, 0, 0));
                certDetailsPanel.add(certDetails);
                certDetailsPanel.add(Box.createGlue());
                contentPanel.add(certDetailsPanel, BorderLayout.SOUTH);
            }
        }

        JPanel buttonsPanel = new JPanel();
        buttonsPanel.setLayout(new FlowLayout(FlowLayout.TRAILING, 0, 0));

        // Create buttons from okBtnStr and cancelBtnStr strings.
        okBtn = new JButton((okBtnStr == null) ? "" : getMessage(okBtnStr));
        okBtn.addActionListener(this);
        okBtn.setActionCommand(OK_ACTION);
        buttonsPanel.add(okBtn);
        okBtn.setVisible(okBtnStr != null);

        cancelBtn = new JButton((cancelBtnStr == null) ?
            "" : getMessage(cancelBtnStr));
        cancelBtn.addActionListener(this);
        buttonsPanel.add(Box.createHorizontalStrut(6));
        buttonsPanel.add(cancelBtn);
        cancelBtn.setVisible(cancelBtnStr != null);

        // Left button is for OK action.  If it is Visible, set default
        // action to OK.  Security dialog will change this later, in
        // createBottomPanel.
        if (okBtn.isVisible()) {
            dialog.getRootPane().setDefaultButton(okBtn);
        } else if (cancelBtn.isVisible()) {
            dialog.getRootPane().setDefaultButton(cancelBtn);
        }

        JButton [] buttons = { okBtn, cancelBtn };
        resizeButtons(buttons);
        
        if (cacheUpgradeContentString != null) {
            UITextArea cacheUpgradeContentTa =
                    new UITextArea(cacheUpgradeContentString);
            cacheUpgradeContentTa.setBorder(null);
            contentPanel.add(cacheUpgradeContentTa, BorderLayout.NORTH);
            centerPanel.add(contentPanel);
            centerPanel.add(Box.createVerticalStrut(12));
        } else {
            if (includeAppInfo) {
                centerPanel.add(labelsPanel);
                centerPanel.add(Box.createVerticalStrut(12));
            }
            if (alwaysString != null) {
                centerPanel.add(checkboxPanel);
            }
            if (mixedCodeString != null) {
                centerPanel.add(mixedcodePanel);
            }
            if (contentString != null || scrollList != null) {
                centerPanel.add(contentPanel);
            }
        }

        JPanel progressStatusPanel = new JPanel(new BorderLayout());
        centerPanel.add(progressStatusPanel);

        JPanel bottomPanel = new JPanel(new BorderLayout());
        if (showProgress) {
	    progressStatusLabel = new JLabel(" ");                        
            progressStatusLabel.setFont(smallerFont);
	    progressStatusPanel.add(progressStatusLabel, BorderLayout.WEST);

            progressBar.setBorder(BorderFactory.createEmptyBorder(2, 0, 2, 0));
            bottomPanel.add(progressBar, BorderLayout.CENTER);
        }
        bottomPanel.add(buttonsPanel, BorderLayout.EAST);
        centerPanel.add(bottomPanel);

        if (Config.isJavaVersionAtLeast13()) {
             dialog.getRootPane().getInputMap(
                JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),"cancel");

            dialog.getRootPane().getActionMap().put(
                "cancel", new AbstractAction() {
                public void actionPerformed(ActionEvent evt) {
                     cancelAction();
                }
            });
        }
        return centerPanel;
    }




    
    /*
     * bottom panel contains icon indicating the security alert level,
     * two bullets with most significant security warnings,
     * link label - to view more details about security warnings.
     */
    private JPanel createBottomPanel(boolean showMoreInfo){

        GridBagLayout gb = new GridBagLayout(); 
        GridBagConstraints c = new GridBagConstraints();  
        c.fill = GridBagConstraints.HORIZONTAL;

        
        bottomPanel = new JPanel();
        bottomPanel.setLayout(gb);
        
        JSeparator sep = new JSeparator();
        sep.setPreferredSize(new Dimension(DIALOG_WIDTH, 1));
        
        // Fill in both directions.
        c.fill = GridBagConstraints.HORIZONTAL;
        
        c.gridwidth = GridBagConstraints.REMAINDER;        
        gb.setConstraints(sep, c);            
        bottomPanel.add(sep);

        if (alertStrs != null || infoStrs != null) {        
            // Icon 32x32 pixels with indication of secutiry alert - high/low
            securityIcon = new JLabel();

            // If there are no messages in securityAlerts, show
            // SECURITY_ALERT_LOW icon in the lower left corner of
            // security dialog.
            String imageFile = SECURITY_ALERT_HIGH;
        
            if (alertStrs == null || alertStrs.length == 0) {
                imageFile = SECURITY_ALERT_LOW;
            
                // if all verified and no security warnings, select checkbox
                if (always != null) {
                    always.setSelected(true);
                }
            } else if (mixedCodeString == null) {
                // Set default action for the dialog to CANCEL
                dialog.getRootPane().setDefaultButton(cancelBtn);
            }
            securityIcon.setIcon(new ImageIcon(
                ClassLoader.getSystemResource(imageFile)));

            // Add icon to the bottom panel.
            c.insets = new Insets(12, 12, 12, 20);
            c.gridy =1; 
            c.gridwidth = 1;
            gb.setConstraints(securityIcon, c);
            bottomPanel.add(securityIcon);
            
            // According to UE spec, we should use font size 1 point smaller
            // then default font, or set it to min required for particular 
            // locale.
            Font f = ResourceManager.getUIFont();                        
            int newFontSize = getSubpanelFontSize();
            
            // lowerPanel contains "More Information..." label
            // Construct it if we need "More Information..." label
            // This code comes first, because we'll need the length of 
            // this component to calculate the remaining available space
            // to be used by text area.
            int moreInfoLength = 0;
            FancyButton moreInfoLbl = null;
            if (showMoreInfo) {
                moreInfoLbl = 
                        new FancyButton("dialog.template.more.info");
                                
                Font newFont = f.deriveFont((float)newFontSize);
                moreInfoLbl.setFont(newFont);
                moreInfoLbl.addMouseListener(new MouseListener(){
                    public void mouseClicked(MouseEvent me){
                        if (me.getComponent() instanceof FancyButton){
                            showMoreInfo();
                        }
                    }
                    public void mousePressed(MouseEvent me) {}
                    public void mouseReleased(MouseEvent me) {}
                    public void mouseEntered(MouseEvent me) {}
                    public void mouseExited(MouseEvent me) {}
                });
                moreInfoLbl.addActionListener(new ActionListener(){
                    public void actionPerformed(ActionEvent ae){
                        showMoreInfo();
                    }
                });
           
                moreInfoLength = moreInfoLbl.getPreferredSize().width 
                        + moreInfoLbl.getInsets().left
                        + moreInfoLbl.getInsets().right;                
            }            
        
            // If there are no alerts (alertStrs is null, or length is 0),
            // then we should show only first message from infoStrs.
            // this is how it will work for security dialog...                    
            int textAreaWidth = DIALOG_WIDTH - 12 * 2 // left/right border
                    - securityIcon.getPreferredSize().width // shield width
                    - 20 // gap between shield and text area 
                    - 6 // gap between text area and More Info link
                    - moreInfoLength - 8; // more info button length.
            UITextArea bulletText = new UITextArea(
                    newFontSize, textAreaWidth, false);
  
            
            // Set font to PLAIN.  By default, UITextArea's font will be system
            // l&f style.  While we want this particular text are to appear
            // in PLAIN font on all platforms through all l&f(s)
            Font plain = 
                    bulletText.getFont().deriveFont(Font.PLAIN);
            bulletText.setFont(plain);
        
            if ( (alertStrs == null || alertStrs.length == 0)
                && infoStrs != null && infoStrs.length != 0) {
                // If there are no alerts, use first string from the infoStrs.
                bulletText.setText( (infoStrs[ 0 ] != null)?
                        infoStrs[ 0 ] : " " );
                bulletText.setSize(bulletText.getPreferredSize());
            } else if (alertStrs != null && alertStrs.length != 0) {
                // If there are any alers, use first string from alertStrs.
                bulletText.setText( (alertStrs[ 0 ] != null)?
                        alertStrs[ 0 ] : " " );
                
                bulletText.setSize(bulletText.getPreferredSize());
            }
             
            // Set insets around text area according to the spec
            c.gridwidth = GridBagConstraints.RELATIVE;
            c.insets = new Insets(12, 0, 12, 6);
            c.weightx = 0;
            gb.setConstraints(bulletText, c);            
            bottomPanel.add(bulletText);
                        
            c.insets = new Insets(12, 0, 12, 12);
            c.gridwidth = GridBagConstraints.REMAINDER;
                
            if (moreInfoLbl != null){
                c.gridx = GridBagConstraints.LINE_END;
                gb.setConstraints(moreInfoLbl, c);
                bottomPanel.add(moreInfoLbl);            
            }
        } else {
            bottomPanel.setBorder(BorderFactory.createEmptyBorder(0, 0, 2, 0));
        }

        return bottomPanel;
    }
    
    
    /*
     * Create panel with the following info:
     * - Text area with description of what the password is for
     * - "User Name" label followed by a text field
     * - "Password" label followed by a password field
     * - "Domain" label followed by a text field.
     * - "OK" and "Cancel" buttons
     *
     * This panel might have either one, two or all three pairs of
     * label/text field combinations from above list.
     */
    private JPanel createPasswordPanel(String details, boolean showUserName, 
            boolean showDomain, String userName, String domain,  
            boolean saveEnabled, char[] sugPass , String scheme ){ 
        JLabel uNameLbl = new JLabel();
        JLabel domainLbl = new JLabel();
        
        // According to UE spec, set font to plain/bold where needed.
        // On different l&f labels and text areas are bold/plain.  We want 
        // them to correspond to UE spec, so setting bold/plain font manually.
        Font plain = uNameLbl.getFont().deriveFont(Font.PLAIN);
        Font bold = uNameLbl.getFont().deriveFont(Font.BOLD);
        
        JLabel banner = new JLabel();
        banner.setIcon(ResourceManager.getIcon("pwd-masthead.png"));
        
        // If User Name label needed, create it.
        if (showUserName){
            String userNameKey = "password.dialog.username";
            uNameLbl.setText(getMessage(userNameKey));
            uNameLbl.setDisplayedMnemonic(
                    ResourceManager.getAcceleratorKey(userNameKey));
            pwdName = new JTextField();
            // Suggest a User Name if possible
            pwdName.setText( userName );
            uNameLbl.setLabelFor(pwdName);
            uNameLbl.setFont(bold);
        }
        
        // Always create "Password:" label - this is password dialog.
        String passwordKey = "password.dialog.password";
        JLabel passwordLbl = new JLabel(getMessage(passwordKey));
        password = new JPasswordField();
        // Suggest a password
        password.setText( String.valueOf(sugPass) );
        passwordLbl.setLabelFor(password);
        passwordLbl.setDisplayedMnemonic(
                ResourceManager.getAcceleratorKey(passwordKey));
        passwordLbl.setFont(bold);
        
        if (showDomain){
            String domainKey = "password.dialog.domain";
            domainLbl.setText(getMessage(domainKey));
            pwdDomain = new JTextField();
            // Suggest Domain Name
            pwdDomain.setText( domain );
            domainLbl.setLabelFor(pwdDomain);
            domainLbl.setDisplayedMnemonic(
                    ResourceManager.getAcceleratorKey(domainKey));
            domainLbl.setFont(bold);
        }                

        GridBagLayout gb = new GridBagLayout();
        GridBagConstraints c = new GridBagConstraints();

        JPanel elementsPanel = new JPanel();
        elementsPanel.setLayout(gb);

        // Fill in both directions.
        c.fill = GridBagConstraints.BOTH;
        
        // Make banner occupy the whole row
        c.gridwidth = GridBagConstraints.REMAINDER;
        gb.setConstraints(banner, c);
        elementsPanel.add(banner);
        
        JSeparator sep = new JSeparator();
        gb.setConstraints(sep, c);
        elementsPanel.add(sep);
               
        // Add text describing what password we are asking for.
        // Use constructor that allows us to specify the width of 
        // the text area - we want width to be the width of the
        // banner minus horizontal insets.
        UITextArea detailsText =  new UITextArea( uNameLbl.getFont().getSize(), 
                                  banner.getPreferredSize().width - 12 * 2, 
                                  false);
        detailsText.setFont(plain);
        
        // This is already translated string.
        detailsText.setText(details);
                    
        // This is to workaround JTextArea bug 4924163.
        Dimension pref = detailsText.getPreferredSize();
        detailsText.setSize(pref.width, pref.height);
            
        c.gridwidth = GridBagConstraints.REMAINDER;
        c.insets = new Insets(12, 12, 0, 12);
        gb.setConstraints(detailsText, c);
        elementsPanel.add(detailsText);

        Insets labelInsets = new Insets(12, 12, 0, 6);
        Insets fieldInsets = new Insets(12, 0, 0, 12);
        
        // Specify insets, and add label/text field combo.
        c.gridwidth = 1;
        c.insets = labelInsets;
        
        // If "User name" label/text field are needed, add them,
        if (showUserName){
            gb.setConstraints(uNameLbl, c);
            elementsPanel.add(uNameLbl);

            // After label add text field for user's name and make it use
            // the rest of the row.
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.insets = fieldInsets;
            gb.setConstraints(pwdName, c);
            elementsPanel.add(pwdName);
        }
        // Add password label/text field.
        //c.insets = new Insets(0, HORIZONTAL, 0, HORIZONTAL);
        c.gridwidth = 1;
        c.insets = labelInsets;
        gb.setConstraints(passwordLbl, c);
        elementsPanel.add(passwordLbl);

        c.gridwidth = GridBagConstraints.REMAINDER;
        c.insets = fieldInsets;
        gb.setConstraints(password, c);
        elementsPanel.add(password);
        
        // If we need "Domain" label/text field, add them.
        if (showDomain){
            c.gridwidth = 1;
            c.insets = labelInsets;
            gb.setConstraints(domainLbl, c);
            elementsPanel.add(domainLbl);

            // After label add text field for domain and make it use
            // the rest of the row.
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.insets = fieldInsets;
            gb.setConstraints(pwdDomain, c);
            elementsPanel.add(pwdDomain);
        }
        // If enable password caching option is checked in JCP 
	if (Config.getBooleanProperty(Config.SEC_USE_PASSWORD_CACHE_KEY)) {
          // Is password persistance available?  If so display a save check box
          if ( saveEnabled == true ){
            c.gridwidth = GridBagConstraints.REMAINDER;
            c.insets = new Insets(12, 8, 0, 12);
            String key = "password.dialog.save";
             always = new JCheckBox(getMessage(key));
             
             // If we have a saved password the check the save box by default
             if ( sugPass.length > 0 ) {
               always.setSelected(true);
             } else {
                always.setSelected(false);
             }
             gb.setConstraints(always, c);
             elementsPanel.add(always);
          }
	}
        
        JPanel btnsPanel = new JPanel();
        btnsPanel.setLayout(new BoxLayout(btnsPanel, BoxLayout.LINE_AXIS));
        
        // Create "OK" and "Cancel" buttons.  No need for mnemonics,
        // just set "OK" to be default button.
        okBtn = new JButton(getMessage("common.ok_btn"));
        okBtn.setActionCommand(OK_ACTION);
        okBtn.addActionListener(this);
        dialog.getRootPane().setDefaultButton(okBtn);
        
        cancelBtn = new JButton(getMessage("common.cancel_btn"));
        cancelBtn.addActionListener(this);
        
        dialog.getRootPane().getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(
                KeyStroke.getKeyStroke(KeyEvent.VK_ESCAPE, 0),"cancel");

        dialog.getRootPane().getActionMap().put("cancel", new AbstractAction() {
            public void actionPerformed(ActionEvent evt) {
                cancelAction();
            }
        });
        
        JButton buttons[] = {okBtn, cancelBtn};
        resizeButtons(buttons);

        btnsPanel.add(Box.createHorizontalGlue());
        btnsPanel.add(okBtn);
        btnsPanel.add(Box.createHorizontalStrut(6));
        btnsPanel.add(cancelBtn);
        btnsPanel.add(Box.createHorizontalStrut(12));
                
        // Add scheme information is provided:
        JPanel schemePanel = null;
        if (scheme != null){
            schemePanel = new JPanel();
            schemePanel.setLayout(new FlowLayout(FlowLayout.LEADING, 0, 0));
            schemePanel.add(Box.createHorizontalStrut(12));
            MessageFormat mf = 
                    new MessageFormat(getMessage("password.dialog.scheme"));
        
            Object [] args = { scheme };            
            JLabel schemeLabel = new JLabel(mf.format(args));
            schemeLabel.setFont(plain);
            schemePanel.add(schemeLabel);            
        }

        JPanel allComponents = new JPanel();
        allComponents.setLayout(new BoxLayout(allComponents, 
                                              BoxLayout.PAGE_AXIS));

        allComponents.add(elementsPanel);
        // Add 24-pixel vertical strut befor buttons panel
        allComponents.add(Box.createVerticalStrut(12 * 2));
        allComponents.add(btnsPanel);
        allComponents.add(Box.createVerticalStrut(12));
        if (schemePanel != null){
            allComponents.add(new JSeparator());
            allComponents.add(Box.createVerticalStrut(12));
            allComponents.add(schemePanel);
            allComponents.add(Box.createVerticalStrut(12));
        }

        return allComponents;
    }
    
    void showMoreInfo(){
        MoreInfoDialog info;
        if (throwable ==  null && detailPanel == null) {
            info = new MoreInfoDialog(dialog, alertStrs, infoStrs, 
                securityInfoCount, certs, start, end);
        } else {
            info = new MoreInfoDialog(dialog, detailPanel, throwable, certs);
        }
        info.setVisible(true);
    } 
    
    void showMixedcodeMoreInfo(){
        MoreInfoDialog info;
        info = new MoreInfoDialog(dialog, null, infoStrs, 0, null, 0, 0);
        info.setVisible(true);
    } 

    void showCertificateDetails(){
        // Find out which certificate is selected and show details about 
        // selected certificate.
        int selectedIndex = scrollList.getSelectedIndex();
                        
        X509Certificate[] selectCert = null;
        for (Iterator iter = clientAuthCertsMap.values().iterator();
                               selectedIndex >= 0 && iter.hasNext();
                               selectedIndex--) {
                                
            selectCert = (X509Certificate[]) iter.next();
        }
                    
        // Only show details when certificate is available
        if (selectCert != null){
            CertificateDialog.showCertificates(this.dialog,
                selectCert, 0, selectCert.length);
        }
    }

    public void setVisible(boolean visible) {
        if (visible) {
            final DialogListener dl = UIFactory.getDialogListener();
            final JDialog dlg = dialog;
            final Frame dummy = dummyFrame;
            Runnable runner = new Runnable() {
                public void run() {
                    if (dl != null) {
                        dl.beforeShow();
                    }
                    dlg.pack();
                    if (dummy != null) {
                        dummy.setVisible(true);
                    }
                    dlg.setVisible(true);
                }
            };

            if (SwingUtilities.isEventDispatchThread()) {
                runner.run();
            } else {
                try {
                    SwingUtilities.invokeAndWait(runner);
                } catch (Exception e) {
                    Trace.ignored(e);
                }
            }
        } else {
            dialog.setVisible(false);
	    dialog.dispose();
            if (dummyFrame != null) {
                dummyFrame.setVisible(false);
                dummyFrame.dispose();
            }
        }
        
    }

    public void setMasthead(String text, boolean singleLine) {
        if (masthead != null) {
            topText = text;
            masthead.setText(text);
            if (singleLine) {
                masthead.setBorder(
                    BorderFactory.createEmptyBorder(16, 12, 16, 6));
            } else {
                masthead.setBorder(
                    BorderFactory.createEmptyBorder(6, 12, 6, 6));
            }
        }
    }

    public void setIcon(Image image) {
        topIcon.setIcon(new ImageIcon(image));
    }
             
    /*
     * According to UI guidelines, all buttons should have the same length.
     * This function is to define the longest button in the array of buttons
     * and set all buttons in array to be the length of the longest button.
     */
    public static void resizeButtons(JButton [] buttons){
        int len = buttons.length;
        
        // Find out the longest button        
        int widest = 50;
        for ( int i = 0; i < len; i++){
            if ( buttons[i].getPreferredSize().width > widest ){
                widest = buttons[i].getPreferredSize().width;
            }
        }
        
        // Now set all buttons to be same length.
        for ( int i = 0; i < len; i++){
            Dimension d = buttons[i].getPreferredSize();
            d.width=widest;
            buttons[i].setPreferredSize( d );
        }
    }
    
    private String getMessage(String id) {
        return ResourceManager.getMessage(id);
    }  
    
    public void actionPerformed(ActionEvent ae){
        // Get action command from event
        String command = ae.getActionCommand();
        
        if (command.equals(OK_ACTION)){
            userAnswer = UIFactory.OK;
            if (always != null && always.isSelected()) {
                userAnswer = UIFactory.ALWAYS;
            }
            if (stayAliveOnOk == true) {
                // This is used to allow the UpdateDialog 
                // to morph into progress dialog without closing.
                return;
            }
            if (password != null){
                pwd = password.getPassword();
            }
            if (pwdName != null){
                userName = pwdName.getText();
            }
            if (pwdDomain != null){
                domain = pwdDomain.getText();
            }
            if (scrollList != null){
                userAnswer = scrollList.getSelectedIndex();
            }
        } else {
            // in this case is "Detail" button, not "Cancel"
            if (throwable != null || detailPanel != null) {
                showMoreInfo();
                return;
            }
            userAnswer = UIFactory.CANCEL;
            
            // Special case for client certificates selection - if 
            // user clicked "Cancel", return "-1" - to indicate that
            // no certificate was selected by user:
            if (scrollList != null){
                userAnswer = UIFactory.ERROR;
            }
        }
        // hide the dialog.  We'll return from the dialog,
        // and who ever called it will retrieve user's answer
        // and will dispose of the dialog after that.
        this.setVisible(false);
    }

    public void cancelAction() {
        userAnswer = UIFactory.CANCEL;
        this.setVisible(false);
    }
    
    public int getUserAnswer(){
        return userAnswer;
    }

    void setUserAnswer(int answer) {
        userAnswer = answer;
    }
    
    char [] getPassword(){
        return pwd;
    }
    
    String getUserName(){
        return userName;
    }
    
    String getDomain(){
        return domain;
    }

    public boolean isPasswordSaved() {
        return( always != null && always.isSelected() );
    }

    public void progress(int progress) {
        if (progressBar != null) {
            if (progress <= 100) {
                boolean vis = progressBar.isVisible();
                progressBar.setValue(progress);
                progressBar.setVisible(true);
                if (!vis) {
                    dialog.pack();
                }
            } else {
                progressBar.setVisible(false);
            }
        } else {
        }
    }

    void setMastheadBackground (Image image) {
        if (masthead != null) {
            masthead.setBackgroundImage(image);
        }
    }

    public void disposeDialog() {
        dialog.dispose();
    }

    public Component deriveParent(Component owner, String title) {

        if (owner == null && Config.getOSName().equals("Windows")) {
            // on windows, for null owners, make a Frame off screen
            dummyFrame = new Frame(title);
            dummyFrame.setLocation(-4096, -4096);
            return (Component) dummyFrame;
        }
            
        for (Component comp = owner; comp != null; comp = comp.getParent()) {
            if (comp instanceof Dialog || comp instanceof Frame) {
                return comp;
            }
        }
        return (Component) null;
    }

    public JDialog getDialog() {
        return dialog;
    }

    public void setInfo(String name, String publisher, URL urlFrom) {
        if (nameInfo != null) {
            nameInfo.setText(name);
        }
        if (publisherInfo != null) {
            appPublisher = publisher;
            publisherInfo.setText(publisher);
        }
        if (urlInfo != null) {
            appURL = urlFrom;
            String from = " ";
            String tooltip = "";

            // If app's location is null, set it to an empty string.
            if (urlFrom != null){
                from = urlFrom.getProtocol() + "://" + urlFrom.getHost();
                int port = urlFrom.getPort();
                if (port != -1){
                    from = from + ":" + Integer.toString(port);
                }
                tooltip = urlFrom.toString();
            }
            urlInfo.setText(from);
            urlInfo.setToolTipText(tooltip);
        }
    }

    void showOk(boolean show) {
        JButton [] buttons = {okBtn, cancelBtn };
        resizeButtons(buttons);
        okBtn.setVisible(show);
    }

    void stayAlive() {
        stayAliveOnOk = true;
    }

    public void setProgressStatusText(String text) {
        if (progressStatusLabel != null) {
	    if (text == null || text.length() == 0) {
		text = " ";
	    }
            progressStatusLabel.setText(text);
	}
    }

    public void imageAvailable(URL url, String version,
                               Image image, File file) {
            final int w = image.getWidth(null);
            final int h = image.getHeight(null);
            final Image imageIn = image;
            final JLabel label = topIcon;
            new Thread(new Runnable() {
                public void run() {
                    Image im = imageIn;
                    if (ICON_SIZE != w || ICON_SIZE != h) {
                         im = imageIn.getScaledInstance(
                         ICON_SIZE, ICON_SIZE, Image.SCALE_DEFAULT);
                    }
                    label.setIcon(new ImageIcon(im));
                }
            }).start();

    }

    public void finalImageAvailable(URL url, String version,
                                           Image image, File file) {
        imageAvailable(url, version, image, file);
    }
    
    public static int getSubpanelFontSize(){
        Font f = ResourceManager.getUIFont();
         
        // Use 2 points smaller font for unix OS and 1 point
        // smaller font for windows.
        int newFontSize = f.getSize() - 2;
        if (Config.getOSName().equalsIgnoreCase("windows")){
            newFontSize = f.getSize() - 1;
        }
        
        // Make sure the font size is not less then  minimum allowed 
        //for current locale.  If it is, fall back to minimum allowed size.
        if (newFontSize < minFontSize){
            newFontSize = minFontSize;
        }
        
        return newFontSize;
    }


    private JDialog dialog = null;
    private AppInfo ainfo = null;
    private String topText = null;
    private String appTitle = null;
    private String appPublisher = null;
    private URL appURL = null;
    private Frame dummyFrame = null;
    private boolean useErrorIcon = false;
    private boolean useWarningIcon = false;
    private boolean useInfoIcon = false;
    private boolean useMixcodeIcon = false;

    private JLabel progressStatusLabel = null;
    private JPanel topPanel, centerPanel, bottomPanel;
    private JLabel topIcon, securityIcon, nameInfo, publisherInfo, urlInfo;
    private JButton okBtn, cancelBtn;
    private JCheckBox always;
    private JLabel mixedCodeLabel;
    private UITextArea masthead = null;
    
    private final static int ICON_SIZE = 48;

    // The default return value from this dialog is 1 - which is "Deny" for
    // security dialog.
    private int userAnswer = UIFactory.ERROR;
    
    // Defines max dialog width.
    private final int DIALOG_WIDTH = 510;
    private final int MAX_LARGE_SCROLL_WIDTH = 600;
    
    // This is the color used for background in the top part of all dialogs.
    private final Color BG = Color.white;
    
    // Visual indication of security level alert - either high or medium.
    // Located in the lower left corner at the bottom of the dialog.
    private final String SECURITY_ALERT_HIGH = 
            "com/sun/deploy/resources/image/security_high.png";
    private final String SECURITY_ALERT_LOW  = 
            "com/sun/deploy/resources/image/security_low.png";
         
    // According to the UI spec, the width of the main message text in the upper
    // panel should be 426 pixels.
    private static int MAIN_TEXT_WIDTH = 426;
    
    private final String OK_ACTION = "OK";
    
    private final int MAX_BUTTONS = 2;
    
    // These are for security dialog only.
    private int start;
    private int end;
    private Certificate[] certs;
    private String[] alertStrs;
    private String[] infoStrs;
    private int securityInfoCount;
    
    private Color originalColor;
    private Cursor handCursor = new Cursor(Cursor.HAND_CURSOR); 
    private Cursor originalCursor = null;

    protected JProgressBar progressBar = null;
    private boolean stayAliveOnOk = false;
    private String contentString = null;
    private String cacheUpgradeContentString = null;
    private String contentLabel = null;
    private String alwaysString = null;
    private String mixedCodeString = null;
    private boolean contentScroll = false;
    private boolean includeMasthead = true;
    private boolean includeAppInfo = true;
    private boolean largeScroll = false;
    private Throwable throwable = null;
    private JPanel detailPanel = null;
    private char [] pwd = new char[0];
    private String userName;
    private String domain;
    private JTextField pwdName, pwdDomain;
    private JPasswordField password;
    private JList scrollList;
    private boolean showDetails = false;
    HashMap clientAuthCertsMap;
    static int minFontSize = ResourceManager.getMinFontSize();
    private boolean majorWarning = false;
}
