/*
 * @(#)LayerArray.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig4;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import sun.plugin.javascript.navig.JSType;


/** 
 * <p> Emulate the layer Array object in the JavaScript Document Object Model
 * in Navigator 4.
 * </p>
 */
public class LayerArray extends sun.plugin.javascript.navig.Array {

    /**
     * <p> Construct a new LayerArray object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     * @param length Length of the array.
     */
    protected LayerArray(int instance, String context, int length) {
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
	return resolveObject(JSType.Layer, context);
    }
}
