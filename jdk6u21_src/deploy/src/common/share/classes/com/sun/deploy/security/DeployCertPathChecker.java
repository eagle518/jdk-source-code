/*
 * @(#)DeployCertPathChecker.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.util.Collection;
import java.util.Collections;
import java.util.Set;
import java.security.cert.PKIXCertPathChecker;
import java.security.cert.CertPathValidatorException;
import java.security.cert.CertificateException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import sun.security.x509.NetscapeCertTypeExtension;
import sun.security.validator.PKIXValidator;


/**
 * This class extends from PKIXCertPathChecker for "PKIX" algorithm to check
 * certificate validation for signed jar file.
 * This is for special Netscape_cert_type extension check for Java deployment only
 *
 * @version 1.0
 * @author Dennis Gu
 */
final class DeployCertPathChecker extends PKIXCertPathChecker {

     // Cert length and validator
     private int remainingCerts;
     private PKIXValidator pv;

     // Certificate stores used for signed applet verification
     private final static String OID_BASIC_CONSTRAINTS = "2.5.29.19";
     private final static String OID_NETSCAPE_CERT_TYPE = "2.16.840.1.113730.1.1";
     private final static Set extSet = Collections.singleton(OID_NETSCAPE_CERT_TYPE);

     // Netscape certificate type extension
     private final static String NSCT_OBJECT_SIGNING_CA = NetscapeCertTypeExtension.OBJECT_SIGNING_CA;
     private final static String NSCT_SSL_CA = NetscapeCertTypeExtension.SSL_CA;
     private final static String NSCT_S_MIME_CA = NetscapeCertTypeExtension.S_MIME_CA;

     DeployCertPathChecker(PKIXValidator pv)
     {
	this.pv = pv;
     }

     public void check(Certificate cert, Collection unresCritExts) 
		throws CertPathValidatorException {

     	X509Certificate xcert = (X509Certificate) cert;
	String msg = null;

        if (unresCritExts != null && !unresCritExts.isEmpty())
	{
           unresCritExts.remove(OID_NETSCAPE_CERT_TYPE);
        }

	// No basic constraints check for user cert
	remainingCerts--;
	if (remainingCerts == 0)
	   return;

	try {
	  // CA certificate does not include basic constraints extension
	  if (xcert.getExtensionValue(OID_BASIC_CONSTRAINTS) == null)
	  {
	    // Check Netscape certificate type extension
            if (xcert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
	    {
              if (CertUtils.getNetscapeCertTypeBit(xcert, NSCT_OBJECT_SIGNING_CA) == false)
	      {
		 Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.certtypebit");
		 msg = ResourceManager.getMessage("trustdecider.check.basicconstraints.certtypebit");
		 throw new CertPathValidatorException(msg);
	      }
	    }
	    else
	    {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.extensionvalue");
	      msg = ResourceManager.getMessage("trustdecider.check.basicconstraints.extensionvalue");
	      throw new CertPathValidatorException(msg);
	    }
	  }
	  else
	  {
	    // Check Netscape certificate type extension
            if (xcert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
            {
              // Require either bits 5,6 are false or that at least bit 7 be true
	      if ((CertUtils.getNetscapeCertTypeBit(xcert, NSCT_SSL_CA) == true ||
                   CertUtils.getNetscapeCertTypeBit(xcert, NSCT_S_MIME_CA) == true) &&
                   CertUtils.getNetscapeCertTypeBit(xcert, NSCT_OBJECT_SIGNING_CA) == false)
              {
                 Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.bitvalue");
	         msg = ResourceManager.getMessage("trustdecider.check.basicconstraints.bitvalue");
	         throw new CertPathValidatorException(msg);
              }
            }
	  }
	} //try
	catch (IOException ioe) {
	   throw new CertPathValidatorException();
	}
	catch (CertificateException ce) {
	   throw new CertPathValidatorException();
	}

	// If we get here, no error
	return;
     }

     public Set getSupportedExtensions() {
        return extSet;
     }

     public boolean isForwardCheckingSupported() {
        return true;
     }

     public void init(boolean forward) throws CertPathValidatorException {
	remainingCerts = pv.getCertPathLength();
     }
}
