/*
 * %W% %E%
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import java.awt.BorderLayout;
import java.awt.Component;
import java.lang.Object;
import java.lang.String;
import javax.swing.BorderFactory;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextArea;
import com.sun.javaws.jnl.LaunchDesc;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;

public class AutoDownloadPrompt extends Object {

    public static int _result = -1;

    /**
     * Display a dialog to determine if the user wants to allow Java Web Start
     * to download a particular JRE version.
     *
     * @param parent  - component to parent the dialog.
     * @param ld      - the application launch descriptor.
     *
     * @return <code>true</code> if the user wants to allow the download 
     *         <code>false</code> otherwise.
     */
    public static boolean prompt(Component parent, LaunchDesc ld) {

	// only ask once
	if (_result >= 0) { return (_result == 0); }

        String appTitle = ld.getInformation().getTitle();
        String jreVersion = ld.getResources().getSelectedJRE().getVersion();
        String title = ResourceManager.getString("download.jre.prompt.title");

	String message[] =  {
	    ResourceManager.getString("download.jre.prompt.text1",
                                               appTitle, jreVersion),
	    "",
	    ResourceManager.getString("download.jre.prompt.text2")
	};


        JButton [] options = {
            new JButton(
		ResourceManager.getString("download.jre.prompt.okButton")),
            new JButton(
		ResourceManager.getString("download.jre.prompt.cancelButton"))
        };

        options[0].setMnemonic(ResourceManager.getAcceleratorKey(
	    "download.jre.prompt.okButton"));
        options[1].setMnemonic(ResourceManager.getAcceleratorKey(
	    "download.jre.prompt.cancelButton"));

        _result = DialogFactory.showOptionDialog(parent,
                            DialogFactory.QUESTION_MESSAGE,
                            message, title, options, options[0]);

        return (_result == 0);
    }
}
