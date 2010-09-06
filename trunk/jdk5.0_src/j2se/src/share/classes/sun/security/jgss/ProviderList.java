/*
 * @(#)ProviderList.java	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss; 
 
import org.ietf.jgss.*; 
import sun.security.jgss.spi.*;
import java.security.NoSuchProviderException;
import java.security.Provider;
import java.security.Security;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.HashMap;
import java.util.Enumeration;
import java.util.Iterator;

/**
 * This class stores the list of providers that this
 * GSS-Implementation is configured to use. The GSSManagerImpl class
 * queries this class whenever it needs a mechanism's factory.<p>
 *
 * This class stores an ordered list of pairs of the form 
 * <provider, oid>. When it attempts to instantiate a mechanism
 * defined by oid o, it steps through the list looking for an entry
 * with oid=o, or with oid=null. (An entry with oid=null matches all
 * mechanisms.) When it finds such an entry, the corresponding
 * provider is approached for the mechanism's factory class.
 * At instantiation time this list in initialized to contain those
 * system wide providers that contain a property of the form
 * "GssApiMechanism.x.y.z..." where "x.y.z..." is a numeric object
 * identifier with numbers x, y, z, etc. Such a property is defined
 * to map to that provider's implementation of the MechanismFactory
 * interface for the mechanism x.y.z...
 * As and when a MechanismFactory is instantiated, it is
 * cached for future use. <p>
 *
 * An application can cause more providers to be added by means of
 * the addProviderAtFront and addProviderAtEnd methods on
 * GSSManager which get delegated to this class. The
 * addProviderAtFront method can also cause a change in the ordering
 * of the providers without adding any new providers, by causing a
 * provider to move up in a list. The method addProviderAtEnd can
 * only add providers at the end of the list if they are not already
 * in the list. The rationale is that an application will call 
 * addProviderAtFront when it wants a provider to be used in
 * preference over the default ones. And it will call
 * addProviderAtEnd when it wants a provider to be used in case
 * the system ones don't suffice.<p>
 *
 * If a mechanism's factory is being obtained from a provider as a
 * result of encountering a entryof the form <provider, oid> where
 * oid is non-null, then the assumption is that the application added 
 * this entry and it wants this mechanism to be obtained from this
 * provider. Thus is the provider does not actually contain the
 * requested mechanism, an exception will be thrown. However, if the
 * entry were of the form <provider, null>, then it is viewed more
 * liberally and is simply skipped over if the provider does not claim to
 * support the requested mechanism.
 */

final class ProviderList {

    private static final String PROV_PROP_PREFIX;
    private static final int PROV_PROP_PREFIX_LEN;

    private static final String SPI_MECH_FACTORY_TYPE 
	= "sun.security.jgss.spi.MechanismFactory";
    
    private static final Oid DEFAULT_MECH_OID;
    // Obtain the mechanism factory from the instance's providers
    private MechanismFactory defaultMechFactory;

    static {
	PROV_PROP_PREFIX = "GssApiMechanism.";
	PROV_PROP_PREFIX_LEN = PROV_PROP_PREFIX.length();
	
	/*
	 * Set the default mechanism. Kerberos v5 is the default
	 * mechanism unless it is overridden by a system property.
	 */

	Oid krb5Oid = null;
	Oid tempOid = null;
	String tempOidStr = null;
	try {
	    krb5Oid = new Oid("1.2.840.113554.1.2.2");
	    tempOidStr = 
		(String) java.security.AccessController.doPrivileged(
			   new sun.security.action.GetPropertyAction(
					  "sun.security.jgss.mechanism"));
	if (tempOidStr != null)
	    tempOid = new Oid(tempOidStr);
	} catch (GSSException e) {
	    // Ignore a problematic tempOidStr since we will set the
	    // Oid ourselves
	}
	DEFAULT_MECH_OID = (tempOid != null ? tempOid : krb5Oid);
    }

    private ArrayList preferences = new ArrayList(5);;
    private HashMap factories = new HashMap(5);
    private HashSet mechs = new HashSet(5);

    /**
     * Constructor for ProviderList. It initializes the list to a
     * subset of the system wide security providers.
     */
    public ProviderList() {

	// Prune the list of system GSS Providers and store it
	
	Provider[] systemList = Security.getProviders();
	
	for (int i = 0; i < systemList.length; i++) {
	    try {
		addProviderAtEnd(systemList[i], null);
	    } catch (GSSException e) {
		// Move on to the next provider
	    }
	} // End of for loop
    }

    /**
     * Determines if the given provider property represents a GSS-API
     * Oid to MechanismFactory mapping.
     * @return true if this is a GSS-API property, false otherwise.
     */
    private boolean isMechFactoryProperty(String prop) {
	return (prop.startsWith(PROV_PROP_PREFIX) ||
		prop.regionMatches(true, 0, // Try ignoring case
				   PROV_PROP_PREFIX, 0,
				   PROV_PROP_PREFIX_LEN));
    }

    private Oid getOidFromMechFactoryProperty (String prop)
	throws GSSException {

	String oidPart = prop.substring(PROV_PROP_PREFIX_LEN); 
	return new Oid(oidPart);
    }

    /**
     * Obtains a MechanismFactory for a given mechanism.
     * @param mechOid the oid of the desired mechanism
     * @return a MechanismFactory for the desired mechanism obtained
     * from the most preferable provider
     * @throws GSSException when an explicitly configured provider
     * does not support the desired mechanism, or when no provider
     * supports the desired mechanism.
     */
    synchronized public MechanismFactory getMechFactory(Oid mechOid)
	throws GSSException {
	
	if (mechOid == null || mechOid.equals(DEFAULT_MECH_OID))
	    return defaultMechFactory;
	
	// Iterate thru all preferences to find right provider
	String className;
	Provider p;
	PreferencesEntry entry;

	Iterator list = preferences.iterator();
	while (list.hasNext()) {
	    entry = (PreferencesEntry) list.next();
	    if (entry.impliesMechanism(mechOid)) {
		MechanismFactory retVal = getMechFactory(entry, mechOid);
		if (retVal != null)
		    return retVal;
	    }
	} // end of while loop
	throw new GSSExceptionImpl(GSSException.BAD_MECH, mechOid.toString());
    }

    /**
     * Helper routine that uses a preferences entry to obtain an
     * implementation of a MechanismFactory from it.
     * @param e the preferences entry that contains the provider and
     * either a null of an explicit oid that matched the oid of the
     * desired mechanism.
     * @param mechOid the oid of the desired mechanism
     * @throws GSSException If the application explicitly requested
     * this entry's provider to be used for the desired mechanism but 
     * some problem is encountered
     */
    private MechanismFactory getMechFactory(PreferencesEntry e,
						 Oid mechOid) 
						 throws GSSException {
	Provider p = e.getProvider();
	
	/*
	 * See if a MechanismFactory was previously instantiated for
	 * this provider and mechanism combination.
	 */
	PreferencesEntry searchEntry = new PreferencesEntry(p, mechOid);
	MechanismFactory retVal 
	    = (MechanismFactory) factories.get(searchEntry);
	if (retVal == null) {
	    /*
	     * Apparently not. Now try to instantiate this class from
	     * the provider.
	     */
	    String prop = PROV_PROP_PREFIX + mechOid.toString();
	    String className = p.getProperty(prop);
	    if (className != null) {
		retVal = getMechFactoryImpl(p, className);
		factories.put(searchEntry, retVal);
	    } else {
		/*
		 * This provider does claim to support this
		 * mechanism. If the application explicitly requested
		 * that this provider be used for this mechanism, then
		 * throw an exception
		 */
		if (e.getOid() != null)
		    throw new GSSExceptionImpl(GSSException.BAD_MECH,
					   "Provider " + p.getName()
					   + " does not support mechanism "
					   + mechOid.toString());
	    }
	}
	return retVal;
    }

    /**
     * Helper routine to obtain a MechanismFactory implementation
     * from the same class loader as the provider of this
     * implementation.
     * @param p the provider whose classloader must be used for
     * instantiating the desired MechanismFactory
     * @ param className the name of the MechanismFactory class
     * @throws GSSException If some error occurs when trying to
     * instantiate this MechanismFactory.
     */
    private static MechanismFactory getMechFactoryImpl(Provider p,
						       String className) 
	throws GSSException {
	
	String errPrefix = "class configured by provider "
	    + p.getName() + " for GSS-API Mechanism Factory: " 
	    + className;

	try {
	    Class baseClass = Class.forName(SPI_MECH_FACTORY_TYPE);
	    
	    /*  
	     * Load the implementation class with the same class loader
	     * that was used to load the provider.
	     * In order to get the class loader of a class, the
	     * caller's class loader must be the same as or an ancestor of
	     * the class loader being returned. Otherwise, the caller must
	     * have "getClassLoader" permission, or a SecurityException
	     * will be thrown.
	     */

	    ClassLoader cl = p.getClass().getClassLoader();
	    Class implClass;
	    if (cl != null) {
		implClass = cl.loadClass(className);
	    } else {
		implClass = Class.forName(className);
	    }
	    
	    if (baseClass.isAssignableFrom(implClass)) {
		return (MechanismFactory) implClass.newInstance();
	    } else {
		throw new GSSExceptionImpl(GSSException.BAD_MECH,
				       errPrefix
				       + " is not a " 
				       + SPI_MECH_FACTORY_TYPE);
	    }
	} catch (ClassNotFoundException e) {
	    throw new GSSExceptionImpl(GSSException.BAD_MECH,
				   errPrefix
				   +   "cannot be found - "
				   + e.getMessage());
	} catch (InstantiationException e) {
	    throw (GSSExceptionImpl) new GSSExceptionImpl(GSSException.BAD_MECH,
				   errPrefix
				   + " cannot be instantiated ").initCause(e);
	} catch (IllegalAccessException e) {
	    throw new GSSExceptionImpl(GSSException.BAD_MECH,
				   errPrefix
				   + " cannot be accessed - "
				   + e.getMessage());
	} catch (SecurityException e) {
	    throw new GSSExceptionImpl(GSSException.BAD_MECH, errPrefix
				   + " cannot be accessed - "
				   + e.getMessage());
	}
   }
    
    public Oid getDefaultMechanism() {
	return DEFAULT_MECH_OID;
    }

    public Oid[] getMechs() {
	return (Oid[]) mechs.toArray(new Oid[]{});
    }

    synchronized public void addProviderAtFront(Provider p, Oid mechOid) 
	throws GSSException {

	PreferencesEntry newEntry = new PreferencesEntry(p, mechOid);
	PreferencesEntry oldEntry;
	boolean foundSomeMech;

	Iterator list = preferences.iterator();
	while (list.hasNext()) {
	    oldEntry = (PreferencesEntry) list.next();
	    /*
	      System.out.println("addProviderAtFront: " 
	      + newEntry
	      + " implies "
	      + oldEntry
	      + "?");
	    */
	    if (newEntry.implies(oldEntry))
		list.remove();
	}

	if (mechOid == null)
	    foundSomeMech = addAllMechsFromProvider(p);
	else {
	    String oidStr = mechOid.toString();
	    if (p.getProperty(PROV_PROP_PREFIX + oidStr)
		== null)
		throw new GSSExceptionImpl(GSSException.BAD_MECH,
				       "Provider " + p.getName()
				       + " does not support "
				       + oidStr);
	    mechs.add(mechOid);
	    foundSomeMech = true;
	}

	if (foundSomeMech)
	    preferences.add(0, newEntry);

    }

    synchronized public void addProviderAtEnd(Provider p, Oid mechOid) 
	throws GSSException {

	PreferencesEntry newEntry = new PreferencesEntry(p, mechOid);
	PreferencesEntry oldEntry;
	boolean foundSomeMech;

	Iterator list = preferences.iterator();
	while (list.hasNext()) {
	    oldEntry = (PreferencesEntry) list.next();
	    /*
	      System.out.println("addProviderAtEnd: " 
	      + oldEntry
	      + " implies "
	      + newEntry
	      + "?");
	    */
	    if (oldEntry.implies(newEntry))
		return;
	}

	// System.out.println("addProviderAtEnd: No it is not redundant");
	
	if (mechOid == null)
	    foundSomeMech = addAllMechsFromProvider(p);
	else {
	    String oidStr = mechOid.toString();
	    if (p.getProperty(PROV_PROP_PREFIX + oidStr)
		== null)
		throw new GSSExceptionImpl(GSSException.BAD_MECH,
				       "Provider " + p.getName()
				       + " does not support "
				       + oidStr);
	    mechs.add(mechOid);
	    foundSomeMech = true;
	}

	if (foundSomeMech)
	    preferences.add(newEntry);

    }

    /**
     * Helper routine to go through all properties contined in a
     * provider and add its mechanisms to the list of supported
     * mechanisms. If no default mechanism has been assinged so far,
     * it sets the default MechanismFactory and Oid as well.
     * @param p the provider to query
     * @return true if there is at least one mechanism that this
     * provider contributed, false otherwise
     */
    private boolean addAllMechsFromProvider(Provider p) {
	
	String prop;
	boolean retVal = false;
	
	// Get all props for this provider
	Enumeration props = p.keys();

	// See if there are any GSS prop's
	while (props.hasMoreElements()) {
	    prop = (String) props.nextElement();
	    if (isMechFactoryProperty(prop)) {
		// Ok! This is a GSS provider!
		try {
		    Oid mechOid = getOidFromMechFactoryProperty(prop);
		    if (defaultMechFactory == null && 
			DEFAULT_MECH_OID.equals(mechOid)) {
			String className = p.getProperty(prop);
			defaultMechFactory =
			    getMechFactoryImpl(p, className);
			PreferencesEntry factoryEntry =
			    new PreferencesEntry(p, DEFAULT_MECH_OID);
			factories.put(factoryEntry,
				      defaultMechFactory);
		    }
		    mechs.add(mechOid);
		    retVal = true;
		} catch (GSSException e) {
		    // Skip to next property
		}
	    } // Processed GSS property
	} // while loop

	return retVal;

    }

    /**
     * Stores a provider and a mechanism oid indicating that the
     * provider should be used for the mechanism. If the mechanism
     * Oid is null, then it indicates that this preference holds for
     * any mechanism.<p>
     *
     * The ProviderList maintains an ordered list of
     * PreferencesEntry's and iterates thru them as it tries to
     * instantiate MechanismFactory's.
     */
    final class PreferencesEntry {
	private Provider p;
	private Oid oid;
	public PreferencesEntry(Provider p, Oid oid) {
	    this.p = p;
	    this.oid = oid;
	}

	public boolean equals(Object other) {
	    if (other instanceof PreferencesEntry) {
		return equals((PreferencesEntry) other);
	    } else {
		return false;
	    }
	}

	public boolean equals(PreferencesEntry other) {

	    if (p.getName().equals(other.p.getName())) {

		if (oid != null && other.oid != null) {
		    return oid.equals(other.oid);
		} else {
		    return (oid == null && other.oid == null);
		}
	    }

	    return false;

	}

	/**
	 * Determines if a preference implies another. A preference
	 * implies another if the latter is subsumed by the
	 * former. e.g., <Provider1, null> implies <Provider1, OidX>
	 * because the null in the former indicates that it should
	 * be used for all mechanisms.
	 */
	public boolean implies(Object other) {

	    if (other instanceof PreferencesEntry) {
		PreferencesEntry temp = (PreferencesEntry) other;
		return (equals(temp) ||
			p.getName().equals(temp.p.getName()) &&
			oid == null);
	    } else {
		return false;
	    }
	}

	public Provider getProvider() {
	    return p;
	}

	public Oid getOid() {
	    return oid;
	}

	/**
	 * Determines if this entry is applicable to the desired
	 * mechanism. The entry is applicable to the desired mech if
	 * it contains the same oid or if it contains a null oid
	 * indicating that it is applicable to all mechs.
	 * @param mechOid the desired mechanism
	 * @return true if the provider in this entry should be
	 * queried for this mechanism.
	 */
	public boolean impliesMechanism(Oid oid) {
	    return (this.oid == null || this.oid.equals(oid));
	}

	// For debugging
	public String toString() {
	    StringBuffer buf = new StringBuffer("<");
	    buf.append(p.getName());
	    buf.append(", ");
	    buf.append(oid);
	    buf.append(">");
	    return buf.toString();
	}
	
    }

    /*
    public static void main(String[] args) throws GSSException {

	Provider p = new sun.security.jgss.SunProvider();
	//	GSSManager mgr = GSSManager.getInstance();
	//	mgr.addProviderAtEnd(p, null);

	ProviderList list = new ProviderList();
	list.addProviderAtFront(p, null);
	System.out.println("Default mechanism oid is: " + DEFAULT_MECH_OID);
	System.out.println("Default mechanism's factory is: " 
			   + list.defaultMechFactory.getClass());
	System.out.println("Provider and Mechanism preferences are:");
	Iterator itr = list.preferences.iterator();
	while (itr.hasNext())
	    System.out.println(itr.next());
	System.out.println("Factories created so far are:");
	itr = list.factories.entrySet().iterator();
	while (itr.hasNext())
	    System.out.println(itr.next());

    }
    */
}

