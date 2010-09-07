/*
 * @(#)GeneralPanel.java	1.27 10/03/24
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
import com.sun.deploy.ui.AboutDialog;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.DialogTemplate;

import java.io.*;
import java.lang.*;
import java.lang.reflect.*;
import java.net.URL;
import java.net.URLClassLoader;


public class GeneralPanel extends JPanel{
    
    public GeneralPanel(){        
        initComponents();
    }

    JButton cacheViewBtn;

    public void initComponents(){
        // Use this Layout when Extention panel is back...
        setLayout(new BorderLayout());
        //setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
        setBorder(new EmptyBorder(new Insets(5, 5, 5, 5)));

	JPanel topPanel = new JPanel();
	topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));

        JPanel aboutPanel = new JPanel();
        aboutPanel.setBorder(new TitledBorder(
                             new TitledBorder(new EtchedBorder()), 
                             getMessage("general.about.border"), 
                             TitledBorder.DEFAULT_JUSTIFICATION, 
                             TitledBorder.DEFAULT_POSITION));
        aboutPanel.setLayout(new BorderLayout());

        JTextArea aboutTextArea = new JSmartTextArea(getMessage("general.about.text"));
        
        JPanel aboutBtnPanel = new JPanel();
        aboutBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        JButton aboutBtn = new JButton(getMessage("general.about.btn"));
        aboutBtn.setMnemonic(ResourceManager.getVKCode("general.about.btn.mnemonic"));
        aboutBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                aboutBtnActionPerformed(evt);
            }
        });
        aboutBtn.setToolTipText(getMessage("general.about.btn.tooltip"));
        aboutBtnPanel.add(aboutBtn);
        
        aboutPanel.add(aboutTextArea, BorderLayout.NORTH);
        aboutPanel.add(aboutBtnPanel, BorderLayout.SOUTH);
        

        /*
         * Create TemporaryFiles subpanel to be displayed
         * on GeneralTab.
         */
        JPanel tempFilesPanel = new JPanel();
        tempFilesPanel.setLayout(new BorderLayout());

        tempFilesPanel.setBorder(new TitledBorder(
             new TitledBorder(new EtchedBorder()), 
             getMessage("general.cache.border.text"), 
             TitledBorder.DEFAULT_JUSTIFICATION, 
             TitledBorder.DEFAULT_POSITION));


        JPanel tempFilesBtnPanel = new JPanel();
        tempFilesBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        cacheViewBtn = makeButton("general.cache.view.text");
        cacheViewBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                viewBtnAction();
            }
        });
 
        JButton tempFilesSettingsBtn = makeButton("general.cache.settings.text");
        tempFilesSettingsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                tempFilesSettingsBtnActionPerformed(evt);
            }
        });
        tempFilesSettingsBtn.setToolTipText(getMessage("temp.files.settings.btn.tooltip"));
        
        JButton [] btns = {cacheViewBtn, tempFilesSettingsBtn};
        DialogTemplate.resizeButtons( btns );     

        JSmartTextArea tempFilesText = new JSmartTextArea(getMessage("general.cache.desc.text"));
        tempFilesBtnPanel.add(tempFilesSettingsBtn);
        tempFilesBtnPanel.add(cacheViewBtn);
        tempFilesPanel.add(tempFilesText, BorderLayout.NORTH);
        tempFilesPanel.add(tempFilesBtnPanel, BorderLayout.SOUTH);


        /*
         * Create NetworkSettings subpanel to be displayed
         * on GeneralTab.
         */
        JPanel networkPanel = new JPanel();
        networkPanel.setLayout(new BorderLayout());
        networkPanel.setBorder(new TitledBorder(
            new TitledBorder(new EtchedBorder()), 
            getMessage("general.network.border.text"), 
            TitledBorder.DEFAULT_JUSTIFICATION, 
            TitledBorder.DEFAULT_POSITION));
      
        JPanel networkBtnPanel = new JPanel();
        networkBtnPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
        JButton networkSettingsBtn = makeButton("general.network.settings.text");
        networkSettingsBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                networkSettingsBtnActionPerformed(evt);
            }
        });
        networkSettingsBtn.setToolTipText(getMessage("network.settings.btn.tooltip"));
        networkBtnPanel.add(networkSettingsBtn);

        JSmartTextArea networkSettingsText = new JSmartTextArea(getMessage("general.network.desc.text"));
        networkPanel.add(networkSettingsText, BorderLayout.NORTH);
        networkPanel.add(networkBtnPanel, BorderLayout.SOUTH);

	  topPanel.add(aboutPanel);
        topPanel.add(networkPanel);
        topPanel.add(tempFilesPanel);

	  add(topPanel, BorderLayout.CENTER);
          
    }

    private void aboutBtnActionPerformed(ActionEvent evt){
        AboutDialog dlg = new AboutDialog((JFrame) this.getTopLevelAncestor(), true);
        dlg.setLocationRelativeTo(this);
        dlg.setVisible(true);        
    }

    void viewBtnAction() {
	String path = Config.getJavaHome() + File.separator + "lib" +
		File.separator + "javaws.jar";
	URL [] urls = new URL[1];
	try {
	    urls[0] = new URL("file", null, -1, path);
	    Thread t = Thread.currentThread();
	    ClassLoader cl = new URLClassLoader(urls, 
						t.getContextClassLoader());
	    t.setContextClassLoader(cl);
	    Class cacheViewer = cl.loadClass("com.sun.javaws.ui.CacheViewer");

	    JFrame parent = (JFrame) this.getTopLevelAncestor();

	    // Find a static showCacheViewer(JFrame parent) method
	    Class[] type = { (new JFrame()).getClass() };
	    Method method = cacheViewer.getMethod("showCacheViewer", type);
	    // Check that method is static
	    if (!Modifier.isStatic(method.getModifiers())) {
	        throw new NoSuchMethodException(
		    "com.sun.javaws.ui.CacheViewer.showCacheViewer");
	    }
	    method.setAccessible(true);
	    Object [] args = new Object[1];
	    args[0] = (Object) parent;
	    // Invoke method
	    method.invoke(null, args);
	} catch (Throwable e) {
	    Trace.ignored(e);
	}
    }

    private void tempFilesSettingsBtnActionPerformed(ActionEvent evt) {           
        CacheSettingsDialog cacheSettingsDlg = 
            new CacheSettingsDialog( (JFrame) this.getTopLevelAncestor(),
                                     true);
        cacheSettingsDlg.pack();
        cacheSettingsDlg.setLocationRelativeTo(this);
        cacheSettingsDlg.setVisible(true);
    }

    private void networkSettingsBtnActionPerformed(ActionEvent evt) {        
        /*
         * Set ControlPanel to be the parent of the NetworkSettingsDialog.
         * This way, when ControlPanel is maximized, the NetworkSettingsDialog
         * will appear on top of ControlPanel window.
         */
        NetworkSettingsDialog networkSettingsDlg = 
            new NetworkSettingsDialog ( (JFrame) this.getTopLevelAncestor(),
                                        true);
        networkSettingsDlg.pack();
        networkSettingsDlg.setLocationRelativeTo(this);
        networkSettingsDlg.setVisible(true);
    }

    private String getMessage(String id)
    {
	return ResourceManager.getMessage(id);
    }
    
    public JButton makeButton(String key) {
	JButton b = new JButton(getMessage(key));
	b.setMnemonic(ResourceManager.getVKCode(key + ".mnemonic"));
	return b;
    }    

    void enableViewButton(boolean enabled) {

        // unapplied info is prior to disabled info. 
	if(!enabled){ 
            cacheViewBtn.setToolTipText(
                getMessage("general.cache.view.tooltip.unapplied")); 
            cacheViewBtn.setEnabled(false); 
         } 
         else if (!Config.getBooleanProperty(Config.CACHE_ENABLED_KEY)) { 
             cacheViewBtn.setToolTipText(
                 getMessage("general.cache.view.tooltip.disabled"));
            cacheViewBtn.setEnabled(false);
         } 
         else {
             cacheViewBtn.setToolTipText(
                 getMessage("general.cache.view.tooltip"));
             cacheViewBtn.setEnabled(true);
         } 
    }
}
