/*
 * @(#)Form.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Form object in the JavaScript Document Object Model.
 * </p>
 */
class Form extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Method table contains all method info in the Element object. </p>
     */
    private static HashMap methodTable = new HashMap();

    /**
     * <p> Field table contains all properties info in the Form object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all field info in the Window object.
	//
	methodTable.put("reset",    Boolean.FALSE);
	methodTable.put("submit",   Boolean.FALSE);

	fieldTable.put("action",   Boolean.TRUE);
	fieldTable.put("elements", Boolean.FALSE);
	fieldTable.put("encoding", Boolean.TRUE);
	fieldTable.put("method",   Boolean.TRUE);
	fieldTable.put("target",   Boolean.TRUE);
	fieldTable.put("name",     Boolean.TRUE);
	fieldTable.put("length",   Boolean.FALSE);
    }

    /**
     * <p> Construct a new Form object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    Form(int instance, String context)  {
	super(instance, context);

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, methodTable);
    }


    /**
     * <p> Retrieves a named member of the Form object. Equivalent to 
     * "self.document.forms[].name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

        // Deal with special properties first.
        //

        if (name.equals("elements"))  {
            return resolveObject(JSType.ElementArray, context + ".elements", this);
        }

        // Try to match the name of the property with the field table.
        //
        try {
        return super.getMember(name);
        }
        catch(JSException exp)
        {
                //Check whether it is a valid object- if so check is it of type Element or ElementArray

                String type = evalScript(instance, "javascript: typeof(" + context + "." + name + ")" );
                if(type != null && type.equalsIgnoreCase("object"))
                {
                        String objClass = evalScript(instance, "javascript:" + context + "." + name + ".constructor.name");
                        if(objClass.equalsIgnoreCase("Input") || objClass.equalsIgnoreCase("HTMLInputElement"))
                                return resolveObject(JSType.Element, context + "." + name);
                        else
                        {
                                // we don't have an Element, but may be an ElementArray
                                objClass = evalScript(instance, "javascript:" + context + "." + name + "[0].constructor.name");
                                if(objClass.equalsIgnoreCase("Input") || objClass.equalsIgnoreCase("HTMLInputElement"))
                                        return resolveObject(JSType.ElementArray, context + "." + name);
                                else
                                        throw exp;
                        }
                }
                else
                {
                        throw exp;
                }
        }
    }
}
