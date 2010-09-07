/*
 * @(#)X509Util.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.ui.AppInfo;
import java.io.IOException;
import java.net.URL;
import java.util.Vector;
import java.util.ArrayList;
import java.security.Principal;
import java.security.PublicKey;
import java.security.KeyStore;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateParsingException;
import java.security.cert.CertificateNotYetValidException;
import sun.security.util.DerValue;
import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;
import sun.security.util.ObjectIdentifier;
import java.util.List;
import java.util.Date;


class X509Util {

    private final static String OID_EXTENDED_KEY_USAGE = "2.5.29.37"; 

    static Principal getIssuerPrincipal(Certificate cert) {
	try {
	    return ((X509Certificate)cert).getIssuerX500Principal();
	} catch (NoSuchMethodError nsme) {
	    Trace.println("NoSuchMethodError: X509Certificate.getIssuerX500Principal(): "+nsme, TraceLevel.SECURITY);
	    return ((X509Certificate)cert).getIssuerDN();
	}
    }


    static Principal getSubjectPrincipal(Certificate cert) {
	try {
	    return ((X509Certificate)cert).getSubjectX500Principal();
	} catch (NoSuchMethodError nsme) {
	    Trace.println("NoSuchMethodError: X509Certificate.getSubjectX500Principal(): "+nsme, TraceLevel.SECURITY);
	    return ((X509Certificate)cert).getSubjectDN();
	}
    }

    static List getExtendedKeyUsage(Certificate cert) throws 
		java.security.cert.CertificateParsingException {
	try {
	    return ((X509Certificate)cert).getExtendedKeyUsage();
	} catch (NoSuchMethodError nsme) {
	    Trace.println("NoSuchMethodError: X509Certificate.getExtendedKeyUsage(): "+nsme, TraceLevel.SECURITY);
	    try {
		return oldGetExtendedKeyUsage((X509Certificate)cert);
	    } catch (IOException ioe) {
		Trace.ignoredException(ioe);
		String str = ioe.toString();
		throw new java.security.cert.CertificateParsingException(str);
	    }
	    catch (CertificateException ce) {
		Trace.ignoredException(ce);
		String str = ce.toString();
		throw new java.security.cert.CertificateParsingException(str);
	    }
	}
    }

    /*
     * oldGetExtendedKeyUsage - must be inline since only in 1.4+
     */
    private static List oldGetExtendedKeyUsage(X509Certificate cert)
            throws CertificateException, IOException {

        byte[] extVal = cert.getExtensionValue(OID_EXTENDED_KEY_USAGE);
        if (extVal == null) {
           return null;
        }
 
        DerInputStream in = new DerInputStream(extVal);
        byte[] encoded = in.getOctetString();
        DerValue val = new DerValue(encoded);
  
        Vector keyUsages = new Vector(1, 1);
        while (val.data.available() != 0) {
             DerValue seq = val.data.getDerValue();
             ObjectIdentifier usage = seq.getOID();
             keyUsages.addElement(usage);
        }
 
        ArrayList al = new ArrayList(keyUsages.size());
        for (int i = 0; i < keyUsages.size(); i++) {
            al.add(keyUsages.elementAt(i).toString());
        }
        return al;
    }

    /**
     * <P> Display the security dialog about the certificate, and ask the user
     * for granting permission.
     * </P>
     *
     * @return 0 if "Yes" option is selected from the security dialog.
     * @return 1 if "No" option is selected from the security dialog.
     * @return 2 if "Always" option is selected from the security dialog.
     */
    static int showSecurityDialog(final Certificate[] certs, 
                                final URL codeLocation,
                                final int start, final int end, 
				final boolean rootCANotValid, 
                                final int certValidity, 
				final Date timeStamp, final AppInfo ainfo)
                                throws CertificateException
    {
        int ret = showSecurityDialog(certs, codeLocation,
                        start, end, rootCANotValid, certValidity, 
                        timeStamp, ainfo, false);
        return ret;
    }

    static int showSecurityDialog(final Certificate[] certs, 
                                final URL codeLocation,
                                final int start, final int end, 
				final boolean rootCANotValid, 
                                final int certValidity, 
				final Date timeStamp, final AppInfo ainfo, 
				final boolean jnlpFlag)
                                throws CertificateException
    {
        int ret = TrustDeciderDialog.showDialog(certs, codeLocation,
                        start, end, rootCANotValid, certValidity, 
                        timeStamp, ainfo, false, null, jnlpFlag);
        return ret;
    }

}


