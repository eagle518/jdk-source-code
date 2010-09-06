/*
 * @(#)SessionCertStore.java	1.23 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import com.sun.deploy.util.Trace;


/**
 * SessionCertStore is a class that represents the session certificate 
 * store which contains all the certificates that are recognized. The
 * certificates store in this certificate store is only valid in the current
 * browser session. It is used in the certification verification process 
 * when signed applet is encountered.
 */
public final class SessionCertStore implements CertStore
{
    // Collection of JPI certificates
    private Collection _certs = new HashSet();

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("sessioncertstore.cert.loading");

	// Do nothing

	Trace.msgSecurityPrintln("sessioncertstore.cert.loaded");
    }


    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("sessioncertstore.cert.saving");

	// Do nothing

	Trace.msgSecurityPrintln("sessioncertstore.cert.saved");
    }

 
    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("sessioncertstore.cert.adding");

	_certs.add(cert);

	Trace.msgSecurityPrintln("sessioncertstore.cert.added");
    }

    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException
    {
	Trace.msgSecurityPrintln("sessioncertstore.cert.removing");

	_certs.remove(cert);

	Trace.msgSecurityPrintln("sessioncertstore.cert.removed");
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
	Trace.msgSecurityPrintln("sessioncertstore.cert.instore");

	return _certs.contains(cert);
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
	Trace.msgSecurityPrintln("sessioncertstore.cert.canverify");

	// Session Certificate store is not intended to be used for verification.
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
	Trace.msgSecurityPrintln("sessioncertstore.cert.iterator");

	return _certs.iterator();
    }

    /**
    * Obtain KeyStore Object
    */
    private KeyStore getKeyStore(int storeType)
    {
	Trace.msgSecurityPrintln("sessioncertstore.cert.getkeystore");
	return null;
    }
}
