/*
 * @(#)DispatchImpl.java	1.22 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.javascript.ocx.JSObject;
import sun.plugin.liveconnect.JavaScriptProtectionDomain;
import sun.plugin.viewer.context.IExplorerAppletContext;
import sun.plugin.security.PluginClassLoader;
import java.applet.Applet;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.SocketPermission;
import java.io.FilePermission;
import java.io.File;
import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.security.AllPermission;
import java.security.Permissions;
import java.security.Permission;
import java.security.PermissionCollection;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.AccessControlException;
import java.security.PrivilegedActionException;
import sun.net.www.ParseUtil;
import sun.security.util.SecurityConstants;
import java.lang.reflect.InvocationTargetException;
import com.sun.deploy.util.Trace;

/**
 *  DispatchImpl encapsulates a Java Object and provides Dispatch interface
 *  It is responsible for maintaining the identity and type of one instance of
 *  a Java object. Objects of this type are used to invoke methods on the java
 *  object that they represent.
 */
public class DispatchImpl implements Dispatch
{
    JavaClass targetClass = null;
    Object targetObj = null;
    int handle = 0;
    int wndHandle = 0;
    AccessControlContext context = null;
    boolean isBridge = false;

    /*
     * Constructor
     * @param obj the object to be wrapped
     */
    public DispatchImpl(Object obj, int id)
    {
	targetObj = obj;
	handle = id;
    }

    /**
     * Invoke a method according to the method index.
     *
     * @param flag Invoke flag
     * @param index Method index
     * @param params Arguments.
     * @return Java object.
     */
    public Object invoke(final int flag, final int index, final Object []params)
        throws  Exception
    {
	try {
	    //No security constraints in case of ActiveX bridge application
	    if(isBridge)
		return invokeImpl(flag, index, params);

	    if(context == null) {
		context = createContext();
	    }

	    // Invoke the method within the applet sand box security restricitions
	    return AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		    public Object run() throws Exception{
			return invokeImpl(flag, index, params);
		    }
		}, context	
	    );
	}catch(Exception exc) {
	    Throwable cause = exc.getCause();
	    if(cause == null) {
		cause = exc;
	    }

       	    Trace.liveConnectPrintException(cause);
	    throw new Exception(cause.toString());
	}
    }

    public AccessControlContext createContext() {
    	try {
	    ProtectionDomain[] domains = new ProtectionDomain[1];
	    //Obtain the java code origin
	    ProtectionDomain pd = (ProtectionDomain)AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    return targetObj.getClass().getProtectionDomain();
		}
	    });

	    CodeSource cs = null;
	    URL url = null;
	    if(pd != null)
		cs = pd.getCodeSource();
	    if(cs != null)
		url = cs.getLocation();

	    domains[0] = getJSProtectionDomain(url, targetObj.getClass());
	    return new AccessControlContext(domains);
	}catch(Exception exc) {
       	    Trace.liveConnectPrintException(exc);
	}
	
	return null;    
    }

    /**
     * Invoke a method according to the method index.
     *
     * @param flag Invoke flag
     * @param index Method index
     * @param params Arguments.
     * @return Java object.
     */
    public Object invokeImpl(int flag, int index, Object []params)
        throws  Exception
    {
	Object retObj = null;
	Dispatcher disp = null;
	try {
	    if(params != null)
		convertParams(params);
	    disp = targetClass.getDispatcher(flag, index, params);
	    if(disp != null) {
		retObj = disp.invoke(targetObj, params);
		if(retObj != null)
		    retObj = Utils.convertReturn(disp.getReturnType(), retObj, handle);
	    }
	    return retObj;		
        } catch (Throwable e) {
	    Throwable cause = e.getCause();
	    if(cause == null) {
		cause = e;
	    }

       	    Trace.liveConnectPrintException(cause);
	    throw new Exception(cause.toString());
        }
    }

    /**
     * Return the Java object wrapped by this proxy
     */
    public Object getWrappedObject()
    {
        return targetObj;
    }

    /**
     * Return the class object of the java object wrapped by this proxy
     */
    public JavaClass getTargetClass() {
	if(targetClass == null && targetObj != null) {
	    targetClass = new JavaClass(targetObj.getClass());
	}

	return targetClass;
    }

    public int getReturnType(int flag, int index, Object[] params) {
	Class clazz = null;

	try {
		Dispatcher dispatcher = null;

		if (params != null) {
		    convertParams(params);
		}

		dispatcher = targetClass.getDispatcher(flag, index, params);
		if (dispatcher != null) {
		    clazz = dispatcher.getReturnType();
		}

	} catch (InvocationTargetException e) {
		Trace.liveConnectPrintException(e);
		clazz = null;
	}

	return Utils.getType(clazz);
    }

    public int getIdForName(final String name) throws Exception{
	try {
	    //No security constraints in case of ActiveX bridge application
	    if(isBridge)
		return getIdForNameImpl(name);

	    if(context == null) {
		context = createContext();
	    }

	    // Invoke the method within the applet sand box security restricitions
	    Integer retVal = (Integer)AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		    public Object run() throws Exception{
			return new Integer(getIdForNameImpl(name));
		    }
		}, context	
	    );
	    return retVal.intValue();
	}catch(PrivilegedActionException pe) {
	}

	return -1;
    }

    /*
     *
     */
    public int getIdForNameImpl(String name) throws Exception{
	int id = -1;

	if(targetClass == null && targetObj != null) {
	    targetClass = new JavaClass(targetObj.getClass());
	}
	
	if(targetClass != null) {
	    id = targetClass.getIdForName(name);
	}

	//Trace.liveConnectPrintln("Object: " + targetObj + " Name: " + name + " Id: " + id);
		    
	return id;
    }

    /*
     * Unwraps the wrapped java object arguments
     */
    private void convertParams(Object []params) {
	for(int i=0;i<params.length;i++) {
	    if(params[i] != null && params[i] instanceof DispatchImpl) {
		params[i] = ((DispatchImpl)params[i]).getWrappedObject();
	    } else if(params[i] != null && params[i] instanceof DispatchClient){
		JSObject jsObj = null;
		if (!isBridge) {
		    jsObj = new JSObject((DispatchClient)params[i]);
		    jsObj.setIExplorerAppletContext((IExplorerAppletContext)
				((Applet)targetObj).getAppletContext());
		} else {
		    jsObj = new JSObject((DispatchClient)params[i], handle);
		}		    
		params[i] = jsObj;
	    }
	}
    }

    /**
     * Returns a protection domain that represents the default permission
     * for a given URL.
     *
     * @param urlString URL
     * @return protection domain.
     */
    public static ProtectionDomain getJSProtectionDomain(URL url, Class clazz)
	throws MalformedURLException {
	
	// Obtain default java applet policy
	Policy policy = (Policy)AccessController.doPrivileged(new PrivilegedAction() {
	    public Object run() {
		return Policy.getPolicy();
	    }
	});

	CodeSource cs = new CodeSource(null, (java.security.cert.Certificate[])null);
	final PermissionCollection pc = policy.getPermissions(cs);

	// Inject permission for JS to call Java to do networking, fix #6299744
	pc.add(new java.util.PropertyPermission("http.agent", "read"));

	if(url != null) {
	    Permission p;
	    String path = null;
	    try {
		p = url.openConnection().getPermission();
	    } catch (java.io.IOException ioe) {
		p = null;
	    }

	    if (p instanceof FilePermission) {
		path = p.getName();
	    } else if ((p == null) && (url.getProtocol().equals("file"))) {
		path = url.getFile().replace('/', File.separatorChar);
		path = ParseUtil.decode(path);
	    } else if (p instanceof SocketPermission) {
		/* 
		Socket permission to connect back to the host
		*/
		String host = url.getHost();
		pc.add(new SocketPermission(host,  
		    SecurityConstants.SOCKET_CONNECT_ACCEPT_ACTION));
	    }

	    if(path != null &&
		(clazz.getClassLoader() instanceof PluginClassLoader)) {
		//We need to add an additional permission to read recursively
		if (path.endsWith(File.separator)) {
		    path += "-";
		} else {
		    int endIndex = path.lastIndexOf(File.separatorChar);
		    if (endIndex != -1)
			path = path.substring(0, endIndex+1) + "-";
		}

		pc.add(new FilePermission(path, SecurityConstants.FILE_READ_ACTION));
	    }
	}

	return new JavaScriptProtectionDomain(pc);
    }

    public String toString() {
	if(targetObj != null) {
	    return targetObj.toString();
	}
	return null;
    }

    public int getWindowHandle() {
	if(wndHandle == 0) {
	    wndHandle = getWindowHandle(handle);
	}
	return wndHandle;
    }

    protected void setBridge() {
	isBridge = true;
    }

    native int getWindowHandle(int id);
}




