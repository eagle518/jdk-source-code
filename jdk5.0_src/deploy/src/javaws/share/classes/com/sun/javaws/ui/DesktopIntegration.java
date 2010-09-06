/*
 * @(#)DesktopIntegration.java	1.14 04/04/04
 *
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import com.sun.javaws.Main;
import com.sun.javaws.SplashScreen;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.ShortcutDesc;
import com.sun.deploy.util.*;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;

public class DesktopIntegration extends JOptionPane {

    private int _answer = Config.SHORTCUT_ASK_USER;

    public DesktopIntegration(Frame owner, String appTitle, boolean doDesktop, boolean doMenu) {
        super();
	initComponents(appTitle, doDesktop, doMenu);
    }

    private void initComponents(String appTitle, boolean doDesktop, boolean doMenu) {
        Object messages[] = new Object[2];
        JButton buttons[] = new JButton[3];

        buttons[0] = new JButton(
		ResourceManager.getString("install.yesButton"));
        buttons[0].setMnemonic(
		ResourceManager.getVKCode("install.yesMnemonic"));
        final JButton yesButton = buttons[0];

        buttons[1] = new JButton(
		ResourceManager.getString("install.noButton"));
        buttons[1].setMnemonic(
		ResourceManager.getVKCode("install.noMnemonic"));
        final JButton noButton = buttons[1];
    
        buttons[2] = new JButton(
		ResourceManager.getString("install.configButton"));
        buttons[2].setMnemonic(
		ResourceManager.getVKCode("install.configMnemonic"));
	final JButton configureButton = buttons[2];

	int i;
        for (i=0; i<3; i++) {
            buttons[i].addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent ae) {
                    JButton b = (JButton) ae.getSource();
		    if (b == configureButton) {
		        Main.launchJavaControlPanel("advanced");
			return;
		    }
		    if (b ==  yesButton) { _answer = Config.SHORTCUT_YES; }
		    else if (b == noButton) { _answer = Config.SHORTCUT_NO; }
		    Component source = (Component) ae.getSource();
		    JDialog dialog = null;
	
		    // Iterate through container to find JDialog
		    while (source.getParent() != null) {		
			if (source instanceof JDialog) {
			    dialog = (JDialog) source;
			}
			source = source.getParent();
		    }
		    
		    if (dialog != null) {
			dialog.setVisible(false);
		    }
                }
            });  
        }

	String message = null;

	if (Config.getOSName().equalsIgnoreCase("Windows")) {
	    // windows platform
	    if (doDesktop && doMenu) {
		message = ResourceManager.getString("install.windows.both.message", appTitle);
	    } else if (doDesktop) {
		message = ResourceManager.getString("install.desktop.message", appTitle);
	    } else if (doMenu) {
		message = ResourceManager.getString("install.windows.menu.message", appTitle);
	    }
	} else {
	    // unix platform GNOME
	    if (doDesktop && doMenu) {
		message = ResourceManager.getString("install.gnome.both.message", appTitle);
	    } else if (doDesktop) {
		message = ResourceManager.getString("install.desktop.message", appTitle);
	    } else if (doMenu) {
		message = ResourceManager.getString("install.gnome.menu.message", appTitle);
	    }
	}

	setOptions(buttons);
	setMessage(message);
	setMessageType(JOptionPane.WARNING_MESSAGE);
	setInitialValue(buttons[0]);
    }
    
    public static int showDTIDialog(Frame owner, LaunchDesc ld) {

	String appTitle = ld.getInformation().getTitle();
	ShortcutDesc sd = ld.getInformation().getShortcut();

	boolean doDesktop = (sd == null) ? true : sd.getDesktop();
	boolean doMenu = (sd == null) ? true : sd.getMenu();

	LookAndFeel lookAndFeel = DeployUIManager.setLookAndFeel();
	
	DesktopIntegration pane = new DesktopIntegration(owner, appTitle, doDesktop, doMenu);
	JDialog dialog = pane.createDialog(owner, ResourceManager.getString("install.title", appTitle));
	DialogFactory.positionDialog(dialog);
	SplashScreen.hide();
	dialog.setVisible(true);
	
	DeployUIManager.restoreLookAndFeel(lookAndFeel);
	
	return pane._answer;
	
    }
}






