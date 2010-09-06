/*
 * @(#)JSObjectResolver.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSException;

/** 
 * <p> JSObjectResolver is an interface for resolving JSObject from a string.
 * </p>
 */
public interface JSObjectResolver {

    /** 
     * <p> Create a new JSObject.
     * </p>
     *
     * @param js JSObject instance.
     * @param type JSObject type.
     * @param instance Plugin instance.
     * @param context Evaluation context.
     * @param custom Custom object for JSObject creation.
     * @return New JSObject with type "type".
     */
    Object resolveObject(JSObject js, String type, int instance, String context, Object custom) 
    throws JSException;
}
