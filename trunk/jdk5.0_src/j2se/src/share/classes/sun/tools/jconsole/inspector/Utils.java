/*
 * @(#)Utils.java	1.11 04/05/26
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;


// java import
import java.lang.*;
import java.math.*;
import java.lang.reflect.*;
import java.awt.*;
import java.util.*;
import java.io.*;
import javax.swing.*;

import javax.management.ObjectName;
import javax.management.openmbean.*;
//

public class Utils {
    private static int uniqueId= (int)(new Date().getTime()%100);
    private static Object objectClipBoard;
    private static int locationX=0, locationY=0;
    private static String INDENT_UNIT = "   ";
    private Utils () {        
    }
    
    public synchronized static Object getClipboardContents() {  
	return objectClipBoard;
    }
	
	
    public synchronized static void setClipboardContents(Object o) {
	objectClipBoard = o;
    }
	
    public static Point getNextLocation() {
	Point p = new Point(locationX,locationY); 
	locationX += 30; 
	locationY += 30;
	return p;
    }
	
    /**
     * This method attempts to generate an obvious ObjectName tag 
     * based on the class name of the object.
     */
    public static String generateName(String classString) {
	int pos;
	if ((pos=classString.lastIndexOf('.'))>0)
	    return classString.substring(pos+1,
					 classString.length()) + uniqueId++;
	else
	    return classString + uniqueId++;
    }
	
    public static String getUniqueId() {
	return Integer.toString(uniqueId++);
    }
    
    /**
     * This method returns the class matching the name className.
     * It's used to cater for the primitive types.
     */
    public static Class getClass(String className) throws Exception {
	try {
	    return Class.forName(className);
	}
	catch (ClassNotFoundException e) {
	    if (className.equals("int"))
		return Integer.TYPE;
	    if (className.equals("long"))
		return Long.TYPE;
	    if (className.equals("short"))
		return Short.TYPE;
	    if (className.equals("double"))
		return Double.TYPE;
	    if (className.equals("byte"))
		return Byte.TYPE;
	    if (className.equals("boolean"))
		return Boolean.TYPE;
	    if (className.equals("float"))
		return Float.TYPE;
	    if (className.equals("byte"))
		return Byte.TYPE;
	    if (className.equals("char"))
		return Character.TYPE;
	    throw e;
	}
    }

    public static boolean isSupportedDataStructure(Object elem) {
	if(isSupportedArray(elem)) return true;
	if(elem instanceof Collection) return true;
	return false;
    }

    public static boolean isSupportedArray(Object elem) {
	if(elem == null) return false;
	if(elem.getClass().isArray()) {
	    String name = elem.getClass().getName();
	    int index = name.lastIndexOf("[");
	    
	    //No support for multi-dimentional array.
	    if(index != 0) return false;
	    
	    return true;  
	} else
	    return false;
    }
    
    /**
     * This methods provides a readable className if it's an array otherwise
     * return null
     */
    public static String getArrayClassName(String name) {
	String className = null;
	    if (name.startsWith("[")) {
		//array
		int index = name.lastIndexOf("[");
		className = name.substring(index+1,name.length());
		char identifier = className.charAt(0);
		switch(identifier) {
		case 'L' :
		    //real class
		    className =  className.substring(1,className.length()-1);
		    break;
		    //Primitive Types
		case 'B' :
		    className = Byte.TYPE.getName();
		    break;
		case 'C' :
		    className = Character.TYPE.getName();
		    break;
		case 'D' :
		    className = Double.TYPE.getName();
		    break;
		case 'F' :
		    className = Float.TYPE.getName();
		    break;
		case 'I' :
		    className = Integer.TYPE.getName();
		    break;
		case 'J' :
		    className = Long.TYPE.getName();
		    break;
		case 'S' :
		    className = Short.TYPE.getName();
		    break;
		default :
		    className = Boolean.TYPE.getName();
		}
	    }
	return className;
    }

    /**
     * This methods provides a readable className if it's an array, 
     * otherwise return the same className
     */
    public static String getReadableClassName(String name) {
	String className = getArrayClassName(name);
	if(className == null) return name;
	
	int index = name.lastIndexOf("[");
	StringBuffer brackets = new StringBuffer(className);
	
	for (int i=0;i<= index; i++) {
	    brackets.append("[ ]");
	}
	return brackets.toString();
    }
		
    
    /**
     * This method tells weather the type is editable (means can be created 
     * with a String or not)
     */
    public static boolean isEditableType(String type) {
	if (type.equals("int")||
	    type.equals("long")||
	    type.equals("float")||
	    type.equals("short")||
	    type.equals("double")||
	    type.equals("char")||
	    type.equals("byte")||
	    type.equals("boolean") ||
	    type.equals("java.lang.Number")||
	    type.equals("java.math.BigInteger")||
	    type.equals("java.math.BigDecimal")||
	    type.equals("java.lang.Integer")||
	    type.equals("java.lang.Long")||
	    type.equals("java.lang.Float")||
	    type.equals("java.lang.Short")||
	    type.equals("java.lang.Double")||
	    type.equals("java.lang.Boolean") ||
	    type.equals("java.lang.Byte") || 
	    type.equals("java.lang.String") ||
	    type.equals("javax.management.ObjectName")) {
	    return true;
	}
	else {
	    return false;
	}
    }
	
    /**
     * This method inserts a default value for the standard java types, 
     * else it inserts the text name of the expected class type. 
     * It acts to give a clue as to the input type.
     */
    public static String getDefaultValue(String type) {
	if (type.equals("int")||
	    type.equals("long")||
	    type.equals("float")||
	    type.equals("short")||
	    type.equals("double")||
	    type.equals("char")||
	    type.equals("byte")||
	    type.equals("java.lang.Number")||
	    type.equals("java.math.BigInteger")||
	    type.equals("java.math.BigDecimal")||
	    type.equals("java.lang.Integer")||
	    type.equals("java.lang.Long")||
	    type.equals("java.lang.Float")||
	    type.equals("java.lang.Short")||
	    type.equals("java.lang.Double")||
	    type.equals("java.lang.Byte")) {
	    return "0";
	}
	else if (type.equals("boolean")) {
	    return "true";
	}   
	else {
	    int i;
	    if ((i =type.lastIndexOf('.'))>0)
		return type.substring(i+1,type.length());
	    else
		return type;
	}
    }
	
    /**
     * This method attempts to create an object of "type" by creation
     * with the "value" paramter.
     * eg. calling createObjectFromString("java.lang.Integer","10")
     * will return an Integer object initialized to 10.
     */
    public static Object createObjectFromString(String type, String value) 
	throws Exception {
	Object result;
	if (type.equals("int"))
	    result = new Integer(value);
	else 
	    if (type.equals("long"))
		result = new Long(value);
	    else 
		if (type.equals("double"))
		    result = new Double(value);
		else
		    if (type.equals("float"))
			result = new Float(value);
		    else
			if (type.equals("short"))
			    result = new Short(value);
			else 
			    if (type.equals("boolean"))
				result = new Boolean(value);
			    else
				if (type.equals("char"))
				    result = new Character(value.charAt(0));
				else 
				    if (type.equals("byte"))
					result = new Byte(value);
				    else
					if (type.equals("java.lang.Integer")) 
					    result = new Integer(value);
					else
					    if (type.equals("java.lang.Long")) 
						result = new Long(value);
					    else
						if (type.equals("java.lang.Float")) 
						    result = new Float(value);
						else
						    if (type.equals("java.lang.Double")) 
							result = new Double(value);
						    else
							if (type.equals("java.lang.Short")) 
							    result = new Short(value);
							else
							    if (type.equals("java.lang.Boolean")) 
								result = new Boolean(value);
							    else
								if (Utils.getClass("java.lang.Number").isAssignableFrom(Utils.getClass(type))) { 
								    result = createNumberFromString(value);
								}
								else {

								    //hack for null value
								    if (value.toString().equals("null")) {
									result = null;
								    }
								    else {
									if(type.equals("javax.management.ObjectName")) {
									    result = new ObjectName(value);
									}											// try create the object with a single String constructor
									Class sig[] = {value.toString().getClass()};
									Constructor c = Utils.getClass(type).getConstructor(sig);
									Object[] paramArray = new Object[1];
									paramArray[0] = value;
									result = c.newInstance(paramArray);	
								    }		
								}
	return result;
    }
	
    /**
     * This method is responsible for taking the inputs given by the user
     * into a usefull object array for passing into a parameter array.
     */
    public static Object[] getParameters(XTextField[] inputs, String[] params) throws Exception {
	Object result[] = new Object[inputs.length];
	Object userInput;
	for (int i = 0; i < inputs.length; i++) {
	    userInput = inputs[i].getValue();
	    // if it's already a complex object, use the value
	    // else try to instantiate with string constructor
	    if (userInput instanceof XObject) {
		result[i] = ((XObject)userInput).getObject();
	    }
	    else {
		result[i] = null;
		
		if (params[i].toString().equals("int"))
		    result[i] = new Integer((String)userInput);
		else 
		    if (params[i].toString().equals("long"))
			result[i] = new Long((String)userInput);
		    else
			if (params[i].toString().equals("float"))
			    result[i] = new Float((String)userInput);
			else
			    if (params[i].toString().equals("double"))
				result[i] = new Double((String)userInput);
			    else 
				if (params[i].toString().equals("short"))
				    result[i] = new Short((String)userInput);
				else
				    if (params[i].toString().equals("char"))
					result[i] = new Character(((String)userInput).charAt(0));
				    else
					if (params[i].toString().equals("boolean"))
					    result[i] = new Boolean((String)userInput);
					else
					    if (Utils.getClass("java.lang.Number").isAssignableFrom(Utils.getClass(params[i]))) {
						result[i] = createNumberFromString((String)userInput);
					    }
					    else {
						//hack for null value
						if (userInput.toString().equals("null")) {
						    result[i] = null;
						}
						else {
						    // try create the object with a single String constructor
						    Class sig[] = {userInput.toString().getClass()};
						    Constructor c = (Utils.getClass(params[i]).getConstructor(sig));
						    Object[] paramArray = new Object[1];
						    paramArray[0] = userInput;
						      				
						    result[i] = c.newInstance(paramArray);
						}
					    }
	    } 
	}
	return result;
    }
	
    private static java.lang.Number createNumberFromString(String val) 
	throws NumberFormatException {
	java.lang.Number result;			
	try { 
	    return result = new Integer(val); 
	} 
	catch(NumberFormatException e1) { 
	    try { 
		return result = new Long(val); 
	    } 
	    catch(NumberFormatException e2) { 
		try { 
		    return result = new Float(val); 
		} 
		catch(NumberFormatException e3) { 
		    try { 
			return result = new Double(val); 
		    } 
		    catch(NumberFormatException e4){ 
			try { 
			    return result = new Short(val); 
			} 
			catch(NumberFormatException e5){ 							
			    try { 
				return result = new Byte(val); 
			    } 
			    catch(NumberFormatException e6){ 
				try { 
				    return result = new BigDecimal(val); 
				} 
				catch(NumberFormatException e7){ 
				    try { 
					return result = new BigInteger(val); 
				    } 
				    catch(NumberFormatException e8){ 
					throw new NumberFormatException("Cannot convert String \""+ val +"\" to java.lang.Number");
				    }
				}
			    }
			}
						
		    } 
		} 
	    } 
	}
    }
	
    public static boolean 
	acceptDrag4337114WorkAround(java.awt.dnd.DragGestureEvent e) {
	//Drag on non selected Jinternal frame produce a bug
	//one way to avoid the bug is see if the last event is a mouse exited
	   
	Iterator events = e.iterator();
	    
	Object last = null;
	while (events.hasNext()) {
	    last = events.next();
	}
	if (last instanceof java.awt.event.MouseEvent) {
	    java.awt.event.MouseEvent event = 
		(java.awt.event.MouseEvent) last;
	    if (event.getID() == java.awt.event.MouseEvent.MOUSE_EXITED) {
		return false;
	    }
	    else {
		return true;
	    }
	}
	else {
	    return true;
	}
    }

    /**
     * If the exception is wrapped, unwrap it.
     */
    public static Throwable getActualException(Throwable e) {
	if (e.getClass().getName().
	    equals("javax.management.MBeanException")) {
	    return invokeMethod(e,"getTargetException");
	}
	if (e.getClass().getName().
	    equals("javax.management.RuntimeMBeanException")) {
	    return invokeMethod(e,"getTargetException");
	}
	if (e.getClass().getName().
	    equals("javax.management.ReflectionException")) {
	    return invokeMethod(e,"getTargetException");
	}
	if (e.getClass().getName().
	    equals("com.sun.jdmk.comm.CommunicationException")) {
	    return invokeMethod(e,"getTargetException");
	}
	if (e.getClass().getName().
	    equals("com.sun.jdmk.ProxyMBeanInstantiationException")) {
	    return invokeMethod(e,"getTargetException");
	}
	return e;
    }
	
    private static Throwable invokeMethod(Throwable o, String methodName) {
	try {
	    Method m = o.getClass().getDeclaredMethod(methodName,
						      (Class[])null);
	    return (Throwable)m.invoke(o,(Object[])null);
	}
	catch (Throwable e) {
	    return o;
	}
    }
	
    public static void sleep(int ms) {
	try {
	    Thread.sleep(ms);
	}
	catch (Throwable t) {
	}
    }
}
