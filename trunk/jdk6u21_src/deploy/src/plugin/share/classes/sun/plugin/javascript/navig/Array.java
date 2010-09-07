/*
 * @(#)Array.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;


/** 
 * <p> Emulate the Array object in the JavaScript Document Object Model
 * in Navigator 3.x and 4.x.
 * </p>
 */
public abstract class Array extends sun.plugin.javascript.navig.JSObject {

    /** 
     * <p> Length of the array. </p>
     */
    protected int length = -1;


   /**
     * <p> Construct a new Array object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     * @param length Length of the array.
     */
    protected Array(int instance, String context, int length) {
	super(instance, context);
	this.length = length;
    }

    /**
     * <p> Retrieves a named member of a Array object. Equivalent to 
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {

	if (name.equals("length"))
	    return new Integer(length);

        return super.getMember(name);
    }


    /**
     * <p> Retrieves an indexed member of a Array object. Equivalent to 
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) throws JSException {
	if (index < 0 || index >= length)
	    throw new JSException("getSlot does not support " + toString() + "[" + index + "]");

	return createObject(context + "[" + index + "]");
    }

    /**
     * <p> Sets an indexed member of a Array object. Equivalent to 
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) throws JSException {
	if (index < 0 || index >= length)
	    throw new JSException("setSlot does not support " + toString() + "[" + index + "]");

	evaluate(context + "[" + index + "]=" + value.toString());
    }

    
    /** 
     * <p> Create an array element.
     * </p>
     *
     * @param String Array element context.
     * @return New array element.
     */
    protected abstract Object createObject(String context) throws JSException;
}
