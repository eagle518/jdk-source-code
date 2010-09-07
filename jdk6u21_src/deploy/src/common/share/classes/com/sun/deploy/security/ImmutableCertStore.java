/*
* @(#)ImmutableCertStore.java	1.4 10/03/24 
*
* Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
* ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
*/

package com.sun.deploy.security;

import java.io.IOException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collection;

/**
 * ImmutableCertStore is a wrapper class that represents a certificate store 
 * which cannot be modified.
 *
 * @author Stanley Man-Kit Ho
 */
final class ImmutableCertStore implements CertStore
{
    // Cert store to delegate
    private CertStore certStore = null;
    
    ImmutableCertStore(CertStore certStore) 
    {
        this.certStore = certStore;
    }
 
    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException    
    {
	// Delegate to actual cert store
        certStore.load();
    }
 
    public void load(boolean integrityCheck) throws 
		                IOException, CertificateException,
                                KeyStoreException, NoSuchAlgorithmException
    {
	// Delegate to actual cert store
	certStore.load(integrityCheck);
    }
   
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
	throw new IOException("Cannot modify certificate store.");
    }
 
    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean add(Certificate cert) throws KeyStoreException 
    {
	throw new KeyStoreException("Cannot modify certificate store.");
    }
 
    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     * @param tsFlag true if certificate is valid.
     */
    public boolean add(Certificate cert, boolean tsFlag) throws KeyStoreException 
    {
	throw new KeyStoreException("Cannot modify certificate store.");
    }
 
    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException 
    {     
	throw new IOException("Cannot modify certificate store.");
    }
 
    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException 
    {
	// Delegate to actual cert store
	return certStore.contains(cert);
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
	// Delegate to actual cert store
	return certStore.contains(cert, tsFlag);
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
	// Delegate to actual cert store
	return certStore.verify(cert);
    }
 
    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return collection for certificates
     */
    public Collection getCertificates() throws KeyStoreException
    {
	// Delegate to actual cert store
	return certStore.getCertificates();
    }
}
