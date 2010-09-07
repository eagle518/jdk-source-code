/*
 * @(#)Layer.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig4;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import sun.plugin.javascript.navig.JSType;



/** 
 * <p> Emulate the Layer object in the JavaScript Document Object Model
 * in Navigator 4.
 * </p>
 */
public class Layer extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Field table contains all properties info in the Layer object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    /**
     * <p> Method table contains all methods info in the Layer object. </p>
     */
    private static HashMap methodTable = new HashMap();

    static {

	// Initialize all method and field info in the Layer object.
	//
	fieldTable.put("document",     Boolean.FALSE);
	fieldTable.put("name",	       Boolean.FALSE);
	fieldTable.put("left",	       Boolean.TRUE);
	fieldTable.put("top",	       Boolean.TRUE);
	fieldTable.put("pageX",	       Boolean.TRUE);
	fieldTable.put("pageY",        Boolean.TRUE);
	fieldTable.put("zIndex",       Boolean.TRUE);
	fieldTable.put("visibility",   Boolean.TRUE);
	fieldTable.put("clip.top",     Boolean.TRUE);
	fieldTable.put("clip.left",    Boolean.TRUE);
	fieldTable.put("clip.right",   Boolean.TRUE);
	fieldTable.put("clip.bottom",  Boolean.TRUE);
	fieldTable.put("clip.width",   Boolean.TRUE);
	fieldTable.put("clip.height",  Boolean.TRUE);
	fieldTable.put("clip.top",     Boolean.TRUE);
	fieldTable.put("background",   Boolean.TRUE);
	fieldTable.put("bgColor",      Boolean.TRUE);
	fieldTable.put("siblingAbove", Boolean.FALSE);
	fieldTable.put("siblingBelow", Boolean.FALSE);
	fieldTable.put("above",        Boolean.FALSE);
	fieldTable.put("below",	       Boolean.FALSE);
	fieldTable.put("parentLayer",  Boolean.FALSE);
	fieldTable.put("src",	       Boolean.TRUE);

	methodTable.put("moveBy",	  Boolean.FALSE);
	methodTable.put("moveTo",	  Boolean.FALSE);
	methodTable.put("moveToAbsolute", Boolean.FALSE);
	methodTable.put("resizeBy",	  Boolean.FALSE);
	methodTable.put("resizeTo",	  Boolean.FALSE);
	methodTable.put("moveAbove",	  Boolean.FALSE);
	methodTable.put("moveBelow",	  Boolean.FALSE);
	methodTable.put("load",		  Boolean.FALSE);
    }

    
    /**
     * <p> Construct a new Layer object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context
     */
    protected Layer(int instance, String context) {
	super(instance, context);

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, methodTable);
    }


    /**
     * <p> Retrieves a named member of the Layer object. Equivalent to 
     * "self.document.layers[].name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

	if (name.equals("document"))  {
	    Object val = evaluate(context + ".document");
	    if (val == null)
		return null;
	    else
		return resolveObject(JSType.Document, context + ".document");
	}
	else if (name.equals("siblingAbove"))  {
	    Object val = evaluate(context + ".siblingAbove");
	    if (val == null)
		return null;
	    else
		return resolveObject(JSType.Layer, context + ".siblingAbove");
	}
	else if (name.equals("siblingBelow"))	{
	    Object val = evaluate(context + ".siblingBelow");
	    if (val == null)
		return null;
	    else
		return resolveObject(JSType.Layer, context + ".siblingBelow");
	}
	else if (name.equals("above"))  {
	    Object val = evaluate(context + ".above");
	    if (val == null)
		return null;
	    else
    		return resolveObject(JSType.Layer, context + ".above");
	}
	else if (name.equals("below"))	{
	    Object val = evaluate(context + ".below");
	    if (val == null)
		return null;
	    else
		return resolveObject(JSType.Layer, context + ".below");
	}
	else if (name.equals("parentLayer"))  {
	    Object val = evaluate(context + ".parentLayer");
	    if (val == null)
		return null;
	    else
		return resolveObject(JSType.Layer, context + ".parentLayer");
	}

        return super.getMember(name);
    }
}
