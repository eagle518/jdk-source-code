/*
 * @(#)ReferenceWrapper.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.rmi.registry;


import java.rmi.*;
import java.rmi.server.UnicastRemoteObject;

import javax.naming.*;


/**
 * The ReferenceWrapper class is a Remote wrapper for Reference
 * objects.  It wraps around a Reference on the server, and makes the
 * Reference accessible to clients.
 *
 * @author Scott Seligman
 * @version 1.9 10/03/23
 */


public class ReferenceWrapper
	extends UnicastRemoteObject
	implements RemoteReference
{
    protected Reference wrappee;	// reference being wrapped

    public ReferenceWrapper(Reference wrappee)
	    throws NamingException, RemoteException
    {
	this.wrappee = wrappee;
    }

    public Reference getReference() throws RemoteException {
	return wrappee;
    }

    private static final long serialVersionUID = 6078186197417641456L;
}
