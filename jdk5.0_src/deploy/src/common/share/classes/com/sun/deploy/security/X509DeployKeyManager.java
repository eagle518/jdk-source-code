/*
 * @(#)X509DeployKeyManager.java	1.19 04/05/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.net.Socket;
import java.security.AccessController;
import java.security.UnrecoverableKeyException;
import java.security.KeyStore;
import java.security.Principal;
import java.security.PrivateKey;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.util.HashMap;
import javax.net.ssl.X509KeyManager;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLEngine;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPasswordField;
import javax.swing.LookAndFeel;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DialogFactory;
import com.sun.deploy.util.DeployUIManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;

/**
 * This class implements a simple key management policy, deciding what
 * certificate chains should be used.  That is, it is a "trust provider"
 * which is driven <em>purely</em> by locally stored certificates.  It
 * can be administratively substituted by another policy.
 *
 * @version 1.0
 * @author Dennis Gu 
 */

// NOTE:  this is final because we've not yet subclassed this and we don't
// know all of what's involved.  This _should_ probably get subclassed, if
// for no other reason than to enable user (or perhaps remote administrator)
// participation in trust decisions.
//
// Subclassing is like any other interface though:  it needs customers
// before it has a prayer of being done right.  And support for external
// annotations, like enabling/disabling or scoping the trust extended.

public final class X509DeployKeyManager implements X509KeyManager {

    private X509KeyManager myKeyManager = null;
    private X509KeyManager browserKeyManager = null;
    private String mykeyStore = null;
    private KeyStore browserKeyStore = null;
    private boolean isWindows = (Config.getOSName().indexOf("Windows") != -1);
    
    // Thread local storage to remember if client certificate dialog was 
    // cancelled by the users to avoid future recursive calls from JSSE.
    //
    private static ThreadLocal clientCertDialogCancelled = new ThreadLocal() 
						{
						    protected synchronized Object initialValue() 
						    {
							return Boolean.FALSE;
						    }
						};

    // Thread local storage to remember if password dialog was 
    // cancelled by the users to avoid future recursive calls from JSSE.
    //
    private static ThreadLocal passwdDialogCancelled = new ThreadLocal() 
							{
							    protected synchronized Object initialValue() 
							    {
								return Boolean.FALSE;
							    }
							};

    /*
     */
    public X509DeployKeyManager()
    {
	// Get client authentication certificate file deployment.clientauthcerts
        mykeyStore = Config.getUserClientAuthCertFile();
        
	// see if user is allowed to use browser keystore
	if (Config.getBooleanProperty(Config.SEC_USE_BROWSER_KEYSTORE_KEY)) 
	{ 
	    Service service = com.sun.deploy.services.ServiceManager.getService();
	    browserKeyStore = service.getBrowserClientAuthKeyStore();
	}        
    }

    private void init() throws KeyStoreException,
				NoSuchAlgorithmException,
				NoSuchProviderException,
				FileNotFoundException,
				IOException,
				UnrecoverableKeyException,
				CertificateException
    {
	try
	{
	  AccessController.doPrivileged(new PrivilegedExceptionAction() {

                public Object run() throws KeyStoreException, NoSuchAlgorithmException,
                                   NoSuchProviderException, FileNotFoundException, IOException,
				   UnrecoverableKeyException, CertificateException
                {
		  do_init();
		  return null;
		}
           });
	}
	catch (PrivilegedActionException e)
        {
	  Exception ex = e.getException();

          if (ex instanceof KeyStoreException)
          	throw (KeyStoreException)ex;
          else if (ex instanceof NoSuchAlgorithmException)
          	throw (NoSuchAlgorithmException)ex;
          else if (ex instanceof NoSuchProviderException)
          	throw (NoSuchProviderException)ex;
          else if (ex instanceof FileNotFoundException)
          	throw (FileNotFoundException)ex;
          else if (ex instanceof IOException)
          	throw (IOException)ex;
          else if (ex instanceof UnrecoverableKeyException)
          	throw (UnrecoverableKeyException)ex;
          else if (ex instanceof CertificateException)
          	throw (CertificateException)ex;
          else
	    	Trace.securityPrintException(e);
	}
    }

    /**
     * Initialize keyManager
     */
    private void do_init() throws KeyStoreException,
				NoSuchAlgorithmException,
				NoSuchProviderException,
				FileNotFoundException,
				IOException,
				UnrecoverableKeyException,
				CertificateException
    {
	// Get browserKeyManager
	if (browserKeyStore != null)
	{
	    // Load browser keystore
	    browserKeyStore.load(null, new char[0]);

	    // Obtain key manager factory
	    KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509", "SunJSSE");

	    // Initialize key manager factory
	    kmf.init(browserKeyStore, new char[0]);
	    KeyManager[] kmArray = kmf.getKeyManagers();

	    // Loop through until we find X509KeyManager object
	    int i = 0;
	    while (i < kmArray.length)
	    {
		if (kmArray[i] instanceof X509KeyManager)
		{
		    browserKeyManager = (X509KeyManager) kmArray[i];
		    break;
		}
		else
		    i++;
	    } //end while loop	
	}		    	

	// Get myKeyManager
	File keyStoreFile = new File(mykeyStore);
	if (keyStoreFile.exists())
	{
	    boolean tryAgain = true;
	    
	    // Loop until either keystore is loaded or is cancelled by user.
	    while (tryAgain)
	    {  
		try
		{
		    // Pop up password dialog box to get keystore password
		    char[] keyPassphrase = getPasswordDialog("clientauth.password.dialog.text");
		    
		    // If password dialog is cancelled.
		    if (passwdDialogCancelled.get() == Boolean.TRUE)
			break;
		
		    // Obtain key store
		    String keyStoreType = System.getProperty("javax.net.ssl.keyStoreType");

		    if (keyStoreType == null)
			keyStoreType = "JKS";

		    KeyStore ks = KeyStore.getInstance(keyStoreType);
      
		    ks.load(new BufferedInputStream(new FileInputStream(mykeyStore)), keyPassphrase);
		    
		    // Obtain key manager factory
		    KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509", "SunJSSE");

		    // Initialize key manager factory
		    kmf.init(ks, keyPassphrase);

		    KeyManager[] kmArray = kmf.getKeyManagers();

		    // Loop through until we find X509KeyManager object
		    int i = 0;
		    while (i < kmArray.length)
		    {
			if (kmArray[i] instanceof X509KeyManager)
			{
			    myKeyManager = (X509KeyManager) kmArray[i];
			    break;
			}
			else
			    i++;
		    } //end while loop	
		  
		    tryAgain = false;  
		}
		catch (IOException ioe)
		{	
		    ioe.printStackTrace();

		    if (Trace.isAutomationEnabled() == false) 
		    {
			// Show error message
			String errorMsg = getMessage("clientauth.password.dialog.error.text");
			String errorTitle = getMessage("clientauth.password.dialog.error.caption");
			DialogFactory.showExceptionDialog(null, ioe, errorMsg, errorTitle);
		    }
		}	    
	    }	    	      
	}	
    } 
   
    /**
     */
    public synchronized String chooseClientAlias(final String[] keyType, 
				final Principal[] issuers, final Socket socket)
    {
      HashMap clientAuthCertsMap = new HashMap();
      String certName = null;

      // The user didn't click on cancel button
      if (clientCertDialogCancelled.get() == Boolean.FALSE)
      {
	// Get the list of matched certs name
	for (int i = 0; i < keyType.length; i++)
	{
    	    String[] aliases = getClientAliases(keyType[i], issuers);
    	    if ((aliases != null) && (aliases.length > 0))
	    {
        	for (int j=0; j<aliases.length; j++)
		{
		    X509Certificate[] certs = getCertificateChain(aliases[j]);
		    clientAuthCertsMap.put(aliases[j], certs);
		}
    	    }
	}

	// Display the dialog box for user to select
	final HashMap theClientAuthCertsMap = clientAuthCertsMap;
	certName =  (String)AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
			try {
				return DeploySysRun.execute(new DeploySysAction() {
					public Object execute() throws Exception {
						LookAndFeel lookAndFeel = null;
						try{
							// Change look and feel
							lookAndFeel = DeployUIManager.setLookAndFeel();
							
							ClientCertDialog dialog = new ClientCertDialog(theClientAuthCertsMap);
							return dialog.DoModal();
						} finally {
							// Restore look and feel
							DeployUIManager.restoreLookAndFeel(lookAndFeel);
						}
				}});
			} catch(Exception e) {
				// should never happen
				Trace.ignoredException(e);
				return null;
			}
	}});

	// set flag if user click on cancel button
	if (certName == null)
	   clientCertDialogCancelled.set(Boolean.TRUE);

	return certName;
      }
      else //User click on cancel button
	return null;
    }

    public String chooseEngineClientAlias(String[] keyType,
	    Principal[] issuers, SSLEngine engine) {
	return chooseClientAlias(keyType, issuers, null);
    }

    public synchronized String chooseServerAlias(final String keyType, 
				final Principal[] issuers, final Socket socket)
    {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && passwdDialogCancelled.get() == Boolean.FALSE)
	       init();
	}
	catch (Exception e) {
	    e.printStackTrace();
	}

	String serverAliasName = null;
	
	if (myKeyManager != null)
	    serverAliasName = myKeyManager.chooseServerAlias(keyType, issuers, socket);

	if (serverAliasName == null && browserKeyManager != null)
	    serverAliasName = browserKeyManager.chooseServerAlias(keyType, issuers, socket);
	
	return serverAliasName;
    }

    public String chooseEngineServerAlias(String keyType,
	    Principal[] issuers, SSLEngine engine) { 
	return chooseServerAlias(keyType, issuers, null);
    }

    public synchronized X509Certificate[] getCertificateChain(final String alias)
    {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && passwdDialogCancelled.get() == Boolean.FALSE)
	       init();
	}
	catch (Exception e) {
	    e.printStackTrace();
	}

	X509Certificate[] certChain = null;
	
	if (myKeyManager != null)
	    certChain = myKeyManager.getCertificateChain(alias);
	    
	if (certChain == null && browserKeyManager != null)
	    certChain = browserKeyManager.getCertificateChain(alias);
	    
	return certChain;	
    }

    public synchronized String[] getClientAliases(final String keyType, final Principal[] issuers)
    {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && passwdDialogCancelled.get() == Boolean.FALSE)
	       init();
	}
	catch (Exception e) {
	    e.printStackTrace();
	}
    
	String[] myClientAliases = null;
	String[] browserClientAliases = null;
	
	if (myKeyManager != null) 
	    myClientAliases = myKeyManager.getClientAliases(keyType, issuers);
	    
	if (browserKeyManager != null)
	    browserClientAliases = browserKeyManager.getClientAliases(keyType, issuers);
	    
	if (myClientAliases == null)
	    return browserClientAliases;
	else if (browserClientAliases == null)
	    return myClientAliases;
	else
	{    
	    String[] temp = new String[myClientAliases.length + browserClientAliases.length];
            
	    System.arraycopy(myClientAliases, 0, temp, 0, myClientAliases.length);
	    System.arraycopy(browserClientAliases, 0, temp, myClientAliases.length, browserClientAliases.length);
	    return temp;
	}
    }

    public synchronized String[] getServerAliases(final String keyType, final Principal[] issuers)
    {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && passwdDialogCancelled.get() == Boolean.FALSE)
	       init();
	}
	catch (Exception e) {
	    e.printStackTrace();
	}

	String[] myServerAliases = null;
	String[] browserServerAliases = null;
	
	if (myKeyManager != null) 
	    myServerAliases = myKeyManager.getServerAliases(keyType, issuers);
	    
	if (browserKeyManager != null)
	    browserServerAliases = browserKeyManager.getServerAliases(keyType, issuers);
	    
	if (myServerAliases == null)
	    return browserServerAliases;
	else if (browserServerAliases == null)
	    return myServerAliases;
	else
	{    
	    String[] temp = new String[myServerAliases.length + browserServerAliases.length];
            
	    System.arraycopy(myServerAliases, 0, temp, 0, myServerAliases.length);
	    System.arraycopy(browserServerAliases, 0, temp, myServerAliases.length, browserServerAliases.length);
	    return temp;
	}
    }

    public PrivateKey getPrivateKey(String alias)
    {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && passwdDialogCancelled.get() == Boolean.FALSE)
	       init();
	}
	catch (Exception e) {
	    e.printStackTrace();
	}
    
	PrivateKey privateKey = null;
	
	if (myKeyManager != null)
	    privateKey = ((X509KeyManager)myKeyManager).getPrivateKey(alias);

	if (privateKey == null && browserKeyManager != null)
	    privateKey = ((X509KeyManager)browserKeyManager).getPrivateKey(alias);

	return privateKey;	
    }

    private char[] getPasswordDialog(final String inLabel) 
    {
        try 
	{
            char[] passwd = (char[])(DeploySysRun.execute(new DeploySysAction() {
                public Object execute() throws Exception {
		    LookAndFeel lookAndFeel = null;

		    try
		    {
			// Change look and feel
			lookAndFeel = DeployUIManager.setLookAndFeel();
			
			return getPasswordDialogImp(inLabel);
		    }
		    finally
		    {
			// Restore look and feel
			DeployUIManager.restoreLookAndFeel(lookAndFeel);
		    }
                }}));
                
            if (passwd == null)
		passwdDialogCancelled.set(Boolean.TRUE);
	
	    return passwd;    
        }
        catch(Exception e) { // should never happen
            Trace.ignoredException(e);
            return null;
        }
    }

    private char[] getPasswordDialogImp(String inLabel)
    {
	// Pop up password dialog box
        Object dialogMsg = getMessage(inLabel);
        JPasswordField passwordField = new JPasswordField();

        Object[] msgs = new Object[2];
        msgs[0] = dialogMsg.toString();
        msgs[1] = passwordField;

        JButton okButton = new JButton(getMessage("clientauth.password.dialog.buttonOK"));
        JButton cancelButton = new JButton(getMessage("clientauth.password.dialog.buttonCancel"));

        String title = getMessage("clientauth.password.dialog.caption");
        Object[] options = {okButton, cancelButton};
        int selectValue = DialogFactory.showOptionDialog(DialogFactory.QUESTION_MESSAGE,
					msgs, title, options, options[0]);

        // for security purpose, DO NOT put password into String.
        // Reset password as soon as possible.
        final char[] mypassword = passwordField.getPassword();

        // User click OK button
        if (selectValue == 0)
	   return mypassword;
	else
	   return null;
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    private static String getMessage(String key)  {
        return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }
}
