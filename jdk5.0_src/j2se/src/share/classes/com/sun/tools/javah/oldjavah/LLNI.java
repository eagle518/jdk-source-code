/*
 * @(#)LLNI.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javah.oldjavah;

import java.io.File;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.util.Hashtable;
import sun.tools.java.ArrayType;
import sun.tools.java.ClassNotFound;
import sun.tools.java.Identifier;
import sun.tools.java.MemberDefinition;
import sun.tools.java.MethodType;
import sun.tools.java.Type;
import sun.tools.tree.Expression;
import sun.tools.tree.DoubleExpression;
import sun.tools.tree.FloatExpression;
import sun.tools.tree.IntExpression;
import sun.tools.tree.LongExpression;

public class LLNI extends Gen {

    protected final char  pathChar = File.separatorChar; 
    protected final char  innerDelim = '$';	/* For inner classes */
    protected Hashtable   doneHandleTypes;
    MemberDefinition[]	  fields;
    private boolean       doubleAlign;
    private int           padFieldNum = 0;

    LLNI(boolean doubleAlign) { this.doubleAlign = doubleAlign; }

    protected String getIncludes() {
	return "";
    }

    protected void write(OutputStream o, String clazz)
    throws ClassNotFoundException {
	String cname     = mangleClassName(clazz);
	PrintWriter pw   = wrapWriter(o);
	fields = env.getLocalFieldsAndMethods(clazz);
	generateDeclsForClass(pw, clazz, cname);
    }

    protected void generateDeclsForClass(PrintWriter pw,
					 String clazz, String cname)
    throws ClassNotFoundException {
	doneHandleTypes  = new Hashtable();
	/* The following handle types are predefined in "typedefs.h". Suppress
	   inclusion in the output by generating them "into the blue" here. */
	genHandleType(null, "java.lang.Class");
	genHandleType(null, "java.lang.ClassLoader");
	genHandleType(null, "java.lang.Object");
	genHandleType(null, "java.lang.String");
	genHandleType(null, "java.lang.Thread");
	genHandleType(null, "java.lang.ThreadGroup");
	genHandleType(null, "java.lang.Throwable");

	pw.println("/* LLNI Header for class " + clazz + " */\n");
	pw.println("#ifndef _Included_" + cname);
	pw.println("#define _Included_" + cname);
	pw.println("#include \"typedefs.h\"");
	pw.println("#include \"llni.h\"");
	pw.println("#include \"jni.h\"\n");
	forwardDecls(pw, clazz);
	structSectionForClass(pw, clazz, cname);
	methodSectionForClass(pw, clazz, cname);
	pw.println("#endif");
    }

    protected void genHandleType(PrintWriter pw, String clazz) {
	String cname = mangleClassName(clazz);
	if (!doneHandleTypes.containsKey(cname)) {
	    doneHandleTypes.put(cname, cname);
	    if (pw != null) {
		pw.println("#ifndef DEFINED_" + cname);
		pw.println("    #define DEFINED_" + cname);
		pw.println("    GEN_HANDLE_TYPES(" + cname + ");");
		pw.println("#endif\n");
	    }
	}
    }

    protected String mangleClassName(String s) { 
	return s.replace('.', '_')
	        .replace(pathChar, '_')
	        .replace(innerDelim, '_'); 
    }

    protected void forwardDecls(PrintWriter pw, String clazz)
    throws ClassNotFoundException {

	if (clazz.equals("java.lang.Object"))
	    return;
	genHandleType(pw, clazz);
	String superClassName = env.getSuperClassName(clazz);
	    forwardDecls(pw, superClassName);

	for (int i = 0; i < fields.length; i++) {
	    MemberDefinition field = fields[i];
	    if ((!field.isStatic() && field.isVariable()) || field.isNative()) {
		String sig = field.getType().getTypeSignature();
		if (sig.charAt(0) != '[')
		    forwardDeclsFromSig(pw, sig);
	    }
	}
    }

    protected void forwardDeclsFromSig(PrintWriter pw, String sig) {
	int    len = sig.length();
	int    i   = sig.charAt(0) == '(' ? 1 : 0;
	/* Skip the initial "(". */
	while (i < len) {
	    if (sig.charAt(i) == 'L') {
		int j = i + 1;
		while (sig.charAt(j) != ';') j++;
		genHandleType(pw, sig.substring(i + 1, j));
		i = j + 1;
	    } else {
		i++;
	    }
	}
    }

    protected void structSectionForClass(PrintWriter pw,
					 String jname, String cname) 
    throws ClassNotFoundException {
	if (cname.equals("java_lang_Object")) {
	  pw.println("/* struct java_lang_Object is defined in typedefs.h. */");
	  pw.println();
	  return;
	}
	pw.println("#if !defined(__i386)");
	pw.println("#pragma pack(4)");
	pw.println("#endif");
	pw.println();
	pw.println("struct " + cname + " {");
	pw.println("    ObjHeader h;");
	pw.print(fieldDefs(jname, cname));
	if (jname.equals("java.lang.Class"))
	    pw.println("    Class *LLNI_mask(cClass);" +
		       "  /* Fake field; don't access (see oobj.h) */");
	pw.println("};\n\n#pragma pack()");
	pw.println();
	return;
    }

    private static class FieldDefsRes {
      public String className;	/* Name of the current class. */
      public FieldDefsRes parent;
      public String s;
      public int byteSize;
      public boolean bottomMost;
      public boolean printedOne = false;

      FieldDefsRes(String className, FieldDefsRes parent, boolean bottomMost) {
	this.className = className; this.parent = parent;
	this.bottomMost = bottomMost;
	int byteSize = 0;
	if (parent == null) this.s = "";
	else this.s = parent.s;
      }
    }

    /* Returns "true" iff added a field. */
    private boolean doField(FieldDefsRes res, MemberDefinition field,
			    String cname, boolean padWord)
      throws ClassNotFoundException {
	if (field.isVariable()) {
	  String fieldDef = addStructMember(field, cname, padWord);
	  if (fieldDef != null) {
	    if (!res.printedOne) { /* add separator */
	      if (res.bottomMost) {
		if (res.s.length() != 0)
		  res.s = res.s + "    /* local members: */\n";
	      } else {
		res.s = res.s + "    /* inherited members from " + 
		  res.className + ": */\n";
	      }
	      res.printedOne = true;
	    }
	    res.s = res.s + fieldDef;
	    return true;
	  }
	}
	// Otherwise.
	return false;
    }

    private int doTwoWordFields(FieldDefsRes res, String clazz,
				int offset, String cname, boolean padWord)
      throws ClassNotFoundException {
	boolean first = true;
	MemberDefinition[] members = env.getLocalFieldsAndMethods(clazz);
	for (int i = 0; i < members.length; i++) {
	  MemberDefinition field = members[i];
	  if (!field.isVariable()) continue;
	  int tc = field.getType().getTypeCode();
	  boolean twoWords = (tc == TC_LONG || tc == TC_DOUBLE);
	  if (twoWords && doField(res, field, cname, first && padWord)) {
	    offset += 8; first = false;
	  }
	}
	return offset;
    }

    protected String fieldDefs(String clazz, String cname)
      throws ClassNotFoundException {
	FieldDefsRes res = fieldDefs(clazz, cname, true);
	return res.s;
    }

    protected FieldDefsRes fieldDefs(String clazz, String cname,
				     boolean bottomMost)
      throws ClassNotFoundException {
	FieldDefsRes res;
	int offset;
	boolean didTwoWordFields = false;
	String supername = env.getSuperClassName(clazz);
	if (supername != null) {
	  res = new FieldDefsRes(clazz,
				 fieldDefs(supername, cname, false),
				 bottomMost);
	  offset = res.parent.byteSize;
	} else {
	  res = new FieldDefsRes(clazz, null, bottomMost);
	  offset = 0;
	}
	
	MemberDefinition[] members = env.getLocalFieldsAndMethods(clazz);
	for (int i = 0; i < members.length; i++) {
	  MemberDefinition field = members[i];
	  if (!field.isVariable()) continue;
	  
	  if (doubleAlign && !didTwoWordFields && (offset % 8) == 0) {
	    offset = doTwoWordFields(res, clazz, offset, cname, false);
	    didTwoWordFields = true;
	  }
	  int tc = field.getType().getTypeCode();
	  boolean twoWords = (tc == TC_LONG || tc == TC_DOUBLE);
	  if (!doubleAlign || !twoWords) {
	    if (doField(res, field, cname, false)) offset += 4;
	  }
	}
	if (doubleAlign && !didTwoWordFields) {
	  if ((offset % 8) != 0) offset += 4;
	  offset = doTwoWordFields(res, clazz, offset, cname, true);
	}
	res.byteSize = offset;
	return res;
    }

    /* OVERRIDE: This method handles instance fields */
    protected String addStructMember(MemberDefinition field, String cname,
				     boolean padWord)
    throws ClassNotFoundException {
      String res = null;
      if (field.isStatic()) {
	  res = addStaticStructMember(field, cname);
	  if (res == null) /* JNI didn't handle it, print comment. */
	    res = "    /* Inaccessible static: " + field + " */\n";
      } else {
	if (padWord) res = "    java_int padWord" + padFieldNum++ + ";\n";
	res = "    " + llniType(field.getType(), false, false) + " " 
	    + llniFieldName(field);
	if (isLongOrDouble(field.getType())) res = res + "[2]";
	res = res + ";\n";
      }
      return res;
    }

    static private final boolean isWindows = 
	System.getProperty("os.name").startsWith("Windows");

    /*
     * This method only handles static final fields.
     */
    protected String addStaticStructMember(MemberDefinition field, String cname)
    throws ClassNotFoundException {
	String res = null;
	Expression exp;

	if (!field.isStatic())
	    return res;
	if (!field.isFinal())
	    return res;

	try {
	    exp = (Expression)field.getValue(env);
	} catch (ClassNotFound e) {
	    throw new ClassNotFoundException(e.getMessage());
	}
	if (exp != null) {
	    /* Constant. */
	    String     cn     = cname + "_" + field.getName().toString();
	    String     suffix = null;
	    long	   val = 0;
	    /* Can only handle int, long, float, and double fields. */
	    if (exp instanceof IntExpression) {
		suffix = "L";
		val = ((Integer) exp.getValue()).intValue();
	    }
	    if (exp instanceof LongExpression) {
		// Visual C++ supports the i64 suffix, not LL
		suffix = isWindows ? "i64" : "LL";
		val = ((Long) exp.getValue()).longValue();
	    }
	    if (exp instanceof FloatExpression)  suffix = "f";
	    if (exp instanceof DoubleExpression) suffix = "";
	    if (suffix != null) {
		// Some compilers will generate a spurious warning
		// for the integer constants for Integer.MIN_VALUE
		// and Long.MIN_VALUE so we handle them specially.
		if ((suffix.equals("L") && (val == Integer.MIN_VALUE)) ||
		    (suffix.equals("LL") && (val == Long.MIN_VALUE))) {
		    res = "    #undef  " + cn + "\n"
			+ "    #define " + cn
			+ " (" + (val + 1) + suffix + "-1)\n";
		} else {
		    res = "    #undef  " + cn + "\n"
			+ "    #define " + cn + " "
			+ exp.getValue().toString()
			+ suffix + "\n";
		}
	    }
	}
	return res;
    }

    protected void methodSectionForClass(PrintWriter pw,
					 String clazz, String cname) 
    throws ClassNotFoundException {
	String methods = methodDecls(clazz, cname);

	if (methods.length() != 0) {
	    pw.println("/* Native method declarations: */\n");
	    pw.println("#ifdef __cplusplus");
	    pw.println("extern \"C\" {");
	    pw.println("#endif\n");
	    pw.println(methods);
	    pw.println("#ifdef __cplusplus");
	    pw.println("}");
	    pw.println("#endif");
	}
    }

    protected String methodDecls(String clazz, String cname)
    throws ClassNotFoundException {

	String res = "";
	for (int i = 0; i < fields.length; i++) {
	    MemberDefinition field = fields[i];
	    if (field.isNative())
		res = res + methodDecl(field, clazz, cname);
	}
	return res;
    }

    protected String methodDecl(MemberDefinition field,
				String clazz, String cname)
    throws ClassNotFoundException {
	String res;
	String sig = field.getType().getTypeSignature();
	boolean longName = needLongName(field, clazz);

	if (sig.charAt(0) != '(')
	    Util.error("invalid.method.signature", sig);
	MethodType mtype = (MethodType)field.getType();
	Type retType = mtype.getReturnType();
	res = "JNIEXPORT " + jniType(retType) + " JNICALL\n"
	    + jniMethodName(field, cname, longName) 
            + "(JNIEnv *, " + cRcvrDecl(field, cname);
	Type argTypes[] = mtype.getArgumentTypes();
	/* It would have been nice to include the argument names in the
	   declaration, but there seems to be a bug in the "BinaryField"
	   class, causing the getArguments() method to return "null" for 
	   most (non-constructor) methods. */
	for (int i = 0; i < argTypes.length; i++)
	    res = res + ", " + jniType(argTypes[i]);
	res = res + ");\n";
	return res;
    }

    protected final boolean needLongName(MemberDefinition method,
					 String clazz)
    throws ClassNotFoundException {
	Identifier mIdent = method.getName();
	for (int i = 0; i < fields.length; i++) {
	    MemberDefinition field = fields[i];
	    if ((field != method) &&
		field.isNative() && (mIdent == field.getName()))
		return true;
	}
	return false;
    }

    protected final String jniMethodName(MemberDefinition method, String cname, 
					 boolean longName) { 
	String res = "Java_" + cname + "_" + method.getName().toString();

	if (longName) {
	    MethodType mTypes = (MethodType)method.getType();
	    Type argTypes[] = mTypes.getArgumentTypes();
	    res = res + "__";
	    for (int i = 0; i < argTypes.length; i++)
		res = res + nameToIdentifier(argTypes[i].getTypeSignature());
	}
	return res;
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
		    /*
		    else if (env.isThrowable(t))
			return "jthrowable";
		     */
		    else 
			return "jobject";
		}
	    }
	}
	Util.bug("jni.unknown.type");
	return null; /* dead code. */
    }

    protected String llniType(Type t, boolean handleize, boolean longDoubleOK) {
	String res = null;
	if (t instanceof ArrayType) {
	    Type elmT = t.getElementType();
	    switch (elmT.getTypeCode()) {
	    case TC_BOOLEAN: res = "IArrayOfBoolean"; break;
	    case TC_BYTE:    res = "IArrayOfByte";    break;
	    case TC_CHAR:    res = "IArrayOfChar";    break;
	    case TC_SHORT:   res = "IArrayOfShort";   break;
	    case TC_INT:     res = "IArrayOfInt";     break;
	    case TC_LONG:    res = "IArrayOfLong";    break;
	    case TC_FLOAT:   res = "IArrayOfFloat";   break;
	    case TC_DOUBLE:  res = "IArrayOfDouble";  break;
	    case TC_ARRAY:
	    case TC_CLASS:   res = "IArrayOfRef";     break;
	    }
	    if (!handleize) res = "DEREFERENCED_" + res;
	} else {
	    switch (t.getTypeCode()) {
	    case TC_VOID:    res = "void"; break;
	    case TC_BOOLEAN:
	    case TC_BYTE:
	    case TC_CHAR:
	    case TC_SHORT:
	    case TC_INT:     res = "java_int"; break;
	    case TC_LONG:    res = longDoubleOK ? "java_long"
				 : "val32 /* java_long */";
	    break;
	    case TC_FLOAT:   res = "java_float"; break;
	    case TC_DOUBLE:  res = longDoubleOK ? "java_double"
				 : "val32 /* java_double */";
	    break;
	    case TC_CLASS:   res = "I" + 
				 mangleClassName(t.getClassName().toString());
	    if (!handleize) res = "DEREFERENCED_" + res;
	    break;
	    }
	}
	return res;
    }

    protected final String cRcvrDecl(MemberDefinition field, String cname) {
	return (field.isStatic() ? "jclass" : "jobject");
    }

    protected String maskName(String s) { 
	return "LLNI_mask(" + s + ")";
    }

    protected String llniFieldName(MemberDefinition field) {
	return maskName(field.getName().toString());
    }

    protected final boolean isLongOrDouble(Type t) {
	int tc = t.getTypeCode();
	return tc == TC_LONG || tc == TC_DOUBLE;
    }

    /* Do unicode to ansi C identifier conversion.
       %%% This may not be right, but should be called more often. */
    protected final String nameToIdentifier(String name) {
	int len = name.length();
	StringBuffer buf = new StringBuffer(len);
	for (int i = 0; i < len; i++) {
	    char c = name.charAt(i);
      
	    if (isASCIILetterOrDigit(c))
		buf.append(c);
	    else if (c == '/')
		buf.append('_');
	    else if (c == '.')
		buf.append('_');
	    else if (c == '_')
		buf.append("_1");
	    else if (c == ';')
		buf.append("_2");
	    else if (c == '[')
		buf.append("_3");
	    else
		buf.append("_0" + ((int)c));
	}
	return new String(buf);
    }

    protected final boolean isASCIILetterOrDigit(char c) {
	if (((c >= 'A') && (c <= 'Z')) ||
	    ((c >= 'a') && (c <= 'z')) ||
	    ((c >= '0') && (c <= '9')))
	    return true;
	else
	    return false;
    }
}
