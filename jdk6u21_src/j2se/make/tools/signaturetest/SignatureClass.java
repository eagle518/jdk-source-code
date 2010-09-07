/*
 * @(#)SignatureClass.java	1.10 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.Properties;

/** This is class for founding of the declared members. This class
 *  founds declared methods using reflection, but subclasses could uses
 *  other methods, for example class file parsing. **/
public class SignatureClass implements ClassConstants{
    /** class object which is required in the declared members founding. **/
    Class classObject;
    /** formats definitions. **/
    protected DefinitionFormat filter;
    protected Properties details;
    /** current ClassFinder. **/
    private ClassFinder loader;

    /** creates instance of the SignatureClass with given classObject
     *  and filter.**/
    SignatureClass(Class classObject, DefinitionFormat filter,
                   ClassFinder loader, Properties details) {
        this.details = details;
        this.classObject = classObject;
        this.filter = filter;
        this.loader = loader;
    }

    /** creates null instance **/
    protected SignatureClass() {
    }

    /** returns definition of the class as MemberEntry.
        @param isNestedClass true if required definition as nested classs. **/
    public MemberEntry getMemberEntry(boolean isNestedClass) {
        return new MemberEntry(this, this.filter, isNestedClass);
    }

    /** returns all methods declared by current class. **/
    public MemberEntry[] getDeclaredMethods() {
        Method[] methods = classObject.getDeclaredMethods();
        MemberEntry[] retVal = new MemberEntry[methods.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new MemberEntry(methods[i], filter);
        return retVal;
    }

    /** returns all fields declared by current class. **/
    public MemberEntry[] getDeclaredFields() {
        Field[] fields = classObject.getDeclaredFields();
        MemberEntry[] retVal = new MemberEntry[fields.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new MemberEntry(fields[i], filter);
        return retVal;
    }
    
    /** returns all classes declared by current class. **/
    public SignatureClass[] getDeclaredClasses() {
        Class nested[] = classObject.getDeclaredClasses();
        SignatureClass[] retVal = new SignatureClass[nested.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new SignatureClass(nested[i], filter, loader, details);
        return retVal;
    }

    /** returns all constructors declared by current class. **/
    public MemberEntry[] getDeclaredConstructors() {
        Constructor[] ctors = classObject.getDeclaredConstructors();
        MemberEntry[] retVal = new MemberEntry[ctors.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new MemberEntry(ctors[i], filter);
        return retVal;
    }

    /** returns all interfaces implemented by current class. **/
    public SignatureClass[] getInterfaces() {
        Class interfaces[] = classObject.getInterfaces();
        SignatureClass[] retVal = new SignatureClass[interfaces.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new SignatureClass(interfaces[i], filter, loader,
                                           details);
        return retVal;
    }

    /** returns all public methods included inherited methods.**/
    public MemberEntry[] getMethods() {
        Method[] methods = classObject.getMethods();
        MemberEntry[] retVal = new MemberEntry[methods.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new MemberEntry(methods[i], filter);
        return retVal;
    }

    /** returns all public fields included inherited fields.**/
    public MemberEntry[] getFields() {
        Field[] fields = classObject.getFields();
        MemberEntry[] retVal = new MemberEntry[fields.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new MemberEntry(fields[i], filter);
        return retVal;
    }
    
    /** returns all public classes included inherited classes.**/
    public SignatureClass[] getClasses() {
        Class nested[] = classObject.getClasses();
        SignatureClass[] retVal = new SignatureClass[nested.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new SignatureClass(nested[i], filter, loader, details);
        return retVal;
    }

    /** returns all public constructors.**/
    public MemberEntry[] getConstructors() {
        Constructor[] ctors = classObject.getConstructors();
        MemberEntry[] retVal = new MemberEntry[ctors.length];
        for (int i = 0; i < retVal.length; i++)
            retVal[i] = new MemberEntry(ctors[i], filter);
        return retVal;
    }

    /** returns direct superclass. **/
    public SignatureClass getSuperclass() {
        Class spr = classObject.getSuperclass();
        if (spr == null)
            return null;
        else
            return new SignatureClass(spr, filter, loader, details);
    }

    /** returns class which declares current class. If the cuttent
     *  class regular class then null will be returned. **/
    public SignatureClass getDeclaringClass() {
        Class declClass = classObject.getDeclaringClass();
        if (declClass == null)
            return null;
        else 
            return new SignatureClass(classObject.getDeclaringClass(),
                                      filter, loader, details);
    }

    /** returns qualified name. **/
    public String getName() {
        return classObject.getName();
    }

    /** returns modifiers. **/
    public int getModifiers() {
        String value = details.getProperty("NestedProtected");
        int m = ACC_PUBLIC + ACC_SYNCHRONIZED +
                ACC_ABSTRACT + ACC_FINAL + ACC_INTERFACE;
        m = (value == null) ? m + ACC_PROTECTED : m;
        value = details.getProperty("NestedStatic");
        m = (value == null) ? m + ACC_STATIC : m;
        return classObject.getModifiers() & m;
    }
    
    /** checks equality of the current SignatureClass and Object o. **/
    public boolean equals(Object o) {
        if (o instanceof SignatureClass) 
            return this.classObject.equals(((SignatureClass)o).classObject);
        else
            return super.equals(o);
    }

    /** loads class using the current class loading method. **/
    public SignatureClass loadClass(String name) throws ClassNotFoundException {
        return loader.loadClass(name);
    }
}

    

    
    
