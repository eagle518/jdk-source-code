/*
 * @(#)CertificateHostnameVerifier.java	1.24 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLPeerUnverifiedException;
import java.util.HashSet;
import java.util.Iterator;
import java.util.ArrayList;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.text.MessageFormat;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.Trace;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.AppInfo;

/**
 * CertificateHostnameVerifier is a callback mechanism so that
 * we may supply a policy for handling the case 
 * where the host to connect to and the server name 
 * from the certificate mismatch.
 */

public final class CertificateHostnameVerifier 
		implements javax.net.ssl.HostnameVerifier {

    // HashSet to store hostname/certHostname pair
    private static HashSet hashSet = new HashSet();

    public CertificateHostnameVerifier()
    {
	// no-op
    }

    /**
     * Verify that the hostname is an acceptable match with the 
     * server's received certificate(s).
     *
     * @param hostname the host name
     * @param session SSLSession used on the connection to host
     * @return true if the certificate host name is acceptable
     */
    public boolean verify(String hostname, SSLSession session)
    {
	// By default set the host name as unknown
	String certHostname = 
	    ResourceManager.getMessage("https.dialog.unknown.host");

        X509Certificate peerCert;
	try {
            // get the subject's certificate
            Certificate [] cert = session.getPeerCertificates();

            if (cert[0] instanceof java.security.cert.X509Certificate)
                peerCert = (java.security.cert.X509Certificate) cert[0];
            else
                throw new SSLPeerUnverifiedException("");

	    // Obtain HTTPS server name
	    String certDN = peerCert.getSubjectDN().getName();
	    if(certDN != null) {
	        int cn = certDN.toUpperCase().indexOf("CN=");
	        if(cn != -1) {
		    int end = certDN.indexOf(",", cn);
		    if (end != -1) {
		        certHostname = certDN.substring(cn+3, end);
		    } else {
		        certHostname = certDN.substring(cn+3);
		    }
	       }
	    }
	} catch (SSLPeerUnverifiedException e) {
	    return false;
	}

	// Get subject name list from alternative extension as well
	ArrayList certHostnameList = CertUtils.getServername(peerCert);

	if (CertUtils.checkWildcardDomainList(hostname, certHostnameList)) {
	    return true;
	}
	    
	// Check if we have seen this pair before
	//	    
	for (Iterator iter = hashSet.iterator(); iter.hasNext();)
	{
	    Object[] elements = (Object[]) iter.next();

	    if (elements[0].toString().equalsIgnoreCase(hostname)
		&& elements[1].toString().equalsIgnoreCase(certHostname))
		return true;
	}

	// No, we haven't seen it, so popup dialog
	return showHostnameMismatchDialog(hostname, certHostname);
    }

    /**
     * Ask the user to see if the hostname is an acceptable match 
     * with the value from the common name entry in the server 
     * certificate's distinguished name.
     *
     * @param hostname the host name
     * @param certHostname the common name entry from the certificate
     * @return true if the certificate host name is acceptable
     */
    private boolean showHostnameMismatchDialog(String hostname, 
					       String certHostname) {

	String title = ResourceManager.getMessage("https.dialog.caption");
        String message = ResourceManager.getMessage("https.dialog.masthead");
        
        // Create AppInfo with TYPE_UNKNOWN, since we'll use this for both
        // plugin and webstart.  Set "publisher" to certHostname and "Name"
        // to hostname.  Cannot set "From" because it needs to be URL, while
        // in some cases we'll have "Unknown host" for hostname value.
        AppInfo info = new AppInfo(AppInfo.TYPE_UNKNOWN, hostname, 
                certHostname, null, null, null, false, false,  null, null);
        
        // approve action button should have "Run" string.
        String okString = ResourceManager.getMessage(
                "security.dialog.signed.buttonContinue");                
        
        // cancel action button should have "Cancel" string.
        String cancelString = ResourceManager.getMessage(
                "security.dialog.signed.buttonCancel");
	int result = UIFactory.ERROR;
        MessageFormat mf = new MessageFormat(ResourceManager
                .getMessage("security.dialog.hostname.mismatch.sub"));
        Object[] args = {hostname, certHostname};
	String [] warning = {mf.format(args)};
	// Check if automation is enabled
	if (Trace.isAutomationEnabled() == false){
	    if (Config.getBooleanProperty(Config.SEC_JSSE_HOST_WARN_KEY)) {
		result = UIFactory.showSecurityDialog(
                        info, title, message, certHostname, null, false,
                        false, okString, cancelString, warning, null,
                        false, null, -1, -1, false );
            } else {
		result = UIFactory.OK;
	    }
	} else {
	    Trace.msgSecurityPrintln(
                    "hostnameverifier.automation.ignoremismatch");
	    result = UIFactory.OK;
	}

	// Store result
	if (result == UIFactory.OK)
	{   
	    Object[] elements = new Object[2];
	    elements[0] = hostname;
	    elements[1] = certHostname;

	    hashSet.add(elements);
	}

	return (result == UIFactory.OK);
    }

    /** 
     * Reset certificate hostname verifier cache
     */
    public static void reset()
    {
	hashSet.clear();
    }
}
