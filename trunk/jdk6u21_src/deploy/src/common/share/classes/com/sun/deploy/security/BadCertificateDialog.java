/*
 * @(#)BadCertificateDialog.java	1.11 10/03/24 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.security.cert.CertificateException;
import java.security.CodeSource;
import java.text.MessageFormat;
import java.util.ArrayList;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JPanel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.ui.AppInfo;
import com.sun.deploy.ui.UIFactory;


public class BadCertificateDialog {

    private static boolean _isHttps = false;

    /**
     * <P> show BadCertificateDialog.
     * </P>
     *
     */
    public static void showDialog(CodeSource cs, final AppInfo ainfo, 
                             final Exception exception) {

        final java.security.cert.Certificate[] certs = cs.getCertificates();

        String message;
        String title = getMessage("security.badcert.caption");
               
        if (getHttpsDialog()) {
            message = getMessage("security.badcert.https.text");
        } else if (exception instanceof CertificateConfigException) {
            message = getMessage("security.badcert.config.text");
        } else {
            message = getMessage("security.badcert.text");
        }

        // Show dialog
        if (Trace.isAutomationEnabled() == false) {
            UIFactory.showCertificateExceptionDialog(
		null, ainfo, exception, message, title, certs);
        } else {       // If automation is enabled
             Trace.msgSecurityPrintln("trustdecider.automation.badcert");
        }    
    }

    private static String getMessage(String key)  {
        return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }
    
    /**
     * Method returns TRUE value when it came from HTTPS site 
     */
    private static boolean getHttpsDialog() {
        return _isHttps;
    }

    /**
     * Method set boolean value to TRUE if it call from HTTPS site.
     */
    public static void setHttpsDialog(boolean httpsDialogType) {
        _isHttps = httpsDialogType;
    }
}





