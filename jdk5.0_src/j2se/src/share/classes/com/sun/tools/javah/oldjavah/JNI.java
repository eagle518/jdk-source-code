/*
 * @(#)JNI.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javah.oldjavah;

import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Vector;
import java.util.Enumeration;
import sun.tools.java.MemberDefinition;
import sun.tools.java.Type;
import sun.tools.java.ArrayType;
import sun.tools.java.MethodType;

/**
 * Header file generator for JNI.
 *
 * @version 1.4, 12/03/01
 */
public class JNI extends Gen {

    public String getIncludes() {
	return "#include <jni.h>";
    }
    
    public void write(OutputStream o, String clazz)
	throws ClassNotFoundException {
	String cname = Mangle.mangle(clazz, Mangle.Type.CLASS);
	PrintWriter pw = wrapWriter(o);
	pw.println(guardBegin(cname));
	pw.println(cppGuardBegin());

	/* Write statics. */
	MemberDefinition[] fields = env.getAllFields(clazz);
	for (int i = 0; i < fields.length; i++) {
	    if (!fields[i].isStatic())
		continue;
	    String s = null;
	    s = defineForStatic(clazz, fields[i]);
	    if (s != null) {
		pw.println(s);
	    }
	}
	
	/* Write methods. */
	MemberDefinition[] methods = env.getNativeMethods(clazz);
	for (int i = 0; i < methods.length; i++) {
	    MemberDefinition md = methods[i];
	    MethodType mt = (MethodType)md.getType();
	    String methodName = md.getName().toString();

	    boolean longName = false;
	    for (int j = 0; j < methods.length; j++) {
		if (methods[j] != md &&
		    methodName.equals(methods[j].getName().toString()))
		    longName = true;
	    }

	    pw.println("/*");
	    pw.println(" * Class:     " + cname);
	    pw.println(" * Method:    " + 
		       Mangle.mangle(methodName, Mangle.Type.FIELDSTUB));
	    pw.println(" * Signature: " + mt.getTypeSignature());
	    pw.println(" */");
	    pw.println("JNIEXPORT " + jniType(mt.getReturnType()) +
		       " JNICALL " + 
		       Mangle.mangleMethod(md, clazz,
					 (longName) ? 
					 Mangle.Type.METHOD_JNI_LONG :
					 Mangle.Type.METHOD_JNI_SHORT));
	    pw.print("  (JNIEnv *, ");
	    Type[] args = mt.getArgumentTypes();
	    if (md.isStatic())
		pw.print("jclass");
	    else
		pw.print("jobject");
	    if (args.length > 0)
		pw.print(", ");
	    for (int j = 0; j < args.length; j++) {
		pw.print(jniType(args[j]));
		if (j != (args.length - 1)) {
		    pw.print(", ");
		}
	    }
	    pw.println(");\n");
	}
	
	pw.println(cppGuardEnd());
	pw.println(guardEnd(cname));
    }

    protected final String jniType(Type t) {
	if (t instanceof ArrayType) {
	    Type elmT = t.getElementType();
	    switch (elmT.getTypeCode()) {
	    case TC_BOOLEAN: return "jbooleanArray";
	    case TC_BYTE:    return "jbyteArray";
	    case TC_CHAR:    return "jcharArray";
	    case TC_SHORT:   return "jshortArray";
	    case TC_INT:     return "jintArray";
	    case TC_LONG:    return "jlongArray";
	    case TC_FLOAT:   return "jfloatArray";
	    case TC_DOUBLE:  return "jdoubleArray";
	    case TC_ARRAY:
	    case TC_CLASS:   return "jobjectArray";
	    }
	} else {
	    switch (t.getTypeCode()) {
	    case TC_VOID:    return "void";
	    case TC_BOOLEAN: return "jboolean";
	    case TC_BYTE:    return "jbyte";
	    case TC_CHAR:    return "jchar";
	    case TC_SHORT:   return "jshort";
	    case TC_INT:     return "jint";
	    case TC_LONG:    return "jlong";
	    case TC_FLOAT:   return "jfloat";
	    case TC_DOUBLE:  return "jdouble";
	    case TC_CLASS:   
		{
		    if (t == Type.tString)
			return "jstring";
		    else if (t == Type.tClassDesc)
			return "jclass";
		    else if (env.isThrowable(t))
			return "jthrowable";
		    else 
			return "jobject";
		}
	    }
	}
	Util.bug("jni.unknown.type");
	return null; /* dead code. */
    }
}
    
