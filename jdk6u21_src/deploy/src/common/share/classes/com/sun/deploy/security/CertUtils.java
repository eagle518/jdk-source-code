/*
 * @(#)CertUtils.java	1.18 10/05/20 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

import java.io.IOException;
import java.util.Collections;
import java.util.Date;
import java.util.List;
import java.util.Set;
import java.util.Iterator;
import java.util.Collection;
import java.util.ArrayList;
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
import sun.security.x509.CRLDistributionPointsExtension;
import sun.security.x509.AuthorityInfoAccessExtension;
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
    private final static String OID_CRL = "2.5.29.31";
    private final static String OID_AIA = "1.3.6.1.5.5.7.1.1";

    // Netscape certificate type extension
    private final static String NSCT_OBJECT_SIGNING_CA = NetscapeCertTypeExtension.OBJECT_SIGNING_CA;
    private final static String NSCT_OBJECT_SIGNING = NetscapeCertTypeExtension.OBJECT_SIGNING;
    private final static String NSCT_SSL_CA = NetscapeCertTypeExtension.SSL_CA;
    private final static String NSCT_S_MIME_CA = NetscapeCertTypeExtension.S_MIME_CA;
    private final static String NSCT_S_MIME    = NetscapeCertTypeExtension.S_MIME;
    private final static String NSCT_SSL_CLIENT = NetscapeCertTypeExtension.SSL_CLIENT;
    private final static String NSCT_SSL_SERVER = NetscapeCertTypeExtension.SSL_SERVER;

    // bit numbers in the key usage extension
    private final static int KU_SIGNATURE = 0;

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
    static boolean getNetscapeCertTypeBit(X509Certificate cert, String type)
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

    /**
     * Utility method checking if bit 'bit' is set in this certificates
     * key usage extension.
     */
    private static boolean checkKeyUsage(X509Certificate cert, int bit) {
        boolean[] keyUsage = cert.getKeyUsage();
        if (keyUsage == null) {
            return true;
        }
        return (keyUsage.length > bit) && keyUsage[bit];
    }

    /**
     * Utility method checking if the extended key usage extension in
     * certificate cert allows use for expectedEKU.
     */
    private static boolean checkEKU(X509Certificate cert, String expectedEKU) 
			throws CertificateException {
	List eku = cert.getExtendedKeyUsage();
	if (eku == null) {
	    return true;
	}
	return eku.contains(expectedEKU) || eku.contains(OID_EKU_ANY_USAGE);
    }

    /**
     * Check whether this certificate can be used for TLS client
     * authentication.
     */
    static boolean checkTLSClient(X509Certificate cert)
			throws CertificateException {

        if (checkKeyUsage(cert, KU_SIGNATURE) == false) {
	    Trace.msgSecurityPrintln("clientauth.checkTLSClient.checkKeyUsage");
	    return false;
        }

        if (checkEKU(cert, OID_EKU_CLIENT_AUTH) == false) {
	    Trace.msgSecurityPrintln("clientauth.checkTLSClient.checkEKU");
	    return false;
        }

	return true;
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

    /*
     * Get the value of CRL extension. If the extension is not present at all, 
     * we return false.
     */
    static boolean getCertCRLExtension(X509Certificate cert)
            throws IOException
    {
	byte[] crlValue = cert.getExtensionValue(OID_CRL);

	if (crlValue == null) {
	   Trace.msgSecurityPrintln("trustdecider.check.validation.crl.notfound");
           return false;
  	}

	// remove outer OCTET STRING wrapper
	if (crlValue[0] == DerValue.tag_OctetString) {
	   crlValue = new DerValue(crlValue).getOctetString();
	}
	
	Trace.msgSecurityPrintln(extractSubjectAliasName(cert));
	CRLDistributionPointsExtension crl =
                new CRLDistributionPointsExtension(new Boolean(false), crlValue);
	Trace.msgSecurityPrintln(crl.toString());
	return true;
    }

    /*
     * Get the value of OCSP (AIA). If AIA is not present at all, 
     * we return false.
     */
    static boolean getCertAIAExtension(X509Certificate cert)
            throws IOException
    {
	byte[] aiaValue = cert.getExtensionValue(OID_AIA);

	if (aiaValue == null) {
	   Trace.msgSecurityPrintln("trustdecider.check.validation.ocsp.notfound");
           return false;
  	}

	// remove outer OCTET STRING wrapper
	if (aiaValue[0] == DerValue.tag_OctetString) {
	   aiaValue = new DerValue(aiaValue).getOctetString();
	}
	
	Trace.msgSecurityPrintln(extractSubjectAliasName(cert));
	AuthorityInfoAccessExtension aia =
                new AuthorityInfoAccessExtension(new Boolean(false), aiaValue);
	Trace.msgSecurityPrintln(aia.toString());
	return true;
    }

    /**
     * Check if domain name matches certificate subject name.
     *
     * @param String hostname 
     * @param ArrayList subject name list
     * @return true if matches 
     */
    static boolean checkWildcardDomainList(String hostname, ArrayList subjectNameList)
    {
	for (int i=0; i<subjectNameList.size(); i++) {
	    String subjectName = (String)subjectNameList.get(i);
	    if (checkWildcardDomain(hostname, subjectName)) {
		return true;
	    }
	}
	return false;
    }

    /**
     * Check if domain name matches certificate subject name.
     * The "*" character may be used as the lef-most name component
     * of the server's common name.
     * example: *.yoursite.com matches www.yoursite.com
     * example: w*.yoursite.com matches www.yoursite.com
     *
     * @param String hostname 
     * @param String subject name
     * @return true if matches 
     */
    private static boolean checkWildcardDomain(String hostname, String subjectName)
    {
	if (hostname == null || hostname.length() == 0 ||
	    subjectName == null || subjectName.length() == 0) {
	   return false;
	}

	// Check if hostname and subjectName are exact match.
	subjectName = subjectName.trim();
	hostname = hostname.trim();
	if (subjectName.equalsIgnoreCase(hostname)) {
	    return true;
	}

	// Now we have to check wildcard string in certificate
	int starIdx = subjectName.indexOf("*.");	
	if (starIdx == -1) {
	   return false;
	}

	// Now we do have *. in string, check right-most name component
	String rightSubjectName = subjectName.substring(starIdx+1);
	String leftSubjectName = subjectName.substring(0, starIdx);

	return (!leftSubjectName.contains(".") &&
		hostname.indexOf(leftSubjectName) == 0 &&
	    	hostname.endsWith(rightSubjectName) &&
	    	hostname.length() >= subjectName.length());
    }

    /*
     * Extract the name of the SSL server from the certificate.
     */
    public static ArrayList getServername(X509Certificate peerCert) {
	ArrayList servernameList = new ArrayList();
        try {
            // compare to subjectAltNames if dnsName is present
            Collection subjAltNames = peerCert.getSubjectAlternativeNames();
            if (subjAltNames != null) {
                for (Iterator itr = subjAltNames.iterator(); itr.hasNext(); ) {
                    List next = (List)itr.next();
                    if (((Integer)next.get(0)).intValue() == 2) {
                        // add dNSName with host in url
                        String dnsName = ((String)next.get(1));
			servernameList.add(dnsName);
                    }
                }
		if (servernameList.size() > 0) {
		    return servernameList;
		}	
            }
	} catch (NoSuchMethodError nsme) {
            // ignore
        } catch (java.security.cert.CertificateException e) {
            // ignore
        }

        // else check against common name in the subject field
        String certHostname = extractSubjectAliasName(peerCert);
	servernameList.add(certHostname);
        return servernameList;
    }

}

