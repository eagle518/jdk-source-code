/*
 * @(#)ActivationGroup_Stub.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package java.rmi.activation;

/**
 * <code>ActivationGroup_Stub</code> is a stub class
 * for the subclasses of <code>java.rmi.activation.ActivationGroup</code>
 * that are exported as a <code>java.rmi.server.UnicastRemoteObject</code>.
 *
 * @version	1.3, 12/19/03
 * @since	1.2
 */
public final class ActivationGroup_Stub
    extends java.rmi.server.RemoteStub
    implements java.rmi.activation.ActivationInstantiator, java.rmi.Remote
{
    /**
     * Constructs a stub for the <code>ActivationGroup</code> class.  It
     * invokes the superclass <code>RemoteStub(RemoteRef)</code>
     * constructor with its argument, <code>ref</code>.
     *
     * @param	ref a remote ref
     */
    public ActivationGroup_Stub(java.rmi.server.RemoteRef ref) {
    }
    
    /**
     * Stub method for <code>ActivationGroup.newInstance</code>.  Invokes
     * the <code>invoke</code> method on this instance's
     * <code>RemoteObject.ref</code> field, with <code>this</code> as the
     * first argument, a two-element <code>Object[]</code> as the second
     * argument (with <code>id</code> as the first element and
     * <code>desc</code> as the second element), and -5274445189091581345L
     * as the third argument, and returns the result.  If that invocation
     * throws a <code>RuntimeException</code>, <code>RemoteException</code>, 
     * or an <code>ActivationException</code>, then that exception is
     * thrown to the caller.  If that invocation throws any other
     * <code>java.lang.Exception</code>, then a
     * <code>java.rmi.UnexpectedException</code> is thrown to the caller
     * with the original exception as the cause.
     *
     * @param	id an activation identifier
     * @param	desc an activation descriptor
     * @return  the result of the invocation
     * @throws	java.rmi.RemoteException if invocation results in
     *		a <code>RemoteException</code>
     * @throws	java.rmi.activation.ActivationException if invocation
     * 		results in an <code>ActivationException</code>
     */
    public java.rmi.MarshalledObject newInstance(
 				java.rmi.activation.ActivationID id,
				java.rmi.activation.ActivationDesc desc)
	throws java.rmi.RemoteException,
	    java.rmi.activation.ActivationException
    {
	return null;
    }
}
