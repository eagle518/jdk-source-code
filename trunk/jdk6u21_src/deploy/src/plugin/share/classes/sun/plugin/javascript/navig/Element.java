/*
 * @(#)Element.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;


/** 
 * <p> Emulate the Element object in the JavaScript Document Object Model.
 * </p>
 */
class Element extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Method table contains all method info in the Element object. </p>
     */
    private static HashMap methodTable = new HashMap();

    /**
     * <p> Field table contains all properties info in the Element object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all method and field info in the Element object.
	//
	methodTable.put("blur",    Boolean.FALSE);
	methodTable.put("click",   Boolean.FALSE);
	methodTable.put("focus",   Boolean.FALSE);
	methodTable.put("select",  Boolean.FALSE);

	fieldTable.put("checked",	 Boolean.TRUE);
	fieldTable.put("defaultChecked", Boolean.FALSE);
	fieldTable.put("defaultValue",   Boolean.FALSE);
	fieldTable.put("form",		 Boolean.FALSE);
	fieldTable.put("length",	 Boolean.FALSE);
	fieldTable.put("name",		 Boolean.FALSE);
	fieldTable.put("options",	 Boolean.FALSE);
	fieldTable.put("selectedIndex",	 Boolean.FALSE);
	fieldTable.put("type",		 Boolean.FALSE);
	fieldTable.put("value",		 Boolean.TRUE);
    }

    /**
     * <p> Form object that the element is contained in. </p>
     */
    private Form form;


    /**
     * <p> Construct a new Element object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     * @param form Form object that the element is contained in.
     */
    Element(int instance, String context, Form form) {
	super(instance, context);
	this.form = form;

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, methodTable);
    }


    /**
     * <p> Retrieves a named member of the Element object. Equivalent to 
     * "self.document.forms[].elements[].name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

	// Deal with special properties first.
	//
	if (name.equals("form"))
	    return form;
	else if (name.equals("options"))  {
	    return resolveObject(JSType.OptionArray, context + ".options");
	}

	// Try to match the name of the property with the field table.
	//
        Object value = super.getMember(name);

	// Fix for #4479001 - TextField/TextArea returns null when value
	// is not keyed.
	//
	if (value == null && name.equals("value"))
	    value = "";

	return value;
    }
}
