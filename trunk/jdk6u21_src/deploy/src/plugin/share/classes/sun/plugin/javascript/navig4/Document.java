/*
 * @(#)Document.java	1.11 10/03/24
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
 * <p> Emulate the Document object in the JavaScript Document Object Model
 * in Navigator 4.
 * </p>
 */
public class Document extends sun.plugin.javascript.navig.Document {


    /**
     * <p> Method table contains all method info in the Document object. </p>
     */
    private static HashMap methodTable = new HashMap();

    /**
     * <p> Field table contains all properties info in the Document object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all method and field info in the Document object.
	//
	methodTable.put("getSelection",   Boolean.TRUE);

	fieldTable.put("layers",	  Boolean.FALSE);
    }


    /**
     * <p> Construct a new Document object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    protected Document(int instance, String context)  {
	super(instance, context);

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, methodTable);
    }

 
    /**
     * <p> Retrieves a named member of the Document object. Equivalent to 
     * "self.document.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

	if (name.equals("layers"))  {
	    return resolveObject(JSType.LayerArray, context + ".layers");
	}

        return super.getMember(name);
    }
}
