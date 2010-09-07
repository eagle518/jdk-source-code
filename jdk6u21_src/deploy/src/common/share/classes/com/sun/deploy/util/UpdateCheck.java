/*
 * @(#)UpdateCheck.java	1.7 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.panel.PlatformSpecificUtils;

public class UpdateCheck {
    /**
     * Construct the UpdateCheck Dialog
     */
    public UpdateCheck() {}

    // This method should be called from win32 platforms only
    public static void showDialog() {
        
        // this is for loading regutils.dll, which is required by the two
        // JNI calls below
        new PlatformSpecificUtils();
        
	//From 5.0U6+ onwards: 
	//No more prompting user, enable JavaAutoUpdate silently
	//if was disabled and Windows platform && user is administrator
	// && if PromptAutoUpdateCheck registry key  exists
	//Leaving the method name as showDialog(), to minimize the changes made

	if (!shouldPromptForAutoCheck()) return;

	//int  result = UIFactory.showUpdateCheckDialog();
	//result == UIFactory.ASK_ME_LATER, ask Later. Nothing to do
	//result == UIFactory.OK, enable Java Update & remove 
        //                        PromptAutoUpdateCheck key
	//result == UIFactory.CANCEL, no AutoUpdateChecks, remove 
        //                            PromptAutoUpdateCheck key

	//if (result != UIFactory.ASK_ME_LATER) handleUserResponse(result);

	handleUserResponse(UIFactory.OK);
	return;
    }

    public static native boolean shouldPromptForAutoCheck();
    public static native void handleUserResponse(int response);

} 
