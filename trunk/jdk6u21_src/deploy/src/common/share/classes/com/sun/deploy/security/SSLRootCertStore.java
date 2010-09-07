/*
 * @(#)SSLRootCertStore.java	1.36 10/03/24 
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.AccessController;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.GeneralSecurityException;
import java.util.Collection;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Random;
import java.util.Enumeration;
import java.util.TreeSet;
import java.util.HashSet;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import java.net.PasswordAuthentication;


/**
 * SSLRootCertStore is a class that represents the certificate 
 * stores which contains all the Https root CA certificates. It is used in 
 * the certification verification process when signed applet is encountered.
 */
public final class SSLRootCertStore implements CertStore
{
    private static String _userFilename = null;
    private static String _systemFilename = null;

    private long _userLastModified = 0;
    private long _sysLastModified = 0;

    // Collection of Https root CA cert keystore
    private KeyStore _deploymentUserHttpsCACerts = CertUtils.createEmptyKeyStore();
    private KeyStore _deploymentSystemHttpsCACerts = CertUtils.createEmptyKeyStore();
    
    // Password for keystore
    private char[] keyPassphrase = new char[0];
    private boolean cancelFlag = false;
    private int certStoreType = 0;

    static
    {
	// Get SSL root CA file jssecacerts 
	_userFilename = Config.getUserSSLRootCertificateFile();

	// Load cacerts from old running JRE directory
        if (Config.isJavaVersionAtLeast15()) {
	    _systemFilename = Config.getSystemSSLRootCertificateFile();
        }
        else {
            _systemFilename = Config.getOldSystemSSLRootCertificateFile();
        }

        // Get root CA file cacerts if HTTPS root CA cert file is invalid
	if (_systemFilename == null || new File(_systemFilename).exists() == false)
	{
            if (Config.isJavaVersionAtLeast15()) {
    	        _systemFilename = Config.getSystemRootCertificateFile();
	    }
	    else {
    	        _systemFilename = Config.getOldSystemRootCertificateFile();
	    }
	}
    }

    private SSLRootCertStore(int storeType) {
        certStoreType = storeType;
    }

    public static CertStore getCertStore() {
	return new ImmutableCertStore(new SSLRootCertStore(CertStore.ALL));
    }

    public static CertStore getUserCertStore() {
	return new SSLRootCertStore(CertStore.USER);
    }

    public static CertStore getSystemCertStore() {
	return new ImmutableCertStore(new SSLRootCertStore(CertStore.SYSTEM));
    }

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
	load(false);
    }

    public void load(boolean integrityCheck) throws 
				IOException, CertificateException,
                       		KeyStoreException, NoSuchAlgorithmException
    {
	long lastModified;

        if ((certStoreType & CertStore.USER) == CertStore.USER) {
	   if (_userFilename != null) {
	      // lastModified will return 0 if file not exist, so it
	      // won't be loaded
	      lastModified = CertUtils.getFileLastModified(_userFilename);
	      if (lastModified != _userLastModified) {	
		 _deploymentUserHttpsCACerts = loadCertStore(_userFilename, integrityCheck);
		 _userLastModified = lastModified;
	      } 
	   }
	}

        if ((certStoreType & CertStore.SYSTEM) == CertStore.SYSTEM) {
	   if (_systemFilename != null) {
	      lastModified = CertUtils.getFileLastModified(_systemFilename);
	      if (lastModified != _sysLastModified) {		
		 _deploymentSystemHttpsCACerts = loadCertStore(_systemFilename, integrityCheck);
		 _sysLastModified = lastModified;
	      }
	   }
	}
    }

    private KeyStore loadCertStore(final String filename, final boolean integrityCheck) throws 
				IOException, CertificateException,
				KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.loading", new Object[] {filename});

	final File file = new File(filename);
	final KeyStore keyStore = KeyStore.getInstance("JKS");
        keyStore.load(null, null);

	try
	{     
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
    
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    // Only load the jssecacerts if it exits.
		    if (file.exists())
		    {
		    	FileInputStream fis = new FileInputStream(file);		    
		    	BufferedInputStream bis = new BufferedInputStream(fis);

			// Initialize the keystore with/without no password
                        if (integrityCheck) {
                           cancelFlag = false;
                           keyStore.load(bis, new char[0]);
                        }
                        else {
                           keyStore.load(bis, null);
                        }

		    	bis.close();
		    	fis.close();
		    }
		    else
		    {
			Trace.msgSecurityPrintln("roothttpscertstore.cert.noload", new Object[] {filename});
		    }

		    return null;
		}
	    });
	}
	catch (PrivilegedActionException e)
	{
	    Exception ex = e.getException();

	    if (ex instanceof IOException) {
		if (integrityCheck) {
                   FileInputStream fis = new FileInputStream(file);
                   BufferedInputStream bis = new BufferedInputStream(fis);

                   
                   CredentialInfo passwordInfo = 
                           UIFactory.showPasswordDialog(null,
                           ResourceManager.getMessage("password.dialog.title"),
                           ResourceManager.getMessage(
                           "roothttpscertstore.password.dialog.text"),
                           false, false, null, false);

                   // User didn't hit cancel button
                   if ( passwordInfo != null ) {
                      cancelFlag = false;
                      // Get modified password for trusted certificate store
                      keyPassphrase = passwordInfo.getPassword();
                      keyStore.load(bis, keyPassphrase);
                   }
                   else {
                      cancelFlag = true;
                   }

                   bis.close();
                   fis.close();
                }
                else {
                  throw (IOException)ex;
                }
            }
	    else if (ex instanceof CertificateException)
		throw (CertificateException)ex;
	    else if (ex instanceof KeyStoreException)
		throw (KeyStoreException)ex;
	    else if (ex instanceof NoSuchAlgorithmException)
		throw (NoSuchAlgorithmException)ex;
	    else
		Trace.securityPrintException(e);
	}
	Trace.msgSecurityPrintln("roothttpscertstore.cert.loaded", new Object[] {filename});
	return keyStore;
    }

 
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.saving", new Object[] {_userFilename});

	try
        {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException, CertificateException,
                                           KeyStoreException, NoSuchAlgorithmException
                {
		    File file = new File(_userFilename);
                    file.getParentFile().mkdirs();
                    FileOutputStream fos = new FileOutputStream(file);
                    BufferedOutputStream bos = new BufferedOutputStream(fos);

                     _deploymentUserHttpsCACerts.store(bos, keyPassphrase);
                    bos.close();
                    fos.close();
                    return null;
                }
             });
        }
        catch (PrivilegedActionException e)
        {
            Exception ex = e.getException();

            if (ex instanceof IOException)
                throw (IOException)ex;
            else if (ex instanceof CertificateException)
                throw (CertificateException)ex;
            else if (ex instanceof KeyStoreException)
                throw (KeyStoreException)ex;
            else if (ex instanceof NoSuchAlgorithmException)
                throw (NoSuchAlgorithmException)ex;
            else
		Trace.securityPrintException(e);
        }
               
        Trace.msgSecurityPrintln("roothttpscertstore.cert.saved", new Object[] {_userFilename});
    }


    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean add(Certificate cert) throws KeyStoreException 
    {
	return add(cert, false);
    }

    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     * @param tsFlag true if certificate is valid.
     */
    public boolean add(Certificate cert, boolean tsFlag) throws KeyStoreException 
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.adding");

	if (cancelFlag) {
           return false;
        }

	// Add one only if it doesn't exist in User keyStore
        String newAlias = _deploymentUserHttpsCACerts.getCertificateAlias(cert);
        if (newAlias == null)
        {
            // Generate a unique alias for the certificate
            Random rand = new Random();
            boolean found = false;
            String alias = null;

            // Loop until we found a unique alias that is not in the store
            do
            {
                alias = "userhttpscacert" + rand.nextLong();
                Certificate c = _deploymentUserHttpsCACerts.getCertificate(alias);
                if (c == null)
                    found = true;
            }
            while (found == false);

            _deploymentUserHttpsCACerts.setCertificateEntry(alias, cert);

            Trace.msgSecurityPrintln("roothttpscertstore.cert.added", new Object[]{alias});
        }

	return true;
    }

    /**
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException,  KeyStoreException 
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.removing");

	if (cancelFlag) {
           return false;
        }

	String alias = _deploymentUserHttpsCACerts.getCertificateAlias(cert);

        if (alias != null)
            _deploymentUserHttpsCACerts.deleteEntry(alias);

        Trace.msgSecurityPrintln("rrothttpscertstore.cert.removed", new Object[] {alias});
	return true;
    }

    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException 
    {
	return contains(cert, false);
    }

    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @param tsFlag true if only valid certificate is checked.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert, boolean tsFlag) throws KeyStoreException 
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.instore");

	// Certificate alias returned only if there is a match
        String alias = null;

	alias = _deploymentSystemHttpsCACerts.getCertificateAlias(cert);
	
	if (alias != null) // in system cert store
	    return true;

	alias = _deploymentUserHttpsCACerts.getCertificateAlias(cert);
        return (alias != null);
    }


    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store. 
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */ 
    public boolean verify(Certificate cert) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.canverify");
	Trace.msgSecurityPrintln("roothttpscertstore.cert.tobeverified", new Object[] {cert});
 
	StringBuffer sb = new StringBuffer();

	// Enumerate each root CA certificate in the root store
	Enumeration enumSystem = _deploymentSystemHttpsCACerts.aliases();
	Enumeration enumUser = _deploymentUserHttpsCACerts.aliases();

        while (enumSystem.hasMoreElements() || enumUser.hasMoreElements())
        {
            String alias;
            Certificate rootCert;

            if (enumSystem.hasMoreElements())
            {
               alias = (String) enumSystem.nextElement();
               rootCert = _deploymentSystemHttpsCACerts.getCertificate(alias);
            }
            else
            {	
		alias = (String) enumUser.nextElement();
		rootCert = _deploymentUserHttpsCACerts.getCertificate(alias);
            }

            Trace.msgSecurityPrintln("roothttpscertstore.cert.tobecompared", new Object[] {rootCert});

	    try
            {
                cert.verify(rootCert.getPublicKey());
                Trace.msgSecurityPrintln("roothttpscertstore.cert.verify.ok");
                return true;
            }
            catch (GeneralSecurityException e)
            {
                // Ignore exception
            }
        }

	Trace.msgSecurityPrintln("roothttpscertstore.cert.verify.fail");

	return false;
    }


    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return collection for certificates
     */
    public Collection getCertificates() throws KeyStoreException
    {
        HashSet sslRootCerts = new HashSet();

        if ((certStoreType & CertStore.USER) == CertStore.USER) {
           sslRootCerts.addAll(getCertificates(CertStore.USER));
	}

        if ((certStoreType & CertStore.SYSTEM) == CertStore.SYSTEM) {
           sslRootCerts.addAll(getCertificates(CertStore.SYSTEM));
	}

        return sslRootCerts;
    }

    private Collection getCertificates(int myCertStoreType) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.getcertificates");

        Collection certCollection = new ArrayList();
	KeyStore ks = null; 

	if (myCertStoreType == CertStore.USER) {
	   ks = _deploymentUserHttpsCACerts;
	}
	else {
	   ks = _deploymentSystemHttpsCACerts;
	}
        Enumeration keyAliases = ks.aliases();

	// Construct a TreeSet object to sort the certificate list
	TreeSet tsCerts = new TreeSet();

        while (keyAliases.hasMoreElements())
        {
	    // Get certificate alias from iterator
	    String alias = (String) keyAliases.nextElement();
	    tsCerts.add(alias);
	}

	Iterator itrCerts = tsCerts.iterator();
	while (itrCerts.hasNext())
	{
            // Get certificate from store
            String sortAlias = (String) itrCerts.next();
            Certificate cert = ks.getCertificate(sortAlias);

            // Add certificate into collection
            certCollection.add(cert);
        }              

	return certCollection;
    }
}
