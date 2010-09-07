/*
 * @(#)DeployCallbackHandler.java      1.2 05/02/28
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;


import java.io.IOException;
import javax.security.auth.callback.*;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.resources.ResourceManager;
import java.net.PasswordAuthentication;

/*
 * This class extends from CallbackHandler, and it is trigger by 
 * KeyStore.Builder class.
 * We will pop up a password dialog box when a password is needed to
 * access the specific keystore file
 *
 * @version 1.0
 * @author Dennis Gu
 */

final class PasswordCallbackHandler implements CallbackHandler {
  private String msgText = null;
  private char[] keyPassphrase = null;

  PasswordCallbackHandler(String msgText){
        this.msgText = msgText;
  }

  public void handle(Callback[] callbacks) throws
                IOException, UnsupportedCallbackException {

        for (int i=0; i<callbacks.length; i++) {
            if (callbacks[i] instanceof PasswordCallback) {
                PasswordCallback pc = (PasswordCallback)callbacks[i];

                // Pop up password dialog box to get keystore password
                CredentialInfo passwordInfo = 
                        UIFactory.showPasswordDialog(null, 
			getMessage("password.dialog.title"), 
			getMessage(msgText), false, false, null, false);

                // If user pressed "Cancel" in password dialog, return value
                // from dialog will be null.Check here if user pressed "Cancel".
		if ( passwordInfo != null) {
                    pc.setPassword(passwordInfo.getPassword());
		}
		else {
		   // User click on cancel button
		   pc.setPassword(null);
		}
            }
        }
  }

  /**
    * Method to get an internationalized string from the resource.
    */
  private static String getMessage(String key)  {
        return ResourceManager.getMessage(key);
  }
}
