/*
 * @(#)Anchor.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;



/** 
 * <p> Emulate the Anchor object in the JavaScript Document Object Model
 * in Navigator 3.x.
 * </p>
 */
public class Anchor extends sun.plugin.javascript.navig.JSObject {

    /**
     * <p> Construct a new Anchor object. 
     * </p>
     * 
     * @param instance Native plugin instance.
     * @param context Evaluation context.
     */
    protected Anchor(int instance, String context) {
	super(instance, context);
    }
}
