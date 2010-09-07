/*
 * @(#)Utils.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import sun.plugin.javascript.ocx.JSObject;
import java.lang.reflect.Array;
import sun.plugin.com.Dispatch;

public final class Utils
{
    public static Object convertReturn(Class retType, Object value, int handle)
    {
	Object retValue = value;

	if(retValue == null) {
	    if (retType.equals(String.class)) {
		retValue = new NullString();
	    } else {
		if (retType.isArray()) {
		    retValue = new Object[0];
		}
	    }
	} else {
	    if (!retType.isArray()) {
			return convertRetVal(retType, value, handle);
	    } else {
			Class compType = value.getClass().getComponentType();
                
			if(compType.isPrimitive()) return value;

			int len = Array.getLength(value);
		    
			if(requiresUnWrapping(compType))
				compType = DispatchClient.class;
			else
				compType = DispatchImpl.class; 

			retValue = Array.newInstance(compType, len);

			// Convert array element one-by-one, recursively.
			for (int i=0; i < len; i++) {
				Array.set(retValue, i, convertReturn(compType, Array.get(value, i), handle));
			}
	    }
	}
	return retValue;
    }

    private static boolean requiresWrapping(Class type) {
	if( Number.class.isAssignableFrom(type) || 
	    type == String.class || type == Character.class || 
	    type == Boolean.class )
	    return false;
	else
	    return true;
    }

    private static boolean requiresUnWrapping(Class type) {
	if(sun.plugin.javascript.ocx.JSObject.class.isAssignableFrom(type))
	    return true;
	else 
	    return false;
    }

    /* <p>
     * This method is invoked internally by convertReturn to 
     * convert the returned value into correct type
     *
     * 
     * params: retType -- The actual returned type
     *         obj     -- The returned value from invoke
     *         handle  -- Windows handler
     * return: returns the wrapped object or the original data if primitive
     *
     *</p>
     */
    private static Object convertRetVal(Class retType, Object obj, int handle) {
		if (retType.isPrimitive() || retType == String.class) {
			return obj;
		}
		else if (requiresUnWrapping(retType)) {
			return ((JSObject)obj).getDispatchClient();
		}
		else {
			return new DispatchImpl(obj, handle);
		}
    }
	
    /* <p>
     * This method is invoked by DispatchClient to convert the argument to appropriate
     * type before calling to Javascript
     * <p>
     */
    public static Object[] convertArgs(Object args[], int handle) {
	Object [] convertedObj = new Object[args.length];
	for(int i=0;i<args.length;i++ ) {
	    if(args[i] != null)
		convertedObj[i] = convertArg(args[i], handle);
	}
	return convertedObj;
    }

    public static Object convertArg(Object obj, int handle) {
	if(requiresUnWrapping(obj.getClass()))
	    return ((JSObject)obj).getDispatchClient();
	else if(requiresWrapping(obj.getClass())) {
	    assert(!(obj instanceof DispatchImpl));
	    return new DispatchImpl(obj, handle);
	}else 
	    return obj;
    }

    public static int getType(Class clazz) {
	if(clazz == Void.TYPE)
	    return 0;
	else if(clazz == Boolean.TYPE)
	    return Dispatch.JT_BOOL;
	else if(clazz == Byte.TYPE)
	    return Dispatch.JT_BYTE;
	else if(clazz == Character.TYPE)
	    return Dispatch.JT_CHAR;
	else if(clazz == Short.TYPE)
	    return Dispatch.JT_SHORT;
	else if(clazz == Integer.TYPE)
	    return Dispatch.JT_INT;
	else if(clazz == Long.TYPE)
	    return Dispatch.JT_LONG;
	else if(clazz == Float.TYPE)
	    return Dispatch.JT_FLOAT;
	else if(clazz == Double.TYPE)
	    return Dispatch.JT_DOUBLE;
	else if(clazz == String.class) 
	    return Dispatch.JT_STRING;
	else if(clazz.isArray())
	    return Dispatch.JT_ARRAY;
	    
	return Dispatch.JT_OBJECT;
    }
}
