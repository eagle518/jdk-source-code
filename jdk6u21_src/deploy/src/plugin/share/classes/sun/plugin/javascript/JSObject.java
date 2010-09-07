/*
 * @(#)JSObject.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript;

import java.applet.Applet;
import java.applet.AppletContext;
import netscape.javascript.JSException;

/** 
 * <p> Derived from abstract class netscape.javascript.JSObject so applet 
 * can access the JavaScript Document Object Model in the browser. This
 * is a base class that will be used by both IE and NS in Java Plug-in.
 * By default, all methods will throw JSException if the method is not 
 * overriden by the derived class.
 * </p>
 */
public abstract class JSObject extends netscape.javascript.JSObject {

    /**
     * <p> Calls a JavaScript method. Equivalent to 
     * "this.methodName(args[0], args[1], ...)" in JavaScript.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @return Result of the method.
     */
    public Object call(String methodName, Object args[]) throws JSException {
	throw new JSException("call does not support " + toString() + "." + methodName);
    }

    /** 
     * <p> Evaluates a JavaScript expression. The expression is a string of 
     * JavaScript source code which will be evaluated in the context given by 
     * "this".
     * </p>
     *
     * @param s The JavaScript expression.
     * @return Result of the JavaScript evaluation.
     */    public Object eval(String s) throws JSException {
	throw new JSException("eval does not support " + s);
    }

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to 
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException {
	throw new JSException("getMember does not support " + toString() + "." + name);
    }

    /** 
     * <p> Sets a named member of a JavaScript object. Equivalent to 
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) throws JSException {
	throw new JSException("setMember does not support " + toString() + "." + name);
    }

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public void removeMember(String name) throws JSException {
	throw new JSException("removeMember does not support " + toString() + "." + name);
    }

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to 
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) throws JSException {
	throw new JSException("getSlot does not support " + toString() + "[" + index + "]");
    }

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to 
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) throws JSException {
	throw new JSException("setSlot does not support " + toString() +  "[" + index + "]");
    }

    /**
     * <p> Lock this JSObject for current page session to prevent it 
     * from GC.
     * Defualt behavior is doing nothing.
     * </p>
     */
     public void lock() {

     }
	
	 /**
     * <p> Clean up the JSObject when
     * the applet object/bean is gone
     * </p>
     */
    public void cleanup() {
    }
	
}
