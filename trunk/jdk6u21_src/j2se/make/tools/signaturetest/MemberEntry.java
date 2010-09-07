/*
 * @(#)MemberEntry.java	1.9 10/03/23
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

/** This class represents member names and signatures which are used by
 *  SignatureTest and APIChangesTest. The original name of the member
 *  is begun with signature constants. It contains all modifiers, type for
 *  field or method, name, throws clause. This name could be formatted by
 *  DefinitionFormat. **/
public class MemberEntry implements SignatureConstants, ClassConstants{
    /** name of the member. **/
    private String entry;
    /** signature of the member. **/
    private String key;
    /** object that initializes current MemberEntry. **/
    protected Object object;
    /** deterninates if the current entry is Synthetic **/
    protected boolean isSynthetic = false;
    /** modifiers. **/
    private int modifiers;
    /** declaring class **/
    private String declaringClass = "";
    
    /** creates definition for given Class
     *  @param cl given Class.
     *  @param converter DefinitionFormat which formats name of the member.
     *  @param isNestedClass specify if this name of the nested class is name
     *  as member of the enclosing class. **/
    public MemberEntry(SignatureClass cl, DefinitionFormat converter,
                       boolean isNestedClass) {
	String name = cl.getName();
        int pos;
 	String s = printClassAccess(cl.getModifiers());
        if (isNestedClass) {
            entry = converter.getDefinition(INNER + s + name);
            key = INNER + name.substring(name.lastIndexOf('$') + 1);
            int k = name.lastIndexOf('$');
            declaringClass = (k < 0) ? "" : name.substring(0, k);
        } else {
            entry = converter.getDefinition(CLASS + s + name);
            key = CLASS + name;
        }
        this.object = cl;
        this.modifiers = cl.getModifiers();
    }

    /** creates definition for given Field
     *  @param field given Field.
     *  @param converter DefinitionFormat which formats name of the member.**/
    public MemberEntry(Field field, DefinitionFormat converter) {
        boolean isQualifiedNames = converter.isQualifiedNamesUsed();
        
	String mod = FIELD + printAccess(field.getModifiers()) + 
	             (field.getType()).getName() + " ";
	if (isQualifiedNames) 
	    mod = mod + field.getDeclaringClass().getName() + ".";
        mod = mod + field.getName();
	entry = converter.getDefinition(mod);
        key = FIELD + field.getName();
        this.object = field;
        this.modifiers = field.getModifiers();
        this.declaringClass = field.getDeclaringClass().getName();
    }
	
    /** creates definition for given Method
     *  @param meth given Method.
     *  @param converter DefinitionFormat which formats name of the member.**/
    public MemberEntry(Method meth, DefinitionFormat converter) {
        boolean isQualifiedNames = converter.isQualifiedNamesUsed();
	Class exceptions[];
	Class returnType;
	int i;
	int startIndex = 0;
	String modifiers = printAccess(meth.getModifiers());

        String retVal = METHOD;
	
	returnType = meth.getReturnType();
	exceptions = meth.getExceptionTypes();

	Class args[] = meth.getParameterTypes();
	// Construct the string
	String signature = meth.getName() + "(";
	if (args.length > 0) {
	    signature = signature + args[0].getName();
	}
	for (int j = 1; j < args.length; j++) {
	    signature = signature + "," + args[j].getName();
	}
	signature = signature +")";
        
        key = METHOD + signature;        
	if (isQualifiedNames)
	    signature = meth.getDeclaringClass().getName() + "." + signature;
	retVal = retVal + modifiers + returnType.getName() + " " + signature;
        String ExceptionName[] = new String[exceptions.length];
        for (int j = 0; j < exceptions.length; j++)
            ExceptionName[j] = exceptions[j].getName();
        sort(ExceptionName);
        if (ExceptionName.length > 0) {
            retVal = retVal + " throws " + ExceptionName[0];
        }
        for (i = 1; i < ExceptionName.length; i++) {
            retVal = retVal + "," + ExceptionName[i];
        }
	entry = converter.getDefinition(retVal);
        this.object = meth;
        this.modifiers = meth.getModifiers();
        this.declaringClass = meth.getDeclaringClass().getName();
    }

    /** creates definition for given Constructor
     *  @param ctor given Constructor.
     *  @param converter DefinitionFormat which formats name of the member.**/
    public MemberEntry(Constructor ctor, DefinitionFormat converter) {
	Class args[];
	Class exceptions[];
	int i;
	int startIndex = 0;
	String modifiers = printAccess(ctor.getModifiers());
	String retVal = CONSTRUCTOR;
        String signature;
	
	exceptions = ctor.getExceptionTypes();

	args = ctor.getParameterTypes();
	// Construct the string
        String name = ctor.getDeclaringClass().getName();
	int pos = Math.max(name.lastIndexOf("."), name.lastIndexOf("$"));
	signature = name.substring(pos + 1) + "(";
	if (args.length > 0) {
	    signature = signature + args[0].getName();
	}
	for (int j = 1; j < args.length; j++) {
	    signature = signature + "," + args[j].getName();
	}
	signature = signature + ")";

        retVal = retVal + modifiers + signature;
	// The exceptions is required in ordering.
            String ExceptionName[] = new String[exceptions.length];
            for (int j = 0; j < exceptions.length; j++)
                ExceptionName[j] = exceptions[j].getName();
            sort(ExceptionName);
            if (ExceptionName.length > 0) {
                retVal = retVal + " throws " + ExceptionName[0];
            }
            for (i = 1; i < ExceptionName.length; i++) {
                retVal = retVal + "," + ExceptionName[i];
            }
	entry = converter.getDefinition(retVal);
        key = CONSTRUCTOR + signature;
        this.object = ctor;
        this.modifiers = ctor.getModifiers();
        this.declaringClass = ctor.getDeclaringClass().getName();
    }

    /** creates definition for given name of the member.
     *  @param ctor given name of the member..
     *  @param converter DefinitionFormat which formats name of the member.**/
    public MemberEntry(String definition, DefinitionFormat converter) {
        this.setData(definition, converter);
    }

    /** creates entry with null fields. **/
    protected MemberEntry() {
    }

    /** set all data of the current definition. **/
    protected void setData(String definition, DefinitionFormat converter) {
        String currentDef = converter.getDefinition(definition);
        entry = currentDef;

        // creates Signature of the member
        String type = currentDef.substring(0, CLASS.length());
        int pos, namePos, localNamePos;
        pos = currentDef.lastIndexOf(" throws ");
        pos = (pos < 0) ? currentDef.length() : pos;
        namePos = currentDef.lastIndexOf('(', pos - 1);
        namePos = (namePos < 0) ? pos : namePos;
        localNamePos = currentDef.lastIndexOf('.', namePos - 1) + 1;
        namePos = currentDef.lastIndexOf(' ', namePos - 1) + 1;
        if (localNamePos < namePos)
            localNamePos = namePos;
        
        String key = currentDef.substring(localNamePos, pos);

        if (currentDef.startsWith(INTERFACE) ||
            currentDef.startsWith(SUPER)) {
            this.key = type;
            entry = currentDef;
        } else if (currentDef.startsWith(INNER)) {
            this.key = type + key.substring(key.lastIndexOf('$') + 1);
            entry = currentDef;
        } else if (currentDef.startsWith(CLASS)) {
            this.key = type + key.substring(key.lastIndexOf(' ') + 1);
            entry = currentDef;
        } else {
            this.key = type + key;
            if (converter.isQualifiedNamesUsed()) {
                entry = currentDef;
            } else {
                String shortDef = currentDef.substring(0, namePos) +
                                  currentDef.substring(localNamePos);
                entry = shortDef;
            }
        }
        this.object = definition;
        declaringClass = definition.substring(definition.lastIndexOf(' ') + 1);
        pos = declaringClass.lastIndexOf('.');
        declaringClass = (pos < 0) ? "" : declaringClass.substring(pos);
    }
        

    /** returns array of the String which is sorted lexicographically.
     *  @param arr array of the String which required to be sorted. **/
    static private String[] sort(String[] arr) {
	String temp;
	int pos;
	for (int j = 0; j < arr.length; j++) {
	    pos = j;
	    for (int i = j + 1; i < arr.length; i++)
		if (arr[i].compareTo(arr[pos]) < 0)
		    pos = i;
	    temp = arr[j];
	    arr[j] = arr[pos];
	    arr[pos] = temp;
	}
	return arr;
    }

    /** convert access flags to the String. **/
    protected static String printAccess(int access) {
	return (
            ((access & ACC_PUBLIC)    != 0 ? "public " : "") +
            ((access & ACC_PRIVATE)   != 0 ? "private " : "") +
            ((access & ACC_PROTECTED) != 0 ? "protected " : "") +
            ((access & ACC_STATIC)    != 0 ? "static " : "") +
            ((access & ACC_TRANSIENT) != 0 ? "transient " : "") +
            ((access & ACC_SYNCHRONIZED) != 0 ? "synchronized " : "") +
            ((access & ACC_ABSTRACT)  != 0 ? "abstract " : "") +
            ((access & ACC_NATIVE)    != 0 ? "native " : "") +
            ((access & ACC_FINAL)     != 0 ? "final " : "" )+
            ((access & ACC_INTERFACE) != 0 ? "interface " : "" ) +
            ((access & ACC_VOLATILE)  != 0 ? "volatile " : "") );
    }

    /** convert access flags of the class to the String. **/
    protected static String printClassAccess(int access) {
	return (
            ((access & ACC_PUBLIC)    != 0 ? "public " : "") +
            ((access & ACC_PRIVATE)   != 0 ? "private " : "") +
            ((access & ACC_PROTECTED) != 0 ? "protected " : "") +
            ((access & ACC_STATIC)    != 0 ? "static " : "") +
            ((access & ACC_TRANSIENT) != 0 ? "transient " : "") +
            ((access & ACC_SYNCHRONIZED) != 0 ? FLAG_SUPER + " " : "") +
            ((((access & ACC_ABSTRACT)  != 0) ||
             ((access & ACC_INTERFACE) != 0)) ? "abstract " : "") +
            ((access & ACC_NATIVE)    != 0 ? "native " : "") +
            ((access & ACC_FINAL)     != 0 ? "final " : "" )+
            ((access & ACC_INTERFACE)  != 0 ? "interface " : "" ) +
            ((access & ACC_VOLATILE)  != 0 ? "volatile " : "") );
    }

    /** returns signature of the member. **/
    public String getKey() {
        return key;
    }

    /** returns name of the member. **/
    public String getEntry() {
        return entry;
    }

    /** check equality of the given and current object **/
    public boolean equals(Object o) {
        if (o instanceof MemberEntry)
            return this.object.equals(((MemberEntry)o).object);
        else
            return super.equals(o);
    }

    /** determinates if the current entry is protected. **/
    public boolean isProtected() {
        if (object instanceof String)
            return (((String)object).indexOf(" protected ") >= 0);
        else
            return Modifier.isProtected(modifiers);
    }

    /** determinates if the current entry is public. **/
    public boolean isPublic() {
        if (object instanceof String)
            return (((String)object).indexOf(" public ") >= 0);
        else
            return Modifier.isPublic(modifiers);
    }

    /** determinates if the current entry is abstract. **/
    public boolean isAbstract() {
        if (object instanceof String)
            return (((String)object).indexOf(" abstract ") >= 0);
        else
            return Modifier.isAbstract(modifiers);
    }

    /** returns current entry as String. **/
    public String toString() {
        return entry;
    }

    public String getDeclaringClass() {
        return declaringClass;
    }

    /** returns true if the current entry is Synthetic. The current
     *  method checks only access methods using rules specified in the Inner
     *  Classes Specification, but this policy could be changed in
     *  the cubclasses **/
    public boolean isSynthetic() {
        if (entry.startsWith(METHOD) && isProtected()) {
            String ts = key.substring(Math.max(key.lastIndexOf(' '),
                                               key.lastIndexOf('.')) + 1);
            if (ts.startsWith("access$")) {
                boolean retVal = (ts.length() > 7);
                for (int k = 7; k < ts.length(); k++)
                    if ((ts.charAt(k) < '0') || (ts.charAt(k) > '9'))
                        retVal = false;
                return retVal;
            }
        }
        return false;
    }
}
