/*
 * @(#)Window.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import sun.plugin.javascript.navig.Document;
import sun.plugin.javascript.navig.Navigator;
import sun.plugin.javascript.navig.History;


/** 
 * <p> Emulate the Window object in the JavaScript Document Object Model
 * in Navigator 3.x.
 * </p>
 */
public class Window extends sun.plugin.javascript.navig.JSObject {

   /**
    * <p> Method table contains all method info in the Window object. </p>
    */
   private static HashMap methodTable = new HashMap();

   /**
    * <p> Field table contains all properties info in the Window object. </p>
    */
   private static HashMap fieldTable = new HashMap();

   static {

	// Initialize all method and field info in the Window object.
	//
	methodTable.put("alert",	Boolean.FALSE);
	methodTable.put("blur",		Boolean.FALSE);
	methodTable.put("clearTimeout",	Boolean.FALSE);
	methodTable.put("close",	Boolean.FALSE);
	methodTable.put("confirm",	Boolean.TRUE);
	methodTable.put("focus",	Boolean.FALSE);
	methodTable.put("open",		Boolean.TRUE);
	methodTable.put("prompt",	Boolean.TRUE);
	methodTable.put("scroll",	Boolean.FALSE);
	methodTable.put("setTimeout",	Boolean.TRUE);

	fieldTable.put("closed",	Boolean.FALSE);
	fieldTable.put("defaultStatus",	Boolean.FALSE);
	fieldTable.put("frames",	Boolean.FALSE);
	fieldTable.put("history",	Boolean.FALSE);
	fieldTable.put("length",	Boolean.FALSE);
	fieldTable.put("location",	Boolean.FALSE);
	fieldTable.put("name",		Boolean.TRUE);
	fieldTable.put("navigator",	Boolean.FALSE);
	fieldTable.put("opener",	Boolean.TRUE);
	fieldTable.put("parent",	Boolean.TRUE);
	fieldTable.put("self",	        Boolean.FALSE);
	fieldTable.put("status",	Boolean.TRUE);
	fieldTable.put("top",		Boolean.TRUE);
	fieldTable.put("window",	Boolean.FALSE);
    }

    /** 
     * <p> Counter for helping generating window name when a new 
     * window is created.
     * </p>
     */
    private static long varCount = 0;

    /**
     * <p> Construct a new Window object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    public Window(int instance, String context)  {
	super(instance, context);
	addObjectTable(fieldTable, methodTable);
    }


    /**
     * <p> Construct a new Window object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     */
    public Window(int instance) {
	this(instance, "self");
    }

    /**
     * <p> Invokes a named method of the Window object.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     */
    public Object call(String methodName, Object args[]) throws JSException {

	if (methodName.equals("open"))
	{
	    // This is a very special case. Window.open is a method that
	    // explicitly open a new window and return the Window object.
	    // Since we have no access to the JavaScript engine inside 
	    // the browser, all we can do is to use Window's name as a 
	    // way to associate the new window.
	    //
	    Object params[];

	    if (args == null || args.length == 0)  {
		params = new Object[] { "", generateVarName("__pluginwin")};
	    }
	    else if (args.length == 1)  {
		params = new Object[] { args[0], generateVarName("__pluginwin")};
	    }
	    else    {
		params = args;
	    }

    	    // Call the function and get back [object Window]
	    Object ret = super.call(methodName, params);

	    if (ret == null)  {
		// Cannot open new window.
		throw new JSException("call does not support " + toString() + "." + methodName);
	    }

	    // If it is successful, now this is the tricky part. 
	    // We must create a new context relative to the window name.
	    String winContext = context + ".open(";

	    // Construct argument lists.
    	    for (int i=0; i < params.length; i++)
	    {
		if (params[i] instanceof java.lang.String)
		    winContext += "'" + params[i].toString() + "'";
		else
    	    	    winContext += params[i].toString();

		if (i != params.length - 1)
		    winContext += ", ";
	    }
	
	    winContext += ")";

	    // Create new window object based on this new context
	    return resolveObject(JSType.Window, winContext);
	}

	try {
    	    return super.call(methodName, args);
	} catch (JSException e) {
	    // If cannot call the default methods in the Window object, it may 
	    // be a custom method.

	    // Test if the function is there
	    if (eval(context + "." + methodName) == null)
		throw new JSException("call does not support " + toString() + "." + methodName);

	    // Otherwise, the function exists.
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

	    // Generate variable name.
	    String varName = generateVarName("__pluginVar");
	    Object result = eval(varName + "=" + str);
	    
	    if (result != null)   {
    		// Convert return type to JSObject, if possible.
    		result = resolveObject(result.toString().trim(), context + "." + varName);
	    }

	    return result;
	}
    }

    /**
     * <p> Retrieves a named member of the Window object. Equivalent to 
     * "self.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
     public Object getMember(String name) throws JSException {

	// Deal with special properties first.
	//
	
	if (name.equals("document"))  {
	    return resolveObject(JSType.Document, context + ".document");
	}
	else if (name.equals("history"))  {
	    return resolveObject(JSType.History, context + ".history");
	}
	else if (name.equals("location"))  {
	    return resolveObject(JSType.Location, context + ".location");
	}
	else if (name.equals("frames"))	{
	    return resolveObject(JSType.FrameArray, context + ".frames");
	}
	else if (name.equals("navigator"))  {
	    return resolveObject(JSType.Navigator, "navigator");
	}
	else if (name.equals("self"))
	    return this;
	else if (name.equals("window"))
	    return this;
	else if (name.equals("parent"))
	    return resolveObject(JSType.Window, context + ".parent");
	else if (name.equals("top"))
	    return resolveObject(JSType.Window, context + ".top");
	else if (name.equals("opener"))
	    return resolveObject(JSType.Window, context + ".opener");

	// Try to match the name of the property with the field table.
	//
	try {
    	    return super.getMember(name);
	} catch (JSException e)  {
	    // If cannot get the default property in the Window object, it may 
	    // be a custom property.

	    // Generate variable name.
	    String varName = generateVarName("__pluginVar");

	    Object result = eval(varName + "=" + context + "." + name);

	    if (result != null)  {
		// Convert return type to JSObject, if possible.
		result = resolveObject(result.toString().trim(), context + "." + varName);
	    }

	    return result;
	}
    }

    /** 
     * <p> Sets a named member of the Window object. Equivalent to 
     * "self.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
     public void setMember(String name, Object value) throws JSException {

	try {
	    super.setMember(name, value);
	} catch (JSException e)  {
	    // If cannot set default property in the Window object, it may 
	    // be a custom property.
	    if (value instanceof java.lang.String)   
		eval(context + "." + name + "='" + value.toString() + "'");
	    else  
		eval(context + "." + name + "=" + value.toString());
	}
    }

    
    /** 
     * <p> Helper function for generating new varaible name with prefix.
     * </p>
     *
     * @param prefix Variable name prefix.
     * @return New variable name.
     */
    protected static String generateVarName(String prefix)  {
	if (prefix != null)
	    return prefix + varCount++;
	else
	    return "__pluginTemp" + varCount++;
    }
}
