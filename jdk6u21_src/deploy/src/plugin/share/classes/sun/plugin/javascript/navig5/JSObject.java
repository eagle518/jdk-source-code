/*
 * @(#)JSObject.java	1.29 03/12/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig5;

import java.applet.Applet;
import java.net.URL;
import java.security.AccessController;
import java.security.AccessControlContext;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.CodeSource;
import java.security.ProtectionDomain;
import java.security.AccessControlException;
import java.security.AllPermission;
import sun.plugin.javascript.JSContext;
import netscape.javascript.JSException;
import sun.plugin.viewer.context.NetscapeAppletContext;
import com.sun.deploy.util.Trace;

/**
 * <p> Emulate netscape.javascript.JSObject so applet can access the JavaScript
 * Document Object Model in the Mozilla.
 *
 * This class provides a wrapper around the latest JSObject definition in
 * Mozilla. In Mozilla, the layout and JS thread are combined into one, and
 * the restriction is that we are allowed to access any LiveConnect interface from
 * the JS thread (main thread) only. Thus, all calls in this class will be
 * forwarded to the main thread for execution.
 *
 * Notice that almost all native methods will take an array of certificates,
 * security context, etc as arguments. This is required for supporting LiveConnect
 * with security enabled. The certificate params are basically the break-down
 * representation of the CodeSource, and this is done to avoid multiple JNI
 * callback for extracting the information inside the CodeSource object.
 * The AccessControlContext is a snap-shot of the current Java stack
 * representation. LiveConnect will use these information to determine if
 * certain operations are allowed or not.
 * </p>
 */
public class JSObject extends sun.plugin.javascript.JSObject {

    private int nativeJSObject = 0;
    private int jsThreadID = 0;
    private int handle  = 0;

    private NetscapeAppletContext nac = null;
    private boolean released = false;

    public final static int JSOBJECT_GETWINDOW = 1;
    public final static int JSOBJECT_GETMEMBER = 2;
    public final static int JSOBJECT_GETSLOT = 3;
    public final static int JSOBJECT_SETMEMBER = 4;
    public final static int JSOBJECT_SETSLOT = 5;
    public final static int JSOBJECT_REMOVEMEMBER = 6;
    public final static int JSOBJECT_CALL = 7;
    public final static int JSOBJECT_EVAL = 8;
    public final static int JSOBJECT_TOSTRING = 9;
    public final static int JSOBJECT_FINALIZE = 10;

    /**
     * <p> Construct a JSObject. It is declared as protected so the applet can
     * not contains classes derived from it.
     * </p>
     *
     * @param Plug-in instance 
     */
    public JSObject(int handle)  {
	this.handle = handle;
	jsThreadID = JSGetThreadID(handle);
	nativeJSObject = JSGetNativeJSObject();
    }

    /**
     * <p> Construct a JSObject. It is declared as protected so the applet can
     * not contains classes derived from it.
     * </p>
     *
     * @param jsThreadID JavaScript Thread ID
     * @param native JavaScript object
     */
    public JSObject(int jsThreadID, int nativeJSObject)  {
	this.jsThreadID = jsThreadID;
	this.nativeJSObject = nativeJSObject;
    }

    /**
     * <p> Set the AppletContext for the JSObject
     * so that any JSObject associated with this JSObject
     * will bind to the same AppletContext. By doing this, we can
     * destroy the JSObject when the applet/bean is gone
     *</p>
     * @param NetscapeAppletContext nac
     */
     public void setNetscapeAppletContext(NetscapeAppletContext nac) {
     this.nac = nac;
	 this.handle = nac.getAppletContextHandle();
	 nac.addJSObjectToExportedList(this);
     }

    /**
     * <p> Cleanup is called to indicate that the applet/bean
     * is on the way to be destroyed
     * </p>
     *
     * @param 
     */
    public synchronized void cleanup() {
	if (!released) {
	    /* Pass null pluginInstance to the browser since "pluginInst" may have
	       been already destroyed.
	    */
	    JSObjectCleanup(jsThreadID, handle, nativeJSObject);
	    released = true;

	}
    }

    /*
     * <p> A helper method to call Javascript
     * </p>
     *
     */
    private Object invoke(int invokeCode, String methodName, Object[] args) throws JSException
    {
	if (released)
	    throw new JSException("Native DOM Object has been released");
	
	SecurityContext ctx = SecurityContext.getCurrentSecurityContext();
	
	boolean isAllPermission = false;
	try {
	    AccessControlContext acc = ctx.getAccessControlContext();

	    acc.checkPermission(new AllPermission());

	    isAllPermission = true;
	}
	
	catch (AccessControlException e) {
	}
	
	Trace.msgLiveConnectPrintln("jsobject.invoke.url.permission", 
                                    new Object[] {ctx.getURL(), String.valueOf(isAllPermission)});

	
	Object result = JSObjectInvoke(invokeCode, jsThreadID, handle, nativeJSObject, 
								   ctx.getURL(), methodName, args, isAllPermission);

	if (result != null && result instanceof sun.plugin.javascript.navig5.JSObject)
	    ((sun.plugin.javascript.navig5.JSObject)result).setNetscapeAppletContext(nac);

	return result;
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
    public Object call(String methodName, Object args[]) throws JSException
    {
        Trace.msgLiveConnectPrintln("jsobject.call", 
                                    new Object[] {methodName});

	return invoke(JSOBJECT_CALL, methodName, args);
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
    public Object eval(String s) throws JSException
    {
        Trace.msgLiveConnectPrintln("jsobject.eval", 
				    new Object[] {s});

	return invoke(JSOBJECT_EVAL, s, null);
    }

    /**
     * <p> Retrieves a named member of a JavaScript object. Equivalent to
     * "this.name" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @return The value of the propery.
     */
    public Object getMember(String name) throws JSException
    {
        Trace.msgLiveConnectPrintln("jsobject.getMember", 
                                    new Object[] {name});

	return invoke(JSOBJECT_GETMEMBER, name, null);
    }

    /**
     * <p> Sets a named member of a JavaScript object. Equivalent to
     * "this.name = value" in JavaScript.
     * </p>
     *
     * @param name The name of the JavaScript property to be accessed.
     * @param value The value of the propery.
     */
    public void setMember(String name, Object value) throws JSException
    {

    Trace.msgLiveConnectPrintln("jsobject.setMember", 
                                    new Object[] {name});
	Object[] args = new Object[1];
	args[0] = value;                                    

	invoke(JSOBJECT_SETMEMBER, name, args);
    }

    /**
     * <p> Removes a named member of a JavaScript object.
     * </p>
     *
     * @param name The name of the JavaScript property to be removed.
     */
    public void removeMember(String name) throws JSException
    {
        Trace.msgLiveConnectPrintln("jsobject.removeMember",
                                    new Object[] {name});

	invoke(JSOBJECT_REMOVEMEMBER, name, null);
    }

    /**
     * <p> Retrieves an indexed member of a JavaScript object. Equivalent to
     * "this[index]" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     * @return The value of the indexed member.
     */
    public Object getSlot(int index) throws JSException
    {
        Trace.msgLiveConnectPrintln("jsobject.getSlot", 
                                    new Object[] {String.valueOf(index)});
	Object[] args = new Object[1];
	args[0] = new Integer(index);

	return invoke(JSOBJECT_GETSLOT, null, args);
    }

    /**
     * <p> Sets an indexed member of a JavaScript object. Equivalent to
     * "this[index] = value" in JavaScript.
     * </p>
     *
     * @param int The index of the array to be accessed.
     */
    public void setSlot(int index, Object value) throws JSException
    {
        Trace.msgLiveConnectPrintln("jsobject.setSlot", 
                                    new Object[] {String.valueOf(index)});

	Object[] args = new Object[2];
	args[0] = new Integer(index);
	args[1] = value;

	invoke(JSOBJECT_SETSLOT, null, args);
    }

    /**
     * <p> Convert JSObject to a string.
     * </p>
     */
    public String toString()
    {
	Object ret = invoke(JSOBJECT_TOSTRING, null, null);
	
	if (ret != null)
	    return ret.toString();

	return null;
}

    /**
     * <p> Free up any outstanding resource that was held.
     * </p>
     */
    public void finalize() 
    {
		cleanup();
    }

    /**
     * <p> Obtain native JavaScript object for a given plugin instance peer.
     * </p>
     *
     * @param peer nsIPluginInstancePeer
     * @return int native JavaScript object
     */
    private int JSGetNativeJSObject()
    {
	Object ret = invoke(JSOBJECT_GETWINDOW, null, null);

	if (ret != null && ret instanceof Integer)
       return ((Integer)(ret)).intValue();
	else
		throw new JSException("Native Window is destroyed");
    }


    private native Object JSObjectInvoke(int code, int jsThreadID, int handle, int nativeJSObj,
					 String url, String methodName, Object[] args, boolean isAllPermission);

    private native void JSObjectCleanup(int jsThreadID, int handle, int nativeJSObject);
	
    /**
     * <p> Obtain JavaScript Thread ID associated with a given plugin instance peer.
     * </p>
     *
     * @param peer nsIPluginInstancePeer interface pointer
     * @return int JavaScript thread ID associated with the nsIPluginInstancePeer
     */
    private static native int JSGetThreadID(int peer);    
}


/**
 * SecurityContext encapulates all the security context on the current Java stack.
 */
class SecurityContext
{
    private ProtectionDomain domain;
    private AccessControlContext ctx;

    SecurityContext(ProtectionDomain domain, AccessControlContext ctx)
    {
	this.domain = domain;
	this.ctx = ctx;
    }

    String getURL()
    {
	if (domain != null)
	{
	    CodeSource src = domain.getCodeSource();

	    if (src != null)
	    {
		URL u = src.getLocation();
		if (u != null)
		{
		    StringBuffer buffer = new StringBuffer();
		    String protocol = u.getProtocol();
		    String host = u.getHost();
		    int port = u.getPort();

		    buffer.append(protocol);
		    buffer.append("://");
		    buffer.append(host);

		    if (port != -1)
			buffer.append(":" + port);

		    return buffer.toString();
		}
	    }
	}

	return "file://";
    }

    byte[][] getCertChain()
    {
	return null;
    }

    int[] getCertLength()
    {
	return null;
    }

    int getNumOfCert()
    {
	return 0;
    }

    AccessControlContext getAccessControlContext()
    {
	return ctx;
    }

    static class PrivilegedBlockAction implements PrivilegedExceptionAction {
	AccessControlContext ctx;

	PrivilegedBlockAction(AccessControlContext ctx)
	{
	    this.ctx = ctx;
	}

	public Object run() throws Exception {
	    SecurityManager sm = System.getSecurityManager();

	    if (sm != null && sm instanceof sun.plugin.security.ActivatorSecurityManager)
	    {
		sun.plugin.security.ActivatorSecurityManager mgr = (sun.plugin.security.ActivatorSecurityManager) sm;

		Class[] clazz = mgr.getExecutionStackContext();

		// Walk up the stack to find a class loader.
		for (int i=0; i < clazz.length; i++)
		{
		    Class c = clazz[i];
		    ClassLoader loader = c.getClassLoader();
		    if (loader instanceof java.net.URLClassLoader || loader instanceof sun.applet.AppletClassLoader || sun.applet.AppletClassLoader.class.isAssignableFrom(c))
			return new SecurityContext(c.getProtectionDomain(), ctx);
		}
	    }

	    return new SecurityContext(null, ctx);
	}
    }

    static SecurityContext getCurrentSecurityContext()
    {
	// Obtain snap shot of the stack
	AccessControlContext ctx = AccessController.getContext();

	try {
	    return (SecurityContext) AccessController.doPrivileged(new PrivilegedBlockAction(ctx));
	} catch (PrivilegedActionException e) {
	    Trace.securityPrintException(e);
	    return new SecurityContext(null, ctx);
	}
    }
}

