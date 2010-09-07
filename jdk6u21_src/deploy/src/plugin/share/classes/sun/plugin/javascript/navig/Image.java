/*
 * @(#)Image.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import java.util.HashMap;
import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Image object in the JavaScript Document Object Model.
 * </p>
 */
public class Image extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Field table contains all properties info in the Image object. </p>
     */
    private static HashMap fieldTable = new HashMap();

    static {

	// Initialize all method and field info in the Image object.
	//
	fieldTable.put("border",   Boolean.FALSE);
	fieldTable.put("complete", Boolean.FALSE);
	fieldTable.put("height",   Boolean.FALSE);
	fieldTable.put("hspace",   Boolean.FALSE);
	fieldTable.put("lowsrc",   Boolean.TRUE);
	fieldTable.put("name",     Boolean.FALSE);
	fieldTable.put("src",	   Boolean.TRUE);
	fieldTable.put("vspace",   Boolean.FALSE);
	fieldTable.put("width",	   Boolean.FALSE);
    }

    
    /**
     * <p> Construct a new Image object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     */
    protected Image(int instance, String context) {
	super(instance, context);
	addObjectTable(fieldTable, null);
    }
}
