/*
 * @(#)PlatformMXBeanInvocationHandler.java	1.2 04/04/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.util.Map;
import java.util.HashMap;
import javax.management.Attribute;
import javax.management.InstanceNotFoundException;
import javax.management.MBeanServerConnection;
import javax.management.MBeanServerInvocationHandler;
import javax.management.NotificationBroadcaster;
import javax.management.NotificationEmitter;
import javax.management.ObjectName;

public class PlatformMXBeanInvocationHandler 
        extends MBeanServerInvocationHandler {

    private final MBeanServerConnection connection;
    private final ObjectName objectName;
    private final Class<?> mxbeanInterface;
    private final MXBeanSupport mxbeanSupport;
    public PlatformMXBeanInvocationHandler(MBeanServerConnection mbsc,
                                           ObjectName on,
                                           Class<?> mxbeanInterface) 
        throws java.io.IOException {
        super(mbsc, on);
        if (mbsc == null || on == null || mxbeanInterface == null) {
            throw new IllegalArgumentException("Null parameter");
        }

        this.connection = mbsc;
        this.objectName = on;
        this.mxbeanInterface = mxbeanInterface;
        this.mxbeanSupport = MXBeanSupport.newProxy(mxbeanInterface);
    }

    public Object invoke(Object proxy, Method method, Object[] args)
             throws Throwable {

        // Forward the NotificationEmitter interface to superclass
        final Class methodClass = method.getDeclaringClass();
        if (methodClass.equals(NotificationBroadcaster.class) || 
            methodClass.equals(NotificationEmitter.class)) {
            return super.invoke(proxy, method, args);
        }

        return mxbeanSupport.forward(connection, objectName, method, args);
    }

}
