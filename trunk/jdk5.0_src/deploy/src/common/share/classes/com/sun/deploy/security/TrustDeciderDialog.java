/*
 * @(#)TrustDeciderDialog.java	1.57 04/07/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.io.IOException;
import java.net.URL;
import java.security.Principal;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.cert.X509Certificate;
import java.text.MessageFormat;
import java.text.DateFormat;
import java.util.ArrayList;
import java.util.Date;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.LookAndFeel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.config.Config;


class TrustDeciderDialog implements ActionListener {

    public static final int TrustOption_GrantThisSession = 0;
    public static final int TrustOption_Deny = 1;
    public static final int TrustOption_GrantAlways = 2;


    private java.security.cert.Certificate[] certs = null;
    private int start  = 0;
    private int end = 0;
    private boolean rootCANotValid = false;
    private boolean timeNotValid = false;
    private boolean httpsDialog = false;
    private Date timeStampDate = null;
    private String codeTypeKey = null;

    /**
     * <P> Constructor for TrustDeciderDialog.
     * </P>
     *
     * @param certs Array of certificates
     */
    public TrustDeciderDialog(java.security.cert.Certificate[] certs, 
		int start, int end, boolean rootCANotValid, 
		boolean timeNotValid)  {
	this(certs, start, end, rootCANotValid, timeNotValid, null,
		"trustdecider.code.type.applet");
    }

    public TrustDeciderDialog(java.security.cert.Certificate[] certs,
		int start, int end, boolean rootCANotValid,
		boolean timeNotValid, Date timeStampDate)  {
	this(certs, start, end, rootCANotValid, timeNotValid, timeStampDate,
		"trustdecider.code.type.applet");
    }

    public TrustDeciderDialog(java.security.cert.Certificate[] certs, 
		int start, int end, boolean rootCANotValid, 
		boolean timeNotValid, Date timeStampDate, String codeTypeKey)  {
	this.certs = certs;
	this.start = start;
	this.end = end;
	this.rootCANotValid = rootCANotValid;
	this.timeNotValid = timeNotValid;
	this.timeStampDate = timeStampDate;
	this.codeTypeKey = codeTypeKey;
    }

    /**
     * <P> Show TrustDeciderDialog.
     * </P>
     *
     * @return 0 if "Grant this session" button is clicked.
     * @return 1 or -1 if "Deny" is clicked.
     * @return 2 if "Grant Always" button is clicked.
     */
    int DoModal()  
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    int ret = -1;

	    // Check if the certificate is a x.509 certificate
	    //
	    if (certs[start] instanceof X509Certificate 
		&& certs[end-1] instanceof X509Certificate)
	    {
		X509Certificate cert = (X509Certificate) certs[start];
		X509Certificate cert2 = (X509Certificate) certs[end-1];

		Principal prinSubject = cert.getSubjectDN();
		Principal prinIssuer = cert2.getIssuerDN();

		// Extract subject name
		String subjectDNName = prinSubject.getName();

		String subjectName = null;

		int i = subjectDNName.indexOf("CN=");
		int j = 0;

		if (i < 0)
		{
		    subjectName = getMessage("security.dialog.unknown.subject");
		}
		else
		{
		    try
		    {
			// Shift to the beginning of the CN text
			i = i + 3;

			// Check if it begins with a quote
			if (subjectDNName.charAt(i) == '\"')
			{
			    // Skip the quote
			    i = i + 1;

			    // Search for another quote
			    j = subjectDNName.indexOf('\"', i);
			}
			else
			{
			    // No quote, so search for comma
			    j = subjectDNName.indexOf(',', i);
			}

			if (j < 0)
			    subjectName = subjectDNName.substring(i);
			else
			    subjectName = subjectDNName.substring(i, j);
		    }
		    catch (IndexOutOfBoundsException e)
		    {
			subjectName = getMessage("security.dialog.unknown.subject");
		    }
		}


		// Extract issuer name
		String issuerDNName = prinIssuer.getName();
		String issuerName = null;

		i = issuerDNName.indexOf("O=");
		j = 0;	

		if (i < 0)
		{
		    issuerName = getMessage("security.dialog.unknown.issuer");
		}
		else
		{
		    try
		    {
			// Shift to the beginning of the O text
			i = i + 2;

			// Check if it begins with a quote
			if (issuerDNName.charAt(i) == '\"')
			{
			    // Skip the quote
			    i = i + 1;

			    // Search for another quote
			    j = issuerDNName.indexOf('\"', i);
			}
			else
			{
			    // No quote, so search for comma
			    j = issuerDNName.indexOf(',', i);
			}

			if (j < 0)
			    issuerName = issuerDNName.substring(i);
			else
			    issuerName = issuerDNName.substring(i, j);
		    }
		    catch (IndexOutOfBoundsException e)
		    {
			issuerName = getMessage("security.dialog.unknown.issuer");
		    }
		}

		// Construct dialog message
		ArrayList dialogMsgArray = new ArrayList();
		MessageFormat mf = null;
	    
		// Based on signed applet or Https site, display different messages in dialog box
		// Bug 4517290 fix

		if (getHttpsDialog()) {
		    String key = (rootCANotValid) ? 
			"security.dialog_https.text0a" : 
			"security.dialog_https.text0";
		    mf = new MessageFormat(getMessage(key));
		    Object [] args = { subjectName, issuerName };
		    dialogMsgArray.add(mf.format(args));
		} else {
		    String key = (rootCANotValid) ? 
			"security.dialog.text0a" : "security.dialog.text0";
		    mf = new MessageFormat(getMessage(key));
		    Object [] args = { getMessage(codeTypeKey), 
				       subjectName, issuerName };
		    dialogMsgArray.add(mf.format(args));
		}

                if (Config.getBooleanProperty(Config.SEC_NOTINCA_WARN_KEY)) { 
		    dialogMsgArray.add("");
   
		    JLabel label = new JLabel();
		    if (rootCANotValid) {
	                label.setText(getMessage("security.dialog.rootCANotValid"));
		        label.setIcon(DialogFactory.getWarningIcon());
		    } else {
			label.setText(getMessage("security.dialog.rootCAValid"));
		        label.setIcon(DialogFactory.getInfoIcon());
		    }
		    dialogMsgArray.add(label);
		}

		if (Config.getBooleanProperty(Config.SEC_EXPIRED_WARN_KEY)) {
		    dialogMsgArray.add("");
		    if (timeNotValid)
		    {
		        JLabel label = new JLabel(
				getMessage("security.dialog.timeNotValid"));
		        label.setIcon(DialogFactory.getWarningIcon());
		        dialogMsgArray.add(label);
		    }
		    else
		    {
		        // For Timestamp info
			if (timeStampDate != null) {
			   mf = new MessageFormat(getMessage("security.dialog.timeValidTS"));
			   Object[] argsTS = {getMessage(codeTypeKey)};

			   JLabel label = new JLabel(mf.format(argsTS));
		           label.setIcon(DialogFactory.getInfoIcon());
		           dialogMsgArray.add(label);
			}
			else {
		           JLabel label = new JLabel(
				getMessage("security.dialog.timeValid"));
		           label.setIcon(DialogFactory.getInfoIcon());
		           dialogMsgArray.add(label);
			}
		    }

		    // For Timestamp info
                    if (timeStampDate != null)
                    {
			dialogMsgArray.add("");

			mf = new MessageFormat(getMessage("security.dialog.timestamp.text1"));

			// Get the right date format for timestamp
			DateFormat df = DateFormat.getDateTimeInstance(DateFormat.LONG, DateFormat.LONG);
			String tsStr = df.format(timeStampDate);
			Object[] argsTS = {getMessage(codeTypeKey), tsStr};

			JLabel label = new JLabel(mf.format(argsTS));
			label.setIcon(DialogFactory.getInfoIcon());
			dialogMsgArray.add(label);
		    }
		}

		if (!rootCANotValid ) {
		    mf = new MessageFormat(getMessage("security.dialog.text1"));
		    Object[] args2 = { subjectName, subjectName };

		    dialogMsgArray.add(mf.format(args2));
		}

		JButton infoButton = new JButton(getMessage("security.dialog.buttonViewCert"));
		infoButton.setMnemonic(getAcceleratorKey("security.dialog.buttonViewCert")); 
		infoButton.addActionListener(this);

		JPanel buttonPanel = new JPanel();
		buttonPanel.add(infoButton);
		buttonPanel.setLayout(new FlowLayout(FlowLayout.RIGHT));
		dialogMsgArray.add(buttonPanel);

		JButton yesButton = new JButton(getMessage("security.dialog.buttonYes")); 
		JButton noButton = new JButton(getMessage("security.dialog.buttonNo")); 
		JButton alwaysButton = new JButton(getMessage("security.dialog.buttonAlways")); 
		yesButton.setMnemonic(getAcceleratorKey("security.dialog.buttonYes")); 
		noButton.setMnemonic(getAcceleratorKey("security.dialog.buttonNo")); 
		alwaysButton.setMnemonic(getAcceleratorKey("security.dialog.buttonAlways")); 
		Object[] options = { yesButton, 
				     noButton, 
				     alwaysButton };

		// Show dialog
		if (Trace.isAutomationEnabled() == false)
		{
		    if (rootCANotValid || timeNotValid) {
		       ret = DialogFactory.showOptionDialog(DialogFactory.QUESTION_MESSAGE, 
				    dialogMsgArray.toArray(), getMessage("security.dialog.caption"), options, options[1]);
		    }
		    else {
                       ret = DialogFactory.showOptionDialog(DialogFactory.QUESTION_MESSAGE,
                                    dialogMsgArray.toArray(), getMessage("security.dialog.caption"), options, options[0]);
		    }
		}
		else
		{	// If automation is enabled
		    Trace.msgSecurityPrintln("trustdecider.automation.trustcert");
		    ret = 0;
		}
	    }	    	

	    return ret;	    	
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

  
    /**
     * <P> Launch Certificate dialog if the "More Info" button is clicked.
     * </P>
     *
     * @param e ActionEvent object.
     */
    public void actionPerformed(ActionEvent e)  
    {
	// Show the certificate dialog
    	CertificateDialog dialog = new CertificateDialog(certs, start, end);
	dialog.DoModal();			
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
    private boolean getHttpsDialog()
    {
	return httpsDialog;
    }

    /**
     * Method set boolean value to TRUE if it call from HTTPS site.
     */
    public void setHttpsDialog(boolean httpsDialogType)
    {
	httpsDialog = httpsDialogType;
    }
}





