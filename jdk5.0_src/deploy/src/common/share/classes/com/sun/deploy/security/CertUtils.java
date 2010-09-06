/*
 * @(#)CertUtils.java	1.9 04/06/11 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Set;
import java.util.Iterator;
import java.security.Principal;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.cert.CertificateEncodingException;
import java.security.cert.CertificateExpiredException;
import java.security.cert.CertificateParsingException;
import java.security.cert.CertificateNotYetValidException;
import java.security.KeyStore;
import java.text.MessageFormat;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;
import sun.security.x509.NetscapeCertTypeExtension;
import sun.security.util.DerValue;
import sun.security.util.DerInputStream;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.AccessController;
import java.io.File;

public class CertUtils 
{
    // Certificate stores used for signed applet verification
    //
    private final static String OID_BASIC_CONSTRAINTS = "2.5.29.19";
    private final static String OID_KEY_USAGE = "2.5.29.15";
    private final static String OID_EXTENDED_KEY_USAGE = "2.5.29.37";
    private final static String OID_NETSCAPE_CERT_TYPE = "2.16.840.1.113730.1.1";
    private final static String OID_EKU_ANY_USAGE = "2.5.29.37.0";
    private final static String OID_EKU_CODE_SIGNING = "1.3.6.1.5.5.7.3.3";
    private final static String OID_EKU_SERVER_AUTH =  "1.3.6.1.5.5.7.3.1";
    private final static String OID_EKU_CLIENT_AUTH =  "1.3.6.1.5.5.7.3.2";
    private final static String OID_EKU_TIME_STAMPING = "1.3.6.1.5.5.7.3.8";

    // Netscape certificate type extension
    private final static String NSCT_OBJECT_SIGNING_CA = NetscapeCertTypeExtension.OBJECT_SIGNING_CA;
    private final static String NSCT_OBJECT_SIGNING = NetscapeCertTypeExtension.OBJECT_SIGNING;
    private final static String NSCT_SSL_CA = NetscapeCertTypeExtension.SSL_CA;
    private final static String NSCT_S_MIME_CA = NetscapeCertTypeExtension.S_MIME_CA;
    private final static String NSCT_S_MIME    = NetscapeCertTypeExtension.S_MIME;
    private final static String NSCT_SSL_CLIENT = NetscapeCertTypeExtension.SSL_CLIENT;
    private final static String NSCT_SSL_SERVER = NetscapeCertTypeExtension.SSL_SERVER;

    public static KeyStore createEmptyKeyStore() {
	KeyStore ks = null;
	try {
	    ks = KeyStore.getInstance("JKS");
	    ks.load(null, null);
	} catch (Exception e) {
	    Trace.ignoredException(e);
	}
	return ks;
    }


    public static void checkUsageForCodeSigning(X509Certificate currentCert, int index)
	throws CertificateException, IOException
    {
	checkUsageForCodeSigning(currentCert, index, false);
    }

    public static void checkUsageForCodeSigning(X509Certificate currentCert, int index, boolean checkTimeStamping)
	               throws CertificateException, IOException
    {
	String msg = null;

	Set critSet = currentCert.getCriticalExtensionOIDs();
	
	if (critSet == null)
	   critSet = Collections.EMPTY_SET;

	// Check for the basic constraints extension
	if (CertUtils.checkBasicConstraintsForCodeSigning(currentCert, critSet, index) == false)
	{
	   Trace.msgSecurityPrintln("trustdecider.check.basicconstraints");
	   msg = ResourceManager.getMessage("trustdecider.check.basicconstraints");
	   throw new CertificateException(msg);
	}

	// Check for the key usage extension
	if (index == 0)
	{
	   if (CertUtils.checkLeafKeyUsageForCodeSigning(currentCert, critSet, checkTimeStamping) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage");
	      msg = ResourceManager.getMessage("trustdecider.check.leafkeyusage");
	      throw new CertificateException(msg);
	   }
	}
	else
	{
	   if (CertUtils.checkSignerKeyUsage(currentCert, critSet) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.signerkeyusage");
	      msg = ResourceManager.getMessage("trustdecider.check.signerkeyusage");
	      throw new CertificateException(msg);
	   }
	}

	// Certificate contains unknown critical extensions
	if (!critSet.isEmpty())
	{
	   Trace.msgSecurityPrintln("trustdecider.check.extensions");
	   msg = ResourceManager.getMessage("trustdecider.check.extensions");
	   throw new CertificateException(msg);
	}
    }

    /*
     * Check the Basic Constraints
     * If failed, we return false.
     */
    private static boolean checkBasicConstraintsForCodeSigning(X509Certificate cert, Set critSet, int index)
		          throws CertificateException, IOException
    {
	critSet.remove(OID_BASIC_CONSTRAINTS);
	critSet.remove(OID_NETSCAPE_CERT_TYPE);

	// No basic constraints check for user cert
	if (index == 0)
	   return true;

	// CA certificate does not include basic constraints extension
        if (cert.getExtensionValue(OID_BASIC_CONSTRAINTS) == null)
        {
	   // Check Netscape certificate type extension
           if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
           {
              if (getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == false)
	      {
		 Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.certtypebit");
                 return false;
	      }
           }
	   else
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.extensionvalue");
	      return false;
	   }
        }
        else
        {
	   // Check Netscape certificate type extension
           if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
           {
	      // Require either all of bits 5,6,7 are false or 
	      // that at least bit 7 be true
              if ((getNetscapeCertTypeBit(cert, NSCT_SSL_CA) != false || 
                  getNetscapeCertTypeBit(cert, NSCT_S_MIME_CA) != false ||
                  getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) != false) && 
                  getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING_CA) == false)
	      {
	         Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.bitvalue");
		 return false;
	      }
	   }

           int constraints = cert.getBasicConstraints();
	   // End user tried to act as a CA
           if (constraints < 0)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.enduser");
              return false;
	   }

           // Violated path length constraints
           if ( (index-1) > constraints)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.basicconstraints.pathlength");
              return false;
	   }
        }

	return true;
    }
    
    /*
     * Verify the key usage and extended key usage for intermediate
     * certificates.
     */
    private static boolean checkLeafKeyUsageForCodeSigning(X509Certificate cert, Set critSet, boolean checkTimeStamping)
            throws CertificateException, IOException 
    { 
	critSet.remove(OID_KEY_USAGE);

        // check key usage extension
        boolean[] keyUsageInfo = cert.getKeyUsage();

        if (keyUsageInfo != null) 
	{
	   // Invalid key usage extension
	   if (keyUsageInfo.length == 0)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.length");
	      return false;
	   }

	   // keyUsageInfo[0] is for digitalSignature
	   // require digitalSignature to be set
	   boolean digitalSignature = keyUsageInfo[0];
	   if (!digitalSignature)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.digitalsignature");
	      return false;
	   }
	}

	// Check extened key usage extension only if critical
	// (interoperability problems with some certificates)
	List extKeyUsageInfo = X509Util.getExtendedKeyUsage(cert);
	Set nonCritSet = cert.getNonCriticalExtensionOIDs();

	if ((extKeyUsageInfo != null) && (critSet.contains(OID_EXTENDED_KEY_USAGE)
				         || nonCritSet.contains(OID_EXTENDED_KEY_USAGE)))
	{
	   critSet.remove(OID_EXTENDED_KEY_USAGE);

	   if (checkTimeStamping)
	       {
		   // For timestamping TSA checking
		   if (extKeyUsageInfo.contains(OID_EKU_ANY_USAGE) == false && 
		       extKeyUsageInfo.contains(OID_EKU_TIME_STAMPING) == false)
		       {
			   Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.tsaextkeyusageinfo");
			   return false;
		       }
	       }
	   else
	       {
		   if (extKeyUsageInfo.contains(OID_EKU_ANY_USAGE) == false && 
		       extKeyUsageInfo.contains(OID_EKU_CODE_SIGNING) == false)
		       {
			   Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.extkeyusageinfo");
			   return false;
		       }
	       }
	}

	// Check Netscape certificate type extension
        if (cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE) != null)
        {
           if (getNetscapeCertTypeBit(cert, NSCT_OBJECT_SIGNING) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.leafkeyusage.certtypebit");
              return false;
	   }
        }

	return true;
    }

    /*
     * Verify the key usage and extended key usage for intermediate
     * certificates.
     */
    private static boolean checkSignerKeyUsage(X509Certificate cert, Set critSet)
            throws CertificateException, IOException 
    { 
	critSet.remove(OID_KEY_USAGE);

        // check key usage extension
        boolean[] keyUsageInfo = cert.getKeyUsage();
        if (keyUsageInfo != null) 
	{
	   // keyUsageInfo[5] is for keyCertSign.
	   if ((keyUsageInfo.length < 6) || (keyUsageInfo[5] == false))
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.signerkeyusage.lengthandbit");
	      return false;
	   }
	}

	// Check extened key usage extension only if critical
	// (interoperability problems with some certificates)
	List extKeyUsageInfo = X509Util.getExtendedKeyUsage(cert);
	Set nonCritSet = cert.getNonCriticalExtensionOIDs();
	if ((extKeyUsageInfo != null) && (critSet.contains(OID_EXTENDED_KEY_USAGE) 
					 || nonCritSet.contains(OID_EXTENDED_KEY_USAGE)))
	{
	   critSet.remove(OID_EXTENDED_KEY_USAGE);
	   if (extKeyUsageInfo.contains(OID_EKU_ANY_USAGE) == false &&
	       extKeyUsageInfo.contains(OID_EKU_CODE_SIGNING) == false)
	   {
	      Trace.msgSecurityPrintln("trustdecider.check.signerkeyusage.keyusage");
	      return false;
	   }
	}

	return true;
    }

    /*
     * Get the value of the specified bit in the Netscape certificate type
     * extension. If the extension is not present at all, we return false.
     */
    private static boolean getNetscapeCertTypeBit(X509Certificate cert, String type)
            throws CertificateException, IOException
    {
        byte[] extVal = cert.getExtensionValue(OID_NETSCAPE_CERT_TYPE);
        if (extVal == null)
            return false;

        DerInputStream in = new DerInputStream(extVal);
        byte[] encoded = in.getOctetString();
        encoded = new DerValue(encoded).getUnalignedBitString().toByteArray();

        NetscapeCertTypeExtension extn = new NetscapeCertTypeExtension(encoded);

        Boolean val = (Boolean)extn.get(type);
        return val.booleanValue();
    }

    /*
     * Returns true iff the issuer of <code>cert1</code> corresponds to the
     * subject (owner) of <code>cert2</code>.
     *
     * @return true iff the issuer of <code>cert1</code> corresponds to the
     * subject (owner) of <code>cert2</code>, false otherwise.
     */
    public static boolean isIssuerOf(X509Certificate cert1,
				     X509Certificate cert2)
    {
	Principal issuer = cert1.getIssuerDN();
	Principal subject = cert2.getSubjectDN();
	if (issuer.equals(subject))
	    return true;
	return false;
    }
    
    
    /** 
     * Extract from quote
     */
    private static String extractFromQuote(String s, String prefix)
    {
	if ( s == null)
	    return null;

	// Search for issuer name
	//
	int x = s.indexOf(prefix);
	int y = 0;

	if (x >= 0)
	{
	    x = x + prefix.length();

	    // Search for quote
	    if (s.charAt(x) == '\"')
	    {
		// if quote is found, search another quote
		// skip the first quote
		x = x + 1;
		y = s.indexOf('\"', x);
	    }
	    else // quote is not found, search for comma
		y = s.indexOf(',', x);

	    if (y < 0)
		return s.substring(x);			
	    else
		return s.substring(x, y);			
	}
	else // No match
	    return null;
    }

    /**
     * Extrace CN from DN in the certificate.
     *
     * @param cert X509 certificate
     * @return CN
     */
    public static String extractSubjectAliasName(X509Certificate cert)
    {
	String subjectName = ResourceManager.getMessage("config.unknownSubject");

	// Extract CN from the DN for each certificate
	try 
	{
	    Principal principal = cert.getSubjectDN();

	    // Extract subject name
	    String subjectDNName = principal.getName();

	    // Extract subject name
	    subjectName = extractFromQuote(subjectDNName, "CN=");

	    // If no 'CN=' attribute available, we try 'O=' and 'OU'
	    if (subjectName == null)
	    {
		String subOName = extractFromQuote(subjectDNName, "O=");
		String subOUName = extractFromQuote(subjectDNName, "OU=");

		if (subOName!=null || subOUName!=null)
		{
		    MessageFormat mfSubject = new MessageFormat(ResourceManager.getMessage("config.certShowOOU"));
		    Object[] args = {subOName, subOUName};

		    if (subOName == null)
			args[0] = "";

		    if (subOUName == null)
			args[1] = "";

		    subjectName = mfSubject.format(args);
		}
	    }

	    if (subjectName == null)
		subjectName = ResourceManager.getMessage("config.unknownSubject");
	}
	catch (Exception e) 
	{
	    //Trace.printException(e);
	}

	return subjectName;
    }

    /**
     * Extrace CN from DN in the certificate.
     *
     * @param cert X509 certificate
     * @return CN
     */
    public static String extractIssuerAliasName(X509Certificate cert)
    {
	String issuerName = ResourceManager.getMessage("config.unknownIssuer");

	// Extract CN from the DN for each certificate
	try 
	{
	    Principal principalIssuer = cert.getIssuerDN();

	    // Extract subject name
	    String issuerDNName = principalIssuer.getName();

	    // Extract issuer name
	    issuerName = extractFromQuote(issuerDNName, "CN=");

	    // If no 'CN=' attribute available, we try 'O=' and 'OU'
	    if (issuerName == null)
	    {
		String issOName = extractFromQuote(issuerDNName, "O=");
		String issOUName = extractFromQuote(issuerDNName, "OU=");

		if (issOName!=null || issOUName!=null)
		{
		    MessageFormat mfIssuer= new MessageFormat(ResourceManager.getMessage("config.certShowOOU"));
		    Object[] args = {issOName, issOUName};

		    if (issOName == null)
			args[0] = "";

		    if (issOUName == null)
			args[1] = "";

		    issuerName = mfIssuer.format(args);
		}
	    }

	    if (issuerName == null)
		issuerName = ResourceManager.getMessage("config.unknownIssuer");
	}
	catch (Exception e) 
	{
	    //Trace.printException(e);
	}

	return issuerName;
    }   

    static long getFileLastModified(final String filename) {
	long lastModified = 0;
	try {
	    lastModified = ((Long) AccessController.doPrivileged(new PrivilegedExceptionAction() {    
		    public Object run() throws SecurityException {
			return new Long(new File(filename).lastModified());
		    }
		})).longValue();
	} catch (PrivilegedActionException e) {		
	    Trace.securityPrintException(e);
	}
	return lastModified;
    }
}
