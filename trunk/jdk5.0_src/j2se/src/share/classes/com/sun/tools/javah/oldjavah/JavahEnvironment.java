/*
 * @(#)JavahEnvironment.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javah.oldjavah;

import sun.tools.java.Environment;
import sun.tools.java.ClassPath;
import sun.tools.java.Type;
import sun.tools.java.Identifier;
import sun.tools.java.BinaryClass;
import sun.tools.java.ClassDefinition;
import sun.tools.java.ClassDeclaration;
import sun.tools.java.MemberDefinition;
import sun.tools.java.ClassFile;
import java.util.Hashtable;
import java.util.Vector;
import java.util.Stack;
import java.io.File;
import java.io.InputStream;
import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.IOException;

/**
 * <code>JavahEnvironment</code> provides classfile structure related
 * services to <code>javah</code>. In particular, it provides lists of
 * native methods and fields of a given class.<p>
 *
 * This class extends <code>sun.tools.java.Environment</code> because
 * <code>BinaryClass.load()</code> requires an environment. An environment,
 * in general, is used by <code>sun.tools.java.*</code> to remember loaded
 * classes, and process error messages. Note that
 * <code>JavahEnvironment</code> is not intended to be a general purpose
 * environment, and implements minimal functionality sufficient to read
 * classfiles (which is why it is package private).<p>
 *
 * <code>javah</code> can not use reflection. If it did, it will not be
 * able to generate header files for newer versions of system classes.
 * 
 * @version 1.5, 12/03/01
 * @see     sun.tools.java.Environment
 * @see     sun.tools.java.BinaryClass
 * @see     sun.tools.java.MemberDefinition
 * @see     sun.tools.java.ClassPath 
 */
class JavahEnvironment extends Environment {

    /**
      * A datastructure to remember loaded <code>ClassDeclaration</code>s.
      * @see sun.tools.java.ClassDeclaration
      */
    Hashtable classDecls = new Hashtable();
    ClassPath classPath = null;
    
    JavahEnvironment(String classPathStr) {
	this.classPath = new ClassPath(classPathStr);
    }

    MemberDefinition[] getNativeMethods(String classname) 
		throws ClassNotFoundException {
	Vector natives = new Vector();
	BinaryClass bc = this.getClass(classname);
	for (MemberDefinition md = bc.getFirstMember();  
	     md != null ; md = md.getNextMember()) {
	    if (md.isNative()) {
		natives.add(md);
	    }
	}
	MemberDefinition[] ret = new MemberDefinition[natives.size()];
	for (int i = 0; i < ret.length; i++) {
	    ret[i] = (MemberDefinition)natives.elementAt(i);
	}
	return ret;
    }

    /**
     * Including super classes' fields.
     */
    MemberDefinition[] getAllFields(String classname) 
		throws ClassNotFoundException {
	Vector fields = new Vector();
	ClassDefinition cd = null;
	Stack s = new Stack();
	
	cd = this.getClass(classname);
	while (true) {
	    s.push(cd);
	    ClassDeclaration c = cd.getSuperClass();
	    if (c == null) break;
	    String superClazz = c.getName().toString();
	    cd = this.getClass(superClazz);
	}
	
	while (!s.empty()) {
	    cd = (ClassDefinition)s.pop();
	    for (MemberDefinition md = cd.getFirstMember();  
		 md != null ; md = md.getNextMember()) {
		if (!md.isMethod()) {
		    fields.add(md);
		}
	    }
	}
	MemberDefinition[] ret = new MemberDefinition[fields.size()];
	for (int i = 0; i < ret.length; i++) {
	    ret[i] = (MemberDefinition)fields.elementAt(i);
	}
	return ret;
    }

    /**
     * Excluding super classes' fields.
     */
    MemberDefinition[] getLocalFieldsAndMethods(String classname) 
		throws ClassNotFoundException {
	Vector fields = new Vector();
	ClassDefinition cd = null;
	
	cd = this.getClass(classname);
	for (MemberDefinition md = cd.getFirstMember();  
	     md != null ; md = md.getNextMember()) {
	    fields.add(md);
	}
	MemberDefinition[] ret = new MemberDefinition[fields.size()];
	for (int i = 0; i < ret.length; i++) {
	    ret[i] = (MemberDefinition)fields.elementAt(i);
	}
	return ret;
    }


    /**
     * Get the superclass name.
     */
    String getSuperClassName(String classname) 
		throws ClassNotFoundException {
	ClassDefinition cd = null;
	
	cd = this.getClass(classname);
	ClassDeclaration c = cd.getSuperClass();
	if (c == null) return null;
	else return c.getName().toString();
    }
	
    /* 
     * Should use weak refs. Hard cache, remembering everthing ever
     * loaded, means that when given a large set of classes, the VM
     * heapsize must be large. But in realistic situations too many
     * classes are not fed to javah, so for now this is okay.
     */
    private Hashtable binClassCache = new Hashtable();

    public BinaryClass getClass(String className) 
        	throws ClassNotFoundException {
        try {
	    BinaryClass b = null;
	    if ((b = (BinaryClass)binClassCache.get(className)) != null) {
		return b;
	    }
	    String fileName = 
		className.replace('.', File.separatorChar).concat(".class");
	    ClassFile cf = classPath.getFile(fileName);
	    if (cf == null) {
		throw new ClassNotFoundException(className);
	    }
	    if (Util.verbose)
		Util.log("[Loaded " + cf.getPath() + "]");
	    InputStream in = cf.getInputStream();
	    b = BinaryClass.load(this, new DataInputStream
				 (new BufferedInputStream(in)));
	    in.close();
	    binClassCache.put(className, b);
	    return b;
	} catch (IOException ioe) {
	    throw new ClassNotFoundException(className);
	}
    }

    public ClassDeclaration getClassDeclaration(Identifier nm) {
	return getClassDeclaration(Type.tClass(nm));
    }
    
    public ClassDeclaration getClassDeclaration(Type t) {
	//System.out.println("@@getClassDeclaration: got ... " + t.toString());
	ClassDeclaration c = (ClassDeclaration)classDecls.get(t);
	if (c == null) {
	    classDecls.put(t, c = new ClassDeclaration(t.getClassName()));
	}
	return c;
    }

    public final boolean isThrowable(Type t) {
	try {
	    ClassDefinition cd = this.getClass(t.toString());
	    while (true) {
		if (cd.getName().toString().equals("java.lang.Throwable")) {
		    return true;
		}
		ClassDeclaration c = cd.getSuperClass();
		if (c == null) break;
		String superClazz = c.getName().toString();
		cd = this.getClass(superClazz);
	    }
	    return false;
	} catch (ClassNotFoundException cnfe) {
	    Util.error("super.class.not.found", cnfe.getMessage());
	}
	return false; /* dead code */
    }

    private boolean verbose = false;
    public void setVerbose(boolean state) {
	this.verbose = state;
    }
}

