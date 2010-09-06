/*
 * @(#)Dispatcher.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.server;

import java.rmi.Remote;
import java.rmi.server.RemoteCall;

/**
 * The Dispatcher interface allows the transport to make
 * the upcall to the server side remote reference.
 */
public interface Dispatcher {

    /**
     * Call to dispatch to the remote object (on the server side).
     * The up-call to the server and the marshaling of return result
     * (or exception) should be handled before returning from this
     * method.
     * @param obj the target remote object for the call
     * @param call the "remote call" from which operation and
     * method arguments can be obtained.
     * @exception RemoteException unable to marshal
     * return result 
     */
    void dispatch(Remote obj, RemoteCall call)
	throws java.io.IOException;
}
