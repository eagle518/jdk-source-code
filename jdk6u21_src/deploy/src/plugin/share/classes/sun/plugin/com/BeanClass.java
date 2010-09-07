/*
 * @(#)BeanClass.java	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.beans.BeanInfo;
import java.beans.MethodDescriptor;
import java.beans.PropertyDescriptor;
import java.beans.EventSetDescriptor;
import java.beans.Introspector;
import java.beans.IntrospectionException;
import java.util.Vector;
import java.util.HashMap;
import com.sun.deploy.resources.ResourceManager;

/**
 *  BeanClass is the Java side companion of the BeanClass COM object.
 *  Its job is to keep track of all the things we need to know about a
 *  Java class. Particularly what methods it has and how to call them.
 *  This class helps the JavaProxy class to make method calls on a
 *  particular instance of a Java object.
 */
public class BeanClass extends JavaClass
{
    private BeanInfo bInfo = null;

    private boolean collected = false;
    private MethodDescriptor methods[] = null;
    private PropertyDescriptor props[] = null;
    private EventSetDescriptor eds[] = null;

    NameIDMap evtIDMap = new NameIDMap();

    Method evtMethods[] = null;

    public BeanClass(Class classToWrap){
        super(classToWrap);
	try {
	    bInfo = Introspector.getBeanInfo(wrappedClass);
	}catch(IntrospectionException ixc){
	    ixc.printStackTrace();
	}
    }

    private MethodDescriptor getMethodDescriptor(int index) {
	return methods[index - Dispatch.methodBase];
    }

    private PropertyDescriptor getProperty(int index) {
	return props[index - Dispatch.propertyBase];
    }

    private Method getEventMethod(int index) {
	return evtMethods[index - Dispatch.eventBase];
    }

    public BeanInfo getBeanInfo(){
	return bInfo;
    }

    private MethodDescriptor getMethodDescriptor1(int index, Object [] args) 
	throws InvocationTargetException {

        int correlationIndex = -1;
        int minConversionCount = java.lang.Integer.MAX_VALUE;
	boolean ambiguous = false;

        // Find the minimum overloading correlation by enumerating through
        // the Method object array.
	String name = getMethodDescriptor(index).getName();
	int i = methIDMap.get(name);

        for (; i < methods.length; i++){
	    if( !methods[i].getName().equals(name) )
		continue;

            Class[] paramTypes = methods[i].getMethod().getParameterTypes();
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
		new Exception( getMethodDescriptor(index).getName() + 
				ResourceManager.getMessage("com.method.notexists") ));
	}

	return methods[correlationIndex];
    }

    public Dispatcher getDispatcher(int flag, int index, Object[] params) 
	throws InvocationTargetException {

	collect();
	Dispatcher disp = null;

	if((Dispatch.methodBase & index) > 0) {
	    MethodDescriptor md = getMethodDescriptor1(index, params);
	    if(md != null)
		disp = new MethodDispatcher(md.getMethod());
	}
	
	if((Dispatch.propertyBase & index) > 0) {
	    PropertyDescriptor pd = getProperty(index);
	    if(pd != null) {
		if( (flag & Dispatch.PROPERTYGET) > 0 ) {
		    disp = new MethodDispatcher(pd.getReadMethod());
		} else {
		    disp = new MethodDispatcher(pd.getWriteMethod());
		}
	    }
	}

	return disp;
    }

    synchronized protected void collect()
    {
        if (collected == false) {
	    methods = bInfo.getMethodDescriptors();
	    props = bInfo.getPropertyDescriptors();
	    eds = bInfo.getEventSetDescriptors();

	    Packager.sort(methods);
	    Packager.sort(props);

	    Vector evtMds = new Vector();
	    for(int i=0;i<eds.length;i++){
		Method m[] = eds[i].getListenerMethods();
		for(int j=0;j<m.length;j++){
		    evtMds.addElement(m[j]);
		}
	    }

	    evtMethods = (Method[])evtMds.toArray(new Method[0]);
	    sort(evtMethods);

	    if(methods != null) {
		for(int i=0;i<methods.length;i++) {
		    //Need to support overloaded methods
		    int id = methIDMap.get(methods[i].getName());
		    if(id == -1)
			methIDMap.put(methods[i].getName(), i);
		}
	    }

	    if(props != null) {
		for(int i=0;i<props.length;i++) {
		    fieldIDMap.put(props[i].getName(), i);
		}
	    }

	    if(eds != null) {
		for(int i=0;i<evtMethods.length;i++) {
		    //Need to support overloaded events
		    String name = evtMethods[i].getName();

		    int id = evtIDMap.get(name);
		    if(id == -1)
			evtIDMap.put(name, i);
		}
	    }

            collected = true;
        }
    }

    protected int getIdForName(String name) throws Exception {
	collect();
	int id = -1;

	//match methods
	id = methIDMap.get(name);
	if(id != -1)
	    return id+Dispatch.methodBase;

	id = getPropertyId(name);

	if(id == -1)
	    id = getEventId(name);

	if(id == -1)
	    throw new Exception( name + ResourceManager.getMessage("com.notexists") );

	return id;
    }


    public int getEventId(String name){
	collect();
    	//match events
	int id = evtIDMap.get(name);
	if(id != -1)
	    return id+Dispatch.eventBase;

	return -1;
    }

    public int getPropertyId(String name){
	collect();
	//match properties
	int id = fieldIDMap.get(name);
	if(id != -1)
	    return id+Dispatch.propertyBase;

	return -1;
    }

    protected int getReturnType(int id){
	Class clz = null;
	if(id >= Dispatch.methodBase)
	    clz = methods[id-Dispatch.methodBase].getMethod().getReturnType();
	else if(id >= Dispatch.propertyBase)
	    clz = props[id-Dispatch.propertyBase].getPropertyType(); 

	return Utils.getType(clz);
    }
}

