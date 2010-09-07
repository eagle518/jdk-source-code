/*
 * @(#)BeansApplet.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin;

import java.applet.Applet;
import java.awt.BorderLayout;
import java.awt.Component;

public class BeansApplet extends Applet
{
    Object bean;
    Component c;

    BeansApplet(Object obj) {
        bean = obj;
	if(bean instanceof Component) {
	    c = (Component)obj;
	}
    }

    public void init() {
    	if(c != null)
	    setLayout(new BorderLayout());
    }

    public void start() {
	if(c != null)
	    add(c);
    }

    public void stop() {
	if(c != null)
	    remove(c);
    }

    public void destroy() {
        c = null;
	bean = null;
    }

    public Object getBean() {
	return bean;
    }
}

