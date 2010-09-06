/*
 * @(#)AppletEvent.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.applet;

import java.util.EventObject;


/**
 * AppletEvent class.
 *
 * @version 1.13, 12/19/03
 * @author  Sunita Mani
 */

public class AppletEvent extends EventObject {

    private Object arg;
    private int id;


    public AppletEvent(Object source, int id, Object argument) {
	super(source);
	this.arg = argument;
	this.id = id;
    }

    public int getID() {
	return id;
    }

    public Object getArgument() {
	return arg;
    }

    public String toString() {
	String str = getClass().getName() + "[source=" + source + " + id="+ id;
	if (arg != null) {
	    str += " + arg=" + arg;
	}
	str += " ]";
	return str;
    }
}
 

