/*
 * @(#)UpdateDialog.java	1.5 10/03/24
 *
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui;

import com.sun.javaws.jnl.LaunchDesc;
import com.sun.javaws.jnl.UpdateDesc;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.resources.ResourceManager;

public class UpdateDialog {

    private static final String PROMPT_RUN = "update.dialog.prompt-run";
    private static final String PROMPT_UPDATE = "update.dialog.prompt-update";
    private static final String TITLE_KEY = "update.dialog.title";
    
    /**
     * Display a dialog to determine if user wants to do update
     * @param ld the main application LaunchDesc
     * @param dw the DownloadWinow which is the parent of the popup dialog
     *           Use browser window if null when in plugin
     * @return true if user want to do update, false if want to launch from
     *              cache or exit
     */
    
    public static boolean showUpdateDialog(LaunchDesc ld, DownloadWindow dw) {
	String key = getKey(ld);
        int result;
	
	String name = ld.getInformation().getTitle();
	String message = ResourceManager.getString(key, name);
	String title = ResourceManager.getMessage(TITLE_KEY);

	if (null == dw) {
	    result = showConfirmDialog(ld, message, title);
        } else {
            result = dw.showConfirmDialog(ld, message, title);
        }
	
	return (result == UIFactory.OK);
    }
    
    private static int showConfirmDialog(LaunchDesc ld, String message, String title) {
	return UIFactory.showConfirmDialog(null, ld.getAppInfo(),
					   message, title);
    }
    
    private static String getKey(LaunchDesc ld) {
	return (ld.getUpdate().getPolicy() == UpdateDesc.POLICY_PROMPT_RUN)? PROMPT_RUN : PROMPT_UPDATE;
    }
}







