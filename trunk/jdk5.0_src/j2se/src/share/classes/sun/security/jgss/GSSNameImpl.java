/*
 * @(#)GSSNameImpl.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss;  
  
import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import java.util.Set;
import java.util.HashMap;
import java.util.HashSet;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import sun.security.util.ObjectIdentifier;
import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;

/**
 * This is the implementation class for GSSName. Conceptually the
 * GSSName is a container with mechanism specific name elements. Each 
 * name element is a representation of how that particular mechanism
 * would canonicalize this principal.
 *
 * Generally a GSSName is created by an application when it supplies 
 * a sequence of bytes and a nametype that helps each mechanism
 * decide how to interpret those bytes.
 *
 * It is not necessary to create name elements for each available
 * mechanism at the time the application creates the GSSName. This
 * implementation does this lazily, as and when name elements for
 * mechanisms are required to be handed out. (Generally, other GSS
 * classes like GSSContext and GSSCredential request specific
 * elements depending on the mechanisms that they are dealing with.)
 * Assume that getting a mechanism to parse the applciation specified
 * bytes is an expensive call.
 *
 * When a GSSName is canonicalized wrt some mechanism, it is supposed
 * to discard all elements of other mechanisms and retain only the
 * element for this mechanism. In GSS terminology this is called a
 * Mechanism Name or MN. This implementation tries to retain the
 * application provided bytes and name type just in case the MN is
 * asked to produce an element for a mechanism that is different.
 *
 * When a GSSName is to be exported, the name element for the desired
 * mechanism is converted to a byte representation and written
 * out. It might happen that a name element for that mechanism cannot 
 * be obtained. This happens when the mechanism is just not supported 
 * in this GSS-API or when the mechanism is supported but bytes
 * corresponding to the nametypes that it understands are not
 * available in this GSSName.
 *
 * This class is safe for sharing. Each retrieval of a name element
 * from getElement() might potentially add a new element to the
 * hashmap of elements, but getElement() is synchronized.
 *
 * @author Mayank Upadhyay
 * @version 1.9, 12/19/03
 * @since 1.4
 */

class GSSNameImpl implements GSSName {

    private GSSManagerImpl gssManager = null;

    /*
     * Store whatever the application passed in. We will use this to
     * get individual mechanisms to create name elements as and when
     * needed.
     * Store both the String and the byte[]. Leave I18N to the
     * mechanism by allowing it to extract bytes from the String!
     */
    
    private String appNameStr = null;
    private byte[] appNameBytes = null;
    private Oid appNameType = null;

    /*
     * When we figure out what the printable name would be, we store
     * both the name and its type.
     */

    private String printableName = null;
    private Oid printableNameType = null;
    
    private HashMap elements = null;
    private GSSNameSpi mechElement = null;

    GSSNameImpl(GSSManagerImpl gssManager,
		GSSNameSpi mechElement)
	throws GSSException {

	this(gssManager, 
	     mechElement.toString(), mechElement.getStringNameType(),
	     mechElement.getMechanism());

    }

    GSSNameImpl(GSSManagerImpl gssManager, 
		       Object appName, 
		       Oid appNameType)
	throws GSSException{ 
	this(gssManager, appName, appNameType, null);
    }
    
    GSSNameImpl(GSSManagerImpl gssManager, 
			Object appName,
			Oid appNameType,
			Oid mech) 
	throws GSSException {
	
	if (appName == null)
	    throw new GSSExceptionImpl(GSSException.BAD_NAME,
				   "Cannot import null name");
	if (appNameType != null &&
	    appNameType.equals(NT_EXPORT_NAME)) {
	    importName(gssManager, appName);
	} else {
	    init(gssManager, appName, appNameType, mech);
	}
    }
    
    private void init(GSSManagerImpl gssManager,
		      Object appName, Oid appNameType, 
		      Oid mech)
	throws GSSException {
	
	this.gssManager = gssManager;
	this.elements = new HashMap(gssManager.getMechs().length);

	if (appName instanceof String) {
	    this.appNameStr = (String) appName;
	    /*
	     * If appNameType is null, then the nametype for this printable
	     * string is determined only by interrogating the
	     * mechanism. Thus, defer the setting of printableName and
	     * printableNameType till later.
	     */
	    if (appNameType != null) {
		printableName = appNameStr;
		printableNameType = appNameType;
	    }
	} else {
	    this.appNameBytes = (byte[]) appName;
	}
	
	this.appNameType = appNameType;
	
	mechElement = getElement(mech);
	
	/*
	 * printableName will be null if appName was in a byte[] or if
	 * appName was in a String but appNameType was null.
	 */
	if (printableName == null) {
	    printableName = mechElement.toString();
	    printableNameType = mechElement.getStringNameType();
	}
	    
	/*
	 *  At this point the GSSNameImpl has the following set:
	 *   appNameStr or appNameBytes
	 *   appNameType (could be null)
	 *   printableName
	 *   printableNameType
	 *   mechElement (which also exists in the hashmap of elements)
	 */
    }
	
    private void importName(GSSManagerImpl gssManager,
			    Object appName)
	throws GSSException {

	int pos = 0;
	byte[] bytes = null;    

	if (appName instanceof String) {
	    try {
		bytes = ((String) appName).getBytes("UTF-8");
	    } catch (UnsupportedEncodingException e) {
		// Won't happen
	    }
	} else
	    bytes = (byte[]) appName;

	if ((bytes[pos++] != 0x04) ||
	    (bytes[pos++] != 0x01))
	    throw new GSSExceptionImpl(GSSException.BAD_NAME,
				   "Exported name token id is corrupted!");

	int oidLen  = (((0xFF & bytes[pos++]) << 8) |
		       (0xFF & bytes[pos++]));
	ObjectIdentifier temp = null;
	try {
	    DerInputStream din = new DerInputStream(bytes, pos,
						    oidLen);
	    temp = new ObjectIdentifier(din);
	} catch (IOException e) {
	    throw new GSSExceptionImpl(GSSException.BAD_NAME,
		       "Exported name Object identifier is corrupted!");
	}
	Oid oid = new Oid(temp.toString());
	pos += oidLen;
	int mechPortionLen = (((0xFF & bytes[pos++]) << 24) |
			      ((0xFF & bytes[pos++]) << 16) |
			      ((0xFF & bytes[pos++]) << 8) |
			      (0xFF & bytes[pos++]));
	byte[] mechPortion = new byte[mechPortionLen];
	System.arraycopy(bytes, pos, mechPortion, 0, mechPortionLen);

	init(gssManager, mechPortion, null, oid);
    }

    public GSSName canonicalize(Oid mech) throws GSSException{ 
	
	if (mech == null)
	    mech = gssManager.getDefaultMechanism();

	Object appName = null;
	if (appNameStr != null)
	    appName = appNameStr;
	else
	    appName = appNameBytes;

	return new GSSNameImpl(gssManager, appName, appNameType, mech);
    }

    /**
     * This method may return false negatives. But if it says two
     * names are equals, then there is some mechanism that
     * authenticates them as the same principal.
     */
    public boolean equals(GSSName other) throws GSSException { 

	if (this.isAnonymous() || other.isAnonymous())
	    return false;

	if (other == this)
	    return true;

	if (! (other instanceof GSSNameImpl))
	    return equals(gssManager.createName(other.toString(), 
						other.getStringNameType()));
	
	/*
	 * TBD: Do a comparison of the appNameStr/appNameBytes if
	 * available. If that fails, then proceed with this test.
	 */

	GSSNameImpl that = (GSSNameImpl) other;

	GSSNameSpi myElement = this.mechElement;
	GSSNameSpi element = that.mechElement;

	/*
	 * TBD: If they are not of the same mechanism type, convert both to 
	 * Kerberos since it is guaranteed to be present.
	 */
	if ((myElement == null) && (element != null)) {
	    myElement = this.getElement(element.getMechanism());
	} else if ((myElement != null) && (element == null)) {
	    element = that.getElement(myElement.getMechanism());
	}
	
	if (myElement != null && element != null) {
	    return myElement.equals(element);
	}

	if ((this.appNameType != null) && 
	    (that.appNameType != null)) {
	    if (!this.appNameType.equals(that.appNameType)) {
		return false;
	    }
	    byte[] myBytes = null; 
	    byte[] bytes = null; 
	    try {
		myBytes = 
		    (this.appNameStr != null ? 
		     this.appNameStr.getBytes("UTF-8") :
		     this.appNameBytes);
		bytes =
		    (that.appNameStr != null ? 
		     that.appNameStr.getBytes("UTF-8") :
		     that.appNameBytes);
	    } catch (UnsupportedEncodingException e) {
		// Won't happen
	    }

	    return GSSManagerImpl.compareBytes(myBytes, bytes);
	}

	return false;

    }

    /**
     * Returns a hashcode value for this GSSName.
     *
     * @return a hashCode value
     */
    public int hashCode() {
	/*
	 * TBD:
	 * In order to get this to work reliably and properly(!), obtain a
	 * Kerberos name element for the name and then call hashCode on its 
	 * string representation. But this cannot be done if the nametype
	 * is not one of those supported by the Kerberos provider and hence 
	 * this name cannot be imported by Kerberos. In that case return a
	 * constant value!
	 */

	return 1;
    }

    public boolean equals(Object another){ 

	try {
	    // TBD: This can lead to an infinite loop. Extract info
	    // and create a GSSNameImpl with it.

	    if (another instanceof GSSName)
		return equals((GSSName) another);
	} catch (GSSException e) {
	    // Squelch it and return false
	}

	    return false;
    }

    /**
     * Returns a flat name representation for this object. The name
     * format is defined in RFC 2743:
     *<pre>
     * Length           Name          Description
     * 2               TOK_ID          Token Identifier
     *                                 For exported name objects, this
     *                                 must be hex 04 01.
     * 2               MECH_OID_LEN    Length of the Mechanism OID
     * MECH_OID_LEN    MECH_OID        Mechanism OID, in DER
     * 4               NAME_LEN        Length of name
     * NAME_LEN        NAME            Exported name; format defined in
     *                                 applicable mechanism draft.
     *</pre>
     *
     * Note that it is not required to canonicalize a name before
     * calling export(). i.e., the name need not be an MN. If it is
     * not an MN, an implementation defined algorithm can be used for 
     * choosing the mechanism which should export this name.
     *
     * @return the flat name representation for this object
     * @exception GSSException with major codes NAME_NOT_MN, BAD_NAME,
     *	BAD_NAME, FAILURE.   
     */
    public byte[] export() throws GSSException{ 

	if (mechElement == null) {
	    /* Use default mech */
	    mechElement = getElement (null);
	}
	
	byte[] mechPortion = mechElement.export();
	byte[] oidBytes = null;
	ObjectIdentifier oid = null;
	
	try {
	    oid = new ObjectIdentifier(
				       mechElement.getMechanism().toString());
	} catch (IOException e) {
	    throw new GSSExceptionImpl(GSSException.FAILURE,
				       "Invalid OID String ");
	}
	DerOutputStream dout = new DerOutputStream();
	try {
	    dout.putOID(oid);
	} catch (IOException e) {
	    throw new GSSExceptionImpl(GSSException.FAILURE,
				   "Could not ASN.1 Encode "
				   + oid.toString());
	}
	oidBytes = dout.toByteArray();

	byte[] retVal = new byte[2 
				+ 2 + oidBytes.length 
				+ 4 + mechPortion.length];
	int pos = 0;
	retVal[pos++] = 0x04;
	retVal[pos++] = 0x01;
	retVal[pos++] = (byte) (oidBytes.length>>>8);
	retVal[pos++] = (byte) oidBytes.length;
	System.arraycopy(oidBytes, 0, retVal, pos, oidBytes.length);
	pos += oidBytes.length;
	retVal[pos++] = (byte) (mechPortion.length>>>24);
	retVal[pos++] = (byte) (mechPortion.length>>>16);
	retVal[pos++] = (byte) (mechPortion.length>>>8);
	retVal[pos++] = (byte)  mechPortion.length;
	System.arraycopy(mechPortion, 0, retVal, pos, mechPortion.length);
	return retVal;
    }
  
    /*
     * For testing. Prints out the complete state of this GSSNameImpl.
     *
     private String getTempArrayStr(HashMap elements) {
     if (elements == null)
     return "{}";
     
     StringBuffer buf = new StringBuffer("{");
     Object[] array = elements.values().toArray();
     for (int i = 0; i < array.length; i++)
     buf.append(" " + array[i]);
     buf.append("}");
     return buf.toString();
     }
    */
    
    public String toString(){ 
	/*
	 *For testing:
	 *
	 String tempStr = " [" + appNameStr + " " 
	     + getNameTypeStr(appNameType) + " " 
	 + mechElement + " " + getTempArrayStr(elements) + "]";
	 return tempStr;
	*/

	 return printableName;
	
    }

    public Oid getStringNameType() throws GSSException{ 
	return printableNameType;
    }

    public boolean isAnonymous(){ 
	return false; // TBD
    }

    public boolean isMN() {
        return true; // Since always canonicalized for some mech
    }

    synchronized GSSNameSpi getElement(Oid mechOid) 
	throws GSSException {

	GSSNameSpi retVal = (GSSNameSpi) elements.get(mechOid);

	if (retVal == null) {
	    Object appName = null;
	    if (appNameStr != null)
		appName = appNameStr;
	    else
		appName = appNameBytes;

	    retVal = gssManager.getNameElement(appName, appNameType,
					       mechOid);
	    elements.put(mechOid, retVal);
	}
	return retVal;
    }

    Set getElements() {
	return new HashSet(elements.values());
    }

    /*
     * TBD:
     * For testing only: Will remove.
     */

  public static final  String getHexBytes(byte[] bytes) {
	return getHexBytes(bytes, 0, bytes.length);
  }

  public static final  String getHexBytes(byte[] bytes, int len) {
	return getHexBytes(bytes, 0, len);
  }
        
  public static final String getHexBytes(byte[] bytes, int pos, int len) {

    StringBuffer sb = new StringBuffer();
    for (int i = 0; i < len; i++) {
        
      int b1 = (bytes[i]>>4) & 0x0f;
      int b2 = bytes[i] & 0x0f;
        
      sb.append(Integer.toHexString(b1));
      sb.append(Integer.toHexString(b2));
      sb.append(' ');
    }
    return sb.toString();
  }

    static Oid NT_KRB5_PRINCIPAL = null;
    public static void main(String[] args) throws Exception {

	Oid krb5Mech = new Oid("1.2.840.113554.1.2.2");
	NT_KRB5_PRINCIPAL = new Oid("1.2.840.113554.1.2.2.1");

	GSSManager mgr = GSSManager.getInstance();

	String mduUser = "mdu";
	String mduPrincipal = "mdu@KRBNT-JSN.ENG.SUN.COM";
	String nfsHostBased = "nfs@vishwas.eng.sun.com";
	String nfsPrincipal =
	    ("nfs/vishwas.eng.sun.com@KRBNT-JSN.ENG.SUN.COM");

	GSSName mduUserName = mgr.createName(mduUser,
					     NT_USER_NAME);
	System.out.println(getNameTypeStr(mduUserName.getStringNameType())
			     + " " 
			     + "mduUserName=" + mduUserName);
	System.out.println("");

	GSSName nfsHostBasedName = mgr.createName(nfsHostBased,
						  NT_HOSTBASED_SERVICE);
	System.out.println(getNameTypeStr(nfsHostBasedName.getStringNameType())
			   + " " 
			   + "nfsHostBasedName=" + nfsHostBasedName);
	System.out.println("");

	GSSName mduPrincipalName = mgr.createName(mduPrincipal,
						  NT_KRB5_PRINCIPAL);
	System.out.println(getNameTypeStr(mduPrincipalName.getStringNameType())
			   + " " 
			   + "mduPrincipalName=" + mduPrincipalName);
	System.out.println("");
	
	GSSName nfsPrincipalName = mgr.createName(nfsPrincipal,
						  NT_KRB5_PRINCIPAL);
	System.out.println(getNameTypeStr(nfsPrincipalName.getStringNameType())
			   + " " 
			   + "nfsPrincipalName=" + nfsPrincipalName);
	System.out.println("");

	test2();
	
    }

    private static String getNameTypeStr(Oid nameTypeOid) {

	if (nameTypeOid == null)
	    return "(NT is null)";

	if (nameTypeOid.equals(NT_USER_NAME))
	    return "NT_USER_NAME";
	if (nameTypeOid.equals(NT_HOSTBASED_SERVICE))
	    return "NT_HOSTBASED_SERVICE";
	if (nameTypeOid.equals(NT_EXPORT_NAME))
	    return "NT_EXPORT_NAME";
	if (nameTypeOid.equals(NT_KRB5_PRINCIPAL))
	    return "NT_KRB5_PRINCIPAL";
	else
	    return "Unknown";
    }

   private static void test2() throws GSSException {

	Oid mechOid = new ProviderList().getDefaultMechanism();

	GSSManager mgr = GSSManager.getInstance();

	GSSName name1 = mgr.createName("mdu", NT_USER_NAME);
	System.out.println("name1: " + name1);
	System.out.println(name1.getStringNameType());
	System.out.println("---");

	GSSName name5 = name1.canonicalize(mechOid);
	System.out.println("name5: " + name5);
	System.out.println(name5.getStringNameType());
	System.out.println("---");
	System.out.println("---");

	GSSName name2 = mgr.createName("nfs@vishwas.eng.sun.com", 
				       NT_HOSTBASED_SERVICE);
	System.out.println("name2: " + name2);
	System.out.println(name2.getStringNameType());
	System.out.println("---");

	GSSName name6 = name2.canonicalize(mechOid);
	System.out.println("name6: " + name6);
	System.out.println(name6.getStringNameType());
	System.out.println("---");
	System.out.println("---");

	GSSName name3 = mgr.createName("mdu", NT_USER_NAME, mechOid);
	System.out.println("name3: " + name3);
	System.out.println(name3.getStringNameType());
	System.out.println("---");

	GSSName name7 = name3.canonicalize(mechOid);
	System.out.println("name7: " + name7);
	System.out.println(name7.getStringNameType());
	System.out.println("---");
	System.out.println("---");

	GSSName name4 = mgr.createName("nfs@vishwas.eng.sun.com", 
				       NT_HOSTBASED_SERVICE,
				       mechOid);
	System.out.println(name4);
	System.out.println(name4.getStringNameType());
	System.out.println("---");

	GSSName name8 = name4.canonicalize(mechOid);
	System.out.println(name8);
	System.out.println(name8.getStringNameType());
	System.out.println("---");
	System.out.println("---");

	System.out.println("Comparing name1 to name5: " + name1.equals(name5));
	System.out.println("Comparing name2 to name6: " + name2.equals(name6));
	System.out.println("Comparing name3 to name7: " + name3.equals(name7));
	System.out.println("Comparing name4 to name8: " + name4.equals(name8));

	System.out.println("Comparing name1 to name3: " + name1.equals(name3));
	System.out.println("Comparing name5 to name7: " + name5.equals(name7));
	System.out.println("Comparing name2 to name8: " + name2.equals(name8));
	System.out.println("Comparing name4 to name6: " + name4.equals(name6));

	byte[] exportedName = name1.export();
	System.out.println(getHexBytes(exportedName));
	GSSName name11 = mgr.createName(exportedName, NT_EXPORT_NAME);
	System.out.println(name11);
	System.out.println(name1.equals(name11));
    }

}


