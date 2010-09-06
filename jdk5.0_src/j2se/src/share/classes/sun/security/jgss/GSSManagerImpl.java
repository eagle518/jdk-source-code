/*
 * @(#)GSSManagerImpl.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
package sun.security.jgss;  
  
import org.ietf.jgss.*; 
import sun.security.jgss.spi.*;
import java.io.*;
import java.security.NoSuchProviderException;
import java.security.Provider;
import java.security.Security;
import java.util.Hashtable;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.Vector;
import java.util.Enumeration;

/**
 * This class provides the default implementation of the GSSManager
 * interface.
 */
public class GSSManagerImpl extends GSSManager {

    private ProviderList list = new ProviderList();

    public  Oid[] getMechs(){
	return list.getMechs();
    }

    public Oid[] getNamesForMech(Oid mech)
	throws GSSException {

	MechanismFactory factory = list.getMechFactory(mech);
	return factory.getNameTypes();
    }
    
    public Oid[] getMechsForName(Oid nameType){
	Oid[] mechs = getMechs();
	Oid[] retVal = new Oid[mechs.length];
	int pos = 0;

	// Iterate thru all mechs in GSS

	for (int i = 0; i < mechs.length; i++) {
	    // what nametypes does this mech support?
	    try {
		Oid[] namesForMech = getNamesForMech(mechs[i]);
	    // Is the desired Oid present in that list?
		if (nameType.containedIn(namesForMech)) {
		    retVal[pos++] = mechs[i];
		}
	    } catch (GSSException e) {
		// Squelch it and just skip over this mechanism
	    }
	}

	// Trim the list is needed

	if (pos < retVal.length) {
	    Oid[] temp = new Oid[pos];
	    for (int i = 0; i < pos; i++)
		temp[i] = retVal[i];
	    retVal = temp;
	}

	return retVal;

    }
  
    public GSSName createName(String nameStr, Oid nameType)
	throws GSSException {
	return new GSSNameImpl(this, nameStr, nameType);
    }

    public GSSName createName(byte name[], Oid nameType)
	throws GSSException {
	return new GSSNameImpl(this, name, nameType);
    }

    public GSSName createName(String nameStr, Oid nameType,
			      Oid mech) throws GSSException{
	return new GSSNameImpl(this, nameStr, nameType, mech);
    }

    public GSSName createName(byte name[], Oid nameType, Oid mech)
	throws GSSException{
	return new GSSNameImpl(this, name, nameType, mech);
    }
  
    public GSSCredential createCredential (int usage)
	throws GSSException{
	return new GSSCredentialImpl(this, usage);
    }

    public GSSCredential createCredential (GSSName aName,
					   int lifetime, Oid mech, int usage)
	throws GSSException{
	return new GSSCredentialImpl(this, aName, lifetime, mech, usage);
    }

    public GSSCredential createCredential(GSSName aName,
					  int lifetime, Oid mechs[], int usage)
	throws GSSException{ 
	return new GSSCredentialImpl(this, aName, lifetime, mechs, usage);
    }

    public GSSContext createContext(GSSName peer, Oid mech,
				    GSSCredential myCred, int lifetime)
	throws GSSException{
	return new GSSContextImpl(this, peer, mech, myCred, lifetime);
    }

    public GSSContext createContext(GSSCredential myCred)
	throws GSSException{ 
	return new GSSContextImpl(this, myCred);
    }

    public GSSContext createContext(byte [] interProcessToken)
	throws GSSException{ 
	return new GSSContextImpl(this, interProcessToken);
    }

    public void addProviderAtFront(Provider p, Oid mech)
	throws GSSException{ 
	list.addProviderAtFront(p, mech);
    }

    public void addProviderAtEnd(Provider p, Oid mech)
	throws GSSException{
	list.addProviderAtEnd(p, mech);
    }

    GSSCredentialSpi getCredentialElement(GSSNameSpi name, 
	  int initLifetime, int acceptLifetime, Oid mech, int usage)
	throws GSSException {
	/*
	  System.out.println("GSSManagerImpl.getCredentialElement with name=" +
	  name + " mech=" + mech);
	*/
	MechanismFactory factory = list.getMechFactory(mech);
	/*
	  System.out.println("GSSManagerImpl.getCredentialElement: " +
	  "found MechanismFactory.");
	*/
	return factory.getCredentialElement(name, initLifetime,
					    acceptLifetime, usage);
	// TBD: If this throws an exception, try the same provider as
	// the name
    }

    GSSNameSpi getNameElement(Object name, Oid nameType, Oid mech)
	throws GSSException {
	MechanismFactory factory = list.getMechFactory(mech);
	if (name instanceof String)
	    return factory.getNameElement((String) name, nameType);
	else
	    return factory.getNameElement((byte[]) name, nameType);
    }

    GSSContextSpi getMechanismContext(GSSNameSpi peer, 
				      GSSCredentialSpi myInitiatorCred,
				      int lifetime, Oid mech)
	throws GSSException {
	MechanismFactory factory = list.getMechFactory(mech);
	return factory.getMechanismContext(peer, myInitiatorCred, lifetime);
	// TBD: If this throws an exception, try the same provider as
	// the name and the credential
    }
    
    GSSContextSpi getMechanismContext(GSSCredentialSpi myAcceptorCred, Oid mech) 
	throws GSSException {
	MechanismFactory factory = list.getMechFactory(mech);
	return factory.getMechanismContext(myAcceptorCred);
	// TBD: If this throws an exception, try the same provider as
	// the credential
    }
    
    GSSContextSpi getMechanismContext(byte[] exportedContext, Oid mech) 
	throws GSSException{
	MechanismFactory factory = list.getMechFactory(mech);
	return factory.getMechanismContext(exportedContext);
    }

    Oid getDefaultMechanism() {
	return list.getDefaultMechanism();
    }

    static boolean compareBytes(byte[] a, byte[] b) {

	if (a.length != b.length)
	    return false;

	for (int i = 0; i < a.length; i++) {
	    if (a[i] != b[i])
		return false;
	}

	return true;
    }

}
