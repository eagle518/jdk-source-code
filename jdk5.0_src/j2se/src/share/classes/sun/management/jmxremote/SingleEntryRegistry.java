/*
 * @(#)SingleEntryRegistry.java	1.3 04/07/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/* 
 * @(#)file      SingleEntryRegistry.java 
 * @(#)author    Sun Microsystems, Inc. 
 * @(#)version   1.3 
 * @(#)date      04/07/26 
 * @(#)build	 @BUILD_TAG_PLACEHOLDER@
 * 
 * @COPYRIGHT_MINI_LEGAL_NOTICE_PLACEHOLDER@
 */ 

package sun.management.jmxremote;

import java.rmi.AccessException;
import java.rmi.NotBoundException;
import java.rmi.Remote;
import java.rmi.RemoteException;
import java.rmi.server.RMIClientSocketFactory;
import java.rmi.server.RMIServerSocketFactory;

import sun.rmi.registry.RegistryImpl;

/** A Registry that consists of a single entry that never changes. */
public class SingleEntryRegistry extends RegistryImpl {
    SingleEntryRegistry(int port, String name, Remote object)
	    throws RemoteException {
	super(port);
	this.name = name;
	this.object = object;
    }

    SingleEntryRegistry(int port,
			RMIClientSocketFactory csf,
			RMIServerSocketFactory ssf,
			String name,
			Remote object)
	    throws RemoteException {
	super(port, csf, ssf);
	this.name = name;
	this.object = object;
    }

    public String[] list() {
	return new String[] {name};
    }

    public Remote lookup(String name) throws NotBoundException {
	if (name.equals(this.name))
	    return object;
	throw new NotBoundException("Not bound: \"" + name + "\" (only " +
				    "bound name is \"" + this.name + "\")");
    }

    public void bind(String name, Remote obj) throws AccessException {
	throw new AccessException("Cannot modify this registry");
    }

    public void rebind(String name, Remote obj) throws AccessException {
	throw new AccessException("Cannot modify this registry");
    }

    public void unbind(String name) throws AccessException {
	throw new AccessException("Cannot modify this registry");
    }

    private final String name;
    private final Remote object;
}
