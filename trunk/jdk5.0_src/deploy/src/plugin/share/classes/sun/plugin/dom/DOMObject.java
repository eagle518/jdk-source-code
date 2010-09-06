/*
 * @(#)DOMObject.java	1.5 01/12/03
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom;

import org.w3c.dom.*;
import sun.plugin.dom.exception.*;
import netscape.javascript.JSException;
import netscape.javascript.JSObject;


/** 
 * <p> Derived from abstract class netscape.javascript.JSObject so applet 
 * can access the JavaScript Document Object Model in the browser. This
 * is a base class that will be used by both IE and NS in Java Plug-in.
 * By default, all methods will throw JSException if the method is not 
 * overriden by the derived class.
 * </p>
 */
public class DOMObject  {
    // Underlying JSObject
    private JSObject jsobj;

    /**
     * Construct a DOMObject using a JSObject.
     */
    public DOMObject(JSObject jsobj) {
	this.jsobj = jsobj;
    }

    public void lock() {
	if(this.jsobj instanceof sun.plugin.javascript.JSObject) {
	    ((sun.plugin.javascript.JSObject)jsobj).lock();
	}
    }

	public boolean equals(Object other) {
		if(!(other instanceof DOMObject)) {
			return false;
		}

		return jsobj.equals(((DOMObject)other).jsobj);

	}

    /**
     * <p> Calls a JavaScript method. Equivalent to 
     * "this.methodName(args[0], args[1], ...)" in JavaScript.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @return Result of the method.
     */
    public Object call(String methodName, Object args[]) throws DOMException {
	// Check thread access
	checkThreadAccess();

	try {
	    // Unwrap all parameters
	    Object[] params = null;

	    if (args != null) {
		params = new Object[args.length];

		for (int i=0; i < args.length; i++)
		    params[i] = unwrapObject(args[i]);
	    }

	    // Delegate the call to JSObject
	    Object ret = jsobj.call(methodName, params);
	    return wrapObject(ret);

	} catch (JSException e) {
	    throw new BrowserNotSupportedException(e.toString());
	}
    }

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to 
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws DOMException {
	// Check thread access
	checkThreadAccess();

	try {
	    // Delegate the call to JSObject
	    Object ret = jsobj.getMember(name);

	    // Wrap JSObject
	    return wrapObject(ret);
	}
	catch (JSException e) {
	    throw new BrowserNotSupportedException(e.toString());
	}
    }

    /** 
     * <p> Sets a named member of a JavaScript object. Equivalent to 
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) throws DOMException {
	// Check thread access
	checkThreadAccess();

	// Unwrap JSObject
	value = unwrapObject(value);

	try {
	    // Delegate the call to JSObject
	    jsobj.setMember(name, value);
	} catch (JSException e) {
	    throw new BrowserNotSupportedException(e.toString());
	}
    }

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public void removeMember(String name) throws DOMException {
	// Check thread access
	checkThreadAccess();

	try {
	    // Delegate the call to JSObject
	    jsobj.removeMember(name);
	} catch (JSException e) {
	    throw new BrowserNotSupportedException(e.toString());
	}
    }

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to 
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) throws DOMException  {
	// Check thread access
	checkThreadAccess();

	try {
	    // Delegate the call to JSObject
	    Object ret = jsobj.getSlot(index);

	    // Wrap JSObject
	    return wrapObject(ret);
	} catch (JSException e) {
	    throw new BrowserNotSupportedException(e.toString());
	}
    }

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to 
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) throws DOMException {
	// Check thread access
	checkThreadAccess();

	// Unwrap JSObject
	value = unwrapObject(value);

	try {
	    // Delegate the call to JSObject
	    jsobj.setSlot(index, value);
	} catch (JSException e) {
	    throw new BrowserNotSupportedException(e.toString());
	}
    }

    public String toString() {
	Object obj = getJSObject();
	if(obj == null)
	    return null;

	return obj.toString();
    }


    /**
     * Throws exception if DOM object is not accessed from the
     * DOM dispatch thread.
     */
    private void checkThreadAccess() throws InvalidAccessException {

	// Check if sun.plugin.dom.DOMService on the stack. If so,
	// It is called from the DOMService.invokeLater or 
	// DOMService.invokeAndWait.
	//
/*	SecurityManager sm = System.getSecurityManager();

	if (sm != null && sm instanceof sun.plugin.security.ActivatorSecurityManager) {
	    // Check if DOMService class is on the stack
	    sun.plugin.security.ActivatorSecurityManager mgr = (sun.plugin.security.ActivatorSecurityManager) sm;

	    Class[] clazz = mgr.getExecutionStackContext();

	    // Walk up the stack to find a class loader.
	    for (int i=0; i < clazz.length; i++) {
		Class c = clazz[i];
	    
		if (sun.plugin.dom.DOMService.class.isAssignableFrom(c))
		    return;
	    }
	}

	throw new InvalidAccessException("Invalid access to DOM object from non-dispatch thread.");
*/    }

    /** 
     * Wraps object into DOMObject if the type is JSObject.
     */
    private Object wrapObject(Object obj) {
	if (obj == null)
	    return obj;

	if (obj instanceof JSObject) {
	    return new DOMObject((JSObject) obj);
	}

	return obj;
    }

    /**
     * Unwraps object from DOMObject into JSObject.
     */
    private Object unwrapObject(Object obj) {
	if (obj == null)
	    return obj;

	if (obj instanceof DOMObject) {
	    DOMObject dObj = (DOMObject) obj;

	    return dObj.getJSObject();
	}

	return obj;
    }


    /**
     * Returns underlying JSObject.
     */
    public Object getJSObject() {
	return jsobj;
    }
}
