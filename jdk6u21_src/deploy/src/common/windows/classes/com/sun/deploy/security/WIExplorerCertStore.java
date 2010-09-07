/*
 * @(#)WIExplorerCertStore.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.GeneralSecurityException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateFactory;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;


/**
 * WIExplorerCertStore is a class that represents the certificate store in
 * Internet Explorer. 
 */
abstract class WIExplorerCertStore implements CertStore
{
    protected final static String OID_EKU_CODE_SIGNING = "1.3.6.1.5.5.7.3.3";
    protected final static String OID_EKU_SERVER_AUTH =  "1.3.6.1.5.5.7.3.1";
    protected final static String OID_EKU_CLIENT_AUTH =  "1.3.6.1.5.5.7.3.2";

    private Collection certs = new ArrayList();

    /**
     * Construct an WIExplorerCertStore object.
     */
    WIExplorerCertStore()
    {
    }

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
             			KeyStoreException, NoSuchAlgorithmException
    {
	load(false);
    }

    /**
     * Load the certificate store into memory.
     */
    public void load(boolean integrityCheck) throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("iexplorer.cert.loading", new Object[]{getName()});

	// Clear all certificates
	certs.clear();

	// Load certificates from browsers
	loadCertificates(getName(), getExtendedKeyUsageFilters(), certs);

	Trace.msgSecurityPrintln("iexplorer.cert.loaded", new Object[]{getName()});
    }    

    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	throw new KeyStoreException("Cannot store certificate in Internet Explorer \"" + getName() + "\" certificate store.");
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
	throw new KeyStoreException("Cannot add certificate in Internet Explorer \"" + getName() + "\" certificate store.");
    }

    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException 
    {
	throw new KeyStoreException("Cannot remove certificate from Internet Explorer \"" + getName() + "\" certificate store.");
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
	Trace.msgSecurityPrintln("iexplorer.cert.instore", new Object[] {getName()});

        return certs.contains(cert);
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
	Trace.msgSecurityPrintln("iexplorer.cert.canverify", new Object[] {getName()});
	Trace.msgSecurityPrintln("iexplorer.cert.tobeverified", new Object[] {cert});
	
	for (Iterator iter = getCertificates().iterator(); iter.hasNext(); )
	{
	    X509Certificate rootCert = (X509Certificate) iter.next();

	    Trace.msgSecurityPrintln("iexplorer.cert.tobecompared", new Object[] {getName(), rootCert});

	    try
	    {
    		cert.verify(rootCert.getPublicKey());

		Trace.msgSecurityPrintln("iexplorer.cert.verify.ok", new Object[] {getName()});
		return true;
	    }
	    catch (GeneralSecurityException e)
	    {
		// Ignore exception		
	    }
	}

	Trace.msgSecurityPrintln("iexplorer.cert.verify.fail", new Object[] {getName()});

	return false;
    }


    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return Collection for certificates
     */
    public Collection getCertificates() 
    {
	Collection certCollection = new ArrayList();

	Iterator itrCerts = certs.iterator();
	while (itrCerts.hasNext())
	{
   	    // Get certificate from store
            Certificate cert = (Certificate) itrCerts.next();

	    // Add certificate into collection
            certCollection.add(cert);
	}

	return certCollection;
    }

    /**
     * Load certificates from IE cert store into Collection.
     *
     * @param name Name of cert store.
     * @param extendedKeyUsageOIDFilter EKU OID filters
     * @param certCollection Collection of certificates.
     */
    private native void loadCertificates(String name, String[] extendedKeyUsageOIDFilters, Collection certCollection);

    /**
     * Generates certificates from byte data and stores into cert collection.
     *
     * @param data Byte data.
     * @param certCollection Collection of certificates.
     */
    private void generateCertificate(byte[] data, Collection certCollection) 
    {
	try
	{
	    ByteArrayInputStream bis = new ByteArrayInputStream(data);

	    // Obtain certificate factory
	    CertificateFactory cf = CertificateFactory.getInstance("X.509");

	    // Generate certificate
	    Collection c = cf.generateCertificates(bis);
	    Iterator i = c.iterator();
	    while (i.hasNext()) 
	    {
		X509Certificate cert = (X509Certificate)i.next();

		certCollection.add(cert);
	    }
	}
	catch (CertificateException e)
	{
	    e.printStackTrace();
	}
	catch (Throwable te)
	{
	    te.printStackTrace();
	}
    }

    /**
     * Return name of the Internet Explorer cert store.
     */
    protected abstract String getName();
 
    /**
     *  Return OID filters for extended key usage.
     */
    protected abstract String[] getExtendedKeyUsageFilters();
}
