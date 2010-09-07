/*
 * @(#)DeploySigningCertStore.java	1.45 05/04/19 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
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
import java.util.Collection;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;
import java.util.TreeSet;
import java.util.HashSet;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import java.net.PasswordAuthentication;


/**
 * DeploySigningCertStore is a class that represents the permanent certificate 
 * store which contains all the certificates that are recognized. It 
 * is used in the certification verification process when signed jar is 
 * encountered.
 */
public final class DeploySigningCertStore implements CertStore
{
    private static String _userFilename = null;
    private static String _systemFilename = null;

    private long _userLastModified = 0;
    private long _sysLastModified = 0;

    // Collection of deployment certificates
    private KeyStore _deploymentUserCerts = CertUtils.createEmptyKeyStore();
    private KeyStore _deploymentSystemCerts = CertUtils.createEmptyKeyStore();

    // Password for keystore
    private char[] keyPassphrase = new char[0];
    private boolean cancelFlag = false;
    private int certStoreType = 0;

    static
    {
	// Get deployment certificate filenames
	_userFilename = Config.getUserTrustedCertificateFile(); 
	_systemFilename = Config.getSystemTrustedCertificateFile();
    }

    private DeploySigningCertStore(int storeType) {
	certStoreType = storeType;
    }

    public static CertStore getCertStore() {
	return new ImmutableCertStore(new DeploySigningCertStore(CertStore.ALL));
    }

    public static CertStore getUserCertStore() {
	return new DeploySigningCertStore(CertStore.USER);
    }

    public static CertStore getSystemCertStore() {
	return new ImmutableCertStore(new DeploySigningCertStore(CertStore.SYSTEM));
    }

    /**
     * Load the certificate store into memory without integrity check.
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
		 _deploymentUserCerts = loadCertStore(_userFilename, integrityCheck);
		 _userLastModified = lastModified;
	      } 
	   }
	}	

	if ((certStoreType & CertStore.SYSTEM) == CertStore.SYSTEM) {
	   if (_systemFilename != null) {
	      lastModified = CertUtils.getFileLastModified(_systemFilename);
	      if (lastModified != _sysLastModified) {	
		 _deploymentSystemCerts = loadCertStore(_systemFilename, integrityCheck);
		 _sysLastModified = lastModified;
	      } 
	   }
	}
    }

    private KeyStore loadCertStore(final String filename, final boolean integrityCheck) 
			throws IOException, CertificateException, 
			KeyStoreException, NoSuchAlgorithmException 
    {
	Trace.msgSecurityPrintln("deploycertstore.cert.loading", 
				  new Object[] {filename});

	final File file = new File(filename);
        final KeyStore keyStore = KeyStore.getInstance("JKS");
        keyStore.load(null, null);

	try
	{ 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			
		public Object run() throws IOException, CertificateException,
				   KeyStoreException, NoSuchAlgorithmException
		{
		    // Only load the cert store if it exists
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
                           "deploycertstore.password.dialog.text"), 
                           false, false, null, false);

		   // User didn't hit cancel button
		   if ( passwordInfo != null) {
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

	Trace.msgSecurityPrintln("deploycertstore.cert.loaded", 
				  new Object[]{filename});
	return keyStore;
    }

    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("deploycertstore.cert.saving", 
				  new Object[]{_userFilename});
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

	    	    _deploymentUserCerts.store(bos, keyPassphrase);

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

	Trace.msgSecurityPrintln("deploycertstore.cert.saved", 
				  new Object[] {_userFilename});
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
	Trace.msgSecurityPrintln("deploycertstore.cert.adding");

	if (cancelFlag) {
	   return false;
	}

	// Add one only if it doesn't exist in User keyStore
	// or it exist but has a Timestamping flag in alias
        String oldAlias = _deploymentUserCerts.getCertificateAlias(cert);
	boolean certNotExist = true;

	if (oldAlias != null) {
	   try {
	       // If we found a match in trusted cert store,
	       // We probably don't need to add to cert store if
	       // 1. We encounter a valid certificate. Or
	       // 2. No TSFLAG in alias name for the matched cert
	       if ( tsFlag || (oldAlias.indexOf(TSFLAG) == -1)) {
	          certNotExist = false;
	       }
	       else {
		  // If this is an expired certificate and 
	    	  // we found a Timestamping flag in alias name,
	    	  // we have to remove this certificate first
		  remove(cert);
	       }
	   }
	   catch (IOException ioe)
	   { Trace.securityPrintException(ioe); }
	}

        if (certNotExist)
	{
    	    // Generate a unique alias for the certificate
	    Random rand = new Random();
	    String alias = null;

	    // Generate a unique alias name which is not in the trusted store
	    while (true) {
	       if (tsFlag) {
		  alias = "deploymentusercert" + TSFLAG + rand.nextLong();
	       }
	       else {
	     	  alias = "deploymentusercert" + rand.nextLong();
	       }
    	       if (_deploymentUserCerts.getCertificate(alias) == null)
		  break;
	    }

	    _deploymentUserCerts.setCertificateEntry(alias, cert);
	    Trace.msgSecurityPrintln("deploycertstore.cert.added", 
				      new Object[]{ alias});
	}

	return true;
    }

    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException,  KeyStoreException 
    {
	if (cancelFlag) {
	   return false;
	}

	Trace.msgSecurityPrintln("deploycertstore.cert.removing");

	String alias = _deploymentUserCerts.getCertificateAlias(cert);

	if (alias != null)
	    _deploymentUserCerts.deleteEntry(alias);

	Trace.msgSecurityPrintln("deploycertstore.cert.removed", new Object[] {alias});

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
	Trace.msgSecurityPrintln("deploycertstore.cert.instore");

	// Certificate alias returned only if there is a match
	String alias = _deploymentUserCerts.getCertificateAlias(cert);

	if (alias != null && ( !tsFlag || alias.indexOf(TSFLAG) > -1)) {
	   return true;
	}

	alias = _deploymentSystemCerts.getCertificateAlias(cert);
	return (alias != null && (!tsFlag || alias.indexOf(TSFLAG) > -1));
    }

    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */
    public boolean verify(Certificate cert)
    {
        Trace.msgSecurityPrintln("deploycertstore.cert.canverify");

	// Deployn Cert store is not intended to be used for verification.

	return false;
    }

    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return Collection for certificates
     */
    public Collection getCertificates() throws KeyStoreException
    {
        HashSet deploySigningCerts = new HashSet();

	if ((certStoreType & CertStore.USER)  == CertStore.USER) {
           deploySigningCerts.addAll(getCertificates(CertStore.USER));
        }

	if ((certStoreType & CertStore.SYSTEM) == CertStore.SYSTEM) {
           deploySigningCerts.addAll(getCertificates(CertStore.SYSTEM));
        }

        return deploySigningCerts;
    }

    private Collection getCertificates(int myCertStoreType) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("deploycertstore.cert.getcertificates");

	Collection certCollection = new ArrayList();
	KeyStore ks = null;

        if (myCertStoreType == CertStore.USER) {
           ks = _deploymentUserCerts;
        }
        else {
           ks = _deploymentSystemCerts;
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
