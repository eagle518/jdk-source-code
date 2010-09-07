/*
 * @(#)PropertyGetDispatcher.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Field;
import com.sun.deploy.util.Trace;

/**
 * A <code>PropertyGetDispatcher</code> provides information about,
 * and access to, a property on a class or interface.  The
 * reflected property may be a class or an instance property
 * (including an abstract method).
 *
 * @see Member
 * @see java.lang.Class
 * @see java.lang.Field#get()
 * @see java.lang.Field#getType()
 */
public class PropertyGetDispatcher implements Dispatcher
{
    private Field field = null;

    /**
     * Constructor
     */
    public PropertyGetDispatcher(Field inField) {
	field = inField;
    }

    public Object invoke(Object obj, Object []params)throws Exception {
	Object propVal = null;
	if(field != null) {
	    Trace.msgLiveConnectPrintln("com.field.get", new Object[] {field});
	    propVal = field.get(obj);
	}	    

	return propVal;
    }

    public Class getReturnType() {
	return field.getType();
    }
}
