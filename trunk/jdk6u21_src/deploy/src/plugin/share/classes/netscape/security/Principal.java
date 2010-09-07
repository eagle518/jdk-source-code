/*
 * @(#)Principal.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

import java.net.URL;


/**
 * All access control decisions boil down to who is allowed to do what. 
 * The Principal class represents the who, and the Target class represents 
 * the what. In this system, principals may be cryptographic digital 
 * signatures (actually, the certificates behind the signatures), or they 
 * may be URL codebases. 
 *
 * In the cryptographic case, both full certificates and their smaller 
 * fingerprints are supported, and the equals() method will recompute the 
 * fingerprint, if necessary, to make its comparison. 
 * 
 * In the codebase case, regular expressions are planned to be supported, 
 * but are not presently available. 
 *  
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public final class Principal 
{
    /** 
     * a type of Principal - an exact URL codebase 
     */
    public final static int CODEBASE_EXACT = 0x0001;

    /** 
     * a type of Principal - a regular expression which may match many 
     * odebases. This is reserved for a future version of the security 
     * system and is not supported currently. 
     */
    public final static int CODEBASE_REGEXP = 0x0002;

    /** 
     * a type of Principal - a cryptographic certificate (in general, 
     * the public key of a signer, as signed by a certificate authority) 
     */
    public final static int CERT = 0x0003;

    /**
     * a type of Principal - a secure hash of a certificate 
     */
    public final static int CERT_FINGERPRINT = 0x0004;

    /** 
     * a certificate key into Certificate database (for internal use 
     * only) 
     */
    public final static int CERT_KEY = 0x0005;


    private int type = CERT;
    private URL url = null;

    /** 
     * This constructor is only used by the Persistent store. Not 
     * guaranteed to be supported in future versions. 
     */
    public Principal()
    {
    }

    /** 
     * This constructor makes a CODEBASE_EXACT principal from the URL 
     */
    public Principal(URL url)
    {
	this.url = url;
    }

    /**
     * This constructor allows you to specify a principal based on a 
     * human-readable string. This is the typical way CODEBASE Principals 
     * are created. CERT_FINGERPRINT's can also be made this way, using an 
     * ASCII coded representation like so: ##:##:##, where each ## 
     * represents one byte (two hex digits). 
     *
     * @param type One of: CODEBASE_EXACT, CODEBASE_REGEXP, CERT, or 
     *	      CERT_FINGERPRINT 
     * @param value String representation of the principal. 
     */
    public Principal(int type,
		     String value)
    {
	this.type = type;
    }

    /** 
     * This constructor allows you to specify CERT and CERT_FINGERPRINT 
     * principals using a more compact input representation. For CODEBASE 
     * principals, the value parameter (an array of bytes) is treated as an 
     * ASCII string. 
     *
     * @param type One of: CODEBASE_EXACT, CODEBASE_REGEXP, CERT, or 
     *	      CERT_FINGERPRINT 
     * @param value Binary (byte-array) representation of the principal. 
     */
    public Principal(int type,
                     byte value[])
    {
	this.type = type;
    }

    /** 
     * This is for internal use only
     */ 
    public Principal(int type,
		     byte value[],
                     Class cl)
    {
	this.type = type;
    }

    /** 
     * @return true if the type of this Principal is CODEBASE_EXACT or 
     *	       CODEBASE_REGEXP 
     */
    public boolean isCodebase()
    {
	return (isCodebaseExact() || isCodebaseRegexp());
    }

    /**
     * @return true if the type of this Principal is CODEBASE_EXACT 
     */
    public boolean isCodebaseExact()
    {
	return (type == CODEBASE_EXACT);
    }

    /**
     * @return true if the type of this Principal is CODEBASE_REGEXP 
     */
    public boolean isCodebaseRegexp()
    {
	return (type == CODEBASE_REGEXP);
    }


    /** 
     * @return if the type of this Principal is CERT
     */
    public boolean isCert()
    {
	return (type == CERT);
    }

    /**
     * @return if the type of this Principal is CERT_FINGERPRINT
     */
    public boolean isCertFingerprint()
    {
	return (type == CERT_FINGERPRINT);
    }

    /** 
     * This methods prepends the type of the Principal to its minimal 
     * string representation (see toString()). This is useful to print 
     * while debugging your code. 
     * 
     * @return a String repesentation of the Principal
     */
    public String toVerboseString()
    {
	return toString();
    }

    /** 
     * @return A string with the proper name from the certificate (e.g.: 
     *	       "Netscape Communications Corporation") or the DNS name from 
     *	       a codebase (e.g.: "home.netscape.com"). 
     */
    public String getVendor()
    {
	return null;
    }

    /** 
     * This is the most verbose Principal to String converter. The output is 
     * intended for presentation to a user and will be appropriately presented 
     * in the user's language (if supported). 
     *
     * @return an HTML table, formatted in a style suitable for a Web 
     *	       browser's "Document Info" window. 
     */
    public String toVerboseHtml()
    {
	return null;
    }

    public String getNickname()
    {
	return null;
    }

    public boolean isSystemPrincipal()
    {
	return false;
    }

    public static int getZigPtr(Class cl)
    {
	return -1;
    }
}
