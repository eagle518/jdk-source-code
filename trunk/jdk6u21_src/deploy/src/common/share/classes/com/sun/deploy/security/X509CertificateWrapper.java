/*
 * @(#)X509CertificateWrapper.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.security.cert.X509Certificate;
import java.util.Date;
import java.util.Set;
import java.util.List;
import java.util.Collection;
import java.math.BigInteger;
import java.security.Principal;
import java.security.PublicKey;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.CertificateParsingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.SignatureException;
import javax.security.auth.x500.X500Principal;
import sun.security.x509.NetscapeCertTypeExtension;


/**
 * This class extends from X509Certificate class
 * This is for special Netscape_cert_type extension check in 
 * getBasicConstraints method for Java deployment only
 *
 * @version 1.0
 * @author Dennis Gu
 */


final class X509CertificateWrapper extends X509Certificate {

     private final X509Certificate cert;

     // Certificate stores used for signed applet verification
     private final static String OID_NETSCAPE_CERT_TYPE = "2.16.840.1.113730.1.1";

     // Netscape certificate type extension
     private final static String NSCT_OBJECT_SIGNING_CA = NetscapeCertTypeExtension.OBJECT_SIGNING_CA;
     private final static String NSCT_SSL_CA = NetscapeCertTypeExtension.SSL_CA;
     private final static String NSCT_S_MIME_CA = NetscapeCertTypeExtension.S_MIME_CA;

     X509CertificateWrapper(X509Certificate cert) {
	this.cert = cert;
     }

     public int getBasicConstraints() {
	int bc = cert.getBasicConstraints();
	if (bc != -1)
	   return bc;

	try {
	    // Check Netscape certificate type extension
            if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
            {
           	// Require any of bits 5,6,7 are true
           	if ((CertUtils.getNetscapeCertTypeBit(cert, NSCT_SSL_CA) == true ||
		   CertUtils.getNetscapeCertTypeBit(cert, NSCT_S_MIME_CA) == true ||
		   CertUtils.getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == true)) 
           	{
		   return Integer.MAX_VALUE;
           	}
            } 
	}
	catch (CertificateException ce) {
	    ce.printStackTrace();
	}
	catch (IOException ioe) {
	    ioe.printStackTrace();
	}

	// If we get here, check basic constraints failed.
	return -1;
     }	

    public boolean[] getKeyUsage() {
	return cert.getKeyUsage();
    }

    public boolean[] getSubjectUniqueID() {
	return cert.getSubjectUniqueID();
    }

    public boolean[] getIssuerUniqueID() {
	return cert.getIssuerUniqueID();
    }

    public byte[] getSigAlgParams() {
	return cert.getSigAlgParams();
    }

    public String getSigAlgOID() {
	return cert.getSigAlgOID();
    }

    public String getSigAlgName() {
	return cert.getSigAlgName();
    }

    public byte[] getSignature() {
	return cert.getSignature();
    }

    public BigInteger getSerialNumber() {
	return cert.getSerialNumber();
    }

    public Date getNotAfter() {
	return cert.getNotAfter();
    }

    public Date getNotBefore() {
	return cert.getNotBefore();
    }

    public Principal getSubjectDN() {
	return cert.getSubjectDN();
    }

    public byte[] getTBSCertificate() throws CertificateEncodingException {
	return cert.getTBSCertificate();
    }

    public int getVersion() {
	return cert.getVersion();
    }

    public Principal getIssuerDN() {
	return cert.getIssuerDN();
    }

    public void checkValidity() throws 
		CertificateExpiredException, CertificateNotYetValidException {
	// We overwrite this method and checkValidity
	// will be done in Java deployment code
	return;
    }

    public void checkValidity(Date date) throws 
		CertificateExpiredException, CertificateNotYetValidException {
	// We overwrite this method and checkValidity
	// will be done in Java deployment code
	return;
    }

    public PublicKey getPublicKey() {
	return cert.getPublicKey();
    }

    public byte[] getEncoded() throws CertificateEncodingException {
	return cert.getEncoded();
    }

    public String toString() {
	return cert.toString();
    }

    public void verify(PublicKey key) throws CertificateException, SignatureException, 
		InvalidKeyException, NoSuchAlgorithmException, NoSuchProviderException {
	cert.verify(key);
    }

    public void verify(PublicKey key, String sigProvider) throws CertificateException, 
		InvalidKeyException, NoSuchAlgorithmException, NoSuchProviderException,
		SignatureException {
	cert.verify(key,sigProvider);
    }

    public byte[] getExtensionValue(String oid) {
	return cert.getExtensionValue(oid);
    }

    public Set getNonCriticalExtensionOIDs() {
	return cert.getNonCriticalExtensionOIDs();
    }

    public Set getCriticalExtensionOIDs() {
	return cert.getCriticalExtensionOIDs();
    }

    public boolean hasUnsupportedCriticalExtension() {
	return cert.hasUnsupportedCriticalExtension();
    }

    public List getExtendedKeyUsage() throws CertificateParsingException {
	return cert.getExtendedKeyUsage();
    }

    public Collection getIssuerAlternativeNames() throws CertificateParsingException {
	return cert.getIssuerAlternativeNames();
    }

    public X500Principal getIssuerX500Principal() {
	return cert.getIssuerX500Principal();
    }

    public Collection getSubjectAlternativeNames() throws CertificateParsingException {
	return cert.getSubjectAlternativeNames();
    }

    public X500Principal getSubjectX500Principal() {
	return cert.getSubjectX500Principal();
    }

    public boolean equals(Object other) {
	return cert.equals(other);
    }

    public int hashCode() {
	return cert.hashCode();
    }

}
