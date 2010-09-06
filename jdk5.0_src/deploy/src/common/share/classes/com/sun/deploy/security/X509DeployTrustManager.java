/*
 * @(#)X509DeployTrustManager.java	1.44 04/03/04 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.deploy.security;

import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import javax.net.ssl.X509TrustManager;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;


/**
 * This class implements a simple trust management policy, deciding what
 * certificate chains should be used.  That is, it is a "trust provider"
 * which is driven <em>purely</em> by locally stored certificates.  It
 * can be administratively substituted by another policy.
 *
 * <P> Examples of more complex policies abound.  They include caching certs
 * as they are verified, consulting certificate servers (perhaps via LDAP)
 * and/or security policy servers, checking CRLs, supporting restrictions on
 * cross certification, implementing path length checks, recognizing key
 * usage or other restrictions associated with X509v3 certs, and more.
 * (The IETF PKIX working group defines an algorithm which incorporates
 * many of those policy features directly, and others by implication.)
 *
 * <P> This decider does NOT support user interaction for any policy, such
 * as accepting previously unrecognized CA certificates or to choose which
 * of several certified identities should be presented.  It currently supports
 * only two applications, namely authentication of peers in secure channels,
 * and verifying signatures used in code signing.
 *
 * <P><hr><em><b>NOTE:</b>  It'll be important to provide per-certificate
 * annotations, for example to disable use of a cert (while still storing
 * it reliably) or limit the purposes for which it may be used.  That's not
 * done yet by this class.</em>
 *
 * <P><em> Also, at this time only five CAs (all from VeriSign) are part of
 * the compiled-in defaults.  This will change.</em>
 *
 * @version 1.11
 * @author David Brownell
 */

// NOTE:  this is final because we've not yet subclassed this and we don't
// know all of what's involved.  This _should_ probably get subclassed, if
// for no other reason than to enable user (or perhaps remote administrator)
// participation in trust decisions.
//
// Subclassing is like any other interface though:  it needs customers
// before it has a prayer of being done right.  And support for external
// annotations, like enabling/disabling or scoping the trust extended.

public final class X509DeployTrustManager implements X509TrustManager {

    private X509TrustManager trustManager = null;

    // Certificate stores used for signed applet verification
    // Session store for storing trusted certificate
    private static CertStore rootStore = null;
    private static CertStore sslRootStore = null;
    private static CertStore permanentStore = null;
    private static CertStore sessionStore = null;
    private static CertStore deniedStore = null;
    private static CertStore browserSSLRootStore = null;
    private static boolean isBrowserSSLRootStoreLoaded = false;

    static
    {
	reset();		
    }

    /**
     * Reset the TrustDecider.
     */
    public static void reset()
    { 
	rootStore = RootCertStore.getInstance();
	sslRootStore = new SSLRootCertStore();
	permanentStore = new DeploySSLCertStore();
	sessionStore = new SessionCertStore();
	deniedStore = new DeniedCertStore();

	// see if user is allowed to use browser cert store
	if (Config.getBooleanProperty(Config.SEC_USE_BROWSER_KEYSTORE_KEY)) 
	{ 
	    Service service = com.sun.deploy.services.ServiceManager.getService();
	    browserSSLRootStore = service.getBrowserSSLRootCertStore();
	    isBrowserSSLRootStoreLoaded = false;
	}
    }

    /*
     * Note, we don't do any date validity checking here, because at
     * the time the keystore is loaded, we may have valid certificates,
     * but at access time, they may be invalid.  Or vice-versa,
     * we may have soon-to-be valid certificates that wouldn't be
     * loaded.  The checks should be done at the time of access.
     */
    public X509DeployTrustManager() throws KeyStoreException, 
					   NoSuchAlgorithmException,
					   NoSuchProviderException,
					   CertificateException  {

	// Obtain trust manager factory
	TrustManagerFactory tmf = TrustManagerFactory.getInstance("SunX509", "SunJSSE");

	// Initialize trust manager factory
	tmf.init((KeyStore) null);
	
	TrustManager[] tmArray = tmf.getTrustManagers();

	trustManager = (X509TrustManager) tmArray[0];
    }

    /**
     * Returns if the client certificates can be trusted.
     *
     * @param chain certificates which establish an identity for the client.
     *	    Chains of arbitrary length are supported, and certificates
     *	    marked internally as trusted will short-circuit signature checks.
     *
     * @throws CertificateException if the certificate chain is not trusted
     *          by this TrustManager.
     */
    public synchronized void checkClientTrusted(final X509Certificate chain[],
						final String authType)
	    throws CertificateException {
	// We may process the extended key usage extension
	// in the future. Then the code for checkClientTrusted()
	// may be different from the code for checkServerTrusted().

    	boolean rootCANotValid = false;
	boolean timeNotValid = false;

	if (trustManager == null) {
	    throw new IllegalStateException("TrustManager should not be null");
	}

	int result = 1;
	
	try
	{
	    // Load certificate store
	    rootStore.load();		
	    sslRootStore.load();		
	    permanentStore.load();		
	    sessionStore.load();
	    deniedStore.load();

	    // only load browser root store once
	    if (browserSSLRootStore != null && !isBrowserSSLRootStoreLoaded) {
		browserSSLRootStore.load();
		isBrowserSSLRootStoreLoaded = true;
	    }

	    // Check if the certificate is denied before
	    if (deniedStore.contains(chain[0])) {
		throw new CertificateException("Certificate has been denied");
	    }

	    try {
	        trustManager.checkClientTrusted(chain, authType);
		return;
	    } catch (CertificateException e) {
		// ignore, failed the check.
	    }

	    // We need to determine if the certificate has been stored
	    // in the session certificate store
	    if (sessionStore.contains(chain[0])) {
		return;
	    }

	    // We need to determine if the certificate has been stored
	    // in the permanent certificate store
	    if (permanentStore.contains(chain[0]))
		return;

	    //loop through all certs in chain.
	    int i = 0;
	    while (i < chain.length) 
	    {
	      // RootCACert or SSLRootCACert or BrowserSSLRootCACert is valid
	      if (rootStore.verify(chain[i]) == false 
	  	  && sslRootStore.verify(chain[i]) == false 
	  	  && (browserSSLRootStore == null || browserSSLRootStore.verify(chain[i]) == false))
	      {		  
		  rootCANotValid = true;
	      }

	      // Check if the cert is expired.
	      try {
		chain[i].checkValidity();
	      }
	      catch (CertificateExpiredException e1) {
		timeNotValid = true;
	      }
	      catch (CertificateNotYetValidException e2) {
		timeNotValid = true;
	      }

	      i++;
	    }

	    // Show dialog
	    if (Trace.isAutomationEnabled() == false)
	    {
		try {
		    final boolean theRootCANotValid = rootCANotValid;
		    final boolean theTimeNotValid = timeNotValid;
		    result = ((Integer)AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws Exception{
			    return DeploySysRun.execute(new DeploySysAction() {
				public Object execute() throws Exception {
				    TrustDeciderDialog dialog = new TrustDeciderDialog(chain, 0, chain.length, theRootCANotValid, theTimeNotValid);
				    dialog.setHttpsDialog(true);
				    return new Integer(dialog.DoModal());
				}
			    });
			}
		    })).intValue();
		} catch(Exception e) {
		    // should never happen
		    Trace.ignoredException(e);
		}
	    }
	    else
	    {	
		// If automation is enabled
		Trace.msgSecurityPrintln("x509trustmgr.automation.ignoreclientcert");
		result = 0;
	    }

	    if (result == 0)
	    {
		// Grant session 
		// Trust this certificate
		sessionStore.add(chain[0]);
		sessionStore.save();
	    }
	    else if (result == 2)
	    {
		// Grant always 
		// Trust this certificate
		permanentStore.add(chain[0]);
		permanentStore.save();
	    }
	    else
	    {
		// Deny
		deniedStore.add(chain[0]);
		deniedStore.save();
	    }
	}
	catch(CertificateException ce)
	{
	    throw ce;
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	}
    
	if (result != 0 && result != 2) {
            throw new CertificateException("Java couldn't trust Client");
        }

	return;
    }

    /**
     * Returns if the server certifcates can be trusted.
     *
     * @param chain certificates which establish an identity for the server.
     *	    Chains of arbitrary length are supported, and certificates
     *      marked internally as trusted will short-circuit signature checks.
     *
     * @throws CertificateException if the certificate chain is not trusted
     *          by this TrustManager.
     */
    public synchronized void checkServerTrusted(final X509Certificate chain[],
						final String authType) 
	    throws CertificateException {
	// We may process the extended key usage extension
	// in the future. Then the code for checkClientTrusted()
	// may be different from the code for checkServerTrusted().

    	boolean rootCANotValid = false;
	boolean timeNotValid = false;

	if (trustManager == null) {
	    throw new IllegalStateException("TrustManager should not be null");
	}

	int result = 1;

	try
	{	   
	    // Load certificate store
	    rootStore.load();
	    sslRootStore.load();
	    permanentStore.load();	
	    sessionStore.load();	
	    deniedStore.load();	

	    // only load browser root store once
	    if (browserSSLRootStore != null && !isBrowserSSLRootStoreLoaded) {
		browserSSLRootStore.load();
		isBrowserSSLRootStoreLoaded = true;
	    }
	   
	    // Check if the certificate is denied before
	    if (deniedStore.contains(chain[0])) {
		throw new CertificateException("Certificate has been denied");
	    }

	    try {
	        trustManager.checkServerTrusted(chain, authType);
		return;
	    } catch (CertificateException e) {
		// ignore, failed the check.
	    }

	    // We need to determine if the certificate has been stored
	    // in the session certificate store
	    if (sessionStore.contains(chain[0])) {
		return;
	    }

	    // We need to determine if the certificate has been stored
	    // in the permanent certificate store
	    if (permanentStore.contains(chain[0]))
		return;

	    //loop through all certs in chain.
	    int i = 0;
	    while (i < chain.length) 
	    {
		// RootCACert or SSLRootCACert or BrowserSSLRootCACert is valid
		if (rootStore.verify(chain[i]) == false 
		    && sslRootStore.verify(chain[i]) == false 
		    && (browserSSLRootStore == null || browserSSLRootStore.verify(chain[i]) == false))
		{		  
		    rootCANotValid = true;
		}

	      // Check if the cert is expired.
	      try 
	      {
		chain[i].checkValidity();
	      }
	      catch (CertificateExpiredException e1) {
		timeNotValid = true;
	      }
	      catch (CertificateNotYetValidException e2) {
		timeNotValid = true;
	      }

	      i++;
	    }

	    // Show dialog
	    if (Trace.isAutomationEnabled() == false)
	    {
		try {
		    final boolean theRootCANotValid = rootCANotValid;
		    final boolean theTimeNotValid = timeNotValid;
		    result = ((Integer)AccessController.doPrivileged(new PrivilegedExceptionAction() {
			public Object run() throws Exception{
			    return DeploySysRun.execute(new DeploySysAction() {
				public Object execute() throws Exception {
				    TrustDeciderDialog dialog = new TrustDeciderDialog(chain, 0, chain.length, theRootCANotValid, theTimeNotValid);
				    dialog.setHttpsDialog(true);
				    return new Integer(dialog.DoModal());
				}
			    });
			}
		    })).intValue();
		} catch(Exception e) {
		    // should never happen
		    Trace.ignoredException(e);
		}
	    }
	    else
	    {	
		// If automation is enabled
		Trace.msgSecurityPrintln("x509trustmgr.automation.ignoreservercert");
		result = 0;
	    }

	    if (result == 0)
	    {
		// Grant session 
		// Trust this certificate
		sessionStore.add(chain[0]);
		sessionStore.save();
	    }
	    else if (result == 2)
	    {
		// Grant always 
		// Trust this certificate
		permanentStore.add(chain[0]);
		permanentStore.save();
	    }
	    else
	    {
		// Deny
		deniedStore.add(chain[0]);
		deniedStore.save();
	    }
	}
	catch(CertificateException ce)
	{
	    throw ce;
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}

	if (result != 0 && result != 2) {
            throw new CertificateException("Java couldn't trust Server");
        }

	return;
    }

    /**
     * Returns a list of CAs accepted to authenticate entities for the
     * specified purpose.
     *
     * @param purpose activity for which CAs should be trusted
     * @return list of CAs accepted for authenticating such tasks
     */
    public X509Certificate[] getAcceptedIssuers() {

        return (X509Certificate[]) ((X509TrustManager)trustManager).getAcceptedIssuers();
    }
}
