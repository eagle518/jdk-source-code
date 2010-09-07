/*
 * @(#)TypeConverter.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Array;
import java.text.NumberFormat;
import java.text.ParseException;
import com.sun.deploy.util.Trace;
import com.sun.deploy.resources.ResourceManager;

/**
 *  TypeConverter is used for converting an array of objects to the appropiate
 *  types that is used for method invocation in reflection.
 *
 *  Basically, only primitive types and string are converted if possible. This
 *  should make the method invocation through reflection much easier.
 */
public class TypeConverter
{
    /**
     *  Convert all parameters in an object array into the appropiate types
     *  accordingly to the defined class object in the class array.
     *
     *  @param clazzArray Class array
     *  @param params Object array
     *  @return Converted object array
     */
    static Object[] convertObjectArray(Class[] clazzArray, Object[] params)
            throws IllegalArgumentException
    {
	if(params == null || params.length == 0)
	    return params;

        if (clazzArray.length != params.length)
            throw new IllegalArgumentException(ResourceManager.getMessage("com.method.argCountInvalid"));

        Object[] result = new Object[params.length];

        // Convert parameter one-by-one
        for (int i=0; i < params.length; i++)
        {
            result[i] = convertObject(clazzArray[i], params[i]);
        }

        return result;
    }


    /**
     *  Convert object into the appropiate type accordingly to
     *  the class object.
     *
     *  @param clazz Class object
     *  @param param Object
     *  @return Converted object
     */
    public static Object convertObject(Class clazz, Object param)
                    throws IllegalArgumentException
    {
        if (param == null)
            return param;

        Class paramClazz = param.getClass();

        Trace.msgLiveConnectPrintln("com.field.needsConversion", 
				    new Object[] {paramClazz.getName(), clazz.getName()});

        // If the type matches, no need to convert
        if (clazz.isAssignableFrom(paramClazz)) {
            return param;
	}

	if(clazz.equals(netscape.javascript.JSObject.class)) {
	    return new WrapperJSObject(param);
	}

        if(param instanceof DispatchImpl) {
	    return ((DispatchImpl)param).getWrappedObject();
        }

        // Conversion is only done for primitive types and string, or primitive array
        if (clazz == java.lang.String.class)
        {
	    if(param instanceof Number) {
		NumberFormat nf = NumberFormat.getNumberInstance();
		try {
		     return nf.parse(param.toString()).toString();		    
		}catch(ParseException pexc) {
		    //Ignore and return the value of toString()
		}
	    }
	    return param.toString();
        }
        else if (clazz.isArray())
        {
            if (paramClazz.isArray())
            {
                Class compType = clazz.getComponentType();
                int len = Array.getLength(param);
                Object result = Array.newInstance(compType, len);

                // Convert array element one-by-one, recursively.
                for (int i=0; i < len; i++)
                    Array.set(result, i, convertObject(compType, Array.get(param, i)));

                return result;
            }
        }
        else if (clazz.isPrimitive() || Number.class.isAssignableFrom(clazz) 
		|| clazz == Character.class || clazz == Boolean.class)
        {
            String clazzName = clazz.getName();
	    boolean number = param instanceof Number;
	    boolean string = param instanceof String;

	    // The conversion is for primitive type
            //
            // Notice that char is not converted automatically
            //
            if (clazzName.equals("boolean") || clazzName.equals("java.lang.Boolean"))
            {
                return new Boolean(param.toString());
            }

	    if (clazzName.equals("byte") || clazzName.equals("java.lang.Byte"))
	    {
		if(string)
		    return Byte.valueOf((String)param);
		else if(number)
		    return new Byte(((Number)param).byteValue());
	    }
	    else if (clazzName.equals("short") || clazzName.equals("java.lang.Short"))
	    {
		if(string)
		    return Short.valueOf((String)param);
		else if(number)
		    return new Short(((Number)param).shortValue());
	    }
	    else if (clazzName.equals("int") || clazzName.equals("java.lang.Integer"))
	    {
		if(string)
		    return Integer.valueOf((String)param);
		else if(number)
		    return new Integer(((Number)param).intValue());
	    }
	    else if (clazzName.equals("long") || clazzName.equals("java.lang.Long"))
	    {
		if(string)
		    return Long.valueOf((String)param);
		else if(number)
		    return new Long(((Number)param).longValue());
	    }
	    else if (clazzName.equals("float") || clazzName.equals("java.lang.Float"))
	    {
		if(string)
		    return Float.valueOf((String)param);
		else if(number)
		    return new Float(((Number)param).floatValue());
	    }
	    else if (clazzName.equals("double") || clazzName.equals("java.lang.Double"))
	    {
		if(string)
		    return Double.valueOf((String)param);
		else if(number)
		    return new Double(((Number)param).doubleValue());
	    }
            else if (clazzName.equals("char") || clazzName.equals("java.lang.Character"))
            {
		if(string)
		    return new Character( (char)(Short.decode((String)param).shortValue()) );
		else if(number)
		    return new Character((char) ((Number)param).shortValue());
            }
	    else
	    {
		return param;
	    }
	}

        // We cannot handle conversion for non-primitive type
        // good luck.
        throw new IllegalArgumentException( paramClazz.getName() + 
		ResourceManager.getMessage("com.field.typeInvalid") + clazz.getName() );
    }
}



