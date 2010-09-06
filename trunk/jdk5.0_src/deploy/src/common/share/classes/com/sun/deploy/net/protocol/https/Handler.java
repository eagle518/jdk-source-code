/*
 * @(#)Handler.java	1.10 04/03/24
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	HTTP stream opener
 */

package com.sun.deploy.net.protocol.https;

import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.SecureRandom;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.KeyManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.security.BrowserKeystore;
import com.sun.deploy.services.ServiceManager;


/** open an http input stream given a URL */
public class Handler extends sun.net.www.protocol.https.Handler 
{
    static 
    {
	try 
	{	  
	    // Hook up TrustManager
	    java.security.AccessController.doPrivileged(new PrivilegedExceptionAction()
	    {
		public Object run() throws Exception
		{
		    // see if user is allowed to use browser cert store
		    if (Config.getBooleanProperty(Config.SEC_USE_BROWSER_KEYSTORE_KEY)) {
			// register security providers for browser keystore
			BrowserKeystore.registerSecurityProviders();
		    }

		    // Get the platform dependent Random generator
		    final SecureRandom sr = ServiceManager.getService().getSecureRandom();
		    
		    sr.nextInt();

		    // Hostname verifier
		    HostnameVerifier verifier = null;
		 
		    // Create hostname verifier			
		    verifier = new com.sun.deploy.security.CertificateHostnameVerifier();
		  
		    javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(verifier);

		    // Get SSL context
		    SSLContext sslContext = SSLContext.getInstance("SSL");

		    // Create custom trust manager
		    TrustManager trustManager = new com.sun.deploy.security.X509DeployTrustManager();

		    TrustManager[] trustManagerArray = new TrustManager[1];
		    trustManagerArray[0] = trustManager; 

		    // Create custom key manager
		    KeyManager keyManager = new com.sun.deploy.security.X509DeployKeyManager();

		    KeyManager[] keyManagerArray = new KeyManager[1];
		    keyManagerArray[0] = keyManager;

		    // Set custom keymanager and trust manager in SSL context
		    sslContext.init(keyManagerArray, trustManagerArray, sr);
		    
		    javax.net.ssl.HttpsURLConnection.setDefaultSSLSocketFactory(sslContext.getSocketFactory());

		    return null;
		}
	    });
	} 
	catch (PrivilegedActionException e) 
	{
	    e.printStackTrace();
	}
    }
}
