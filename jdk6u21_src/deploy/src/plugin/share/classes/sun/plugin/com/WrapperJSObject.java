/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package sun.plugin.com;

import netscape.javascript.JSObject;
import netscape.javascript.JSException;
import java.lang.reflect.*;

final class WrapperJSObject extends JSObject {
	
	private Object	realObject;

	public WrapperJSObject(Object realObject) {
		this.realObject = realObject;
	}


	public String toString() {
		return realObject.toString();
	}

	public int hashCode() {
		return realObject.hashCode();
	}

	public boolean equals(Object another) {
		if(another == null)
			return false;

		if(another instanceof WrapperJSObject) {
			return ((WrapperJSObject)another).realObject.equals(realObject);
		}

		return another.equals(realObject);
	}

    public Object call(String methodName, Object args[]) 
		throws JSException {
		
		Class[] argClasses = null;
		if(args != null) {
			argClasses = new Class[args.length];
			for(int index = 0; index < args.length; index ++) {
				argClasses[index] = args[index].getClass();
			}
		}

		try {
			Method m = realObject.getClass().getDeclaredMethod(methodName, argClasses);
			return m.invoke(realObject, args);
		} catch(Exception e) {
			throw new JSException(JSException.EXCEPTION_TYPE_EMPTY, (Object)e);
		}
	}

    /** 
     * <p> Evaluates a JavaScript expression. The expression is a string of 
     * JavaScript source code which will be evaluated in the context given by 
     * "this".
     * </p>
     *
     * @param s The JavaScript expression.
     * @return Result of the JavaScript evaluation.
     */
    public Object eval(String s) throws JSException {
		return call("eval", new Object[]{s});
	}

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to 
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) 
		throws JSException {
		return call("getMember", new Object[] {name}); 
	}

    /** 
     * <p> Sets a named member of a JavaScript object. Equivalent to 
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) 
		throws JSException {
		call("setMember", new Object[] {name, value});
	}

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public void removeMember(String name) 
		throws JSException {
		call("removeMember", new Object[] { name });
	}

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to 
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) 
		throws JSException {
		return call("getSlot", new Object[] { new Integer(index) });
	}

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to 
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) 
		throws JSException {
		call("setSlot", new Object[] { new Integer(index), value});
	}
}
