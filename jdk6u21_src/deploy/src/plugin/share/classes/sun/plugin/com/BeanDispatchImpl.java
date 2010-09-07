/*
 * @(#)BeanDispatchImpl.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.applet.Applet;
import java.beans.Beans;
import java.awt.Component;
import java.awt.Color;
import java.awt.Font;
import java.lang.reflect.Method;

/**
 *  BeanDispatchImpl encapsulates a Java Object and provides Dispatch interface
 *  It is responsible for maintaining the identity and type of one instance of
 *  a Java object. Objects of this type are used to invoke methods on the java
 *  object that they represent.
 */
public class BeanDispatchImpl extends DispatchImpl implements AmbientProperty
{
    /*
     * Constructor
     * @param obj the object to be wrapped
     */
    public BeanDispatchImpl(Object obj, int id) {
	super(obj, id);
	setBridge();
    }


    /**
    * Invoke a method according to the method index.
    *
    * @param flag Invoke flag
    * @param index Method index
    * @param params Arguments.
    * @return Java object.
    */
    public Object invoke(int flag, int index, Object []params)
	throws  Exception {
	Object obj = invokeImpl(flag, index, params);
	if(obj instanceof DispatchImpl) {
	    ((DispatchImpl)obj).setBridge();
	}
	return obj;
    }

    public JavaClass getTargetClass() {
	if(targetClass == null && targetObj != null) {
	    targetClass = new BeanClass(targetObj.getClass());
	}

	return targetClass;
    }

    public void setBackground(int r, int g, int b){
	if(targetObj instanceof Component) {
	    Component comp = (Component)targetObj;
	    comp.setBackground(new Color(r, g, b));
	}
    }

    public void setForeground(int r, int g, int b){
	if(targetObj instanceof Component) {
	    Component comp = (Component)targetObj;
	    comp.setForeground(new Color(r, g, b));
	}
    }

    public void setFont(String name, int style, int size){
	if(targetObj instanceof Component) {
	    Component comp = (Component)targetObj;
	    comp.setFont(new Font(name, style, size));
	}
    }

    public int getBackground(){
	if(targetObj instanceof Component) {
	    Component comp = (Component)targetObj;
	    return comp.getBackground().getRGB();
	}

	return 0;
    }

    public int getForeground(){
	if(targetObj instanceof Component) {
	    Component comp = (Component)targetObj;
	    return comp.getForeground().getRGB();
	}

	return 0;
    }

    public Font getFont(){
	if(targetObj instanceof Component) {
	    Component comp = (Component)targetObj;
	    return comp.getFont();
	}

	return null;
    }
}

