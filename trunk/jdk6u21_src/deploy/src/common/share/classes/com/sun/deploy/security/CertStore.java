/*
 * @(#)CertStore.java	1.23 10/03/24 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security; 

import java.io.IOException;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collection;


/**
 * CertStore is an interface that represents the certificate 
 * stores that are used in the certification verification process 
 * when signed applet is encountered.
 */
public interface CertStore
{

    final static int USER = 1;
    final static int SYSTEM = 2;
    final static int ALL = USER | SYSTEM;

    // Hard code string in alias for Timestamping
    final static String TSFLAG = "$tsflag";

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException;

    /**
     * Load the certificate store into memory.
     *
     * @param integrityCheck true if keystore need password to load.
     */
    public void load(boolean integrityCheck) throws IOException, CertificateException, 
				KeyStoreException, NoSuchAlgorithmException;

    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException;

    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean add(Certificate cert) throws KeyStoreException;

    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     * @param tsFlag true if certificate is valid.
     */
    public boolean add(Certificate cert, boolean tsFlag) throws KeyStoreException;

    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException;

    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException;

    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @param tsFlag true if only valid certificate is checked.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert, boolean tsFlag) throws KeyStoreException;

    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */
    public boolean verify(Certificate cert) throws KeyStoreException; 

    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return Collection for certificates
     */
    public Collection getCertificates() throws KeyStoreException;
}
