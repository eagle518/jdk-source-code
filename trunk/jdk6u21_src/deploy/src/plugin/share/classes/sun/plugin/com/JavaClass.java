/*
 * @(#)JavaClass.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.util.HashMap;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Member;
import java.util.Comparator;
import sun.plugin.javascript.ReflectUtil;
import java.lang.reflect.InvocationTargetException;
import com.sun.deploy.resources.ResourceManager;

/**
 *  JavaClass is the Java side companion of the JavaClass COM object.
 *  Its job is to keep track of all the things we need to know about a
 *  Java class. Particularly what methods it has and how to call them.
 *  This class helps the JavaProxy class to make method calls on a
 *  particular instance of a Java object.
 */
public class JavaClass
{
    private static final boolean writeDebug = false;

    /* Flags for IDispatch::Invoke */
    protected Class wrappedClass = null;

    private boolean collected = false;
    protected Method methods[] = null;
    protected Field fields[] = null;
    NameIDMap methIDMap = new NameIDMap();
    NameIDMap fieldIDMap = new NameIDMap();

    public JavaClass(Class classToWrap)
    {
        wrappedClass = classToWrap;
    }

    public String getName()
    {
        return wrappedClass.getName();
    }

    public Method getMethod(int index) {
	return methods[index - Dispatch.methodBase];
    }

    public Method getMethod1(int index, Object [] args) 
	throws InvocationTargetException {

        int correlationIndex = -1;
        int minConversionCount = java.lang.Integer.MAX_VALUE;
	boolean ambiguous = false;

        // Find the minimum overloading correlation by enumerating through
        // the Method object array.
	String name = getMethod(index).getName();
	int i = methIDMap.get(name);

        for (; i < methods.length; i++){
	    if( !methods[i].getName().equals(name) )
		continue;

            Class[] paramTypes = methods[i].getParameterTypes();
	    if((args == null || args.length == 0) && paramTypes.length == 0)
		return methods[i];

	    ParameterListCorrelator correlator = new ParameterListCorrelator(paramTypes, args);
	    if(correlator.parametersCorrelateToClasses()) {
		// Check overloading correlation for all parameters
		// to the appropiate type
		int conversionCount = correlator.numberOfConversionsNeeded();
		if(conversionCount < minConversionCount) {
		    correlationIndex = i;
		    minConversionCount = conversionCount;
		    ambiguous = false;
		    if(conversionCount == 0)
			break;
		} else if (conversionCount == minConversionCount) {
		    ambiguous = true;
		}
	    }
        }

	if(ambiguous == true) {
            // Collision occurs in overloading.
            throw new InvocationTargetException(
                new Exception( ResourceManager.getMessage("com.method.ambiguous") ));
	}


        // The minimum overloading conversionCount has been found.
        // Check if there is collision in overloading.
        if (minConversionCount == java.lang.Integer.MAX_VALUE)
	{
            throw new InvocationTargetException(
		new Exception( ResourceManager.getFormattedMessage("com.method.notexists",
                                                                   new Object[] { getMethod(index).getName() } )) );
	}

	return methods[correlationIndex];
    }

    public Field getField(int index) {
	return fields[index - Dispatch.propertyBase];
    }

    public Dispatcher getDispatcher(int flag, int index, Object[] params) 
	throws InvocationTargetException {

	Dispatcher disp = null;

	if((Dispatch.methodBase & index) > 0) {
	    Method method = getMethod1(index, params);
	    if(method != null)
		disp = new MethodDispatcher(method);
	}
	
	if((Dispatch.propertyBase & index) > 0) {
	    Field field = getField(index);
	    if(field != null) {
		if( (flag & Dispatch.PROPERTYGET) > 0 ) {
		    disp = new PropertyGetDispatcher(field);
		} else {
		    disp = new PropertySetDispatcher(field);
		}
	    }
	}

	return disp;
    }

    synchronized protected void collect()
    {
        if (collected == false) {
	    methods = ReflectUtil.getJScriptMethods(wrappedClass);
	    fields = ReflectUtil.getJScriptFields(wrappedClass);
	    sort(methods);
	    sort(fields);

	    if(methods != null) {
		for(int i=0;i<methods.length;i++) {
		    //Need to support overloaded methods
		    int id = methIDMap.get(methods[i].getName());
		    if(id == -1)
			methIDMap.put(methods[i].getName(), i);
		}
	    }

	    if(fields != null) {
		for(int i=0;i<fields.length;i++) {
		    fieldIDMap.put(fields[i].getName(), i);
		}
	    }

            collected = true;
        }
    }

    static void sort(Object[] elements) {
	java.util.Arrays.sort(elements, new Comparator() {
	    public int compare(Object o1, Object o2) {
		String x = ((Member)o1).getName();
		String y = ((Member)o2).getName();
		return x.compareTo(y);
	    }
	});
    }

    protected int getIdForName(String name) throws Exception {
	collect();

	int id = methIDMap.get(name);
	if(id != -1)
	    return id+Dispatch.methodBase;

	id = fieldIDMap.get(name);
	if(id != -1)
	    return id+Dispatch.propertyBase;

	throw new Exception( ResourceManager.getFormattedMessage("com.notexists", new Object[] { name }) );
    }

}


class NameIDMap extends HashMap {
    public int get(String name) {
	Integer temp = (Integer)super.get(name.toLowerCase());
	if(temp != null)
	    return temp.intValue();
	else
	    return -1;
    }

    public void put(String name, int val) {
	super.put((Object)name.toLowerCase(), new Integer(val));
    }
}
