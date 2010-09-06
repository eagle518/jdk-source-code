/*
 * @(#)BadCertificateDialog.java	1.4 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import javax.swing.LookAndFeel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeployUIManager;


public class BadCertificateDialog {

    private static boolean _isHttps = false;

    /**
     * <P> show BadCertificateDialog.
     * </P>
     *
     */
    public static void show(CodeSource cs, String codeTypeKey, 
		 	    final Exception exception) {

        final java.security.cert.Certificate[] certs = cs.getCertificates();
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();


            // Construct dialog message
            ArrayList dialogMsgArray = new ArrayList();
            MessageFormat mf = null;
           
            if (getHttpsDialog()) {
                mf = new MessageFormat(getMessage(
			"security.badcert.https.text"));
            } else if (exception instanceof CertificateConfigException) {
                mf = new MessageFormat(getMessage(
		    "security.badcert.config.text"));
	    } else {
                mf = new MessageFormat(getMessage(
		    "security.badcert.text"));
	    }

            Object[] args = { getMessage(codeTypeKey) };

            dialogMsgArray.add(mf.format(args));

            JButton exceptionButton = new JButton(
		getMessage("security.badcert.viewException"));
            exceptionButton.setMnemonic(
		getAcceleratorKey("security.badcert.viewException"));
            exceptionButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
		    DialogFactory.showExceptionDialog(exception);
		}
            });  

            JButton infoButton = new JButton(
		getMessage("security.badcert.viewCert"));
            infoButton.setMnemonic(
		getAcceleratorKey("security.badcert.viewCert"));
            infoButton.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    (new CertificateDialog(certs, 0, certs.length)).DoModal();
		}
            });  

            JPanel buttonPanel = new JPanel();
            buttonPanel.add(exceptionButton);
            buttonPanel.add(infoButton);
            buttonPanel.setLayout(new FlowLayout(FlowLayout.CENTER));
            dialogMsgArray.add(buttonPanel);

            // Show dialog
            if (Trace.isAutomationEnabled() == false) {
                DialogFactory.showInformationDialog(dialogMsgArray.toArray(), 
		    getMessage("security.badcert.caption"));
            } else {       // If automation is enabled
                Trace.msgSecurityPrintln("trustdecider.automation.badcert");
            }    
	} finally {
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
	String ret = com.sun.deploy.resources.ResourceManager.getMessage(key);
	return ret;
    }

    private static int getAcceleratorKey(String key) {
        return com.sun.deploy.resources.ResourceManager.getAcceleratorKey(key);
    }
    
    /**
     * Method returns TRUE value when it came from HTTPS site 
     */
    private static boolean getHttpsDialog()
    {
	return _isHttps;
    }

    /**
     * Method set boolean value to TRUE if it call from HTTPS site.
     */
    public static void setHttpsDialog(boolean httpsDialogType)
    {
	_isHttps = httpsDialogType;
    }
}





