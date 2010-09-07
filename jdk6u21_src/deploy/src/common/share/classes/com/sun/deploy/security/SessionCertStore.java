/*
 * @(#)SessionCertStore.java	1.30 10/03/24 
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
 * SessionCertStore is a class that represents the session certificate 
 * store which contains all the certificates that are recognized. The
 * certificates store in this certificate store is only valid in the current
 * browser session. It is used in the certification verification process 
 * when signed applet is encountered.
 */
public final class SessionCertStore implements CertStore
{
    // Collection of JPI certificates
    private KeyStore sessionKS = null;

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
	Trace.msgSecurityPrintln("sessioncertstore.cert.loading");

	if (sessionKS == null) {
	   try {
               sessionKS = KeyStore.getInstance("JKS");
	       sessionKS.load(null,new char[0]);
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
	Trace.msgSecurityPrintln("sessioncertstore.cert.adding");

   	// Add one only if it doesn't exist in Session keyStore
	// or it exist but has a Timestamping flag in alias
	String oldAlias = sessionKS.getCertificateAlias(cert);
	boolean certNotExist = true;

	if (oldAlias != null) {
	   try {
	       // If this is a valid certificate (tsFlag is true)
               // or alias didn't contain TSFLAG alias
               // then we don't need to add again.
	       if (tsFlag || (oldAlias.indexOf(TSFLAG) == -1)) {
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

	   // Generate a unique alias name which is not in the session store
           while (true) {
	      // Add TSFLAG if it is a valid certificate
	      if (tsFlag) {
	         alias = "deploymentsessioncert" + TSFLAG + rand.nextLong();
	      }
	      else {
	         alias = "deploymentsessioncert" + rand.nextLong();
	      }
       	      if (sessionKS.getCertificate(alias) == null)
		 break;
           }

	   sessionKS.setCertificateEntry(alias, cert);
	   Trace.msgSecurityPrintln("sessioncertstore.cert.added");
	}

	return true;
    }

    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException
    {
	Trace.msgSecurityPrintln("sessioncertstore.cert.removing");

	String alias = sessionKS.getCertificateAlias(cert);

	if (alias != null)
	   sessionKS.deleteEntry(alias);

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
	Trace.msgSecurityPrintln("sessioncertstore.cert.instore");

	// Certificate alias returned only if there is a match
	String alias = sessionKS.getCertificateAlias(cert);

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
	Trace.msgSecurityPrintln("sessioncertstore.cert.canverify");

	// Session Certificate store is not intended to be used for verification.
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
	Trace.msgSecurityPrintln("sessioncertstore.cert.getcertificates");

	Collection certCollection = new ArrayList();
	Enumeration keyAliases = sessionKS.aliases();

        while (keyAliases.hasMoreElements())
        {
            // Get certificate from store
	    String myAlias = (String) keyAliases.nextElement();
            Certificate cert = sessionKS.getCertificate(myAlias);

            // Add certificate into collection
            certCollection.add(cert);
        }

        return certCollection;
    }
}
