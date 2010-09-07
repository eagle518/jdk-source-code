/*
 * @(#)COMEventHandler.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com.event;

import sun.plugin.com.DispatchImpl;
import java.lang.reflect.Method;
import sun.plugin.com.Utils;
import sun.plugin.com.BeanClass;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyVetoException;

public class COMEventHandler implements COMEventListener{
    private int handle;
    private BeanClass bClass;
    private final static String propertyChange = "propertyChange";
    private final static String vetoableChange = "vetoableChange";
    
    COMEventHandler(int handle, BeanClass bClass) {
	this.handle = handle;
	this.bClass = bClass;
    }

    public void notify(Object evt, Method m)throws Throwable{
	if(m.getName().equals(propertyChange)) {
	    propertyChangeHandler((PropertyChangeEvent)evt);
	} else if (m.getName().equals(vetoableChange)) {
	    vetoableChangeHandler((PropertyChangeEvent)evt);
	}

	//System.out.println("Event: " + m.getName());
	int evtID = bClass.getEventId(m.getName());
	if(evtID != -1) {
	    notifyEvent(handle, evtID, evt, m);
	}
    }

    private void notifyEvent(int handle, int evtID, Object evt, Method m) {
	Class []types = m.getParameterTypes();
	Object obj = Utils.convertReturn(types[0], (Object)evt, handle);
	nativeNotifyEvent(handle, evtID, new Object[]{obj});
    }


    private void propertyChangeHandler(PropertyChangeEvent evt){
	String name = evt.getPropertyName();
	int propID = bClass.getPropertyId(name);
	if(propID != -1) {
	    nativeNotifyProperty(handle, propID);
	}
    }

    private void vetoableChangeHandler(PropertyChangeEvent evt) throws PropertyVetoException{
	String name = evt.getPropertyName();
	boolean result = true;
	int propID = bClass.getPropertyId(name);
	if(propID != -1) {
	    if(!nativeNotifyVetoable(handle, propID)){
		throw new PropertyVetoException(name, evt);
	    }
	}
    }

    private native boolean nativeNotifyVetoable(int handle, int dispid);
    private native void nativeNotifyProperty(int handle, int dispid);
    private native void nativeNotifyEvent(int handle, int dispid, Object[] args);
}

