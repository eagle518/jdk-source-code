/*
 * @(#)Navigator.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig4;


import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Navigator object in the JavaScript Document Object Model
 * in Navigator 4.x.
 * </p>
 */
public class Navigator extends sun.plugin.javascript.navig.Navigator {

   /**
    * <p> Method table contains all method info in the Navigator object. </p>
    */
   private static HashMap methodTable = new HashMap();

   /**
    * <p> Field table contains all properties info in the Navigator object. </p>
    */
   private static HashMap fieldTable = new HashMap();

   static {

	// Initialize all method and field info in the Navigator object.
	//
	methodTable.put("preference",  Boolean.TRUE);

	fieldTable.put("language",	Boolean.FALSE);
	fieldTable.put("platform",	Boolean.FALSE);
    }


    /**
     * <p> Construct a new Navigator object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     */
    protected Navigator(int instance)  {
	super(instance);

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, methodTable);
    }
}
