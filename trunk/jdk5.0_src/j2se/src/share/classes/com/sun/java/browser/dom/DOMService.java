/*
 * @(#)DOMService.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.java.browser.dom;

public abstract class DOMService 
{
    /**
     * Returns new instance of a DOMService. The implementation
     * of the DOMService returns depends on the setting of the
     * com.sun.java.browser.dom.DOMServiceProvider property or, 
     * if the property is not set, a platform specific default.
     * 
     * Throws DOMUnsupportedException if the DOMService is not 
     * available to the obj.
     *
     * @param obj Object to leverage the DOMService
     */
    public static DOMService getService(Object obj) 
		  throws DOMUnsupportedException
    {
	try
	{
	    String provider = (String) java.security.AccessController.doPrivileged(
		   new sun.security.action.GetPropertyAction("com.sun.java.browser.dom.DOMServiceProvider"));

	    Class clazz = DOMService.class.forName("sun.plugin.dom.DOMService");

	    return (DOMService) clazz.newInstance();
	}
	catch (Throwable e)
	{
	    throw new DOMUnsupportedException(e.toString());
	}
    }

    /**
     * An empty constructor is provided. Implementations of this
     * abstract class must provide a public no-argument constructor
     * in order for the static getService() method to work correctly.        
     * Application programmers should not be able to directly
     * construct implementation subclasses of this abstract subclass.
     */
    public DOMService()
    {
    }

    /**
     * Causes action.run() to be executed synchronously on the 
     * DOM action dispatching thread. This call will block until all 
     * pending DOM actions have been processed and (then) 
     * action.run() returns. This method should be used when an 
     * application thread needs to access the browser's DOM. 
     * It should not be called from the DOMActionDispatchThread. 
     * 
     * Note that if the DOMAction.run() method throws an uncaught 
     * exception (on the DOM action dispatching thread),  it's caught 
     * and re-thrown, as an DOMAccessException, on the caller's thread.
     *
     * If the DOMAction.run() method throws any DOM security related
     * exception (on the DOM action dispatching thread), it's caught
     * and re-thrown, as an DOMSecurityException, on the caller's thread.
     * 
     * @param action DOMAction.
     */    
    public abstract Object invokeAndWait(DOMAction action) throws DOMAccessException;

    /**
     * Causes action.run() to be executed asynchronously on the 
     * DOM action dispatching thread. This method should be used 
     * when an application thread needs to access the browser's 
     * DOM. It should not be called from the DOMActionDispatchThread. 
     *
     * Note that if the DOMAction.run() method throws an uncaught 
     * exception (on the DOM action dispatching thread),  it will not be
     * caught and re-thrown on the caller's thread.
     * 
     * @param action DOMAction.
     */    
    public abstract void invokeLater(DOMAction action);
}

