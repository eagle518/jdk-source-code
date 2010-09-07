/*
 * @(#)JSObject.java	1.26 02/08/20
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.ocx;

import netscape.javascript.JSException;
import sun.plugin.com.DispatchClient;
import sun.plugin.viewer.context.IExplorerAppletContext;
import com.sun.deploy.util.Trace;

/*
 * JSObject
 *
 * This class is responsible for providing an implementation of the 
 * JSObject inside Internet Explorer browsers family. 
 *
 * @author Jerome Dochez
 * @version 1.2
 */ 

public class JSObject extends  sun.plugin.javascript.JSObject {

    public static final int DISPATCH_METHOD = 0x1;
    public static final int DISPATCH_PROPERTYGET = 0x2;
    public static final int DISPATCH_PROPERTYPUT = 0x4;
    private DispatchClient dispClient = null;
    private int handle = 0;
    
    // object toString() value
    private String  infoString = null;

    /**
     * <p>
     * Create a new Instance of the JSObject interface.
     * JSObject can only be created when acting as a proxy to an 
     * IDispatch implementation
     * </p>
     *
     * @param int native implementation instance of the JSObject
     */
    public JSObject(DispatchClient client) {
	dispClient = client;
    }

    public JSObject(DispatchClient client, int handle) {
	dispClient = client;
	this.handle = handle;
    }

    /*
     * <p>
     * Set the applet context for this JSObject, this is used 
     * when this JSObject creates new JSObject as method invocation returns
     * </p>
     *
     * @param aac Applet Context which identify which applet "owns" this JSObject
     */
    public void setIExplorerAppletContext(IExplorerAppletContext aac) {
	this.aac = aac;
	this.handle = aac.getAppletContextHandle();
	aac.addJSObjectToExportedList(this);
	if(lockThis)
	    aac.addJSObjectToLockedList(this);

	this.aac = aac;
    }

    /**
     * <p>
     * finalization
     * </p>
     */
    public void finalize() {
	cleanup();
    }

    /**
     * <p>
     * Release the native implementation object
     * </p>
     */
    public synchronized void cleanup() {
	if(!released)
	{
	    dispClient.release(handle);
	    released = true;
	}
    }

    /**
     * <p> 
     * Calls a JavaScript method. Equivalent to 
     * "this.methodName(args[0], args[1], ...)" in JavaScript.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @return Result of the method.
     */
    public synchronized Object invoke(String methodName, Object args[], int flag) throws JSException 
    {
	// Check validity of DOM object
	checkValidity();

	Trace.msgLiveConnectPrintln("com.method.jsinvoke", new Object[] {methodName});

	Object result = dispClient.invoke(handle, methodName, flag, args);

	if (result != null && result instanceof sun.plugin.javascript.ocx.JSObject) {
	   ((sun.plugin.javascript.ocx.JSObject)result).setIExplorerAppletContext(aac);
	}

	return result;
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
    public Object call(String methodName, Object args[]) throws JSException {
	return invoke(methodName, args, DISPATCH_METHOD);
    }

    /** 
     * <p> Evaluates a JavaScript expression. The expression is a string of 
     * JavaScript source code which will be evaluated in the context given by 
     * "this".
     * </p>
     *
     * @param s The JavaScript expression.
     * @return Result of the JavaScript evaluation.
     */
    public synchronized Object eval(String s) throws JSException 
    {
	Object args[] = new Object[1];

	// Escape all the '\'' and '\\'s in the string s
	StringBuffer buffer = new StringBuffer();

	for (int i=0; i < s.length(); i++) {
	    char c = s.charAt(i);

	    if (c == '\'' || c == '\"' || c == '\\')
		buffer.append('\\');

	    buffer.append(c);
	}


	// Fixed #4324382   
	args[0]="evalIntermediateValueToReturn=0;evalIntermediateValueToReturn=eval('" + buffer.toString() + "');";

        netscape.javascript.JSObject jsObject = this; 

        // get a string representation of the JavaScipt object
        String strRep = toString();

        // if object is non-window type, need to get right JavaScript object
        if (strRep.indexOf("Window") == -1) {
            jsObject = aac.getJSObject();
        }

	try {
	    jsObject.call("execScript", args);
	} catch (JSException ex0) {
	    throw new JSException("Failure to evaluate " + s);
	}

	try {
	    // Exception may be thrown if there is no return value.
	    return jsObject.getMember("evalIntermediateValueToReturn");    
	} catch (JSException ex1) {
	    return null;
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
    public Object getMember(String name) throws JSException 
    {
	return invoke(name, null, DISPATCH_PROPERTYGET);
    }

    /** 
     * <p> Sets a named member of a JavaScript object. Equivalent to 
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) throws JSException {
	Object args[] = { value };
	invoke(name, args, DISPATCH_PROPERTYPUT);
    }

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public void removeMember(String name) throws JSException {
	throw new JSException("removeMember does not support " + toString() + "." + name);
    }

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to 
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) throws JSException 
    {
	Object[] args = new Object[2];
	args[0] = new Integer(index);
	args[1] = new Integer(index);

	return invoke("item", args, DISPATCH_METHOD);
    }

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to 
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) throws JSException {

	Object args[] = new Object[2];
	args[0] = new Integer(index);
	args[1] = value;

        invoke("add", args, DISPATCH_METHOD);
    }


    /**
     * <p> Retrieves the string representation of a JavaScript object. 
     * </p>
     *
     * @return The string representation.
     */
    public String toString() {
	if(infoString == null) {
	    infoString = dispClient.getDispType(handle);
	    if(infoString == null)
		infoString = objectToString();
	    else 
		infoString = "[object " + infoString + "]";
	}

	return infoString;
    }

    private String objectToString() {
	Object retVal = null;
	String strVal = null;

	try {
	    retVal = invoke("toString", null, DISPATCH_METHOD);
	} catch (JSException e)	{
	    retVal = "[object JSObject]";
	}

	if(retVal != null)
	    strVal = retVal.toString();

	return strVal;
    }

    /**
     * Check if the native DOM Object has been released.
     */
    private void checkValidity()
		 throws JSException
    {
	if (released)
	    throw new JSException("Native DOM object has been released");
    }

    public DispatchClient getDispatchClient() {
	return dispClient;
    }

    public void lock() {
	if(this.aac == null) {
	    lockThis = true;
	} else {
	    this.aac.addJSObjectToLockedList(this);
	}	    
    }

	public boolean equals(Object other) {
		if(!(other instanceof sun.plugin.javascript.ocx.JSObject))
			return false;

		sun.plugin.javascript.ocx.JSObject jsOther = (sun.plugin.javascript.ocx.JSObject)other;
		if(jsOther.handle != handle)
			return false;

		return dispClient.equals(handle, jsOther.dispClient);
	}

    private sun.plugin.viewer.context.IExplorerAppletContext aac;
    private boolean lockThis = false;
    private boolean released = false;
}

