/*
 * @(#)Location.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Location object in the JavaScript Document Object Model.
 * </p>
 */
class Location extends sun.plugin.javascript.navig.URL {

   /**
    * <p> Method table contains all method info in the Location object. </p>
    */
   private static HashMap methodTable = new HashMap();

   static {

	// Initialize all method info in the Location object.
	//
	methodTable.put("reload",   Boolean.FALSE);
	methodTable.put("replace",  Boolean.FALSE);
    }

    
    /**
     * <p> Construct a new Location object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    Location(int instance, String context) {
	super(instance, context);
	addObjectTable(null, methodTable);
    }
}
