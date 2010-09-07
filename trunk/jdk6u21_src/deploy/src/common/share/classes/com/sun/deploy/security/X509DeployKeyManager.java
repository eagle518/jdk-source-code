/*
 * @(#)X509DeployKeyManager.java	1.36 10/05/21
 *
 * Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import java.security.KeyStore.Builder;
import java.security.KeyStore.CallbackHandlerProtection;
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
import java.util.Arrays;
import java.util.Iterator;
import javax.net.ssl.X509KeyManager;
import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.ManagerFactoryParameters;
import javax.net.ssl.KeyStoreBuilderParameters;
import javax.net.ssl.HandshakeCompletedListener;
import javax.net.ssl.HandshakeCompletedEvent;
import javax.net.ssl.SSLSocket;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JPasswordField;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import java.net.PasswordAuthentication;
import java.lang.reflect.Method;

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
    private String userKeyStore = null;
    private String systemKeyStore = null;
    private KeyStore browserKeyStore = null;
    private boolean isWindows = 
		(Config.getOSName().indexOf("Windows") != -1);

    // Cached hostname and client authentication certificate
    private static HashMap clientAuthCertsCachedMap = new HashMap();
    
    // Thread local storage to remember if client certificate dialog was 
    // cancelled by the users to avoid future recursive calls from JSSE.
    //
    private static ThreadLocal clientCertDialogCancelled = new ThreadLocal() 
    {
	protected synchronized Object initialValue() {
	    return Boolean.FALSE;
	}
    };

    // Thread local storage to remember if password dialog was 
    // cancelled by the users to avoid future recursive calls from JSSE.
    //
    private static ThreadLocal passwdDialogCancelled = new ThreadLocal() {
	protected synchronized Object initialValue() {
	    return Boolean.FALSE;
	}
    };

    /*
     */
    public X509DeployKeyManager()
    {
	// Get client auth certificate file deployment.clientauthcerts
        userKeyStore = Config.getUserClientAuthCertFile();
        systemKeyStore = Config.getSystemClientAuthCertFile();
        
	// see if user is allowed to use browser keystore
	if (Config.getBooleanProperty(Config.SEC_USE_BROWSER_KEYSTORE_KEY)) 
	{ 
	    Service service = ServiceManager.getService();
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
                public Object run() throws KeyStoreException, 
		 	NoSuchAlgorithmException,
                	NoSuchProviderException, FileNotFoundException, 
			IOException, UnrecoverableKeyException, 
			CertificateException
                {
		  do_init();
		  return null;
		}
             });
	} catch (PrivilegedActionException e) {
	    Exception ex = e.getException();
            if (ex instanceof KeyStoreException) {
          	throw (KeyStoreException)ex;
            } else if (ex instanceof NoSuchAlgorithmException) {
          	throw (NoSuchAlgorithmException)ex;
            } else if (ex instanceof NoSuchProviderException) {
          	throw (NoSuchProviderException)ex;
            } else if (ex instanceof FileNotFoundException) {
          	throw (FileNotFoundException)ex;
            } else if (ex instanceof IOException) {
          	throw (IOException)ex;
            } else if (ex instanceof UnrecoverableKeyException) {
          	throw (UnrecoverableKeyException)ex;
            } else if (ex instanceof CertificateException) {
          	throw (CertificateException)ex;
            } else {
	    	Trace.securityPrintException(e);
	    }
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
	browserKeyManager = getBrowserKeyManager(browserKeyStore);

	// Get myKeyManager
	// Runnig JRE 1.6 and later
	if (Config.isJavaVersionAtLeast16()) {
	   myKeyManager = getNewMyKeyManager(userKeyStore, systemKeyStore);
	}
	else {
	   // For running old JRE in Java Webstart
	   myKeyManager = getLegacyMyKeyManager(userKeyStore);
	}
    } 
   
    /**
     */
    public synchronized String chooseClientAlias(final String[] keyType, 
			final Principal[] issuers, final Socket socket) {
        HashMap clientAuthCertsMap = new HashMap();
        HashMap clientAuthTypeMap = new HashMap();
        String certName = null;
	String hostname = null;

        if (clientCertDialogCancelled.get().equals(Boolean.FALSE))
        {
	    // For JFB only fix 6357710
	    // Use reflection to access SSLSocketImpl class and method
	    // Get remote hostname
	    // 6951688: fix for 6357710 to be made available for SE
	    try {
	        Class sslClazz = Class.forName("com.sun.net.ssl.internal.ssl.SSLSocketImpl");

                // if (socket instanceof sslClass)
	        boolean sslImplFlag = sslClazz.isInstance(socket);

                if (sslImplFlag) {
	            //SSLSocketImpl socketImpl = (SSLSocketImpl)socket;
		    Object SSLSocketImplObj = sslClazz.cast(socket);

                    // hostname = socketImpl.getHost(); 
	            final Method privateGetHostMethod = sslClazz.getDeclaredMethod("getHost", null);

		    AccessController.doPrivileged(new PrivilegedExceptionAction() {
                        public Object run() throws IOException {
	                    privateGetHostMethod.setAccessible(true);
                            return null;
                        }
                    });

	            hostname = (String)privateGetHostMethod.invoke(SSLSocketImplObj, null);
	        }
	      } catch (Exception ex) {
		Trace.msgSecurityPrintln("clientauth.readFromCache.failed");
		if (ex != null) {
	    	    Trace.msgSecurityPrintln(ex.toString());
		}
	      }

	    // Get the list of matched certs name
	    for (int i = 0; i < keyType.length; i++)
	    {
    	        String[] aliases = getClientAliases(keyType[i], issuers);
    	        if ((aliases != null) && (aliases.length > 0))
	        {
        	    for (int j=0; j<aliases.length; j++)
		    {
			int certTypeLen = CertType.PLUGIN.getType().length();
			String newAlias = aliases[j].substring(certTypeLen);
                    	X509Certificate[] certs = getCertificateChain(newAlias);

			// Check client certificate before display to user
			try {
			  if (CertUtils.checkTLSClient(certs[0])) {
                    	    clientAuthCertsMap.put(newAlias, certs);

                    	    if (aliases[j].startsWith(CertType.PLUGIN.getType())) {
                       	   	clientAuthTypeMap.put(newAlias, CertType.PLUGIN);
			    }

                    	    if (aliases[j].startsWith(CertType.BROWSER.getType())) {
                           	clientAuthTypeMap.put(newAlias, CertType.BROWSER);
			    }
			  }
			} catch (CertificateException ce) {
			  Trace.msgSecurityPrintln("clientauth.checkTLSClient.failed", 
						    new Object[]{ newAlias});
			}
		    }
    	        }
	    }

	    // Display the dialog box for user to select
	    // Only display when user didn't hit cancel button
	    if (passwdDialogCancelled.get().equals(Boolean.FALSE))
            {
	        final HashMap theClientAuthCertsMap = clientAuthCertsMap;
	        final HashMap theClientAuthTypeMap = clientAuthTypeMap;

		// For JFB only fix 6357710
		// Check cache value first if it is available
		// 6951688: Fix for 6357710 to be made available for SE
		if (hostname != null && clientAuthCertsCachedMap.size() > 0) {
              	    Iterator hostCachedItor = clientAuthCertsCachedMap.keySet().iterator();
		    String cachedHostname = null;	
		    String certNameCached = null;	
		    while (hostCachedItor.hasNext()) {
			cachedHostname = (String)hostCachedItor.next();	
			if (cachedHostname.compareToIgnoreCase(hostname)==0) {
			    certNameCached = (String)clientAuthCertsCachedMap.get(cachedHostname);
			    Trace.msgSecurityPrintln("clientauth.readFromCache.success",
						      new Object[]{ certNameCached});
			    return certNameCached; 
			}
		    }
	          }

		// No cache value available
		if (Config.getBooleanProperty(Config.SEC_USE_CLIENTAUTH_AUTO_KEY) &&
			(theClientAuthCertsMap.size() == 1))
           	{
              	   Object[] certNameArray = theClientAuthCertsMap.keySet().toArray();
                   certName = (String) certNameArray[0];
           	}
		else {
	           DeploySysAction action = new DeploySysAction() {
                     public Object execute() {
		        return ClientCertDialog.showDialog(
			    theClientAuthCertsMap, theClientAuthTypeMap);
		     }
	           };
	           certName = (String) 
			DeploySysRun.executePrivileged(action, null);
		} // more than one cert
	    }

	    // set flag if user click on cancel button
	    if (certName == null) {
	        clientCertDialogCancelled.set(Boolean.TRUE);
	    }

	    // For JFB only fix 6357710
	    // add handshake listener
	    // 6951688: Fix for 6357710 to made for SE
	    if (socket instanceof SSLSocket) {
    		HandshakeCompletedListener myListener = new MyListener(hostname, certName);
    		((SSLSocket)socket).addHandshakeCompletedListener(myListener);
	    }

	    return certName;
        } else {
	    return null;
        }
    }

    public String chooseEngineClientAlias(String[] keyType,
	    Principal[] issuers, SSLEngine engine) {
	return chooseClientAlias(keyType, issuers, null);
    }

    public synchronized String chooseServerAlias(final String keyType, 
		final Principal[] issuers, final Socket socket) {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && 
		passwdDialogCancelled.get().equals(Boolean.FALSE)) {
	        init();
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}

	String serverAliasName = null;
	
	if (myKeyManager != null) {
	    serverAliasName = 
		myKeyManager.chooseServerAlias(keyType, issuers, socket);
	}
	if (serverAliasName == null && browserKeyManager != null) {
	    serverAliasName = browserKeyManager.chooseServerAlias(
						keyType, issuers, socket);
	}
	
	return serverAliasName;
    }

    public String chooseEngineServerAlias(String keyType,
	    Principal[] issuers, SSLEngine engine) { 
	return chooseServerAlias(keyType, issuers, null);
    }

    public synchronized X509Certificate[] getCertificateChain(
		final String alias) {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && 
		passwdDialogCancelled.get().equals(Boolean.FALSE)) {
	       init();
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}

	X509Certificate[] certChain = null;
	
	if (myKeyManager != null) {
	    if (!alias.contains("Mozilla") && !alias.contains("MSCrypto")) {
	       certChain = myKeyManager.getCertificateChain(alias);
	    }
	} 
	if (certChain == null && browserKeyManager != null) {
	    certChain = browserKeyManager.getCertificateChain(alias);
	}
	return certChain;	
    }

    public synchronized String[] getClientAliases(final String keyType, 
					final Principal[] issuers) {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && 
		passwdDialogCancelled.get().equals(Boolean.FALSE)) {
	        init();
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}
    
	String[] myClientAliases = null;
	String[] browserClientAliases = null;
	
	if (myKeyManager != null) {
	    myClientAliases = 
		myKeyManager.getClientAliases(keyType, issuers);
	}    
	if (browserKeyManager != null) {
	    browserClientAliases = 
		browserKeyManager.getClientAliases(keyType, issuers);
	}
	    
	if (myClientAliases == null) {
	    if (browserClientAliases != null) {
		for (int i=0; i<browserClientAliases.length; i++){
		    browserClientAliases[i] = CertType.BROWSER.getType() + browserClientAliases[i];
                }
	    }
	    return browserClientAliases;
	} else if (browserClientAliases == null) {
	    if (myClientAliases != null) {
                for (int i=0; i<myClientAliases.length; i++){
                    myClientAliases[i] = CertType.PLUGIN.getType() + myClientAliases[i];
                }
            }
	    return myClientAliases;
	} else {    
	    for (int i=0; i<myClientAliases.length; i++)
                myClientAliases[i] = CertType.PLUGIN.getType() + myClientAliases[i];

            for (int i=0; i<browserClientAliases.length; i++)
                browserClientAliases[i] = CertType.BROWSER.getType() + browserClientAliases[i];

	    String[] temp = new String[
		myClientAliases.length + browserClientAliases.length];
            
	    System.arraycopy(myClientAliases, 0, 
			     temp, 0, myClientAliases.length);

	    System.arraycopy(browserClientAliases, 0, 
			     temp, myClientAliases.length, 
			     browserClientAliases.length);
	    return temp;
	}
    }

    public synchronized String[] getServerAliases(final String keyType, 
					final Principal[] issuers) {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && 
		passwdDialogCancelled.get().equals(Boolean.FALSE)) {
	        init();
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}

	String[] myServerAliases = null;
	String[] browserServerAliases = null;
	
	if (myKeyManager != null)  {
	    myServerAliases = 
		myKeyManager.getServerAliases(keyType, issuers);
	}
	    
	if (browserKeyManager != null) {
	    browserServerAliases = 
		browserKeyManager.getServerAliases(keyType, issuers);
	}
	    
	if (myServerAliases == null) {
	    return browserServerAliases;
	} else if (browserServerAliases == null) {
	    return myServerAliases;
	} else {    
	    String[] temp = new String[
		myServerAliases.length + browserServerAliases.length];
            
	    System.arraycopy(myServerAliases, 0, 
		temp, 0, myServerAliases.length);

	    System.arraycopy(browserServerAliases, 0, 
		temp, myServerAliases.length, browserServerAliases.length);
	    return temp;
	}
    }

    public PrivateKey getPrivateKey(String alias) {
	try {
	    if ((myKeyManager == null && browserKeyManager == null) && 
		passwdDialogCancelled.get().equals(Boolean.FALSE)) {
	        init();
	    }
	} catch (Exception e) {
	    e.printStackTrace();
	}
    
	PrivateKey privateKey = null;
	
	if (myKeyManager != null) {
	    if (!alias.contains("Mozilla") && !alias.contains("MSCrypto")) {
	       privateKey = ((X509KeyManager)myKeyManager).getPrivateKey(alias);
	    }
	}

	if (privateKey == null && browserKeyManager != null) {
	    privateKey = 
		((X509KeyManager)browserKeyManager).getPrivateKey(alias);
	}

	return privateKey;	
    }

    private X509KeyManager getBrowserKeyManager(KeyStore myBrowserKeyStore)
		throws KeyStoreException, NoSuchAlgorithmException,
                	NoSuchProviderException, FileNotFoundException,
			IOException, UnrecoverableKeyException, 
			CertificateException

    {
	X509KeyManager myBrowserKeyManager = null;

	if (myBrowserKeyStore != null)
	{
	    // Load browser keystore
	    myBrowserKeyStore.load(null, new char[0]);

	    // Obtain key manager factory
	    KeyManagerFactory kmf = 
		KeyManagerFactory.getInstance("SunX509", "SunJSSE");

	    // Initialize key manager factory
	    kmf.init(myBrowserKeyStore, new char[0]);
	    KeyManager[] kmArray = kmf.getKeyManagers();

	    // Loop through until we find X509KeyManager object
	    int i = 0;
	    while (i < kmArray.length)
	    {
		if (kmArray[i] instanceof X509KeyManager)
		{
		    myBrowserKeyManager = (X509KeyManager) kmArray[i];
		    break;
		}
		else
		    i++;
	    } //end while loop	
	}		    	

	return myBrowserKeyManager;
    }

    private X509KeyManager getNewMyKeyManager(String userMykeyStore, String systemMykeyStore)
		throws KeyStoreException, NoSuchAlgorithmException,
                	NoSuchProviderException, FileNotFoundException,
			IOException, UnrecoverableKeyException, 
			CertificateException

    {
	// For running old JRE in Java Webstart
	X509KeyManager myNewKeyManager = null;

	File userKeyStoreFile = new File(userMykeyStore);
	File systemKeyStoreFile = new File(systemMykeyStore);
	if (userKeyStoreFile.exists() || systemKeyStoreFile.exists()) {
	   try {
             	// Specify keystore builder parameters for PKCS#12
               	PasswordCallbackHandler usermcb = new PasswordCallbackHandler
               		("clientauth.user.password.dialog.text");
          	PasswordCallbackHandler sysmcb = new PasswordCallbackHandler
               		("clientauth.system.password.dialog.text");

 		Builder fsBuilder = null;
		Builder ssBuilder = null;

		if (userKeyStoreFile.exists()){
		   fsBuilder = Builder.newInstance("JKS", null,
			userKeyStoreFile, new CallbackHandlerProtection(usermcb));
		}

		if (systemKeyStoreFile.exists()) {
		   ssBuilder = Builder.newInstance("JKS", null,
                        systemKeyStoreFile, new CallbackHandlerProtection(sysmcb));
		}

		// Wrap them as key manager parameters
		ManagerFactoryParameters ksParams = new KeyStoreBuilderParameters
			(Arrays.asList(new Builder[] {fsBuilder, ssBuilder}));

		// Obtain key manager factory
		KeyManagerFactory kmf = KeyManagerFactory.getInstance("NewSunX509");

		// Initialize key manager factory
		kmf.init(ksParams);

		KeyManager[] kmArray = kmf.getKeyManagers();

		// Loop through until we find X509KeyManager object
		int i = 0;
		while (i < kmArray.length) {
		   if (kmArray[i] instanceof X509KeyManager) {
		      myNewKeyManager = (X509KeyManager) kmArray[i];
		      break;
		   }
		   else {
		      i++;
		   }
		} //end while loop
	   }
	   catch (Exception ioe) {
     	      ioe.printStackTrace();

              if (Trace.isAutomationEnabled() == false) {
	    	 // Show error message
	         String errorMsg = getMessage("clientauth.password.dialog.error.text");
		 String errorTitle = getMessage("clientauth.password.dialog.error.caption");
		 UIFactory.showExceptionDialog(null, ioe, errorMsg, errorTitle);
	      }
	   }
	}

	return myNewKeyManager;
    }

    private X509KeyManager getLegacyMyKeyManager(String userMykeyStore)
		throws KeyStoreException, NoSuchAlgorithmException,
                	NoSuchProviderException, FileNotFoundException,
			IOException, UnrecoverableKeyException, 
			CertificateException

    {
	// For running old JRE in Java Webstart
	X509KeyManager myLegacyKeyManager = null;

	File mykeyStoreFile = new File(userMykeyStore);
	if (mykeyStoreFile.exists()) {
	   boolean tryAgain = true;
	    
	   // Loop until either keystore is loaded or is cancelled by user.
	   while (tryAgain) {  
	    	try {
		    // Pop up password dialog box to get keystore password
		    char[] keyPassphrase = 
			getPasswordDialog("clientauth.user.password.dialog.text");
		    
		    // If password dialog is cancelled.
		    if (passwdDialogCancelled.get().equals(Boolean.TRUE)) {
			break;
		    }
		
		    // Obtain key store
		    String keyStoreType = System.getProperty("javax.net.ssl.keyStoreType");

		    if (keyStoreType == null) {
			keyStoreType = "JKS";
		    }

		    KeyStore ks = KeyStore.getInstance(keyStoreType);
      
		    ks.load(new BufferedInputStream(
			new FileInputStream(userMykeyStore)), keyPassphrase);
		    
		    // Obtain key manager factory
		    KeyManagerFactory kmf = 
			KeyManagerFactory.getInstance("SunX509", "SunJSSE");

		    // Initialize key manager factory
		    kmf.init(ks, keyPassphrase);

		    KeyManager[] kmArray = kmf.getKeyManagers();

		    // Loop through until we find X509KeyManager object
		    int i = 0;
		    while (i < kmArray.length)
		    {
			if (kmArray[i] instanceof X509KeyManager)
			{
			    myLegacyKeyManager = (X509KeyManager) kmArray[i];
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
			String errorMsg = getMessage( "clientauth.password.dialog.error.text");
			String errorTitle = getMessage( "clientauth.password.dialog.error.caption");
			UIFactory.showExceptionDialog(null, ioe, errorMsg, errorTitle);
		    }
		}	    
	  } // While	loop    	      
	} // KeyStoreFile exist

	return myLegacyKeyManager;
    }

    private char[] getPasswordDialog(final String inLabel) 
    {
        // pass translated strings to the dialog.
        CredentialInfo passwordInfo = UIFactory.showPasswordDialog(null, 
                getMessage("password.dialog.title"),
                getMessage(inLabel), false, false, null, false);                

        // If user pressed "Cancel" in password dialog, return value
        // from dialog will be null.  Check here if user pressed "Cancel"
        if ( passwordInfo == null ) {
	    passwdDialogCancelled.set(Boolean.TRUE);
            return null;
        } else {
            return passwordInfo.getPassword();
        }
    }

    /**
     * Method to get an internationalized string from the resource.
     */
    private static String getMessage(String key)  {
        return ResourceManager.getMessage(key);
    }

    private static int getAcceleratorKey(String key) {
        return ResourceManager.getAcceleratorKey(key);
    }

    /**
     * Implement HandshakeCompletedListener to add valid client certificate into cache.
     */
    final static class MyListener implements HandshakeCompletedListener {
	private String hostname;
	private String certName;

	public MyListener(String socketHostName, String aliasName) {
	    hostname = socketHostName;
	    certName = aliasName;
	}

	public void handshakeCompleted(HandshakeCompletedEvent event) {
	    SSLSocket sslSocket = event.getSocket();

	    // Add this client certificate into cache
	    if (hostname != null) {
	        clientAuthCertsCachedMap.put(hostname, certName);
	    }

	    // remove this listener
	    sslSocket.removeHandshakeCompletedListener(this);
	}

	// We want to have only one listener for the same hostname
	public boolean equals(Object obj) {
	    if (!(obj instanceof MyListener)) return false;

	    MyListener newListener = (MyListener)obj;
	    if (newListener.hostname.compareToIgnoreCase(this.hostname) == 0) {
		return true;
	    }
	    else {
		return false;
	    }
	}
    }
}
