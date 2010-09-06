/*
 * @(#)Option.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Option object in the JavaScript Document Object Model.
 * </p>
 */
class Option extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Field table contains all properties info in the Option object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all field info in the Option object.
	//
	fieldTable.put("defaultSelected", Boolean.FALSE);
	fieldTable.put("index",		  Boolean.FALSE);
	fieldTable.put("selected",	  Boolean.TRUE);
	fieldTable.put("text",		  Boolean.TRUE);
	fieldTable.put("value",		  Boolean.TRUE);
    }

    
    /**
     * <p> Construct a new Option object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    Option(int instance, String context) {
	super(instance, context);
	addObjectTable(fieldTable, null);
    }
}
