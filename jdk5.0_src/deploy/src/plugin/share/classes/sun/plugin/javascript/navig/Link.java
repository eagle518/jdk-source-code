/*
 * @(#)Link.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Link object in the JavaScript Document Object Model.
 * </p>
 */
public class Link extends sun.plugin.javascript.navig.URL {

    /**
     * <p> Field table contains all properties info in the Link object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all method and field info in the Link object.
	//
	fieldTable.put("target", Boolean.TRUE);
    }

    
    /**
     * <p> Construct a new Link object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     */
    protected Link(int instance, String context) {
	super(instance, context);
	addObjectTable(fieldTable, null);
    }
}
