package com.sun.beans;

import java.beans.*;
import java.io.*; 
import java.util.*; 

import org.xml.sax.*;

/**
 * <b>WARNING</b>: This class is an implementation detail and only meant
 * for use within the core platform. You should NOT depend upon it! This
 * API may change drastically between dot dot release, and it may even be
 * removed.
 *
 * @see java.beans.XMLEncoder
 * @see java.io.ObjectInputStream
 *
 * @since 1.4
 * 
 * @version 1.5 11/20/00
 * @author Philip Milne
 */
public class ObjectHandler extends HandlerBase {

    public static Class typeNameToClass(String typeName) {
        typeName = typeName.intern();
        if (typeName == "boolean") return Boolean.class;
        if (typeName == "byte") return Byte.class;
        if (typeName == "char") return Character.class;
        if (typeName == "short") return Short.class;
        if (typeName == "int") return Integer.class;
        if (typeName == "long") return Long.class;
        if (typeName == "float") return Float.class;
        if (typeName == "double") return Double.class;
        if (typeName == "void") return Void.class;
        return null;
    }

    public static Class typeNameToPrimitiveClass(String typeName) {
        typeName = typeName.intern();
        if (typeName == "boolean") return boolean.class;
        if (typeName == "byte") return byte.class;
        if (typeName == "char") return char.class;
        if (typeName == "short") return short.class;
        if (typeName == "int") return int.class;
        if (typeName == "long") return long.class;
        if (typeName == "float") return float.class;
        if (typeName == "double") return double.class;
        if (typeName == "void") return void.class;
        return null;
    }

    public static Class classForName(String name) throws ClassNotFoundException {
	return classForName(name, Thread.currentThread().getContextClassLoader());
    }

    public static Class classForName(String name, ClassLoader cl) 
	throws ClassNotFoundException {
	if (cl == null) {
	    throw new IllegalArgumentException("ClassLoader cannot be null");
	}

        Class primitiveType = typeNameToPrimitiveClass(name);
        if (primitiveType != null) {
            return primitiveType;
        }
        return Class.forName(name, false, cl);
    }

    private Hashtable environment; 
    private Vector expStack; 
    private StringBuffer chars; 
    private XMLDecoder is; 
    private ClassLoader ldr;
    private int itemsRead = 0; 

    public ObjectHandler() { 
        environment = new Hashtable();
        expStack = new Vector(); 
        chars = new StringBuffer(); 
    }

    public ObjectHandler(XMLDecoder is) { 
        this();
        this.is = is; 
    } 
    
    /* loader can be null */
    public ObjectHandler(XMLDecoder is, ClassLoader loader) {
        this(is);
        this.ldr = loader;
    }


    public void reset() {
        expStack.clear();
        chars.setLength(0); 
        MutableExpression e = new MutableExpression();
        e.setTarget(classForName2("java.lang.Object"));
        e.setMethodName("null");
        expStack.add(e); 
    }

    private Object getValue(Expression exp) { 
        try { 
            return exp.getValue(); 
        }
        catch (Exception e) { 
            if (is != null) {
                is.getExceptionListener().exceptionThrown(e); 
            }
            return null; 
        }
    }
    
    private void addArg(Object arg) { 
        lastExp().addArg(arg); 
    } 
    
    private Object pop(Vector v) { 
        int last = v.size()-1; 
        Object result = v.get(last); 
        v.remove(last); 
        return result; 
    }
    
    private Object eval() { 
        return getValue(lastExp()); 
    }
    
    private MutableExpression lastExp() { 
        return (MutableExpression)expStack.lastElement(); 
    } 
    
    public Object dequeueResult() { 
        Object[] results = lastExp().getArguments(); 
        return results[itemsRead++]; 
    }
    
    private boolean isPrimitive(String name) { 
        return name != "void" && typeNameToClass(name) != null;
    }
    
    private void simulateException(String message) { 
        Exception e = new Exception(message); 
    	e.fillInStackTrace(); 
        if (is != null) {
            is.getExceptionListener().exceptionThrown(e); 
        }
    } 
	
    private Class classForName2(String name) {
        try { 
            if (ldr != null) {
                return classForName(name, ldr);
            } else {
                return classForName(name); 
            }
        }
        catch (ClassNotFoundException e) { 
            if (is != null) {
                is.getExceptionListener().exceptionThrown(e);
            }
        } 
        return null; 
    } 
    
    private HashMap getAttributes(AttributeList attrs) { 
        HashMap attributes = new HashMap(); 
        if (attrs != null && attrs.getLength() > 0) {
            for(int i = 0; i < attrs.getLength(); i++) { 
                attributes.put(attrs.getName(i), attrs.getValue(i)); 
            }
        } 
        return attributes; 
    }
    
    public void startElement(String name, AttributeList attrs) throws SAXException { 
        name = name.intern(); // Xerces parser does not supply unique tag names.
        chars.setLength(0); 

        HashMap attributes = getAttributes(attrs);  
        MutableExpression e = new MutableExpression(); 
        
        // Target
        String className = (String)attributes.get("class"); 
        if (className != null) {
            e.setTarget(classForName2(className));
        } 
        
        // Property
        Object property = attributes.get("property"); 
        String index = (String)attributes.get("index"); 
        if (index != null) { 
            property = new Integer(index); 
            e.addArg(property); 
        } 
        e.setProperty(property); 
        
        // Method
        String methodName = (String)attributes.get("method"); 
        if (methodName == null && property == null) {
            methodName = "new"; 
        } 
        e.setMethodName(methodName); 
        
        // Tags
        if (name == "string") {
            e.setTarget(String.class);
            e.setMethodName("new");
        }
        else if (isPrimitive(name)){
            Class wrapper = typeNameToClass(name);
            e.setTarget(wrapper);
            e.setMethodName("new");
        }
        else if (name == "class") {
            e.setTarget(Class.class);
            e.setMethodName("forName");
        }
        else if (name == "null") {
            // Create an arbitrary expression that has a value of null - for
            // consistency.
            e.setTarget(Object.class);
            e.setMethodName("getSuperclass");
            e.setValue(null);
        }
        else if (name == "void") { 
            if (e.getTarget() == null) { // this check is for "void class="foo" method= ..." 
	        e.setTarget(eval()); 
	    }
        }
    	else if (name == "array") { 
            // The class attribute means sub-type for arrays. 
            String subtypeName = (String)attributes.get("class"); 
            Class subtype = (subtypeName == null) ? Object.class : classForName2(subtypeName); 
            String length = (String)attributes.get("length"); 
            if (length != null) { 
                e.setTarget(java.lang.reflect.Array.class); 
                e.addArg(subtype); 
                e.addArg(new Integer(length)); 
            }
            else { 
                Class arrayClass = java.lang.reflect.Array.newInstance(subtype, 0).getClass(); 
                e.setTarget(arrayClass); 
            }
        } 
        else if (name == "java") { 
            e.setValue(is); // The outermost scope is the stream itself. 
        }
	else if (name == "object") { 
	}
	else { 
            simulateException("Unrecognized opening tag: " + name + " " + attrsToString(attrs)); 
	    return; 
        } 
		
        // ids
        String idName = (String)attributes.get("id");
        if (idName != null) {
            environment.put(idName, e);
        }

        // idrefs
        String idrefName = (String)attributes.get("idref");
        if (idrefName != null) {
            e.setValue(lookup(idrefName));
        }

        // fields
        String fieldName = (String)attributes.get("field"); 
        if (fieldName != null) { 
            e.setValue(getFieldValue(e.getTarget(), fieldName)); 
        }
        expStack.add(e); 
    }
    
    private Object getFieldValue(Object target, String fieldName) { 
        try { 
            Class type = target.getClass(); 
            if (type == Class.class) { 
                type = (Class)target; 
            } 
            java.lang.reflect.Field f = type.getField(fieldName); 
            return f.get(target); 
        }
        catch (Exception e) { 
            if (is != null) {
                is.getExceptionListener().exceptionThrown(e); 
            }
            return null; 
        }
    }            
            
    private String attrsToString(AttributeList attrs) { 
        StringBuffer b = new StringBuffer(); 
        for (int i = 0; i < attrs.getLength (); i++) {
            b.append(attrs.getName(i)+"=\""+attrs.getValue(i)+"\" ");
        }
        return b.toString(); 
    }            
            
    public void characters(char buf [], int offset, int len) throws SAXException {
        chars.append(new String(buf, offset, len));
    }

    public Object lookup(String s) { 
        Expression e = (Expression)environment.get(s); 
        if (e == null) { 
            simulateException("Unbound variable: " + s); 
        }
        return getValue(e); 
    }

    public void endElement(String name) throws SAXException {
        name = name.intern(); // Xerces parser does not supply unique tag names.
        if (name == "java") { 
            return; 
        }
        if (isPrimitive(name) || name == "string" || name == "class") {
            addArg(chars.toString());
        }
        if (name == "object" || name == "array" || name == "void" ||
                isPrimitive(name) || name == "string" || name == "class" ||
                name == "null") {
            Expression e = (Expression)pop(expStack); 
            Object value = getValue(e); 
            if (name != "void") {
                addArg(value); 
            }
        }
        else { 
            simulateException("Unrecognized closing tag: " + name);
        }
    }
}


class MutableExpression extends Expression { 
    private Object target;
    private String methodName;

    private Object property; 
    private Vector argV = new Vector();  

    private String capitalize(String propertyName) { 
        if (propertyName.length() == 0) { 
            return propertyName; 
        }
        return propertyName.substring(0, 1).toUpperCase() + propertyName.substring(1);
    } 
    
    public MutableExpression() { 
        super(null, null, null); 
    } 
    
    public Object[] getArguments() { 
    	return argV.toArray(); 
    }
    
    public String getMethodName() { 
    	if (property == null) { 
            return methodName;
    	} 
        int setterArgs = (property instanceof String) ? 1 : 2; 
        String methodName = (argV.size() == setterArgs) ? "set" : "get"; 
        if (property instanceof String) { 
            return methodName + capitalize((String)property); 
        }
        else { 
            return methodName; 
        }
    }
    
    public void addArg(Object arg) { 
    	argV.add(arg); 
    } 
    
    public void setTarget(Object target) { 
    	this.target = target; 
    } 
    
    public Object getTarget() {
        return target;
    }

    public void setMethodName(String methodName) { 
    	this.methodName = methodName; 
    }

    public void setProperty(Object property) { 
    	this.property = property; 
    }

    public void setValue(Object value) { 
        super.setValue(value);
    }

    public Object getValue() throws Exception { 
        return super.getValue();
    }
}
