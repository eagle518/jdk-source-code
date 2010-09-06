/*
 * @(#)ElementArray.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;


/** 
 * <p> Emulate the element Array object in the JavaScript Document Object Model.
 * </p>
 */
class ElementArray extends sun.plugin.javascript.navig.Array {

    /** 
     * <p> Form object that the element array is contained. </p>
     */
    private Form form;

    /**
     * <p> Construct a new ElementArray object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     * @param length Length of the array.
     */
    ElementArray(int instance, String context, int length, Form form) {
	super(instance, context, length);
	this.form = form;
    }


    /** 
     * <p> Create an array element.
     * </p>
     *
     * @param String Array element context.
     * @return New array element.
     */
    protected Object createObject(String context) throws JSException {
	return resolveObject(JSType.Element, context, form);
    }
}
