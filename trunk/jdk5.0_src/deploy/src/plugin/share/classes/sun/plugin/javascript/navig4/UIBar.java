/*
 * @(#)UIBar.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig4;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the UIBar object in the JavaScript Document Object Model
 * in Navigator 4.x.
 * </p>
 */
class UIBar extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Field table contains all properties info in the Link object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all method and field info in the Link object.
	//
	fieldTable.put("visible", Boolean.TRUE);
    }

    
    /**
     * <p> Construct a new UIBar object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     */
    UIBar(int instance, String context) {
	super(instance, context);

	// Setup object property and method table.
	//
	addObjectTable(fieldTable, null);
    }
}
