/*
 * @(#)URL.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;


/** 
 * <p> Emulate the URL object in the JavaScript Document Object Model.
 * </p>
 */
class URL extends sun.plugin.javascript.navig.JSObject {

    private static HashMap fieldTable = new HashMap();

   /**
    * <p> Field table contains all properties info in the URL object. </p>
    */
    
    static {
	fieldTable.put("hash",	    Boolean.TRUE);
	fieldTable.put("host",	    Boolean.TRUE);
	fieldTable.put("hostname",  Boolean.TRUE);
	fieldTable.put("href",	    Boolean.TRUE);
	fieldTable.put("pathname",  Boolean.TRUE);
	fieldTable.put("port",	    Boolean.TRUE);
	fieldTable.put("protocol",  Boolean.TRUE);
	fieldTable.put("search",    Boolean.TRUE);
    }


    /**
     * <p> Construct a new URL object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    URL(int instance, String context) {
	super(instance, context);
	addObjectTable(fieldTable, null);
    }
}
