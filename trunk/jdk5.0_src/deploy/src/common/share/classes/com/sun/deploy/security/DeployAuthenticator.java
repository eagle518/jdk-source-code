/*
 * @(#)DeployAuthenticator.java	1.4 03/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.awt.Component;
import java.awt.Frame;
import java.net.Authenticator;
import java.net.PasswordAuthentication;
import java.util.HashMap;
import javax.swing.BoxLayout;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JPasswordField;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.Box;
import javax.swing.LookAndFeel;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.Trace;


/**
 * Assist proxy authentication and web server authentication.
 * A unique instance of this class is registered at startup.
 *
 * @see java.net.Authenticator
 * @see java.net.PasswordAuthentication
 */
public class DeployAuthenticator extends Authenticator
{
	// NTLM scheme
	private static final String SCHEME_NTLM = "NTLM";

    /**
     * <p>
     * HTTP/HTTPS connection to a specified site, only needs to be authenticated once.
     * To avoid that if BrowserAuthenticator actually returns a wrong username/passowrd,
     * which will cause fall back no happening, and Java side dialog box never show up,
     * we will have to track the site that has already been authenticated with
     * BrowserAuthenticator's username/password.
     * </p>
     */
     private static HashMap baSites = new HashMap();

    // Parent frame
    protected Frame parentFrame = null;

    /**
     * Return browser authenticator.
     */
    private BrowserAuthenticator getBrowserAuthenticator()
    {
	Service service = com.sun.deploy.services.ServiceManager.getService();

	return service.getBrowserAuthenticator();
    }

    /**
     * <p>
     * Called when password authorization is needed by the superclass. The
     * instance is registered as being the default http proxy authentication
     * facility and will be called by the HTTP framework classes.
     * </p>
     * @return The PasswordAuthentication collected from the
     *		user, or null if none is provided.
     */
    protected synchronized PasswordAuthentication getPasswordAuthentication()
    {
	PasswordAuthentication pa = null;

	try
	{
		java.net.InetAddress site = getRequestingSite();
		String siteName;
		if (site != null) {
				siteName = site.toString();
		} else {
			siteName = getRequestingHost();
			if(siteName == null || siteName.length() == 0)
				siteName = getMessage("net.authenticate.unknownSite");
		}

		StringBuffer key = new StringBuffer(getRequestingProtocol());
		key.append(':');
		key.append(getRequestingHost());
		key.append(':');
		key.append(getRequestingPort());
		key.append(':');
		key.append(getRequestingScheme());
		key.append(':');
		key.append(getRequestingPrompt());
		// If we already try browser authenticator, but still can not get us through, don't try again
		if(!baSites.containsKey(key.toString())) {
		    BrowserAuthenticator browserAuthenticator = getBrowserAuthenticator();
		    if(browserAuthenticator != null) {
			pa = browserAuthenticator.getAuthentication(getRequestingProtocol(), 
						getRequestingHost(), getRequestingPort(), getRequestingScheme(), 
						getRequestingPrompt(), getRequestingURL(), getRequestorType() == RequestorType.PROXY);
			    if(pa != null) {
				baSites.put(key.toString(), key.toString());
				return pa;
			}
		    }
		}

		// Print out tracing
		StringBuffer buffer = new StringBuffer();
		buffer.append("Firewall authentication: site=");
		buffer.append(getRequestingSite());
		buffer.append(":" + getRequestingPort());
		buffer.append(", protocol=");
		buffer.append(getRequestingProtocol());
		buffer.append(", prompt=");
		buffer.append(getRequestingPrompt());
		buffer.append(", scheme=");
		buffer.append(getRequestingScheme());

		Trace.netPrintln(buffer.toString());

		// Request the username/password from the user
		pa = openDialog(siteName, getRequestingPrompt(), getRequestingScheme());
	}
	catch (Exception e)
	{
		// We should catch all exception so the connection may continue
		Trace.netPrintException(e);
	}

	return pa;
    }


    /*
     * <p>
     * Open the dialog to request the user/password information
     * from the user. The dialog box is based on Internet Explorer 4
     * dialog minus the icon
     * </p>
     *
     * @param site the HTTP site we are trying to connect
     * @param prompt the HTTP prompt from the server
     * @param scheme the HTTP scheme
     * @return the PasswordAuthentication object encapsulating the
     * username/password entered by the user
     */
    private PasswordAuthentication openDialog(String site, String prompt, String scheme) {
	final String theSite = site;
	final String thePrompt = prompt;
	final String theScheme = scheme;
	final Component parent = parentFrame;
	try {
	    return (PasswordAuthentication)DeploySysRun.execute(new DeploySysAction() {
		public Object execute() throws Exception {
		    return openDialogImpl(parent, theSite, thePrompt, theScheme);
		}});
	}
	catch(Exception e) {
	    // should never happen
	    Trace.ignoredException(e);
	    return null;
	}
    }

    private PasswordAuthentication openDialogImpl(Component parent, String site, String prompt, String scheme)
    {
	LookAndFeel lookAndFeel = null;

	try
	{
	    // Change look and feel
	    lookAndFeel = DeployUIManager.setLookAndFeel();

	    if (site == null)
		site = "";

	    if (prompt == null)
		prompt = "";

	    if (scheme == null)
		scheme = "";


	    JTextField userName = new JTextField();
	    userName.setColumns(15);
            JLabel userNameLabel = new JLabel(getMessage("net.authenticate.username"));
            userNameLabel.setDisplayedMnemonic(ResourceManager.getVKCode("net.authenticate.username.mnemonic"));
            userNameLabel.setLabelFor(userName);

            JPasswordField password = new JPasswordField();
	    password.setColumns(15);
            JLabel passwordLabel = new JLabel(getMessage("net.authenticate.password"));
            passwordLabel.setDisplayedMnemonic(ResourceManager.getVKCode("net.authenticate.password.mnemonic"));
            passwordLabel.setLabelFor(password);

            JTextField domain = new JTextField();
            domain.setColumns(15);
            JLabel domainLabel = new JLabel(getMessage("net.authenticate.domain"));
            domainLabel.setDisplayedMnemonic(ResourceManager.getVKCode("net.authenticate.domain.mnemonic"));
            domainLabel.setLabelFor(domain);

            boolean isNTLM = (scheme != null)?SCHEME_NTLM.equalsIgnoreCase(scheme):false;

	    JPanel optionPane = new JPanel();
	    optionPane.setLayout(new BoxLayout(optionPane, BoxLayout.Y_AXIS));


	    optionPane.add(new JLabel(getMessage("net.authenticate.firewall")));
	    optionPane.add(Box.createVerticalStrut(5));
		if(!isNTLM) {
			optionPane.add(new JLabel(getMessage("net.authenticate.realm")));
			optionPane.add(Box.createVerticalStrut(5));
		}
	    optionPane.add(new JLabel(getMessage("net.authenticate.scheme")));
	    optionPane.add(Box.createVerticalStrut(15));
            optionPane.add(userNameLabel);
	    optionPane.add(Box.createVerticalStrut(15));
            optionPane.add(passwordLabel);
	    optionPane.add(Box.createVerticalStrut(15));
		if(isNTLM) {
                        optionPane.add(domainLabel);
			optionPane.add(Box.createVerticalStrut(15));
		}

	    JPanel optionPane2 = new JPanel();
	    optionPane2.setLayout(new BoxLayout(optionPane2, BoxLayout.Y_AXIS));
	    optionPane2.add(new JLabel(site));
	    optionPane2.add(Box.createVerticalStrut(5));
		if(!isNTLM) {
			optionPane2.add(new JLabel(prompt));
			optionPane2.add(Box.createVerticalStrut(5));
		}

	    optionPane2.add(new JLabel(scheme));
	    optionPane2.add(Box.createVerticalStrut(10));
	    optionPane2.add(userName);
	    optionPane2.add(Box.createVerticalStrut(10));
	    optionPane2.add(password);
		if(isNTLM) {
			optionPane2.add(Box.createVerticalStrut(10));
			optionPane2.add(domain);
		}

	    JPanel finalPane = new JPanel();
	    finalPane.setLayout(new BoxLayout(finalPane, BoxLayout.X_AXIS));
	    finalPane.add(optionPane);
	    finalPane.add(Box.createHorizontalStrut(15));
	    finalPane.add(optionPane2);

	    Object[] objects = new Object[] { finalPane };

	    int result = DialogFactory.showConfirmDialog(parent, objects, getMessage("net.authenticate.caption"));

	    if (result == JOptionPane.OK_OPTION)
	    {
			PasswordAuthentication pa;

			// for security purpose, DO NOT put password into String. Keep it in char array and
			// reset after used.
			char[] userPassword = password.getPassword();
			String theUserName;
			String theDomain = domain.getText();
			if(isNTLM && theDomain != null && theDomain.length() > 0) {
				theUserName = theDomain + '\\' + userName.getText();
			} else {
				theUserName = userName.getText();
			}


			pa = new PasswordAuthentication(theUserName, userPassword);
			// for security reason, reset password
			java.util.Arrays.fill(userPassword, ' ');

			return pa;

	    } else {
			return null;
	    }
	}
	finally
	{
	    // Restore look and feel
	    DeployUIManager.restoreLookAndFeel(lookAndFeel);
	}
    }

    /*
     * <p>
     * Helper method to load resources for i18n
     * </p>
     */
    private String getMessage(String key) {
	return ResourceManager.getMessage(key);
    }

    public void setParentFrame(Frame f) {
	parentFrame = f;
    }
}



