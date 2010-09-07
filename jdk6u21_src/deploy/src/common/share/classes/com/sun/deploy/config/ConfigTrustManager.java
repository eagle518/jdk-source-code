/*
 * @(#)ConfigTrustManager.java	1.2 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;
import javax.net.ssl.TrustManagerFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import com.sun.deploy.util.Trace;

/**
 * This class implements a simple trust management policy, deciding what
 * certificate chains should be used.  That is, it is a "trust provider"
 * which is driven <em>purely</em> by locally stored certificates.  It
 * can be administratively substituted by another policy.
 *
 * This Class will load both JRE keystore and IE browser keystore to validate
 * the https server root CA.
 *
 * @version 1.0
 * @author Dennis Gu
 */

class ConfigTrustManager implements X509TrustManager {

    private static SSLSocketFactory ssf = null;
    private X509TrustManager defaultTM = null;
    private X509TrustManager nativeTM = null;

    public ConfigTrustManager() throws KeyStoreException,
                                           NoSuchAlgorithmException,
                                           NoSuchProviderException,
                                           CertificateException	 {

	TrustManager[] defaultTMArray = generateTrustManager(false);
	TrustManager[] nativeTMArray = generateTrustManager(true);

	if (defaultTMArray.length > 0) {
	    defaultTM = (X509TrustManager) defaultTMArray[0];
	}
	if (nativeTMArray.length > 0) {
	    nativeTM = (X509TrustManager) nativeTMArray[0];
	}
    }

    /*
     * This method will reset SSLSocketFactory 
     */
    public static void resetHttpsFactory(HttpsURLConnection https) {

        try {
	    if (ssf == null) {
	    	// Create Config TrustManager
	    	TrustManager tm = new ConfigTrustManager();
	    	TrustManager[] trustManagerArray = new TrustManager[1];
            	trustManagerArray[0] = tm;

            	// Set custom trust manager in SSL context
	    	SSLContext ctx = SSLContext.getInstance("TLS");
	    	ctx.init(null, trustManagerArray, null);

	    	ssf = ctx.getSocketFactory();
	    }

	    if (ssf != null) {
	        Trace.securityPrintln("Reset SSLSocketFactory using Config TrustManager");
                https.setSSLSocketFactory(ssf);
	    }
        } catch (Exception e) {
           e.printStackTrace();
        }
    }

    public void checkClientTrusted(X509Certificate chain[], String authType)
            throws CertificateException {

	return;
    }

    public X509Certificate[] getAcceptedIssuers() {
	X509Certificate[] defaultTMIssuers = ((X509TrustManager)defaultTM).getAcceptedIssuers();
	X509Certificate[] nativeTMIssuers = ((X509TrustManager)nativeTM).getAcceptedIssuers();

	X509Certificate[] allTMIssuers = 
			new X509Certificate[defaultTMIssuers.length + nativeTMIssuers.length];

	System.arraycopy(defaultTMIssuers, 0, allTMIssuers, 0, defaultTMIssuers.length);

	System.arraycopy(nativeTMIssuers, 0, allTMIssuers, defaultTMIssuers.length, 
			 nativeTMIssuers.length);

	return allTMIssuers;
    }

    public void checkServerTrusted(X509Certificate[] chain, String authType) 
			throws CertificateException {
	boolean fallback = false;

	if (defaultTM == null && nativeTM == null) {
	    throw new CertificateException("Both TrustManager are null");
	}

	if (defaultTM != null) {
	    try {
		defaultTM.checkServerTrusted(chain, authType);
		return;
	    } catch (CertificateException ce) {
		Trace.securityPrintln("Default TrustManager check failed");
		if (nativeTM == null) {
		    Trace.securityPrintln("No native TrustManager available");
		    throw ce;
		}
		else {
		    fallback = true;
		}
	    }
	}

	if (nativeTM != null && (fallback || defaultTM == null)) {
	    try {
		Trace.securityPrintln("Using native TrustManager");
	        nativeTM.checkServerTrusted(chain, authType);
	    } catch (CertificateException ce) {
		Trace.securityPrintln("Native(IE browser) TrustManager check failed");
		throw ce;
	    }
	}
    }

    private TrustManager[] generateTrustManager(boolean nativeFlag) {
	String defaultPasswd = "";

	try {
	    String algorithm = TrustManagerFactory.getDefaultAlgorithm();
	    TrustManagerFactory tmf = TrustManagerFactory.getInstance(algorithm);

	    if (nativeFlag) {
		// Using IE Browser keystore
	    	KeyStore ks = KeyStore.getInstance("Windows-ROOT");
	    	ks.load(null, defaultPasswd.toCharArray());

	    	tmf.init(ks);
	    }
	    else {
		// Using default keystore
	        tmf.init((KeyStore)null);
	    }
	    return tmf.getTrustManagers();
	} catch (Exception e) {
	    e.printStackTrace();
	}

	return null;
    }
}
