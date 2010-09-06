/*
 * @(#)SSLRootCertStore.java	1.26 04/02/25 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import java.util.LinkedHashSet;
import java.util.Random;
import java.util.Enumeration;
import java.util.TreeSet;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.Trace;


/**
 * SSLRootCertStore is a class that represents the certificate 
 * stores which contains all the Https root CA certificates. It is used in 
 * the certification verification process when signed applet is encountered.
 */
public final class SSLRootCertStore implements CertStore
{
    private static String _filenameUser = null;
    private static String _filenameSys = null;

    private long _userLastModified = 0;
    private long _sysLastModified = 0;

    // Collection of Https root CA cert keystore
    private KeyStore _deploymentUserHttpsCACerts = CertUtils.createEmptyKeyStore();
    private KeyStore _deploymentSystemHttpsCACerts = CertUtils.createEmptyKeyStore();
    
    static
    {
	// Get SSL root CA file jssecacerts 
	_filenameUser = Config.getUserSSLRootCertificateFile();
        _filenameSys = Config.getSystemSSLRootCertificateFile();

        // Get root CA file cacerts if HTTPS root CA cert file is invalid
	if (_filenameSys == null || new File(_filenameSys).exists() == false)
	{
    	    _filenameSys = Config.getSystemRootCertificateFile();
	}
    }

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
      
	long lastModified;
	if (_filenameUser != null) {
	    // lastModified will return 0 if file not exist, so it
	    // won't be loaded
	    lastModified = CertUtils.getFileLastModified(_filenameUser);
	    if (lastModified != _userLastModified) {	
		_deploymentUserHttpsCACerts = load(_filenameUser);
		_userLastModified = lastModified;
	    } 
	}

	if (_filenameSys != null) {
	    lastModified = CertUtils.getFileLastModified(_filenameSys);
	    if (lastModified != _sysLastModified) {		
		_deploymentSystemHttpsCACerts = load(_filenameSys);
		_sysLastModified = lastModified;
	    }
	}
    }

    private KeyStore load(final String filename) throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.loading", new Object[] {filename});

	final KeyStore keyStore = KeyStore.getInstance("JKS");
        keyStore.load(null, null);

	try
	{     
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
    
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    File file = new File(filename);

		    // Only load the jssecacerts if it exits.
		    if (file.exists())
		    {
		    	FileInputStream fis = new FileInputStream(file);		    
		    	BufferedInputStream bis = new BufferedInputStream(fis);

		    	// Initialize the keystore
		   	keyStore.load(bis, null);

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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.loaded", new Object[] {filename});
	return keyStore;
    }

 
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.saving", new Object[] {_filenameUser});

	try
        {
            AccessController.doPrivileged(new PrivilegedExceptionAction() {
                public Object run() throws IOException, CertificateException,
                                           KeyStoreException, NoSuchAlgorithmException
                {
		    File file = new File(_filenameUser);
                    file.getParentFile().mkdirs();
                    FileOutputStream fos = new FileOutputStream(file);
                    BufferedOutputStream bos = new BufferedOutputStream(fos);

                     _deploymentUserHttpsCACerts.store(bos, new char[0]);
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
               
        Trace.msgSecurityPrintln("roothttpscertstore.cert.saved", new Object[] {_filenameUser});
    }


    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException 
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.adding");

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
    }

    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException,  KeyStoreException 
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.removing");

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
     * @return Iterator for iterating certificates
     */
    public Iterator iterator(int storeType) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.iterator");

        LinkedHashSet certCollection = new LinkedHashSet();
	KeyStore ks = getKeyStore(storeType);
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

        return certCollection.iterator();
    }

    /**
     * Obtain KeyStore Object
     */
    private KeyStore getKeyStore(int storeType)
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.getkeystore");
	return (storeType == CertStore.USER) ? _deploymentUserHttpsCACerts : _deploymentSystemHttpsCACerts;
    }
}
