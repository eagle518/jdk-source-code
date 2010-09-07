/*
 * @(#)Window.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.javascript.navig4;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import sun.plugin.javascript.navig4.JSObjectFactory;
import sun.plugin.javascript.navig.Array;
import sun.plugin.javascript.navig.JSType;


/** 
 * <p> Emulate the Window object in the JavaScript Document Object Model
 * in Navigator 4.x.
 * </p>
 */
public class Window extends sun.plugin.javascript.navig.Window {

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
	methodTable.put("back",			  Boolean.FALSE);
	methodTable.put("clearInterval",	  Boolean.FALSE);
	methodTable.put("disableExternalCapture", Boolean.FALSE);
	methodTable.put("enableExternalCapture",  Boolean.FALSE);
	methodTable.put("find",			  Boolean.TRUE);
	methodTable.put("forward",		  Boolean.FALSE);
	methodTable.put("home",			  Boolean.FALSE);
	methodTable.put("moveBy",		  Boolean.FALSE);
	methodTable.put("moveTo",	          Boolean.FALSE);
	methodTable.put("print",		  Boolean.FALSE);
	methodTable.put("resizeBy",		  Boolean.FALSE);
	methodTable.put("resizeTo",		  Boolean.FALSE);
	methodTable.put("scrollBy",		  Boolean.FALSE);
	methodTable.put("scrollTo",		  Boolean.FALSE);
	methodTable.put("setInterval",		  Boolean.FALSE);
	methodTable.put("stop",			  Boolean.FALSE);

	fieldTable.put("innerHeight",	Boolean.TRUE);
	fieldTable.put("innerWidth",	Boolean.TRUE);
	fieldTable.put("height",	Boolean.TRUE);
	fieldTable.put("width",		Boolean.TRUE);
	fieldTable.put("locationbar",	Boolean.FALSE);
	fieldTable.put("menubar",	Boolean.FALSE);
	fieldTable.put("outerHeight",	Boolean.TRUE);
	fieldTable.put("outerWidth",	Boolean.TRUE);
	fieldTable.put("pageXOffset",	Boolean.TRUE);
	fieldTable.put("pageYOffset",	Boolean.TRUE);
	fieldTable.put("personalbar",	Boolean.FALSE);
	fieldTable.put("scrollbars",	Boolean.FALSE);
	fieldTable.put("statusbar",	Boolean.FALSE);
	fieldTable.put("toolbar",	Boolean.FALSE);

    }


    /**
     * <p> Test if the JSObject factory has been set.
     * </p>
     */
    private static boolean factorySet = false;


    /**
     * <p> Construct a new Window object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    public Window(int instance, String context)  {
	super(instance, context);

	// Set JSObject resolver.
	try  {
	    if (factorySet == false)   {
		setResolver(new JSObjectFactory());
		factorySet = true;
	    }
	} catch (JSException e)  {
	}

	// Setup object property and method table.
	//
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
	
	if (name.equals("locationbar"))
	    return resolveObject(JSType.UIBar, context + ".locationbar");
	else if (name.equals("menubar"))
	    return resolveObject(JSType.UIBar, context + ".menubar");
	else if (name.equals("personalbar"))
	    return resolveObject(JSType.UIBar, context + ".personalbar");
	else if (name.equals("scrollbars"))
	    return resolveObject(JSType.UIBar, context + ".scrollbars");
	else if (name.equals("statusbar"))
	    return resolveObject(JSType.UIBar, context + ".statusbar");
	else if (name.equals("toolbar"))
	    return resolveObject(JSType.UIBar, context + ".toolbar");


	// Try to match the name of the property with the field table.
	//
        return super.getMember(name);
    }
}
