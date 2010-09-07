/*
 * @(#)JSObject.java	1.25 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.Iterator;
import java.util.HashMap;
import java.util.LinkedList;
import netscape.javascript.JSException;
import com.sun.deploy.util.Trace;

/** 
 * <p> sun.plugin.javascript.navig.JSObject is a base class for all
 * JSObject representation in Navigator inside Java Plug-in. e.g.
 * Window, Document, Location, ..., and so on. 
 * </p>
 */
public class JSObject extends sun.plugin.javascript.JSObject {

    /**
     * <p> Evaluation context. </p>
     */
    protected String context;
     
    /**
     * <p> Native plugin instance handle. </p>
     */
    protected int instance;
    
    /**
     * <p> Method vectors to store all methods info. </p>
     */
    private LinkedList methodVector = new LinkedList();

    /**
     * <p> Field vectors to store all fields info. </p>
     */
    private LinkedList fieldVector = new LinkedList();

    /**
     * <p> JSObject resolver for creating JSObject from a string. </p>
     */
    private static JSObjectResolver resolver = null;


    /**
     * <p> Construct a JSObject.
     * </p>
     *
     * @param instance Native plugin instance handle.
     * @param context Evaluation context.
     */
    protected JSObject(int instance, String context)  {
	this.instance = instance;
	this.context = context;
    }

    /**
     * <p> Retrieves a named member of a JavaScript object based on the fieldTable. 
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

	try  {

	    boolean found = false;

	    synchronized(fieldVector)
	    {
		for (Iterator iter = fieldVector.iterator() ; iter.hasNext() ;) {

		    HashMap fieldTable = (HashMap) iter.next();
		    Boolean b = (Boolean) fieldTable.get(name);

		    // Test if the property exists
		    if (b != null)
		    {	
			found = true;
			break;
		    }
		}
	    }

	    // It is VERY important to dispatch the call outside
	    // synchronization block, or it may become a deadlock	
	    //
	    if (found)
		return evaluate(context + "." + name);   

	} catch (Throwable e)  {
	}

	return super.getMember(name);
    }

    /** 
     * <p> Sets a named member of a JavaScript object based on the fieldTable.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) throws JSException {

	try {
		if(value == null)
			value = "";

	    boolean found = false;

	    synchronized(fieldVector)
	    {
		for (Iterator iter = fieldVector.iterator() ; iter.hasNext() ;) {

		    HashMap fieldTable = (HashMap) iter.next();
		    Boolean val = (Boolean) fieldTable.get(name);

		    // Test if the property exists and is writable
		    if (val != null && val.booleanValue())	
		    {
			found = true;
			break;
		    }	
		}
	    }
	    
	    // It is VERY important to dispatch the call outside
	    // synchronization block, or it may become a deadlock	
	    //
	    if (found)
	    {		
		if (value instanceof java.lang.String)    
    		    evaluate(context + "." + name + "='" + value.toString() + "'");   
		else
    		    evaluate(context + "." + name + "=" + value.toString());   
		return;
	    }

	} catch (Throwable e)  {
	}

	super.setMember(name, value);
    }

    /**
     * <p> Calls a JavaScript method based on the methodTable.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @return Result of the method.
     */
    public Object call(String methodName, Object[] args) 
    throws JSException 
    {
	try {
    
	    boolean found = false;

	    synchronized(methodVector)
	    {
		for (Iterator iter = methodVector.iterator() ; iter.hasNext() ;) {

		    HashMap methodTable = (HashMap) iter.next();
		    Boolean val = (Boolean) methodTable.get(methodName);

		    // Check if the method exists
		    if (val != null)	
		    {
			found = true;
			break;
		    }
		}
	    }

	    // It is VERY important to dispatch the call outside
	    // synchronization block, or it may become a deadlock	
	    //
	    if (found)
	    {
		String str = context + "." + methodName + "(";

		// Construct argument lists.
		if (args != null)
		{
		    for (int i=0; i < args.length; i++)
		    {
			if (args[i] instanceof java.lang.String)
			    str += "'" + args[i].toString() + "'";
			else
    			    str += args[i].toString();

			if (i != args.length - 1)
			    str += ", ";
		    }
		}
		
		str += ")";

  		return evaluate(str); 
	    }
	} catch (Throwable e)  {
	}


	return super.call(methodName, args);
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
    public Object eval(String s) throws JSException {
	return evaluate(s);
    }


    /** 
     * <p> Converts a JSObject to a String. 
     * </p>
     *
     * @return The string representation of the JSObject.
     */
    public String toString() {
	return context;
    }

    
    /**
     * <p> Evaluate a JavaScript expression. Wait for the result until it is done.
     * </p>
     *
     * @param s JavaScript Expression.
     * @return Result of the evaluation.
     */    
    protected synchronized Object evaluate(String s) throws JSException  {
	String res = evalScript(instance, "javascript: " + s);
        Trace.msgLiveConnectPrintln("jsobject.eval",  new Object[] {s, res});
	return res;
    }


    /** 
     * <p> Native method for evaluating the JavaScript Expression.
     * </p>
     *
     * @param instance Native plugin instance.
     * @param s The JavaScript Expression.
     * @param waitTime The time-out value for waiting for the result.
     * @return String The result of the evaluation in string.
     */
    public native String evalScript(int instance, String s);


    /** 
     * <p> Return a JSObject resolver. </p>
     */
    protected static JSObjectResolver getResolver()  {
	if (resolver == null)
	    resolver = new JSObjectFactory();

	return resolver;
    }


    /** 
     * <p> Set a JSObject resolver. </p>
     *
     * @param res JSObject resolver.
     */
    protected static void setResolver(JSObjectResolver res) throws JSException  {
	if (resolver == null)
	    resolver = res;
	else
	    throw new JSException("JSObject resolver already exists.");
    }


    /** 
     * <p> Resolve a JSObject from a string.
     * </p>
     * 
     * @param type String containing JSObject type.
     * @param context Evaluation context.
     * @param custom Custom object for object creation.
     * @return Resulting object.
     */
    protected Object resolveObject(String type, String context, Object custom) 
    throws JSException
    {
	JSObjectResolver res = getResolver();
	return res.resolveObject(this, type, instance, context, custom);
    }


    /** 
     * <p> Resolve a JSObject from a string.
     * </p>
     * 
     * @param type String containing JSObject type.
     * @param context Evaluation context.
     * @return Resulting object.
     */
    protected Object resolveObject(String type, String context) throws JSException  {
	return resolveObject(type, context, null);
    }

    
    /** 
     * <p> Add field table and method table to the JSObject, so "setMember", "getMember"
     * and "call" will work according to the resulting field and method tables.
     * </p>
     *
     * @param fieldTable Field table.
     * @param methodTable Method table.
     */
    protected final void addObjectTable(HashMap fieldTable, HashMap methodTable)  {
	if (fieldTable != null)
	{
	    synchronized (fieldVector)
	    {
		fieldVector.add(0, fieldTable);
	    }
	}

	if (methodTable != null)
	{
	    synchronized (methodVector)
	    {
	        methodVector.add(0, methodTable);
	    }
	}
    }
}
