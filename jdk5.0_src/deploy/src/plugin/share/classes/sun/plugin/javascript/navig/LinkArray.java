/*
 * @(#)LinkArray.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;


/** 
 * <p> Emulate the link Array object in the JavaScript Document Object Model.
 * </p>
 */
public class LinkArray extends sun.plugin.javascript.navig.Array {

    /**
     * <p> Construct a new LinkArray object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     * @param length Length of the array.
     */
    protected LinkArray(int instance, String context, int length) {
	super(instance, context, length);
    }


    /** 
     * <p> Create an array element.
     * </p>
     *
     * @param String Array element context.
     * @return New array element.
     */
    protected Object createObject(String context) throws JSException {
	return resolveObject(JSType.Link, context);
    }
}
