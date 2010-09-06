/*
 * @(#)GSSCredentialImpl.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss;

import org.ietf.jgss.*;
import sun.security.jgss.spi.*;
import java.util.Set;
import java.util.HashSet;
import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Map;

class GSSCredentialImpl implements GSSCredential {
    
    private GSSManagerImpl gssManager = null;
    private boolean destroyed = false;

    /*
     * We store all elements in a hashtable, using <oid, usage> as the
     * key. This makes it easy to locate the specific kind of credential we
     * need. The implementation needs to be optimized for the case where
     * there is just one element (tempCred).
     */
    private Hashtable hashtable = null;

    // TBD: Remove this? Or keep it as an optimization for single
    // mech usage?
    private GSSCredentialSpi tempCred = null;
    
    public GSSCredentialImpl(GSSManagerImpl gssManager, int usage)
	throws GSSException { 
	this(gssManager, null, GSSCredential.DEFAULT_LIFETIME, 
	     (Oid[]) null, usage);
    }

    public GSSCredentialImpl(GSSManagerImpl gssManager, GSSName name,
			     int lifetime, Oid mech, int usage)
	throws GSSException {
	init(gssManager);
	add(name, lifetime, lifetime, mech, usage);
    }

    public GSSCredentialImpl(GSSManagerImpl gssManager, GSSName name,
			     int lifetime, Oid mechs[], int usage)
	throws GSSException {
	init(gssManager);
	boolean defaultList = false;
	if (mechs == null) {
	    mechs = gssManager.getMechs();
	    defaultList = true;
	}
	
	for (int i = 0; i < mechs.length; i++) {
	    try {
		add(name, lifetime, lifetime, mechs[i], usage);
	    } catch (GSSException e) {
		// TBD: IMP - Replace all squelches with logging API!!!
		e.printStackTrace();
		if (!defaultList)
		    throw e; // else try the next mechanism
	    }
	}
	if (hashtable.size() == 0)
	    throw new GSSException(GSSException.NO_CRED);
    }
    
    GSSCredentialImpl(GSSManagerImpl gssManager,
		      GSSCredentialSpi mechElement) throws GSSException {

	init(gssManager);
	int usage = GSSCredential.ACCEPT_ONLY;
	if (mechElement.isInitiatorCredential()) {
	    if (mechElement.isAcceptorCredential()) {
		usage = GSSCredential.INITIATE_AND_ACCEPT;
	    } else {
		usage = GSSCredential.INITIATE_ONLY;
	    }
	}
	SearchKey key = new SearchKey(mechElement.getMechanism(),
					usage);
	tempCred = mechElement;
	hashtable.put(key, tempCred);
    }

    void init(GSSManagerImpl gssManager) {
	this.gssManager = gssManager;
	hashtable = new Hashtable(gssManager.getMechs().length);
    }

    public void dispose() throws GSSException { 
	if (!destroyed) {
	    Object element;
	    Enumeration values = hashtable.elements();
	    while (values.hasMoreElements()) {
		element = values.nextElement();
		((GSSCredentialSpi) element).dispose();
	    }
	    destroyed = true;
	} 
    }

    public GSSName getName() throws GSSException { 
	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}
	return new GSSNameImpl(gssManager, tempCred.getName());
    }
  
    public GSSName getName(Oid mech) throws GSSException {

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	SearchKey key = null;
	GSSCredentialSpi element = null;

	if (mech == null) {
	    mech = gssManager.getDefaultMechanism();
	}

	key = new SearchKey(mech, GSSCredential.INITIATE_ONLY); 
	element = (GSSCredentialSpi) hashtable.get(key);

	if (element == null) {
	    key = new SearchKey(mech, GSSCredential.ACCEPT_ONLY); 
	    element = (GSSCredentialSpi) hashtable.get(key);
	}

	if (element == null) {
	    key = new SearchKey(mech, GSSCredential.INITIATE_AND_ACCEPT); 
	    element = (GSSCredentialSpi) hashtable.get(key);
	}

	if (element == null) {
	    throw new GSSExceptionImpl(GSSException.BAD_MECH, mech.toString());
	}

	return new GSSNameImpl(gssManager, element.getName());
	
    }
  
    /**
     * Returns the remaining lifetime of this credential. The remaining
     * lifetime is defined as the minimum lifetime, either for initiate or
     * for accept, across all elements contained in it. Not terribly
     * useful, but required by GSS-API.
     */
    public int getRemainingLifetime() throws GSSException { 

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	SearchKey tempKey;
	GSSCredentialSpi tempCred;
	int tempLife = 0, tempInitLife = 0, tempAcceptLife = 0;
	int min = INDEFINITE_LIFETIME;

	for (Enumeration e = hashtable.keys(); e.hasMoreElements(); ) {
	    tempKey = (SearchKey) e.nextElement();
	    tempCred = (GSSCredentialSpi) hashtable.get(tempKey);
	    if (tempKey.getUsage() == INITIATE_ONLY)
		tempLife = tempCred.getInitLifetime();
	    else if (tempKey.getUsage() == ACCEPT_ONLY)
		tempLife = tempCred.getAcceptLifetime();
	    else {
		tempInitLife = tempCred.getInitLifetime();
		tempAcceptLife = tempCred.getAcceptLifetime();
		tempLife = (tempInitLife < tempAcceptLife ?
			    tempInitLife: 
			    tempAcceptLife);
	    }
	    if (min > tempLife)
		min = tempLife;
	}

	return min;
    }
  
    public int getRemainingInitLifetime(Oid mech) throws GSSException { 

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	GSSCredentialSpi element = null;
	SearchKey key = null;
	boolean found = false;
	int max = 0;

	if (mech == null) {
	    mech = gssManager.getDefaultMechanism();
	}
	
	key = new SearchKey(mech, GSSCredential.INITIATE_ONLY); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    found = true;
	    if (max < element.getInitLifetime())
		max = element.getInitLifetime();
	}
	
	key = new SearchKey(mech, GSSCredential.INITIATE_AND_ACCEPT); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    found = true;
	    if (max < element.getInitLifetime())
		max = element.getInitLifetime();
	}
	
	if (!found) {
	    throw new GSSExceptionImpl(GSSException.BAD_MECH, mech.toString());
	}

	return max;
	
    }
  
    public int getRemainingAcceptLifetime(Oid mech) throws GSSException {

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	GSSCredentialSpi element = null;
	SearchKey key = null;
	boolean found = false;
	int max = 0;

	if (mech == null) {
	    mech = gssManager.getDefaultMechanism();
	}
	
	key = new SearchKey(mech, GSSCredential.ACCEPT_ONLY); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    found = true;
	    if (max < element.getAcceptLifetime())
		max = element.getAcceptLifetime();
	}
	
	key = new SearchKey(mech, GSSCredential.INITIATE_AND_ACCEPT); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    found = true;
	    if (max < element.getAcceptLifetime())
		max = element.getAcceptLifetime();
	}
	
	if (!found) {
	    throw new GSSExceptionImpl(GSSException.BAD_MECH, mech.toString());
	}

	return max;
	
    }
  
    /**
     * Returns the usage mode for this credential. Returns
     * INITIATE_AND_ACCEPT if any one element contained in it supports
     * INITIATE_AND_ACCEPT or if two different elements exist where one
     * support INITIATE_ONLY and the other supports ACCEPT_ONLY.
     */
    public int getUsage() throws GSSException { 

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	SearchKey tempKey;
	boolean initiate = false;
	boolean accept = false;

	for (Enumeration e = hashtable.keys(); e.hasMoreElements(); ) {
	    tempKey = (SearchKey) e.nextElement();
	    if (tempKey.getUsage() == INITIATE_ONLY)
		initiate = true;
	    else if (tempKey.getUsage() == ACCEPT_ONLY)
		accept = true;
	    else
		return INITIATE_AND_ACCEPT;
	}
	if (initiate) {
	    if (accept)
		return INITIATE_AND_ACCEPT;
	    else
		return INITIATE_ONLY;
	} else
	    return ACCEPT_ONLY;
    }
  
    public int getUsage(Oid mech) throws GSSException {

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	GSSCredentialSpi element = null;
	SearchKey key = null;
	boolean initiate = false;
	boolean accept = false;

	if (mech == null) {
	    mech = gssManager.getDefaultMechanism();
	}

	key = new SearchKey(mech, GSSCredential.INITIATE_ONLY); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    initiate = true;
	}
	
	key = new SearchKey(mech, GSSCredential.ACCEPT_ONLY); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    accept = true;
	}
	
	key = new SearchKey(mech, GSSCredential.INITIATE_AND_ACCEPT); 
	element = (GSSCredentialSpi) hashtable.get(key);
	
	if (element != null) {
	    initiate = true;
	    accept = true;
	}
	
	if (initiate && accept)
	    return GSSCredential.INITIATE_AND_ACCEPT;
	else if (initiate)
	    return GSSCredential.INITIATE_ONLY;
	else if (accept)
	    return GSSCredential.ACCEPT_ONLY;
	else
	    throw new GSSExceptionImpl(GSSException.BAD_MECH, mech.toString());
    }
  
    public Oid[] getMechs() throws GSSException { 

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}
	Oid[] retVal = {tempCred.getMechanism()};
	return retVal; // TBD
    }
    
    public void add(GSSName name, int initLifetime, int acceptLifetime,
		    Oid mech, int usage) throws GSSException { 

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	if (mech == null) {
	    mech = gssManager.getDefaultMechanism();
	}

	SearchKey key = new SearchKey(mech, usage);
	if (hashtable.containsKey(key)) {
	    throw new GSSExceptionImpl(GSSException.DUPLICATE_ELEMENT,
				   getElementStr(mech, usage));
	}
	
	// TBD: If not instance of GSSNameImpl then throw exception
	// Application mixing GSS implementations
	GSSNameSpi nameElement = (name == null ? null :
				  ((GSSNameImpl)name).getElement(mech));
	
	tempCred = gssManager.getCredentialElement(nameElement, 
						   initLifetime, 
						   acceptLifetime,
						   mech, 
						   usage);
	/* 
	 * Not all mechanisms support the concept of one credential element
	 * that can be used for both initiating and accepting a context. In
	 * the event that an application requests usage INITIATE_AND_ACCEPT
	 * for a credential from such a mechanism, the GSS framework will
	 * need to obtain two different credential elements from the
	 * mechanism, one that will have usage INITIATE_ONLY and another
	 * that will have usage ACCEPT_ONLY. The mechanism will help the
	 * GSS-API realize this by returning a credential element with
	 * usage INITIATE_ONLY or ACCEPT_ONLY prompting it to make another
	 * call to getCredentialElement, this time with the other usage
	 * mode.
	 */

	if (usage == GSSCredential.INITIATE_AND_ACCEPT && 
	    (!tempCred.isAcceptorCredential() ||
	    !tempCred.isInitiatorCredential())) {

	    int currentUsage;
	    int desiredUsage;

	    if (!tempCred.isInitiatorCredential()) {
		currentUsage = GSSCredential.ACCEPT_ONLY;
		desiredUsage = GSSCredential.INITIATE_ONLY;
	    } else {
		currentUsage = GSSCredential.INITIATE_ONLY;
		desiredUsage = GSSCredential.ACCEPT_ONLY;
	    }
	    
	    key = new SearchKey(mech, currentUsage);
	    hashtable.put(key, tempCred);
	    
	    tempCred = gssManager.getCredentialElement(nameElement, 
						       initLifetime, 
						       acceptLifetime,
						       mech, 
						       desiredUsage);
	    
	    key = new SearchKey(mech, desiredUsage);
	    hashtable.put(key, tempCred);
	} else {
	    hashtable.put(key, tempCred);
	}
    }
    
    public boolean equals(Object another) { 

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	if (this == another) {
	    return true;
	}

	if (!(another instanceof GSSCredentialImpl)) {
	    return false;
	}

	/*
	 * TBD:
	 * The RFC says: "Tests if this GSSCredential refers to the same
	 * entity as the supplied object.  The two credentials must be
	 * acquired over the same mechanisms and must refer to the same
	 * principal.  Returns "true" if the two GSSCredentials refer to
	 * the same entity; "false" otherwise."
	 *
	 * Well, when do two credentials refer to the same principal? Do
	 * they need to have one GSSName in common for the different
	 * GSSName's that the credential elements return? Or do all
	 * GSSName's have to be in common when the names are exported with
	 * their respective mechanisms for the credential elements?
	 */
	return false;
	
    }
    
    /**
     * Returns a hashcode value for this GSSCredential.
     *
     * @return a hashCode value
     */
    public int hashCode() {

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}
	/*
	 * TBD:
	 * Again, a tough cookie to crack. Decide on a criteria for equals
	 * first then do this. The specification for equals in the RFC is
	 * obviously broken.
	 */
	return 1;
    }

    /**
     * Returns the specified mechanism's credential-element.
     *
     * @param mechOid - the oid for mechanism to retrieve
     * @param throwExcep - boolean indicating if the function is
     *    to throw exception or return null when element is not
     *    found.
     * @return mechanism credential object
     * @exception GSSException of invalid mechanism
     */
    GSSCredentialSpi getElement(Oid mechOid, boolean initiate) 
	throws GSSException {

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	SearchKey key;
	GSSCredentialSpi element;

	if (mechOid == null) {
	    /*
	     * First see if the default mechanism satisfies the
	     * desired usage.
	     */
	    mechOid = gssManager.getDefaultMechanism();
	    key = new SearchKey(mechOid, 
				 initiate? INITIATE_ONLY : ACCEPT_ONLY);
	    element = (GSSCredentialSpi) hashtable.get(key);
	    if (element == null) {
		key = new SearchKey(mechOid, INITIATE_AND_ACCEPT);
		element = (GSSCredentialSpi) hashtable.get(key);
		if (element == null) {
		    /*
		     * Now just return any element that satisfies the 
		     * desired usage.
		     */
		    Object[] elements = hashtable.entrySet().toArray();
		    for (int i = 0; i < elements.length; i++) {
			element = (GSSCredentialSpi) 
			    ((Map.Entry)elements[i]).getValue();
			if (element.isInitiatorCredential() ==
			    initiate)
			    break;
		    } // for loop
		}
	    }
	} else {
	    
	    if (initiate)
		key = new SearchKey(mechOid, INITIATE_ONLY);
	    else
		key = new SearchKey(mechOid, ACCEPT_ONLY);
	    
	    element = (GSSCredentialSpi) hashtable.get(key);
	    
	    if (element == null) {
		key = new SearchKey(mechOid, INITIATE_AND_ACCEPT);
		element = (GSSCredentialSpi) hashtable.get(key);
	    }
	}

	if (element == null)
	    throw new GSSExceptionImpl(GSSException.NO_CRED,
				   getElementStr(mechOid,
				     initiate? INITIATE_ONLY : ACCEPT_ONLY));
	return element;
    }

    Set getElements() {
	Object o;
	HashSet retVal = new HashSet(hashtable.size());
	Enumeration values = hashtable.elements();
	while (values.hasMoreElements()) {
	    o = values.nextElement();
	    retVal.add(o);
	}
	return retVal;
    }

    private static String getElementStr(Oid mechOid, int usage) {
	String displayString = mechOid.toString();
	if (usage == GSSCredential.INITIATE_ONLY) {
	    displayString = 
		displayString.concat(" usage: Initiate");
	} else if (usage == GSSCredential.ACCEPT_ONLY) {
	    displayString = 
		displayString.concat(" usage: Accept");
	} else {
	    displayString = 
		displayString.concat(" usage: Initiate and Accept");
	}
	return displayString;
    }

    public String toString() {

	if (destroyed) {
	    throw new IllegalStateException("This credential is " +
					"no longer valid");
	}

	GSSCredentialSpi element = null;
	StringBuffer buffer = new StringBuffer("[GSSCredential: ");
	Object[] elements = hashtable.entrySet().toArray();
	for (int i = 0; i < elements.length; i++) {
	    try {
		buffer.append('\n');
		element = (GSSCredentialSpi) 
		    ((Map.Entry)elements[i]).getValue();
		buffer.append(element.getName());
		buffer.append(' ');
		buffer.append(element.getMechanism());
		buffer.append(element.isInitiatorCredential() ? 
			      " Initiate" : "");
		buffer.append(element.isAcceptorCredential() ? 
			      " Accept" : "");
		buffer.append(" [");
		buffer.append(element.toString());
		buffer.append(']');
	    } catch (GSSException e) {
		// skip to next element
	    }
	}
	buffer.append(']');
	return buffer.toString();
    }
    
    static class SearchKey {
	private Oid mechOid = null;
	private int usage = GSSCredential.INITIATE_AND_ACCEPT;
	public SearchKey(Oid mechOid, int usage) {

	    this.mechOid = mechOid;
	    this.usage = usage;
	}
	public int getUsage() {
	    return usage;
	}
	public boolean equals(Object other) {
	    if (! (other instanceof SearchKey))
		return false;
	    SearchKey that = (SearchKey) other;
	    return ((this.mechOid.equals(that.mechOid)) &&
		    (this.usage == that.usage));
	}
	public int hashCode() {
	    return mechOid.hashCode();
	}
    }

}
