/*
 * @(#)TrustDecider.java	1.92 10/05/11
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.lang.InterruptedException;
import java.lang.RuntimeException;
import java.io.IOException;
import java.io.DataInputStream;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLConnection;
import java.util.LinkedList;
import java.util.List;
import java.util.LinkedHashSet;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.Collection;
import java.util.Arrays;
import java.util.HashSet;
import java.util.HashMap;
import java.security.CodeSigner;
import java.security.Timestamp;
import java.security.CodeSource;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.GeneralSecurityException;
import java.security.InvalidAlgorithmParameterException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.Principal;
import java.security.cert.CertPath;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateParsingException;
import java.security.cert.CertificateNotYetValidException;
import java.security.cert.CertPathValidatorException;
import java.security.cert.CertPathBuilderException;
import java.security.cert.CertificateFactory;
import java.security.cert.TrustAnchor;
import java.security.cert.CertStoreParameters;
import java.security.cert.CollectionCertStoreParameters;
import java.security.cert.PKIXParameters;
import java.security.cert.PKIXCertPathValidatorResult;
import java.security.cert.CertPathValidator;
import java.security.cert.X509CRL;
import java.security.cert.CRLException;
import java.security.Security;
import javax.security.auth.x500.X500Principal;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.DeployLock;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.services.Service;
import com.sun.deploy.services.ServiceManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.AppInfo;
import sun.security.provider.certpath.OCSP;
import sun.security.provider.certpath.OCSP.RevocationStatus.CertStatus;
import sun.security.validator.Validator;
import sun.security.validator.PKIXValidator;
import sun.security.validator.ValidatorException;

public class TrustDecider 
{
    // Certificate stores used for signed applet verification
    //
    private static CertStore rootStore = null;
    private static CertStore permanentStore = null;
    private static CertStore sessionStore = null;
    private static CertStore deniedStore = null;
    private static CertStore browserRootStore = null;
    private static CertStore browserTrustedStore = null;
    private static List jurisdictionList = null;
    private static final List preTrustList = 
	Collections.singletonList("OU=Java Signed Extensions,OU=Corporate Object Signing,O=Sun Microsystems Inc");
    private static boolean isBrowserRootStoreLoaded = false;
    private static X509CRL crl509 = null;
    private static boolean ocspValidConfig = false;
    private static String ocspSigner = null;
    private static String ocspURL = null;
    private static boolean crlCheck = false;
    private static boolean ocspCheck = false;
    private static boolean ocspEECheck = false;
    private static HashSet deniedURL = null;
	private static DeployLock deployLock = null;

    public static final long PERMISSION_GRANTED_FOR_SESSION = 1;
    public static final long PERMISSION_DENIED = 0;

    static
    {
        deployLock = new DeployLock();
	reset();
    }
    

    /** 
     * Reset the deny session store from applet reload action
     */
    public static void resetDenyStore(){
        Trace.msgSecurityPrintln("trustdecider.check.reset.denystore");
        boolean locked = false;
        try {
            locked = deployLock.lock();
            deniedStore = new DeniedCertStore();
            deniedURL.clear();
        } catch (InterruptedException ie) {
            throw new RuntimeException(ie);
        } finally {
            if (locked) {
                deployLock.unlock();
            }
        }
    }

    /**
     * Return denied signed jar file URL list.
     */
    public static HashSet getDeniedURL () {
        boolean locked = false;
        try {
            locked = deployLock.lock();
            return deniedURL;
        } catch (InterruptedException ie) {
            throw new RuntimeException(ie);
        } finally {
            if (locked) {
                deployLock.unlock();
            }
        }
    }


    /**
     * Reset the TrustDecider.
     */
    public static void reset()
    {
        boolean locked = false;
        try {
            locked = deployLock.lock();

	    rootStore = RootCertStore.getCertStore();
	    permanentStore = DeploySigningCertStore.getCertStore();
	    sessionStore = new SessionCertStore();
	    deniedStore = new DeniedCertStore();
	    deniedURL = new HashSet();
	    jurisdictionList = null;

	    // Get Jurisdiction list from file
	    if (Config.getBooleanProperty(Config.SEC_USE_PRETRUST_LIST_KEY)) { 
	        jurisdictionList = preTrustList;
	    }

	    // see if user is allowed to use browser cert store
	    if (Config.getBooleanProperty(Config.SEC_USE_BROWSER_KEYSTORE_KEY)) 
	    { 
	        Service service = com.sun.deploy.services.ServiceManager.getService();
	        browserRootStore = service.getBrowserSigningRootCertStore();
	        browserTrustedStore = service.getBrowserTrustedCertStore();	  
	        isBrowserRootStoreLoaded = false;
	    }

	    // Get CRL properties based on config file
	    try {
	        java.security.AccessController.doPrivileged(new PrivilegedExceptionAction()
                {
                    public Object run() throws Exception
                    {
		    // Get system property for CRL validation
		    crlCheck = Config.getBooleanProperty(Config.SEC_USE_VALIDATION_CRL_KEY);
		    if (crlCheck) {
		        String crlURL = Config.getProperty(Config.SEC_USE_VALIDATION_CRL_URL_KEY);
		        if (crlURL != null && crlURL.length() > 0 ) {
			    CertificateFactory cf = CertificateFactory.getInstance("X509");
           		    URL url = new URL(crlURL);

           		    URLConnection connection = url.openConnection();
           		    connection.setDoInput(true);
           		    connection.setUseCaches(false);
           		    DataInputStream inStream = new DataInputStream(connection.getInputStream());
           		    crl509 = (X509CRL)cf.generateCRL(inStream);
           		    inStream.close();
		        }
        	    }
		    return null;
                    }
                });
            }
            catch (PrivilegedActionException e)
            {
                e.printStackTrace();
            }

	    // Get OCSP properties based on config file
	    try {
	        java.security.AccessController.doPrivileged(new PrivilegedExceptionAction()
                {
                    public Object run() throws Exception
                    {
		    // Get system property for OCSP validation
		    ocspCheck = Config.getBooleanProperty(Config.SEC_USE_VALIDATION_OCSP_KEY);
		    if (ocspCheck) {
		        ocspSigner = Config.getProperty(Config.SEC_USE_VALIDATION_OCSP_SIGNER_KEY);
        	        ocspURL = Config.getProperty(Config.SEC_USE_VALIDATION_OCSP_URL_KEY);

        	        // User select to use Signer and URL in config file
        	        if (ocspSigner != null && ocspSigner.length() > 0 &&
				    ocspURL != null && ocspURL.length() > 0) {
			    ocspValidConfig = true;
		        }
		    }
		    ocspEECheck = Config.getBooleanProperty(Config.SEC_USE_VALIDATION_OCSP_EE_KEY);
		    return null;
                    }
                });
            }
            catch (PrivilegedActionException e)
            {
                e.printStackTrace();
            }
        } catch (InterruptedException ie) {
            throw new RuntimeException(ie);
        } finally {
            if (locked) {
                deployLock.unlock();
            }
        }

    }

    /* Returns:
         0 if permissions are no granted
         1 if permissions are granted for session only
         expirationDate if permissions are granted for time interval */
    public static long isAllPermissionGranted(CodeSource cs)
		throws CertificateEncodingException, 
		CertificateExpiredException, CertificateNotYetValidException, 
		CertificateParsingException, CertificateException, 
		KeyStoreException, NoSuchAlgorithmException, IOException,
		CRLException, InvalidAlgorithmParameterException {
	return isAllPermissionGranted(cs, new AppInfo());

    }

    public static long isAllPermissionGranted(CodeSource cs, AppInfo ainfo) 
		throws CertificateEncodingException, 
		CertificateExpiredException, CertificateNotYetValidException, 
		CertificateParsingException, CertificateException, 
		KeyStoreException, NoSuchAlgorithmException, IOException, 
		CRLException, InvalidAlgorithmParameterException {
	return isAllPermissionGranted(cs, ainfo, false);
    }

    /*
     * isAllPermissionGranted()
     *
     * This method is synchronized so we will never show two security dialogs at
     * once, and so we will not ever show two dialogs for the same certificate.
     * We need to synchronize the "check if trusted", "ask-user", 
     * "save-trust-decision" sequence.
     * It has been suggested that instead of just synchronizing the
     * "check, ask, save" sequence, we first do the check unsynchronized, then
     * do a synchronized "check, ask, save" if it isn't already granted.
     * We might want to do this in a later release.
     */
    public synchronized static long isAllPermissionGranted(CodeSource cs, 
                AppInfo ainfo, boolean jnlpFlag) 
                throws CertificateEncodingException, 
		CertificateExpiredException, CertificateNotYetValidException, 
		CertificateParsingException, CertificateException, 
		KeyStoreException, NoSuchAlgorithmException, IOException, 
		CRLException, InvalidAlgorithmParameterException {

	// Local variable for OCSP and CRL revocation check
    	boolean crlCheckLocal = crlCheck;
    	boolean ocspCheckLocal = ocspCheck;

        boolean locked = false;
        try {
            locked = deployLock.lock();

            Certificate[] certs = cs.getCertificates();
            URL locationURL = cs.getLocation();

            // If no certificate is found, simply return false.
            if (certs == null)
                return PERMISSION_DENIED;

            // Check our Session cert store, Permanent cert store and Deny cert store
            // first. If any cert in the certificate chain has been stored already,
            // we will give all permission to the applet.
            int 	  	start = 0;
            int 	  	end = 0;	    
            int		chainNum = 0;
            String		msg = null;

            // Loading certificate stores 
            rootStore.load();
            permanentStore.load();
            sessionStore.load();		
            deniedStore.load();	

            // only load browser root store once
            if (browserRootStore != null && !isBrowserRootStoreLoaded) {
                browserRootStore.load();
                isBrowserRootStoreLoaded = true;
            }
            if (browserTrustedStore != null)
                browserTrustedStore.load();

            // determine each signer and its certificate chain
            List chains = new ArrayList();
            while (end < certs.length) {
                List certChain = new ArrayList();
                int i = start;
                while ((i+1) < certs.length) {
                    if ((certs[i] instanceof X509Certificate)
                        && (certs[i+1] instanceof X509Certificate)
                        && CertUtils.isIssuerOf((X509Certificate)certs[i], (X509Certificate)certs[i+1])) 
                        i++;
                    else 
                        break;
                }
                end = i + 1;

                // Copy the one signer cert chain to new chain
                for (int j=start; j<end; j++) {
                    certChain.add(certs[j]);
                }

                chains.add(certChain);
                start = end;
                chainNum++;
            } //Break down the multi-signer

            // We are using the new Validator class in JSSE for JRE version 1.6 and later only
            boolean validatorSupport = true;
            try {
                Class validatorClass = Class.forName("sun.security.validator.Validator", true,
                    ClassLoader.getSystemClassLoader());
                if (validatorClass == null)
                    validatorSupport = false;
            } catch (ClassNotFoundException cnfe) {
                Trace.msgSecurityPrintln("trustdecider.check.validate.notfound");
                validatorSupport = false;
            }

            if (Config.isJavaVersionAtLeast16() && validatorSupport) {
                Trace.msgSecurityPrintln("trustdecider.check.validate.certpath.algorithm");

                // If we get here, no cert in chain has been stored in Session or Permanent store.
                // If they are not in Deny store either, we have to pop up security dialog box 
                // for each signer's certificate one by one.
                boolean rootCANotValid = false;
                boolean timeNotValid = false;
                long trustDecision = PERMISSION_DENIED;
                long expirationDate = 0;
                int certValidity = CertificateStatus.VALID;
                int certValidityNoTS = CertificateStatus.VALID;
	
                // Get a collection of certs from rootStore and browserRootStore
                // Add user, system root ca and browser ca to validate LinkedHashSet 
                LinkedHashSet allRootCerts = new LinkedHashSet();
                allRootCerts.addAll(rootStore.getCertificates());

                // Add browser root ca to validate collection
                if (browserRootStore != null) {
                    allRootCerts.addAll(browserRootStore.getCertificates());
                }

		// Add subject of root ca into hashmap
                HashMap allTrustedSubjects = new HashMap();

		for (Iterator iterCA = allRootCerts.iterator(); iterCA.hasNext();) {
             	    X509Certificate cert = (X509Certificate) iterCA.next();
		    allTrustedSubjects.put(cert.getSubjectX500Principal(), cert);
           	}

                // Loop for multi-signer
                Iterator iterChains = chains.iterator();
                chainNum = 0;
                while (iterChains.hasNext()) {
                    List certList = (List) iterChains.next();
                    X509Certificate[] certArr = (X509Certificate[]) certList.toArray
                        (new X509Certificate[0]);

                    // Check if cert has been expired
                    CertificateExpiredException certExpiredException = null;
                    CertificateNotYetValidException certNotYetValidException = null;

                    //min certificate expiration date for certificates in the current chain
                    expirationDate = Long.MAX_VALUE;
                    for (int i=0; i<certArr.length; i++) {
                        long ntime = certArr[i].getNotAfter().getTime();
                        if (ntime < expirationDate) {
                            expirationDate = ntime;
                        }

                        // Check if the certificate is valid and has not expired.
                        try {
                            certArr[i].checkValidity();
                        }
                        catch (CertificateExpiredException e1) {
                            if (certExpiredException == null) {
                                certExpiredException = e1;
                                certValidity = CertificateStatus.EXPIRED;
                                certValidityNoTS = CertificateStatus.EXPIRED;
                                timeNotValid = true;
                            }
                        }
                        catch (CertificateNotYetValidException e2) {
                            if (certNotYetValidException == null) {
                                certNotYetValidException = e2;
                                certValidity = CertificateStatus.NOT_YET_VALID;
                                certValidityNoTS = CertificateStatus.NOT_YET_VALID;
                                timeNotValid = true;
                            }
                        }
                    }
                    int certArrLen = certArr.length; 
                    X509Certificate last = certArr[certArrLen - 1];
                    Principal issuer = last.getIssuerX500Principal();
                    Principal subject = last.getSubjectX500Principal();

                    // add self-signed cert to root certs but remember this
                    if (issuer.equals(subject) && !allRootCerts.contains(last)) {

		        // If the root CA is replaced, we will still treat it as trust CA.
			if (!isReplacedCA(allTrustedSubjects, last)) {
                            // make sure user is allowed to grant to certs not in CA
                            if (!Config.getBooleanProperty(Config.SEC_ASKGRANT_NOTCA_KEY)) {
                                // user not allowed to grant permissions to this cert
                                msg = ResourceManager.getMessage(
                                    "trustdecider.user.cannot.grant.notinca");
                                throw new CertificateException(msg);
                            }

                            rootCANotValid = true;
			}
                        allRootCerts.add(last);
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

                                Date certNotAfter = certArr[certArrLen-1].getNotAfter();
                                Date certNotBefore = certArr[certArrLen-1].getNotBefore();

                                // Check if time stamp is in the valid period
                                if (timeStampInfo.before(certNotAfter) && timeStampInfo.after(certNotBefore)) {
                                    Trace.msgSecurityPrintln("trustdecider.check.timestamping.valid");

                                    // Check TSA certificate chain
                                    boolean tsaValid = checkTSAPath(tsCertPath, allRootCerts);
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
                    }
                    catch(NoSuchMethodError nsme) {
                        Trace.msgSecurityPrintln("trustdecider.check.timestamping.notfound");
                    }

                    // If we found trusted.publishers file
                    boolean isTrustedExtension = false;
                    if (jurisdictionList != null) {
                        Trace.msgSecurityPrintln("trustdecider.check.jurisdiction.found");

                        // if the certificate is valid and has trusted root CA,
                        // we will check if it is trusted extension ceritificate
                        if (!rootCANotValid && (certValidity == CertificateStatus.VALID)) {
                            Trace.msgSecurityPrintln("trustdecider.check.trustextension.on");
                            isTrustedExtension = checkTrustedExtension(certArr[0]);
                        } 
                        else {
                            Trace.msgSecurityPrintln("trustdecider.check.trustextension.off");
                        }
                    }
                    else {
                        Trace.msgSecurityPrintln("trustdecider.check.jurisdiction.notfound");
                    }

                    // If this is a extension installation,
		    // and it is signed by valid and trusted certificate
		    // but not pre-trusted certificate
		    // and does not perform a unsecure operation
		    // and certificate is not expired (discard timestamping)
		    // We will turn on ocsp/crl check
                    if (!rootCANotValid && (certValidityNoTS == CertificateStatus.VALID) &&
			(ainfo.getType() == 3) && !jnlpFlag && !isTrustedExtension) {
                        // we will turn on OCSP and CRL only for the first time 
                        if (!permanentStore.contains(certArr[0])) {
                            crlCheckLocal = true; 
                            ocspCheckLocal = true; 
                            // Set security property to check end entity revocation only
                            Security.setProperty("com.sun.security.onlyCheckRevocationOfEECert", "true");
                            Trace.msgSecurityPrintln("trustdecider.check.extensioninstall.on");
                        }
		    }

                    // Start Certificate Validation
                    boolean hasCRL = false;
                    boolean hasOCSP = false;
		    PKIXParameters params = null;
                    try {
                        // Get Validator
                        Validator v = Validator.getInstance("PKIX", Validator.VAR_PLUGIN_CODE_SIGNING, allRootCerts);
                        // Set parameters and enable revocation
                        PKIXValidator pkixv = (PKIXValidator) v;
                        params = pkixv.getParameters();
                        params.addCertPathChecker(new DeployCertPathChecker(pkixv));

                        // ---------------------------------------
                        // Start CRL Checking if option is enabled
                        // ---------------------------------------
                        if (crlCheckLocal) {
                            Trace.msgSecurityPrintln("trustdecider.check.validation.crl.on");

                            if (crl509 != null) {
                                hasCRL = true;
                            }
                            else {
                                // Check if certificate has CRL extension
                                for (int i=0; i<certArr.length; i++) {
                                    if (CertUtils.getCertCRLExtension(certArr[i])) {
                                        hasCRL = true;
                                    }
                                }
                            }
                            params = doCRLValidation(params, hasCRL);
                        }
                        else {
                            Trace.msgSecurityPrintln("trustdecider.check.validation.crl.off");
                        }

                        // ----------------------------------------
                        // Start OCSP Checking if option is enabled
                        // ----------------------------------------
                        if (ocspCheckLocal) {
                            Trace.msgSecurityPrintln("trustdecider.check.validation.ocsp.on");

                            if (ocspValidConfig) {
                                hasOCSP = true;
                            }
                            else {
                                // Check if certificate has AIA extension
                                for (int i=0; i<certArr.length; i++) {
                                    if (CertUtils.getCertAIAExtension(certArr[i])) {
                                        hasOCSP = true;
                                    }
                                }
                            }
                            doOCSPValidation(params, allRootCerts, certArr, hasOCSP, crlCheckLocal);
                        }
                        else {
                            Trace.msgSecurityPrintln("trustdecider.check.validation.ocsp.off");
                        }

                        // Using X509CertificateWrapper class for validation
                        X509Certificate[] certArrWrapper = new X509Certificate[certArrLen];
                        for (int k=0; k<certArrLen; k++)
                            certArrWrapper[k] = new X509CertificateWrapper(certArr[k]);

                        v.validate(certArrWrapper);

                        // If we get here, OCSP or CRL validation succeed.
                        if ((crlCheckLocal && hasCRL) || (ocspCheckLocal && hasOCSP) ) {
                            Trace.msgSecurityPrintln("trustdecider.check.revocation.succeed");
                        }
                    }
                    catch (CertificateException ce) {
                        // make sure user is allowed to grant to certs not in CA
                        if (!Config.getBooleanProperty(Config.SEC_ASKGRANT_NOTCA_KEY)) {
                            // user not allowed to grant permissions to this cert
                            msg = ResourceManager.getMessage(
                                "trustdecider.user.cannot.grant.notinca");
                            throw new CertificateException(msg);
                        }

                        if (ce instanceof ValidatorException) {
                            ValidatorException ve = (ValidatorException) ce;
                            if (ValidatorException.T_NO_TRUST_ANCHOR.equals(ve.getErrorType())) {
                                rootCANotValid = true;
                            }
                            else {
                                // Check CRL and OCSP status 
                                String revokedMsg = "Certificate has been revoked";
                                if ((crlCheckLocal && hasCRL) || (ocspCheckLocal && hasOCSP)) {
                                    String msgErr = ve.getMessage();
                                    // This cert has been revoked by CRL or OCSP.
                                    if (msgErr.contains(revokedMsg)) {
                                        Trace.msgSecurityPrintln("trustdecider.check.validation.revoked");
                                    }
                                    else {
                                        Trace.msgSecurityPrintln(msgErr);
                                    }
                                    throw ve;
                                }
                                else { //No CRL and OCSP check
                                    throw ve;
                                }
                            } // Valid CA
                        } // ValidatorException
                        else {
                            throw ce;
                        }
                    } // Catch
                    catch (IOException e) {
                        Trace.msgSecurityPrintln(e.getMessage());
                        throw e;
                    }
                    catch (InvalidAlgorithmParameterException iape) {
                        Trace.msgSecurityPrintln(iape.getMessage());
                        throw iape;
                    }
                    catch (CRLException crle) {
                        Trace.msgSecurityPrintln(crle.getMessage());
                        throw crle;
                    }
                    finally {
                        Security.setProperty("com.sun.security.onlyCheckRevocationOfEECert", "false");
                    }

		    // Check the revocation status on a best-effort basis of EE cert via 
		    // OCSP for the following condition:
 		    // 1. OCSP has not been previously checked
		    // 2. OCSP responder is specified
		    // 3. it is not a self-signed certificate. (cert length > 1) 
		    // 4. Certificate is valid (and unexpired - discard timestamp)
		    // 5. Root CA is trusted
		    // 6. This is not pre-trusted certificate
		    // 7. Network failures (timeouts, offline, etc) are OK.
		    // By default, all EE certs are checked (can be disabled via control panel).
		    if (certArrLen > 1 && !ocspCheckLocal && ocspEECheck && 
				!isTrustedExtension && !rootCANotValid && 
				(certValidityNoTS == CertificateStatus.VALID)) {
			// check revocation status of EE cert with OCSP
			// first time only
			if (!permanentStore.contains(certArr[0])) {
			    try {
				CertStatus status = doOCSPEEValidation
					(certArr[0], certArr[1], allRootCerts, params.getDate());
				if (status != CertStatus.GOOD) {
				    // Show error dialog and allow user to continue 
				    // with sandbox permissions (Java applet)
				    Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.bad");
                            	    msg = ResourceManager.getMessage
					("trustdecider.check.ocsp.ee.revoked");
				    throw new CertificateException (msg);
				}
				else {
				    Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.good");
			 	}
			    } catch (IOException ioe) {
				// By default, network failures are o.k.
				// but we should log this ...
				Trace.msgSecurityPrintln(ioe.getMessage());
			    } catch (CertPathValidatorException cve) {
				Trace.msgSecurityPrintln(cve.getMessage());
				throw new CertificateException(cve);
			    }
			} // not in permanent store
		    } // Check EE OCSP
		    else {
			Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.off");
		    }

                    // Remove the root cert just added
                    if (rootCANotValid){
                        allRootCerts.remove(last);
                    }

                    // Check with deny store. 
                    boolean denyFlag = false;

                    // If we find this cert in deny cert store,
                    // check whether user denied a valid or expired certificate.
                    // if it is a valid cert, then we will always deny it.
                    if (deniedStore.contains(certArr[0])) {
                        deniedURL.add(locationURL);
                        if (deniedStore.contains(certArr[0], true)) {
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
                        if (permanentStore.contains(certArr[0]))  {
                            if (!timeNotValid ||
                                permanentStore.contains(certArr[0], true) == false) {
                                return expirationDate; // PERMISSION_GRANTED
                            }
                        }

                        // If this is trusted extension certificate
                        // we won't popup security dialog box as well.
                        if (isTrustedExtension) {
                            if (permanentStore.contains(certArr[0], true) == false) {
                                // Add to trusted cert store automatically 
                                CertStore userPermanentStore = DeploySigningCertStore.getUserCertStore();
                                userPermanentStore.load(true);
                                if (userPermanentStore.add(certArr[0], true)) {
                                    userPermanentStore.save(); 
                                }
                                Trace.msgSecurityPrintln("trustdecider.check.trustextension.add");
                                return expirationDate; // PERMISSION_GRANTED
                            }
                        } // trusted extension

                        // We need to determine if the certificate has been stored
                        // in the session certificate store
                        if (sessionStore.contains(certArr[0]))  {
                            if (!timeNotValid ||
                                sessionStore.contains(certArr[0], true) == false) {
                                return PERMISSION_GRANTED_FOR_SESSION;
                            }
                        }

                        // Check if the certificate has been trusted by the browser
                        if (browserTrustedStore != null && 
                            browserTrustedStore.contains(certArr[0])) {
                            return PERMISSION_GRANTED_FOR_SESSION;
                        }

                        // see if user is allowed to see any certificate dialog
                        if (!Config.getBooleanProperty(Config.SEC_ASKGRANT_SHOW_KEY)) { 
                            // user not allowed to grant permissions to new certs ... 
                            msg = ResourceManager.getMessage(
                                "trustdecider.user.cannot.grant.any");
                            throw new CertificateException(msg);
                        }

                        if (locked) {
                            deployLock.unlock();
                            locked = false;
                        }

                        // This certificate chain has not been encountered before, 
                        // popup the certificate dialog
                        int action = X509Util.showSecurityDialog(certArr, 
                            cs.getLocation(), 0, certArrLen, rootCANotValid, 
                            certValidity, timeStampInfo, ainfo, jnlpFlag); 

                        locked = deployLock.lock();
                        // Persist the action in either the permanent or session 
                        // certificate store.
                        if (action == TrustDeciderDialog.TrustOption_GrantThisSession) {
                            Trace.msgSecurityPrintln("trustdecider.user.grant.session");

                            // Grant this session 
                            sessionStore.add(certArr[0], !timeNotValid);
                            sessionStore.save();
                            trustDecision = PERMISSION_GRANTED_FOR_SESSION;
                        }
                        else if (action == TrustDeciderDialog.TrustOption_GrantAlways) {
                            Trace.msgSecurityPrintln("trustdecider.user.grant.forever");

                            // Grant always 
                            CertStore userPermanentStore = DeploySigningCertStore.getUserCertStore();
                            userPermanentStore.load(true);
                            if (userPermanentStore.add(certArr[0], !timeNotValid)) {
                                userPermanentStore.save(); 
                            }
                            trustDecision = expirationDate; //PERMISSION_GRANTED;
                        }
                        else { 
                            Trace.msgSecurityPrintln("trustdecider.user.deny");

                            // Deny
                            deniedStore.add(certArr[0], !timeNotValid);
                            deniedStore.save();
                            deniedURL.add(locationURL);
                        }

                        // If user Grant permission, just pass all security checks.
                        // If user Deny first signer, pop up security box for second signer certs  
                        if (trustDecision != PERMISSION_DENIED) 
                            return trustDecision;
                    } //Not in Deny store
                    chainNum++;
                } //loop for multi-signer
            } // new JRE
            else {
                // In order to support old JRE version for Java web start, we have to call our own
                // Certificate validator old JRE early than 1.6.
                Trace.msgSecurityPrintln("trustdecider.check.validate.legacy.algorithm");
                if (CertValidator.validate(cs, ainfo, certs, chainNum,
                    rootStore, browserRootStore, browserTrustedStore, 
                    sessionStore, permanentStore, deniedStore)) {
                    /* this only affects possibility of caching of result of validation */
                    return PERMISSION_GRANTED_FOR_SESSION;
                }
            }
        } catch (InterruptedException ie) {
            throw new RuntimeException(ie);
        } finally {
            if (locked) {
                deployLock.unlock();
            }
        }
	return PERMISSION_DENIED;
    }

    private static boolean checkTSAPath(CertPath tsCertPath, LinkedHashSet allRootCerts)
    {
	Trace.msgSecurityPrintln("trustdecider.check.timestamping.tsapath");

	Validator v = Validator.getInstance("PKIX", Validator.VAR_TSA_SERVER, allRootCerts);
	List tsCertList = tsCertPath.getCertificates();
	X509Certificate[] tsCerts = (X509Certificate[]) tsCertList.toArray(new X509Certificate[0]);

	try {
       	    tsCerts = v.validate(tsCerts);
	}
	catch (CertificateException ce) {
	    // not in cacerts or something else
	    Trace.msgSecurityPrintln(ce.getMessage());
	    return false;
	}

	// No error happened, return true
	return true;
    }

    private static PKIXParameters doCRLValidation(PKIXParameters params, boolean hasCRL) 
		throws IOException, InvalidAlgorithmParameterException, 
		CRLException, NoSuchAlgorithmException
    {
	// Start CRL validation and load the CRL from system config
	if (crl509 != null) { 
	   Trace.msgSecurityPrintln("trustdecider.check.validation.crl.system.on");
	   System.clearProperty("com.sun.security.enableCRLDP");

	   params.setRevocationEnabled(true);
	   params.addCertStore(java.security.cert.CertStore.getInstance("Collection",
			new CollectionCertStoreParameters(Collections.singletonList(crl509))));
	}
	else {
	   Trace.msgSecurityPrintln("trustdecider.check.validation.crl.system.off");

	   // Enable CRL if this certificate has CRL extension
	   params.setRevocationEnabled(hasCRL);
	   System.setProperty("com.sun.security.enableCRLDP", Boolean.toString(hasCRL));
	}

	return params;
    }

    private static void doOCSPValidation(PKIXParameters params, LinkedHashSet allRootCerts,
		X509Certificate[] certArr, boolean hasOCSP, boolean crlCheckLocal) 
		throws IOException
    {
	X509Certificate ocspCert = null;
	boolean ocspSignerFound = false;

        // enable OCSP
	Security.setProperty("ocsp.enable", Boolean.toString(hasOCSP));

	// Set OCSP response URL from system file
	if (ocspValidConfig)
           Security.setProperty("ocsp.responderURL", ocspURL);

	params.setRevocationEnabled(hasOCSP);

	// User choice to user Signer and URL in config file
	if (ocspValidConfig) { 	
	   Trace.msgSecurityPrintln("trustdecider.check.validation.ocsp.system.on");
	   String ocspCertName = null;

	   // Find OCSP server cert from CAs
	   for (Iterator iterCA = allRootCerts.iterator(); iterCA.hasNext();) {
             ocspCert = (X509Certificate) iterCA.next();
	     ocspCertName = CertUtils.extractSubjectAliasName(ocspCert);
	     if (ocspSigner.equals(ocspCertName)) {
		ocspSignerFound = true;
	        break;
	     }
	   }

	   // Set OCSP server cert
	   if (ocspSignerFound && ocspCert != null) {
	      Security.setProperty("ocsp.responderCertSubjectName", 
			ocspCert.getSubjectX500Principal().getName());
	   }
	}
	else {
	   Trace.msgSecurityPrintln("trustdecider.check.validation.ocsp.system.off");
	}

	// In case OCSP failed, it should failover to CRL
	if (!crlCheckLocal && hasOCSP) {
	   System.setProperty("com.sun.security.enableCRLDP", "true");
	}
    }

    private static CertStatus doOCSPEEValidation (X509Certificate cert, 
		X509Certificate issuerCert, LinkedHashSet allRootCerts, Date date)
		throws IOException, CertPathValidatorException {

	Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.start");

	URI responderURI = null;
	X509Certificate responderCert = issuerCert;
	if (ocspValidConfig) {
	    try {
		responderURI = new URI(ocspURL);
	    } 
	    catch (URISyntaxException use) {
		/* assume no ocsp responder */ 
	        Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.responderURI.no");
	        return CertStatus.GOOD;
	    }

	    // Find OCSP server cert from CAs
	    for (Iterator iterCA = allRootCerts.iterator(); iterCA.hasNext();) {
		X509Certificate ocspCert = (X509Certificate) iterCA.next();
		String ocspCertName = CertUtils.extractSubjectAliasName(ocspCert);
		if (ocspSigner.equals(ocspCertName)) {
		    responderCert = ocspCert;
		    break;
		}
	    }
	} else {
	    responderURI = OCSP.getResponderURI(cert);
	}

	if (responderURI == null) {
	    // no valid ocsp responder specified, return GOOD status
	    Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.responderURI.no");
	    return CertStatus.GOOD;
	}
	else {
	    String resURI = responderURI.toString();
	    Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.responderURI.value", new Object[] {resURI});
	}

	CertStatus ocspStatus = OCSP.check(cert, issuerCert,
			responderURI,responderCert,date).getCertStatus();
	String ocspStr = ocspStatus.name();
	Trace.msgSecurityPrintln("trustdecider.check.ocsp.ee.return.status", new Object[] {ocspStr});

	return ocspStatus;
    }

    private static boolean checkTrustedExtension(X509Certificate cert) 
    {
	Trace.msgSecurityPrintln("trustdecider.check.trustextension.jurisdiction");

	// get subject DN from cert
  	X500Principal certPrincipal = cert.getSubjectX500Principal();

  	// get canonical RFC 2253 String DN
  	String subjectDN = certPrincipal.getName();

  	// iterate over each jurisdiction from jurisdiction file
	Iterator iterAttr = jurisdictionList.iterator();
	while (iterAttr.hasNext()) {
	    // Get String
            String jurisdictionStr = (String) iterAttr.next();

    	    // compare subjectDN to jurisdiction string
    	    if (subjectDN.endsWith(jurisdictionStr)) {
		Trace.msgSecurityPrintln("trustdecider.check.trustextension.jurisdiction.found");
        	return true;
	    }
  	}
  	return false;
    }

    private static boolean isReplacedCA(HashMap trustedSubjects, X509Certificate last) 
    {
	Trace.msgSecurityPrintln("trustdecider.check.replacedCA.start");

	X500Principal issuer = last.getIssuerX500Principal();
	X500Principal subject = last.getSubjectX500Principal();
	if (trustedSubjects.containsKey(issuer) &&
	    isSignatureValid((X509Certificate)trustedSubjects.get(issuer), last)) {
	    Trace.msgSecurityPrintln("trustdecider.check.replacedCA.succeed");
	    return true;
	}

	Trace.msgSecurityPrintln("trustdecider.check.replacedCA.failed");
	return false;
    }

    private static boolean isSignatureValid(X509Certificate iss, X509Certificate sub) {
        try {
            sub.verify(iss.getPublicKey());
        } catch (Exception ex) {
            return false;
        }
        return true;
    }
}
