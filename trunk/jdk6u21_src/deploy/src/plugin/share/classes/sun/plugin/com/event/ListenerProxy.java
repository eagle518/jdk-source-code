/*
 * @(#)ListenerProxy.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com.event;

import java.lang.reflect.Method;
import java.lang.reflect.Proxy;
import java.lang.reflect.InvocationHandler;
import sun.plugin.com.DispatchImpl;
import sun.plugin.com.BeanClass;
import java.beans.BeanInfo;
import java.beans.EventSetDescriptor;

public class ListenerProxy implements InvocationHandler {
    private COMEventHandler handler;
    private Object proxy;
    private DispatchImpl dispImpl;
    private boolean registerStatus = false;
    private Class lClasses[] = null;

    public ListenerProxy(int handle, DispatchImpl impl) {
	//Create a COMEventHandler
	dispImpl = impl;
	BeanClass bClass = (BeanClass)dispImpl.getTargetClass();
	handler = new COMEventHandler(handle, bClass);
    }

    public Object invoke(Object proxy, Method m, Object[] args)
	throws Throwable {

	if(lClasses == null)
	    return null;	    

	boolean event = false;
	for(int i=0;i<lClasses.length;i++) {
	    if(m.getDeclaringClass() == lClasses[i]){
		event = true;
		break;
	    }
	}

	if(handler != null && event) {
	    handler.notify(args[0], m);
	    return null;
	}
	else {
	    return m.invoke(dispImpl, args);
	}
    }

    public void unregister() {
	synchronized(this) {
	    if(!registerStatus)
		return;

	    try {
		Object bObj = dispImpl.getWrappedObject();
		BeanClass bClass = (BeanClass)dispImpl.getTargetClass();
		BeanInfo bInfo = bClass.getBeanInfo();
		EventSetDescriptor eds[]= bInfo.getEventSetDescriptors();

		Method lMethods[] = new Method[eds.length];
		Object args[] = new Object[] {proxy};
		for(int i=0;i<eds.length;i++) {
		    lMethods[i] = eds[i].getRemoveListenerMethod();
		    lMethods[i].invoke(bObj, args);
		}

		proxy = null;
	    }catch(Throwable exc){
		exc.printStackTrace();
	    }
	    registerStatus = false;
	}
    }

    public void register() {
	synchronized(this) {
	    if(registerStatus)
		return;

	    try {
		Object bObj = dispImpl.getWrappedObject();
		BeanInfo bInfo = ((BeanClass)dispImpl.getTargetClass()).getBeanInfo();
		EventSetDescriptor eds[]= bInfo.getEventSetDescriptors();

		lClasses = new Class[eds.length];
		Method lMethods[] = new Method[eds.length];

		for(int i=0;i<eds.length;i++) {
		    lClasses[i] = eds[i].getListenerType();
		    lMethods[i] = eds[i].getAddListenerMethod();
		}

		ClassLoader cl = bObj.getClass().getClassLoader();
		proxy = Proxy.newProxyInstance( cl, lClasses, this );
		Object args[] = new Object[] {proxy};
		for(int i=0;i<eds.length;i++) {
		    lMethods[i].invoke(bObj, args);
		}
	    }catch(Throwable exc){
		exc.printStackTrace();
	    }

	    registerStatus = true;
	}
    }
}




