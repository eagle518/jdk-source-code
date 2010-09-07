/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import com.sun.deploy.ui.UIFactory;

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
        if (_result < 0) { 
            String jreVersion = 
                ld.getResources().getSelectedJRE().getVersion();
            String title = ResourceManager.getString(
                "download.jre.prompt.title");

            String message = 
		ResourceManager.getString("download.jre.prompt", jreVersion);

            _result = UIFactory.showConfirmDialog(parent, ld.getAppInfo(),
						  message, title);
        }
        return (_result == UIFactory.OK);
    }
}
