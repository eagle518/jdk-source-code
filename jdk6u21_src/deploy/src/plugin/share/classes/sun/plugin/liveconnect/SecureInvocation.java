/*
 * @(#)SecureInvocation.java	1.38 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.liveconnect;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.InvocationTargetException;
import java.lang.Thread;
import java.io.FilePermission;
import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.CodeSource;
import java.security.Policy;
import java.security.ProtectionDomain;
import java.security.AllPermission;
import java.security.Permissions;
import java.security.PermissionCollection;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.net.SocketPermission;
import java.net.URL;
import java.net.MalformedURLException;
import sun.plugin.services.PlatformService;
import sun.plugin.javascript.JSClassLoader;
import sun.plugin.javascript.ReflectUtil;
import com.sun.deploy.util.Trace;

/**
 * <P> SecureInvocation is for implementating nsISecureJNI which allows 
 * JNI to be called with a sandbox. The idea here is to perform the action
 * through reflection with a AccessControlContext. In this way, JNI is not 
 * called directly, but called through this wrapper class with a sandbox
 * instead.
 * </P>
 *
 * @version 	1.1 
 * @author	Stanley Man-Kit Ho
 */


public class SecureInvocation 
{
    /**
     * <P> Construct a new Java object with security context.
     * </P>
     *
     * @param clazz Java object class
     * @param constructor Constructor object 
     * @param jargs jvalueArray in native code
     * @param origin Origin of the call
     * @param isUniversalBrowserRead true if enabled
     * @param isUniversalJavaPermission true if enabled
     * @return resulting object
     */
    private static Object ConstructObject(final Class clazz, final Constructor constructor, final Object[] args, 
		  			 final String origin, final boolean isUniversalBrowserRead,
					 final boolean isUniversalJavaPermission) throws Exception
    {
	try {
	    return AccessController.doPrivileged(new PrivilegedExceptionAction() {
		public Object run() throws Exception {
		    try {
			// Check if the JavaScript call is allowed
			boolean direct;
			direct = checkLiveConnectCaller(clazz, origin, isUniversalBrowserRead);

			// Set up virtual JavaScript Protection Domain
			ProtectionDomain[] domains = new ProtectionDomain[1];

			// Obtain proper protection domain for the call
			if (isUniversalJavaPermission == false)
			    domains[0] = getDefaultProtectionDomain(origin, direct);
			else 
			    domains[0] = getTrustedProtectionDomain();

			AccessControlContext context = new AccessControlContext(domains);

			// Perform the object constructor.
			return AccessController.doPrivileged(new PrivilegedConstructObjectAction(constructor, args),
			    context);

		    } catch (Exception e) {
			Trace.liveConnectPrintException(e);
			throw e;
		    }
		}
	    });
	} catch (PrivilegedActionException e) {
	    throw e;
	}
    }

	/**
     * <P> Call a method on Java object with security context.
     * </P>
     *
     * @param clazz Java object class
     * @param obj Java object which method is called
     * @param method Method object 
     * @param jargs jvalueArray in native code
     * @param origin Origin of the call
     * @param isUniversalBrowserRead true if enabled
     * @param isUniversalJavaPermission true if enabled
     * @return resulting object
     */
    private static Object CallMethod(final Class clazz, final Object obj, final Method method, final Object[] args, 
									final String origin, final boolean isUniversalBrowserRead, 
									final boolean isUniversalJavaPermission) throws Exception
    {
		try {
			return AccessController.doPrivileged(new PrivilegedExceptionAction() {
					public Object run() throws Exception {
						try {
							// Check if the JavaScript call is allowed
							boolean direct;
							direct = checkLiveConnectCaller(clazz, origin, isUniversalBrowserRead);

							// Set up virtual JavaScript Protection Domain
							ProtectionDomain[] domains = new ProtectionDomain[1];

							// Obtain proper protection domain for the call
							if (isUniversalJavaPermission == false)
								domains[0] = getDefaultProtectionDomain(origin, direct);
							else 
								domains[0] = getTrustedProtectionDomain();

							AccessControlContext context = new AccessControlContext(domains);

							// Perform the method invocation.
							return AccessController.doPrivileged(new PrivilegedCallMethodAction(method, 
																								obj, args), context);
						} catch (Exception e) {
							Trace.liveConnectPrintException(e);
							throw e;
						}
					}
				});
		}
		catch (PrivilegedActionException e) {
			throw e;
		}
    }


    /**
     * <P> Get a field on Java object with security context.
     * </P>
     *
     * @param clazz Java object class
     * @param obj Java object which field is accessed
     * @param field Field object 
     * @param origin Origin of the call
     * @param isUniversalBrowserRead true if enabled
     * @param isUniversalJavaPermission true if enabled
     * @return resulting object
     */
    private static Object GetField(final Class clazz, final Object obj, final Field field, 
		   		  final String origin, final boolean isUniversalBrowserRead,
				  final boolean isUniversalJavaPermission) throws Exception
    {
	try
	{
	    return AccessController.doPrivileged(new PrivilegedExceptionAction()
	    {
		public Object run() throws Exception
		{
		    try
		    {
			// Check if the JavaScript call is allowed
			checkLiveConnectCaller(clazz, origin, isUniversalBrowserRead);

			// Set up virtual JavaScript Protection Domain
			ProtectionDomain[] domains = new ProtectionDomain[1];

			// Obtain proper protection domain for the call
			if (isUniversalJavaPermission == false)
			    domains[0] = getDefaultProtectionDomain(origin);
			else 
			    domains[0] = getTrustedProtectionDomain();

			AccessControlContext context = new AccessControlContext(domains);

			// Perform the getField
			return AccessController.doPrivileged(new PrivilegedGetFieldAction(field, obj),
		    		    			     context);	
		    } catch (Exception e) {
			Trace.liveConnectPrintException(e);
			throw e;
		    }
		}
	    });
	}
	catch (PrivilegedActionException e) {
	    throw e;
	}
    }



    /**
     * <P> Set a field on Java object with security context.
     * </P>
     *
     * @param clazz Java object class
     * @param obj Java object which field is accessed
     * @param field Field object 
     * @param jval Value to set in the field
     * @param origin Origin of the call
     * @param isUniversalBrowserRead true if enabled
     * @param isUniversalJavaPermission true if enabled
     * @return resulting object
     */
    private static void SetField(final Class clazz, final Object obj, final Field field, final Object val, 
		  	        final String origin, final boolean isUniversalBrowserRead,
			        final boolean isUniversalJavaPermission) throws Exception
    {
	try
	{
	    AccessController.doPrivileged(new PrivilegedExceptionAction()
	    {
		public Object run() throws Exception
		{
		    try
		    {
			// Check if the JavaScript call is allowed
			checkLiveConnectCaller(clazz, origin, isUniversalBrowserRead);

			// Set up virtual JavaScript Protection Domain
			ProtectionDomain[] domains = new ProtectionDomain[1];

			// Obtain proper protection domain for the call
			if (isUniversalJavaPermission == false)
			    domains[0] = getDefaultProtectionDomain(origin);
			else 
			    domains[0] = getTrustedProtectionDomain();

			AccessControlContext context = new AccessControlContext(domains);

			// Perform the setField
			AccessController.doPrivileged(new PrivilegedSetFieldAction(field, obj, val),
	 		  			      context);	
			return null;

		    } catch (Exception e) {
			Trace.liveConnectPrintException(e);
			throw e;
		    }
		}
	    });
	}
	catch (PrivilegedActionException e) {
	    throw e;
	}
    }

    /**
     * <P> Check if LiveConnect call is allowed at all.
     * </P>
     */
    private static boolean checkLiveConnectCaller(Class clazz, 
					       String origin, 
					       boolean isUniversalBrowserRead)
			throws OriginNotAllowedException, MalformedURLException
    {
	// Check if the call is allowed:
	// 
	// The caller must either
	// a) Have the same origin of the callee, or
	// b) "UniversalBrowserRead" is enabled, or
	// c) The callee is a system code - "UniversalBrowserRead" should be true

	// Obtain callee origin
	ProtectionDomain pd = clazz.getProtectionDomain();

	CodeSource cs = pd.getCodeSource();

	if (cs == null)
	{
	    // Callee is system code, anyone can call it
	    Trace.msgLiveConnectPrintln("liveconnect.java.system");
	    return true;
	}	 	

	// If "UniversalBrowserRead" is enabled, return 
	if (isUniversalBrowserRead)
	{
	    Trace.msgLiveConnectPrintln("liveconnect.UniversalBrowserRead.enabled");
	    return false;
	}

	URL location = cs.getLocation();
	URL callerLocation = null;
	
	if (origin != null)
	{
	    try
	    {
		callerLocation = new URL(origin);
	    }
	    catch (MalformedURLException e)
	    {
		e.printStackTrace();
		return false;
	    }
	}

	// If origin is the same, return
	if (location != null && callerLocation != null)
	{
	    // Compare only the scheme, host and port
	    if (location.getProtocol().equalsIgnoreCase(callerLocation.getProtocol())
		&& location.getHost().equalsIgnoreCase(callerLocation.getHost())
		&& location.getPort() == callerLocation.getPort())
	    {
		Trace.msgLiveConnectPrintln("liveconnect.same.origin");
		return false;
	    }
	}
	
	throw new OriginNotAllowedException("JavaScript is not from the same origin as the Java code, caller="
					    + callerLocation + ", callee=" + location);
    }


    /**
     * Returns a protection domain that represents the default permission
     * for a give URL.
     *
     * @param urlString URL
     * @return protection domain.
     */
    private static ProtectionDomain getDefaultProtectionDomain(String urlString)
				    throws MalformedURLException
    {
	return getDefaultProtectionDomain(urlString, false);
    }

    private static ProtectionDomain getDefaultProtectionDomain(String urlString, boolean direct)
				    throws MalformedURLException
    {
	Trace.msgLiveConnectPrintln("liveconnect.default.policy", new Object[] {urlString});

	URL url = null;

	if (urlString != null)
	{
	    try
	    {
		url = new URL(urlString);
	    }
	    catch (MalformedURLException e)
	    {
		// URL is malformed, so don't use URL to deduce
		// security policy - ignore exception
	    }
	}

	// Obtain Java policy
	Policy policy = Policy.getPolicy();
	CodeSource cs = new CodeSource(url, (java.security.cert.Certificate[])null);
	final PermissionCollection pc = policy.getPermissions(cs);

        // Inject permission for JS to call Java to do networking, fix #6299744
        pc.add(new java.util.PropertyPermission("http.agent", "read"));

	if (url == null || url.getProtocol().equals("file")) {

	    pc.add(new FilePermission("<<ALL FILES>>", "read"));
	}
	else
	{
	    // By default, socket permission is added by applet viewer 
	    // on-the-fly. However, since we are not going through the
	    // same code path, we need to add it manually here.
	    //

            String tmpHost = url.getHost();
	    int port = url.getPort();
            if (tmpHost == null || tmpHost.equals("")) {
                // Probably some sort of nested URL
                // Try to figure out the real host, but don't worry if we fail
                try {
                    URL tmpURL = new URL(url.getFile());
                    tmpHost = tmpURL.getHost();
		    port = tmpURL.getPort();
                } catch (Exception e) {
                }
            }

	    InetAddress ina = null;
	    try {
		ina = InetAddress.getByName(tmpHost);
	    } catch (UnknownHostException uhe) {
		// shouldn't happen
		Trace.ignoredException(uhe);
	    }

            final String host = (ina != null) ? ina.getHostName() : tmpHost;
	    if (port == -1) {
		port = url.getDefaultPort();
	    }
            // Only grant permission to connect to the host if we have a valid one
            if (host != null && !host.equals("")) {

		SocketPermission tmpSocketPerm = null;
		// for localhost, we allow connection back to the originating port only
		if (host.equals("localhost")) {
		    tmpSocketPerm = new SocketPermission(host + ":" + port, "connect,accept");
		} else {
		    tmpSocketPerm = new SocketPermission(host, "connect,accept");
		}

		final SocketPermission socketPerm = tmpSocketPerm;

                // If script calls directly into Java apis (not through applet code)
                // then treat the script as if it were an applet downloaded by the plugin itself
                // via a web proxy.
                if (direct) {

                    Class socketPermClass = null;
                    try {
                        socketPermClass = Class.forName("java.net.SocketPermission");
                    } catch (ClassNotFoundException cnfe) {
                        // should not happen
                        //Trace.ignoredException(cnfe);
                        return null;
                    }
        
                    Method denyMethod = null;
                    try {
                        denyMethod = socketPermClass.getDeclaredMethod("setDeny", new Class[0]);
                    } catch (NoSuchMethodException nsme) {
                        // method not available,  skip cached ip check
                        //Trace.ignoredException(nsme);
                    }
        
                    final Method lookupMethod = denyMethod;
                    AccessController.doPrivileged(
                        new PrivilegedAction() {
                            public Object run() {
                                if (lookupMethod != null) {
                                    lookupMethod.setAccessible(true);
                                }
                                return null;
                            }
                        });
                
                    try {
                        // Invoke SocketPermission.setDeny() method
                        if (lookupMethod != null) {
                            lookupMethod.invoke((Object)socketPerm,
                                                new Object[0]);
                        }
                    } catch (IllegalAccessException iae) {
                        //Trace.ignoredException(iae);
                    } catch (InvocationTargetException ite) {
                        //Trace.ignoredException(ite);
                    }
                }
        
                AccessController.doPrivileged(new PrivilegedAction() {
                        public Object run() {
                            pc.add(socketPerm);
                            return null;
                        }
                    });
            }
	}

	return new JavaScriptProtectionDomain(pc);
    }


    /**
     * Returns a protection domain that represents all permission.
     *
     * @return All permission protection domain.
     */
    private static ProtectionDomain getTrustedProtectionDomain()
    {
	Trace.msgLiveConnectPrintln("liveconnect.UniversalJavaPermission.enabled");

	Permissions pc = new Permissions();
	pc.add(new AllPermission());

	return new JavaScriptProtectionDomain(pc);
    }
}


/**
 * <P> PrivilegedCallMethodAction is used for executing the priviledge 
 * action for constructing a new Java object.
 * </P>
 */
class PrivilegedConstructObjectAction implements PrivilegedExceptionAction {

    Constructor constructor;
    Object[] args;

    PrivilegedConstructObjectAction(Constructor constructor, Object[] args)
    {
		this.constructor = constructor;
		this.args = args;

		// Ensure the argument is not null
		if (this.args == null)
			this.args = new Object[0];
    }

    public Object run() throws Exception {
		/* Check whether the caller has package access permission */
		JSClassLoader.checkPackageAccess(constructor.getDeclaringClass());

		return constructor.newInstance(args);
    }
}


/**
 * <P> PrivilegedCallMethodAction is used for executing the priviledge 
 * action for calling a method in an object.
 * </P>
 */
class PrivilegedCallMethodAction implements PrivilegedExceptionAction {

    Method method;
    Object obj;
    Object[] args;

    PrivilegedCallMethodAction(Method method, Object obj, Object[] args)
    {
		this.method = method;
	
		this.obj = obj;
		this.args = args;

		// Ensure the argument is not null
		if (this.args == null)
			this.args = new Object[0];
    }

    public Object run() throws Exception {
	/*
	 *  Browser uses reflection to collect methods/fields/constructors 
	 *  through this method call. Therefore JSClassLoader is not used
	 *  in such cases. However it requires filtering to avoid exposing
	 *  inaccessible methods/fields/constructors. 
	 *
	 *  Also, Mozilla ignores abstract methods, therefore those are 
	 *  replaced by the concrete class methods
	 */
	if(obj instanceof Class) {
	    String name = method.getName();
	    Class cls = (Class)obj;
	    if(name.equals("getMethods")) {
		Method[] methods = ReflectUtil.getJScriptMethods(cls);
		for (int i=0; i < methods.length; i++) {
		    Method m = methods[i];
		    if (Modifier.isAbstract(m.getModifiers())) {
			Class[] params = m.getParameterTypes();
			methods[i] = cls.getMethod(m.getName(), params);
		    }
		}
		return methods; 
	    }else if (name.equals("getFields")) {
		return ReflectUtil.getJScriptFields(cls);		
	    }else if (name.equals("getConstructors")) {
		if (!Modifier.isPublic(cls.getModifiers()) ||
		    !JSClassLoader.isPackageAccessible(cls)) {
		    return new Constructor[0];
		}
	    }
	}

	Method actualMethod = ReplaceMethod.getJScriptMethod(method);
	if(actualMethod != null)
	    return JSClassLoader.invoke(actualMethod, obj, args);
	else
	    throw new NoSuchMethodException(method.getName());
    }
}


/**
 * <P> PrivilegedGetFieldAction is used for executing the priviledge action
 * for getting a field in an object.
 * </P>
 */
class PrivilegedGetFieldAction implements PrivilegedExceptionAction {
    Field field;
    Object obj;

    PrivilegedGetFieldAction(Field field, Object obj)
    {
		this.field = field;
		this.obj = obj;
    }

    public Object run() throws Exception {	
		/* Check whether the caller has package access permission */
		JSClassLoader.checkPackageAccess(field.getDeclaringClass());

		return field.get(obj);
    }
}


/**
 * <P> PrivilegedSetFieldAction is used for executing the priviledge action
 * for setting a field in an object.
 * </P>
 */
class PrivilegedSetFieldAction implements PrivilegedExceptionAction {

    Field field;
    Object obj;
    Object val;

    PrivilegedSetFieldAction(Field field, Object obj, Object val)
    {
		this.field = field;
		this.obj = obj;
		this.val = val;
    }

    public Object run() throws Exception {
		/* Check whether the caller has package access permission */
		JSClassLoader.checkPackageAccess(field.getDeclaringClass());
	
		field.set(obj, val);
    	return null;
    }
}


