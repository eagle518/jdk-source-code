/*
 * @(#)ServiceManager.java	1.13 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

/**
 * The <code>ServiceManager</code> provides static methods to lookup JNLP services. This class
 * is abstract and final and cannot be instantiated.
 * <p>
 * Requests are delegated to a <code>ServiceManagerStub</code>
 * object. This object must be set by the JNLP Client on startup using the
 * <code>setServiceManagerStub</code> method.
 *
 * @since 1.0
 *
 * @see ServiceManagerStub
 */
public final class ServiceManager {
    static private ServiceManagerStub _stub = null;
    
    /** Private constructor in order to prevent instantiation */
    private ServiceManager() { /* dummy */ }

    /**
     * Asks the JNLP Client for a service with a given name. The lookup
     * must be idempotent, that is return the same object for each invocation
     * with the same name.
     *
     * @param name  Name of service to lookup.
     *
     * @return     An object implementing the service. <code>null</code>
     *             will never be returned. Instead an exception will be thrown.
     *
     * @exception  <code>UnavailableServiceException</code> if the service is not available, or if <code>name</code> is null.
     */
    public static Object lookup(String name) throws UnavailableServiceException {
	if (_stub != null) {
	    return _stub.lookup(name);
	} else {
	    throw new UnavailableServiceException("uninitialized");
	}
    }
    
    /**
     * Returns the names of all services implemented by the JNLP Client.
     */
    public static String[] getServiceNames() {
	if (_stub != null) {
	    return _stub.getServiceNames();
	} else {
	    return null;
	}
    }
    
    /**
     * Sets the object that all <code>lookup</code> and <code>getServiceNames</code>
     * requests are delegated to. The <code>setServiceManagerStub</code> call is ignored
     * if the stub has already been set.
     * <p>
     * This method should be called exactly once by the JNLP Client, and never be
     * called by a launched application.
     *
     * @param stub The ServiceManagerStub object to delegate to
     */
    public static synchronized void setServiceManagerStub(ServiceManagerStub stub) {
	if (_stub == null) {
	    _stub = stub;
	}
    }
}

