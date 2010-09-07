/*
 * @(#)DOMService.java	1.3 02/10/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import com.sun.java.browser.dom.*;


public final class DOMService extends com.sun.java.browser.dom.DOMService
{
    /**
     * An empty constructor is provided. Implementations of this
     * abstract class must provide a protected no-argument constructor
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
    public Object invokeAndWait(DOMAction action) throws DOMAccessException
    {
	return action.run(new sun.plugin.dom.DOMAccessor());
    }


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
    public void invokeLater(DOMAction action)
    {
	action.run(new sun.plugin.dom.DOMAccessor());
    }
}


class DOMAccessor implements com.sun.java.browser.dom.DOMAccessor
{
    com.sun.java.browser.dom.DOMServiceProvider provider = new sun.plugin.dom.DOMServiceProvider();

    public DOMAccessor()
    {
    }

    /**
     * Returns the Document object of the DOM.
     */
    public org.w3c.dom.Document getDocument(Object obj) throws org.w3c.dom.DOMException
    {
	try
	{
	    return provider.getDocument(obj);
	}
	catch(Exception e)
	{
	    e.printStackTrace();
	    return null;
	}
    }

    /**
     * Returns a DOMImplementation object.
     */
    public org.w3c.dom.DOMImplementation getDOMImplementation()
    {
	return provider.getDOMImplementation();	
    }
}            
