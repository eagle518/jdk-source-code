/*
 * @(#)MozillaCertStore.java	1.1 04/02/04
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
import java.lang.reflect.*;
import com.sun.deploy.config.Config;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.Trace;

/**
 * MozillaCertStore is a class that represents the certificate store in
 * Mozilla. 
 */
abstract class MozillaCertStore implements CertStore
{
    ////////////////////////////////////////////////////
    // Trust manipulation
    ////////////////////////////////////////////////////
    protected static final int VALID_PEER          = (1<<0);
    protected static final int TRUSTED_PEER        = (1<<1); // CERTDB_TRUSTED
    protected static final int VALID_CA            = (1<<3);
    protected static final int TRUSTED_CA          = (1<<4);
    protected static final int USER                = (1<<6);
    protected static final int TRUSTED_CLIENT_CA   = (1<<7);
    
    private Collection certs = new ArrayList();
    
    /**
     * Construct an MozillaCertStore object.
     */
    MozillaCertStore()
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

    public void load(boolean integrityCheck) throws IOException, CertificateException,
                              KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("mozilla.cert.loading", new Object[]{getName()});

	// Clear all certificates
	certs.clear();

	try
	{
	    // Load certificates from browsers
	    //CryptoManager cm = (org.mozilla.jss.CryptoManager) BrowserKeystore.getJSSCryptoManager();	    
	    //CryptoManager cm = (org.mozilla.jss.CryptoManager) BrowserKeystore.getJSSCryptoManager();	    
	    //org.mozilla.jss.crypto.X509Certificate[] certsArray = cm.getCACerts();
	    Class jsscm = Class.forName("org.mozilla.jss.CryptoManager", true,
					 ClassLoader.getSystemClassLoader());
	    Object cm = BrowserKeystore.getJSSCryptoManager();	    
	    Method gccsmeth = jsscm.getMethod("getCACerts", null);
	    Object[] certsArray = (Object[]) gccsmeth.invoke(cm, null);

	    for(int i=0; i < certsArray.length; i++ ) 
	    {
		// Get cert nick name from mozilla db using reflection
	    	Class jsscertarr = Class.forName("org.mozilla.jss.crypto.X509Certificate", true,
						  ClassLoader.getSystemClassLoader());
	    	Method getNickNameMeth = jsscertarr.getMethod("getNickname", null);
	    	String nickname = (String) getNickNameMeth.invoke(certsArray[i], null);

		// Get isCertValid method using reflection
	    	Class jsscertusage = Class.forName("org.mozilla.jss.CryptoManager$CertUsage", true,
						    ClassLoader.getSystemClassLoader());
		Class partypes2[] = new Class[] {String.class, Boolean.TYPE, jsscertusage};
		Method isCertValidMeth = jsscm.getMethod("isCertValid", partypes2);

		// Filter out invalid CA cert for signing
                if (isTrustedSigningCACertStore())
                {
                    // The cert usage for ObjectSigning in Mozilla is a mess. Most of the root CA cert
                    // for ObjectSigning don't have the ObjectSigning EKU, so we include EmailSigner
                    // as well for signing verification.
                    //

		    // For ObjectSigner
                    //if (cm.isCertValid(nickname, false, ObjectSigner) == false
                    //    && cm.isCertValid(nickname, false, EmailSigner) == false)
		    Field osfield = jsscertusage.getField("ObjectSigner");

		    Object arglist2[] = new Object[3];
		    arglist2[0] = nickname;
		    arglist2[1] = Boolean.FALSE;
		    arglist2[2] = osfield.get(certsArray[i]);
	  	    Boolean objSigner = (Boolean) isCertValidMeth.invoke(cm, arglist2);

		    // For EmailSigner 
		    Field esfield = jsscertusage.getField("EmailSigner");
		    arglist2[2] = esfield.get(certsArray[i]);
	  	    Boolean emailSigner = (Boolean) isCertValidMeth.invoke(cm, arglist2);

		    if (!objSigner.booleanValue() && !emailSigner.booleanValue())
                        continue;
                }

		// Filter out invalid CA cert for SSL
                //if (cm.isCertValid(nickname, false, CertUsage.SSLCA) == false)
                if (isTrustedSSLCACertStore())
                {
		    Field sslcafield = jsscertusage.getField("SSLCA");
		    Object arglist3[] = new Object[3];
		    arglist3[0] = nickname;
		    arglist3[1] = Boolean.FALSE;
		    arglist3[2] = sslcafield.get(certsArray[i]);
	  	    Boolean sslca = (Boolean) isCertValidMeth.invoke(cm, arglist3);

		    if (!sslca.booleanValue())
                       continue;
                }

		// Convert org.mozilla.jss.crypto.X509Certificate[] into
		// java.security.cert.X509Certificate[] because this is
		// what J2SE recognizes. 
		//generateCertificate(certsArray[i].getEncoded(), certs);
		Method getEncodedMeth = jsscertarr.getMethod("getEncoded", null);
	    	byte[] certdata = (byte[]) getEncodedMeth.invoke(certsArray[i], null);
		generateCertificate(certdata, certs);
	    }   
	}
	catch(Throwable e)
	{
	    e.printStackTrace();
	}
		
	Trace.msgSecurityPrintln("mozilla.cert.loaded", new Object[]{getName()});
    }    

    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	throw new KeyStoreException("Cannot store certificate in Mozilla \"" + getName() + "\" certificate store.");
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
	throw new KeyStoreException("Cannot add certificate in Mozilla \"" + getName() + "\" certificate store.");
    }

    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public boolean remove(Certificate cert) throws IOException, KeyStoreException 
    {
	throw new KeyStoreException("Cannot remove certificate from Mozilla \"" + getName() + "\" certificate store.");
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
	Trace.msgSecurityPrintln("mozilla.cert.instore", new Object[] {getName()});

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
	Trace.msgSecurityPrintln("mozilla.cert.canverify", new Object[] {getName()});
	Trace.msgSecurityPrintln("mozilla.cert.tobeverified", new Object[] {cert});
	
	for (Iterator iter = getCertificates().iterator(); iter.hasNext(); )
	{
	    X509Certificate rootCert = (X509Certificate) iter.next();

	    Trace.msgSecurityPrintln("mozilla.cert.tobecompared", new Object[] {getName(), rootCert});

	    try
	    {
    		cert.verify(rootCert.getPublicKey());
		Trace.msgSecurityPrintln("mozilla.cert.verify.ok", new Object[] {getName()});
		return true;
	    }
	    catch (GeneralSecurityException e)
	    {
		// Ignore exception		
	    }
	}

	Trace.msgSecurityPrintln("mozilla.cert.verify.fail", new Object[] {getName()});

	return false;
    }


    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return collection for certificates
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
     * Return name of the Mozilla cert store.
     */
    protected abstract String getName();
 
    /**
     *  Return true if it is a trusted signing CA cert store
     */
    protected abstract boolean isTrustedSigningCACertStore();

    /**
     *  Return true if it is a trusted SSL CA cert store
     */
    protected abstract boolean isTrustedSSLCACertStore();
}
