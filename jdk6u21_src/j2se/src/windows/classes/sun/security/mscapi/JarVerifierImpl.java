/*
 * @(#)JarVerifierImpl.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.mscapi;

// NOTE: this class was taken from SunJCE
// both files should always be kept in sync

import java.io.*;
import java.util.*;
import java.util.jar.*;
import java.net.URL;
import java.net.JarURLConnection;
import java.net.MalformedURLException;

import java.security.*;
import java.security.cert.*;
import java.security.cert.Certificate;

/**
 * This class verifies a JAR file and all its supporting JAR files.
 * 
 * @author Sharon Liu
 * @since 1.6
 */
final class JarVerifierImpl {

    private static final boolean debug = false;

    // The URL for the JAR file we want to verify.
    private URL jarURL;

    /**
     * Creates a JarVerifier object to verify the given URL.
     *
     * @param jarURL the JAR file to be verified.
     */
    JarVerifierImpl(URL jarURL) {
	this.jarURL = jarURL;
    }

    /**
     * Verify the JAR file is signed by an entity which has a certificate
     * issued by a trusted CA.
     *
     * @param trustedCaCerts certificates of trusted CAs.
     */
    void verify(X509Certificate trustedSigner)
        throws JarException, IOException {
	try {
	    verifyJars(jarURL, null, trustedSigner);
	} catch (NoSuchProviderException nspe) {
	    throw new JarException("Cannot verify " + jarURL.toString());
	} catch (CertificateException ce) {
	    throw new JarException("Cannot verify " + jarURL.toString());
	}
    }

    /**
     * Verify a JAR file and all of its supporting JAR files are signed by 
     * a signer with a certificate which
     * can be traced back to a trusted CA.
     */
    private void verifyJars(URL jarURL, Vector verifiedJarsCache,
			    X509Certificate trustedSigner)
        throws NoSuchProviderException, CertificateException, IOException
    {
	String jarURLString = jarURL.toString();

	// Check whether this JAR file has been verified before.
	if ((verifiedJarsCache == null) ||
	    !verifiedJarsCache.contains(jarURLString)) {
	    
	    // Verify just one jar file and find out the information
	    // about supporting JAR files.
	    String supportingJars = verifySingleJar(jarURL, trustedSigner);

	    // Add the url for the verified JAR into verifiedJarsCache.
	    if (verifiedJarsCache != null)
		verifiedJarsCache.addElement(jarURLString);

	    // Verify all supporting JAR files if there are any.	    
	    if (supportingJars != null) {
		if (verifiedJarsCache == null) {
		    verifiedJarsCache = new Vector();
		    verifiedJarsCache.addElement(jarURLString);
		}
		verifyManifestClassPathJars(jarURL, 
					    supportingJars, 
					    verifiedJarsCache,
					    trustedSigner);
	    }
	}
	
    }

    private void verifyManifestClassPathJars(URL baseURL, 
					   String supportingJars,
					   Vector verifiedJarsCache,
					   X509Certificate trustedSigner) 
        throws NoSuchProviderException, CertificateException, IOException
    {
	// Get individual JAR file names
	String[] jarFileNames = parseAttrClasspath(supportingJars);

	try {
	    // For each JAR file, verify it
	    for (int i = 0; i < jarFileNames.length; i++) {
		URL url = new URL(baseURL, jarFileNames[i]);
		verifyJars(url, verifiedJarsCache, trustedSigner);
	    }
	} catch (MalformedURLException mue) {
	    MalformedURLException ex = new MalformedURLException(
		"The JAR file " + baseURL.toString() +
		" contains invalid URLs in its Class-Path attribute");
	    ex.initCause(mue);
	    throw ex;
	}
    }

    /*
     * Verify the signature on the JAR file and return
     * the value of the manifest attribute "CLASS_PATH".
     * If the manifest doesn't contain the attribute
     * "CLASS_PATH", return null.
     */
    private String verifySingleJar(URL jarURL, X509Certificate trustedSigner)
        throws NoSuchProviderException, CertificateException, IOException
    {
	// If the protocol of jarURL isn't "jar", we should 
	// construct a JAR URL so we can open a JarURLConnection
	// for verifying this provider.
	final URL url = jarURL.getProtocol().equalsIgnoreCase("jar")?
	                jarURL : new URL("jar:" + jarURL.toString() + "!/");

	JarFile jf = null;

	try {
	    try {
		jf = (JarFile) AccessController.doPrivileged(
				   new PrivilegedExceptionAction() {
		    public Object run() throws Exception {
			JarURLConnection conn = (JarURLConnection)
			    url.openConnection();
			return conn.getJarFile();
		    }
		});
	    } catch (java.security.PrivilegedActionException pae) {
		SecurityException se = new SecurityException(
		    "Cannot verify " + url.toString());
		se.initCause(pae);
		throw se;
	    }

	    // Read in each jar entry, so the subsequent call
	    // JarEntry.getCertificates() will return Certificates
	    // for signed entries.
	    // Note: Since jars signed by 3rd party tool, e.g., 
	    // netscape signtool, may have its manifest at the 
	    // end, two separate loops maybe necessary.
	    byte[] buffer = new byte[8192]; 
	    Vector entriesVec = new Vector(); 
	    
	    Enumeration entries = jf.entries(); 
	    while (entries.hasMoreElements()) { 
		JarEntry je = (JarEntry)entries.nextElement(); 
		entriesVec.addElement(je); 
		InputStream is = jf.getInputStream(je); 
		int n; 
		try {
		    while ((n = is.read(buffer, 0, buffer.length)) != -1) { 
			// we just read. this will throw a SecurityException 
			// if  a signature/digest check fails. 
		    } 
		} finally {
		    is.close();
		} 
	    } 
	
	    // Throws JarException if the JAR has no manifest
	    // which means the JAR isn't signed.
	    Manifest man = jf.getManifest();
	    if (man == null)
		throw new JarException(jarURL.toString() + " is not signed.");
	    
	    // Make sure every class file in the JAR is signed properly:
	    // We must check whether the signer's certificate
	    // can be traced back to a trusted CA
	    // Once we've verified that a signer's cert can be
	    // traced back to a trusted CA, the signer's cert
	    // is kept in the cache 'verifiedSignerCache'.
	    Enumeration e = jf.entries();
	    while (e.hasMoreElements()) {
		JarEntry je = (JarEntry) e.nextElement();
	    
		if (je.isDirectory())
		    continue;
		
		// Every file must be signed except files under META-INF.
		Certificate[] certs = je.getCertificates();
		if ((certs == null) || (certs.length == 0)) {
		    if (!je.getName().startsWith("META-INF"))
			throw new JarException(jarURL.toString() +
					       " has unsigned entries - " 
					       + je.getName() );
		} else {
		    // A JAR file may be signed by multiple signers.
		    // So certs may contain mutiple certificate
		    // chains. Check whether at least one of the signers 
		    // can be trusted.
		    // The signer is trusted iff
		    // 1) its certificate chain can be verified
		    // 2) the last certificate on its chain is one of 
		    //    JCE's trusted CAs
		    //         OR
		    //    the last certificate on its chain is issued
		    //    by one of JCE's trusted CAs.

		    int startIndex = 0;
		    X509Certificate[] certChain;
		    boolean signedAsExpected = false;
		    
		    while ((certChain = getAChain(certs, startIndex)) != null) {
			// Verify that the signer matches trustedSigner
			if (trustedSigner.equals(certChain[0])) {
			    signedAsExpected = true;
			    break;
			}

			// Proceed to the next chain.
			startIndex += certChain.length;
		    }

		    if (!signedAsExpected) {
			throw new JarException(jarURL.toString() +
					       " is not signed by a" +
					       " trusted signer.");
		    }
		}
	    }
	    
	    // Return the value of the attribute CLASS_PATH
	    return man.getMainAttributes().getValue(
		       Attributes.Name.CLASS_PATH);
	} finally {
	    if (jf != null) { 
		jf = null;
	    }
	}
    }

    /*
     * Parse the manifest attribute Class-Path.
     */
    private static String[] parseAttrClasspath(String supportingJars)
        throws JarException
    {
	supportingJars = supportingJars.trim();

	int endIndex = supportingJars.indexOf(' ');
	String name = null;
	Vector values = new Vector();
	boolean done = false;

	do {
	    if (endIndex > 0) {
		name = supportingJars.substring(0, endIndex);
		// Since supportingJars has been trimmed,
		// endIndex + 1 must be less than
		// supportingJars.length().
		supportingJars = supportingJars.substring(endIndex + 1).trim();
		endIndex = supportingJars.indexOf(' ');
	    } else {
		name = supportingJars;
		done = true;
	    }
	    if (name.endsWith(".jar")) {
		values.addElement(name);
	    } else {
		// Cannot verify. Throw a JarException.
		throw new JarException("The provider contains " +
				       "un-verifiable components");
	    }
	} while (!done);

	String[] result = new String[values.size()];
	values.copyInto(result);
	
	return result;    
    } 

    private static X509Certificate[] getAChain(Certificate[] certs,
					int startIndex) {
	int i;

	if (startIndex > certs.length - 1)
	    return null;

	for (i = startIndex; i < certs.length - 1; i++) {
	    if (!((X509Certificate)certs[i + 1]).getSubjectDN().equals(
		  ((X509Certificate)certs[i]).getIssuerDN())) {
		break;
	    }
	}
	int certChainSize = (i-startIndex) + 1;
	X509Certificate[] ret = new X509Certificate[certChainSize];
	for (int j = 0; j < certChainSize; j++ ) {
	    ret[j] = (X509Certificate) certs[startIndex + j];
	}
	return ret;	
    }
    
    static boolean doVerification(final Class cc, final String PROVIDERCERT) {
	X509Certificate providerCert;
	try {
	    CertificateFactory certificateFactory = 
	    	CertificateFactory.getInstance("X.509");
	    byte[] b = PROVIDERCERT.getBytes("UTF8");
	    providerCert = (X509Certificate)certificateFactory.generateCertificate
	    					(new ByteArrayInputStream(b));
	} catch (Exception e) {
	    if (debug) {
		e.printStackTrace();
	    }
	    return false;
	}
	
	URL url = (URL)AccessController.doPrivileged(
					  new PrivilegedAction() {
	    public Object run() {
		CodeSource s1 = cc.getProtectionDomain().getCodeSource();
		return s1.getLocation();
	    }
	});
	if (url == null) {
	   return false;
	}

	JarVerifierImpl jv = new JarVerifierImpl(url);
	try {
	    jv.verify(providerCert);
	} catch (Exception e) {
	    return false;
	}
	
	return true;
    }
    
}
