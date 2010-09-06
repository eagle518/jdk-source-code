/*
 * @(#)PropertySetDispatcher.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Field;
import sun.plugin.util.Trace;

/**
 * A <code>PropertyDispatcher</code> provides information about,
 * and access to, a property on a class or interface.  The
 * reflected property may be a class or an instance property
 * (including an abstract method).
 *
 * @see Member
 * @see java.lang.Class
 * @see java.lang.Field#get()
 * @see java.lang.Field#getType()
 */
public class PropertySetDispatcher implements Dispatcher
{
    private Field field = null;

    /**
     * Constructor
     */
    public PropertySetDispatcher(Field inField)
    {
	field = inField;
    }

    public Object invoke(Object obj, Object []params) throws Exception {
	if(field != null) {
	    Trace.msgLiveConnectPrintln("com.field.set", new Object[] {field});
	    Object val = TypeConverter.convertObject(field.getType(), params[0]);
	    field.set(obj, val);
	}
	return null;
    }

    public Class getReturnType() {
	return null;
    }
}
