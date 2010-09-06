/*
 * @(#)Krb5NameElement.java	1.12 04/04/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
  
package sun.security.jgss.krb5;

import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import javax.security.auth.kerberos.*;
import sun.security.krb5.PrincipalName;
import sun.security.krb5.KrbException;
import sun.security.krb5.ServiceName;
import java.io.UnsupportedEncodingException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.security.Provider;
 
/**
 * Implements the GSSNameSpi for the krb5 mechanism.
 *
 * @author Mayank Upadhyay
 * @version 1.12, 04/01/04
 */
public class Krb5NameElement 
    implements GSSNameSpi {

    private PrincipalName krb5PrincipalName; 

    private String gssNameStr = null;
    private Oid gssNameType = null;

    // TBD: Move this concept into PrincipalName's asn1Encode() sometime
    private static String CHAR_ENCODING = "UTF-8";

    private Krb5NameElement(PrincipalName principalName,
			    String gssNameStr,
			    Oid gssNameType) {
	this.krb5PrincipalName = principalName;
	this.gssNameStr = gssNameStr;
	this.gssNameType = gssNameType;
    }

    /**
     * Instantiates a new Krb5NameElement object. Internally it stores the
     * information provided by the input parameters so that they may later
     * be used for output when a printable representaion of this name is
     * needed in GSS-API format rather than in Kerberos format.
     *
     */
    static Krb5NameElement getInstance(String gssNameStr, Oid gssNameType)
	throws GSSException {

	/*
	 * A null gssNameType implies that the mechanism default
	 * Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL be used. 
	 */
	if (gssNameType == null)
	    gssNameType = Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL;
	else
	    if (!gssNameType.equals(GSSName.NT_USER_NAME) &&
	        !gssNameType.equals(GSSName.NT_HOSTBASED_SERVICE) &&
	        !gssNameType.equals(Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL) &&
	        !gssNameType.equals(GSSName.NT_EXPORT_NAME))
		throw new GSSException(GSSException.BAD_NAMETYPE, -1, 
				       gssNameType.toString()
				       +" is an unsupported nametype");
	
	PrincipalName principalName;
	try {

	    if (gssNameType.equals(GSSName.NT_EXPORT_NAME) ||
		gssNameType.equals(Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL)){
		principalName = new PrincipalName(gssNameStr,
				  PrincipalName.KRB_NT_PRINCIPAL);
	    } else {
		
		String[] components = getComponents(gssNameStr);
		
		/*
		 * We have forms of GSS name strings that can come in:
		 *
		 * 1. names of the form "foo" with just one
		 * component. (This might include a "@" but only in escaped
		 * form like "\@")  
		 * 2. names of the form "foo@bar" with two components
		 *
		 * The nametypes that are accepted are NT_USER_NAME, and
		 * NT_HOSTBASED_SERVICE.
		 */
		
		if (gssNameType.equals(GSSName.NT_USER_NAME))
		    principalName = new PrincipalName(gssNameStr, 
				    PrincipalName.KRB_NT_PRINCIPAL);
		else {
		    String hostName = null;
		    String service = components[0];
		    if (components.length >= 2)
			hostName = components[1];
		    
		    String principal = getHostBasedInstance(service, hostName);
		    principalName = new ServiceName(principal,
					    PrincipalName.KRB_NT_SRV_HST);
		}
	    }
	    
	} catch (KrbException e) {
	    throw new GSSException(GSSException.BAD_NAME, -1, e.getMessage());
	}

	return new Krb5NameElement(principalName, gssNameStr, gssNameType);
    }

    static Krb5NameElement getInstance(PrincipalName principalName) {
	return new Krb5NameElement(principalName,
				   principalName.getName(),
				   Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL);
    }

    private static String[] getComponents(String gssNameStr) 
	throws GSSException {

	String[] retVal;

	// TBD: Perhaps provide this parsing code in PrincipalName
    
	// Look for @ as in service@host
	// Assumes host name will not have an escaped '@'
	int separatorPos = gssNameStr.lastIndexOf('@', gssNameStr.length());

	// Not really a separator if it is escaped. Then this is just part
	// of the principal name or service name
	if ((separatorPos > 0) && 
	        (gssNameStr.charAt(separatorPos-1) == '\\')) {
	    // Is the `\` character escaped itself?
	    if ((separatorPos - 2 < 0) ||
		(gssNameStr.charAt(separatorPos-2) != '\\'))
		separatorPos = -1;
	}

	if (separatorPos > 0) {
	    String serviceName = gssNameStr.substring(0, separatorPos);
	    String hostName = gssNameStr.substring(separatorPos+1);
	    retVal = new String[] { serviceName, hostName};
	} else {
	    retVal = new String[] {gssNameStr};
	}

	return retVal;

    }

    private static String getHostBasedInstance(String serviceName, 
					       String hostName) 
	throws GSSException {
	    StringBuffer temp = new StringBuffer(serviceName);

	    try {
		// A lack of "@" defaults to the service being on the local
		// host as per RFC 2743
		// TBD: Move this part into JGSS framework
		if (hostName == null)
		    hostName = InetAddress.getLocalHost().getHostName();

	    } catch (UnknownHostException e) {
		// use hostname as it is
	    }
	    hostName = hostName.toLowerCase();

	    temp = temp.append('/').append(hostName);
	    return temp.toString();
    }

    public final PrincipalName getKrb5PrincipalName() {
	return krb5PrincipalName;
    }

    /**
     * Equal method for the GSSNameSpi objects.
     * If either name denotes an anonymous principal, the call should
     * return false.
     *
     * @param name to be compared with
     * @returns true if they both refer to the same entity, else false
     * @exception GSSException with major codes of BAD_NAMETYPE,
     *	BAD_NAME, FAILURE
     */
    public boolean equals(GSSNameSpi other) throws GSSException {

	if (other == this)
	    return true;

	if (other instanceof Krb5NameElement) {
		Krb5NameElement that = (Krb5NameElement) other;
		return (this.krb5PrincipalName.getName().equals(
			    that.krb5PrincipalName.getName()));
	}
	return false;
    }
    
    /**
     * Returns the principal name in the form user@REALM or
     * host/service@REALM but with the following contraints that are
     * imposed by RFC 1964:
     * <pre>
     *  (1) all occurrences of the characters `@`,  `/`, and `\` within
     *   principal components or realm names shall be quoted with an
     *   immediately-preceding `\`.
     *
     *   (2) all occurrences of the null, backspace, tab, or newline
     *   characters within principal components or realm names will be
     *   represented, respectively, with `\0`, `\b`, `\t`, or `\n`.
     *
     *   (3) the `\` quoting character shall not be emitted within an
     *   exported name except to accomodate cases (1) and (2).
     * </pre>
     */
    public byte[] export() throws GSSException {
	// TBD: Apply the above constraints sometime.
	byte[] retVal = null;
	try {
	    retVal = krb5PrincipalName.getName().getBytes(CHAR_ENCODING);
	} catch (UnsupportedEncodingException e) {
	    // Can't happen
	}
	return retVal;
    }

    /**
     * Get the mechanism type that this NameElement corresponds to.
     *
     * @return the Oid of the mechanism type
     */
    public Oid getMechanism() {
	return (Krb5MechFactory.GSS_KRB5_MECH_OID);
    }
  
    /**
     * Returns a string representation for this name. The printed
     * name type can be obtained by calling getStringNameType().
     *
     * @return string form of this name
     * @see #getStringNameType()
     * @overrides Object#toString
     */
    public String toString() {
	return (gssNameStr);
	// For testing: return (super.toString());
    }

    /**
     * Returns the name type oid.
     */
    public Oid getGSSNameType() {
	return (gssNameType);
    }
    
    /**
     * Returns the oid describing the format of the printable name.
     *
     * @return the Oid for the format of the printed name
     */
    public Oid getStringNameType() {
	// TBD: For NT_EXPORT_NAME return a different name type. Infact,
	// don even store NT_EXPORT_NAME in the cons.
	return (gssNameType);
    }
    
    /**
     * Produces a copy of this object.
     */ 
    public Object clone() {
	return null; // Cannot throw GSSException
    }
  
    /**
     * Indicates if this name object represents an Anonymous name.
     */
    public boolean isAnonymousName() {
	return (gssNameType.equals( GSSName.NT_ANONYMOUS));
    }
  
    public Provider getProvider() {
	return Krb5MechFactory.PROVIDER;
    }

    /*
    public static void main(String[] args) {
	try {
	    System.out.println("Should be mdu: "
			       + Krb5NameElement.getInstance("mdu", 
				       GSSName.NT_USER_NAME));
	    System.out.println("Should be nfs@vishwas.eng.sun.com: "
			       +Krb5NameElement.getInstance("nfs@vishwas.eng.sun.com", 
					   GSSName.NT_HOSTBASED_SERVICE));
	    Krb5NameElement service = 
		Krb5NameElement.getInstance("nfs@vishwas.eng.sun.com", 
					   GSSName.NT_HOSTBASED_SERVICE);
				    // null);
	    System.out.println("Should be 1.3.6.1.5.6.2: "
			       + service.getStringNameType());
	    System.out.println("Should be "
		      + "nfs/vishwas.eng.sun.com@<REALM>: "
		      + service.toString());
	    
	    // will convert to NT_HOSTBASED_SERVICE
	    //System.out.println(Krb5NameElement.getInstance("nfs@vishwas.eng.sun.cOM", 
	    //					   GSSName.NT_ANONYMOUS));

	    Krb5NameElement ele1 = Krb5NameElement.getInstance("mdu",
					       GSSName.NT_USER_NAME);
	    Krb5NameElement ele2 = Krb5NameElement.getInstance("mdu",
					       GSSName.NT_USER_NAME);
	    System.out.println("Should be true: "
			       + ele1.equals((GSSNameSpi)ele2));

	    System.out.println("Should be mdu@<REALM>: "
			       + ele1.toString());
	    System.out.println(Krb5Token.getHexBytes(ele1.export()));

	    Krb5NameElement ele3 = Krb5NameElement.getInstance(service.toString(),
				       Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL);
	    System.out.println(ele3.getGSSNameType() + " "
			       + ele3.toString());

	    Krb5NameElement ele4 = Krb5NameElement.getInstance(ele1.toString(),
				       Krb5MechFactory.NT_GSS_KRB5_PRINCIPAL);
	    System.out.println(ele4.getGSSNameType() + " "
			       + ele4.toString());

	    System.out.println("Should be true: "
			       + ele3.equals(service));
	    System.out.println("Should be true: "
			       + ele4.equals(ele1));

	} catch (GSSException e) {
	    System.out.println(e);
	}
    }
*/
}

