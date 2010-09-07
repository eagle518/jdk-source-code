/*
 * @(#)History.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the History object in the JavaScript Document Object Model.
 * </p>
 */
class History extends sun.plugin.javascript.navig.JSObject {

   /**
    * <p> Method table contains all method info in the History object. </p>
    */
   private static HashMap methodTable = new HashMap();

   /**
    * <p> Field table contains all properties info in the History object. </p>
    */
   private static HashMap fieldTable = new HashMap();

   static {

	// Initialize all method and field info in the History object.
	//
	methodTable.put("back",	    Boolean.FALSE);
	methodTable.put("forward",  Boolean.FALSE);
	methodTable.put("go",	    Boolean.FALSE);
	methodTable.put("toString", Boolean.TRUE);

	fieldTable.put("current",	Boolean.FALSE);
	fieldTable.put("length",	Boolean.FALSE);
	fieldTable.put("next",		Boolean.FALSE);
	fieldTable.put("previous",	Boolean.FALSE);
    }

    
    /**
     * <p> Construct a new History object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    History(int instance, String context)  {
	super(instance, context);
	addObjectTable(fieldTable, methodTable);
    }
}
