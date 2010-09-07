/*
 * @(#)TableOfClass.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.io.PrintWriter;
import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Stack;
import java.util.Vector;

/** The class TableOfClass represents the class with members.
 *  The members are mapped using the signature as keys. For each
 *  key the java.util.Vector is included to the to the instance,
 *  because the more that one member definition with the same signature.This
 *  class provides founding of the protected members using inheritance rules.
 *  This class provides two ways of founding nested classes:<p>- using reflection.
 *  <p>- using previously created table of declared nested classes. **/
public class TableOfClass implements SignatureConstants {
    /** Table of the declared nested classes.**/
    static private ClassCollection nestedClasses = new ClassCollection();
    /** definition of the class. **/
    String classDef;
    /** members of the class. The member signature is mapped by Vector of the
     *  definition with the same signature.**/
    private ClassCollection members;
    /** contains classes which are member of class. **/
    ClassCollection memberClasses;
    /** Class which creates current TableOfClass. **/
    private SignatureClass classObject;
    /** name of the class. **/
    private String name;
    /** formats all definitions. **/
    protected DefinitionFormat converter;
    /** specify if reflection is used for founding of the nested classes. **/
    boolean isReflectUsed = false;

    /** Creates a new TableOfClass for given SignatureClass instance.
     *  @param c  initial class
     *  @param isReflectUsed true if used the java.lang.Class.getDeclaredClasses()
     *  for creating of the nested class definition
     **/
    public TableOfClass(SignatureClass c, boolean isReflectUsed) {
	this(c, null, isReflectUsed);
    }

    /** Creates a new TableOfClass for given nested SignatureClass instance.
     *  @param c  initial class
     *  @param enclClass name of the enclosing class
     *  @param isReflectUsed true if used the java.lang.Class.getDeclaredClasses()
     *  for creating of the nested class definition.**/
    public TableOfClass(SignatureClass c, String enclClass,
                        boolean isReflectUsed) {
	name = (enclClass == null) ? c.getName():
	              (enclClass + "$" + getLocalName(c));
	classObject = c;
        this.isReflectUsed = isReflectUsed;
        this.converter = c.filter;
        if (isReflectUsed)
            memberClasses = new ClassCollection();
	members = new ClassCollection();
	classDef = c.getMemberEntry(false).getEntry();
    }

    
    /** Creates a new TableOfClass for given class definition.
     *  @param c  initial class
     *  @param converter format of the member definitions. **/
    public TableOfClass(String definition, DefinitionFormat converter) {
        this.converter = converter;
	name = definition.substring(definition.lastIndexOf(' ') + 1);
	members = new ClassCollection();
        classDef = converter.getDefinition(definition);
	classObject = null;
        this.isReflectUsed = false;
    }

    /** This method append class with '$' as nested class
     *  and returns Class with name.
     *  @param name qualified name of the nested class. **/
    public static SignatureClass addNestedClass(String name, ClassFinder loader)
        throws ClassNotFoundException {
	SignatureClass c = loader.loadClass(name);
	if (name.indexOf('$') < 0)
	    return null;
	nestedClasses.addElement(name.substring(0, name.lastIndexOf('$')), name);
	return c;
    }

    /** returns class names from table which are declared in the given class.
     *  @param name given class. **/
    public static Vector getDeclaredNestedClasses(String name) {
        Vector h = nestedClasses.get(name);
        return (h == null) ? new Vector() : h;
    }


    /** returns nested classes using reflection. **/
    private static ClassCollection getReflectClasses(SignatureClass c) {
        if (c == null)
            return new ClassCollection();
        ClassCollection retVal = getReflectClasses(c.getSuperclass());
        SignatureClass[] interfaces = c.getInterfaces();
        for (int i = 0; (interfaces != null) && (i < interfaces.length); i++) {
            ClassCollection temp = getReflectClasses(interfaces[i]);
            for (Enumeration e = temp.keys(); e.hasMoreElements();) {
                String key = (String)e.nextElement();
                Vector h = temp.get(key);
                for (int j = 0; j < h.size(); j++) {
                    retVal.addUniqueElement(key, h.elementAt(j));
                }
            }
        }

        SignatureClass[] dec = c.getDeclaredClasses();
        for (int i = 0; (dec != null) && (i < dec.length); i++) {
            String key  = getLocalName(dec[i]);
            if (retVal.get(key) != null)
                retVal.remove(key);
            retVal.addUniqueElement(key, dec[i]);
        }
        return retVal;
    }          


    /** returns local name of the class c. **/
    static private String getLocalName(SignatureClass c) {
	String retVal = c.getName();
	int pos = Math.max(retVal.lastIndexOf("."), retVal.lastIndexOf("$"));
	return retVal.substring(pos + 1);
    }


    /** Returns name of the class. **/
    public String getName() {
	return name;
    }

    /** formats and maps member definition
     *  @param definitions Enumeration of the member definition. **/
    public void createMembers(Enumeration definitions) {
	for (String currentDef = null; definitions.hasMoreElements();) {
	    currentDef = (String)definitions.nextElement();
            String temp = converter.getDefinition(currentDef);
            MemberEntry tempEntry = new MemberEntry(temp, converter);
            MemberEntry entry = new MemberEntry(currentDef, converter);
            if (tempEntry.isPublic() || tempEntry.isProtected() ||
                temp.startsWith(SUPER) || temp.startsWith(INTERFACE))
                members.addElement(entry);
        }
    }

    /** returns definitions of the all nested classes with given name **/
    public Vector getNestedClassDefinitions(String name) {
        Vector retVal = new Vector();
        Vector h = getNestedClass(name);
        if (h == null)
            return new Vector(); 
        for (int i = 0; i < h.size(); i++) {
            SignatureClass c = (SignatureClass)h.elementAt(i);
            retVal.addElement(c.getMemberEntry(true).getEntry());
        }
        return retVal;
    }
    
    /** returns all nested classes with given local name.
     *  @param name given name
     *  @return Vector of the classes. The elements of the
     *  vector are SignatureClass objects. **/
    public Vector getNestedClass(String name) {
        if (isReflectUsed)
            return memberClasses.get(INNER + name);
        else
            return getNestedClass(classObject, name);
    }

    /** Returns name of the class which creates current TableOfClass. **/
    public String getClassName() {
	if (classObject == null)
	    return null;
	return classObject.getName();
    }

    /** Returns Class which creates current TableOfClass. **/
    public SignatureClass getClassObject() {
        return classObject;
    }

    /** Returns the Vector to which the specified signature is mapped. **/
    public Vector get(String key) {
	return members.get(key);
    }

    /** Returns Enumeration of the member's names or signature. **/
    public Enumeration keys() {
	return members.keys();
    }

    /** returns Vector of the nested classes for given enclosing class and local name
     *  @param cl given enclosing class
     *  @param name given local name. **/
    private Vector getNestedClass(SignatureClass cl, String name) {
	if (cl == null)
	    return new Vector(); 
	//found all inherited nested classes with the same name.
	Vector classes = getNestedClass(cl.getSuperclass(), name);
	SignatureClass intf[] = cl.getInterfaces();
	for (int i = 0; i < intf.length; i++) {
	    Vector h = getNestedClass(intf[i], name);
	    for (int j = 0; j < h.size(); j++) {
		Object ob = h.elementAt(j);
		if (classes.indexOf(ob) < 0)
		    classes.addElement(h.elementAt(j));
	    }
	}
	try {
	    SignatureClass c = classObject.loadClass(cl.getName() + "$" + name);
	    //The declared nested class hides all inherited nested classes.
	    Vector h = new Vector();
	    h.addElement(c);
	    return h;
	} catch (ClassNotFoundException e) {
	    return classes;
	} catch (LinkageError er) {
            return classes;
        }
    }
	
    /** creates member definitions for members of the class. **/
    public void createMembers() throws ClassNotFoundException {
	members = new ClassCollection();
	if (classObject == null) 
	    return;
	ClassCollection innrCl = new ClassCollection();
	SignatureClass spr = classObject.getSuperclass();
	members.addElement(SUPER, SUPER + ((spr == null) ? null : spr.getName()));
	SignatureClass[] intfs = getInterfaces();
	for (int i = 0; i < intfs.length; i++)
	    members.addElement(INTERFACE, INTERFACE + intfs[i].getName());
	getMethods();
	getFields();
	//includes public and protected constructors
	MemberEntry[] constr = classObject.getDeclaredConstructors();
	for (int i = 0; i < constr.length; i++) {
	    if (constr[i].isPublic() || constr[i].isProtected())
		members.addElement(constr[i]);
	}
	//includes public and protected nested classes
        if (isReflectUsed)
            innrCl = getReflectClasses(classObject);
        else
            getClasses(classObject, innrCl);
	for (Enumeration e = innrCl.keys(); e.hasMoreElements();) {
	    String nameClass = (String)e.nextElement();
	    Vector h = (Vector)innrCl.get(nameClass);
	    for (int i = 0; i < h.size(); i++) {
		SignatureClass cl = null;
		cl = (SignatureClass)h.elementAt(i);
		if ((Modifier.isPublic(cl.getModifiers())) ||
		    (Modifier.isProtected(cl.getModifiers()))) {
                    MemberEntry entry = cl.getMemberEntry(true);
		    members.addElement(entry);
                    if (isReflectUsed)
                        memberClasses.addElement(entry.getKey(), cl);
		}
	    }
	}
    }

    /** Prints all definitions to given PrintWriter out.**/
    public void writeDefinitions(PrintWriter out) {
	out.println(classDef);
	Vector temp = new Vector();
	for (Enumeration e = members.keys(); e.hasMoreElements();) {
	    Vector h = members.get((String)e.nextElement());
	    for (int i = 0; i < h.size(); i++) {
                String t = (String)h.elementAt(i);
		temp.addElement(t);
	    }
	}
        
	while (!temp.isEmpty()) {
	    String line = (String)temp.elementAt(0);
	    int pos = 0;
	    for (int i = 0; i < temp.size(); i++) 
		if (line.compareTo((String)temp.elementAt(i)) > 0) {
		    line = (String)temp.elementAt(i);
		    pos = i;
		}
	    out.println(line);
	    temp.removeElementAt(pos);
	}
    }

    public boolean isPublic() {
        return classDef.indexOf(" public ") >= 0;
    }
    public boolean isProtected() {
        return classDef.indexOf(" protected ") >= 0;
    }
    /** includes definitions of the given nested classes to ClassCollection.
     *  @param c enclosing class.
     *  @param innrHash ClassCollection which store founded nested classes. **/
    private void getClasses(SignatureClass c, ClassCollection innrHash) 
        throws ClassNotFoundException {
	if (c == null) return;
	// includes inherited methods
	getClasses(c.getSuperclass(), innrHash);
	ClassCollection clFromIntf = new ClassCollection();
	SignatureClass intf[] = c.getInterfaces();
	for (int j = 0; j < intf.length; j++) {
	    getClasses(intf[j], clFromIntf);
	    for (Enumeration e = clFromIntf.keys(); e.hasMoreElements();) {
		String name = (String)e.nextElement();
		Vector h = (Vector)clFromIntf.get(name);
		for (int i = 0; i < h.size(); i++) 
		    innrHash.addUniqueElement(name, h.elementAt(i));
	    }
	    clFromIntf.clear();
	}

	Vector decClass = nestedClasses.get(c.getName());
	if (decClass == null)
	    return;
	for (int i = 0; i < decClass.size(); i++) {
	    String name = (String)decClass.elementAt(i);
	    name = INNER + name.substring(name.lastIndexOf('$') + 1);
	    //The declared nested class hides all inherited nested classes.
	    innrHash.put(name, classObject.loadClass((String)decClass.elementAt(i)));
	}
    }

    /** creates definition of the public and protected methods and
     *  writes it to field members. **/
    private void getMethods() {
	boolean hasNested = false;
	for (SignatureClass t = classObject; t != null; t = t.getSuperclass()) {
	    if (!isReflectUsed && (nestedClasses.get(t.getName()) != null) ||
                isReflectUsed  && (t.getDeclaredClasses().length > 0))
		hasNested = true;
	}

 	ClassCollection methods = getMethods(classObject);

	// Excluding of private and default access methods and access methods.
	for (Enumeration e = methods.keys(); e.hasMoreElements();) {
	    String name = (String)e.nextElement();
	    Vector h = methods.get(name);
	    for (int i = 0; i < h.size(); i++) {
		MemberEntry meth = (MemberEntry)h.elementAt(i);
		// exclude synthetic and privte and package access method.
 		if ((meth.isPublic() || meth.isProtected()) &&
                    !meth.isSynthetic())
		    members.addElement(meth);
	    }
	}
    }

    /** returns all methods of the given class. **/
    private ClassCollection getMethods(SignatureClass c) {
        if (c == null)
            return new ClassCollection();
        ClassCollection methods = getMethods(c.getSuperclass());
        SignatureClass[] intf = c.getInterfaces();
        for (int j = 0; j < intf.length; j++) {
            //the interface members are not protected
            ClassCollection methI = getMethods(intf[j]);
            for (Enumeration e = methI.keys(); e.hasMoreElements();) {
                String key = (String)e.nextElement();
                Vector h = methI.get(key);
                Vector meth = methods.get(key);
                if ((meth == null) || (meth.size() == 1) &&
                    ((MemberEntry)meth.elementAt(0)).isAbstract())
		    //abstract class inherits all abstract methods, 
		    //but inherited non-abstract method implements
		    //all inherited abstract methods.
                    for (Enumeration e1 = h.elements(); e1.hasMoreElements();)
                        methods.addUniqueElement(key, e1.nextElement());
                    
            }
        }
	    
        MemberEntry[] meth = c.getDeclaredMethods();
        for (int i = 0; i < meth.length; i++) {
            String name = meth[i].getKey();
            // The method declared in class overrides all inherited methods 
            methods.put(name, meth[i]);
        }
        return methods;
    }

    /** creates definition of the fields and writes it to the field members. **/
    private void getFields() {
 	ClassCollection fields = getFields(classObject);
	for (Enumeration e = fields.keys(); e.hasMoreElements();) {
	    String name = (String)e.nextElement();
	    Vector h = fields.get(name);
	    for (int i = 0; i < h.size(); i++) {
		MemberEntry field = (MemberEntry)h.elementAt(i);
		if (field.isPublic() || field.isProtected())
		    members.addElement(field);
	    }
	}
    }

    /** returns all fields of the given Class
     *  @param c given Class
     *  @param fields Hashtable which stores fields.**/
    private ClassCollection getFields(SignatureClass c) {
        if (c == null)
            return new ClassCollection();
        ClassCollection retVal = getFields(c.getSuperclass());
        SignatureClass[] interfaces = c.getInterfaces();
        for (int i = 0; i < interfaces.length; i++) {
            ClassCollection fieldI = getFields(interfaces[i]);
            for (Enumeration fields = fieldI.keys(); fields.hasMoreElements();) {
                String name = (String)fields.nextElement();
                Vector h = fieldI.get(name);
                for (Enumeration e = h.elements(); e.hasMoreElements();)
                    retVal.addUniqueElement(name, e.nextElement());
            }
        }
        MemberEntry declaredFields[] = c.getDeclaredFields();
        for (int i = 0; i < declaredFields.length; i++)
            retVal.put(declaredFields[i].getKey(), declaredFields[i]);
        return retVal;
    }
        

    /** returns superinterfaces of given class including inherited
     *  superinterfaces. **/
    public SignatureClass[] getInterfaces() {
	Vector interfaces = new Vector();
        getInterfaces(classObject, interfaces);
        SignatureClass retVal[] = new SignatureClass[interfaces.size()];
	for (int i = 0; i < interfaces.size(); i++)
	    retVal[i] = (SignatureClass)interfaces.elementAt(i);
	return retVal;
    }

    /** writes interfaces of the given class to the Vector. **/
    private void getInterfaces(SignatureClass c, Vector h) {
        if (c == null)
            return;
        getInterfaces(c.getSuperclass(), h);
        SignatureClass intf[] = c.getInterfaces();
        if (c == null)
            return;
        for (int i = 0; i < intf.length; i++) {
            if (!h.contains(intf[i])) {
                h.addElement(intf[i]);
                getInterfaces(intf[i], h);
            }
        }
    }
}
