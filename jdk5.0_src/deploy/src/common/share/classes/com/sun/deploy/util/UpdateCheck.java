/*
 * @(#)UpdateCheck.java	1.2 04/03/12
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import javax.swing.JButton;
import com.sun.deploy.resources.ResourceManager;

public class UpdateCheck {

    /**
     * Construct the UpdateCheck Dialog
     */
    public UpdateCheck() {}

    public static void showDialog() {
        //
        // Prompt User for JavaUpdate Enabling, if
        // Windows platform &&	user is administrator
	// && if  PromptAutoUpdateCheck registry key  exists

	if (!shouldPromptForAutoCheck()) return;
	int  result = DialogFactory.showUpdateCheckDialog();
	//result == 2, ask Later. Nothing to do
	//result == 0, enable Java Update & remove PromptAutoUpdateCheck key
	//result ==1, no AutoUpdateChecks, remove PromptAutoUpdateCheck key
	if (result != 2) handleUserResponse(result);
	return;
    }

    public static native boolean shouldPromptForAutoCheck();
    public static native void handleUserResponse(int response);

} 
