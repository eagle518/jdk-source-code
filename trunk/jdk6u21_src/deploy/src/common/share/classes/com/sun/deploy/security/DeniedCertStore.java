/*
 * @(#)DeniedCertStore.java	1.23 10/03/24 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.util.Collection;
import java.util.HashSet;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;
import java.util.Enumeration;
import java.security.Key;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import com.sun.deploy.util.Trace;


/**
 * DeniedCertStore is a class that represents the certificate 
 * store which contains all the certificates that have been denied. The
 * certificates store in this certificate store is only valid in the current
 * browser session. It is used in the certification verification process 
 * when signed applet or HTTPS is encountered.
 */
final class DeniedCertStore implements CertStore
{
    // Collection of JPI certificates
    private KeyStore deniedKS = null;

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
	if (deniedKS == null) {
           try {
               deniedKS = KeyStore.getInstance("JKS");
               deniedKS.load(null,new char[0]);
           }
           catch (IOException ioe)
           { Trace.msgSecurityPrintln(ioe.getMessage()); }
           catch (KeyStoreException kse)
           { Trace.msgSecurityPrintln(kse.getMessage()); }
           catch (NoSuchAlgorithmException nsae)
           { Trace.msgSecurityPrintln(nsae.getMessage()); }
           catch (CertificateException ce)
           { Trace.msgSecurityPrintln(ce.getMessage()); }
	}
    }

    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	// Do nothing
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
	Trace.msgSecurityPrintln("deniedcertstore.cert.adding");

	// Add one only if it doesn't exist in Denied keyStore
        // or it exist but doesn't has a Timestamping flag in alias
        String oldAlias = deniedKS.getCertificateAlias(cert);
        boolean certNotExist = true;

        if (oldAlias != null) {
	   try {
	       // If this is an expired certificate (tsFlag is false)
               // or alias did contain TSFLAG alias
               // then we don't need to add again.
               if (!tsFlag || (oldAlias.indexOf(TSFLAG) > -1)) {
                  certNotExist = false;
	       }
	       else {
		  // If this is a valid certificate and 
		  // we didn't find a Timestamping flag in alias name,
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

           // Generate a unique alias name which is not in the deny store
	   while (true) {
	      // Add TSFLAG if it is a valid certificate
	      if (tsFlag) {
                 alias = "deploymentdeniedcert" + TSFLAG + rand.nextLong();
	      }
	      else {
                 alias = "deploymentdeniedcert" + rand.nextLong();
	      }
              if (deniedKS.getCertificate(alias) == null)
                 break;
           }

           deniedKS.setCertificateEntry(alias, cert);
           Trace.msgSecurityPrintln("deniedcertstore.cert.added");
        }

	return true;
    }

    /**
     * Remove a certificate from the denied certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException
    {
	Trace.msgSecurityPrintln("deniedcertstore.cert.removing");
	String alias = deniedKS.getCertificateAlias(cert);

        if (alias != null)
           deniedKS.deleteEntry(alias);

	Trace.msgSecurityPrintln("deniedcertstore.cert.removed");
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
	Trace.msgSecurityPrintln("deniedcertstore.cert.instore");

        // Certificate alias returned only if there is a match
        String alias = deniedKS.getCertificateAlias(cert);

	if (tsFlag) {
           return (alias != null && (alias.indexOf(TSFLAG) > -1));
	}
	else {
           return (alias != null);
	}
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
	// Denied Certificate store is not intended to be used for verification.
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
	Trace.msgSecurityPrintln("deniedcertstore.cert.getcertificates");

	Collection certCollection = new ArrayList();
        Enumeration keyAliases = deniedKS.aliases();

        while (keyAliases.hasMoreElements())
        {
            // Get certificate from store
            String myAlias = (String) keyAliases.nextElement();
            Certificate cert = deniedKS.getCertificate(myAlias);

            // Add certificate into collection
            certCollection.add(cert);
        }

        return certCollection;
    }
}
