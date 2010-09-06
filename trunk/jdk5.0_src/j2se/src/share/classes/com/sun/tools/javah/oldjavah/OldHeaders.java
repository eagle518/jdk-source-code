/*
 * @(#)OldHeaders.java	1.10 03/12/19
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
import sun.tools.java.ClassDefinition;
import sun.tools.java.ClassDeclaration;
import sun.tools.java.Type;
import sun.tools.java.ArrayType;
import sun.tools.java.ClassType;
import sun.tools.java.MethodType;

/**
 * Generate prototypes for JDK 1.0 style native method interface.  See
 * also OldStubs.
 *
 * @version 1.4, 12/03/01
 */
public class OldHeaders extends Gen {

    private static final String cindent = "    ";
    private static final String cpad    = 
      "char PAD;\t/* ANSI C requires structures to have a least one member */";

    protected String getIncludes() {
	return "#include <native.h>";
    }

    /**
     * Do two things: (1) write the C "struct" for the class, and (2) write
     * "prototypes" for each of the native methods.
     */
    protected void write(OutputStream o, String clazz) 
					throws ClassNotFoundException {
	PrintWriter pw = wrapWriter(o);
	String cname     = Mangle.mangle(clazz, Mangle.Type.CLASS);
	
	MemberDefinition[] jFields  = env.getAllFields(clazz);
	MemberDefinition[] jMethods = env.getNativeMethods(clazz);
	Vector cMethods  = new Vector();
	Vector cFields   = new Vector();
	Vector sfwd      = new Vector();
	
	/*
	 * Recursively translate _jFields_ into _cFields_, going up to the
	 * most super class. Put any required fwd-declarations into _sfwd_, so
	 * these can be written before writing out _cFields_. Return value is
	 * important because we need to pad for empty structs.
	 */
	int    nfields   = getCStruct(clazz, jFields, sfwd, cFields);
	
	/*
	 * Though we use sfwd as a file-global store for fwd decls, at the top
	 * of the file, we must print fwd decls only for the C struct. So
	 * remember this number because sfwd will grow in the next operation.
	 */
	int    nfwds     = sfwd.size();
	
	/*
	 * Translate _jMethods_ to _cMethods_. Avoid duplicate fwd decls by
	 * knowing that _sfwd_ already contains decls for certain types.
	 */
	cMethods         = getCPrototypes(clazz, jMethods, sfwd);
	
	pw.println(guardBegin(cname));

	/*
	 * Now print the C struct.
	 */
	if (!clazz.equals("java.lang.Object") 
	    	&& !clazz.equals("java.lang.Class")) {
	    /* fwd decls from the struct. */
	    int i = 0;
	    for (Enumeration e = sfwd.elements(); 
		 (i++ < nfwds) && e.hasMoreElements();)
		pw.println("struct H" +
			   (String)e.nextElement() + ";");
	    pw.println();
	    
	    String s = Util.getPlatformString("pack.pragma.start");
	    if (s != null) pw.println(s);
	    
	    /* the C struct. */
	    pw.println("typedef struct Class" + cname + " {");
	    for (Enumeration e = cFields.elements(); e.hasMoreElements();)
		pw.println((String)e.nextElement());
	    if (nfields == 0)
		pw.println(cindent + cpad);
	    pw.println("} Class"+cname+";\n"+"HandleTo("+cname+");\n");

	    s  = Util.getPlatformString("pack.pragma.end");
	    if (s != null) pw.println(s);
	}

	/*
	 * Now print the C prototypes.
	 */
	pw.println(cppGuardBegin());
	for (Enumeration e = cMethods.elements(); e.hasMoreElements();) {
	    pw.println((String)e.nextElement());
	}
	pw.println(cppGuardEnd());
	
	pw.println(guardEnd(cname));
    }
    
    /*
     * See comment at call site.
     */
    private int getCStruct(String clazz, MemberDefinition[] jFields,
			   Vector sfwd, Vector cFields) {
	int fcnt = 0;

	if (jFields.length == 0) {
	    return fcnt;
	} 
	
	for (int i = 0; i < jFields.length; i++) {
	    MemberDefinition f = jFields[i];
	    Type t = f.getType();
	    String fname = Mangle.mangle(f.getName().toString(),
				       Mangle.Type.FIELDSTUB);
	    
	    if (f.isStatic()) {
		String s = defineForStatic(clazz, f);
		if (s != null) { 
		    cFields.add(s);
		}
		continue;
	    }

	    /* So this is a really an instance field... */
	    if (t instanceof ClassType || t instanceof ArrayType)
		cFields.add(cindent + getCType(t, FOR_FIELD) + fname + ";");
	    else
		cFields.add(cindent + getCType(t, FOR_FIELD)
			   + " " + fname + ";");
	    fcnt++;
	    addFwddeclIfRequired(sfwd, t, clazz);
	}
	return fcnt;
    }

    /*
     * See comment at call site.
     */
    private Vector getCPrototypes(String clazz, MemberDefinition[] jMethods, 
				  Vector sfwd) {
	Vector entries = new Vector();

	/* fwd decls for types that have first been seen in this prototype. */
	Vector nfwd = new Vector();

	for (int i = 0; i < jMethods.length; i++) {
	    MemberDefinition md = jMethods[i];
	    MethodType mt = (MethodType)md.getType();
	    Type retType = mt.getReturnType();
	    String retCType = getCType(retType, FOR_METHOD);
	    StringBuffer result = new StringBuffer(80);
	    
	    String mangledClazz = Mangle.mangle(clazz, Mangle.Type.CLASS);
	    String mangledMethod = Mangle.mangle(md.getName().toString(),
					       Mangle.Type.FIELDSTUB);

	    result.append("extern ");
	    
	    nfwd.setSize(0);
	    
	    result.append(retCType);                 /* return type */
	    if (!(retType instanceof ArrayType || 
		  retType instanceof ClassType)) {
		result.append(" ");
	    }
	    /* Methods name */
	    result.append(mangledClazz);
	    result.append("_");
	    result.append(mangledMethod);

	    result.append("(");                      /* begin arguments */
	    Type[] args = mt.getArgumentTypes();

	    result.append("struct H");		     /* this pointer */
	    result.append(mangledClazz);
	    if (args.length == 0) {
		result.append(" *");
	    } else {
		result.append(" *,");
	    }

	    for (int j = 0; j < args.length; j++) {  /* other arguments */
		result.append(getCType(args[j], FOR_METHOD));
		if (j != (args.length - 1)) 	     /* no ',' for last arg */
		    result.append(',');
		addFwddeclIfRequired(sfwd, nfwd, args[j], clazz);
	    }
	    addFwddeclIfRequired(sfwd, nfwd, retType, clazz);
	    result.append(");");                     /* end arguments */

	    /* 
	     * Do fwd-declarations for any new types that might have been seen
	     * first only in this prototype. Also make sure that such types
	     * now propogate to the global fwd-declarations set.
	     */
	    for (Enumeration ef = nfwd.elements();ef.hasMoreElements();) {
		String s = (String)ef.nextElement();
		entries.add("struct H" + s + ";");
		sfwd.add(s);
	    }
	    entries.add(result.toString());
	}
	return entries;
    }


    /* 
     * Treat _decls_ as a "set" of types. If the C type of _m_ is not in this
     * set _decls_, then add it. However do not do the addition if the Java
     * name of the type matched _omit_.
     */
    private boolean addFwddeclIfRequired(Vector decls, Type t, String omit) {
	if (t instanceof ClassType) {
	    String cname = 
		Mangle.mangle(((ClassType)t).getClassName().toString(),
			    Mangle.Type.CLASS);
	    if (!decls.contains(cname) && !omit.equals(cname)) {
		decls.add(cname);
		return true;
	    } 
	} else if (t instanceof ArrayType &&
		   ((ArrayType)t).getElementType() instanceof ClassType) {
	    ClassType eType = (ClassType)((ArrayType)t).getElementType();
	    String cname = 
		Mangle.mangle(((ClassType)eType).getClassName().toString(),
			    Mangle.Type.CLASS);
	    if (!decls.contains(cname) && !omit.equals(cname)) {
		decls.add(cname);
		return true;
	    }
	}
	return false;
    }

    /*
     * Same as above, except here we make sure the type is neither in d1 nor
     * in d2, and we add it to d2.
     */
    private boolean addFwddeclIfRequired(Vector d1, Vector d2, 
				      Type t, String omit) {
	if (t instanceof ClassType) {
	    String cname = 
		Mangle.mangle(((ClassType)t).getClassName().toString(),
			    Mangle.Type.CLASS);
	    if (!d1.contains(cname) && !omit.equals(cname)
			&& !d2.contains(cname)) {
		d2.add(cname);
		return true;
	    }
	} else if (t instanceof ArrayType &&
		   ((ArrayType)t).getElementType() instanceof ClassType) {
	    ClassType eType = (ClassType)((ArrayType)t).getElementType();
	    String cname = 
		Mangle.mangle(((ClassType)eType).getClassName().toString(),
			    Mangle.Type.CLASS);
	    if (!d1.contains(cname) && !omit.equals(cname)
		&& !d2.contains(cname)) {
		d2.add(cname);
		return true;
	    }
	}
	return false;
    }
    
    /*
     * Translate a Java type into a C type for declaration
     * purposes. There are subtle differences in how the translation
     * is done depending on whether the type is being generated for a
     * method declartion (prototypes) or object declaration (C
     * struct). 
     */
    private static final int FOR_FIELD  = 0;
    private static final int FOR_METHOD = 1;

    private String getCType(Type t, int ForM) {
	int tc = t.getTypeCode();
	switch (tc) {
	case TC_VOID:    return "void";
	case TC_INT:     return "int32_t";
	case TC_BOOLEAN: return "/*boolean*/ int32_t";
	case TC_LONG:	 return "int64_t";
	case TC_FLOAT:	 return "float";
	case TC_DOUBLE:	 return "double";
	case TC_BYTE:	 return (ForM == FOR_FIELD) ? "int32_t" : "int8_t";
	case TC_SHORT:	 return (ForM == FOR_FIELD) ? "int32_t" : "short";
	case TC_CHAR:	 return (ForM == FOR_FIELD) ? "int32_t" : "unicode";
	case TC_CLASS:
	    {
		String cname = 
		    Mangle.mangle(((ClassType)t).getClassName().toString(),
				  Mangle.Type.CLASS);
		return "struct H" + cname + " *"; 
	    }
	case TC_ARRAY: 
	    {
		String result = null;
		Type elt = ((ArrayType)t).getElementType();
		int eltTC = elt.getTypeCode();
		switch (eltTC) {
		case TC_BOOLEAN: result = "HArrayOfInt *"; break;
		case TC_BYTE:    result = "HArrayOfByte *"; break;
		case TC_CHAR:    result = "HArrayOfChar *"; break;
		case TC_SHORT:   result = "HArrayOfShort *"; break;
		case TC_INT:     result = "HArrayOfInt *"; break;
		case TC_LONG:    result = "HArrayOfLong *"; break;
		case TC_FLOAT:   result = "HArrayOfFloat *"; break;
		case TC_DOUBLE:  result = "HArrayOfDouble *"; break;
		case TC_ARRAY:   result = "HArrayOfArray *"; break;
		case TC_CLASS:   
		    {
			result = "HArrayOfObject *";
			if (ForM == FOR_METHOD && elt == Type.tString)
			    result = "HArrayOfString *";
			break;
		    }
		default:
		    Util.bug("unknown.array.type");
		}
		if (ForM == FOR_METHOD)
		    return result;
		if (ForM == FOR_FIELD)
		    return "struct " + result;
	    }
	default: 
	    /* can't happen */ 
	    Util.bug("unknown.type.for.field");
	}
	return null; /* dead code */
    }
}
