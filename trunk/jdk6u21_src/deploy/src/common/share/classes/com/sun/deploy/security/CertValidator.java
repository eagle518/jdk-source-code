/*
 * @(#)CertValidator.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * This class define the CertValidator for the old  
 * JRE verstion which don't have Validator class
 *
 * @version 1.0
 * @author Dennis Gu
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.Collection;
import java.text.DateFormat;
import java.text.ParseException;
import java.security.CodeSigner;
import java.security.Timestamp;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.GeneralSecurityException;
import java.security.Principal;
import java.security.PublicKey;
import java.security.KeyStore;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateParsingException;
import java.security.cert.CertificateNotYetValidException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.ui.AppInfo;
import sun.security.x509.NetscapeCertTypeExtension;
import sun.security.util.DerValue;
import sun.security.util.DerInputStream;
import com.sun.deploy.config.Config;

/**
 * CertValidator is a class to do the validate for the certificate
 * which signed jar file, this class will be called only when OLD JRE
 * in Java webstart is used, which is early than JRE 1.6
 */

final class CertValidator 
{
    // Certificate stores used for signed applet verification
    //
    public static boolean validate(CodeSource cs, AppInfo ainfo, 
				Certificate[] certs, int chainNum,
				CertStore rootStore, CertStore browserRootStore, 
				CertStore browserTrustedStore, CertStore sessionStore, 
				CertStore permanentStore, CertStore deniedStore) 
		throws CertificateEncodingException, 
		CertificateExpiredException, CertificateNotYetValidException, 
		CertificateParsingException, CertificateException, 
		KeyStoreException, NoSuchAlgorithmException, IOException {


	// Start validation for old JRE (pre 1.6)
    	HashMap trustedPrincipals = new HashMap();
	String	msg = null;
	boolean rootCANotValid = false;
	boolean timeNotValid = false;
        int certValidity = CertificateStatus.VALID;
	boolean trustDecision = false;
	int start = 0;
	int end = 0;	    
	
	// Substitute invalid or missing certs from trust store
	trustedPrincipals = getCertMap(rootStore, browserRootStore);
	Date date = new Date();
	certs = canonicalize(certs, date, trustedPrincipals);

	chainNum = 0;
	while (end < certs.length) {
	    int i = start;
	    CertificateExpiredException certExpiredException = null;
	    CertificateNotYetValidException certNotYetValidException = null;

	    for (i = start; i < certs.length; i++)
	    {
		    X509Certificate currentCert = null;
		    X509Certificate issuerCert = null;

		    if (certs[i] instanceof X509Certificate)
			currentCert = (X509Certificate) certs[i];

		    if (((i+1)<certs.length) && certs[i+1] instanceof X509Certificate)
			issuerCert = (X509Certificate) certs[i+1];
		    else
			issuerCert = currentCert;
	
		    // Check if the certificate is valid and has not expired.
		    //
		    try
		    {
		    	currentCert.checkValidity();
		    }
		    catch (CertificateExpiredException e1)
		    {
		    	if (certExpiredException == null)
			   certExpiredException = e1;
		    }
		    catch (CertificateNotYetValidException e2)
		    {
		    	if (certNotYetValidException == null)
			   certNotYetValidException = e2;
		    }

		    // Check certificate extensions
		    // If the root CA is in our cacerts file or in browser cert store
		    // or it is the last cert in cert chain, skip the extensions check
		    if (rootStore.contains(currentCert) == false && (i+1) != certs.length 
			&& CertUtils.isIssuerOf(currentCert, issuerCert))
		    {
			if (browserRootStore == null 
			    || browserRootStore.contains(currentCert) == false)
			{
			    // Check usage for code signing
			    CertUtils.checkUsageForCodeSigning(currentCert, i-start);
			}
		    }

		    if (CertUtils.isIssuerOf(currentCert, issuerCert))
		    {
			// Check certificate signature
			// We verify that this issuer did indeed sign the certificate.
			try {
			   currentCert.verify(issuerCert.getPublicKey());
			} catch(GeneralSecurityException se) {
		            Trace.msgSecurityPrintln("trustdecider.check.signature");
			    msg = ResourceManager.getMessage("trustdecider.check.signature");
			    throw new CertificateException(msg);
			}
		    } 
		    else 
			break;
	    } // loop certs in one chain
	    end = (i < certs.length) ? (i + 1): i;

	    // Pop up security dialog if option is enabled
            if (!Config.getBooleanProperty(Config.SEC_ASKGRANT_SHOW_KEY)) { 
               // user not allowed to grant permissions to new certs ... 
               msg = ResourceManager.getMessage(
                     "trustdecider.user.cannot.grant.any");
               throw new CertificateException(msg);
            }

	    // Otherwise, we need to popup a dialog and determine the outcome
	    // First, we need to verify if the certificate chain is
	    // signed by a CA
	    //
	    boolean verified = (rootStore.verify(certs[end-1])) 
		|| (browserRootStore != null && browserRootStore.verify(certs[end-1]));
			    
	    if (verified == false) {
	       // make sure user is allowed to grant to certs not in CA
	       if (!Config.getBooleanProperty(Config.SEC_ASKGRANT_NOTCA_KEY)) { 
                  // user not allowed to grant permissions to this cert
                  msg = ResourceManager.getMessage(
                        "trustdecider.user.cannot.grant.notinca");
                  throw new CertificateException(msg); 
               } 
	       rootCANotValid = true;
	    }

	    // Check if the user still want to proceed if the cert expired.
	    if (certExpiredException != null) {
	       timeNotValid = true;
               certValidity = CertificateStatus.EXPIRED;
	    }
            if (certNotYetValidException != null){
                timeNotValid = true;
                certValidity = CertificateStatus.NOT_YET_VALID;
            }                

	    // Check time stamp info if available
            Date timeStampInfo = null;

	    try { // Handle Java web start to running old JRE
	        CodeSigner[] signersArray = cs.getCodeSigners();
                Timestamp ts = signersArray[chainNum].getTimestamp();

                // If we do have timestamping info
                if (ts != null) {
		   Trace.msgSecurityPrintln("trustdecider.check.timestamping.yes");

                   timeStampInfo = ts.getTimestamp();
                   CertPath tsCertPath = ts.getSignerCertPath();

		   // If the certificate has been expired.
                   if (timeNotValid) {
		      Trace.msgSecurityPrintln("trustdecider.check.timestamping.need");

                      Date certNotAfter = ((X509Certificate)certs[end-1]).getNotAfter();
                      Date certNotBefore = ((X509Certificate)certs[end-1]).getNotBefore();

                      // Check if time stamp is in the valid period
                      if (timeStampInfo.before(certNotAfter) && timeStampInfo.after(certNotBefore)) {
		         Trace.msgSecurityPrintln("trustdecider.check.timestamping.valid");

			 // Check TSA certificate chain
			 boolean tsaValid = checkTSAPath(tsCertPath, date, browserRootStore, 
						rootStore, trustedPrincipals);
			 if (tsaValid) {
			    timeNotValid = false;
                            certValidity = CertificateStatus.VALID;
			 }
			 else {		
			    timeStampInfo = null;	
			 }
                      }
		      else
		         Trace.msgSecurityPrintln("trustdecider.check.timestamping.invalid");
                   }
		   else // No need to check timestamping
		      Trace.msgSecurityPrintln("trustdecider.check.timestamping.noneed");
                } // ts available
                else // No timestamping info
		   Trace.msgSecurityPrintln("trustdecider.check.timestamping.no");
	    } //try
            catch(NoSuchMethodError nsme) {
	        Trace.msgSecurityPrintln("trustdecider.check.timestamping.notfound");
            }

	    // Check with deny store first.
	    boolean denyFlag = false;

	    // If we find this cert in deny cert store,
            // check whether user denied a valid or expired certificate.
            // if it is a valid cert, then we will always deny it.
	    if (deniedStore.contains(certs[start])) {
               if (deniedStore.contains(certs[start], true)) {
                  denyFlag = true;
               }
               else {
	 	  denyFlag = timeNotValid;
	       }
	    } 

	    // Only pop up security dialog box for the cert chain which
            // 1. Not in Deny cert store.
            // 2. It exist in Deny cert store (but it is a expired one),
            //    the cert chain we encounter is valid one.
            if (!denyFlag) {
                // First the certificate must be stored in the permanent cert store.
                // Then we will only trust it if the cert valid.
		// or we have trusted expired cert
		if (permanentStore.contains(certs[start])) {
		   if (!timeNotValid || 
			permanentStore.contains(certs[start], true) == false) {
       		      return true;
		   }
		}

                // We need to determine if the certificate has been stored
                // in the session certificate store
		if (sessionStore.contains(certs[start]))  {
		   if (!timeNotValid ||
			sessionStore.contains(certs[start], true) == false) {
       		      return true;
		   }
		}

	 	// Check if the certificate has been trusted by the browser
                if (browserTrustedStore != null &&
			browserTrustedStore.contains(certs[start])) {
		   return true;
                }

		// This certificate chain has not been encountered before, 
		// popup the certificate dialog
		int action = X509Util.showSecurityDialog(
                        certs, cs.getLocation(), start, end, 
			rootCANotValid, certValidity, timeStampInfo, ainfo); 

		// Persist the action in either the permanent or session 
		// certificate store.
		if (action == TrustDeciderDialog.TrustOption_GrantThisSession)
		{
		     Trace.msgSecurityPrintln("trustdecider.user.grant.session");

		     // Grant this session 
		     sessionStore.add(certs[start], !timeNotValid);
		     sessionStore.save();
		     trustDecision = true;
		}
		else if (action == TrustDeciderDialog.TrustOption_GrantAlways)
		{
		     Trace.msgSecurityPrintln("trustdecider.user.grant.forever");

		     // Grant always
		     CertStore userPermanentStore = DeploySigningCertStore.getUserCertStore();
                     userPermanentStore.load(true);
                     if (userPermanentStore.add(certs[start], !timeNotValid)) {
                        userPermanentStore.save();
                     }
                     trustDecision = true;
	 	}
		else
		{ 
		     Trace.msgSecurityPrintln("trustdecider.user.deny");

		     // Deny
		     deniedStore.add(certs[start], !timeNotValid);
		     deniedStore.save(); 
		}

		// If user Grant permission, just pass all security checks.
		// If user Deny first signer, pop up security box for second signer certs  
		if (trustDecision)
		     return true;
	    } // not in Deny store
            start = end;
	    chainNum++;
	}

	return false;
    }

    private static boolean checkTSAPath(CertPath tsCertPath, Date currDate, 
					CertStore browserRootStore, CertStore rootStore,
					HashMap trustedPrincipals)
    {
	Trace.msgSecurityPrintln("trustdecider.check.timestamping.tsapath");

	try 
	{
	    List tsCertList = tsCertPath.getCertificates();
	    Object[] tsCertsObj = tsCertList.toArray();

	    // Create certificate array
	    int certLen = tsCertsObj.length;	

	    Certificate[] tsCerts = new Certificate[certLen];
	    for (int k=0; k<certLen; k++) 
		tsCerts[k] = (Certificate)tsCertsObj[k];

	    // Build missing ca in the chain
	    tsCerts = canonicalize(tsCerts, currDate, trustedPrincipals);

	    // Get the TrustAnchor of the TSA
	    int tsSize = tsCerts.length;
	    Certificate trustAnchorCert = tsCerts[tsSize - 1];

	    // Check if TSA is in root ca
	    if (rootStore.verify(trustAnchorCert) == true 
		|| (browserRootStore != null && browserRootStore.verify(trustAnchorCert) == true)) 
	    {
		Trace.msgSecurityPrintln("trustdecider.check.timestamping.inca");

		// Now we have to check each cert in cert chain
		for (int i = 0; i < (tsSize-1); i++) 
		{
		    X509Certificate currentCert = (X509Certificate) tsCerts[i];
		    X509Certificate issuerCert = (X509Certificate) tsCerts[i+1];

		    try
		    {
			// Check certificate usage with timestamping check
			CertUtils.checkUsageForCodeSigning(currentCert, i, true);

			// Check certificate signature
			// We verify that this issuer did indeed sign the certificate.
			currentCert.verify(issuerCert.getPublicKey());
		    } 
		    catch(GeneralSecurityException se) 
		    {
			Trace.msgSecurityPrintln("trustdecider.check.signature");
			return false;
		    }
		}
		
		// No error happened, return true
		return true;
	    }
	    else 
	    { // not in cacerts
		Trace.msgSecurityPrintln("trustdecider.check.timestamping.notinca");
		return false;
	    }
	}
	catch (Exception e) 
	{
	    return false;
	}
    }
    
    /**
     * Obtain all the certificates in the cacerts file
     */
    private static synchronized HashMap getCertMap(CertStore rootStore, CertStore browserRootStore) 
					throws KeyStoreException
    {
	HashMap trustedPrinMap = new HashMap();

	if (rootStore != null) {
	   Iterator iterRootStore = rootStore.getCertificates().iterator();

           while (iterRootStore.hasNext())
           {
              // Get certificate from store
              Certificate cert = (Certificate) iterRootStore.next();

              // Add certificate into collection
	      if (cert instanceof X509Certificate)
		 trustedPrinMap = addTrustedCert((X509Certificate)cert, trustedPrinMap);
           }
	}

	if (browserRootStore != null) {
	   Iterator iterBrowserRoot = browserRootStore.getCertificates().iterator();

           while (iterBrowserRoot.hasNext())
           {
              // Get certificate from store
              Certificate cert = (Certificate) iterBrowserRoot.next();

              // Add certificate into collection
	      if (cert instanceof X509Certificate)
		 trustedPrinMap = addTrustedCert((X509Certificate)cert, trustedPrinMap);
           }
	}

	return trustedPrinMap;
    }

    /**
     * Add a certificate as trusted.
     */
    private static HashMap addTrustedCert(X509Certificate cert, HashMap trustedPrincipals)
    {
	Principal principal = X509Util.getSubjectPrincipal(cert);
	Collection coll = (Collection)trustedPrincipals.get(principal);

	if (coll == null)
	{
	   // This actually should be a set, but duplicate entries
	   // are not a problem and we can avoid the Set overhead
	   coll = new ArrayList();

	   trustedPrincipals.put(principal,coll);
	}
	coll.add(cert);

	return trustedPrincipals;
     }

    /*
     * Rewrite the certificate chain to substitue locally trusted
     * certificates in place of certificates. Also add missing self-signed
     * root certificates.
     */
    private static Certificate[] canonicalize(Certificate[] chain, Date date, 
				HashMap trustedPrincipals) throws CertificateException
    {
	List c = new ArrayList(chain.length);
	boolean updated = false;

	if (chain.length == 0)
	   return chain;

	for (int i = 0; i < chain.length; i++)
	{
	    X509Certificate currentCert = (X509Certificate)chain[i];
	    X509Certificate trustedCert = getTrustedCertificate(currentCert, date, 
							trustedPrincipals);

	    if (trustedCert != null)
	    {
	        Trace.msgSecurityPrintln("trustdecider.check.canonicalize.updatecert");
		currentCert = trustedCert;
	    	updated = true;
	    }

	    c.add(currentCert);	

	    // If the final cert in single signed chain is not self-signed, append a
	    // trusted certificate with a matching subject, if available.
	    Principal subjectName = X509Util.getSubjectPrincipal(chain[i]);
	    Principal issuerName = X509Util.getIssuerPrincipal(chain[i]);
	    Principal nextSubjectName = null;

	    if (i < chain.length - 1) 
	    {
	        nextSubjectName = X509Util.getSubjectPrincipal(chain[i+1]);
	    }

	    if (!issuerName.equals(subjectName) && !issuerName.equals(nextSubjectName)) 
	    { 
	 	X509Certificate issuer = getTrustedIssuerCertificate((X509Certificate)chain[i], 
							date, trustedPrincipals);
	   	if (issuer != null)
	   	{
	           // Add missing root cert
	      	   Trace.msgSecurityPrintln("trustdecider.check.canonicalize.missing");
	      	   updated = true;
	      	   c.add(issuer);
	   	}
	    }
	}

	if (updated)
	   return (Certificate[])c.toArray(new Certificate[c.size()]);
	else
	   return chain;
    }

    /*
     * Return a valid, trusted certificate that matches the input certificate,
     * or null if no such certificate can be found.
     * This method is used to replace a given certificate with a different
     * but equivalent certificate that is currently valid. This is often
     * useful as CAs reissue their root certificates with a new validity period. 
     */
    private static X509Certificate getTrustedCertificate(X509Certificate cert, Date date,
							HashMap trustedPrincipals)
    {
	Principal certSubjectName = X509Util.getSubjectPrincipal(cert);
	List list = (List)trustedPrincipals.get(certSubjectName);
	if (list == null)
	   return null;

	Principal certIssuerName = X509Util.getIssuerPrincipal(cert);
	PublicKey certPublicKey = cert.getPublicKey();
	
	for (Iterator ir = list.iterator(); ir.hasNext(); )
	{
	    X509Certificate mycert = (X509Certificate)ir.next();
	    if (mycert.equals(cert))
	       continue;

	    if (!X509Util.getIssuerPrincipal(mycert).equals(certIssuerName))
	       continue;

	    if (!mycert.getPublicKey().equals(certPublicKey))
	       continue;

	    try {
	       mycert.checkValidity(date);
	    }
	    catch (Exception e){
	       continue;
	    }

	    // All tests pass, this must be the one to use
	    Trace.msgSecurityPrintln("trustdecider.check.gettrustedcert.find");
	    return mycert;
	}
	return null;
    }

    private static X509Certificate getTrustedIssuerCertificate(X509Certificate cert, Date date,
							HashMap trustedPrincipals)
    {
	Principal certIssuerName = X509Util.getIssuerPrincipal(cert);
	List list = (List)trustedPrincipals.get(certIssuerName);

	if (list == null)
	   return null;

	for (Iterator ir = list.iterator(); ir.hasNext(); )
	{
	    X509Certificate mycert = (X509Certificate)ir.next();
	    try {
	       mycert.checkValidity(date);
	    }
	    catch (Exception e){
	       continue;
	    }
	    Trace.msgSecurityPrintln("trustdecider.check.gettrustedissuercert.find");
	    return mycert;
	}
	return null;
    }

    public static boolean isSigner(Certificate check, Certificate signer)
    {
        try {
            check.verify(signer.getPublicKey());
            return true;
        } catch(Exception e) {
            return false;
        }
    }
}
