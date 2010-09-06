/*
 * @(#)JSObject.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.javascript;

import java.applet.Applet;
import java.applet.AppletContext;
import sun.plugin.javascript.JSContext; 


/** 
 * <p> Emulate netscape.javascript.JSObject so applet can access the JavaScript
 * Document Object Model in the browser.
 * </p>
 */
public abstract class JSObject {

    /** 
     * <p> Construct a JSObject. It is declared as protected so the applet can
     * not contains classes derived from it. 
     * </p>
     */
    protected JSObject()  {
    }

    /**
     * <p> Calls a JavaScript method. Equivalent to 
     * "this.methodName(args[0], args[1], ...)" in JavaScript.
     * </p>
     *
     * @param methodName The name of the JavaScript method to be invoked.
     * @param args An array of Java object to be passed as arguments to the method.
     * @return Result of the method.
     */
    public abstract Object call(String methodName, Object args[]) throws JSException;

    /** 
     * <p> Evaluates a JavaScript expression. The expression is a string of 
     * JavaScript source code which will be evaluated in the context given by 
     * "this".
     * </p>
     *
     * @param s The JavaScript expression.
     * @return Result of the JavaScript evaluation.
     */
    public abstract Object eval(String s) throws JSException;

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to 
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public abstract Object getMember(String name) throws JSException;

    /** 
     * <p> Sets a named member of a JavaScript object. Equivalent to 
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public abstract void setMember(String name, Object value) throws JSException;

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public abstract void removeMember(String name) throws JSException;

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to 
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public abstract Object getSlot(int index) throws JSException;

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to 
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public abstract void setSlot(int index, Object value) throws JSException;

    /** 
     * <p> Returns a JSObject for the window containing the given applet. 
     * </p>
     *
     * @param applet The applet.
     * @return JSObject for the window containing the given applet.
     */
    public static JSObject getWindow(Applet applet) throws JSException {

	try
	{
	    if (applet != null)
	    {
		String obj = (String) applet.getParameter("MAYSCRIPT");

		// Comment out MAYSCRIPT check because Internet Explorer doesn't support
		// it.
//		if (obj != null && (obj.equals("") || (new Boolean(obj).booleanValue() == true)))
		{
		    // MAYSCRIPT is enabled

		    AppletContext c = applet.getAppletContext();

		    // The applet context must implement the sun.plugin.javascript.JSContext
		    // in order for us to get the handle that can be used when evaluating 
		    // JavaScript expression.
		    //
		    JSObject ret = null;

		    if (c instanceof sun.plugin.javascript.JSContext)
		    {
			JSContext j = (JSContext) c;
			ret = j.getJSObject();
		    }

		    if (ret != null)
		       return ret;
		}
	    }
	}
	catch (Throwable e)
	{
	    throw new JSException(JSException.EXCEPTION_TYPE_ERROR, e);
	}

	throw new JSException();
    }
}
