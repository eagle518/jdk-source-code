/*
 * @(#)AnchorArray.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;


/** 
 * <p> Emulate the anchor Array object in the JavaScript Document Object Model
 * in Navigator 3.x.
 * </p>
 */
public class AnchorArray extends sun.plugin.javascript.navig.Array {

    /**
     * <p> Construct a new AnchorArray object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     * @param length Length of the array.
     */
    protected AnchorArray(int instance, String context, int length) {
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
	return resolveObject(JSType.Anchor, context);
    }
}
