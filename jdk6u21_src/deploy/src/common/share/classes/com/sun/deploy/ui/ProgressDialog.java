/*
 * @(#)ProgressDialog.java	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

import java.net.URL;
import java.awt.Dialog;
import java.awt.Window;
import java.awt.Toolkit;
import java.awt.Image;
import java.awt.Dimension;
import java.awt.Component;
import java.security.cert.Certificate;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.ImageIcon;

import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;

public class ProgressDialog extends DialogTemplate {

    public final static int INVISIBLE = 9999;

    // non-public constructor - should only be called from UIFactory
    ProgressDialog(AppInfo ainfo, 
		   Component owner, 
		   String title,
                   String contentStr,
		   boolean includeOkBtn) 
    {
        super(ainfo, owner, title, ""); 
		
	setProgressContent(
               ResourceManager.getMessage("common.ok_btn"),
               ResourceManager.getMessage("common.cancel_btn"),
               contentStr,
	       includeOkBtn,
               INVISIBLE);
	showOk(includeOkBtn);
	stayAlive();
    }

    public void reset(AppInfo ainfo, String title, boolean includeOkBtn) {
	setTitle(title);
	showOk(includeOkBtn);
	setInfo(ainfo.getTitle(), ainfo.getVendor(), ainfo.getFrom());
	setMasthead("", false);
    }
    
    public void showProgress( int percent ) {
        super.progress(percent);
    }

    public void setTitle(String title) {
        getDialog().setTitle(title);
    }

    public void setMasthead(String text, boolean singleLine) {
        super.setMasthead(text, singleLine);
    }

    public void setApplication(String name, String publisher, String from) {
	URL url = null;
	if (from != null) {
	    try {
		url = new URL(from);
	    } catch (Exception e) {
	    }
	}
	setInfo(name, publisher, url);
    }

    public void setUserAnswer(int answer) {
	super.setUserAnswer(answer);
    }

    public void setIndeterminate(boolean value) {
	if (Config.isJavaVersionAtLeast14()) {
	    progressBar.setIndeterminate(value);
	}
    }
	
}

